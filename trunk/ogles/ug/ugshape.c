/*
 * GLESonGL implementation
 * Version:  1.0
 *
 * Copyright (C) 2003  David Blythe   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID BLYTHE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "ug.h"
#include <math.h>

#define PI_ 3.14159265358979323846

void APIENTRY
ugSolidTorusf(GLfloat ir, GLfloat or, GLint sides, GLint rings) 
{
    GLint i, j, k, triangles;
    GLfloat s, t, x, y, z, twopi, nx, ny, nz;
	GLfloat sin_s, cos_s, cos_t, sin_t, twopi_s, twopi_t;
	GLfloat twopi_sides, twopi_rings;
    static GLfloat* v, *n;
    static GLfloat parms[4];
    GLfloat* p, *q;

    if (v) 
	{
		if (parms[0] != ir || parms[1] != or || parms[2] != sides || parms[3] != rings) 
		{
			free(v);
			free(n);
			n = v = 0;

			glVertexPointer(3, GL_FLOAT, 0, 0);
			glNormalPointer(GL_FLOAT, 0, 0);
		}
    }

    if (!v) 
	{
		parms[0] = ir; 
		parms[1] = or; 
		parms[2] = (GLfloat)sides; 
		parms[3] = (GLfloat)rings;

		p = v = malloc(sides*(rings+1)*2*3*sizeof *v);
		q = n = malloc(sides*(rings+1)*2*3*sizeof *n);

        twopi = 2.0f * (GLfloat)PI_;
		twopi_sides = twopi/sides;
		twopi_rings = twopi/rings;

        for (i = 0; i < sides; i++) 
		{
			for (j = 0; j <= rings; j++) 
			{
				for (k = 1; k >= 0; k--) 
				{
					s = (i + k) % sides + 0.5f;
					t = (GLfloat)( j % rings);

					twopi_s= s*twopi_sides;
					twopi_t = t*twopi_rings;

					cos_s = (GLfloat)cos(twopi_s);
					sin_s = (GLfloat)sin(twopi_s);

					cos_t = (GLfloat)cos(twopi_t);
					sin_t = (GLfloat)sin(twopi_t);

					x = (or+ir*(GLfloat)cos_s)*(GLfloat)cos_t;
					y = (or+ir*(GLfloat)cos_s)*(GLfloat)sin_t;
					z = ir * (GLfloat)sin_s;

					*p++ = x;
					*p++ = y;
					*p++ = z;

					nx = (GLfloat)cos_s*(GLfloat)cos_t;
					ny = (GLfloat)cos_s*(GLfloat)sin_t;
					nz = (GLfloat)sin_s;

					*q++ = nx;
					*q++ = ny;
					*q++ = nz;
				}
			}
		}
    }

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

	triangles = (rings + 1) * 2;

    for(i = 0; i < sides; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, triangles * i, triangles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}



void PlotSpherePoints(GLfloat radius, GLint stacks, GLint slices, GLfloat* v, GLfloat* n)
{

    GLint i, j; 
	GLfloat slicestep, stackstep;

	stackstep = ((GLfloat)PI_) / stacks;
	slicestep = 2.0f * ((GLfloat)PI_) / slices;

	for (i = 0; i < stacks; ++i)		
	{
		GLfloat a = i * stackstep;
		GLfloat b = a + stackstep;

		GLfloat s0 =  (GLfloat)sin(a);
		GLfloat s1 =  (GLfloat)sin(b);

		GLfloat c0 =  (GLfloat)cos(a);
		GLfloat c1 =  (GLfloat)cos(b);

		for (j = 0; j <= slices; ++j)		
		{
			GLfloat c = j * slicestep;
			GLfloat x = (GLfloat)cos(c);
			GLfloat y = (GLfloat)sin(c);

			*n = x * s0;
			*v = *n * radius;

			n++;
			v++;

			*n = y * s0;
			*v = *n * radius;

			n++;
			v++;

			*n = c0;
			*v = *n * radius;

			n++;
			v++;

			*n = x * s1;
			*v = *n * radius;

			n++;
			v++;

			*n = y * s1;
			*v = *n * radius;

			n++;
			v++;

			*n = c1;
			*v = *n * radius;

			n++;
			v++;

		}
	}
}


void APIENTRY
ugSolidSpheref(GLfloat radius, GLint slices, GLint stacks) 
{
    GLint i, triangles; 
    static GLfloat* v, *n;
    static GLfloat parms[3];

    if (v) 
	{
		if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FLOAT, 0, 0);
			glNormalPointer(GL_FLOAT, 0, 0);
		}
    }

    if (!v) 
	{
		parms[0] = radius; 
		parms[1] = (GLfloat)slices; 
		parms[2] = (GLfloat)stacks;

		v = malloc(stacks*(slices+1)*2*3*sizeof *v);
		n = malloc(stacks*(slices+1)*2*3*sizeof *n);

		PlotSpherePoints(radius, stacks, slices, v, n);

	}

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

	triangles = (slices + 1) * 2;

    for(i = 0; i < stacks; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i * triangles, triangles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}


void APIENTRY
ugWireSpheref(GLfloat radius, GLint slices, GLint stacks) 
{
    GLint i, j, f; 
    static GLfloat* v, *n;
    static GLfloat parms[3];

    if (v) 
	{
		if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FLOAT, 0, 0);
			glNormalPointer(GL_FLOAT, 0, 0);
		}
    }

    if (!v) 
	{
		parms[0] = radius; 
		parms[1] = (GLfloat)slices; 
		parms[2] = (GLfloat)stacks;

		v = malloc(stacks*(slices+1)*2*3*sizeof *v);
		n = malloc(stacks*(slices+1)*2*3*sizeof *n);

		PlotSpherePoints(radius, stacks, slices, v, n);

	}

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < stacks; ++i)
	{
		f = i * (slices + 1);

		for (j = 0; j <= slices; ++j)
			glDrawArrays(GL_LINE_LOOP, (f + j)*2, 3);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}



void APIENTRY
ugSolidConef(GLfloat base, GLfloat height, GLint slices, GLint stacks) 
{
    GLint i, j;
    GLfloat twopi, nx, ny, nz;
    static GLfloat* v, *n;
    static GLfloat parms[4];
    GLfloat* p, *q;

    if (v) 
	{
		if (parms[0] != base || parms[1] != height || parms[2] != slices || parms[3] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;

			glVertexPointer(3, GL_FLOAT, 0, 0);
			glNormalPointer(GL_FLOAT, 0, 0);
		}
    }

    if (!v) 
	{
		GLfloat phi = base/height, cphi = (GLfloat)cos(phi), sphi= (GLfloat)sin(phi);

		parms[0] = base; 
		parms[1] = height; 
		parms[2] = (GLfloat)slices;
		parms[3] = (GLfloat)stacks;

		p = v = malloc(stacks*(slices+1)*2*3*sizeof *v);
		q = n = malloc(stacks*(slices+1)*2*3*sizeof *n);

        twopi = 2.0f * (float)PI_;

        for (i = 0; i < stacks; i++) 
		{
			GLfloat r = base*(1.0f - (GLfloat)i/stacks);
			GLfloat r1 = base*(1.0f - (GLfloat)(i+1.0)/stacks);
			GLfloat z = height*i/stacks;
			GLfloat z1 = height*(1.0f+i)/stacks;

			for (j = 0; j <= slices; j++) 
			{
				GLfloat theta = j == slices ? 0.f : (GLfloat)j/slices*twopi;
				GLfloat ctheta = (GLfloat)cos(theta);
				GLfloat stheta = (GLfloat)sin(theta);

				nx = ctheta;
				ny = stheta;
				nz = sphi;

				*p++ = r*nx;
				*p++ = r*ny;
				*p++ = z;

				*q++ = nx*cphi;
				*q++ = ny*cphi;
				*q++ = nz;

				*p++ = r1*nx;
				*p++ = r1*ny;
				*p++ = z1;

				*q++ = nx*cphi;
				*q++ = ny*cphi;
				*q++ = nz;
			}
		}
    }

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < stacks; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i*(slices+1)*2, (slices+1)*2);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}

void APIENTRY
ugSolidCubef(GLfloat size) 
{
    static GLfloat v[108];	   // 108 =  3*12*3

    static const GLfloat cubev[108] = 
	{
		-1., -1., 1.,	/* front */
		 1., -1., 1.,
		-1.,  1., 1.,

		 1., -1., 1.,
		 1.,  1., 1.,
		-1.,  1., 1.,

		-1.,  1., -1.,	/* back */
		 1., -1., -1.,
		-1., -1., -1.,

		-1.,  1., -1.,
		 1.,  1., -1.,
		 1., -1., -1.,

		-1., -1., -1.,	/* left */
		-1., -1.,  1.,
		-1.,  1., -1.,

		-1., -1.,  1.,
		-1.,  1.,  1.,
		-1.,  1., -1.,

		 1., -1.,  1.,	/* right */
		 1., -1., -1.,
		 1.,  1.,  1.,

		 1., -1., -1.,
		 1.,  1., -1.,
		 1.,  1.,  1.,

		-1.,  1.,  1.,	/* top */
		 1.,  1.,  1.,
		-1.,  1., -1.,

		 1.,  1.,  1.,
		 1.,  1., -1.,
		-1.,  1., -1.,

		-1., -1., -1.,	/* bottom */
		 1., -1., -1.,
		-1., -1.,  1.,

		 1., -1., -1.,
		 1., -1.,  1.,
		-1., -1.,  1.,
    };

    static const GLfloat cuben[108] = 
	{
		0., 0., 1.,	/* front */
		0., 0., 1.,
		0., 0., 1.,

		0., 0., 1.,
		0., 0., 1.,
		0., 0., 1.,

		0., 0., -1.,	/* back */
		0., 0., -1.,
		0., 0., -1.,

		0., 0., -1.,
		0., 0., -1.,
		0., 0., -1.,

		-1., 0., 0.,	/* left */
		-1., 0., 0.,
		-1., 0., 0.,

		-1., 0., 0.,
		-1., 0., 0.,
		-1., 0., 0.,

		1., 0., 0.,	/* right */
		1., 0., 0.,
		1., 0., 0.,

		1., 0., 0.,
		1., 0., 0.,
		1., 0., 0.,

		0., 1., 0.,	/* top */
		0., 1., 0.,
		0., 1., 0.,

		0., 1., 0.,
		0., 1., 0.,
		0., 1., 0.,

		0., -1., 0.,	/* bottom */
		0., -1., 0.,
		0., -1., 0.,

		0., -1., 0.,
		0., -1., 0.,
		0., -1., 0.,
    };

    int i;
	size /= 2;

    for(i = 0; i < 108; i++) 
		v[i] = cubev[i] * size;

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, cuben);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}

