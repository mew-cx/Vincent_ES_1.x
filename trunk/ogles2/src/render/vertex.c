/*
** ==========================================================================
**
** vertex.c			Vertex processing functions
**
** --------------------------------------------------------------------------
**
** 06-13-2005		Hans-Martin Will	initial version
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
#include "gl/state.h"
#include "render/render.h"


void GlesPrepareRendering(State * state) {

	GLsizei index;
	GLsizei numEnabledAttribArrays = 0;

	for (index = 0; index < GLES_MAX_VERTEX_ATTRIBS; ++index) {

		if (!state->vertexAttribArray[index].enabled) {
			GLES_MEMCPY(state->tempVertexAttrib + index, state->vertexAttrib + index, sizeof(Cell));
		} else {
			state->enabledAttribArrays[numEnabledAttribArrays++] = index;
		}

		state->numEnabledAttribArrays = numEnabledAttribArrays;
	}
}

void GlesSelectArrayElement(State * state, GLint index) {

	GLsizei idxAttrib;

	for (idxAttrib = 0; idxAttrib < state->numEnabledAttribArrays; ++idxAttrib) {

		GLsizei arrayIndex = state->enabledAttribArrays[idxAttrib];

		GLES_ASSERT(state->vertexAttribArray[arrayIndex].enabled);

		/* TO DO: Fetch attribArray[arrayIndex] from the array[arrayIndex][index] */
		GLES_ASSERT(0);
	}
}



void GlesProcessVertex(State * state, Vertex * result) {
}

/*
** Calculate the interpolation vertex between vertices v0 and v1 at relative position coeff
*/
void GlesInterpolateVertex(Vertex * newVertex, const Vertex * v0, const Vertex * v1, GLfloat coeff) {
	GLuint index;

	/*
	** TODO: restrict this to the actual number of floats used
	*/
	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
		newVertex->data[index] = v1->data[index] + (v0->data[index] - v1->data[index]) * coeff;
	}
}

/*
** Transform the vertex from clip space to viewport space
*/
void GlesViewportTransform(State * state, RasterVertex * raster, Vertex * vertex) {
	GLfloat x = vertex->v.position.x;
	GLfloat y = vertex->v.position.y;
	GLfloat z = vertex->v.position.z;
	GLfloat w = vertex->v.position.w;
	GLfloat depth, invW;

	// fix possible rounding problems
	if (x < -w)	x = -w;
	if (x > w)	x = w;
	if (y < -w)	y = -w;
	if (y > w)	y = w;
	if (z < -w)	z = -w;
	if (z > w)	z = w;

	// keep this value around for perspective-correct texturing
	invW = (w != 0.0f) ? 1.0f / w : 0.0f;

	// Scale 1/Z by 2^10 to avoid rounding problems during prespective correct
	// interpolation
	// See book by LaMothe for more detailed discussion on this
	raster->screen.w = invW;

	raster->screen.x = invW * x * state->viewportScale.x + state->viewportOrigin.x;
	raster->screen.y = invW * y * state->viewportScale.y + state->viewportOrigin.y;
	depth			 = invW * z * state->depthScale      + state->depthOrigin;

	raster->screen.z = GLES_CLAMP(depth);

	raster->varyingData = vertex->data;
}

