/*
** ==========================================================================
**
** state.c			State management functions
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


/* Gloablly shared instance of GL state */
State OglesGlobalState;


void GlesRecordError(State * state, GLenum error) {
	if (state->lastError == GL_NO_ERROR) {
		state->lastError = error;
	}
}

void GlesRecordInvalidEnum(State * state) {
	GlesRecordError(state, GL_INVALID_ENUM);
}

void GlesRecordInvalidValue(State * state) {
	GlesRecordError(state, GL_INVALID_VALUE);
}

void GlesRecordOutOfMemory(State * state) {
	GlesRecordError(state, GL_OUT_OF_MEMORY);
}

GLboolean GlesValidateEnum(State * state, GLenum value, const GLenum * values, GLuint numValues) {
	while (numValues) {
		if (value == *values) {
			return GL_TRUE;
		}

		++values;
		--numValues;
	}

	GlesRecordInvalidEnum(state);
	return GL_FALSE;
}

void GlesInitState(State * state) {

	GLuint index;

	GLES_MEMSET(state, 0, sizeof *state);

	/* array state */

	for (index = 0; index < GLES_MAX_VERTEX_ATTRIBS; ++index) {
		GlesInitArray(state->vertexAttribArray + index);
		state->vertexAttrib[index].x = 0.0f;
		state->vertexAttrib[index].y = 0.0f;
		state->vertexAttrib[index].z = 0.0f;
		state->vertexAttrib[index].w = 1.0f;
	}

	/* buffer state */

	for (index = 0; index < GLES_MAX_BUFFERS; ++index) {
		GlesInitBuffer(state->buffers + index);
		state->bufferFreeList[index] = index + 1;
	}

	state->bufferFreeListHead = 1;
	state->bufferFreeList[GLES_MAX_BUFFERS - 1] = NIL;
	state->arrayBuffer = 0;
	state->elementArrayBuffer = 0;

	/* texture state */

	GlesInitTexture2D(&state->textureState.texture2D);
	GlesInitTexture3D(&state->textureState.texture3D);
	GlesInitTextureCube(&state->textureState.textureCube);

	state->texture2D				= 0;
	state->texture3D				= 0;
	state->textureCube				= 0;

	for (index = 0; index < GLES_MAX_TEXTURES; ++index) {
		state->textures[index].base.textureType = GL_INVALID_ENUM;
		state->textureFreeList[index] = index + 1;
	}

	state->textureFreeListHead = 1;
	state->textureFreeList[GLES_MAX_TEXTURES - 1] = NIL;

	/* shader state */

	for (index = 0; index < GLES_MAX_SHADERS; ++index) {
		state->shaders[index].type = GL_INVALID_ENUM;
		state->textureFreeList[index] = index + 1;
	}

	state->shaderFreeListHead = 1;
	state->shaderFreeList[GLES_MAX_SHADERS - 1] = NIL;

	/* program state */

	for (index = 0; index < GLES_MAX_PROGRAMS; ++index) {
		GlesInitProgram(state->programs + index);
		state->programFreeList[index] = index + 1;
	}

	state->programFreeListHead = 1;
	state->programFreeList[GLES_MAX_PROGRAMS - 1] = NIL;

	state->program = 0;

	/* rendering state */

	state->cullFaceEnabled			= GL_FALSE;
	state->cullMode					= GL_BACK;
	state->frontFace				= GL_CCW;

	/* rasterization state */

	state->lineWidth				= 1.0f;
	state->pointSize				= 1.0f;
	state->polygonOffsetFactor		= 0;
	state->polygonOffsetUnits		= 0;

	/* fragment processing state */

	state->blendEnabled				= GL_FALSE;
	state->blendColor.red			= 0.0f;
	state->blendColor.green			= 0.0f;
	state->blendColor.blue			= 0.0f;
	state->blendColor.alpha			= 0.0f;
	state->blendEqnModeRBG			= GL_FUNC_ADD;
	state->blendEqnModeAlpha		= GL_FUNC_ADD;
	state->blendFuncSrcRGB			= GL_ONE;
	state->blendFuncDstRGB			= GL_ZERO;
	state->blendFuncSrcAlpha		= GL_ONE;
	state->blendFuncDstAlpha		= GL_ZERO;

	state->colorMask.red			= GL_TRUE;
	state->colorMask.green			= GL_TRUE;
	state->colorMask.blue			= GL_TRUE;
	state->colorMask.alpha			= GL_TRUE;

	state->depthTestEnabled			= GL_FALSE;
	state->depthFunc				= GL_LESS;
	state->depthMask				= GL_TRUE;

	state->scissorTestEnabled		= GL_FALSE;
	state->scissorRect.x			= 0;
	state->scissorRect.y			= 0;
	state->scissorRect.width		= GLES_MAX_VIEWPORT_WIDTH;
	state->scissorRect.height		= GLES_MAX_VIEWPORT_HEIGHT;

	state->stencilTestEnabled		= GL_FALSE;
	state->stencilFrontFunc			= GL_ALWAYS;
	state->stencilFrontRef			= 0;
	state->stencilFrontMask			= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilBackFunc			= GL_ALWAYS;
	state->stencilBackRef			= 0;
	state->stencilBackMask			= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilFrontFail			= GL_KEEP;
	state->stencilFrontZfail		= GL_KEEP;
	state->stencilFrontZpass		= GL_KEEP;
	state->stencilBackFail			= GL_KEEP;
	state->stencilBackZfail			= GL_KEEP;
	state->stencilBackZpass			= GL_KEEP;
	state->stencilMask				= (1 << GLES_MAX_STENCIL_BITS) - 1;

	/* multi-sampling */
	state->multiSampleEnabled			= GL_FALSE;
	state->sampleAlphaToCoverageEnabled	= GL_FALSE;
	state->sampleAlphaToOneEnabled		= GL_FALSE;
	state->sampleCoverageEnabled		= GL_FALSE;
	state->sampleCovValue				= 1.0f;
	state->sampleCovInvert				= GL_FALSE;

	/* clear values */

	state->clearColor.red			= 0.0f;
	state->clearColor.green			= 0.0f;
	state->clearColor.blue			= 0.0f;
	state->clearColor.alpha			= 0.0f;

	state->clearDepth				= 1.0f;
	state->clearStencil				= 0;

	/* viewport state */

	state->viewport.x				= 0;
	state->viewport.y				= 0;
	state->viewport.width			= GLES_MAX_VIEWPORT_WIDTH;
	state->viewport.height			= GLES_MAX_VIEWPORT_HEIGHT;

	state->zNear					= 0.0f;
	state->zFar						= 1.0f;

	/* general settings */

	state->packAlignment			= 4;
	state->unpackAlignment			= 4;

	/* hints */

	state->generateMipmapHint			= GL_DONT_CARE;
	state->fragmentShaderDerivativeHint	= GL_DONT_CARE;

	/* error state */

	state->lastError				= GL_NO_ERROR;
}

