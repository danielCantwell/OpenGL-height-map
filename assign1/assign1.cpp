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
#include <time.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* menu IDs */
int g_iMenuId;
int g_iSubMenuRenderID;
int g_iSubMenuShadingID;
int g_iSubMenuTextureID;
int g_iSubMenuMaterialID;
int g_iSubMenuImageID;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

int screenShotCount = 0;
clock_t t;
const float MAX_COUNT = 290;

/* used for options chosen from the user menu */
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
typedef enum { R_POINTS, R_LINES, R_TRIANGLES, R_TRIANGLE_STRIP } RENDERTYPE;
typedef enum { FLAT, SMOOTH } SHADETYPE;
typedef enum { RED, BLUE, GREEN } COLOR;

CONTROLSTATE g_ControlState = ROTATE;
RENDERTYPE g_RenderType = R_TRIANGLE_STRIP;
SHADETYPE g_ShadeType = SMOOTH;
COLOR g_Color = BLUE;

bool wireframeOn = false;

/* state of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
float ** heightValues;

float heightScale = 1.0;

/* values for gluLookAt */
float eyeX = 20.0,		eyeY = 30.0,		eyeZ = 300.0;
float centerX =	128.0,	centerY = 128.0,	centerZ = 0.0;
float upX = 0.0,		upY = 1.0,			upZ = 0.0;

/* values for animation */
bool xGoRight = true;
bool yGoUp = false;
bool zGoOut = true;

bool animateImage = false;

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
	int bpp = pic->bpp;
	int width = pic->nx;
	int height = pic->ny;

	float** rgbToGrayScale = new float*[width];
	for (int i = 0; i < width; i++)
	{
		rgbToGrayScale[i] = new float[height];
	}

	for (int x = 0; x < width; x ++) {
		for (int y = 0; y < height; y ++) {
			int red = PIC_PIXEL(pic, x, y, 0);
			int blue = PIC_PIXEL(pic, x, y, 1);
			int green = PIC_PIXEL(pic, x, y, 2);
			float grayscale = (red + blue + green) / (3.0 * 10);
			rgbToGrayScale[y / bpp][x / bpp] = grayscale;
		}
	}
	return rgbToGrayScale;
}

void myinit()
{
	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	heightValues = calculateHeight(g_pHeightData);
	t = clock();
}

/* define the animation */
void animate()
{
	if (xGoRight) {
		eyeX += 1;
		if (eyeX > g_pHeightData->nx)
			xGoRight = false;
	}
	else {
		eyeX -= 1;
		if (eyeX < 2)
			xGoRight = true;
	}

	if (yGoUp) {
		eyeY += 1;
		if (eyeY > g_pHeightData->ny)
			yGoUp = false;
	}
	else {
		eyeY -= 1;
		if (eyeY < 2)
			yGoUp = true;
	}

	if (zGoOut) {
		eyeZ += 1;
		if (eyeZ > g_pHeightData->nx)
			zGoOut = false;
	}
	else {
		eyeZ -= 1;
		if (eyeZ < (30 * heightScale))
			zGoOut = true;
	}

	clock_t newT = clock() - t;
	float sec = ((float)newT) / CLOCKS_PER_SEC;
	if ((sec > 0.06) && (screenShotCount < MAX_COUNT)) {
		screenShotCount++;
		char c[20];
		_itoa_s(screenShotCount, c, 10);
		saveScreenshot(c);
		t = clock();
	}
}

/* sets the vertex color based on the chosen color and height value of the vertex*/
void setVertexColor(float z)
{
	if (g_Color == BLUE)
		glColor3f(z / (25.0 * heightScale), z / (25.0 * heightScale), 1.0);
	else if (g_Color == RED)
		glColor3f(1.0, z / (25.0 * heightScale), z / (25.0 * heightScale));
	else if (g_Color == GREEN)
		glColor3f(z / (25.0 * heightScale), 1.0, z / (25.0 * heightScale));
}

