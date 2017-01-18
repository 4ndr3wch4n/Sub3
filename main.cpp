/*******************************************************************
	       Multi-Part Model Construction and Manipulation
********************************************************************/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"
#include <ctime>

#define M_PI 3.14159265358979323846

void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
VECTOR3D ScreenToWorld(int x, int y);
void spinAccelerate(void);
void makeEnemySub(void);

static GLfloat theta = 0.0;
static GLfloat thetay = 0.0;
static GLfloat theta2 = 0.0;
static GLfloat spin = 0.0;
static GLfloat spin2 = 0.0;
static float spinSpeed = 0.0;
static float moveSub = 0.0;
static float moveSubz = 0.0;
static float subSpeed = 0.0;
static float moveSubVert = 4.0;
static float xSub = 0.0;
static float z = 0.0;
static float xSub2 = 0.0;
static float z2 = 0.0;
static int currentButton;
static unsigned char currentKey;

GLfloat light_position0[] = { -6.0,  12.0, 0.0,1.0 };
GLfloat light_position1[] = { 6.0,  12.0, 0.0,1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

GLUquadricObj *mySphere;
GLUquadricObj *mySphere2;
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];
GLfloat winX, winY, winZ;
GLdouble posX, posY, posZ;
GLint position = 0;
GLint vertice = 0;
GLint prevX = 0;
GLint prevY = 0;
time_t t;
boolean checkEnemySub = false;
GLfloat t1 = 0;
GLfloat t2 = 0;
GLfloat t3 = 0;
int randomMove = 0;
int counter = 0;
float angles = 0.0;
int main_window;
boolean checkBullet = false;
float bulletX = 0.0;
float bulletY = 0.0;
float bulletZ = 0.0;
float bulletDirection = 0.0;
boolean hit = false;
int bulletCounter = 0;
boolean checkPeri = false;
GLfloat yCam = 0.0;

boolean cameraMode = false;
GLfloat previousX = 0.0;
GLfloat previousY = 0.0;
GLfloat xFrom = 0.0;
GLfloat yFrom = 6.0;
GLfloat zFrom = 22.0;
GLfloat xTo = 0.0;
GLfloat yTo = 0.0;
GLfloat zTo = 0.0;
GLfloat yUp = 1.0;
GLfloat upTheta = 1.0;

QuadMesh *groundMesh = NULL;

// Set up lighting/shading and material properties for submarine - upcoming lecture - just copy for now
GLfloat submarine_mat_ambient[] = { 0.4, 0.2, 0.0, 1.0 };
GLfloat submarine_mat_specular[] = { 0.1, 0.1, 0.0, 1.0 };
GLfloat submarine_mat_diffuse[] = { 0.9, 0.5, 0.0, 1.0 };
GLfloat submarine_mat_shininess[] = { 0.0 };

typedef struct RGB
{
	byte r, g, b;
} RGB;

typedef struct RGBpixmap
{
	int nRows, nCols;
	RGB *pixel;
} RGBpixmap;

RGBpixmap pix[2];

struct BoundingBox
{
  VECTOR3D min;
  VECTOR3D max;
};

BoundingBox BBox;
BoundingBox enemySub;
BoundingBox missle;

// Default Mesh Size
int meshSize = 64;

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef unsigned short ushort;
typedef unsigned long  ulong;

void fskip(FILE *fp, int num_bytes)
{
	int i;
	for (i = 0; i<num_bytes; i++)
		fgetc(fp);
}

ushort getShort(FILE *fp) //helper function
{ //BMP format uses little-endian integer types
  // get a 2-byte integer stored in little-endian form
	char ic;
	ushort ip;
	ic = fgetc(fp); ip = ic;  //first byte is little one 
	ic = fgetc(fp);  ip |= ((ushort)ic << 8); // or in high order byte
	return ip;
}

ulong getLong(FILE *fp) //helper function
{  //BMP format uses little-endian integer types
   // get a 4-byte integer stored in little-endian form
	ulong ip = 0;
	char ic = 0;
	unsigned char uc = ic;
	ic = fgetc(fp); uc = ic; ip = uc;
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 8);
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 16);
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 24);
	return ip;
}

