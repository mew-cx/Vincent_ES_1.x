// ==========================================================================
//
// render.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Rendering Operations
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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
		m_TexCoordArrayEnabled = value;
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

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size != 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (pointer == 0) {
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

		}
	}

	m_ColorArray.pointer = pointer;
	m_ColorArray.stride = stride;
	m_ColorArray.type = type;
	m_ColorArray.size = size;
}

void Context :: NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) { 

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	GLsizei size = 3;

	if (pointer == 0) {
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

		}
	}

	m_NormalArray.pointer = pointer;
	m_NormalArray.stride = stride;
	m_NormalArray.type = type;
	m_NormalArray.size = size;
}

void Context :: VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { 

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size < 2 || size > 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (pointer == 0) {
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

		}
	}

	m_VertexArray.pointer = pointer;
	m_VertexArray.stride = stride;
	m_VertexArray.type = type;
	m_VertexArray.size = size;
}

void Context :: TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { 

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	if (size < 2 || size > 4) {
		RecordError(GL_INVALID_VALUE);
		return;
	}

	if (pointer == 0) {
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

		}
	}

	m_TexCoordArray.pointer = pointer;
	m_TexCoordArray.stride = stride;
	m_TexCoordArray.type = type;
	m_TexCoordArray.size = size;
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
	
	if (target != GL_TEXTURE0) {
		// only have one texture unit
		RecordError(GL_INVALID_VALUE);
		return;
	}

	I32 inverse = EGL_Inverse(q);
	m_DefaultTextureCoords.tu = EGL_Mul(s, inverse);
	m_DefaultTextureCoords.tv = EGL_Mul(t, inverse);
}

void Context :: Normal3x(GLfixed nx, GLfixed ny, GLfixed nz) { 
	m_DefaultNormal = Vec3D(nx, ny, nz);
}


// --------------------------------------------------------------------------
// General preparation/setup for rendering
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Actual mesh rendering
// --------------------------------------------------------------------------

void Context :: DrawArrays(GLenum mode, GLint first, GLsizei count) { 

	switch (mode) {
	case GL_POINTS:
		RenderPoints(first, count);
		break;

	case GL_LINES:
		RenderLines(first, count);
		break;

	case GL_LINE_STRIP:
		RenderLineStrip(first, count);
		break;

	case GL_LINE_LOOP:
		RenderLineLoop(first, count);
		break;

	case GL_TRIANGLES:
		RenderTriangles(first, count);
		break;

	case GL_TRIANGLE_STRIP:
		RenderTriangleStrip(first, count);
		break;

	case GL_TRIANGLE_FAN:
		RenderTriangleFan(first, count);
		break;

	default:
		RecordError(GL_INVALID_ENUM);
		return;
	}
}

