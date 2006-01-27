/*
** ==========================================================================
**
** triangles.c		Triangle rendering functions
**
** --------------------------------------------------------------------------
**
** 06-14-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2005, Hans-Martin Will. All rights reserved.
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
#include "render/render.h"
#include "raster/raster.h"


void GlesPrepareTriangles(State * state) {
}

/*
** Clip a polygon along lower boundary of frustrum
*/
static GLuint ClipLow(const Vertex * input[], GLuint inputCount, const Vertex * output[], Vertex ** nextTemporary, GLuint coord) {

	const Vertex * previous;
	const Vertex * current;
	GLuint resultCount, index;

	if (inputCount < 3) {
		return 0;
	}

	previous = input[inputCount - 1];
	resultCount = 0;

	for (index = 0; index < inputCount; ++index) {

		current = input[index];

		if (current->data[coord] >= -current->v.position.w) {

			if (previous->data[coord] >= -previous->v.position.w) {
				// line segment between previous and current is fully contained in cube
				output[resultCount++] = current;
				//previous = current;
			} else {
				// line segment between previous and current is intersected;
				// create vertex at intersection, then add current
				Vertex * newVertex = (*nextTemporary)++;
				
				GLfloat c_x = current->data[coord];
				GLfloat c_w = current->v.position.w;
				GLfloat p_x = previous->data[coord];
				GLfloat p_w = previous->v.position.w;
				GLfloat num = p_w + p_x; 
				GLfloat denom = (p_w + p_x) - (c_w + c_x);

				GlesInterpolateVertex(newVertex, current, previous, num / denom);
				newVertex->data[coord] = -newVertex->v.position.w;

				output[resultCount++] = newVertex;
				output[resultCount++] = current;
			}
		} else {
			if (previous->data[coord] >= -previous->v.position.w) {
				// line segment between previous and current is intersected;
				// create vertex at intersection and add it
				Vertex * newVertex = (*nextTemporary)++;
				
				GLfloat c_x = current->data[coord];
				GLfloat c_w = current->v.position.w;
				GLfloat p_x = previous->data[coord];
				GLfloat p_w = previous->v.position.w;
				GLfloat num = p_w + p_x; 
				GLfloat denom = (p_w + p_x) - (c_w + c_x);
				
				GlesInterpolateVertex(newVertex, current, previous, num / denom);
				newVertex->data[coord] = -newVertex->v.position.w;

				output[resultCount++] = newVertex;
			}
		}

		previous = current;
	}

	return resultCount;
}

/*
** Clip a polygon along lower boundary of frustrum
*/
static GLuint ClipHigh(const Vertex * input[], GLuint inputCount, const Vertex * output[], Vertex ** nextTemporary, GLuint coord) {

	const Vertex * previous;
	const Vertex * current;
	GLuint resultCount, index;

	if (inputCount < 3) {
		return 0;
	}

	previous = input[inputCount - 1];
	resultCount = 0;

	for (index = 0; index < inputCount; ++index) {

		current = input[index];

		if (current->data[coord] < current->v.position.w) {

			if (previous->data[coord] < previous->v.position.w) {
				// line segment between previous and current is fully contained in cube
				output[resultCount++] = current;
				//previous = current;
			} else {
				// line segment between previous and current is intersected;
				// create vertex at intersection, then add current
				Vertex * newVertex = (*nextTemporary)++;
				
				GLfloat c_x = current->data[coord];
				GLfloat c_w = current->v.position.w;
				GLfloat p_x = previous->data[coord];
				GLfloat p_w = previous->v.position.w;
				GLfloat num = p_w - p_x; 
				GLfloat denom = (p_w - p_x) - (c_w - c_x);

				GlesInterpolateVertex(newVertex, current, previous, num / denom);
				newVertex->data[coord] = newVertex->v.position.w;

				output[resultCount++] = newVertex;
				output[resultCount++] = current;
			}
		} else {
			if (previous->data[coord] < previous->v.position.w) {
				// line segment between previous and current is intersected;
				// create vertex at intersection and add it
				Vertex * newVertex = (*nextTemporary)++;
				
				GLfloat c_x = current->data[coord];
				GLfloat c_w = current->v.position.w;
				GLfloat p_x = previous->data[coord];
				GLfloat p_w = previous->v.position.w;
				GLfloat num = p_w - p_x; 
				GLfloat denom = (p_w - p_x) - (c_w - c_x);
				
				GlesInterpolateVertex(newVertex, current, previous, num / denom);
				newVertex->data[coord] = newVertex->v.position.w;

				output[resultCount++] = newVertex;
			}
		}

		previous = current;
	}

	return resultCount;
}