/* creates the vertices of the heightmap */
void renderHeightMap()
{
	int bpp = g_pHeightData->bpp;
	int width = g_pHeightData->nx;
	int height = g_pHeightData->ny;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* render type can be decided by the user in the menu*/
	switch (g_RenderType) {
	case R_TRIANGLE_STRIP:
		glBegin(GL_TRIANGLE_STRIP);
		break;
	case R_TRIANGLES:
		glBegin(GL_TRIANGLES);
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
	for (int y = 0; y < height - 2; y++) {
		int x = 0;
		/* creating a line of pixels from left to right */
		for (x; x < width; x++) {

			float z = heightValues[x][y] * heightScale;
			setVertexColor(z);
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			setVertexColor(z);
			glVertex3f(x, y + 1, z);
		}
		x--;
		y++;
		/* creating a line of pixels from right to left */
		for (x; x >= 0; x--) {

			float z = heightValues[x][y] * heightScale;
			setVertexColor(z);
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			setVertexColor(z);
			glVertex3f(x, y + 1, z);
		}

	}

	glEnd();
}

/* creates the wireframe for the heightmap */
void renderWireFrame()
{
	int bpp = g_pHeightData->bpp;
	int width = g_pHeightData->nx;
	int height = g_pHeightData->ny;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonOffset(1, 1);

	glBegin(GL_TRIANGLE_STRIP);
	glColor3f(0.0, 0.0, 0.0);
	/* create the vertex points */
	for (int y = 0; y < g_pHeightData->ny - 2; y++) {
		int x = 0;
		/* creating a line of pixels from left to right */
		for (x; x < g_pHeightData->nx; x++) {
			//glColor3f(PIC_PIXEL(g_pHeightData, x, y, 0), PIC_PIXEL(g_pHeightData, x, y, 1), PIC_PIXEL(g_pHeightData, x, y, 2));

			float z = heightValues[x][y] * heightScale;
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			glVertex3f(x, y + 1, z);
		}
		x--;
		y++;
		/* creating a line of pixels from right to left */
		for (x; x >= 0; x--) {
			//glColor3f(PIC_PIXEL(g_pHeightData, x, y, 0), PIC_PIXEL(g_pHeightData, x, y, 1), PIC_PIXEL(g_pHeightData, x, y, 2));

			float z = heightValues[x][y] * heightScale;
			glVertex3f(x, y, z);

			z = heightValues[x][y + 1] * heightScale;
			glVertex3f(x, y + 1, z);
		}

	}
	glDisable(GL_POLYGON_OFFSET_LINE);
	glEnd();
}

/* used for doing all three transformations : translate, rotate, scale */
void transformObject()
{
	/* object translation*/
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	/* object rotation */
	glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
	glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
	glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
	/* object scaling */
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
}

/***** MAIN DISPLAY FUNCTION *****/
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	/* shading model can be decided by the user in the menu */
	if (g_ShadeType == SMOOTH) {
		glShadeModel(GL_SMOOTH);
	}
	else {
		glShadeModel(GL_FLAT);
	}

	glLoadIdentity();

	/* camera view */
	gluLookAt(eyeX,		eyeY,		eyeZ,
			  centerX,	centerY,	centerZ,
			  upX,		upY,		upZ);

	/* translate, rotate, scale */
	transformObject();

	/* create the heightmap */
	renderHeightMap();

	/* if enabled, create the wireframe */
	if (wireframeOn) {
		renderWireFrame();
	}

	/* needed for double buffering*/
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* Perspective camera view */
	gluPerspective(60.0, aspect, .01, 1000);
	glMatrixMode(GL_MODELVIEW);
}

/* Response To Main Menu Item Clicks */
void menufunc(int value)
{
	switch (value)
	{
		/* exit the program */
	case 0:
		exit(0);
		break;
		/* save a screenshot with the correct name */
	case 1:
		screenShotCount++;
		char c[20];
		_itoa_s(screenShotCount, c, 10);
		saveScreenshot(c);
		break;
		/* toggle animation */
	case 2:
		animateImage = !animateImage;
		break;
	}
}

/* sub menu render clicks */
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
	case 3:
		g_RenderType = R_TRIANGLE_STRIP;
	default:
		break;
	}
}

/* sub menu shading clicks */
void shadingMenuFunc(int value)
{
	switch (value)
	{
	case 0:
		g_ShadeType = SMOOTH;
		break;
	case 1:
		g_ShadeType = FLAT;
	}
}

/* sub menu texture clicks */
void textureMenuFunc(int value)
{
	switch (value)
	{
	case 0:
		g_Color = RED;
		break;
	case 1:
		g_Color = GREEN;
		break;
	case 2:
		g_Color = BLUE;
		break;
	}
}

/* sub menu material clicks */
void materialMenuFunc(int value)
{

}

/* sub menu image clicks */
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
	case 4:
		g_pHeightData = jpeg_read("colorImage.jpg", NULL);
		glLoadIdentity();
		g_vLandRotate[2] = -90;
		break;
	}
	heightValues = calculateHeight(g_pHeightData);
}

/* called when no other events are being called */
void doIdle()
{
	if (animateImage)
		animate();

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
			g_vLandTranslate[0] += vMouseDelta[0] * 0.02;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.02;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.02;
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

/* mouse button clicks */
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

/* keyboard buttons presses */
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
		wireframeOn = !wireframeOn;
		break;
	case 'r':
		g_Color = RED;
		break;
	case 'b':
		g_Color = BLUE;
		break;
	case 'g':
		g_Color = GREEN;
		break;
	case 'p':
		g_RenderType = R_POINTS;
		break;
	case 'l':
		g_RenderType = R_LINES;
		break;
	case 't':
		g_RenderType = R_TRIANGLE_STRIP;
		break;
	case 'f':
		g_ShadeType = FLAT;
		break;
	case 's':
		g_ShadeType = SMOOTH;
		break;
	case 'i':
		screenShotCount++;
		char c[20];
		_itoa_s(screenShotCount, c, 10);
		saveScreenshot(c);
		break;
	}
}