void APIENTRY
ugWireCubef(GLfloat size) 
{
    static GLfloat v[72];

    static const GLfloat cubev[72] = 	  // 72 = 3*6*4
	{
		-1., -1., 1.,	/* front */
		 1., -1., 1.,
		 1.,  1., 1.,
		-1.,  1., 1.,

		-1.,  1., -1.,	/* back */
		 1.,  1., -1.,
		 1., -1., -1.,
		-1., -1., -1.,

		-1., -1., -1.,	/* left */
		-1., -1.,  1.,
		-1.,  1.,  1.,
		-1.,  1., -1.,

		 1., -1.,  1.,	/* right */
		 1., -1., -1.,
		 1.,  1., -1.,
		 1.,  1.,  1.,

		-1.,  1.,  1.,	/* top */
		 1.,  1.,  1.,
		 1.,  1., -1.,
		-1.,  1., -1.,

		-1., -1., -1.,	/* bottom */
		 1., -1., -1.,
		 1., -1.,  1.,
		-1., -1.,  1.,
    };

    static const GLfloat cuben[72] = 
	{
		0., 0., 1.,	/* front */
		0., 0., 1.,
		0., 0., 1.,
		0., 0., 1.,

		0., 0., -1.,	/* back */
		0., 0., -1.,
		0., 0., -1.,
		0., 0., -1.,

		-1., 0., 0.,	/* left */
		-1., 0., 0.,
		-1., 0., 0.,
		-1., 0., 0.,

		1., 0., 0.,	/* right */
		1., 0., 0.,
		1., 0., 0.,
		1., 0., 0.,

		0., 1., 0.,	/* top */
		0., 1., 0.,
		0., 1., 0.,
		0., 1., 0.,

		0., -1., 0.,	/* bottom */
		0., -1., 0.,
		0., -1., 0.,
		0., -1., 0.,
    };

    int i;
	size /= 2;

    for(i = 0; i < 72; i++) 
		v[i] = cubev[i] * size;

    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, cuben);

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    for(i = 0; i < 6; i++)
		glDrawArrays(GL_LINE_LOOP, 4*i, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}
