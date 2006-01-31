/*
** ==========================================================================
**
** render.c			Rendering functions
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
#include "render/render.h"



/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glCullFace (GLenum mode) {
	State * state = GLES_GET_STATE();
}

GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {

	State * state = GLES_GET_STATE();

	if (count < 0) {
		RecordInvalidValue(state);
		return;
	}

	/*
	if (!m_VertexArrayEnabled ||
		(m_MatrixPaletteEnabled && (!m_MatrixIndexArrayEnabled || !m_WeightArrayEnabled))) {
		RecordError(GL_INVALID_OPERATION);
		return;
	}

	*/

	GlesPrepareRendering(state);

	switch (mode) {
	case GL_POINTS:
		GlesRenderPoints(state, first, count, 0, 0);
		break;

	case GL_LINES:
		GlesRenderLines(state, first, count, 0, 0);
		break;

	case GL_LINE_STRIP:
		GlesRenderLineStrip(state, first, count, 0, 0);
		break;

	case GL_LINE_LOOP:
		GlesRenderLineLoop(state, first, count, 0, 0);
		break;

	case GL_TRIANGLES:
		GlesRenderTriangles(state, first, count, 0, 0);
		break;

	case GL_TRIANGLE_STRIP:
		GlesRenderTriangleStrip(state, first, count, 0, 0);
		break;

	case GL_TRIANGLE_FAN:
		GlesRenderTriangleFan(state, first, count, 0, 0);
		break;

	default:
		RecordInvalidEnum(state);
		return;
	}
}

GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {

	State * state = GLES_GET_STATE();

	if (count < 0) {
		RecordInvalidValue(state);
		return;
	}

	/*
	if (!m_VertexArrayEnabled ||
		(m_MatrixPaletteEnabled && (!m_MatrixIndexArrayEnabled || !m_WeightArrayEnabled))) {
		RecordError(GL_INVALID_OPERATION);
		return;
	}

	if (m_CurrentElementArrayBuffer) {
		U8 * bufferBase =
			static_cast<U8 *>(m_Buffers.GetObject(m_CurrentElementArrayBuffer)->GetData());

		if (!bufferBase) {
			RecordError(GL_INVALID_OPERATION);
			return;
		}

		size_t offset = static_cast<const U8 *>(indices) - static_cast<const U8 *>(0);
		indices = bufferBase + offset;
	}

	if (!indices) {
		return;
	}

	*/
	GlesPrepareRendering(state);

	switch (mode) {
	case GL_POINTS:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderPoints(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_LINES:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderLines(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_LINE_STRIP:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderLineStrip(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_LINE_LOOP:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderLineLoop(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_TRIANGLES:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderTriangles(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_TRIANGLE_STRIP:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderTriangleStrip(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	case GL_TRIANGLE_FAN:
		if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT) {
			GlesRenderTriangleFan(state, 0, count, type, indices);
		} else {
			RecordInvalidEnum(state);
		}

		break;

	default:
		RecordInvalidEnum(state);
		return;
	}
}

GL_API void GL_APIENTRY glFrontFace (GLenum mode) {
	State * state = GLES_GET_STATE();
}

