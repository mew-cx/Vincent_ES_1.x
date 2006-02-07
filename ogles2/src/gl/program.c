/*
** ==========================================================================
**
** program.c		Program management functions
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

void GlesInitProgram(Program * program) {
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const char * name) {
	State * state = GLES_GET_STATE();
}

GL_API GLuint GL_APIENTRY glCreateProgram (void) {

	State * state = GLES_GET_STATE();
	GLuint program = GlesBindObject(state->programFreeListHead, state->programFreeList, GLES_MAX_PROGRAMS);

	if (program == NIL) {
		GlesRecordError(state, GL_OUT_OF_MEMORY);
		return 0;
	} else {
		GlesInitProgram(state->programs + program);
		return program;
	}
}

GL_API void GL_APIENTRY glDeleteProgram (GLuint program) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders) {
	State * state = GLES_GET_STATE();
}

GL_API GLint GL_APIENTRY glGetAttribLocation (GLuint program, const char *name) {
	State * state = GLES_GET_STATE();
	return -1;
}

GL_API void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei *length, char *infolog) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
}

GL_API GLboolean GL_APIENTRY glIsProgram (GLuint program) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->programFreeListHead, state->programFreeList, GLES_MAX_PROGRAMS, program);
}

GL_API void GL_APIENTRY glLinkProgram (GLuint pogram) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glProgramBinary(GLuint program, const void *binary, GLint length) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUseProgram (GLuint program) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glValidateProgram (GLuint program) {
	State * state = GLES_GET_STATE();
}

