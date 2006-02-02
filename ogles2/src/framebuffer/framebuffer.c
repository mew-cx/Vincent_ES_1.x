/*
** ==========================================================================
**
** framebuffer.h	Framebuffer interface
**
** --------------------------------------------------------------------------
**
** 01-31-2006		Hans-Martin Will	initial version
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
#include "framebuffer/framebuffer.h"

static BufferFormat formats[] = {
	{	GL_RGBA8,				GL_RGBA,			32,		4	},
	{	GL_RGBA4,				GL_RGBA,			16,		2	},
	{	GL_RGB5_A1,				GL_RGBA,			16,		2	},
	{	GL_RGB8,				GL_RGB,				24,		1	},
	{	GL_RGB565_OES,			GL_RGB,				16,		2	},
	{	GL_LUMINANCE,			GL_LUMINANCE,		 8,		1	},
	{	GL_LUMINANCE_ALPHA,		GL_LUMINANCE_ALPHA,	16,		1	},
	{	GL_DEPTH_COMPONENT16,	GL_NONE,			16,		2	},
	{	GL_DEPTH_COMPONENT24,	GL_NONE,			24,		1	},
	{	GL_DEPTH_COMPONENT32,	GL_NONE,			32,		4	},
	{	GL_STENCIL_INDEX1_OES,	GL_NONE,			 1,		1	},
	{	GL_STENCIL_INDEX4_OES,	GL_NONE,			 4,		1	},
	{	GL_STENCIL_INDEX8_OES,	GL_NONE,			 8,		1	},
};

const BufferFormat * GlesGetFormat(GLenum format) {
	GLuint index;

	for (index = 0; index < GLES_ELEMENTSOF(formats); ++index) {
		if (formats[index].format == format) {
			return formats + index;
		}
	}

	return NULL;
}
