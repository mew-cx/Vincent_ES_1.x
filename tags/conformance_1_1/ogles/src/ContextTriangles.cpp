// ==========================================================================
//
// triangles.cpp	Rendering Context Class for 3D Rendering Library
//
//					Rendering Operations for Triangles
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
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
#include "Context.h"
#include "fixed.h"
#include "Rasterizer.h"

using namespace EGL;


// --------------------------------------------------------------------------
// Triangles
// --------------------------------------------------------------------------


void Context :: RenderTriangles(GLint first, GLsizei count) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);

		RenderTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangles(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);

		RenderTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangles(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PrepareTriangle();

	while (count >= 3) {
		count -= 3;

		RasterPos pos0, pos1, pos2;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);

		RenderTriangle(pos0, pos1, pos2);
	}
}


// --------------------------------------------------------------------------
// Triangle Strips
// --------------------------------------------------------------------------


void Context :: RenderTriangleStrip(GLint first, GLsizei count) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}

	if (count >= 2) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);
	}

	if (count >= 3) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);
	}

	if (count >= 4) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);
	}

	if (count >= 5) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);
	}

}


void Context :: RenderTriangleStrip(GLsizei count, const GLubyte * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}

	if (count >= 2) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);
	}

	if (count >= 3) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);
	}

	if (count >= 4) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);
	}

	if (count >= 5) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);
	}

}


void Context :: RenderTriangleStrip(GLsizei count, const GLushort * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 6) {
		count -= 6;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}

	if (count >= 2) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos2, pos1, pos0);
	}

	if (count >= 3) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos2, pos0, pos1);
	}

	if (count >= 4) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos1, pos0, pos2);
	}

	if (count >= 5) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderTriangle(pos1, pos2, pos0);
	}

}


// --------------------------------------------------------------------------
// Triangle Fans
// --------------------------------------------------------------------------


void Context :: RenderTriangleFan(GLint first, GLsizei count) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(first++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangleFan(GLsizei count, const GLubyte * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}
}


void Context :: RenderTriangleFan(GLsizei count, const GLushort * indices) {

	if (count < 3) {
		return;
	}

	m_Rasterizer->PrepareTriangle();

	RasterPos pos0, pos1, pos2;

	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos0);
	SelectArrayElement(*indices++);
	CurrentValuesToRasterPos(&pos1);

	count -= 2;

	while (count >= 2) {
		count -= 2;

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);

		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);
		RenderTriangle(pos0, pos2, pos1);
	}

	if (count >= 1) {
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos2);
		RenderTriangle(pos0, pos1, pos2);
	}
}


namespace {

	inline GLfloat Interpolate(GLfloat x0f, GLfloat x1f, GLfloat coeff) {
		return x1f + (x0f - x1f) * coeff;
	}

	inline EGL_Fixed Interpolate(EGL_Fixed x0, EGL_Fixed x1, GLfloat coeff) {
		return EGL_FixedFromFloat(Interpolate(EGL_FloatFromFixed(x0), EGL_FloatFromFixed(x1), coeff));
	}

	inline void Interpolate(RasterPos& result, const RasterPos& dst, const RasterPos& src, GLfloat coeff) {
		result.m_ClipCoords.setX(Interpolate(dst.m_ClipCoords.x(), src.m_ClipCoords.x(), coeff));
		result.m_ClipCoords.setY(Interpolate(dst.m_ClipCoords.y(), src.m_ClipCoords.y(), coeff));
		result.m_ClipCoords.setZ(Interpolate(dst.m_ClipCoords.z(), src.m_ClipCoords.z(), coeff));
		result.m_ClipCoords.setW(Interpolate(dst.m_ClipCoords.w(), src.m_ClipCoords.w(), coeff));
		result.m_Color.r = Interpolate(dst.m_Color.r, src.m_Color.r, coeff);
		result.m_Color.g = Interpolate(dst.m_Color.g, src.m_Color.g, coeff);
		result.m_Color.b = Interpolate(dst.m_Color.b, src.m_Color.b, coeff);
		result.m_Color.a = Interpolate(dst.m_Color.a, src.m_Color.a, coeff);

		for (size_t index = 0; index < EGL_NUM_TEXTURE_UNITS; ++index) {
			result.m_TextureCoords[index].tu = Interpolate(dst.m_TextureCoords[index].tu, src.m_TextureCoords[index].tu, coeff);
			result.m_TextureCoords[index].tv = Interpolate(dst.m_TextureCoords[index].tv, src.m_TextureCoords[index].tv, coeff);
		}

		result.m_FogDensity = Interpolate(dst.m_FogDensity, src.m_FogDensity, coeff);
	}

