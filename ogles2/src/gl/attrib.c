/*
** ==========================================================================
**
** attrib.c			Attribute functions
**
** --------------------------------------------------------------------------
**
** 06-28-2005		Hans-Martin Will	initial version
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


/*
** --------------------------------------------------------------------------
** Module variables
** --------------------------------------------------------------------------
*/

static GLenum VertexAttribTypeValues[] = {
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_INT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_FIXED
};

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitArray(Array * array) {
	array->ptr			= NULL;
	array->boundBuffer	= 0;
	array->size			= 0;
	array->stride		= 0;
	array->type			= GL_FLOAT;
	array->normalized	= GL_FALSE;
	array->enabled		= GL_FALSE;
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glDisableVertexAttribArray (GLuint index) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttribArray[index].enabled = GL_FALSE;
	}
}

GL_API void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttribArray[index].enabled = GL_TRUE;
	}
}

GL_API void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		switch (pname) {
		case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
			*params = (GLfloat) (state->vertexAttribArray[index].enabled == GL_TRUE);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_SIZE:
			*params = (GLfloat) (state->vertexAttribArray[index].size);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
			*params = (GLfloat) (state->vertexAttribArray[index].stride);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_TYPE:
			*params = (GLfloat) (state->vertexAttribArray[index].type);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
			*params = (GLfloat) (state->vertexAttribArray[index].normalized == GL_TRUE);
			break;

		case GL_CURRENT_VERTEX_ATTRIB:
			params[0] = state->vertexAttrib[index].x;
			params[1] = state->vertexAttrib[index].y;
			params[2] = state->vertexAttrib[index].z;
			params[3] = state->vertexAttrib[index].w;
			break;

		default:
			GlesRecordInvalidEnum(state);
			break;
		}
	}
}

GL_API void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		switch (pname) {
		case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
			*params = state->vertexAttribArray[index].enabled == GL_TRUE;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_SIZE:
			*params = state->vertexAttribArray[index].size;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
			*params = state->vertexAttribArray[index].stride;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_TYPE:
			*params = state->vertexAttribArray[index].type;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
			*params = state->vertexAttribArray[index].normalized == GL_TRUE;
			break;

		case GL_CURRENT_VERTEX_ATTRIB:
			params[0] = (GLint) state->vertexAttrib[index].x;
			params[1] = (GLint) state->vertexAttrib[index].y;
			params[2] = (GLint) state->vertexAttrib[index].z;
			params[3] = (GLint) state->vertexAttrib[index].w;
			break;

		default:
			GlesRecordInvalidEnum(state);
			break;
		}
	}
}

GL_API void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void* *pointer) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER) {
		GlesRecordInvalidEnum(state);
	} else {
		*pointer = (void *) state->vertexAttribArray[index].ptr;
	}
}

GL_API void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) {
	glVertexAttrib4f(index, x, 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], 0.0f, 0.0f, 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
	glVertexAttrib4f(index, x, y, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], 0.0f, 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	glVertexAttrib4f(index, x, y, z, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], values[2], 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttrib[index].x = x;
		state->vertexAttrib[index].y = y;
		state->vertexAttrib[index].z = z;
		state->vertexAttrib[index].w = w;
	}
}

GL_API void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], values[2], values[3]);
	} 
}

GL_API void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * ptr) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (size < 1 || size > 4 || stride < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (!GlesValidateEnum(state, type, VertexAttribTypeValues, GLES_ELEMENTSOF(VertexAttribTypeValues))) {
		return;
	}

	state->vertexAttribArray[index].size		= size;
	state->vertexAttribArray[index].type		= type;
	state->vertexAttribArray[index].normalized	= normalized != GL_FALSE;
	state->vertexAttribArray[index].stride		= stride;
	state->vertexAttribArray[index].ptr			= ptr;
	state->vertexAttribArray[index].boundBuffer = state->arrayBuffer;
}