void readBMPFile(RGBpixmap *pm, char *file)
{
	FILE *fp;
	long index;
	int k, row, col, numPadBytes, nBytesInRow;
	char ch1, ch2;
	ulong fileSize;
	ushort reserved1;    // always 0
	ushort reserved2;     // always 0 
	ulong offBits; // offset to image - unreliable
	ulong headerSize;     // always 40
	ulong numCols; // number of columns in image
	ulong numRows; // number of rows in image
	ushort planes;      // always 1 
	ushort bitsPerPixel;    //8 or 24; allow 24 here
	ulong compression;      // must be 0 for uncompressed 
	ulong imageSize;       // total bytes in image 
	ulong xPels;    // always 0 
	ulong yPels;    // always 0 
	ulong numLUTentries;    // 256 for 8 bit, otherwise 0 
	ulong impColors;       // always 0 
	long count;
	char dum;

	/* open the file */
	if ((fp = fopen(file, "rb")) == NULL)
	{
		printf("Error opening file %s.\n", file);
		exit(1);
	}

	/* check to see if it is a valid bitmap file */
	if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n", file);
		exit(1);
	}

	fileSize = getLong(fp);
	reserved1 = getShort(fp);    // always 0
	reserved2 = getShort(fp);     // always 0 
	offBits = getLong(fp); // offset to image - unreliable
	headerSize = getLong(fp);     // always 40
	numCols = getLong(fp); // number of columns in image
	numRows = getLong(fp); // number of rows in image
	planes = getShort(fp);      // always 1 
	bitsPerPixel = getShort(fp);    //8 or 24; allow 24 here
	compression = getLong(fp);      // must be 0 for uncompressed 
	imageSize = getLong(fp);       // total bytes in image 
	xPels = getLong(fp);    // always 0 
	yPels = getLong(fp);    // always 0 
	numLUTentries = getLong(fp);    // 256 for 8 bit, otherwise 0 
	impColors = getLong(fp);       // always 0 

	if (bitsPerPixel != 24)
	{ // error - must be a 24 bit uncompressed image
		printf("Error bitsperpixel not 24\n");
		exit(1);
	}
	//add bytes at end of each row so total # is a multiple of 4
	// round up 3*numCols to next mult. of 4
	nBytesInRow = ((3 * numCols + 3) / 4) * 4;
	numPadBytes = nBytesInRow - 3 * numCols; // need this many
	pm->nRows = numRows; // set class's data members
	pm->nCols = numCols;
	pm->pixel = (RGB *)malloc(3 * numRows * numCols);//make space for array
	if (!pm->pixel) return; // out of memory!
	count = 0;
	dum;
	for (row = 0; row < numRows; row++) // read pixel values
	{
		for (col = 0; col < numCols; col++)
		{
			int r, g, b;
			b = fgetc(fp); g = fgetc(fp); r = fgetc(fp); //read bytes
			pm->pixel[count].r = r; //place them in colors
			pm->pixel[count].g = g;
			pm->pixel[count++].b = b;
		}
		fskip(fp, numPadBytes);
	}
	fclose(fp);
}

void setTexture(RGBpixmap *p, GLuint textureID)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p->nCols, p->nRows, 0, GL_RGB,
		GL_UNSIGNED_BYTE, p->pixel);
}

int main(int argc, char **argv)
{
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  main_window = glutCreateWindow("Scene Modeller");

  initOpenGL(500,500);
  mySphere = gluNewQuadric();
  mySphere2 = gluNewQuadric();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotionHandler);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(functionKeys);
  glutSetWindow(main_window);
  glutIdleFunc(spinAccelerate);

  glutMainLoop();
  return 0;
}



// Setup openGL */
void initOpenGL(int w, int h)
{
  // Set up viewport, projection, then change to modelview matrix mode - 
  // display function will then set up camera and modeling transforms
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0,1.0,0.2,80.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set up and enable lighting
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  
  glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_LIGHT1);
  
  // Other OpenGL setup
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.6, 0.6, 0.6, 0.0);  
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  // This one is important - renormalize normal vectors 
  glEnable(GL_NORMALIZE);
  
  //Nice perspective.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT , GL_NICEST);
  
  // Set up ground quad mesh
  VECTOR3D origin  = VECTOR3D(-8.0f,0.0f,8.0f);
  VECTOR3D dir1v   = VECTOR3D(1.0f, 0.0f, 0.0f);
  VECTOR3D dir2v   = VECTOR3D(0.0f, 0.0f,-1.0f);
  groundMesh = new QuadMesh(meshSize, 16);
  groundMesh->InitMesh(meshSize, origin, 16, 16, dir1v, dir2v);
  
  VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
  VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
  VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
  float shininess = 0.2;

  readBMPFile(&pix[0], "doge.bmp");  // read texture
  setTexture(&pix[0], 2000);          // assign a unique identifier
  readBMPFile(&pix[1], "cat.bmp");
  setTexture(&pix[1], 2001);

  groundMesh->createBlob();
  groundMesh->SetMaterial(ambient,diffuse,specular,shininess);

  // Set up the bounding box of the scene
  // Currently unused. You could set up bounding boxes for your objects eventually.
  BBox.min.Set(-8.0f, 0.0, -8.0);
  BBox.max.Set( 8.0f, 6.0,  8.0);
}