	inline void InterpolateWithEye(RasterPos& result, const RasterPos& dst, const RasterPos& src, GLfloat coeff) {
		result.m_EyeCoords.setX(Interpolate(dst.m_EyeCoords.x(), src.m_EyeCoords.x(), coeff));
		result.m_EyeCoords.setY(Interpolate(dst.m_EyeCoords.y(), src.m_EyeCoords.y(), coeff));
		result.m_EyeCoords.setZ(Interpolate(dst.m_EyeCoords.z(), src.m_EyeCoords.z(), coeff));
		result.m_EyeCoords.setW(Interpolate(dst.m_EyeCoords.w(), src.m_EyeCoords.w(), coeff));

		Interpolate(result, dst, src, coeff);
	}

	inline size_t ClipLow(RasterPos * input[], size_t inputCount, RasterPos * output[], RasterPos *& nextTemporary, size_t coord) {

		if (inputCount < 3) {
			return 0;
		}

		RasterPos * previous = input[inputCount - 1];
		RasterPos * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			if (current->m_ClipCoords[coord] >= -current->m_ClipCoords.w()) {

				if (previous->m_ClipCoords[coord] >= -previous->m_ClipCoords.w()) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
					//previous = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
					
					GLfloat c_x = current->m_ClipCoords[coord];
					GLfloat c_w = current->m_ClipCoords.w();
					GLfloat p_x = previous->m_ClipCoords[coord];
					GLfloat p_w = previous->m_ClipCoords.w();
					GLfloat num = p_w + p_x; 
					GLfloat denom = (p_w + p_x) - (c_w + c_x);

					Interpolate(newVertex, *current, *previous, num / denom);
					newVertex.m_ClipCoords[coord] = -newVertex.m_ClipCoords.w();

					output[resultCount++] = current;
					//previous = current;
				}
			} else {
				if (previous->m_ClipCoords[coord] >= -previous->m_ClipCoords.w()) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
					
					GLfloat c_x = current->m_ClipCoords[coord];
					GLfloat c_w = current->m_ClipCoords.w();
					GLfloat p_x = previous->m_ClipCoords[coord];
					GLfloat p_w = previous->m_ClipCoords.w();
					GLfloat num = p_w + p_x; 
					GLfloat denom = (p_w + p_x) - (c_w + c_x);
					
					Interpolate(newVertex, *current, *previous, num / denom);
					newVertex.m_ClipCoords[coord] = -newVertex.m_ClipCoords.w();

					//previous = current;
				}
			}

			previous = current;
		}

		return resultCount;

	}

	inline size_t ClipHigh(RasterPos * input[], size_t inputCount, RasterPos * output[], RasterPos *& nextTemporary, size_t coord) {

		if (inputCount < 3) {
			return 0;
		}

		RasterPos * previous = input[inputCount - 1];
		RasterPos * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			if (current->m_ClipCoords[coord] < current->m_ClipCoords.w()) {

				if (previous->m_ClipCoords[coord] < previous->m_ClipCoords.w()) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
					
					GLfloat c_x = current->m_ClipCoords[coord];
					GLfloat c_w = current->m_ClipCoords.w();
					GLfloat p_x = previous->m_ClipCoords[coord];
					GLfloat p_w = previous->m_ClipCoords.w();
					GLfloat num = p_w - p_x; 
					GLfloat denom = (p_w - p_x) - (c_w - c_x);
					
					Interpolate(newVertex, *current, *previous, num / denom); 
					newVertex.m_ClipCoords[coord] = newVertex.m_ClipCoords.w();

					output[resultCount++] = current;
				}
			} else {
				if (previous->m_ClipCoords[coord] < previous->m_ClipCoords.w()) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
					
					GLfloat c_x = current->m_ClipCoords[coord];
					GLfloat c_w = current->m_ClipCoords.w();
					GLfloat p_x = previous->m_ClipCoords[coord];
					GLfloat p_w = previous->m_ClipCoords.w();
					GLfloat num = p_w - p_x; 
					GLfloat denom = (p_w - p_x) - (c_w - c_x);
					
					Interpolate(newVertex, *current, *previous, num / denom); 
					newVertex.m_ClipCoords[coord] = newVertex.m_ClipCoords.w();
				}
			}

			previous = current;
		}

		return resultCount;

	}

	size_t ClipUser(const Vec4f& plane, RasterPos * input[], size_t inputCount, RasterPos * output[], RasterPos *& nextTemporary) {
		if (inputCount < 3) {
			return 0;
		}

		RasterPos * previous = input[inputCount - 1];
		RasterPos * current;
		int resultCount = 0;

		for (size_t index = 0; index < inputCount; ++index) {

			current = input[index];

			GLfloat c = Vec4f(current->m_EyeCoords) * plane;
			GLfloat p = Vec4f(previous->m_EyeCoords) * plane;

			if (c > 0.0f) {
				if (p >= 0.0f) {
					// line segment between previous and current is fully contained in cube
					output[resultCount++] = current;
				} else {
					// line segment between previous and current is intersected;
					// create vertex at intersection, then add current
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
										
					InterpolateWithEye(newVertex, *current, *previous, p / (p - c)); 
					output[resultCount++] = current;
				}
			} else {
				if (p > 0.0f) {
					// line segment between previous and current is intersected;
					// create vertex at intersection and add it
					RasterPos & newVertex = *nextTemporary++;
					output[resultCount++] = &newVertex;
					
					InterpolateWithEye(newVertex, *current, *previous, p / (p - c)); 
				}
			}

			previous = current;
		}

		return resultCount;
	}

}


