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
** Module data
** --------------------------------------------------------------------------
*/

static const GLenum BlendEquationValues[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT
};

static const GLenum BlendFuncValues[] = {
	GL_ZERO,						/* (0, 0, 0)				0		*/
	GL_ONE,							/* (1, 1, 1)				1		*/
	GL_SRC_COLOR,					/* (Rs,Gs,Bs)				As		*/
	GL_ONE_MINUS_SRC_COLOR,			/* (1, 1, 1) - (Rs,Gs,Bs)	1 - As	*/
	GL_DST_COLOR,					/* (Rd,Gd,Bd)				Ad		*/
	GL_ONE_MINUS_DST_COLOR,			/* (1, 1, 1) - (Rd,Gd,Bd)	1 - Ad	*/
	GL_SRC_ALPHA,					/* (As,As,As)				As		*/
	GL_ONE_MINUS_SRC_ALPHA,			/* (1, 1, 1) - (As,As,As)	1 - As	*/
	GL_DST_ALPHA,					/* (Ad,Ad,Ad)				Ad		*/
	GL_ONE_MINUS_DST_ALPHA,			/* (1, 1, 1) - (Ad,Ad,Ad)	1 - Ad	*/
	GL_CONSTANT_COLOR,				/* (Rc,Gc,Bc)				Ac		*/
	GL_ONE_MINUS_CONSTANT_COLOR,	/* (1, 1, 1) - (Rc,Gc,Bc)	1 - Ac	*/
	GL_CONSTANT_ALPHA,				/* (Ac,Ac,Ac)				Ac		*/
	GL_ONE_MINUS_CONSTANT_ALPHA,	/* (1, 1, 1) - (Ac,Ac,Ac)	1 - Ac	*/
	GL_SRC_ALPHA_SATURATE			/* (f, f, f)				1		*/
};

static const GLenum DepthFuncValues[] = {
	GL_NEVER, 
	GL_ALWAYS, 
	GL_LESS,
	GL_LEQUAL, 
	GL_EQUAL, 
	GL_GREATER, 
	GL_GEQUAL, 
	GL_NOTEQUAL
};

static const GLenum StencilFuncValues[] = {
	GL_NEVER, 
	GL_ALWAYS, 
	GL_LESS,
	GL_LEQUAL, 
	GL_EQUAL, 
	GL_GREATER, 
	GL_GEQUAL, 
	GL_NOTEQUAL
};

static const GLenum StencilOpValues[] = {
	GL_KEEP, 
	GL_ZERO, 
	GL_REPLACE, 
	GL_INCR, 
	GL_DECR, 
	GL_INVERT, 
	GL_INCR_WRAP, 
	GL_DECR_WRAP
};

static const GLenum FaceValues[] = {
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};


/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/


GL_API void GL_APIENTRY glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	State * state = GLES_GET_STATE();

	state->blendColor.red	= GlesClampf(red);
	state->blendColor.green = GlesClampf(green);
	state->blendColor.blue	= GlesClampf(blue);
	state->blendColor.alpha = GlesClampf(alpha);
}

GL_API void GL_APIENTRY glBlendEquation (GLenum mode) {
	glBlendEquationSeparate(mode, mode);
}

GL_API void GL_APIENTRY glBlendEquationSeparate (GLenum modeRBG, GLenum modeAlpha) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, modeRBG,   BlendEquationValues, GLES_ELEMENTSOF(BlendEquationValues)) &&
		GlesValidateEnum(state, modeAlpha, BlendEquationValues, GLES_ELEMENTSOF(BlendEquationValues))) {
		state->blendEqnModeRBG   = modeRBG;
		state->blendEqnModeAlpha = modeAlpha;
	}
}

GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) {
	glBlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
}

GL_API void GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, srcRGB,  BlendFuncValues, GLES_ELEMENTSOF(BlendFuncValues)) &&
		GlesValidateEnum(state, dstRGB,	 BlendFuncValues, GLES_ELEMENTSOF(BlendFuncValues)) &&
		GlesValidateEnum(state, srcAlpha,BlendFuncValues, GLES_ELEMENTSOF(BlendFuncValues)) &&
		GlesValidateEnum(state, dstAlpha,BlendFuncValues, GLES_ELEMENTSOF(BlendFuncValues))) {
			state->blendFuncSrcRGB   = srcRGB;
			state->blendFuncDstRGB   = dstRGB;
			state->blendFuncSrcAlpha = srcAlpha;
			state->blendFuncDstAlpha = dstAlpha;
	}
}

GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	State * state = GLES_GET_STATE();
	state->colorMask.red	= red	!= GL_FALSE;
	state->colorMask.green	= green != GL_FALSE;
	state->colorMask.blue	= blue	!= GL_FALSE;
	state->colorMask.alpha	= alpha != GL_FALSE;
}

GL_API void GL_APIENTRY glDepthFunc (GLenum func) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, func, DepthFuncValues, GLES_ELEMENTSOF(DepthFuncValues))) {
		state->depthFunc = func;
	}
}

GL_API void GL_APIENTRY glDepthMask (GLboolean flag) {
	State * state = GLES_GET_STATE();
	state->depthMask = flag != GL_FALSE;
}

GL_API void GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) {
	State * state = GLES_GET_STATE();

	state->sampleCovValue  = GlesClampf(value);
	state->sampleCovInvert = invert != GL_FALSE;
}

GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();

	if (width < 0 || height < 0) {
		GlesRecordInvalidValue(state);
	} else {
		state->scissorRect.x		= x;
		state->scissorRect.y		= y;
		state->scissorRect.width	= width;
		state->scissorRect.height	= height;
	}
}

GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) {
	glStencilFuncSeparate(GL_FRONT_AND_BACK, func, ref, mask);
}

GL_API void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, face, FaceValues, GLES_ELEMENTSOF(FaceValues)) &&
		GlesValidateEnum(state, func, StencilFuncValues, GLES_ELEMENTSOF(StencilFuncValues))) {

		if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
			state->stencilFront.func = func;
			state->stencilFront.ref  = ref;
			state->stencilFront.mask = mask;
		}

		if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
			state->stencilBack.func = func;
			state->stencilBack.ref  = ref;
			state->stencilBack.mask = mask;
		}
	}
}

GL_API void GL_APIENTRY glStencilMask (GLuint mask) {
	State * state = GLES_GET_STATE();
	state->stencilMask = mask;
}

GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
	glStencilOpSeparate(GL_FRONT_AND_BACK, fail, zfail, zpass);
}

GL_API void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, face,  FaceValues,		GLES_ELEMENTSOF(FaceValues)) &&
		GlesValidateEnum(state, fail,  StencilOpValues, GLES_ELEMENTSOF(StencilOpValues)) &&
		GlesValidateEnum(state, zfail, StencilOpValues, GLES_ELEMENTSOF(StencilOpValues)) &&
		GlesValidateEnum(state, zpass, StencilOpValues, GLES_ELEMENTSOF(StencilOpValues))) {

		if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
			state->stencilFront.fail = fail;
			state->stencilFront.zfail = zfail;
			state->stencilFront.zpass = zpass;
		}

		if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
			state->stencilBack.fail = fail;
			state->stencilBack.zfail = zfail;
			state->stencilBack.zpass = zpass;
		}
	}
}
