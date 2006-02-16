/*
** ==========================================================================
**
** fragment.c		Fragment Processing Functions
**
** --------------------------------------------------------------------------
**
** 02-12-2005		Hans-Martin Will	initial version
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
#include "framebuffer/framebuffer.h"

/*
** TO DO:
** ======
**
** At this point, these functions do not support framebuffer formats that
** aggregate multiple buffers into a single array (e.g. stencil8 + depth24)
** or that split a single buffer into to arrays in memory (e.g. RGB565 + A8)
*/

Colorub GlesFrameFetchColor(const Surface * frame, Vec2i coords) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;
	Colorub result;

	switch (frame->colorFormat) {
		case GL_RGB565_OES:
		case GL_UNSIGNED_SHORT_5_6_5:
			{
				GLushort u565 = *((const GLushort *) frame->colorBuffer + offset);
				GLubyte b = (u565 & 0x001Fu) << 3;
				GLubyte g = (u565 & 0x07E0u) >> 3;
				GLubyte r = (u565 & 0xF800u) >> 8;

				result.red	 = r | (r >> 5);
				result.green = g | (g >> 6);
				result.blue	 = b | (b >> 5);
				result.alpha = GLES_UBYTE_MAX;
			}

			break;

		case GL_RGBA4:
		case GL_UNSIGNED_SHORT_4_4_4_4:
			{
				GLushort u4444 = *((const GLushort *) frame->colorBuffer + offset);
				GLubyte r = (u4444 & 0xF000u) >> 8;
				GLubyte g = (u4444 & 0x0F00u) >> 4;
				GLubyte b = (u4444 & 0x00F0u);
				GLubyte a = (u4444 & 0x000Fu) << 4;

				result.red	 = r | (r >> 4);
				result.green = g | (g >> 4);
				result.blue  = b | (b >> 4);
				result.alpha = a | (a >> 4);
			}

			break;

		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_RGB5_A1:
			{
				GLushort u5551 = *((const GLushort *) frame->colorBuffer + offset);
				GLubyte b = (u5551 & 0x003Eu) << 2;
				GLubyte g = (u5551 & 0x07C0u) >> 3;
				GLubyte r = (u5551 & 0xF800u) >> 8;
				GLubyte a = (u5551 & 0x0001u) << 7;

				result.red   = r | (r >> 5);
				result.green = g | (g >> 5);
				result.blue  = b | (b >> 5);
				result.alpha = a ? GLES_UBYTE_MAX : 0;
			}

			break;

		case GL_RGB:
		case GL_RGB8:
			{
				const GLubyte * rgb = (const GLubyte *) frame->colorBuffer + offset * 3;
				result.red	 = rgb[0];
				result.green = rgb[1];
				result.blue  = rgb[2];
				result.alpha = GLES_UBYTE_MAX;
			}

			break;

		case GL_RGBA8:
		case GL_RGBA:
			{
				const GLubyte * rgba = (const GLubyte *) frame->colorBuffer + offset * 4;
				result.red	 = rgba[0];
				result.green = rgba[1];
				result.blue  = rgba[2];
				result.alpha = rgba[3];
			}

			break;

		default:
			result.red = result.green = result.blue = result.alpha = 0;
			break;

	}

	return result;
}

