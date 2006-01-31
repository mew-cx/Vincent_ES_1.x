/*
** ==========================================================================
**
** uniform.c		Uniform variable setters and getters
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
** Public API entry points
** --------------------------------------------------------------------------
*/
GL_API GLint GL_APIENTRY glGetUniformLocation (GLuint program, const char *name) {
	State * state = GLES_GET_STATE();
	return -1;
}

GL_API void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetUniformxv (GLuint program, GLint location, GLfixed *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform1f (GLint location, GLfloat x) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform2f (GLint location, GLfloat x, GLfloat y) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform1i (GLint location, GLint x) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform2i (GLint location, GLint x, GLint y) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform3i (GLint location, GLint x, GLint y, GLint z) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
}

