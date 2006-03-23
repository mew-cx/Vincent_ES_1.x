// ==========================================================================
//
// render.cpp	Rendering Context Class for 3D Rendering Library
//
//				Rendering Operations
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
//
// --------------------------------------------------------------------------
//
// Copyright (c) 2004, Hans-Martin Will. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//	 *  Redistributions of source code must retain the above copyright
// 		notice, this list of conditions and the following disclaimer.
//   *	Redistributions in binary form must reproduce the above copyright
// 		notice, this list of conditions and the following disclaimer in the
// 		documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// ==========================================================================


#include "stdafx.h"
#include "context.h"
#include <string.h>
#include "fixed.h"
#include "surface.h"


using namespace EGL;


// --------------------------------------------------------------------------
// Setup mesh arrays
// --------------------------------------------------------------------------

void Context :: ToggleClientState(GLenum array, bool value) {
	switch (array) {
	case GL_TEXTURE_COORD_ARRAY:
		m_TexCoordArrayEnabled[m_ClientActiveTexture] = value;
		break;

	case GL_COLOR_ARRAY:
		m_ColorArrayEnabled = value;
		break;

	case GL_NORMAL_ARRAY:
		m_NormalArrayEnabled = value;
		break;

	case GL_VERTEX_ARRAY:
		m_VertexArrayEnabled = value;
		break;

	case GL_POINT_SIZE_ARRAY_OES:
		m_PointSizeArrayEnabled = value;
		break;

	default:
		RecordError(GL_INVALID_ENUM);
	}
}

void Context :: DisableClientState(GLenum array) {
	ToggleClientState(array, false);
}

void Context :: EnableClientState(GLenum array) {
	ToggleClientState(array, true);
}

void Context :: ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {

	if (type != GL_UNSIGNED_BYTE && type != GL_FIXED && type != GL_FLOAT) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size != 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride == 0) {
		switch (type) {
		case GL_UNSIGNED_BYTE:
			stride = sizeof (GLubyte) * size;
			break;

		case GL_FIXED:
			stride = sizeof (GLfixed) * size;
			break;

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;
		}
	}

	m_ColorArray.pointer = pointer;
	m_ColorArray.stride = stride;
	m_ColorArray.type = type;
	m_ColorArray.size = size;
	m_ColorArray.boundBuffer = m_CurrentArrayBuffer;
}

void Context :: NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED && type != GL_FLOAT) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	GLsizei size = 3;

	if (stride < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride == 0) {
		switch (type) {
		case GL_BYTE:
			stride = sizeof (GLbyte) * size;
			break;

		case GL_SHORT:
			stride = sizeof (GLshort) * size;
			break;

		case GL_FIXED:
			stride = sizeof (GLfixed) * size;
			break;

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;

		}
	}

	m_NormalArray.pointer = pointer;
	m_NormalArray.stride = stride;
	m_NormalArray.type = type;
	m_NormalArray.size = size;
	m_NormalArray.boundBuffer = m_CurrentArrayBuffer;
}

void Context :: VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED && type != GL_FLOAT) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size < 2 || size > 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride == 0) {
		switch (type) {
		case GL_BYTE:
			stride = sizeof (GLbyte) * size;
			break;

		case GL_SHORT:
			stride = sizeof (GLshort) * size;
			break;

		case GL_FIXED:
			stride = sizeof (GLfixed) * size;
			break;

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;

		}
	}

	m_VertexArray.pointer = pointer;
	m_VertexArray.stride = stride;
	m_VertexArray.type = type;
	m_VertexArray.size = size;
	m_VertexArray.boundBuffer = m_CurrentArrayBuffer;
}

void Context :: TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED && type != GL_FLOAT) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size < 2 || size > 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (stride == 0) {
		switch (type) {
		case GL_BYTE:
			stride = sizeof (GLbyte) * size;
			break;

		case GL_SHORT:
			stride = sizeof (GLshort) * size;
			break;

		case GL_FIXED:
			stride = sizeof (GLfixed) * size;
			break;

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;

		}
	}

	m_TexCoordArray[m_ClientActiveTexture].pointer = pointer;
	m_TexCoordArray[m_ClientActiveTexture].stride = stride;
	m_TexCoordArray[m_ClientActiveTexture].type = type;
	m_TexCoordArray[m_ClientActiveTexture].size = size;
	m_TexCoordArray[m_ClientActiveTexture].boundBuffer = m_CurrentArrayBuffer;
}


// --------------------------------------------------------------------------
// Default values of array is disabled
// --------------------------------------------------------------------------


