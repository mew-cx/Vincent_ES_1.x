// ==========================================================================
//
// render.cpp	Rendering Context Class for OpenGL (R) ES Implementation
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

void Context :: SelectArrayElement(int index) {

	// TO DO: this whole method should be redesigned for efficient pipelining
	if (!m_VertexArrayEnabled) {
		m_CurrentVertex = Vec3D(0, 0, 0);
	} else {
		if (m_VertexArray.size == 3) {
			m_CurrentVertex = 
				Vec3D(m_VertexArray.GetValue(index, 0),
					  m_VertexArray.GetValue(index, 1),
					  m_VertexArray.GetValue(index, 2));
		} else if (m_VertexArray.size == 2) {
			m_CurrentVertex = 
				Vec3D(m_VertexArray.GetValue(index, 0),
					  m_VertexArray.GetValue(index, 1),
					  0);
		} else {
			m_CurrentVertex = 
				Vec4D(m_VertexArray.GetValue(index, 0),
					  m_VertexArray.GetValue(index, 1),
					  m_VertexArray.GetValue(index, 2),
					  m_VertexArray.GetValue(index, 3));
		}
	}

	if (!m_NormalArrayEnabled) {
		m_CurrentNormal = m_DefaultNormal;
	} else {
		m_CurrentNormal = 
			Vec3D(m_NormalArray.GetValue(index, 0),
				  m_NormalArray.GetValue(index, 1),
				  m_NormalArray.GetValue(index, 2));
	}

	if (!m_ColorArrayEnabled) {
		m_CurrentRGBA = m_DefaultRGBA;
	} else {
		m_CurrentRGBA =
			FractionalColor(m_ColorArray.GetValue(index, 0),
							m_ColorArray.GetValue(index, 1),
							m_ColorArray.GetValue(index, 2),
							m_ColorArray.GetValue(index, 3));
	}

	if (!m_TexCoordArrayEnabled) {
		m_CurrentTextureCoords.tu = m_DefaultTextureCoords.tu;
		m_CurrentTextureCoords.tv = m_DefaultTextureCoords.tv;
	} else {
		if (m_TexCoordArray.size < 4) {
			m_CurrentTextureCoords.tu = m_TexCoordArray.GetValue(index, 0);
			m_CurrentTextureCoords.tv = m_TexCoordArray.GetValue(index, 1);
		} else {
			I32 factor = EGL_Inverse(m_TexCoordArray.GetValue(index, 3));
			m_CurrentTextureCoords.tu = EGL_Mul(m_TexCoordArray.GetValue(index, 0), factor);
			m_CurrentTextureCoords.tv = EGL_Mul(m_TexCoordArray.GetValue(index, 1), factor);
		}
	}
}


void Context :: CurrentValuesToRasterPos(RasterPos * rasterPos) {

	// apply model view matrix to vertex coordinate -> eye coordinates vertex
	Vec4D eyeCoords = m_ModelViewMatrixStack.CurrentMatrix() * m_CurrentVertex;

	// apply inverse of model view matrix to normals -> eye coordinates normals
	Vec3D eyeNormal = (m_InverseModelViewMatrix * m_CurrentNormal).Project();

	// TO DO: apply proper re-normalization/re-scaling of normal vector
	if (m_RescaleNormalEnabled || m_NormalizeEnabled) {
		eyeNormal.Normalize();
	}

	// apply projection matrix to eye coordinates 
	Vec4D clipCoords = m_ProjectionMatrixStack.CurrentMatrix() * eyeCoords;

	// perform depth division
	I32 invDenominator = EGL_Inverse(clipCoords.w());
	Vec3D viewPortScale = m_ViewportScale * invDenominator;

	rasterPos->m_WindowCoords.x = EGL_Mul(clipCoords.x(), viewPortScale.x()) + m_ViewportOrigin.x();
	rasterPos->m_WindowCoords.y = EGL_Mul(clipCoords.y(), viewPortScale.y()) + m_ViewportOrigin.y();
	rasterPos->m_WindowCoords.w = clipCoords.w();

	if (m_LightingEnabled) {
		// for each light that is turned on, call into calculation
		int mask = 1;

		if (m_ColorMaterialEnabled) {
			FractionalColor color = m_CurrentRGBA * m_LightModelAmbient;
			color += m_FrontMaterial.GetEmisiveColor();

			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, eyeNormal,
						m_CurrentRGBA, color);
				}
			}

			color.Clamp();
			rasterPos->m_Color = color;
		} else {
			FractionalColor color = m_FrontMaterial.GetAmbientColor() * m_LightModelAmbient;
			color += m_FrontMaterial.GetEmisiveColor();

			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, eyeNormal,
						color);
				}
			}

			color.Clamp();
			rasterPos->m_Color = color;
		}
	} else {
		//	copy current colors to raster pos
		rasterPos->m_Color = m_CurrentRGBA;
	}

	// apply texture transform to texture coordinates
	// really should have a transformation primitive of the correct dimensionality
	Vec3D inCoords(m_CurrentTextureCoords.tu, m_CurrentTextureCoords.tv, 0);
	Vec4D outCoords = m_TextureMatrixStack.CurrentMatrix() * inCoords;
	rasterPos->m_TextureCoords.tu = outCoords.x();
	rasterPos->m_TextureCoords.tv = outCoords.y();
}
