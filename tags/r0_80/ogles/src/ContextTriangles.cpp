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

	inline EGL_Fixed Interpolate(EGL_Fixed x0, EGL_Fixed x1, EGL_Fixed num, EGL_Fixed denom) {
		return x1 + (EGL_Fixed)((((I64)(x0-x1))*num)/denom);
	}

	inline void Interpolate(RasterPos& result, const RasterPos& dst, const RasterPos& src, EGL_Fixed num, EGL_Fixed denom) {
		result.m_ClipCoords.setX(Interpolate(dst.m_ClipCoords.x(), src.m_ClipCoords.x(), num, denom));
		result.m_ClipCoords.setY(Interpolate(dst.m_ClipCoords.y(), src.m_ClipCoords.y(), num, denom));
		result.m_ClipCoords.setZ(Interpolate(dst.m_ClipCoords.z(), src.m_ClipCoords.z(), num, denom));
		result.m_ClipCoords.setW(Interpolate(dst.m_ClipCoords.w(), src.m_ClipCoords.w(), num, denom));
		result.m_Color.r = Interpolate(dst.m_Color.r, src.m_Color.r, num, denom);
		result.m_Color.g = Interpolate(dst.m_Color.g, src.m_Color.g, num, denom);
		result.m_Color.b = Interpolate(dst.m_Color.b, src.m_Color.b, num, denom);
		result.m_Color.a = Interpolate(dst.m_Color.a, src.m_Color.a, num, denom);
		result.m_TextureCoords.tu = Interpolate(dst.m_TextureCoords.tu, src.m_TextureCoords.tu, num, denom);
		result.m_TextureCoords.tv = Interpolate(dst.m_TextureCoords.tv, src.m_TextureCoords.tv, num, denom);
		result.m_FogDensity = Interpolate(dst.m_FogDensity, src.m_FogDensity, num, denom);
	}

	inline int ClipXLow(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setX
#		define COORDINATE x()
#		include "TriangleClipperLow.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}

	inline int ClipXHigh(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setX
#		define COORDINATE x()
#		include "TriangleClipperHigh.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}

	inline int ClipYLow(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setY
#		define COORDINATE y()
#		include "TriangleClipperLow.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}

	inline int ClipYHigh(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setY
#		define COORDINATE y()
#		include "TriangleClipperHigh.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}

	inline int ClipZLow(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setZ
#		define COORDINATE z()
#		include "TriangleClipperLow.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}

	inline int ClipZHigh(RasterPos * input[], int inputCount, RasterPos * output[], RasterPos *& nextTemporary) {

#		define SET_COORDINATE setZ
#		define COORDINATE z()
#		include "TriangleClipperHigh.inc"
#		undef COORDINATE
#		undef SET_COORDINATE

	}


	inline EGL_Fixed Det3x3(EGL_Fixed x0, EGL_Fixed x1, EGL_Fixed x2,
							EGL_Fixed y0, EGL_Fixed y1, EGL_Fixed y2,
							EGL_Fixed z0, EGL_Fixed z1, EGL_Fixed z2) {

		// TODO: Need to replace those float calculations by pseudo-floats (i.e. not fully normalized)
		float result = 
			+EGL_FloatFromFixed(x0) * EGL_FloatFromFixed(y1) * EGL_FloatFromFixed(z2)
			+EGL_FloatFromFixed(x1) * EGL_FloatFromFixed(y2) * EGL_FloatFromFixed(z0)
			+EGL_FloatFromFixed(x2) * EGL_FloatFromFixed(y0) * EGL_FloatFromFixed(z1)
			-EGL_FloatFromFixed(x0) * EGL_FloatFromFixed(y2) * EGL_FloatFromFixed(z1)
			-EGL_FloatFromFixed(x1) * EGL_FloatFromFixed(y0) * EGL_FloatFromFixed(z2)
			-EGL_FloatFromFixed(x2) * EGL_FloatFromFixed(y1) * EGL_FloatFromFixed(z0);

		if (result > 0) 
			return 1;
		else if (result < 0)
			return -1;
		else 
			return 0;
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


inline bool Context :: IsCulled(RasterPos& a, RasterPos& b, RasterPos& c) {

	EGL_Fixed sign = 
		Det3x3(a.m_ClipCoords.w(), a.m_ClipCoords.x(), a.m_ClipCoords.y(),
			   b.m_ClipCoords.w(), b.m_ClipCoords.x(), b.m_ClipCoords.y(),
			   c.m_ClipCoords.w(), c.m_ClipCoords.x(), c.m_ClipCoords.y());

	switch (m_CullMode) {
		case CullModeBack:
			return (sign < 0) ^ m_ReverseFaceOrientation;

		case CullModeFront:
			return (sign > 0) ^ m_ReverseFaceOrientation;

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

	int numVertices = 3;

	numVertices = ClipXLow(array1, numVertices, array2, tempVertices);
	numVertices = ClipXHigh(array2, numVertices, array1, tempVertices);
	numVertices = ClipYLow(array1, numVertices, array2, tempVertices);
	numVertices = ClipYHigh(array2, numVertices, array1, tempVertices);
	numVertices = ClipZLow(array1, numVertices, array2, tempVertices);
	numVertices = ClipZHigh(array2, numVertices, array1, tempVertices);

	if (numVertices >= 3) {
		ClipCoordsToWindowCoords(*array1[0]);
		ClipCoordsToWindowCoords(*array1[1]);

		for (int index = 2; index < numVertices; ++index) {
			ClipCoordsToWindowCoords(*array1[index]);
			m_Rasterizer->RasterTriangle(*array1[0], *array1[index - 1], *array1[index]);
		}
	}
}


