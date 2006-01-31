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
** Internal functions
** --------------------------------------------------------------------------
*/

void InitArray(Array * array) {
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
}

GL_API void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void* *pointer) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *values) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *values) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *values) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *values) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * ptr) {
	State * state = GLES_GET_STATE();
}
