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

	if (type != GL_UNSIGNED_BYTE && type != GL_FIXED && type != GL_FLOAT) {
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

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;
		}
	}

	m_ColorArray.pointer = pointer;
	m_ColorArray.stride = stride;
	m_ColorArray.type = type;
	m_ColorArray.size = size;
}

void Context :: NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) { 

	if (type != GL_BYTE && type != GL_SHORT && type != GL_FIXED && type != GL_FLOAT) {
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

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;

		}
	}

	m_NormalArray.pointer = pointer;
	m_NormalArray.stride = stride;
	m_NormalArray.type = type;
	m_NormalArray.size = size;
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

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
			break;

		}
	}

	m_VertexArray.pointer = pointer;
	m_VertexArray.stride = stride;
	m_VertexArray.type = type;
	m_VertexArray.size = size;
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

		case GL_FLOAT:
			stride = sizeof (GLfloat) * size;
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
// Actual array rendering
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


// --------------------------------------------------------------------------
// Load all the current coordinates from either a specific array or from
// the common settings.
//
// Parameters:
//	index		-	The array index from which any array coordinates should
//					be retrieved.
// --------------------------------------------------------------------------
void Context :: SelectArrayElement(int vertexIndex, int normalIndex, int textureIndex, int colorIndex) {

	// TO DO: this whole method should be redesigned for efficient pipelining
	if (!m_VertexArrayEnabled) {
		m_CurrentVertex = Vec3D(0, 0, 0);
	} else {
		if (m_VertexArray.size == 3) {
			m_CurrentVertex = 
				Vec3D(m_VertexArray.GetValue(vertexIndex, 0),
					  m_VertexArray.GetValue(vertexIndex, 1),
					  m_VertexArray.GetValue(vertexIndex, 2));
		} else if (m_VertexArray.size == 2) {
			m_CurrentVertex = 
				Vec3D(m_VertexArray.GetValue(vertexIndex, 0),
					  m_VertexArray.GetValue(vertexIndex, 1),
					  0);
		} else {
			m_CurrentVertex = 
				Vec4D(m_VertexArray.GetValue(vertexIndex, 0),
					  m_VertexArray.GetValue(vertexIndex, 1),
					  m_VertexArray.GetValue(vertexIndex, 2),
					  m_VertexArray.GetValue(vertexIndex, 3));
		}
	}

	if (!m_NormalArrayEnabled) {
		m_CurrentNormal = m_DefaultNormal;
	} else {
		m_CurrentNormal = 
			Vec3D(m_NormalArray.GetValue(normalIndex, 0),
				  m_NormalArray.GetValue(normalIndex, 1),
				  m_NormalArray.GetValue(normalIndex, 2));
	}

	if (!m_ColorArrayEnabled) {
		m_CurrentRGBA = m_DefaultRGBA;
	} else {
		m_CurrentRGBA =
			FractionalColor(m_ColorArray.GetValue(colorIndex, 0),
							m_ColorArray.GetValue(colorIndex, 1),
							m_ColorArray.GetValue(colorIndex, 2),
							m_ColorArray.GetValue(colorIndex, 3));
	}

	if (!m_TexCoordArrayEnabled) {
		m_CurrentTextureCoords.tu = m_DefaultTextureCoords.tu;
		m_CurrentTextureCoords.tv = m_DefaultTextureCoords.tv;
	} else {
		if (m_TexCoordArray.size < 4) {
			m_CurrentTextureCoords.tu = m_TexCoordArray.GetValue(textureIndex, 0);
			m_CurrentTextureCoords.tv = m_TexCoordArray.GetValue(textureIndex, 1);
		} else {
			I32 factor = EGL_Inverse(m_TexCoordArray.GetValue(textureIndex, 3));
			m_CurrentTextureCoords.tu = EGL_Mul(m_TexCoordArray.GetValue(textureIndex, 0), factor);
			m_CurrentTextureCoords.tv = EGL_Mul(m_TexCoordArray.GetValue(textureIndex, 1), factor);
		}
	}
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

	if (eyeDistance <= m_FogStart) {
		return EGL_ONE;
	} else if (eyeDistance >= m_FogEnd) {
		return 0;
	}

	switch (m_FogMode) {
		default:
		case FogLinear:
			return EGL_Div(m_FogEnd - eyeDistance, m_FogEnd - m_FogStart);

		case FogModeExp:
			return Exp(EGL_Mul(-m_FogDensity, eyeDistance));

		case FogModeExp2:
			return Exp2(EGL_Mul(-m_FogDensity, eyeDistance));
	}

}