void GlesRenderTriangle(State * state, const Vertex * p1, const Vertex * p2, const Vertex * p3) {

	Vertex temporary[16];					/* space for vertices introduced through clipping	*/
	const Vertex * array1[16];				/* list of vertices									*/
	const Vertex * array2[16];				/* list of vertices									*/
	Vertex * nextTemporary = temporary;		/* pointer for next temporary vertex to use			*/
	GLuint numVertices = 3;					/* number of vertices								*/
	GLuint coord, index;					/* index of coordinate to clip						*/
	RasterVertex a, b, c;					/* screen coordinates								*/

	/*
	** Cull triangle based on orientation
	*/
	if (state->cullFaceEnabled) {
		GLfloat det =
			  p1->v.position.w * p2->v.position.x * p3->v.position.y
			+ p2->v.position.w * p3->v.position.x * p1->v.position.y
			+ p3->v.position.w * p1->v.position.x * p2->v.position.y
			- p1->v.position.w * p3->v.position.x * p2->v.position.y
			- p2->v.position.w * p1->v.position.x * p3->v.position.y
			- p3->v.position.w * p2->v.position.x * p1->v.position.y;

		switch (state->cullMode) {
			case GL_BACK:
				if ((det < 0.0f) ^ (state->frontFace == GL_CCW))
					return;

				break;

			case GL_FRONT:
				if ((det > 0.0f) ^ (state->frontFace == GL_CW))
					return;

				break;

			default:
			case GL_FRONT_AND_BACK:
				return;
		}
	}

	/*
	** Clip triangle to frustrum
	*/

	array1[0] = p1;
	array1[1] = p2;
	array1[2] = p3;

	for (coord = 0; coord < 3; ++coord) {
		numVertices = ClipLow(array1, numVertices, array2, &nextTemporary, coord);
		numVertices = ClipHigh(array2, numVertices, array1, &nextTemporary, coord);	
	}

	/*
	** Project to screen space and raster
	*/

	if (numVertices >= 3) {
		GlesViewportTransform(state, &a, array1[0]);
		GlesViewportTransform(state, &b, array1[1]);

		for (index = 2; index < numVertices; ++index) {
			GlesViewportTransform(state, &c, array1[index]);
			GlesRasterTriangle(state, &a, &b, &c);
			b = c;
		}
	}
}

void GlesRenderTriangles(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];

		GlesPrepareTriangles(state);

		while (count >= 3) {
			GLsizei idx;

			for (idx = 0; idx < 3; ++idx) {
				GlesSelectArrayElement(state, NEXT_INDEX);
				GlesProcessVertex(state, pos + idx);
			}

			GlesRenderTriangle(state, pos, pos + 1, pos + 2);

			count -= 3;
		}
	}
}

void GlesRenderTriangleStrip(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];
		GLsizei idx;
		GLsizei even;

		GlesPrepareTriangles(state);

		for (idx = 0; idx < 2; ++idx) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
		}

		count -= 2;
		even = 0;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderTriangle(state, pos + 2, pos + even, pos + (1 ^ even));

			count -= 1;
			if (--idx < 0)
				idx = 2;

			even ^= 1;
		}
	}
}


void GlesRenderTriangleFan(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	if (count >= 3) {
		Vertex pos[3];
		GLsizei idx;

		GlesPrepareTriangles(state);

		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos + 2);
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, pos + 0);

		count -= 2;
		idx = 1;

		while (count >= 1) {
			GlesSelectArrayElement(state, NEXT_INDEX);
			GlesProcessVertex(state, pos + idx);
			GlesRenderTriangle(state, pos + 2, pos + (idx ^ 1), pos + idx);

			count -= 1;
			idx ^= 1;
		}
	}
}

