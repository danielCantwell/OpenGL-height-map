// assign1.cpp : Defines the entry point for the console application.
//

/*
  CSCI 480 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
  */

#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <GL/glu.h>
#include <GL/glut.h>

int g_iMenuId;
int g_iSubMenuRenderID;
int g_iSubMenuTextureID;
int g_iSubMenuMaterialID;
int g_iSubMenuImageID;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

int screenShotCount = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
typedef enum { R_POINTS, R_LINES, R_TRIANGLES } RENDERTYPE;

CONTROLSTATE g_ControlState = ROTATE;
RENDERTYPE g_RenderType = R_TRIANGLES;

/* state of the world */
float g_vLandRotate[3] = { -46.0, 11.0, 0.0 };
float g_vLandTranslate[3] = { -1.78, 0.86, 0.0 };
float g_vLandScale[3] = { 0.01, 0.01, 1.0 };

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
float ** heightValues;

float heightScale = 1.0;

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	int i, j;
	Pic *in = NULL;

	if (filename == NULL)
		return;

	/* Allocate a picture buffer */
	in = pic_alloc(640, 480, 3, NULL);

	printf("File to save to: %s\n", filename);

	for (i = 479; i >= 0; i--) {
		glReadPixels(0, 479 - i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
			&in->pix[i*in->nx*in->bpp]);
	}

	if (jpeg_write(filename, in))
		printf("File saved Successfully\n");
	else
		printf("Error in Saving\n");

	pic_free(in);
}

float** calculateHeight(const Pic* pic)
{
	float** rgbToGrayScale = new float*[256];
	for (int i = 0; i < 256; i++)
	{
		rgbToGrayScale[i] = new float[256];
	}

	for (int x = 0; x < pic->nx; x++) {
		for (int y = 0; y < pic->ny; y++) {
			int red = PIC_PIXEL(pic, x, y, 0);
			int blue = PIC_PIXEL(pic, x, y, 1);
			int green = PIC_PIXEL(pic, x, y, 2);
			float grayscale = (red + blue + green) / (3.0 * 255.0);
			rgbToGrayScale[y][x] = grayscale;
		}
	}
	return rgbToGrayScale;
}

void myinit()
{
	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	heightValues = calculateHeight(g_pHeightData);
}

void animate()
{

}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glShadeModel(GL_FLAT);
	glLoadIdentity();

	gluLookAt(1.0, 10.0, 2.0, 0.0, 5.0, 0.0, 0.0, 0.0, 1.0);

	glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
	glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
	glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

	switch (g_RenderType) {
	case R_TRIANGLES:
		glBegin(GL_TRIANGLE_STRIP);
		break;
	case R_LINES:
		glBegin(GL_LINES);
		break;
	case R_POINTS:
		glBegin(GL_POINTS);
		break;
	default:
		glBegin(GL_TRIANGLE_STRIP);
		break;
	}


	/* create the vertex points */
	for (int y = 0; y < g_pHeightData->ny - 2; y++) {
		int x = 0;
		/* creating a line of pixels from left to right */
		for (x; x < g_pHeightData->nx; x++) {
			//glColor3f(PIC_PIXEL(g_pHeightData, x, y, 0), PIC_PIXEL(g_pHeightData, x, y, 1), PIC_PIXEL(g_pHeightData, x, y, 2));

			float z = heightValues[x][y] * heightScale;
			glColor3f(z, z, 1.0);
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			glColor3f(z, z, 1.0);
			glVertex3f(x, y + 1, z);
		}
		x--;
		y++;
		/* creating a line of pixels from right to left */
		for (x; x >= 0; x--) {
			//glColor3f(PIC_PIXEL(g_pHeightData, x, y, 0), PIC_PIXEL(g_pHeightData, x, y, 1), PIC_PIXEL(g_pHeightData, x, y, 2));

			float z = heightValues[x][y] * heightScale;
			glColor3f(z, z, 1.0);
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			glColor3f(z, z, 1.0);
			glVertex3f(x, y + 1, z);
		}

	}

	glEnd();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h) /* aspect <= 1 */
		glOrtho(-2.0, 2.0, -2.0 / aspect, 2.0 / aspect, -10.0, 10.0);
	else /* aspect > 1 */
		glOrtho(-2.0 * aspect, 2.0 * aspect, -2.0, 2.0, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
}

/* Response To Menu Item Clicks */
void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	case 1:
		screenShotCount++;
		char c[40] = { '0' };
		sprintf_s(c, "../screenshots/%03d.jpg", screenShotCount);
		saveScreenshot(c);
		break;
	}
}