void Context :: FrontFace(GLenum mode) { 

	switch (mode) {
		case GL_CW:
			m_ReverseFaceOrientation = true;
			break;

		case GL_CCW:
			m_ReverseFaceOrientation = false;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

void Context :: CullFace(GLenum mode) { 

	switch (mode) { 
		case GL_FRONT:
			m_CullMode = CullModeFront;
			break;

		case GL_BACK:
			m_CullMode = CullModeBack;
			break;

		case GL_FRONT_AND_BACK:
			m_CullMode = CullModeBackAndFront;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
			break;
	}
}

namespace {
	inline GLfloat Det3x3(GLfloat x0, GLfloat x1, GLfloat x2,
						GLfloat y0, GLfloat y1, GLfloat y2,
						GLfloat z0, GLfloat z1, GLfloat z2) {
		return 
			  x0 * y1 * z2
			+ x1 * y2 * z0
			+ x2 * y0 * z1
			- x0 * y2 * z1
			- x1 * y0 * z2
			- x2 * y1 * z0;
	}
}

inline bool Context :: IsCulled(RasterPos& a, RasterPos& b, RasterPos& c) {

	GLfloat x0 = a.m_ClipCoords.w();
	GLfloat x1 = a.m_ClipCoords.x();
	GLfloat x2 = a.m_ClipCoords.y();
								
	GLfloat y0 = b.m_ClipCoords.w();
	GLfloat y1 = b.m_ClipCoords.x();
	GLfloat y2 = b.m_ClipCoords.y();
								
	GLfloat z0 = c.m_ClipCoords.w();
	GLfloat z1 = c.m_ClipCoords.x();
	GLfloat z2 = c.m_ClipCoords.y();

	GLfloat sign = 
		Det3x3(a.m_ClipCoords.w(), a.m_ClipCoords.x(), a.m_ClipCoords.y(),
			   b.m_ClipCoords.w(), b.m_ClipCoords.x(), b.m_ClipCoords.y(),
			   c.m_ClipCoords.w(), c.m_ClipCoords.x(), c.m_ClipCoords.y());

	switch (m_CullMode) {
		case CullModeBack:
			return (sign < 0.0f) ^ m_ReverseFaceOrientation;

		case CullModeFront:
			return (sign > 0.0f) ^ m_ReverseFaceOrientation;

		default:
		case CullModeBackAndFront:
			return true;
	}
}


void Context :: RenderTriangle(RasterPos& a, RasterPos& b, RasterPos& c) {

	bool culled = IsCulled(a, b, c);

	if (m_CullFaceEnabled) {
		if (culled) {
			return;
		}
	} 
	
	if (m_RasterizerState.GetShadeModel() == RasterizerState::ShadeModelSmooth) {
		if (m_TwoSidedLightning && culled) {
			a.m_Color = a.m_BackColor;
			b.m_Color = b.m_BackColor;
			c.m_Color = c.m_BackColor;
		} else {
			a.m_Color = a.m_FrontColor;
			b.m_Color = b.m_FrontColor;
			c.m_Color = c.m_FrontColor;
		}
	} else {
		if (m_TwoSidedLightning && culled) {
			a.m_Color =  b.m_Color = c.m_Color = c.m_BackColor;
		} else {
			a.m_Color = b.m_Color = c.m_Color = c.m_FrontColor;
		}
	}

	RasterPos * array1[16];
	array1[0] = &a;
	array1[1] = &b;
	array1[2] = &c;
	RasterPos * array2[16];
	RasterPos * tempVertices = m_Temporary;

	size_t numVertices = 3;

	if (m_ClipPlaneEnabled) {
		for (size_t index = 0, mask = 1; index < NUM_CLIP_PLANES; ++index, mask <<= 1) {
			if (m_ClipPlaneEnabled & mask) {
				numVertices = ClipUser(m_ClipPlanes[index], array1, numVertices, array2, tempVertices);

				if (!numVertices) {
					return;
				}

				for (size_t idx = 0; idx < numVertices; ++idx) {
					array1[idx] = array2[idx];
				}
			}
		}
	}

	for (size_t coord = 0; coord < 3; ++coord) {
		numVertices = ClipLow(array1, numVertices, array2, tempVertices, coord);
		numVertices = ClipHigh(array2, numVertices, array1, tempVertices, coord);
	}

	if (numVertices >= 3) {
		ClipCoordsToWindowCoords(*array1[0]);
		ClipCoordsToWindowCoords(*array1[1]);

		for (size_t index = 2; index < numVertices; ++index) {
			ClipCoordsToWindowCoords(*array1[index]);
			m_Rasterizer->RasterTriangle(*array1[0], *array1[index - 1], *array1[index]);
		}
	}
}