void Context :: Color4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
	m_DefaultRGBA.r = red;
	m_DefaultRGBA.g = green;
	m_DefaultRGBA.b = blue;
	m_DefaultRGBA.a = alpha;
}


void Context :: MultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {

	if (target < GL_TEXTURE0 || target >= GL_TEXTURE0 + EGL_NUM_TEXTURE_UNITS) {
		// only have one texture unit
		RecordError(GL_INVALID_VALUE);
		return;
	}

	size_t unit = target - GL_TEXTURE0;

	m_DefaultTextureCoords[unit] = Vec4D(s, t, r, q);
}


void Context :: Normal3x(GLfixed nx, GLfixed ny, GLfixed nz) {
	m_DefaultNormal = Vec3D(nx, ny, nz);
}


// --------------------------------------------------------------------------
// Actual array rendering
// --------------------------------------------------------------------------


bool Context :: Begin(GLenum mode) {
	m_PrimitiveState = 0;
	m_NextIndex = 0;

	switch (mode) {
	case GL_POINTS:
		m_Rasterizer->PreparePoint();
		m_DrawPrimitiveFunction = &Context::DrawPoint;
		m_EndPrimitiveFunction = 0;
		break;

	case GL_LINES:
		m_Rasterizer->PrepareLine();
		m_DrawPrimitiveFunction = &Context::DrawLine;
		m_EndPrimitiveFunction = 0;
		break;

	case GL_LINE_STRIP:
		m_Rasterizer->PrepareLine();
		m_DrawPrimitiveFunction = &Context::DrawLineStrip;
		m_EndPrimitiveFunction = 0;
		break;

	case GL_LINE_LOOP:
		m_Rasterizer->PrepareLine();
		m_DrawPrimitiveFunction = &Context::DrawLineLoop;
		m_EndPrimitiveFunction = &Context::EndLineLoop;
		break;

	case GL_TRIANGLES:
		m_Rasterizer->PrepareTriangle();
		m_DrawPrimitiveFunction = &Context::DrawTriangle;
		m_EndPrimitiveFunction = 0;
		break;

	case GL_TRIANGLE_STRIP:
		m_Rasterizer->PrepareTriangle();
		m_DrawPrimitiveFunction = &Context::DrawTriangleStrip;
		m_EndPrimitiveFunction = 0;
		break;

	case GL_TRIANGLE_FAN:
		m_Rasterizer->PrepareTriangle();
		m_DrawPrimitiveFunction = &Context::DrawTriangleFan;
		m_EndPrimitiveFunction = 0;
		break;

	default:
		RecordError(GL_INVALID_ENUM);
		return false;
	}

	return true;
}

void Context :: End() {
	if (m_EndPrimitiveFunction)
		(this->*m_EndPrimitiveFunction)();

	m_DrawPrimitiveFunction = 0;
	m_EndPrimitiveFunction = 0;
}

