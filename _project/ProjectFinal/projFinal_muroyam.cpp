#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <iostream>
#define GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Matt Muroya

// title of these windows:

const char* WINDOWTITLE = "OpenGL / GLUT Sample -- Matt Muroya";
const char* GLUITITLE = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

//const GLfloat BACKCOLOR[ ] = { .07, .11, .13, 1. };
const GLfloat BACKCOLOR[] = { 0.95, 0.95, 0.95, 1. };

// line width for the axes:

const GLfloat AXES_WIDTH = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;

// for lighting:

const float	WHITE[] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
float   OffsetX, OffsetZ;



// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void    KeyUp(unsigned char, int, int);
void    KeyDown(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

void			Axes(float);
void			HsvRgb(float[3], float[3]);
void			Cross(float[3], float[3], float[3]);
float			Dot(float[3], float[3]);
float			Unit(float[3], float[3]);
float			Unit(float[3]);


// utility to create an array from 3 separate values:

float*
Array3(float a, float b, float c)
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float*
MulArray3(float factor, float array0[])
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float*
MulArray3(float factor, float a, float b, float c)
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf(float low, float high)
{
	float r = (float)rand();               // 0 - RAND_MAX
	float t = r / (float)RAND_MAX;       // 0. - 1.

	return   low + t * (high - low);
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed()
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time(&now);
	double seconds = difftime(now, mktime(&y2k));
	unsigned int seed = (unsigned int)(1000. * seconds);    // milliseconds
	srand(seed);
}

// these are here for when you need them -- just uncomment the ones you need:

//#include "setmaterial.cpp"
//#include "setlight.cpp"
//#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
//#include "bmptotexture.cpp"
//#include "loadobjfile.cpp"
//#include "keytime.cpp"
#define GEOMETRY // enable geometry shader
#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"

// Mesh variables

#define GRID_SIZE                 100
#define GRID_RES_LOW              100 // min 2

#define SPEED_SCALE               100

#define VERTS_PER_CELL            6
#define POS_COORDS_PER_VERT       3
#define TEX_COORDS_PER_VERT       2

GLSLProgram  Terrain;

GLuint       VertexBuffer;
GLuint       TexCoordsBuffer;

GLfloat      VertexArray[POS_COORDS_PER_VERT * VERTS_PER_CELL * GRID_RES_LOW * GRID_RES_LOW];
GLfloat      TexCoordsArray[TEX_COORDS_PER_VERT * VERTS_PER_CELL * GRID_RES_LOW * GRID_RES_LOW];

// Input controls

bool wKeyDown, aKeyDown, sKeyDown, dKeyDown;


enum ScrollModes
{
	AUTO,
	MANUAL
};

int CurrentScrollMode = MANUAL;

// Color theme variables

enum ColorThemes
{
	EARTH,
	SOLID,
	WIRE_LIGHT,
	WIRE_DARK,
	SYNTHWAVE,
	TRON,
	HEATMAP,
	NORMAL_MAP
};

int CurrentTheme = EARTH;


// main program:

int
main(int argc, char* argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display lists that **will not change**:

	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// draw the complete scene:

void
Display()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	// specify shading to be flat:

	// glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	//glMatrixMode( GL_PROJECTION );
	//glLoadIdentity( );
	//if( NowProjection == ORTHO )
	//	glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	//else
	//	gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	//gluLookAt(
	//    0.f, 50.f, 0.f,
	//    0.f, 0.f, -250.f,
	//    0.f, 1.f, 0.f
	//);

	// rotate the scene:

	//glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	//glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	//if( Scale < MINSCALE )
	//	Scale = MINSCALE;
	//glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	//if( DepthCueOn != 0 )
	//{
	//	glFogi( GL_FOG_MODE, FOGMODE );
	//	glFogfv( GL_FOG_COLOR, FOGCOLOR );
	//	glFogf( GL_FOG_DENSITY, FOGDENSITY );
	//	glFogf( GL_FOG_START, FOGSTART );
	//	glFogf( GL_FOG_END, FOGEND );
	//	glEnable( GL_FOG );
	//}
	//else
	//{
	//	glDisable( GL_FOG );
	//}

	// possibly draw the axes:

	//if( AxesOn != 0 )
	//{
	//	glColor3fv( &Colors[NowColor][0] );
	//	glCallList( AxesList );
	//}

	// since we are using glScalef( ), be sure the normals get unitized:

	// glEnable( GL_NORMALIZE );

	// ===== Process input =====

	if (CurrentScrollMode == MANUAL)
	{
		if (wKeyDown) OffsetZ -= 1;
		if (aKeyDown) OffsetX -= 1;
		if (sKeyDown) OffsetZ += 1;
		if (dKeyDown) OffsetX += 1;
	}

	if (CurrentScrollMode == AUTO)
	{
		OffsetZ -= 1;
	}

	Terrain.SetUniformVariable("uOffsetX", OffsetX / SPEED_SCALE);
	Terrain.SetUniformVariable("uOffsetZ", OffsetZ / SPEED_SCALE);


	// ===== Model =====

	glm::mat4 modelMatrix = glm::mat4(1.0f); // Identity matrix

	// Scale
	modelMatrix = glm::scale(modelMatrix, glm::vec3(Scale, Scale, Scale));

	// Rotate
	modelMatrix = glm::rotate(modelMatrix, (float)glm::radians(Xrot), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
	modelMatrix = glm::rotate(modelMatrix, (float)glm::radians(Yrot), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis

	// Translate
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-GRID_SIZE / (float)2, 0, -GRID_SIZE / (float)2));

	// Set uniform
	Terrain.SetUniformVariable("uModelMatrix", modelMatrix);

	// ===== Normal =====

	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
	Terrain.SetUniformVariable("uNormalMatrix", normalMatrix);

	// ===== View (Camera) =====

	glm::vec3 cameraPos = glm::vec3(0.0f, 5.f, 20.0f); // Camera position
	glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, -20.0f); // Target position
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector

	glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraDir, cameraUp);
	Terrain.SetUniformVariable("uViewMatrix", viewMatrix);

	// ===== Projection =====

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(70.f), 1.f, 0.1f, 1000.f);
	Terrain.SetUniformVariable("uProjectionMatrix", projectionMatrix);

	// ===== Terrain shader =====

	// Set vertex attribute pointers
	// Note: This could technically be done in InitGraphics(), but doing it here
	// allows us to change the pointers if we were to have more object
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	Terrain.SetAttributePointer3fv("aVertex", 3, (GLfloat*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordsBuffer);
	Terrain.SetAttributePointer3fv("aTexCoords", 2, (GLfloat*)0); // Modified GLSLProgram to be able to accept attribute size 2

	// Set uniforms
	Terrain.SetUniformVariable("uTime", 10 * ElapsedSeconds());
	Terrain.SetUniformVariable("uNormalMap", 0);
	Terrain.SetUniformVariable("uMultiColor", 0);

	// Set conditional values
	if (CurrentTheme == EARTH)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(.65f, .72f, .77f, 1.0);

		Terrain.SetUniformVariable("uMultiColor", 1);

		Terrain.SetUniformVariable("uBaseColor", 1.f, 1.f, 1.f, 1.0f);
		Terrain.SetUniformVariable("uColor0", .17f, .44f, .50f, 1.0f);
		Terrain.SetUniformVariable("uColor1", .45f, .54f, .28f, 1.0f);
		Terrain.SetUniformVariable("uColor2", .46f, .35f, .25f, 1.0f);
		Terrain.SetUniformVariable("uColor3", 1.0f, 1.0f, 1.0f, 1.0f);

		Terrain.SetUniformVariable("uKa", 0.4f);
		Terrain.SetUniformVariable("uKd", 0.8f);
		Terrain.SetUniformVariable("uKs", 0.2f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == HEATMAP)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(.65f, .72f, .77f, 1.0);

		Terrain.SetUniformVariable("uMultiColor", 1);

		Terrain.SetUniformVariable("uBaseColor", 1.f, 1.f, 1.f, 1.0f);
		Terrain.SetUniformVariable("uColor0", 0.f, 0.f, 1.f, 1.0f);
		Terrain.SetUniformVariable("uColor1", 0.f, 1.f, 0.f, 1.0f);
		Terrain.SetUniformVariable("uColor2", 1.f, 1.f, 0.f, 1.0f);
		Terrain.SetUniformVariable("uColor3", 1.0f, 0.f, 0.f, 1.0f);

		Terrain.SetUniformVariable("uKa", 0.2f);
		Terrain.SetUniformVariable("uKd", 0.8f);
		Terrain.SetUniformVariable("uKs", 0.2f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == SOLID)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glClearColor(.65f, .72f, .77f, 1.0);
		glClearColor(.7f, .7f, .7f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", 1.f, 1.f, 1.f, 1.0f);

		Terrain.SetUniformVariable("uKa", 0.4f);
		Terrain.SetUniformVariable("uKd", 0.8f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == WIRE_LIGHT)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", .0f, 0.0f, 0.0f, 1.0f);

		Terrain.SetUniformVariable("uKa", 1.0f);
		Terrain.SetUniformVariable("uKd", 0.0f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == WIRE_DARK)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", 1.f, 1.f, 1.f, 1.f);

		Terrain.SetUniformVariable("uKa", 1.0f);
		Terrain.SetUniformVariable("uKd", 0.0f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == SYNTHWAVE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(0.10f, 0.01f, 0.22f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", .95f, 0.24f, 0.94f, 1.0f);

		Terrain.SetUniformVariable("uKa", 1.0f);
		Terrain.SetUniformVariable("uKd", 0.0f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == TRON)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(.01f, .09f, .12f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", .32f, 0.92f, 0.92f, 1.0f);

		Terrain.SetUniformVariable("uKa", 1.0f);
		Terrain.SetUniformVariable("uKd", 0.0f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}
	if (CurrentTheme == NORMAL_MAP)
	{
		Terrain.SetUniformVariable("uNormalMap", 1);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		Terrain.SetUniformVariable("uBaseColor", 1.0f, 1.0f, 1.0f, 1.0f);

		Terrain.SetUniformVariable("uKa", 0.0f);
		Terrain.SetUniformVariable("uKd", 0.0f);
		Terrain.SetUniformVariable("uKs", 0.0f);
		Terrain.SetUniformVariable("uSh", 0.0f);
	}


	// Draw
	Terrain.Use();
	glDrawArrays(GL_TRIANGLES, 0, 6 * GRID_RES_LOW * GRID_RES_LOW);
	Terrain.UnUse();

	// =====

	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	//glDisable( GL_DEPTH_TEST );
	//glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	/*glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );*/
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}


void
DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoColorMenu(int id)
{
	NowColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// main menu callback:

void
DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoProjectMenu(int id)
{
	NowProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoThemeMenu(int id)
{
	CurrentTheme = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoScrollMenu(int id)
{
	CurrentScrollMode = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString(float x, float y, float z, char* s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}

void GenerateTerrainMesh(
	float gridSize, // Physical height/width
	int   resolution, // Number of x-coordinates
	float vertexArray[],
	float texCoordsArray[]
)
{
	// Width of single grid cell
	float delta = gridSize / (resolution - 1);

	// Vertex array indices
	int i = 0; // Vertex position
	int j = 0; // Texture coordinates

	for (int z = 0; z < (resolution - 1); z++)
	{
		for (int x = 0; x < (resolution - 1); x++)
		{

			// Initial height
			float pNy = 0.f;

			// P0 position                      // P0 texture coordinates
			float p0x = (x + 0) * delta;        float p0s = p0x / gridSize;
			float p0z = (z + 0) * delta;        float p0t = p0z / gridSize;

			// P1 position                      // P1 texture coordinates
			float p1x = (x + 0) * delta;        float p1s = p1x / gridSize;
			float p1z = (z + 1) * delta;        float p1t = p1z / gridSize;

			// P2 position                      // P2 texture coordinates
			float p2x = (x + 1) * delta;        float p2s = p2x / gridSize;
			float p2z = (z + 0) * delta;        float p2t = p2z / gridSize;

			// P3 position                      // P3 texture coordinates
			float p3x = (x + 1) * delta;        float p3s = p3x / gridSize;
			float p3z = (z + 0) * delta;        float p3t = p3z / gridSize;

			// P4 position                      // P4 texture coordinates
			float p4x = (x + 0) * delta;        float p4s = p4x / gridSize;
			float p4z = (z + 1) * delta;        float p4t = p4z / gridSize;

			// P5 position                      // P5 texture coordinates
			float p5x = (x + 1) * delta;        float p5s = p5x / gridSize;
			float p5z = (z + 1) * delta;        float p5t = p5z / gridSize;

			// Store vertex positions
			vertexArray[i++] = p0x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p0z;

			vertexArray[i++] = p1x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p1z;

			vertexArray[i++] = p2x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p2z;

			vertexArray[i++] = p3x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p3z;

			vertexArray[i++] = p4x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p4z;

			vertexArray[i++] = p5x;
			vertexArray[i++] = pNy;
			vertexArray[i++] = p5z;

			// Store texture coordinates
			texCoordsArray[j++] = p0s;
			texCoordsArray[j++] = p0t;

			texCoordsArray[j++] = p1s;
			texCoordsArray[j++] = p1t;

			texCoordsArray[j++] = p2s;
			texCoordsArray[j++] = p2t;

			texCoordsArray[j++] = p3s;
			texCoordsArray[j++] = p3t;

			texCoordsArray[j++] = p4s;
			texCoordsArray[j++] = p4t;

			texCoordsArray[j++] = p5s;
			texCoordsArray[j++] = p5t;
		}
	}
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// Set input options
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc(Animate);

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

	// Create mesh vertex array with normals and texture coordinates

	GenerateTerrainMesh(
		GRID_SIZE,
		GRID_RES_LOW,
		VertexArray,
		TexCoordsArray
	);

	//// Print vertex array values

	//for (int i = 0; i < POS_COORDS_PER_VERT * VERTS_PER_CELL * (GRID_RES_LOW - 1) * (GRID_RES_LOW - 1); i++)
	//{
	//    if (i % 3 == 0) std::cout << '\n';
	//    std::cout << VertexArray[i] << " ";
	//}
	//std::cout << '\n';

	//// Print texture coordinates

	//for (int i = 0; i < TEX_COORDS_PER_VERT * VERTS_PER_CELL * (GRID_RES_LOW - 1) * (GRID_RES_LOW - 1); i++)
	//{
	//    if (i % 2 == 0) std::cout << '\n';
	//    std::cout << TexCoordArray[i] << " ";
	//}
	//std::cout << '\n';

	// Init shader program
	Terrain.Init();

	// Compile, generate error messages, download executable to GPU
	bool valid = Terrain.Create("terrain.vert", "terrain.geom", "terrain.frag");
	if (!valid)
	{
		fprintf(stderr, "Yuch! The shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo! The shader compiled.\n");
	}

	// Set uniforms

	// Generate VBO handles
	glGenBuffers(1, &VertexBuffer);

	glGenBuffers(1, &TexCoordsBuffer);

	// Send vertex VBO send data
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer); // Dock
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexArray), VertexArray, GL_STATIC_DRAW);
	Terrain.EnableVertexAttribArray("aVertex");

	// Send tex coords VBO data
	glBindBuffer(GL_ARRAY_BUFFER, TexCoordsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoordsArray), TexCoordsArray, GL_STATIC_DRAW);
	Terrain.EnableVertexAttribArray("aTexCoords");
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	// Create axes

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}


// initialize the glui window:

void
InitMenus()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(float));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int thememenu = glutCreateMenu(DoThemeMenu);
	glutAddMenuEntry("Earth", EARTH);
	glutAddMenuEntry("Solid", SOLID);
	glutAddMenuEntry("Wire Light", WIRE_LIGHT);
	glutAddMenuEntry("Wire Dark", WIRE_DARK);
	glutAddMenuEntry("Synthwave", SYNTHWAVE);
	glutAddMenuEntry("TRON", TRON);
	glutAddMenuEntry("Heat Map", HEATMAP);
	glutAddMenuEntry("Normal Map", NORMAL_MAP);

	int scrollmenu = glutCreateMenu(DoScrollMenu);
	glutAddMenuEntry("Manual", MANUAL);
	glutAddMenuEntry("Auto", AUTO);

	int mainmenu = glutCreateMenu(DoMainMenu);
	//glutAddSubMenu(   "Axes",          axesmenu);
	//glutAddSubMenu(   "Axis Colors",   colormenu);

	//glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	//glutAddSubMenu(   "Projection",    projmenu );
	glutAddSubMenu("Color theme", thememenu);
	glutAddSubMenu("Scroll mode", scrollmenu);
	glutAddMenuEntry("Reset", RESET);
	//glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'a':
		AxesOn = (AxesOn == 0) ? 1 : 0;
		break;

	case 'o':
	case 'O':
		NowProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		NowProjection = PERSP;
		break;

		//case 'q':
		//case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT);	// will not return here
		break;				// happy compiler

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void KeyDown(unsigned char key, int x, int y)
{
	if (key == 'w') wKeyDown = true;
	if (key == 'a') aKeyDown = true;
	if (key == 's') sKeyDown = true;
	if (key == 'd') dKeyDown = true;

	if (key == ESCAPE) DoMainMenu(QUIT);
}

void KeyUp(unsigned char key, int x, int y)
{
	if (key == 'w') wKeyDown = false;
	if (key == 'a') aKeyDown = false;
	if (key == 's') sKeyDown = false;
	if (key == 'd') dKeyDown = false;
}


// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = 0;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = { 0.f, 1.f, 0.f, 1.f };

static float xy[] = { -.5f, .5f, .5f, -.5f };

static int xorder[] = { 1, 2, -3, 4 };

static float yx[] = { 0.f, 0.f, -.5f, .5f };

static float yy[] = { 0.f, .6f, 1.f, 1.f };

static int yorder[] = { 1, 2, 3, -2, 4 };

static float zx[] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.;			// red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit(float v[3])
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