/* keyboard special button presses */
void keySpecial(int key, int x, int y)
{
	switch (glutGetModifiers()) {
		/* change eyeZ position. essentially, zoom in and out */
	case GLUT_ACTIVE_ALT:
		switch (key) {
		case GLUT_KEY_UP:
			eyeZ -= 4.0;
			break;
		case GLUT_KEY_DOWN:
			eyeZ += 4.0;
			break;
		}
		break;
		/* change the center position - where the camera is looking */
	case GLUT_ACTIVE_CTRL:
		switch (key) {
		case GLUT_KEY_UP:
			centerY += 4.0;
			break;
		case GLUT_KEY_DOWN:
			centerY -= 4.0;
			break;
		case GLUT_KEY_LEFT:
			centerX -= 4.0;
			break;
		case GLUT_KEY_RIGHT:
			centerX += 4.0;
			break;
		}
		break;
		/* change the eye position - where the camera is positioned */
	case GLUT_ACTIVE_SHIFT:
		switch (key) {
		case GLUT_KEY_UP:
			eyeY += 4.0;
			break;
		case GLUT_KEY_DOWN:
			eyeY -= 4.0;
			break;
		case GLUT_KEY_LEFT:
			eyeX -= 4.0;
			break;
		case GLUT_KEY_RIGHT:
			eyeX += 4.0;
			break;
		}
		break;
		/* change the eye and center positions */
	default:
		switch (key) {
		case GLUT_KEY_UP:
			eyeY += 4.0;
			centerY += 4.0;
			break;
		case GLUT_KEY_DOWN:
			eyeY -= 4.0;
			centerY -= 4.0;
			break;
		case GLUT_KEY_LEFT:
			eyeX -= 4.0;
			centerX -= 4.0;
			break;
		case GLUT_KEY_RIGHT:
			eyeX += 4.0;
			centerX += 4.0;
			break;
		}
		break;
	}

	/* change the heightscale value for the heightmap */
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
	glutAddMenuEntry("Triangle Strip", 3);

	/* shading type sub menu */
	g_iSubMenuShadingID = glutCreateMenu(shadingMenuFunc);
	glutSetMenu(g_iSubMenuShadingID);
	glutAddMenuEntry("Smooth", 0);
	glutAddMenuEntry("Flat", 1);

	/* texture sub menu */
	g_iSubMenuTextureID = glutCreateMenu(textureMenuFunc);
	glutSetMenu(g_iSubMenuTextureID);
	glutAddMenuEntry("Red", 0);
	glutAddMenuEntry("Green", 1);
	glutAddMenuEntry("Blue", 2);

	/* material sub menu */
	g_iSubMenuMaterialID = glutCreateMenu(materialMenuFunc);
	glutSetMenu(g_iSubMenuMaterialID);
	glutAddMenuEntry("Normal", 0);
	glutAddMenuEntry("Shiny", 1);
	glutAddMenuEntry("Reflective", 2);

	/* image sub menu */
	g_iSubMenuImageID = glutCreateMenu(imageMenuFunc);
	glutSetMenu(g_iSubMenuImageID);
	glutAddMenuEntry("Spiral - 8bit - 256x256", 0);
	glutAddMenuEntry("Grand Teton - 8bit - 256x256", 1);
	glutAddMenuEntry("Ohio Pyle - 8bit - 256x256", 2);
	glutAddMenuEntry("Santa Monica Mountains - 8bit - 256x256", 3);
	glutAddMenuEntry("Color Image - 24bit - 1000x750", 4);

	/* create the menu with submenus */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Screenshot", 1);
	glutAddMenuEntry("Animate", 2);
	glutAddSubMenu("Render As", g_iSubMenuRenderID);
	glutAddSubMenu("Shading", g_iSubMenuShadingID);
	glutAddSubMenu("Texture", g_iSubMenuTextureID);
	glutAddSubMenu("Material", g_iSubMenuMaterialID);
	glutAddSubMenu("Image", g_iSubMenuImageID);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char* argv[])
{
	/* check if command line arguments have been supplied */
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

	/* used for double buffering */
	glEnable(GL_DEPTH_TEST);

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
	/* callback for keyboard button press */
	glutKeyboardFunc(keyboard);
	/* callback for keyboard special button press */
	glutSpecialFunc(keySpecial);

	/* do initialization */
	myinit();

	glutMainLoop();
	return 0;
}