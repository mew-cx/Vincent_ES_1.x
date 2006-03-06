// ==========================================================================
//
// ContextLines.cpp	Rendering Context Class for 3D Rendering Library
//
//					Rendering Operations for Lines
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


void Context :: LineWidthx(GLfixed width) { 

	if (width <= 0) {
		RecordError(GL_INVALID_VALUE);
	} else {
		GetRasterizerState()->SetLineWidth(width);
	}
}


// --------------------------------------------------------------------------
// Lines
// --------------------------------------------------------------------------


void Context :: RenderLines(GLint first, GLsizei count) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos1);

		RenderLine(pos0, pos1);
	}
}


void Context :: RenderLines(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);

		RenderLine(pos0, pos1);
	}
}


void Context :: RenderLines(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PrepareLine();

	while (count >= 2) {
		count -= 2;

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos1);

		RenderLine(pos0, pos1);
	}
}


// --------------------------------------------------------------------------
// Line Strips
// --------------------------------------------------------------------------


void Context :: RenderLineStrip(GLint first, GLsizei count) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
		}
	}
}


void Context :: RenderLineStrip(GLsizei count, const GLubyte * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
		}
	}
}


void Context :: RenderLineStrip(GLsizei count, const GLushort * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		--count;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
		}
	}
}


// --------------------------------------------------------------------------
// Line Loops
// --------------------------------------------------------------------------


void Context :: RenderLineLoop(GLint first, GLsizei count) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(first++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
			RenderLine(pos1, start);
		} else {
			RenderLine(pos0, start);
		}
	}
}


void Context :: RenderLineLoop(GLsizei count, const GLubyte * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
			RenderLine(pos1, start);
		} else {
			RenderLine(pos0, start);
		}
	}
}


void Context :: RenderLineLoop(GLsizei count, const GLushort * indices) {

	if (count >= 2) {
		m_Rasterizer->PrepareLine();

		RasterPos pos0, pos1, start;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&start);
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderLine(start, pos0);

		count -= 2;

		while (count >= 2) {
			count -= 2;

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);

			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos0);
			RenderLine(pos1, pos0);
		}

		if (count >= 1) {
			SelectArrayElement(*indices++);
			CurrentValuesToRasterPos(&pos1);
			RenderLine(pos0, pos1);
			RenderLine(pos1, start);
		} else {
			RenderLine(pos0, start);
		}
	}
}


namespace {

	inline GLfloat Interpolate(GLfloat x0f, GLfloat x1f, GLfloat coeff) {
		GLfloat complement = 1.0f - coeff;
		return x1f * complement + x0f * coeff;
	}

	inline EGL_Fixed Interpolate(EGL_Fixed x0, EGL_Fixed x1, GLfloat coeff) {
		return EGL_FixedFromFloat(Interpolate(EGL_FloatFromFixed(x0), EGL_FloatFromFixed(x1), coeff));
	}

	inline void Interpolate(RasterPos& result, const RasterPos& dst, const RasterPos& src, GLfloat coeff, size_t numVarying) {
		result.m_ClipCoords.setX(Interpolate(dst.m_ClipCoords.x(), src.m_ClipCoords.x(), coeff));
		result.m_ClipCoords.setY(Interpolate(dst.m_ClipCoords.y(), src.m_ClipCoords.y(), coeff));
		result.m_ClipCoords.setZ(Interpolate(dst.m_ClipCoords.z(), src.m_ClipCoords.z(), coeff));
		result.m_ClipCoords.setW(Interpolate(dst.m_ClipCoords.w(), src.m_ClipCoords.w(), coeff));

		for (size_t index = 0; index < numVarying; ++index) {
			result.m_Varying[index] = Interpolate(dst.m_Varying[index], src.m_Varying[index], coeff);
		}
	}