void renderMenuFunc(int value)
{
	switch (value) {
	case 0:
		g_RenderType = R_POINTS;
		break;
	case 1:
		g_RenderType = R_LINES;
		break;
	case 2:
		g_RenderType = R_TRIANGLES;
		break;
	default:
		break;
	}
}

void textureMenuFunc(int value)
{

}

void materialMenuFunc(int value)
{

}

void imageMenuFunc(int value)
{
	switch (value) {
	case 0:
		g_pHeightData = jpeg_read("spiral.jpg", NULL);
		break;
	case 1:
		g_pHeightData = jpeg_read("GrandTeton-256.jpg", NULL);
		break;
	case 2:
		g_pHeightData = jpeg_read("OhioPyle-256.jpg", NULL);
		break;
	case 3:
		g_pHeightData = jpeg_read("SantaMonicaMountains-256.jpg", NULL);
		break;
	}
	heightValues = calculateHeight(g_pHeightData);
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update */
	glutPostRedisplay();
}

/* converts mouse drags into information about
rotation/translation/scaling */
void mousedrag(int x, int y)
{
	int vMouseDelta[2] = { x - g_vMousePos[0], y - g_vMousePos[1] };

	switch (g_ControlState)
	{
	case TRANSLATE:
		if (g_iLeftMouseButton)
		{
			g_vLandTranslate[0] += vMouseDelta[0] * 0.01;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.01;
		}
		break;
	case ROTATE:
		if (g_iLeftMouseButton)
		{
			g_vLandRotate[0] += vMouseDelta[1];
			g_vLandRotate[1] += vMouseDelta[0];
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandRotate[2] += vMouseDelta[1];
		}
		break;
	case SCALE:
		if (g_iLeftMouseButton)
		{
			g_vLandScale[0] *= 1.0 + vMouseDelta[0] * 0.01;
			g_vLandScale[1] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandScale[2] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state == GLUT_DOWN);
		break;
	}

	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		g_ControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		g_ControlState = SCALE;
		break;
	default:
		g_ControlState = ROTATE;
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void keySpecial(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_PAGE_UP:
		heightScale += 0.2;
		break;
	case GLUT_KEY_PAGE_DOWN:
		heightScale -= 0.2;
		break;
	}
}

void setupMenus()
{
	/* render type sub menu */
	g_iSubMenuRenderID = glutCreateMenu(renderMenuFunc);
	glutSetMenu(g_iSubMenuRenderID);
	glutAddMenuEntry("Points", 0);
	glutAddMenuEntry("Lines", 1);
	glutAddMenuEntry("Triangles", 2);

	/* texture sub menu */
	g_iSubMenuTextureID = glutCreateMenu(textureMenuFunc);
	glutSetMenu(g_iSubMenuTextureID);
	glutAddMenuEntry("None", 0);
	glutAddMenuEntry("First", 1);
	glutAddMenuEntry("Second", 2);

	/* material sub menu */
	g_iSubMenuMaterialID = glutCreateMenu(materialMenuFunc);
	glutSetMenu(g_iSubMenuMaterialID);
	glutAddMenuEntry("Normal", 0);
	glutAddMenuEntry("Shiny", 1);
	glutAddMenuEntry("Reflective", 2);

	/* image sub menu */
	g_iSubMenuImageID = glutCreateMenu(imageMenuFunc);
	glutSetMenu(g_iSubMenuImageID);
	glutAddMenuEntry("Spiral", 0);
	glutAddMenuEntry("Grand Teton", 1);
	glutAddMenuEntry("Ohio Pyle", 2);
	glutAddMenuEntry("Santa Monica Mountains", 3);

	/* create the menu with submenus */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Screenshot", 1);
	glutAddSubMenu("Render As", g_iSubMenuRenderID);
	glutAddSubMenu("Texture", g_iSubMenuTextureID);
	glutAddSubMenu("Material", g_iSubMenuMaterialID);
	glutAddSubMenu("Image", g_iSubMenuImageID);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char* argv[])
{
	// I've set the argv[1] to spiral.jpg.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your texture name for the "Command Arguments"

	if (argc < 2)
	{
		printf("usage: %s heightfield.jpg\n", argv[0]);
		exit(1);
	}

	/* read in height data from jpeg */
	g_pHeightData = jpeg_read((char*)argv[1], NULL);
	if (!g_pHeightData)
	{
		printf("error reading %s.\n", argv[1]);
		exit(1);
	}

	/* initialization */
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/* create window */
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CSCI 420 Assignment 1 - Height Field");

	glEnable(GL_DEPTH_TEST); // used for double buffering

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	/* create the menu that pops up upon pressing the right mouse button */
	setupMenus();

	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	/* callback for keyboard special button press */
	glutSpecialFunc(keySpecial);

	/* do initialization */
	myinit();

	glutMainLoop();
	return 0;
}