void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glLoadIdentity();

	// Set up the camera
	gluLookAt(xFrom, yFrom, zFrom, 0, 0, 0, 0.0, yUp, 0.0);

	if (checkPeri == true) {
		glTranslatef(xSub, moveSubVert + 6, z);
		glRotatef(-yCam, 0.0, 0.0, 1.0);
		glTranslatef(-xSub, -moveSubVert - 6, -z);
	}

	glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

	//Draw a sphere
	// Apply transformations to construct submarine
	if (checkBullet == true) {
		bulletX -= (0.05 * cos(bulletDirection / 180.0*M_PI));
		bulletZ += (0.05 * sin(bulletDirection / 180.0*M_PI));
		glPushMatrix();
		glTranslatef(xSub+bulletX, bulletY, z+bulletZ);
		glScalef(0.5, 0.5, 0.5);
		gluQuadricDrawStyle(mySphere, GLU_FILL);
		gluQuadricTexture(mySphere, GLU_TRUE);
		gluQuadricNormals(mySphere, GLU_SMOOTH);
		gluSphere(mySphere, 1.0, 20, 20);
		glPopMatrix();
	}

	missle.min.Set(bulletX, bulletY, bulletZ);
	//missle.max.Set(bulletX + 1, moveSubVert + 4.5, bulletZ + 1);
	enemySub.min.Set(xSub2-3+t1, t2 - 3, z2 - 3+t3);
	enemySub.max.Set(xSub2+3+t1, t2 + 5, z2 + 3+t3);

	//printf("X: %f %f %f\nY: %f %f %f\nZ: %f %f %f\n\n", missle.min.x, enemySub.min.x, enemySub.max.x, missle.min.y, enemySub.min.y, enemySub.max.y, missle.min.z, enemySub.min.z, enemySub.max.z);

	if (checkBullet == true) {
		if ((missle.min.x >= enemySub.min.x && missle.min.x <= enemySub.max.x)) {
			if ((missle.min.y >= enemySub.min.y && missle.min.y <= enemySub.max.y)) {
				if ((missle.min.z >= enemySub.min.z && missle.min.z <= enemySub.max.z)) {
					hit = true;
				}
			}
		}
	}

  glPushMatrix(); //apply all
  glTranslatef(xSub, moveSubVert, z);
  if (xSub <= BBox.min.x) { spin = 0; spinSpeed = 0; subSpeed = 0; xSub = BBox.min.x + 2.0; }
  if (xSub > BBox.max.x) { spin = 0; spinSpeed = 0; subSpeed = 0; xSub = BBox.max.x - 2.0; }
  if (z <= BBox.min.z) { spin = 0; spinSpeed = 0; subSpeed = 0; z = BBox.min.z + 2.0; }
  if (z > BBox.max.z) { spin = 0; spinSpeed = 0; subSpeed = 0; z = BBox.max.z - 2.0; }
  if (moveSubVert <= BBox.min.y) { spin = 0; spinSpeed = 0; subSpeed = 0; moveSubVert = BBox.min.y + 0.5; }
  if (moveSubVert > BBox.max.y) { spin = 0; spinSpeed = 0; subSpeed = 0; moveSubVert = BBox.max.y - 0.5; }
  glRotatef(theta, 0.0, 1.0, 0.0);

  glBindTexture(GL_TEXTURE_2D, 2000);
  glPushMatrix(); //body
  glTranslatef(0.0, 4.0, 0.0);
  glScalef(3.0, 1.0, 1.0);
  gluQuadricDrawStyle(mySphere, GLU_FILL);
  gluQuadricTexture(mySphere, GLU_TRUE);
  gluQuadricNormals(mySphere, GLU_SMOOTH);
  gluSphere(mySphere, 1.0, 20, 20);
  glPopMatrix(); //end body

  glPushMatrix(); //top
  glTranslatef(0.0, 5.0, 0.0);
  glScalef(2.0, 1.0, 0.25);

  glBindTexture(GL_TEXTURE_2D, 2000);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // right face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // left face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // bottom face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // back face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // front face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glEnd();

  glPopMatrix(); //end top trans


  glBindTexture(GL_TEXTURE_2D, 2000); // top face of cube
  glPushMatrix(); //apply propellers
  glTranslatef(3.3, 4.0, 0);
  glRotatef(spin, 1.0, 0.0, 0.0);
  glScalef(0.5, 0.50, 0.5);
  glTranslatef(-6.3, -4.0, 0);

  glPushMatrix(); //first propeller
  glTranslatef(5.3, 3.5, 0.0);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // right face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // left face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // bottom face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // back face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // front face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glEnd();

  glPopMatrix(); //end first trans

  glPushMatrix(); //second propeller
  glTranslatef(5.3, 4.0, -0.3);
  glRotatef(120.0, 1.0, 0.0, 0.0);

  glBindTexture(GL_TEXTURE_2D, 2000);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // right face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // left face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // bottom face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // back face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // front face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glEnd();

  glPopMatrix(); //end second trans

  glPushMatrix(); //third propeller
  glTranslatef(5.3, 4.0, 0.3);
  glRotatef(240.0, 1.0, 0.0, 0.0);

  glBindTexture(GL_TEXTURE_2D, 2000);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // right face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // left face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // bottom face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // back face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 2000); // front face of cube
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glNormal3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0, 1.0);
  glNormal3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 1.0);
  glNormal3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0, 0.0);
  glNormal3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glEnd();

  glPopMatrix(); //end third trans

  glPopMatrix(); //apply propellers

  glFlush();
  glPopMatrix(); //apply all

  if (hit == false) {
	  makeEnemySub();
  }

  // Draw ground
  glBindTexture(GL_TEXTURE_2D, 2000);
  groundMesh->DrawMesh(meshSize);

  glDisable(GL_TEXTURE_2D);

  glutSwapBuffers();
}


