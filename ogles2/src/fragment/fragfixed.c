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
#include "fragment/fragment.h"
#include "framebuffer/framebuffer.h"


/*
** -------------------------------------------------------------------------
** Preferably, the following will be inlined
** -------------------------------------------------------------------------
*/

static GLES_INLINE GLubyte clamp(GLushort value) {
	return (value > GLES_UBYTE_MAX) ? (GLubyte) GLES_UBYTE_MAX : (GLubyte) value;
}

static GLES_INLINE GLubyte mul(GLubyte color, GLubyte factor) {
	GLushort prod = color * factor;

	return (prod + (prod >> 7)) >> 8;
}

/*
** -------------------------------------------------------------------------
** Local functions
** -------------------------------------------------------------------------
*/

static GLuint PerformStencilOp(GLenum op, GLuint stencilValue, GLuint ref) {
	switch (op) {
		default:
		case GL_KEEP:
			break;

		case GL_ZERO:
			stencilValue = 0;
			break;

		case GL_REPLACE:
			stencilValue = ref;
			break;

		case GL_INCR:
			if (stencilValue != 0xffffffff) {
				stencilValue++;
			}

			break;

		case GL_INCR_WRAP:
			stencilValue++;
			break;

		case GL_DECR:
			if (stencilValue != 0) {
				stencilValue--;
			}

			break;

		case GL_DECR_WRAP:
			stencilValue--;
			break;

		case GL_INVERT:
			stencilValue = ~stencilValue;
			break;
	}

	return stencilValue;
}