void Context :: DrawArrays(GLenum mode, GLint first, GLsizei count) {

	if (count < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (!m_VertexArrayEnabled) {
		RecordError(GL_INVALID_OPERATION);
		return;
	}

	PrepareRendering();
	
	if (Begin(mode)) {
		while (count-- > 0) {
			(this->*m_DrawPrimitiveFunction)(first++);
		}

		End();
	}
}


void Context :: DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {

	if (count < 0) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (!m_VertexArrayEnabled) {
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

	PrepareRendering();

	if (type == GL_UNSIGNED_BYTE) {
		const GLubyte * ptr = reinterpret_cast<const GLubyte *>(indices);

		if (Begin(mode)) {
			while (count-- > 0) {
				(this->*m_DrawPrimitiveFunction)(*ptr++);
			}

			End();
		}
	} else if (type == GL_UNSIGNED_SHORT) {
		const GLushort * ptr = reinterpret_cast<const GLushort *>(indices);

		if (Begin(mode)) {
			while (count-- > 0) {
				(this->*m_DrawPrimitiveFunction)(*ptr++);
			}

			End();
		}
	} else {
		RecordError(GL_INVALID_ENUM);
	}
}


// --------------------------------------------------------------------------
// Load all the current coordinates from either a specific array or from
// the common settings.
//
// Parameters:
//	index		-	The array index from which any array coordinates should
//					be retrieved.
// --------------------------------------------------------------------------
void Context :: SelectArrayElement(int index, Vertex * rasterPos) {

	assert(m_VertexArray.effectivePointer);

	{
		Vec4D currentVertex;

		m_VertexArray.FetchValues(index, currentVertex.getArray());
		m_ModelViewMatrixStack.CurrentMatrix().Multiply(currentVertex, rasterPos->m_EyeCoords);
		rasterPos->m_ClipCoords = m_ProjectionMatrixStack.CurrentMatrix() * rasterPos->m_EyeCoords;
	}

	if (m_NormalArray.effectivePointer) {
		Vec3D normal;

		m_NormalArray.FetchValues(index, normal.getArray());
		rasterPos->m_EyeNormal = m_InverseModelViewMatrix.Multiply3x3(normal);
	} else {
		rasterPos->m_EyeNormal = m_TransformedDefaultNormal;
	}

	if (m_ColorArray.effectivePointer) {
		m_ColorArray.FetchValues(index, rasterPos->m_FrontColor.getArray());
	} else {
		rasterPos->m_FrontColor = m_DefaultRGBA;
	}

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (m_VaryingInfo->textureBase[unit] >= 0) {
			I32 base = m_VaryingInfo->textureBase[unit];

			if (m_TexCoordArray[unit].effectivePointer) {
				Vec4D texCoords, projectedTexCoords;

				m_TexCoordArray[unit].FetchValues(index, texCoords.getArray());
			
				if (m_TextureMatrixStack[unit].CurrentMatrix().IsIdentity()) {
					projectedTexCoords = texCoords;
				} else {
					m_TextureMatrixStack[unit].CurrentMatrix().Multiply(texCoords, projectedTexCoords);
				}

				// according to Blythe & McReynolds, this should really happen
				// as part of the perspective interpolation
				projectedTexCoords.ProjectiveDivision();
				rasterPos->m_Varying[base]     = projectedTexCoords.x();
				rasterPos->m_Varying[base + 1] = projectedTexCoords.y();
			} else {
				rasterPos->m_Varying[base]     = m_DefaultTransformedTextureCoords[unit].x();
				rasterPos->m_Varying[base + 1] = m_DefaultTransformedTextureCoords[unit].y();
			}
		}
	}

	rasterPos->m_Lit = false;
}


namespace {

	inline EGL_Fixed Exp(EGL_Fixed value) {
		return EGL_FixedFromFloat(exp(EGL_FloatFromFixed(-value)));
	}

	inline EGL_Fixed Exp2(EGL_Fixed value) {
		return Exp(EGL_Mul(value, value));
	}
}


// --------------------------------------------------------------------------
// Calculate the fog density for a vertex at the given distance
// --------------------------------------------------------------------------
EGL_Fixed Context :: FogDensity(EGL_Fixed eyeDistance) const {

	switch (m_FogMode) {
		default:
		case FogLinear:
			return EGL_CLAMP(EGL_Mul((m_FogEnd - eyeDistance) >> m_FogGradientShift, m_FogGradient) + 128, 0, EGL_ONE);

		case FogModeExp:
			return EGL_CLAMP(Exp(EGL_Mul(m_FogDensity, eyeDistance)) + 128, 0, EGL_ONE);

		case FogModeExp2:
			return EGL_CLAMP(Exp2(EGL_Mul(m_FogDensity, eyeDistance)) + 128, 0, EGL_ONE);
	}

}


void Context :: PrepareArray(VertexArray & array, bool enabled, bool isColor) {

	array.effectivePointer = 0;

	if (enabled) {
		if (array.boundBuffer) {
			if (m_Buffers.IsObject(array.boundBuffer)) {
				U8 * bufferBase =
					static_cast<U8 *>(m_Buffers.GetObject(array.boundBuffer)->GetData());

				if (!bufferBase) {
					return;
				}

				size_t offset = static_cast<const U8 *>(array.pointer) - static_cast<const U8 *>(0);
				array.effectivePointer = bufferBase + offset;
			}
		} else {
			array.effectivePointer = array.pointer;
		}
	}

	array.PrepareFetchValues(isColor);
}


void Context :: PrepareRendering() {

	if (m_LightingEnabled) {
		if (m_ColorMaterialEnabled) {
			if (m_TwoSidedLightning) {
                m_GeometryFunction = &Context::LightVertexTwoSidedTrack;
			} else {
				m_GeometryFunction = &Context::LightVertexOneSidedTrack;
			}
		} else {
			if (m_TwoSidedLightning) {
				m_GeometryFunction = &Context::LightVertexTwoSidedNoTrack;
			} else {
				m_GeometryFunction = &Context::LightVertexOneSidedNoTrack;
			}
		}
	} else {
		m_GeometryFunction = &Context::LightVertexNoLight;
	}

	PrepareArray(m_VertexArray,   m_VertexArrayEnabled);
	PrepareArray(m_NormalArray,	  m_NormalArrayEnabled);
	PrepareArray(m_ColorArray,    m_ColorArrayEnabled, true);

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		Vec4D transformedTexCoords;

		PrepareArray(m_TexCoordArray[unit], m_TexCoordArrayEnabled[unit]);
		m_TextureMatrixStack[unit].CurrentMatrix().Multiply(m_DefaultTextureCoords[m_ActiveTexture], 
															m_DefaultTransformedTextureCoords[unit]);
		// according to Blythe & McReynolds, this should really happen
		// as part of the perspective interpolation
		m_DefaultTransformedTextureCoords[unit].ProjectiveDivision();
	}

	PrepareArray(m_PointSizeArray,m_PointSizeArrayEnabled);

	m_TransformedDefaultNormal = m_InverseModelViewMatrix.Multiply3x3(m_DefaultNormal);
}