void Context :: DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) { 
	
	switch (mode) {
	case GL_POINTS:
		if (type == GL_UNSIGNED_BYTE) {
			RenderPoints(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderPoints(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_LINES:
		if (type == GL_UNSIGNED_BYTE) {
			RenderLines(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderLines(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_LINE_STRIP:
		if (type == GL_UNSIGNED_BYTE) {
			RenderLineStrip(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderLineStrip(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_LINE_LOOP:
		if (type == GL_UNSIGNED_BYTE) {
			RenderLineLoop(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderLineLoop(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_TRIANGLES:
		if (type == GL_UNSIGNED_BYTE) {
			RenderTriangles(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderTriangles(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_TRIANGLE_STRIP:
		if (type == GL_UNSIGNED_BYTE) {
			RenderTriangleStrip(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderTriangleStrip(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;

	case GL_TRIANGLE_FAN:
		if (type == GL_UNSIGNED_BYTE) {
			RenderTriangleFan(count, reinterpret_cast<const GLubyte *>(indices));
		} else if (type == GL_UNSIGNED_SHORT) {
			RenderTriangleFan(count, reinterpret_cast<const GLushort *>(indices));
		} else {
			RecordError(GL_INVALID_ENUM);
		}

		break;


	default:
		RecordError(GL_INVALID_ENUM);
		return;
	}
}

/*

General rendering sequence:

  If no vertices given, we are done.

  Transform vertices into world coordinates.
  Transform into screen coordinates.

  Transform normals if normals provided and lights enabled. If no normals are given, use the
  current default normal

  Transform texture coordinates if texture coordinates are given. If no texture coordinates are given,
  use the current default texture coordinate set

  Calculate vertex colors from arrays. If no individual colors are given, use the default color specified.
  Modulate vertex colors with lights and fog if necessary.

  Set up primitives:
	Apply culling & clipping to primitive
	for non-clipped primitive:
		perform depth division
		raster primitive



 */

#if 0

void Context :: SelectArrayElement(int index) {

	// TO DO: this whole method should be redesigned for efficient pipelining
	if (!m_VertexArrayEnabled) {
		m_CurrentVertex.x = 0;
		m_CurrentVertex.y = 0;
		m_CurrentVertex.z = 0;
	} else {
		if (m_VertexArray.size == 3) {
			m_CurrentVertex.x = m_VertexArray.GetValue(index, 0);
			m_CurrentVertex.y = m_VertexArray.GetValue(index, 1);
			m_CurrentVertex.z = m_VertexArray.GetValue(index, 2);
		} else if (m_VertexArray.size == 2) {
			m_CurrentVertex.x = m_VertexArray.GetValue(index, 0);
			m_CurrentVertex.y = m_VertexArray.GetValue(index, 1);
			m_CurrentVertex.z = 0;
		} else {
			I32 result;
			gppInv_16_32s(m_VertexArray.GetValue(index, 3), &result);

			gppMul_16_32s(m_VertexArray.GetValue(index, 0), result, &m_CurrentVertex.x);
			gppMul_16_32s(m_VertexArray.GetValue(index, 1), result, &m_CurrentVertex.y);
			gppMul_16_32s(m_VertexArray.GetValue(index, 2), result, &m_CurrentVertex.z);
		}
	}

	if (!m_NormalArrayEnabled) {
		m_CurrentNormal.x = m_DefaultNormal.x;
		m_CurrentNormal.y = m_DefaultNormal.y;
		m_CurrentNormal.z = m_DefaultNormal.z;
	} else {
		m_CurrentNormal.x = m_NormalArray.GetValue(index, 0);
		m_CurrentNormal.y = m_NormalArray.GetValue(index, 1);
		m_CurrentNormal.z = m_NormalArray.GetValue(index, 2);
	}

	if (!m_ColorArrayEnabled) {
		m_CurrentRGBA.r = m_DefaultRGBA.r;
		m_CurrentRGBA.g = m_DefaultRGBA.g;
		m_CurrentRGBA.b = m_DefaultRGBA.b;
		m_CurrentRGBA.a = m_DefaultRGBA.a;
	} else {
		m_CurrentRGBA.r = m_ColorArray.GetValue(index, 0);
		m_CurrentRGBA.g = m_ColorArray.GetValue(index, 1);
		m_CurrentRGBA.b = m_ColorArray.GetValue(index, 2);
		m_CurrentRGBA.a = m_ColorArray.GetValue(index, 3);
	}

	if (!m_TexCoordArrayEnabled) {
		m_CurrentTextureCoords.tu = m_DefaultTextureCoords.tu;
		m_CurrentTextureCoords.tv = m_DefaultTextureCoords.tv;
	} else {
		if (m_TexCoordArray.size < 4) {
			m_CurrentTextureCoords.tu = m_TexCoordArray.GetValue(index, 0);
			m_CurrentTextureCoords.tv = m_TexCoordArray.GetValue(index, 1);
		} else {
			I32 result;
			gppInv_16_32s(m_TexCoordArray.GetValue(index, 3), &result);

			gppMul_16_32s(m_TexCoordArray.GetValue(index, 0), result, reinterpret_cast<I32 *>(&m_CurrentTextureCoords.tu));
			gppMul_16_32s(m_TexCoordArray.GetValue(index, 1), result, reinterpret_cast<I32 *>(&m_CurrentTextureCoords.tv));
		}
	}
}


void Context :: CurrentValuesToRasterPos(EGL_RASTER_POS * rasterPos) {

	GPP_VEC4D	eyeCoords;
	GPP_VEC4D	eyeNormal;
	GPP_VEC4D	clipCoords;

	// apply model view matrix to vertex coordinate -> eye coordinates vertex
	gppVec3DTransform_16_32s(&m_CurrentVertex, &eyeCoords, m_ModelViewMatrixStack.CurrentMatrix());

	// apply inverse of model view matrix to normals -> eye coordinates normals
	gppVec3DTransform_16_32s(&m_CurrentNormal, &eyeNormal, m_InverseModelViewMatrix);

	// TO DO: apply proper re-normalization/re-scaling of normal vector
	if (m_RescaleNormalEnabled || m_NormalizeEnabled) {
		EGL_NORMALIZE(*reinterpret_cast<GPP_VEC3D *>(&eyeNormal));
	}

	// apply projection matrix to eye coordinates 
	gppVec3DTransform_16_32s(reinterpret_cast<GPP_VEC3D *>(&eyeCoords), &clipCoords, m_ProjectionMatrixStack.CurrentMatrix());

	// perform depth division
	I32 invDenominator;
	gppInv_16_32s(clipCoords.w, &invDenominator);
	gppVec3DScale_16_32s(reinterpret_cast<GPP_VEC3D *>(&clipCoords), invDenominator);

	// apply viewport transform to clip coordinates -> window coordinates
	gppMul_16_32s(clipCoords.x, m_ViewportScale.x, &clipCoords.x);
	gppMul_16_32s(clipCoords.y, m_ViewportScale.y, &clipCoords.y);
	gppMul_16_32s(clipCoords.z, m_ViewportScale.z, &clipCoords.z);
	gppVec3DAdd_n_32s(reinterpret_cast<GPP_VEC3D *>(&clipCoords), &m_ViewportOrigin, 
		reinterpret_cast<GPP_VEC3D *>(&rasterPos->m_WindowsCoords));
	rasterPos->m_WindowsCoords.w = clipCoords.w;

	if (m_LightingEnabled) {
		// for each light that is turned on, call into calculation
		int mask = 1;

		if (m_ColorMaterialEnabled) {
			EGL_COLOR_PRODUCT(m_CurrentRGBA, m_LightModelAmbient,
				rasterPos->m_Color);
			EGL_COLOR_ACCUMULATE(m_FrontMaterial.GetEmisiveColor(), rasterPos->m_Color);

			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, *reinterpret_cast<GPP_VEC3D*>(&eyeNormal),
						m_CurrentRGBA, rasterPos->m_Color);
				}
			}

			rasterPos->m_Color.a = m_CurrentRGBA.a;
		} else {
			EGL_COLOR_PRODUCT(m_FrontMaterial.GetAmbientColor(), m_LightModelAmbient,
				rasterPos->m_Color);
			EGL_COLOR_ACCUMULATE(m_FrontMaterial.GetEmisiveColor(), rasterPos->m_Color);

			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, *reinterpret_cast<GPP_VEC3D*>(&eyeNormal),
						rasterPos->m_Color);
				}
			}

			rasterPos->m_Color.a = m_FrontMaterial.GetDiffuseColor().a;
		}

		EGL_COLOR_CLAMP(rasterPos->m_Color);
	} else {
		//	copy current colors to raster pos
		rasterPos->m_Color = m_CurrentRGBA;
	}

	// adjust depth to rendering surface representation
	rasterPos->m_WindowsCoords.z = m_DrawSurface->DepthBitsFromDepth(rasterPos->m_WindowsCoords.z);

	// adjust color to range [0.. 256) in fixed point representation
	rasterPos->m_Color.r = ((rasterPos->m_Color.r >> 8) * 0xFFFF);
	rasterPos->m_Color.g = ((rasterPos->m_Color.g >> 8) * 0xFFFF);
	rasterPos->m_Color.b = ((rasterPos->m_Color.b >> 8) * 0xFFFF);
	rasterPos->m_Color.a = ((rasterPos->m_Color.a >> 8) * 0xFFFF);

	// apply texture transform to texture coordinates
	// really should have a transformation primitive of the correct dimensionality
	GPP_VEC3D inCoords;
	GPP_VEC4D outCoords;

	inCoords.x = m_CurrentTextureCoords.tu;
	inCoords.y = m_CurrentTextureCoords.tv;
	inCoords.z = 0;

	gppVec3DTransform_16_32s(&inCoords, &outCoords, m_TextureMatrixStack.CurrentMatrix());
	rasterPos->m_TextureCoords.tu = outCoords.x;
	rasterPos->m_TextureCoords.tv = outCoords.y;
}

#endif