// Called at initialization and whenever user resizes the window */
void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0,1.0,0.2,40.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glutSetWindow(main_window);
  glutPostWindowRedisplay(main_window);
}

VECTOR3D pos = VECTOR3D(0,0,0);

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
  currentButton = button;

  switch(button)
  {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_UP)
	{

	}
	else if (state == GLUT_DOWN) //drag direction
	{
		prevX = x;
		prevY = y;
	}
	break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN)
	{

	}
	break;
  case 3: //scroll in
	  break;
  case 4: //scroll out
	  break;
  default:
	break;
  }
  glutPostWindowRedisplay(main_window);
}

// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{

  glutPostWindowRedisplay(main_window);
}


VECTOR3D ScreenToWorld(int x, int y)
{
	// you will need this if you use the mouse
	return VECTOR3D(0);
}// ScreenToWorld()

/* Handles input from the keyboard, non-arrow keys */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		subSpeed += 0.02;
		if (spinSpeed < 3.0) spinSpeed += 0.5;
		else spinSpeed = 2.5;
		break;
	case 'w':
		if (subSpeed > 0) {
			spinSpeed -= 0.5;
			subSpeed -= 0.02;
		}
		break;
	case 'e':
		bulletY = moveSubVert + 4;
		bulletDirection = theta;
		checkBullet = true;
		break;
	case 'r':
		if (yCam >= -20 && checkPeri == true) {
			thetay = -2;
			yCam += thetay;
		}
		break;
	case 'f':
		if (yCam <= 20 && checkPeri == true) {
			thetay = 2;
			yCam += thetay;
		}
		break;
	}

  //glutPostRedisplay();
}

void functionKeys(int key, int x, int y)
{
  VECTOR3D min, max;

  if (key == GLUT_KEY_F1)
  {
	  MessageBox(NULL, "Use q to accelerate \nUse w to brake \nUse e to shoot a torpedo \nUse F2 for periscope view \nUse Up arrow key to ascend \nUse Down arrow key to descend \nUse Left arrow key to turn left \nUse Right arrow key to turn right", "Help", NULL);
  }
  else if (key == GLUT_KEY_F2)
  {
	  if (checkPeri == false)
		  checkPeri = true;
	  else if(checkPeri == true)
		  checkPeri = false;
  }
 
  if (key == GLUT_KEY_UP)
  {
	  moveSubVert += 0.05;
	  spin += 5.0;
  }
  else if (key == GLUT_KEY_DOWN)
  {
	  moveSubVert -= 0.05;
	  spin += 5.0;
  }
  else if (key == GLUT_KEY_LEFT)
  {
	  theta += 2.0;
	  spin += 5.0;
	  if (theta > 360.0)
		  theta -= 360.0;
  }
  else if (key == GLUT_KEY_RIGHT)
  {
	  theta -= 2.0;
	  spin += 5.0;
	  if (theta < 0.0)
		  theta += 360.0;
  }

  glutPostWindowRedisplay(main_window);
}

