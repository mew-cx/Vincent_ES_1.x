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
#include "render/render.h"


/*
** --------------------------------------------------------------------------
** Module local functions
** --------------------------------------------------------------------------
*/

static void FetchByte(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}

static void FetchUByte(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchShort(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchUShort(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchInt(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchUInt(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchByteNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (*src++ * 2 + 1) * (1.0f / 255.0f);
	} while (--elements);
}

static void FetchUByteNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (1.0f/255.0f) * *src++;
	} while (--elements);
}


static void FetchShortNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (*src++ * 2 + 1) * (1.0f / 65536.0f);
	} while (--elements);
}


static void FetchUShortNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (1.0f/65535.0f) * *src++;
	} while (--elements);
}


static void FetchIntNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (GLfloat) ((*src++ * 2 + 1) * (1.0 / 4294967295.0));
	} while (--elements);
}


static void FetchUIntNorm(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLbyte * src = (const GLbyte *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (GLfloat) ((1.0 / 4294967295.0) * *src++);
	} while (--elements);
}


static void FetchFloat(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLfloat * src = (const GLfloat *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = *src++;
	} while (--elements);
}


static void FetchFixed(const void * arrayBase, GLsizei elements, Vec4f * result) {
	const GLfixed * src = (const GLfixed *) arrayBase;
	GLfloat * dst = result->v;
	
	do {
		*dst++ = (GLfloat)(/*(GLdouble)*/ *src++ * (1.0f/65536.0f));
	} while (--elements);
}



/*
** --------------------------------------------------------------------------
** Entry points exported my module
** --------------------------------------------------------------------------
*/
void GlesPrepareRendering(State * state) {

	GLsizei index;
	GLsizei numEnabledAttribArrays = 0;

	for (index = 0; index < GLES_MAX_VERTEX_ATTRIBS; ++index) {

		if (!state->vertexAttribArray[index].enabled) {
			GLES_MEMCPY(state->tempVertexAttrib + index, state->vertexAttrib + index, sizeof(state->vertexAttrib[0]));
		} else {
			Array * vertexArray = state->vertexAttribArray + index;

			if (vertexArray->boundBuffer) {
				vertexArray->effectivePtr = (const GLubyte *) state->buffers[vertexArray->boundBuffer].data + (GLsizeiptr) vertexArray->ptr;
			} else {
				vertexArray->effectivePtr = (const GLubyte *) vertexArray->ptr;
			}

			if (vertexArray->normalized) {
				switch (vertexArray->type) {
				case GL_BYTE:			vertexArray->fetchFunc = FetchByteNorm;		break;
				case GL_UNSIGNED_BYTE:	vertexArray->fetchFunc = FetchUByteNorm;	break;
				case GL_SHORT:			vertexArray->fetchFunc = FetchShortNorm;	break;
				case GL_UNSIGNED_SHORT:	vertexArray->fetchFunc = FetchUShortNorm;	break;
				case GL_INT:			vertexArray->fetchFunc = FetchIntNorm;		break;
				case GL_UNSIGNED_INT:	vertexArray->fetchFunc = FetchUIntNorm;		break;
				case GL_FLOAT:			vertexArray->fetchFunc = FetchFloat;		break;
				case GL_FIXED:			vertexArray->fetchFunc = FetchFixed;		break;
				default:				vertexArray->fetchFunc = NULL;				break;
				}
			} else {
				switch (vertexArray->type) {
				case GL_BYTE:			vertexArray->fetchFunc = FetchByte;			break;
				case GL_UNSIGNED_BYTE:	vertexArray->fetchFunc = FetchUByte;		break;
				case GL_SHORT:			vertexArray->fetchFunc = FetchShort;		break;
				case GL_UNSIGNED_SHORT:	vertexArray->fetchFunc = FetchUShort;		break;
				case GL_INT:			vertexArray->fetchFunc = FetchInt;			break;
				case GL_UNSIGNED_INT:	vertexArray->fetchFunc = FetchUInt;			break;
				case GL_FLOAT:			vertexArray->fetchFunc = FetchFloat;		break;
				case GL_FIXED:			vertexArray->fetchFunc = FetchFixed;		break;
				default:				vertexArray->fetchFunc = NULL;				break;
				}
			}

			state->enabledAttribArrays[numEnabledAttribArrays] = index;
			state->tempVertexAttrib[numEnabledAttribArrays].x = 0.0f;
			state->tempVertexAttrib[numEnabledAttribArrays].y = 0.0f;
			state->tempVertexAttrib[numEnabledAttribArrays].z = 0.0f;
			state->tempVertexAttrib[numEnabledAttribArrays].w = 1.0f;

			++numEnabledAttribArrays;
		}

		state->numEnabledAttribArrays = numEnabledAttribArrays;
	}
}

void GlesSelectArrayElement(State * state, GLint index) {

	GLsizei idxAttrib;

	for (idxAttrib = 0; idxAttrib < state->numEnabledAttribArrays; ++idxAttrib) {

		GLsizei arrayIndex = state->enabledAttribArrays[idxAttrib];
		const Array * vertexArray = state->vertexAttribArray + arrayIndex;
		GLsizeiptr offset;

		GLES_ASSERT(vertexArray->enabled);

		offset = vertexArray->stride * index;

		vertexArray->fetchFunc((const GLubyte *) vertexArray->effectivePtr + offset, 
			vertexArray->size, state->tempVertexAttrib + idxAttrib);
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
void GlesViewportTransform(State * state, RasterVertex * raster, const Vertex * vertex) {
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