// --------------------------------------------------------------------------
// Perform lightning and geometry transformation on the current vertex
// and store the results in buffer for the rasterization stage of the
// pipeline.
//
// Parameters:
//	rasterPos	-	A pointer to a vertex parameter buffer for the
//					rasterization stage
// --------------------------------------------------------------------------
void Context :: CurrentValuesToRasterPos(RasterPos * rasterPos) {
	FractionalColor color;
	FractionalColor backColor;

	// apply projection matrix to eye coordinates 
	rasterPos->m_ClipCoords = m_VertexTransformation * m_CurrentVertex;
	
	if (m_LightingEnabled) {

		// apply model view matrix to vertex coordinate -> eye coordinates vertex
		Vec4D eyeCoords = m_ModelViewMatrixStack.CurrentMatrix() * m_CurrentVertex;

		EGL_Fixed eyeDistance = eyeCoords.z();

		// apply inverse of model view matrix to normals -> eye coordinates normals
		Vec3D eyeNormal = (m_InverseModelViewMatrix * m_CurrentNormal).Project();

		// TO DO: apply proper re-normalization/re-scaling of normal vector
		if (m_RescaleNormalEnabled || m_NormalizeEnabled) {
			eyeNormal.Normalize();
		}

	
		// for each light that is turned on, call into calculation
		int mask = 1;

		if (m_ColorMaterialEnabled) {
			color = m_CurrentRGBA * m_LightModelAmbient;
		} else {
			color = m_FrontMaterial.GetAmbientColor() * m_LightModelAmbient;
		}

		color += m_FrontMaterial.GetEmisiveColor();

		if (m_TwoSidedLightning) {

			backColor = color;		

			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, eyeNormal, 
						m_FrontMaterial, color, backColor);
				}
			}

			color.Clamp();
			backColor.Clamp();
			
			rasterPos->m_FrontColor = color;
			rasterPos->m_BackColor = backColor;
		} else {
			for (int index = 0; index < EGL_NUMBER_LIGHTS; ++index, mask <<= 1) {
				if (m_LightEnabled & mask) {
					m_Lights[index].AccumulateLight(eyeCoords, eyeNormal, 
						m_FrontMaterial, color);
				}
			}

			color.Clamp();
			rasterPos->m_FrontColor = color;
		}
			
		// populate fog density here...
		rasterPos->m_FogDensity = FogDensity(eyeDistance);
	} else {
		//	copy current colors to raster pos
		rasterPos->m_FrontColor = rasterPos->m_BackColor = m_CurrentRGBA;

		if (m_RasterizerState.IsEnabledFog()) {
			// populate fog density here...
			EGL_Fixed eyeDistance = m_ModelViewMatrixStack.CurrentMatrix().GetTransformedZ(m_CurrentVertex);
			rasterPos->m_FogDensity = FogDensity(eyeDistance);
		} else {
			// populate fog density here...
			rasterPos->m_FogDensity = 0;
		}
	}

	// apply texture transform to texture coordinates
	// really should have a transformation primitive of the correct dimensionality

	if (m_TextureMatrixStack.CurrentMatrix().IsIdentity()) {
		rasterPos->m_TextureCoords.tu = m_CurrentTextureCoords.tu;
		rasterPos->m_TextureCoords.tv = m_CurrentTextureCoords.tv;
	} else {
		Vec3D inCoords(m_CurrentTextureCoords.tu, m_CurrentTextureCoords.tv, 0);
		Vec4D outCoords = m_TextureMatrixStack.CurrentMatrix() * inCoords;
		rasterPos->m_TextureCoords.tu = outCoords.x();
		rasterPos->m_TextureCoords.tv = outCoords.y();
	}

}
