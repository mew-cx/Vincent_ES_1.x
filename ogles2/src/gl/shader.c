/*
** ==========================================================================
**
** shader.c			Shader management functions
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

void InitShader(Shader * shader, GLenum shaderType) {
	GLES_ASSERT(shaderType == GL_FRAGMENT_SHADER || shaderType == GL_VERTEX_SHADER);
	shader->type = shaderType;
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glCompileShader (GLuint shader) {
	State * state = GLES_GET_STATE();
}

GL_API GLuint GL_APIENTRY glCreateShader (GLenum type) {

	State * state = GLES_GET_STATE();
	GLuint shader;

	if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
		GlesRecordInvalidEnum(state);
		return 0;
	}

	shader = BindObject(state->shaderFreeListHead, state->shaderFreeList, GLES_MAX_SHADERS);

	if (shader == NIL) {
		GlesRecordError(state, GL_OUT_OF_MEMORY);
		return 0;
	} else {
		InitShader(state->shaders + shader, type);
		return shader;
	}
}

GL_API void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei *length, char *source) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetShaderPrecisionFormatf(GLenum shadertype, GLenum precisiontype, GLfloat * range, GLfloat * precision) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glGetShaderPrecisionFormati(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision) {
	State * state = GLES_GET_STATE();
}

GL_API GLboolean GL_APIENTRY glIsShader (GLuint shader) {

	State * state = GLES_GET_STATE();
	return IsBoundObject(state->shaderFreeListHead, state->shaderFreeList, GLES_MAX_SHADERS, shader);
}

GL_API GLboolean GL_APIENTRY glIsShaderCompilerAvailable (void) {
	State * state = GLES_GET_STATE();
	return GL_FALSE;
}

GL_API GLboolean GL_APIENTRY glIsShaderPrecisionFormatSupported (GLenum shadertype, GLenum precisiontype) {
	State * state = GLES_GET_STATE();
	return GL_FALSE;
}

GL_API void GL_APIENTRY glReleaseShaderCompiler (void) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const char* *string, const GLint *length) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glShaderBinary (GLuint shader, const void * data, GLint length) {
	State * state = GLES_GET_STATE();
}