void Context :: ClipCoordsToWindowCoords(Vertex & pos) {

	// perform depth division
	EGL_Fixed x = pos.m_ClipCoords.x();
	EGL_Fixed y = pos.m_ClipCoords.y();
	EGL_Fixed z = pos.m_ClipCoords.z();
	EGL_Fixed w = pos.m_ClipCoords.w();

	// fix possible rounding problems
	if (x < -w)	x = -w;
	if (x > w)	x = w;
	if (y < -w)	y = -w;
	if (y > w)	y = w;
	if (z < -w)	z = -w;
	if (z > w)	z = w;

	if ((w >> 24) && (w >> 24) + 1) {
		// keep this value around for perspective-correct texturing
		EGL_Fixed invDenominator = EGL_Inverse(w >> 8);

		// Scale 1/Z by 2^4 to avoid rounding problems during prespective correct
		// interpolation
		// See book by LaMothe for more detailed discussion on this
		pos.m_WindowCoords.invW = invDenominator << 4;

		pos.m_WindowCoords.x = 
			EGL_Mul(EGL_Mul(x >> 8, invDenominator), m_ViewportScale.x()) + m_ViewportOrigin.x();

		pos.m_WindowCoords.y = 
			EGL_Mul(EGL_Mul(y >> 8, invDenominator), m_ViewportScale.y()) + m_ViewportOrigin.y();

		pos.m_WindowCoords.depth = 
			EGL_CLAMP(EGL_Mul(z >> 8, EGL_Mul(m_DepthRangeFactor, invDenominator))  + m_DepthRangeBase, 0, 0xffff);

	} else {
		// keep this value around for perspective-correct texturing
		EGL_Fixed invDenominator = w ? EGL_Inverse(w) : 0;

		// Scale 1/Z by 2^12 to avoid rounding problems during prespective correct
		// interpolation
		pos.m_WindowCoords.invW = invDenominator << 12;

		pos.m_WindowCoords.x = 
			EGL_Mul(EGL_Mul(x, invDenominator), m_ViewportScale.x()) + m_ViewportOrigin.x();
		pos.m_WindowCoords.x = ((pos.m_WindowCoords.x + 0x800) & ~0xfff);
		pos.m_WindowCoords.y = 
			EGL_Mul(EGL_Mul(y, invDenominator), m_ViewportScale.y()) + m_ViewportOrigin.y();
		pos.m_WindowCoords.y = ((pos.m_WindowCoords.y + 0x800) & ~0xfff);
		pos.m_WindowCoords.depth = 
			EGL_CLAMP(EGL_Mul(EGL_Mul(z, invDenominator), m_DepthRangeFactor)  + m_DepthRangeBase, 0, 0xffff);

	}

}

void Context :: GetClipPlanex(GLenum plane, GLfixed eqn[4]) {
	if (plane < GL_CLIP_PLANE0 || plane >= GL_CLIP_PLANE0 + NUM_CLIP_PLANES) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	size_t index = plane - GL_CLIP_PLANE0;
	eqn[0] = m_ClipPlanes[index].x();
	eqn[1] = m_ClipPlanes[index].y();
	eqn[2] = m_ClipPlanes[index].z();
	eqn[3] = m_ClipPlanes[index].w();
}

void Context :: ClipPlanex(GLenum plane, const GLfixed *equation) {
	
	if (plane < GL_CLIP_PLANE0 || plane >= GL_CLIP_PLANE0 + NUM_CLIP_PLANES) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	size_t index = plane - GL_CLIP_PLANE0;
	m_ClipPlanes[index] = m_FullInverseModelViewMatrix.Transpose() * Vec4D(equation);
}
