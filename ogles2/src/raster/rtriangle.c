/*
** ==========================================================================
**
** rtriangle.c		Triangle Rasterization
**
** --------------------------------------------------------------------------
**
** 01-27-2006		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"
#include "raster/raster.h"
#include "raster/rutil.h"
#include "fragment/fragment.h"

static GLES_INLINE GLint min(GLint a, GLint b, GLint c) {
	GLint d = a < b ? a : b;
	return d < c ? d : c;
}

static GLES_INLINE GLint max(GLint a, GLint b, GLint c) {
	GLint d = a > b ? a : b;
	return d > c ? d : c;
}

static GLES_INLINE GLfloat det2x2(GLfloat a11, GLfloat a12, GLfloat a21, GLfloat a22) {
	return a11 * a22 - a12 * a21;
}

static void calculateVarying(GLfloat * varying, const GLfloat * base, const GLfloat * dx, const GLfloat * dy, GLint x, GLint y, GLfloat w) {
	GLint index;

	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
		varying[index] = (base[index] + dx[index] * x + dy[index] * y) * w;
	}
}

typedef struct {
	GLES_gpumem_t	colorBuffer;		/* color buffer address	for pixels	*/
	GLES_gpumem_t	depthBuffer;		/* depth buffer address	for pixels	*/
	GLES_gpumem_t	stencilBuffer;		/* stencil buffer address for pixels*/

	const GLfloat * startValues;		/* start values for pixel attributes*/
	const GLfloat *	increments;			/* increments for pixel attributes	*/
} ScanlineInfo;

