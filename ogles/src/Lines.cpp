// ==========================================================================
//
// lines.cpp	Rendering Context Class for 3D Rendering Library
//
//				Rendering Operations for Lines
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
	GetRasterizerState()->SetLineWidth(width);
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

	inline EGL_Fixed InterpolationCoefficient(EGL_Fixed a, EGL_Fixed b, EGL_Fixed c) {
		return EGL_Div(b - a, c - a);
	}

	inline EGL_Fixed Interpolate(EGL_Fixed x0, EGL_Fixed y0, EGL_Fixed x1, EGL_Fixed y1) {
		return EGL_Mul(x0, y0) + EGL_Mul(x1, y1);
	}

	inline void Adjust(RasterPos& dst, const RasterPos& src, EGL_Fixed scale) {
		EGL_Fixed invScale = EGL_ONE - scale;

		dst.m_ClipCoords = dst.m_ClipCoords * scale + src.m_ClipCoords * invScale;
		dst.m_Color = dst.m_Color * scale + src.m_Color * invScale;
		dst.m_TextureCoords.tu = Interpolate(dst.m_TextureCoords.tu, scale, src.m_TextureCoords.tu, invScale);
		dst.m_TextureCoords.tv = Interpolate(dst.m_TextureCoords.tv, scale, src.m_TextureCoords.tv, invScale);
		dst.m_FogDensity = Interpolate(dst.m_FogDensity, scale, src.m_FogDensity, invScale);
	}

	inline bool ClipX(RasterPos& from, RasterPos& to) {
#		define COORDINATE x()
#		include "LineClipper.inc"
#		undef COORDINATE
	}

	inline bool ClipY(RasterPos& from, RasterPos& to) {
#		define COORDINATE y()
#		include "LineClipper.inc"
#		undef COORDINATE
	}

	inline bool ClipZ(RasterPos& from, RasterPos& to) {
#		define COORDINATE z()
#		include "LineClipper.inc"
#		undef COORDINATE
	}
}


void Context :: RenderLine(RasterPos& from, RasterPos& to) {

	if (ClipX(from, to) &&
		ClipY(from, to) &&
		ClipZ(from, to)) {
		ClipCoordsToWindowCoords(from);
		ClipCoordsToWindowCoords(to);
		from.m_Color = from.m_FrontColor;
		to.m_Color = to.m_FrontColor;
		m_Rasterizer->RasterLine(from, to);
	}
}