	inline bool Clip(RasterPos*& from, RasterPos*& to, RasterPos *&tempVertices, size_t coord, size_t numVarying) {
		if (from->m_ClipCoords[coord] < -from->m_ClipCoords.w()) {
			if (to->m_ClipCoords[coord] < -to->m_ClipCoords.w()) {
				return false;
			}

			GLfloat c_x = from->m_ClipCoords[coord];
			GLfloat c_w = -from->m_ClipCoords.w();
			GLfloat p_x = to->m_ClipCoords[coord];
			GLfloat p_w = -to->m_ClipCoords.w();
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *from, *to, num / denom, numVarying);
			from = tempVertices++;

			return true;

		} else if (from->m_ClipCoords[coord] > from->m_ClipCoords.w()) {
			if (to->m_ClipCoords[coord] > to->m_ClipCoords.w()) {
				return false;
			}

			GLfloat c_x = from->m_ClipCoords[coord];
			GLfloat c_w = from->m_ClipCoords.w();
			GLfloat p_x = to->m_ClipCoords[coord];
			GLfloat p_w = to->m_ClipCoords.w();
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *from, *to, num / denom, numVarying);
			from = tempVertices++;

			return true;

		} else if (to->m_ClipCoords[coord] < -to->m_ClipCoords.w()) {

			GLfloat c_x = to->m_ClipCoords[coord];
			GLfloat c_w = -to->m_ClipCoords.w();
			GLfloat p_x = from->m_ClipCoords[coord];
			GLfloat p_w = -from->m_ClipCoords.w();
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *to, *from, num / denom, numVarying);
			to = tempVertices++;

			return true;

		} else if (to->m_ClipCoords[coord] > to->m_ClipCoords.w()) {

			GLfloat c_x = to->m_ClipCoords[coord];
			GLfloat c_w = to->m_ClipCoords.w();
			GLfloat p_x = from->m_ClipCoords[coord];
			GLfloat p_w = from->m_ClipCoords.w();
			
			GLfloat num = p_w - p_x; 
			GLfloat denom = (p_w - p_x) - (c_w - c_x);

			Interpolate(*tempVertices, *to, *from, num / denom, numVarying);
			to = tempVertices++;

			return true;

		} else {
			// no clipping
			return true;
		}
	}

	inline bool ClipUser(const Vec4f& plane, RasterPos*& from, RasterPos*& to, RasterPos *&tempVertices, size_t numVarying) {

		GLfloat f = Vec4f(from->m_EyeCoords) * plane;
		GLfloat t = Vec4f(to->m_EyeCoords) * plane;

		if (f < 0.0f) {
			if (t <= 0.0f) {
				return false;
			}

			Interpolate(*tempVertices, *from, *to, t / (t - f), numVarying);
			from = tempVertices++;

			return true;

		} else if (t < 0.0f) {

			Interpolate(*tempVertices, *to, *from, f / (f - t), numVarying);
			to = tempVertices++;

			return true;

		} else {
			// no clipping
			return true;
		}
	}
}


void Context :: RenderLine(RasterPos& from, RasterPos& to) {

	RasterPos * tempVertices = m_Temporary;
	RasterPos * pFrom = &from;
	RasterPos * pTo = &to;

	if (m_ClipPlaneEnabled) {
		for (size_t index = 0, mask = 1; index < NUM_CLIP_PLANES; ++index, mask <<= 1) {
			if (m_ClipPlaneEnabled & mask) {
				if (!ClipUser(m_ClipPlanes[index], pFrom, pTo, tempVertices, m_VaryingInfo->numVarying)) {
					return;
				}
			}
		}
	}

	if (Clip(pFrom, pTo, tempVertices, 0, m_VaryingInfo->numVarying) &&
		Clip(pFrom, pTo, tempVertices, 1, m_VaryingInfo->numVarying) &&
		Clip(pFrom, pTo, tempVertices, 2, m_VaryingInfo->numVarying)) {

		ClipCoordsToWindowCoords(*pFrom);
		ClipCoordsToWindowCoords(*pTo);

		if (m_VaryingInfo->colorIndex >= 0) {
			if (m_RasterizerState.GetShadeModel() == RasterizerState::ShadeModelSmooth) {
				pFrom->m_FrontColor.toArray(pFrom->m_Varying + m_VaryingInfo->colorIndex);
			} else {
				pTo->m_FrontColor.toArray(pFrom->m_Varying + m_VaryingInfo->colorIndex);
			}

			pTo->m_FrontColor.toArray(pTo->m_Varying + m_VaryingInfo->colorIndex);
		}

		m_Rasterizer->RasterLine(*pFrom, *pTo);
	}
}


