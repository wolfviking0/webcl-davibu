/*
Copyright (c) 2009 David Bucciarelli (davibu@interfree.it)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)
#include <sys/time.h>
#elif defined (WIN32)
#include <windows.h>
#else
	Unsupported Platform !!!
#endif

#include "displayfunc.h"

extern void AllocateBuffers();
extern void FreeBuffers();
extern void UpdateMandel();

int amiMandelCPU;

int width = 512;
int height = 512;
float scale = 3.5f;
float offsetX = -0.5f;
float offsetY = 0.0f;
int maxIterations = 256;
char captionBuffer[256];

static int printHelp = 1;

unsigned int *pixels = NULL;

#ifdef __EMSCRIPTEN__
	#include <emscripten/emscripten.h>
	#define glRasterPos2i(x,y)
	#define glRecti(x,y,z,w)
	#undef GLUT_BITMAP_HELVETICA_18
	#define GLUT_BITMAP_HELVETICA_18 (void*)""
#endif

double WallClockTime() {
#if defined(__linux__) || defined(__APPLE__)
	struct timeval t;
	gettimeofday(&t, NULL);

	return t.tv_sec + t.tv_usec / 1000000.0;
#elif defined (WIN32)
	return GetTickCount() / 1000.0;
#elif defined (__EMSCRIPTEN__)
	return (emscripten_get_now() / 1000.0);	
#else
	Unsupported Platform !!!
#endif
}



unsigned int TextureId;
static unsigned int TextureTarget = GL_TEXTURE_2D;
static unsigned int TextureInternal             = GL_LUMINANCE;
static unsigned int TextureFormat               = GL_LUMINANCE;
static unsigned int TextureType                 = GL_UNSIGNED_BYTE;
static unsigned int ActiveTextureUnit           = GL_TEXTURE1_ARB;
static size_t TextureTypeSize                   = sizeof(float);

static float VertexPos[4][2]            = { { -1.0f, -1.0f },
                                            { +1.0f, -1.0f },
                                            { +1.0f, +1.0f },
                                            { -1.0f, +1.0f } };
static float TexCoords[4][2];

static void CreateTexture(unsigned int width, unsigned int height)
{    
    
    printf("Creating Texture 1 %d x %d...\n", width, height);

    glActiveTexture(ActiveTextureUnit);
    
    glGenTextures( 1, &TextureId );
	glBindTexture(TextureTarget, TextureId);

    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(TextureTarget, 0, TextureInternal, width, height, 0, TextureFormat, TextureType, 0);
    glBindTexture(TextureTarget, 0);

}

static int SetupGraphics(unsigned int width, unsigned int height)
{
    CreateTexture(width,height);

    glClearColor (0.0, 0.0, 0.0, 0.0);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glViewport(0, 0,width,height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    TexCoords[3][0] = 0.0f;
    TexCoords[3][1] = 0.0f;
    TexCoords[2][0] = width;
    TexCoords[2][1] = 0.0f;
    TexCoords[1][0] = width;
    TexCoords[1][1] = height;
    TexCoords[0][0] = 0.0f;
    TexCoords[0][1] = height;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, VertexPos);
    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);
    return GL_NO_ERROR;
}

static void PrintString(void *font, const char *string) {
	#ifdef __EMSCRIPTEN__
		printf("%s\n",string);
	#else
	int len, i;

	len = (int)strlen(string);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, string[i]);
	#endif
}

static void PrintHelp() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.f, 0.f, 0.5f, 0.5f);
	glRecti(40, 40, 600, 440);

	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(300, 420);
	PrintString(GLUT_BITMAP_HELVETICA_18, "Help");

	glRasterPos2i(60, 390);
	PrintString(GLUT_BITMAP_HELVETICA_18, "h - toggle Help");
	glRasterPos2i(60, 360);
	PrintString(GLUT_BITMAP_HELVETICA_18, "arrow Keys - move left/right/up/down");
	glRasterPos2i(60, 330);
	PrintString(GLUT_BITMAP_HELVETICA_18, "PageUp and PageDown - zoom in/out");
	glRasterPos2i(60, 300);
	PrintString(GLUT_BITMAP_HELVETICA_18, "Mouse button 0 + Mouse X, Y - move left/right/up/down");
	glRasterPos2i(60, 270);
	PrintString(GLUT_BITMAP_HELVETICA_18, "Mouse button 2 + Mouse X - zoom in/out");
	glRasterPos2i(60, 240);
	PrintString(GLUT_BITMAP_HELVETICA_18, "+ - increase the max. interations by 32");
	glRasterPos2i(60, 210);
	PrintString(GLUT_BITMAP_HELVETICA_18, "- - decrease the max. interations by 32");

	glDisable(GL_BLEND);
}

void displayFunc(void) {
	UpdateMandel();
	
	glClear(GL_COLOR_BUFFER_BIT);

	#ifndef __EMSCRIPTEN__
		glRasterPos2i(0, 0);
		glDrawPixels(width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
    #else
	
	 	glEnable( TextureTarget );
	    glBindTexture( TextureTarget, TextureId );

	    if(pixels) {
	        glTexSubImage2D(TextureTarget, 0, 0, 0,  width, height, TextureFormat, TextureType, pixels);
	    }

		glBegin(GL_TRIANGLE_STRIP);
		glColor3f( 1, 1, 1 );
	    glTexCoord2i( 0, 0 ); glVertex3f( 0, 0, 0 );
	    glColor3f( 1, 1, 1 );
	    glTexCoord2i( 0, 1 ); glVertex3f( 0, height, 0 );
	    glColor3f( 1, 1, 1 );
	    glTexCoord2i( 1, 0 ); glVertex3f( width, 0, 0 );
	    glColor3f( 1, 1, 1 );
	    glTexCoord2i( 1, 1 ); glVertex3f( width, height, 0 );
	    glEnd();

	    glDisable( TextureTarget );
	    glBindTexture( TextureTarget, 0 );
	    
	 #endif

	// Title
	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(4, height - 16);
	if (amiMandelCPU)
		PrintString(GLUT_BITMAP_HELVETICA_18, "MandelCPU V1.3 (Written by David Bucciarelli)");
	else
		PrintString(GLUT_BITMAP_HELVETICA_18, "MandelGPU V1.3 (Written by David Bucciarelli)");

	// Caption line 0
	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(4, 10);
	PrintString(GLUT_BITMAP_HELVETICA_18, captionBuffer);

	if (printHelp) {
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-0.5, 639.5, -0.5, 479.5, -1.0, 1.0);

		PrintHelp();

		glPopMatrix();
	}

	glutSwapBuffers();
}

void reshapeFunc(int newWidth, int newHeight) {
	// Width must be a multiple of 4
	width = newWidth;
	if (width % 4 != 0)
		width = (width / 4 + 1) * 4;
	height = newHeight;

	glViewport(0, 0, width, height);
	glLoadIdentity();
	glOrtho(-0.5f, width - 0.5f, -0.5f, height - 0.5f, -1.f, 1.f);


	FreeBuffers();
	AllocateBuffers();

	glutPostRedisplay();
}

void keyFunc(unsigned char key, int x, int y) {
	int needRedisplay = 1;

	switch (key) {
        case 's': {
			FILE *f = fopen("image.ppm", "w"); // Write image to PPM file.
			if (!f)
				fprintf(stderr, "Failed to open image file: image.ppm\n");
			else {
				fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);

				int x, y;
				for (y = height - 1; y >= 0; --y) {
					unsigned char *p = (unsigned char *)(&pixels[y * width / 4]);
					for (x = 0; x < width; ++x, p++) {
						fprintf(f, "%d %d %d ", *p, *p, *p);
					}
				}

				fclose(f);
			}
			needRedisplay = 0;
			break;
		}
		case 27: /* Escape key */
		case 'q':
        case 'Q':
			fprintf(stderr, "Done.\n");
			exit(0);
			break;
		case '+':
			maxIterations += 32;
			break;
		case '-':
			maxIterations -= 32;
			break;
		case ' ': /* Refresh display */
			break;
		case 'h':
			printHelp = (!printHelp);
			break;
		default:
			needRedisplay = 0;
			break;
	}

	if (needRedisplay) {
		UpdateMandel();
		displayFunc();
	}
}