/*
** Post-fragment program fragment processing
*/
void GlesProcessFragment(State * state, Vec2i coords, GLuint depth, Colorub color, GLfloat coverage, GLboolean backFace) {

	Colorub		dst;						/* current color of the pixel	*/
	GLuint		zBufferValue;				/* current depth of the pixel	*/
	GLboolean	depthTest;					/* depth test result			*/
	GLint		index;						/* a loop index					*/

	/*
	** ------------------------------------------------------------------------
	** Scissor Test
	** ------------------------------------------------------------------------
	*/
	if (state->scissorTestEnabled) {
		if (coords.x < state->scissorRect.x || coords.x - state->scissorRect.x >= state->scissorRect.width ||
			coords.y < state->scissorRect.y || coords.y - state->scissorRect.y >= state->scissorRect.height) {
			return;
		}
	}

	zBufferValue = GlesFrameFetchDepth(state->writeSurface, coords);

	/*
	** ------------------------------------------------------------------------
	** Depth Test
	** ------------------------------------------------------------------------
	*/

	switch (state->depthFunc) {
		default:
		case GL_NEVER:		depthTest = GL_FALSE;					break;
		case GL_LESS:		depthTest = depth < zBufferValue;		break;
		case GL_EQUAL:		depthTest = depth == zBufferValue;		break;
		case GL_LEQUAL:		depthTest = depth <= zBufferValue;		break;
		case GL_GREATER:	depthTest = depth > zBufferValue;		break;
		case GL_NOTEQUAL:	depthTest = depth != zBufferValue;		break;
		case GL_GEQUAL:		depthTest = depth >= zBufferValue;		break;
		case GL_ALWAYS:		depthTest = GL_TRUE;					break;
	}

	if (!state->stencilTestEnabled && state->depthTestEnabled && !depthTest) {
		return;
	}

	/*
	** ------------------------------------------------------------------------
	** Stencil Test
	** ------------------------------------------------------------------------
	*/

	if (state->stencilTestEnabled) {

		GLboolean stencilTest;
		StencilParams * stencilParams = backFace ? &state->stencilBack : &state->stencilFront;
		GLuint stencilRef = stencilParams->ref & stencilParams->mask;
		GLuint stencilValue = GlesFrameFetchStencil(state->writeSurface, coords);
		GLuint stencil = stencilValue & stencilParams->mask;

		switch (stencilParams->func) {
			default:
			case GL_NEVER:		stencilTest = GL_FALSE;				break;
			case GL_LESS:		stencilTest = stencilRef < stencil;	break;
			case GL_EQUAL:		stencilTest = stencilRef == stencil;break;
			case GL_LEQUAL:		stencilTest = stencilRef <= stencil;break;
			case GL_GREATER:	stencilTest = stencilRef > stencil;	break;
			case GL_NOTEQUAL:	stencilTest = stencilRef != stencil;break;
			case GL_GEQUAL:		stencilTest = stencilRef >= stencil;break;
			case GL_ALWAYS:		stencilTest = GL_TRUE;				break;
		}

		if (!stencilTest) {
			PerformStencilOp(stencilParams->fail, stencilValue, stencilParams->ref);
			GlesFrameWriteStencil(state->writeSurface, coords, stencil & ~state->stencilMask | stencilValue & state->stencilMask);
			return;
		}

		if (depthTest) {
			PerformStencilOp(stencilParams->zpass, stencilValue, stencilParams->ref);
			GlesFrameWriteStencil(state->writeSurface, coords, stencil & ~state->stencilMask | stencilValue & state->stencilMask);
		} else {
			PerformStencilOp(stencilParams->zfail, stencilValue, stencilParams->ref);
			GlesFrameWriteStencil(state->writeSurface, coords, stencil & ~state->stencilMask | stencilValue & state->stencilMask);
		}
	}

	if (state->stencilTestEnabled && !depthTest && state->depthTestEnabled) {
		return;
	}

	/*
	** ------------------------------------------------------------------------
	** Blending
	** ------------------------------------------------------------------------
	*/

	dst = GlesFrameFetchColor(state->writeSurface, coords);

	if (state->blendEnabled) {

		Colorub srcCoeff, dstCoeff;

		switch (state->blendFuncSrcRGB) {
			default:
			case GL_ZERO:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = 0;
				break;

			case GL_ONE:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = GLES_UBYTE_MAX;
				break;

			case GL_DST_COLOR:
				srcCoeff.red	= dst.red;
				srcCoeff.green	= dst.green;
				srcCoeff.blue	= dst.blue;
				break;

			case GL_ONE_MINUS_DST_COLOR:
				srcCoeff.red	= GLES_UBYTE_MAX - dst.red;
				srcCoeff.green	= GLES_UBYTE_MAX - dst.green;
				srcCoeff.blue	= GLES_UBYTE_MAX - dst.blue;
				break;

			case GL_SRC_ALPHA:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = color.alpha;
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = GLES_UBYTE_MAX - color.alpha;
				break;

			case GL_DST_ALPHA:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				srcCoeff.red = srcCoeff.blue = srcCoeff.green = GLES_UBYTE_MAX - dst.alpha;
				break;

			case GL_SRC_ALPHA_SATURATE:
				{
					GLubyte rev = GLES_UBYTE_MAX - dst.alpha;
					GLubyte f = (rev < color.alpha ? rev : color.alpha);
					srcCoeff.red = srcCoeff.blue = srcCoeff.green = f;
				}
				break;
		}

		switch (state->blendFuncSrcAlpha) {
			default:
			case GL_ZERO:
				srcCoeff.alpha = 0;
				break;

			case GL_SRC_ALPHA_SATURATE:
			case GL_ONE:
				srcCoeff.alpha = GLES_UBYTE_MAX;
				break;

			case GL_DST_COLOR:
				srcCoeff.alpha = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_COLOR:
				srcCoeff.alpha = GLES_UBYTE_MAX - dst.alpha;
				break;

			case GL_SRC_ALPHA:
				srcCoeff.alpha = color.alpha;
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				srcCoeff.alpha = GLES_UBYTE_MAX - color.alpha;
				break;

			case GL_DST_ALPHA:
				srcCoeff.alpha = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				srcCoeff.alpha = GLES_UBYTE_MAX - dst.alpha;
				break;
		}

		switch (state->blendFuncDstRGB) {
			default:
			case GL_ZERO:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = 0;
				break;

			case GL_ONE:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = GLES_UBYTE_MAX;
				break;

			case GL_DST_COLOR:
				dstCoeff.red	= color.red;
				dstCoeff.green	= color.green;
				dstCoeff.blue	= color.blue;
				break;

			case GL_ONE_MINUS_DST_COLOR:
				dstCoeff.red	= GLES_UBYTE_MAX - color.red;
				dstCoeff.green	= GLES_UBYTE_MAX - color.green;
				dstCoeff.blue	= GLES_UBYTE_MAX - color.blue;
				break;

			case GL_SRC_ALPHA:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = color.alpha;
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = GLES_UBYTE_MAX - color.alpha;
				break;

			case GL_DST_ALPHA:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				dstCoeff.red = dstCoeff.green = dstCoeff.blue = GLES_UBYTE_MAX - dst.alpha;
				break;

		}

		switch (state->blendFuncDstAlpha) {
			default:
			case GL_ZERO:
				dstCoeff.alpha = 0;
				break;

			case GL_ONE:
				dstCoeff.alpha = GLES_UBYTE_MAX;
				break;

			case GL_DST_COLOR:
				dstCoeff.alpha = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_COLOR:
				dstCoeff.alpha = GLES_UBYTE_MAX - dst.alpha;
				break;

			case GL_SRC_ALPHA:
				dstCoeff.alpha = color.alpha;
				break;

			case GL_ONE_MINUS_SRC_ALPHA:
				dstCoeff.alpha = GLES_UBYTE_MAX - color.alpha;
				break;

			case GL_DST_ALPHA:
				dstCoeff.alpha = dst.alpha;
				break;

			case GL_ONE_MINUS_DST_ALPHA:
				dstCoeff.alpha = GLES_UBYTE_MAX - dst.alpha;
				break;

		}

		for (index = 0; index < GLES_ELEMENTSOF(color.rgba); ++index) {
			color.rgba[index] = clamp(mul(srcCoeff.rgba[index], color.rgba[index]) + mul(dstCoeff.rgba[index], dst.rgba[index]));
		}
	}

	/*
	** ------------------------------------------------------------------------
	** Masking and write to framebuffer
	** ------------------------------------------------------------------------
	*/

	if (state->depthMask) {
		GlesFrameWriteDepth(state->writeSurface, coords, depth);
	}

	if (!state->colorMask.red) {
		color.red = dst.red;
	}

	if (!state->colorMask.green) {
		color.green = dst.green;
	}

	if (!state->colorMask.blue) {
		color.blue = dst.blue;
	}

	if (!state->colorMask.alpha) {
		color.alpha = dst.alpha;
	}

	GlesFrameWriteColor(state->writeSurface, coords, color);
}