void GlesFrameWriteColor(const Surface * frame, Vec2i coords, Colorub rgba) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;

	switch (frame->colorFormat) {
		case GL_RGB565_OES:
		case GL_UNSIGNED_SHORT_5_6_5:
			*((GLushort *) frame->colorBuffer + offset) = 
				(rgba.blue & 0xF8) >> 3 | (rgba.green & 0xFC) << 3 | (rgba.red & 0xF8) << 8;;

			break;

		case GL_RGBA4:
		case GL_UNSIGNED_SHORT_4_4_4_4:
			*((GLushort *) frame->colorBuffer + offset) = 
				(rgba.red   & 0xf0) << 8 |
				(rgba.green & 0xf0) << 4 |
				(rgba.blue  & 0xf0)      |
				 rgba.alpha >> 4;

			break;

		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_RGB5_A1:
			*((GLushort *) frame->colorBuffer + offset) = 
				(rgba.blue & 0xF8) >> 2 | (rgba.green & 0xF8) << 3 | (rgba.red & 0xF8) << 8 | (rgba.alpha & 0x80) >> 7;
			break;

		case GL_RGB:
		case GL_RGB8:
			{
				GLubyte * rgb = (GLubyte *) frame->colorBuffer + offset * 3;
				rgb[0] = rgba.red;
				rgb[1] = rgba.green;
				rgb[2] = rgba.blue;
			}

			break;

		case GL_RGBA8:
		case GL_RGBA:
			{
				GLubyte * dest = (GLubyte *) frame->colorBuffer + offset * 4;
				dest[0] = rgba.red;
				dest[1] = rgba.green;
				dest[2] = rgba.blue;
				dest[3] = rgba.alpha;
			}

			break;
	}
}

GLuint GlesFrameFetchDepth(const Surface * frame, Vec2i coords) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;

	switch (frame->depthFormat) {
		case GL_DEPTH_COMPONENT16:
			return *((const GLushort *) frame->depthBuffer + offset);

		case GL_DEPTH_COMPONENT24:
			{
				const GLubyte * ptr = (const GLubyte *) frame->depthBuffer + offset * 3;
				return (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
			}

		case GL_DEPTH_COMPONENT32:
			return *((const GLuint *) frame->depthBuffer + offset);

		default:
			return 0;
	}
}

void GlesFrameWriteDepth(const Surface * frame, Vec2i coords, GLuint depth) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;

	switch (frame->depthFormat) {
		case GL_DEPTH_COMPONENT16:
			*((GLushort *) frame->depthBuffer + offset) = depth;
			break;

		case GL_DEPTH_COMPONENT24:
			{
				GLubyte * ptr = (GLubyte *) frame->depthBuffer + offset * 3;
				ptr[0] = depth >> 16;
				ptr[1] = depth >> 8;
				ptr[2] = depth;
			}

			break;

		case GL_DEPTH_COMPONENT32:
			*((GLuint *) frame->depthBuffer + offset) = depth;
			break;
	}
}

GLuint GlesFrameFetchStencil(const Surface * frame, Vec2i coords) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;

	switch (frame->stencilFormat) {
		case GL_STENCIL_INDEX4_OES:
			{
				GLubyte ubyte = *((const GLubyte *) frame->stencilBuffer + (offset >> 1));

				if (offset & 1) {
					return ubyte >> 4;
				} else {
					return ubyte & 0xf;
				}
			}

		case GL_STENCIL_INDEX8_OES:
			return *((const GLubyte *) frame->stencilBuffer + (offset >> 1));

		case GL_STENCIL_INDEX1_OES:
			{
				GLubyte ubyte = *((const GLubyte *) frame->stencilBuffer + (offset >> 3));
				return (ubyte >> (offset & 7)) & 1;
			}

		default:
			return 0;
	}

}

void GlesFrameWriteStencil(const Surface * frame, Vec2i coords, GLuint stencil) {
	GLsizeiptr offset = coords.x + frame->width * coords.y;

	switch (frame->stencilFormat) {
		case GL_STENCIL_INDEX4_OES:
			{
				GLubyte * ptr = (GLubyte *) frame->stencilBuffer + (offset >> 1);

				if (offset & 1) {
					*ptr = (*ptr & 0xf) | stencil << 4;
				} else {
					*ptr = (*ptr & 0xf0) | stencil & 0xf;
				}
			}

			break;

		case GL_STENCIL_INDEX8_OES:
			*((GLubyte *) frame->stencilBuffer + (offset >> 1)) = stencil;
			break;

		case GL_STENCIL_INDEX1_OES:
			{
				GLubyte * ptr = (GLubyte *) frame->stencilBuffer + (offset >> 3);
				GLuint shift = offset & 7;
				GLubyte bit = (1 << shift);
				*ptr = ((*ptr) & ~bit) | ((stencil & 1) << shift);
			}

			break;
	}
}