void GlesGenObjects(State * state, GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLsizei n, GLuint *objs) {
	GLuint * base = objs;

	if (n < 0 || objs == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	while (n--) {
		GLuint nextObj = GlesBindObject(freeListHead, freeList, maxElements);

		if (nextObj == NIL) {
			GlesRecordError(state, GL_OUT_OF_MEMORY);

			while (base != objs) {
				GlesUnbindObject(freeListHead, freeList, maxElements, *base++);
			}

			return;
		}

		*objs++ = nextObj;
	}
}

GLuint GlesBindObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements) {
	if (freeListHead != NIL) {
		GLuint result = freeListHead;
		freeListHead = freeList[freeListHead];
		freeList[result] = BOUND;
		return result;
	} else {
		return NIL;
	}
}

void GlesUnbindObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLuint obj) {
	assert(GlesIsBoundObject(freeListHead, freeList, maxElements, obj));

	freeList[obj] = freeListHead;
	freeListHead = obj;
}

GLboolean GlesIsBoundObject(GLuint freeListHead, GLuint * freeList, GLuint maxElements, GLuint obj) {
	if (obj < maxElements) {
		return freeList[obj] == BOUND;
	} else {
		return GL_FALSE;
	}
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glDisable (GLenum cap) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glEnable (GLenum cap) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glFinish (void) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glFlush (void) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode) {
	State * state = GLES_GET_STATE();
}

GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap) {
	State * state = GLES_GET_STATE();
	return GL_FALSE;
}
