/*
** ==========================================================================
**
** buffer.c			Buffer management functions
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




/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void InitBuffer(Buffer * buffer) {
	buffer->data		= NULL;
	buffer->size		= 0;
	buffer->bufferType	= GL_INVALID_ENUM;
	buffer->usage		= GL_INVALID_ENUM;
	buffer->access		= GL_WRITE_ONLY;
	buffer->mapped		= GL_FALSE;
	buffer->mapPointer	= NULL;
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {

	State * state = GLES_GET_STATE();
	GenObjects(state, state->bufferFreeListHead, state->bufferFreeList, GLES_MAX_BUFFERS, n, buffers);
}

GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, void* *params) {
	State * state = GLES_GET_STATE();
}

GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {

	State * state = GLES_GET_STATE();
	return IsBoundObject(state->bufferFreeListHead, state->bufferFreeList, GLES_MAX_BUFFERS, buffer) &&
		state->buffers[buffer].bufferType != GL_INVALID_ENUM;
}

GL_API void* GL_APIENTRY glMapBuffer (GLenum target, GLenum access) {
	State * state = GLES_GET_STATE();
	return 0;
}

GL_API GLboolean GL_APIENTRY glUnmapBuffer (GLenum target) {
	State * state = GLES_GET_STATE();
	return GL_FALSE;
}

