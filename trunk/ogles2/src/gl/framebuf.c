/*
** ==========================================================================
**
** framebuf.c		Framebuffer functions
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
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	State * state = GLES_GET_STATE();

	state->clearColor.red	= GlesClampf(red);
	state->clearColor.green	= GlesClampf(green);
	state->clearColor.blue	= GlesClampf(blue);
	state->clearColor.alpha = GlesClampf(alpha);
}

GL_API void GL_APIENTRY glClearColorx (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) {
	glClearColor(GlesFloatx(red), GlesFloatx(green), GlesFloatx(blue), GlesFloatx(alpha));
}

GL_API void GL_APIENTRY glClearDepthf (GLclampf depth) {
	State * state = GLES_GET_STATE();

	state->clearDepth = GlesClampf(depth);
}

GL_API void GL_APIENTRY glClearDepthx (GLclampx depth) {
	glClearDepthf(GlesFloatx(depth));
}

GL_API void GL_APIENTRY glClearStencil (GLint s) {
	State * state = GLES_GET_STATE();

	state->clearStencil = s;
}

GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) {
	State * state = GLES_GET_STATE();

	zNear = GlesClampf(zNear);
	zFar = GlesClampf(zFar);

	state->depthOrigin = (zNear + zFar) / 2.0f;
	state->depthScale = ((zFar - zNear) / 2.0f) * (1.0f - FLT_EPSILON);
}

GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();

	if (width < 0 || height < 0) {
		GlesRecordInvalidValue(state);
	} else {
		state->viewport.x		= x;
		state->viewport.y		= y;
		state->viewport.width	= width;
		state->viewport.height	= height;

		state->viewportOrigin.x	= x + width / 2.0f;
		state->viewportOrigin.y = y + height / 2.0f;
		state->viewportScale.x = width / 2.0f;
		state->viewportScale.y = height / 2.0f;
	}
}

GL_API void GL_APIENTRY glClear (GLbitfield mask) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
	State * state = GLES_GET_STATE();
}
