/*
** ==========================================================================
**
** fragment.c		Fragment processing functions
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




/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/


GL_API void GL_APIENTRY glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBlendEquation (GLenum mode) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBlendEquationSeparate (GLenum modeRBG, GLenum modeAlpha) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDepthFunc (GLenum func) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDepthMask (GLboolean flag) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glStencilMask (GLuint mask) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
	State * state = GLES_GET_STATE();
}