/*
** TODO items:
** - steping on pixel centers
** - hook up setup of interpolation parameters
** - hook up lazy setup of interpolation parameters; both based on block as well as based on depth test?
** - innermost block of 2-by-2 set can be used to generate derivates, and as starting point for AA
*/
void GlesRasterTriangle(State * state, const RasterVertex * p1, const RasterVertex * p2, const RasterVertex * p3) {

	ScanlineInfo info;					/* scan line information			*/

	// 28.4 fixed-point coordinates
	GLint Y1 = (GLint)(16.0f * p1->screen.y + GLES_HALF);
    GLint Y2 = (GLint)(16.0f * p2->screen.y + GLES_HALF);
    GLint Y3 = (GLint)(16.0f * p3->screen.y + GLES_HALF);

	GLint X1 = (GLint)(16.0f * p1->screen.x + GLES_HALF);
    GLint X2 = (GLint)(16.0f * p2->screen.x + GLES_HALF);
    GLint X3 = (GLint)(16.0f * p3->screen.x + GLES_HALF);

    // Deltas
    GLint DX12 = X1 - X2;
    GLint DX23 = X2 - X3;
    GLint DX31 = X3 - X1;

    GLint DY12 = Y1 - Y2;
    GLint DY23 = Y2 - Y3;
    GLint DY31 = Y3 - Y1;

    // Fixed-point deltas
    GLint FDX12 = DX12 << 4;
    GLint FDX23 = DX23 << 4;
    GLint FDX31 = DX31 << 4;

    GLint FDY12 = DY12 << 4;
    GLint FDY23 = DY23 << 4;
    GLint FDY31 = DY31 << 4;

    // Bounding rectangle; round lower bound down to block size
    GLint minx = ((min(X1, X2, X3) + 0xF) >> 4) & ~(GLES_RASTER_BLOCK_SIZE - 1);
    GLint miny = ((min(Y1, Y2, Y3) + 0xF) >> 4) & ~(GLES_RASTER_BLOCK_SIZE - 1);
    GLint maxx = (max(X1, X2, X3) + 0xF) >> 4;
    GLint maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;

    //(char*&)colorBuffer += miny * stride;

    // Half-edge constants
    GLint C1 = DY12 * X1 - DX12 * Y1;
    GLint C2 = DY23 * X2 - DX23 * Y2;
    GLint C3 = DY31 * X3 - DX31 * Y3;

	GLint x, y, ix, iy; // loop variables
	GLuint index;

	GLfloat dVaryingDx[GLES_MAX_VARYING_FLOATS];
	GLfloat dVaryingDy[GLES_MAX_VARYING_FLOATS];
	GLfloat varying[GLES_MAX_VARYING_FLOATS];
	GLfloat varyingStart[GLES_MAX_VARYING_FLOATS];
	GLfloat dWdX, dWdY, wStart;

	GLfloat area, invArea;

    // Correct for fill convention
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) {
		C1++;
	}

	if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) {
		C2++;
	}

	if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) {
		C3++;
	}

	// setup of interpolation of varying variables goes here

	area = 
		det2x2(p2->screen.x - p1->screen.x, p2->screen.y - p1->screen.y,
			   p3->screen.x - p1->screen.x, p3->screen.y - p1->screen.y);

	invArea = 1.0f / area;

	dWdX =
		det2x2(p2->screen.w - p1->screen.w, p2->screen.y - p1->screen.y,
			   p3->screen.w - p1->screen.w, p3->screen.y - p1->screen.y) * invArea;

	dWdY =
		det2x2(p2->screen.w - p1->screen.w, p2->screen.x - p1->screen.x,
			   p3->screen.w - p1->screen.w, p3->screen.x - p1->screen.x) * invArea;

	wStart = 
		p1->screen.w - (p1->screen.x - minx * 1.0f/16.0f) * dWdX
			         - (p1->screen.y - miny * 1.0f/16.0f) * dWdY;

	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
		GLfloat p1OverW = p1->varyingData[index] * p1->screen.w;
		GLfloat p2OverW = p2->varyingData[index] * p1->screen.w;
		GLfloat p3OverW = p3->varyingData[index] * p1->screen.w;

		dVaryingDx[index] = 
			det2x2(p2OverW - p1OverW, p2->screen.y - p1->screen.y,
				   p3OverW - p1OverW, p3->screen.y - p1->screen.y) * invArea;
		dVaryingDy[index] = 
			det2x2(p2OverW - p1OverW, p2->screen.x - p1->screen.x,
				   p3OverW - p1OverW, p3->screen.x - p1->screen.x) * invArea;

		varyingStart[index] = 
			p1OverW - (p1->screen.x - minx * 1.0f/16.0f) * dVaryingDx[index]
				    - (p1->screen.y - miny * 1.0f/16.0f) * dVaryingDy[index];
	}

	// TODO: have linear interpolaton per block
	// TODO: investiagte: two levels of block sizes

    // Loop through blocks
    for(y = miny; y < maxy; y += GLES_RASTER_BLOCK_SIZE) {
        for(x = minx; x < maxx; x += GLES_RASTER_BLOCK_SIZE) {
            // Corners of block
            GLint x0 = x << 4;
            GLint x1 = (x + GLES_RASTER_BLOCK_SIZE - 1) << 4;
            GLint y0 = y << 4;
            GLint y1 = (y + GLES_RASTER_BLOCK_SIZE - 1) << 4;

            // Evaluate half-space functions
            GLboolean a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
            GLboolean a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
            GLboolean a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
            GLboolean a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
            GLint a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
    
            GLboolean b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
            GLboolean b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
            GLboolean b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
            GLboolean b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
            GLint b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
    
            GLboolean c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
            GLboolean c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
            GLboolean c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
            GLboolean c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
            GLint c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

            // Skip block when outside an edge
			if (a == 0x0 || b == 0x0 || c == 0x0) {
				continue;
			}

            //unsigned int *buffer = colorBuffer;

			// Accept whole block when totally covered
            if (a == 0xF && b == 0xF && c == 0xF) {
                for (iy = 0; iy < GLES_RASTER_BLOCK_SIZE; iy++) {
                    for (ix = x; ix < x + GLES_RASTER_BLOCK_SIZE; ix++) {
						GLfloat w = 1.0f / (wStart + (ix - minx) * dWdX + (iy - miny) * dWdY);
						calculateVarying(varying, varyingStart, dVaryingDx, dVaryingDy, ix - minx, iy - miny, w);

                        //buffer[ix] = 0x00007F00;<< // Green
						/* TODO: fragment processing goes here */
                    }

                    //(char*&)buffer += stride;
                }
            }
            else {
				// Partially covered block
                GLint CY1 = C1 + DX12 * y0 - DY12 * x0;
                GLint CY2 = C2 + DX23 * y0 - DY23 * x0;
                GLint CY3 = C3 + DX31 * y0 - DY31 * x0;

                for (iy = y; iy < y + GLES_RASTER_BLOCK_SIZE; iy++) {
                    GLint CX1 = CY1;
                    GLint CX2 = CY2;
                    GLint CX3 = CY3;

                    for(ix = x; ix < x + GLES_RASTER_BLOCK_SIZE; ix++) {
                        if (CX1 > 0 && CX2 > 0 && CX3 > 0) {
							GLfloat w = 1.0f / (wStart + (ix - minx) * dWdX + (iy - miny) * dWdY);
							calculateVarying(varying, varyingStart, dVaryingDx, dVaryingDy, ix - minx, iy - miny, w);

							//buffer[ix] = 0x0000007F;<< // Blue
							/* TODO: fragment processing goes here */
                        }

                        CX1 -= FDY12;
                        CX2 -= FDY23;
                        CX3 -= FDY31;
                    }

                    CY1 += FDX12;
                    CY2 += FDX23;
                    CY3 += FDX31;

                    //(char*&)buffer += stride;
                }
            }
        }

        //(char*&)colorBuffer += GLES_RASTER_BLOCK_SIZE * stride;

    }
}