void spinAccelerate(void)
{
	spin += spinSpeed;
	if (spin > 360.0)
		spin -= 360.0;

	xSub -= subSpeed * cos(theta / 180.0*M_PI);
	z += subSpeed * sin(theta / 180.0*M_PI);

	//---------------------------- Auto enemy sub movement

	if (counter == 0) {
		randomMove = rand() % 2;
		if (randomMove == 0)
			angles = -90.0;
		else if (randomMove == 1)
			angles = 90.0;
	}

	counter += 1;
	if (counter >= 200) {
		if (randomMove == 0)
			theta2 -= 0.5;
		else if (randomMove == 1)
			theta2 += 0.5;
	}
	xSub2 -= 0.02 * cos(theta2 / 180.0*M_PI);
	z2 += 0.02 * sin(theta2 / 180.0*M_PI);

	if (enemySub.min.x <= -10 || enemySub.max.x >= 10)
		angles = 180.0;

	if (theta2 == angles ) {
		counter = 0;
		if (randomMove == 0)
			angles -= 90.0;
		else if (randomMove == 1)
			angles += 90.0;
	}

	if (theta2 == -360.0 || theta2 == 360.0) {
		theta2 = 0.0;
		if (randomMove == 0)
			angles -= 90.0;
		else if (randomMove == 1)
			angles += 90.0;
	}

	spin2 += 0.5;
	if (spin2 > 360.0)
		spin2 -= 360.0;

	//----bullet movement

	bulletCounter += 1;
	if (bulletX >= 10 || bulletX <= -10 || bulletZ >= 10 || bulletZ <= -10) {
		bulletX = 0;
		bulletZ = 0;
		checkBullet = false;
		if (bulletCounter >= 50 && hit == true) {
			hit = false;
			checkEnemySub = false;
		}
	}

	//----peri cam

	if (checkPeri == true) {
		xFrom = xSub - 2;
		yFrom = moveSubVert + 7;
		zFrom = z;
	}
	else if (checkPeri == false) {
		xFrom = 0;
		yFrom = 6;
		zFrom = 22;
	}

	glutPostWindowRedisplay(main_window);
}

void makeEnemySub()
{

	glPushMatrix();
	glTranslatef(xSub2, 0.0, z2);
	glTranslatef(t1, 0.0, t3);
	glRotatef(theta2, 0.0, 1.0, 0.0);
	glTranslatef(-t1, 0.0, -t3);
	if (checkEnemySub == false) {
		checkEnemySub = true;
		srand((unsigned)time(&t));
		t1 = rand() % 13 - 6;
		t2 = rand() % 6;
		t3 = rand() % 13 - 6;
	}
	glTranslatef(t1, t2, t3);



	glBindTexture(GL_TEXTURE_2D, 2001);
	glPushMatrix(); //body
	glTranslatef(0.0, 4.0, 0.0);
	glScalef(3.0, 1.0, 1.0);
	gluQuadricDrawStyle(mySphere2, GLU_FILL);
	gluQuadricTexture(mySphere2, GLU_TRUE);
	gluQuadricNormals(mySphere2, GLU_SMOOTH);
	gluSphere(mySphere2, 1.0, 20, 20);
	glPopMatrix(); //end body

	glPushMatrix(); //top
	glTranslatef(0.0, 5.0, 0.0);
	glScalef(2.0, 1.0, 0.25);

	glBindTexture(GL_TEXTURE_2D, 2001);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // right face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // left face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // bottom face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // back face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // front face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glPopMatrix(); //end top trans


	glBindTexture(GL_TEXTURE_2D, 2001); // top face of cube
	glPushMatrix(); //apply propellers
	glTranslatef(3.3, 4.0, 0);
	glRotatef(spin2, 1.0, 0.0, 0.0); 
	glScalef(0.5, 0.50, 0.5);
	glTranslatef(-6.3, -4.0, 0);

	glPushMatrix(); //first propeller
	glTranslatef(5.3, 3.5, 0.0);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // right face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // left face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // bottom face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // back face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // front face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glPopMatrix(); //end first trans

	glPushMatrix(); //second propeller
	glTranslatef(5.3, 4.0, -0.3);
	glRotatef(120.0, 1.0, 0.0, 0.0);

	glBindTexture(GL_TEXTURE_2D, 2001);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // right face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // left face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // bottom face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // back face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // front face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glPopMatrix(); //end second trans

	glPushMatrix(); //third propeller
	glTranslatef(5.3, 4.0, 0.3);
	glRotatef(240.0, 1.0, 0.0, 0.0);

	glBindTexture(GL_TEXTURE_2D, 2001);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // right face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // left face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // bottom face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // back face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2001); // front face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glPopMatrix(); //end third trans

	glPopMatrix(); //apply propellers

	glPopMatrix(); //apply all
}