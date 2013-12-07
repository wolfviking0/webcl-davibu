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

/*
 * Based on smallpt, a Path Tracer by Kevin Beason, 2008
 * Modified by David Bucciarelli to show the output via OpenGL/GLUT, ported
 * to C, work with float, fixed RR, ported to OpenCL, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <GL/glut.h>

#include "displayfunc.h"

void FreeBuffers() {
	free(pixels);
}

void AllocateBuffers() {
	const int pixelCount = width * height;
	pixels = malloc(sizeof(int[pixelCount / 4 + 1]));
	memset(pixels, 255, pixelCount);
}

void UpdateMandel() {
	const double startTime = WallClockTime();

	const int maxSize = width > height ? width : height;
	int s;
	for (s = 0; s < width * height / 4; ++s) {
		int t;
		for (t = 0; t < 4; ++t) {
			const int tid = s * 4 + t;

			const int i = tid % width;
			const int j = tid / width;

			const float x0 = ((i * scale) - ((scale / 2) * width)) / maxSize + offsetX;
			const float y0 = ((j * scale) - ((scale / 2) * height)) / maxSize + offsetY;

			float x = x0;
			float y = y0;

			float x2 = x * x;
			float y2 = y * y;

			uint iter = 0;
			for (iter = 0; (x2 + y2 <= 4.f) && (iter < maxIterations); ++iter) {
				y = 2 * x * y + y0;
				x = x2 - y2 + x0;

				x2 = x * x;
				y2 = y * y;
			}

			unsigned char *p = (unsigned char *)pixels;
			if (iter == maxIterations)
				p[tid] = 0;
			else
				p[tid] = (unsigned char)(255.f * (iter / (float)maxIterations) + 0.5f);
		}
	}

	const double elapsedTime = WallClockTime() - startTime;
	const double sampleSec = height * width / elapsedTime;
	sprintf(captionBuffer, "Rendering time: %.3f secs (Sample/sec %.1fK Max. Iterations %d)",
		elapsedTime, sampleSec / 1000.f, maxIterations);
}

int main(int argc, char *argv[]) {
	amiMandelCPU = 1;

	fprintf(stderr, "Usage: %s\n", argv[0]);
	fprintf(stderr, "Usage: %s <window width> <window height> <max. iterations>\n", argv[0]);

	if (argc == 4) {
		width = atoi(argv[4]);
		height = atoi(argv[5]);
		maxIterations = atoi(argv[1]);
	} else if (argc != 1)
		exit(-1);

	AllocateBuffers();
	UpdateMandel();

	//--------------------------------------------------------------------------

	InitGlut(argc, argv, "MandelCPU V1.3 (Written by David Bucciarelli)");

	glutMainLoop();

	//--------------------------------------------------------------------------

	return 0;
}