#define SCALE_STEP (0.1f)
#define OFFSET_STEP (0.025f)
void specialFunc(int key, int x, int y) {
	int needRedisplay = 1;

	switch (key) {
		case GLUT_KEY_UP:
			offsetY += scale * OFFSET_STEP;
			break;
		case GLUT_KEY_DOWN:
			offsetY -= scale * OFFSET_STEP;
			break;
		case GLUT_KEY_LEFT:
			offsetX -= scale * OFFSET_STEP;
			break;
		case GLUT_KEY_RIGHT:
			offsetX += scale * OFFSET_STEP;
			break;
		case GLUT_KEY_PAGE_UP:
			scale *= 1.f - SCALE_STEP;
			break;
		case GLUT_KEY_PAGE_DOWN:
			scale *= 1.f + SCALE_STEP;
			break;
		default:
			needRedisplay = 0;
			break;
	}

	if (needRedisplay) {
		UpdateMandel();
		displayFunc();
	}
}

static int mouseButton0 = 0;
static int mouseButton2 = 0;
static int mouseGrabLastX = 0;
static int mouseGrabLastY = 0;

void mouseFunc(int button, int state, int x, int y) {
	if (button == 0) {
		if (state == GLUT_DOWN) {
			// Record start position
			mouseGrabLastX = x;
			mouseGrabLastY = y;
			mouseButton0 = 1;
		} else if (state == GLUT_UP) {
			mouseButton0 = 0;
		}
	} else if (button == 2) {
		if (state == GLUT_DOWN) {
			// Record start position
			mouseGrabLastX = x;
			mouseGrabLastY = y;
			mouseButton2 = 1;
		} else if (state == GLUT_UP) {
			mouseButton2 = 0;
		}
	}
}

void motionFunc(int x, int y) {
	int needRedisplay = 1;

	if (mouseButton0) {
		const int distX = x - mouseGrabLastX;
		const int distY = y - mouseGrabLastY;

		offsetX -= (40.f * distX / width) * scale * OFFSET_STEP;
		offsetY += (40.f * distY / height) * scale * OFFSET_STEP;

		mouseGrabLastX = x;
		mouseGrabLastY = y;
	} else if (mouseButton2) {
		const int distX = x - mouseGrabLastX;

		scale *= 1.0f - (2.f * distX / width);

		mouseGrabLastX = x;
		mouseGrabLastY = y;
	} else
		needRedisplay = 0;

	if (needRedisplay)
		glutPostRedisplay();
}

void InitGlut(int argc, char *argv[], char *windowTittle) {
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInit(&argc, argv);

	glutCreateWindow(windowTittle);

	SetupGraphics(width, height);

	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyFunc);
	glutSpecialFunc(specialFunc);
	glutDisplayFunc(displayFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);

	glMatrixMode(GL_PROJECTION);

	
}
