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
ugSolidTorusf(GLfloat ir, GLfloat or, GLint sides, GLint rings) {
    int i, j, k;
    GLfloat s, t, x, y, z, twopi, nx, ny, nz;
    static GLfloat* v, *n;
    static GLfloat parms[4];
    GLfloat* p, *q;

    if (v) {
	if (parms[0] != ir || parms[1] != or || parms[2] != sides || parms[3] != rings) {
	    free(v); free(n);
	    n = v = 0;
	    glVertexPointer(3, GL_FLOAT, 0, 0);
	    glNormalPointer(GL_FLOAT, 0, 0);
	}
    }
    if (!v) {
	parms[0] = ir; parms[1] = or; parms[2] = sides; parms[3] = rings;

	p = v = malloc(sides*(rings+1)*2*3*sizeof *v);
	q = n = malloc(sides*(rings+1)*2*3*sizeof *n);
        twopi = 2 * PI_;
        for (i = 0; i < sides; i++) {
	    for (j = 0; j <= rings; j++) {
		for (k = 1; k >= 0; k--) {
		    s = (i + k) % sides + 0.5;
		    t = j % rings;

		    x = (or+ir*cos(s*twopi/sides))*cos(t*twopi/rings);
		    y = (or+ir*cos(s*twopi/sides))*sin(t*twopi/rings);
		    z = ir * sin(s * twopi / sides);
		    *p++ = x;
		    *p++ = y;
		    *p++ = z;
		    nx = cos(s*twopi/sides)*cos(t*twopi/rings);
		    ny = cos(s*twopi/sides)*sin(t*twopi/rings);
		    nz = sin(s * twopi / sides);
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
    for(i = 0; i < sides; i++)
	glDrawArrays(GL_TRIANGLE_STRIP, (rings+1)*2*i, (rings+1)*2);
}

void APIENTRY
ugSolidSpheref(GLfloat radius, GLint slices, GLint stacks) {
    int i, j; 
    GLfloat twopi, nx, ny, nz;
    static GLfloat* v, *n;
    static GLfloat parms[3];
    GLfloat* p, *q;

    if (v) {
	if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) {
	    free(v); free(n);
	    n = v = 0;
	    glVertexPointer(3, GL_FLOAT, 0, 0);
	    glNormalPointer(GL_FLOAT, 0, 0);
	}
    }
    if (!v) {
	parms[0] = radius; parms[1] = slices; parms[2] = stacks;
	p = v = malloc(stacks*(slices+1)*2*3*sizeof *v);
	q = n = malloc(stacks*(slices+1)*2*3*sizeof *n);
        twopi = 2 * PI_;
        for (i = 0; i < stacks; i++) {
	    GLfloat phi = (GLfloat)i/stacks*twopi;
	    GLfloat phi1 = i == stacks-1 ? 0 : (i+1.0f)*twopi/stacks;
	    GLfloat cphi = cos(phi);
	    GLfloat sphi = sin(phi);
	    GLfloat cphi1 = cos(phi1);
	    GLfloat sphi1 = sin(phi1);
	    for (j = 0; j <= slices; j++) {
		GLfloat theta = j == slices ? 0.f : (GLfloat)j/slices*twopi;
		GLfloat ctheta = cos(theta);
		GLfloat stheta = sin(theta);
		nx = cphi*ctheta;
		ny = sphi*ctheta;
		nz = stheta;

		*p++ = radius*nx;
		*p++ = radius*ny;
		*p++ = radius*nz;
		*q++ = nx;
		*q++ = ny;
		*q++ = nz;

		nx = cphi1*ctheta;
		ny = sphi1*ctheta;
		nz = stheta;

		*p++ = radius*nx;
		*p++ = radius*ny;
		*p++ = radius*nz;
		*q++ = nx;
		*q++ = ny;
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
}

void APIENTRY
ugWireSpheref(GLfloat radius, GLint slices, GLint stacks) {
    int i, j; 
    GLfloat twopi, nx, ny, nz;
    static GLfloat* v, *n;
    static GLfloat parms[3];
    GLfloat* p, *q;

    if (v) {
	if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) {
	    free(v); free(n);
	    n = v = 0;
	    glVertexPointer(3, GL_FLOAT, 0, 0);
	    glNormalPointer(GL_FLOAT, 0, 0);
	}
    }
    if (!v) {
	parms[0] = radius; parms[1] = slices; parms[2] = stacks;
	p = v = malloc(stacks*(slices+1)*2*3*sizeof *v);
	q = n = malloc(stacks*(slices+1)*2*3*sizeof *n);
        twopi = 2 * PI_;
        for (i = 0; i < stacks; i++) {
	    GLfloat phi = (GLfloat)i/stacks*twopi;
	    GLfloat phi1 = i == stacks-1 ? 0 : (i+1.0f)*twopi/stacks;
	    GLfloat cphi = cos(phi);
	    GLfloat sphi = sin(phi);
	    GLfloat cphi1 = cos(phi1);
	    GLfloat sphi1 = sin(phi1);
	    for (j = 0; j <= slices; j++) {
		GLfloat theta = j == slices ? 0.f : (GLfloat)j/slices*twopi;
		GLfloat ctheta = cos(theta);
		GLfloat stheta = sin(theta);
		nx = cphi*ctheta;
		ny = sphi*ctheta;
		nz = stheta;

		*p++ = radius*nx;
		*p++ = radius*ny;
		*p++ = radius*nz;
		*q++ = nx;
		*q++ = ny;
		*q++ = nz;

		nx = cphi1*ctheta;
		ny = sphi1*ctheta;
		nz = stheta;

		*p++ = radius*nx;
		*p++ = radius*ny;
		*p++ = radius*nz;
		*q++ = nx;
		*q++ = ny;
		*q++ = nz;
	    }
	}
    }
    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);
    for(i = 0; i < stacks; i++)
	glDrawArrays(GL_LINE_STRIP, i*(slices+1)*2, (slices+1)*2);
}

void APIENTRY
ugSolidConef(GLfloat base, GLfloat height, GLint slices, GLint stacks) {
    int i, j;
    GLfloat twopi, nx, ny, nz;
    static GLfloat* v, *n;
    static GLfloat parms[4];
    GLfloat* p, *q;

    if (v) {
	if (parms[0] != base || parms[1] != height || parms[2] != slices || parms[3] != stacks) {
	    free(v); free(n);
	    n = v = 0;
	    glVertexPointer(3, GL_FLOAT, 0, 0);
	    glNormalPointer(GL_FLOAT, 0, 0);
	}
    }
    if (!v) {
	GLfloat phi = base/height, cphi = cos(phi), sphi= sin(phi);
	parms[0] = base; parms[1] = height; parms[2] = slices; parms[3] = stacks;
	p = v = malloc(stacks*(slices+1)*2*3*sizeof *v);
	q = n = malloc(stacks*(slices+1)*2*3*sizeof *n);
        twopi = 2 * PI_;
        for (i = 0; i < stacks; i++) {
	    GLfloat r = base*(1.0f - (GLfloat)i/stacks);
	    GLfloat r1 = base*(1.0f - (GLfloat)(i+1.0)/stacks);
	    GLfloat z = height*i/stacks;
	    GLfloat z1 = height*(1.0+i)/stacks;
	    for (j = 0; j <= slices; j++) {
		GLfloat theta = j == slices ? 0.f : (GLfloat)j/slices*twopi;
		GLfloat ctheta = cos(theta);
		GLfloat stheta = sin(theta);
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
}

void APIENTRY
ugSolidCubef(GLfloat size) {
    static GLfloat* v, *n;
    static const GLfloat cubev[3*12*3] = {
	-1., -1., 1.,	/* front */
	-1.,  1., 1.,
	 1., -1., 1.,

	 1., -1., 1.,
	-1.,  1., 1.,
	 1.,  1., 1.,

	-1.,  1., -1.,	/* back */
	-1., -1., -1.,
	 1., -1., -1.,

	-1.,  1., -1.,
	 1., -1., -1.,
	 1.,  1., -1.,

	-1., -1., -1.,	/* left */
	-1.,  1., -1.,
	-1., -1.,  1.,

	-1., -1.,  1.,
	-1.,  1., -1.,
	-1.,  1.,  1.,

	 1., -1.,  1.,	/* right */
	 1.,  1.,  1.,
	 1., -1., -1.,

	 1., -1., -1.,
	 1.,  1.,  1.,
	 1.,  1., -1.,

	-1.,  1.,  1.,	/* top */
	-1.,  1., -1.,
	 1.,  1.,  1.,

	 1.,  1.,  1.,
	-1.,  1., -1.,
	 1.,  1., -1.,

	-1., -1., -1.,	/* bottom */
	-1., -1.,  1.,
	 1., -1., -1.,

	 1., -1., -1.,
	-1., -1.,  1.,
	 1., -1.,  1.,
    };

    static const GLfloat cuben[3*12*3] = {
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
    v = malloc(sizeof cubev); memcpy(v, cubev, sizeof cubev);
    n = malloc(sizeof cuben); memcpy(n, cuben, sizeof cuben);
    for(i = 0; i < sizeof cubev/sizeof cubev[0]; i++) {
	v[i] *= size;
    }
    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, 12*3);
}

void APIENTRY
ugWireCubef(GLfloat size) {
    static GLfloat* v, *n;
    static const GLfloat cubev[3*6*4] = {
	-1., -1., 1.,	/* front */
	-1.,  1., 1.,
	 1.,  1., 1.,
	 1., -1., 1.,

	-1.,  1., -1.,	/* back */
	-1., -1., -1.,
	 1., -1., -1.,
	 1.,  1., -1.,

	-1., -1., -1.,	/* left */
	-1.,  1., -1.,
	-1.,  1.,  1.,
	-1., -1.,  1.,

	 1., -1.,  1.,	/* right */
	 1.,  1.,  1.,
	 1.,  1., -1.,
	 1., -1., -1.,

	-1.,  1.,  1.,	/* top */
	-1.,  1., -1.,
	 1.,  1., -1.,
	 1.,  1.,  1.,

	-1., -1., -1.,	/* bottom */
	-1., -1.,  1.,
	 1., -1.,  1.,
	 1., -1., -1.,
    };

    static const GLfloat cuben[3*6*4] = {
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
    v = malloc(sizeof cubev); memcpy(v, cubev, sizeof cubev);
    n = malloc(sizeof cuben); memcpy(n, cuben, sizeof cuben);
    for(i = 0; i < sizeof cubev/sizeof cubev[0]; i++) {
	v[i] *= size;
    }
    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_FLOAT, 0, n);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);
    for(i = 0; i < 6; i++)
	glDrawArrays(GL_LINE_LOOP, 4*i, 4);
}
