// ==========================================================================
//
// RasterizerTraingles.cpp	Rasterizer Class for 3D Rendering Library
//
// The rasterizer converts transformed and lit primitives and creates a 
// raster image in the current rendering surface.
//
// This files contains the traingle rasterization code, which was
// previously in the Rasterizer.cpp source file.
//
// --------------------------------------------------------------------------
//
// 05-22-2004		Hans-Martin Will	initial version
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
#include "Rasterizer.h"
#include "Surface.h"
#include "Texture.h"
#include "arm/FunctionCache.h"

using namespace EGL;


namespace {
	I8 Permutation[8][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 0, 0, 0 },	// impossible
		{ 2, 0, 1 },
		{ 1, 0, 2 },
		{ 0, 0, 0 },	// impossible
		{ 1, 2, 0 },
		{ 2, 1, 0 },
	};

	inline I8 * SortPermutation(I32 x0, I32 x1, I32 x2) {
		U32 y0 = static_cast<U32>(x0);
		U32 y1 = static_cast<U32>(x1);
		U32 y2 = static_cast<U32>(x2);

		I8 * result = Permutation[
			(((y1 - y0) >> 29) & 4) |
			(((y2 - y0) >> 30) & 2) |
			(((y2 - y1) >> 31) & 1)];

		assert(result[0] | result[1] | result[2]);

		return result;
	}

	inline int Greater(I32 x0, I32 x1) {
		U32 y0 = static_cast<U32>(x0);
		U32 y1 = static_cast<U32>(x1);

		return (y1 - y0) >> 31;
	}
	
	inline EGL_Fixed TriangleArea(EGL_Fixed x0, EGL_Fixed y0,
								  EGL_Fixed x1, EGL_Fixed y1, 
								  EGL_Fixed x2, EGL_Fixed y2) {
		return 
			EGL_Abs(
				EGL_Mul(x1, y2) + EGL_Mul(x2, y0) + EGL_Mul(x0, y1)
				- EGL_Mul(x2, y1) - EGL_Mul(x0, y2) - EGL_Mul(x1, y0));
	}

	inline int Log2(int value) {
		if (value <= 1) {
			return 0;
		}

		int result = 0;

		while (value > 1) {
			result++;
			value >>= 1;
		}

		return result;
	}

#define DET_SHIFT 4

	inline EGL_Fixed Det2x2(EGL_Fixed a11, EGL_Fixed a12, EGL_Fixed a21, EGL_Fixed a22) {
		return EGL_Mul(a11 >> DET_SHIFT, a22 >> DET_SHIFT) -
			EGL_Mul(a12 >> DET_SHIFT, a21 >> DET_SHIFT);
	}

}


#define SOLVE_PARAM(dx, dy, p1, p2, p3, scale) \
	EGL_Fixed dx = EGL_Mul(																\
			Det2x2(																		\
				p2 - p1, pos2.m_WindowCoords.y - pos1.m_WindowCoords.y,					\
				p3 - p1, pos3.m_WindowCoords.y - pos1.m_WindowCoords.y),				\
			scale);																		\
	EGL_Fixed dy = EGL_Mul(																\
			Det2x2(																		\
				pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, p2 - p1,					\
				pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, p3 - p1),				\
			scale)

#define SOLVE(dx, dy, param, scale) \
	SOLVE_PARAM(dx, dy, pos1.param, pos2.param, pos3.param, scale)


// ---------------------------------------------------------------------------
// Prepare rasterizer object for triangles
// ---------------------------------------------------------------------------
void Rasterizer :: PrepareTriangle() {
	if (m_State->m_TextureEnabled) {
		SetTexture(m_Texture);
	}

	m_ScanlineFunction = m_FunctionCache->GetFunction(*m_State);
	m_MipMapLevel = 0;
}


// --------------------------------------------------------------------------
// number of pixels done with linear interpolation
// --------------------------------------------------------------------------


#define LOG_LINEAR_SPAN 3					// logarithm of value base 2
#define LINEAR_SPAN (1 << LOG_LINEAR_SPAN)	// must be power of 2

//#define NO_COMPILE
#if !defined(NO_COMPILE) && (defined(ARM) || defined(_ARM_))

inline void Rasterizer :: RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y) {
	RasterInfo rasterInfo;

	rasterInfo.SurfaceWidth = m_Surface->GetWidth();
	rasterInfo.SurfaceHeight = m_Surface->GetHeight();
	size_t offset = y * m_Surface->GetWidth();
	rasterInfo.DepthBuffer = m_Surface->GetDepthBuffer() + offset;
	rasterInfo.ColorBuffer = m_Surface->GetColorBuffer() + offset;
	rasterInfo.StencilBuffer = m_Surface->GetStencilBuffer() + offset;
	rasterInfo.AlphaBuffer = m_Surface->GetAlphaBuffer() + offset;

	// texture info
	Texture * texture = m_Texture->GetTexture(m_MipMapLevel);

	if (texture)
	{
		rasterInfo.TextureLogWidth = texture->GetLogWidth();
		rasterInfo.TextureLogHeight = texture->GetLogHeight();
		rasterInfo.TextureLogBytesPerPixel = texture->GetLogBytesPerPixel();
		rasterInfo.TextureExponent = texture->GetExponent();
		rasterInfo.TextureData = texture->GetData();
	}

	m_ScanlineFunction(&rasterInfo, &start, &end);
}

#else 

inline void Rasterizer :: RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y) {

	// In the edge buffer, z, tu and tv are actually divided by w

	FractionalColor baseColor = start.m_Color;

	if (!(end.m_WindowCoords.x - start.m_WindowCoords.x)) {
		return;
	}

	EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);

	FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;

	EGL_Fixed deltaInvZ = EGL_Mul(end.m_WindowCoords.invZ - start.m_WindowCoords.invZ, invSpan);
	EGL_Fixed deltaInvU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	EGL_Fixed deltaInvV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);

	EGL_Fixed deltaFog = EGL_Mul(end.m_FogDensity - start.m_FogDensity, invSpan);
	EGL_Fixed deltaDepth = EGL_Mul(end.m_WindowCoords.depth - start.m_WindowCoords.depth, invSpan);

	EGL_Fixed invTu = start.m_TextureCoords.tu;
	EGL_Fixed invTv = start.m_TextureCoords.tv;
	EGL_Fixed invZ = start.m_WindowCoords.invZ;

	EGL_Fixed fogDensity = start.m_FogDensity;
	I32 x = EGL_IntFromFixed(start.m_WindowCoords.x);
	I32 xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);
	I32 xLinEnd = x + ((xEnd - x) & ~(LINEAR_SPAN - 1));

	EGL_Fixed z = EGL_Inverse(invZ);
	EGL_Fixed tu = EGL_Mul(invTu, z);
	EGL_Fixed tv = EGL_Mul(invTv, z);
	EGL_Fixed depth = start.m_WindowCoords.depth;

	for (; x < xLinEnd;) {

		invZ += deltaInvZ << LOG_LINEAR_SPAN;
		invTu += deltaInvU << LOG_LINEAR_SPAN;
		invTv += deltaInvV << LOG_LINEAR_SPAN;

		EGL_Fixed endZ = EGL_Inverse(invZ);
		EGL_Fixed endTu = EGL_Mul(invTu, endZ);
		EGL_Fixed endTv = EGL_Mul(invTv, endZ);

		EGL_Fixed deltaZ = (endZ - z) >> LOG_LINEAR_SPAN;
		EGL_Fixed deltaTu = (endTu - tu) >> LOG_LINEAR_SPAN; 
		EGL_Fixed deltaTv = (endTv - tv) >> LOG_LINEAR_SPAN;

		int count = LINEAR_SPAN; 

		tu += deltaTu >> 1;
		tv += deltaTv >> 1;

		do {
			Fragment(x, y, depth, tu, tv, EGL_ONE - fogDensity, baseColor);

			baseColor += colorIncrement;
			depth += deltaDepth;
			fogDensity += deltaFog;
			z += deltaZ;
			tu += deltaTu;
			tv += deltaTv;
			++x;
		} while (--count);

		tu -= deltaTu >> 1;
		tv -= deltaTv >> 1;
	}

	if (x != xEnd) {
		EGL_Fixed endZ = EGL_Inverse(end.m_WindowCoords.invZ);
		EGL_Fixed endTu = EGL_Mul(end.m_TextureCoords.tu, endZ);
		EGL_Fixed endTv = EGL_Mul(end.m_TextureCoords.tv, endZ);

		invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));

		EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);

		tu += deltaTu >> 1;
		tv += deltaTv >> 1;

		for (; x < xEnd; ++x) {

			Fragment(x, y, depth, tu, tv, EGL_ONE - fogDensity, baseColor);

			baseColor += colorIncrement;
			depth += deltaDepth;
			tu += deltaTu;
			tv += deltaTv;
			fogDensity += deltaFog;
		}
	}
}
#endif


// ---------------------------------------------------------------------------
// Render the triangle specified by the three transformed and lit vertices
// passed as arguments. Before calling into the actual rasterization, the
// triangle will be subject to the scissor test, which may subdivide it
// into up to 3 sub-triangles.
//
// Parameters:
//		a, b, c		The three vertices of the triangle
//
// Returns:
//		N/A
// --------------------------------------------------------------------------
void Rasterizer :: RasterTriangle(const RasterPos& a, const RasterPos& b,
								  const RasterPos& c) {
	
	// determine the appropriate mipmapping level
	if (m_State->m_TextureEnabled) {
		if (m_Texture->IsMipMap()) {
			int logWidth = Log2(m_Texture->GetTexture(0)->GetWidth());
			int logHeight = Log2(m_Texture->GetTexture(0)->GetHeight());

			int logWidthA = logWidth >> 1;
			int logHeightA = logHeight >> 1;
			int logWidthB = logWidth - logWidthA;
			int logHeightB = logHeight - logHeightA;

			EGL_Fixed textureArea = 
				TriangleArea(a.m_TextureCoords.tu << logWidthA, a.m_TextureCoords.tv << logHeightA,
							 b.m_TextureCoords.tu << logWidthA, b.m_TextureCoords.tv << logHeightA,
							 c.m_TextureCoords.tu << logWidthA, c.m_TextureCoords.tv << logHeightA);


			EGL_Fixed screenArea = 
				TriangleArea(a.m_WindowCoords.x >> logWidthB, a.m_WindowCoords.y >> logHeightB,
							 b.m_WindowCoords.x >> logWidthB, b.m_WindowCoords.y >> logHeightB,
							 c.m_WindowCoords.x >> logWidthB, c.m_WindowCoords.y >> logHeightB);
			EGL_Fixed invScreenArea;
			
			if (screenArea != 0)
				invScreenArea = EGL_Inverse(screenArea);
			else 
				invScreenArea = EGL_FixedFromInt(256);

			EGL_Fixed ratio = EGL_Mul(textureArea, invScreenArea) >> EGL_PRECISION;
			
			int logRatio = Log2(ratio);
			int maxLevel = (logWidth > logHeight) ? logWidth : logHeight;

			if (logRatio <= 0) {
				m_MipMapLevel = 0;
			} else {
				m_MipMapLevel = logRatio / 2;

				if (m_MipMapLevel > maxLevel) {
					m_MipMapLevel = maxLevel;
				}
			}
			
		} else {
			m_MipMapLevel = 0;
		}
	}
	
	const RasterPos * pos[3];
	pos[0] = &a;
	pos[1] = &b;
	pos[2] = &c;

	// sort by y
	I8 * permutation = SortPermutation(a.m_WindowCoords.y, b.m_WindowCoords.y, c.m_WindowCoords.y);
	const RasterPos &pos1 = *pos[permutation[0]];
	const RasterPos &pos2 = *pos[permutation[1]];
	const RasterPos &pos3 = *pos[permutation[2]];

	EGL_Fixed depth1 = pos1.m_WindowCoords.depth;
	EGL_Fixed depth2 = pos2.m_WindowCoords.depth;
	EGL_Fixed depth3 = pos3.m_WindowCoords.depth;

	// ----------------------------------------------------------------------
	// determine if the depth coordinate needs to be adjusted to
	// support polygon-offset
	// ----------------------------------------------------------------------

	if (m_State->m_PolygonOffsetFillEnabled) {

		EGL_Fixed factor = m_State->m_PolygonOffsetFactor;
		EGL_Fixed units = m_State->m_PolygonOffsetUnits;

		EGL_Fixed depthSlope = 0; 
		// calculation here

		// This calculation is based on the plane equation version of the rasterizer,
		// and should be moved out of this block once the rasterizer is converted

		EGL_Fixed denominator = 
			Det2x2(
				pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, pos2.m_WindowCoords.y - pos1.m_WindowCoords.y,
				pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, pos3.m_WindowCoords.y - pos1.m_WindowCoords.y);

		if (denominator)
		{
			EGL_Fixed invDenominator = EGL_Inverse(denominator);
			SOLVE(dDepthdX, dDepthdY, m_WindowCoords.depth, invDenominator);

			EGL_Fixed gradX = dDepthdX > 0 ? dDepthdX : -dDepthdX;
			EGL_Fixed gradY = dDepthdY > 0 ? dDepthdY : -dDepthdY;

			depthSlope = gradX > gradY ? gradX : gradY;
		}


		I32 offset = EGL_IntFromFixed(EGL_Mul(factor, depthSlope) + units * PolygonOffsetUnitSize);

		if (offset > 0) {
			depth1 = depth1 < DepthRangeMax - offset ? depth1 + offset : depth1;
			depth2 = depth2 < DepthRangeMax - offset ? depth2 + offset : depth2;
			depth3 = depth3 < DepthRangeMax - offset ? depth3 + offset : depth3;
		} else {
			depth1 = depth1 > -offset ? depth1 + offset : depth1;
			depth2 = depth2 > -offset ? depth2 + offset : depth2;
			depth3 = depth3 > -offset ? depth3 + offset : depth3;
		}
	}

	EGL_Fixed invZ1 = pos1.m_WindowCoords.invZ;
	EGL_Fixed invZ2 = pos2.m_WindowCoords.invZ;
	EGL_Fixed invZ3 = pos3.m_WindowCoords.invZ;

	EdgePos start, end;
	start.m_WindowCoords.x = pos1.m_WindowCoords.x + (EGL_ONE/2);
	end.m_WindowCoords.x = pos1.m_WindowCoords.x + (EGL_ONE/2);
	start.m_WindowCoords.invZ = end.m_WindowCoords.invZ = invZ1;
	start.m_WindowCoords.depth = end.m_WindowCoords.depth = depth1;
	start.m_Color = end.m_Color = pos1.m_Color;
	start.m_TextureCoords.tu = end.m_TextureCoords.tu = EGL_Mul(pos1.m_TextureCoords.tu, invZ1);
	start.m_TextureCoords.tv = end.m_TextureCoords.tv = EGL_Mul(pos1.m_TextureCoords.tv, invZ1);
	start.m_FogDensity = end.m_FogDensity = EGL_ONE - pos1.m_FogDensity;

	// set up the triangle
	// init start, end, deltas
	EGL_Fixed deltaY21 = pos2.m_WindowCoords.y - pos1.m_WindowCoords.y;

	EGL_Fixed invDeltaY2;

	EGL_Fixed incX2;
	EGL_Fixed incR2;
	EGL_Fixed incG2;
	EGL_Fixed incB2;
	EGL_Fixed incA2;
	EGL_Fixed incFog2;
	EGL_Fixed incDepth2;

	// perspective interpolation
	EGL_Fixed incZ2;

	if (deltaY21) {
		invDeltaY2 = EGL_Inverse(pos2.m_WindowCoords.y - pos1.m_WindowCoords.y);

		incX2 = EGL_Mul(pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY2);
		incR2 = EGL_Mul(pos2.m_Color.r - pos1.m_Color.r, invDeltaY2);
		incG2 = EGL_Mul(pos2.m_Color.g - pos1.m_Color.g, invDeltaY2);
		incB2 = EGL_Mul(pos2.m_Color.b - pos1.m_Color.b, invDeltaY2);
		incA2 = EGL_Mul(pos2.m_Color.a - pos1.m_Color.a, invDeltaY2);
		incFog2 = EGL_Mul(pos1.m_FogDensity - pos2.m_FogDensity, invDeltaY2);
		incDepth2 = EGL_Mul(depth2 - depth1, invDeltaY2);

		// perspective interpolation
		incZ2 = EGL_Mul(invZ2 - invZ1, invDeltaY2);

	} else {
		invDeltaY2 = 0;

		incX2 = pos2.m_WindowCoords.x - pos1.m_WindowCoords.x;
		incR2 = 0;
		incG2 = 0;
		incB2 = 0;
		incA2 = 0;
		incFog2 = 0;
		incDepth2 = 0;

		// perspective interpolation
		incZ2 = 0;

	}

	EGL_Fixed incTu2 = EGL_Mul(EGL_Mul(pos2.m_TextureCoords.tu, invZ2) - 
							   start.m_TextureCoords.tu, invDeltaY2);

	EGL_Fixed incTv2 = EGL_Mul(EGL_Mul(pos2.m_TextureCoords.tv, invZ2) - 
							   start.m_TextureCoords.tv, invDeltaY2);


	EGL_Fixed deltaY31 = pos3.m_WindowCoords.y - pos1.m_WindowCoords.y;

	EGL_Fixed invDeltaY3;

	EGL_Fixed incX3;
	EGL_Fixed incR3;
	EGL_Fixed incG3;
	EGL_Fixed incB3;
	EGL_Fixed incA3;
	EGL_Fixed incFog3;
	EGL_Fixed incDepth3;

	// perspective interpolation
	EGL_Fixed incZ3;

	if (deltaY31) {
		invDeltaY3 = EGL_Inverse(deltaY31);

		incX3 = EGL_Mul(pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY3);
		incR3 = EGL_Mul(pos3.m_Color.r - pos1.m_Color.r, invDeltaY3);
		incG3 = EGL_Mul(pos3.m_Color.g - pos1.m_Color.g, invDeltaY3);
		incB3 = EGL_Mul(pos3.m_Color.b - pos1.m_Color.b, invDeltaY3);
		incA3 = EGL_Mul(pos3.m_Color.a - pos1.m_Color.a, invDeltaY3);
		incFog3 = EGL_Mul(pos1.m_FogDensity - pos3.m_FogDensity, invDeltaY3);
		incDepth3 = EGL_Mul(depth3 - depth1, invDeltaY3);

		// perspective interpolation
		incZ3 = EGL_Mul(invZ3 - invZ1, invDeltaY3);
	} else {
		invDeltaY3 = 0;

		incX3 = pos3.m_WindowCoords.x - pos1.m_WindowCoords.x;
		incR3 = 0;
		incG3 = 0;
		incB3 = 0;
		incA3 = 0;
		incFog3 = 0;
		incDepth3 = 0;

		// perspective interpolation
		incZ3 = 0;
	}

	EGL_Fixed incTu3 = EGL_Mul(EGL_Mul(pos3.m_TextureCoords.tu, invZ3) - 
							   start.m_TextureCoords.tu, invDeltaY3);

	EGL_Fixed incTv3 = EGL_Mul(EGL_Mul(pos3.m_TextureCoords.tv, invZ3) - 
							   start.m_TextureCoords.tv, invDeltaY3);

	EGL_Fixed deltaY32 = pos3.m_WindowCoords.y - pos2.m_WindowCoords.y;
	EGL_Fixed invDeltaY23;

	EGL_Fixed incX23;
	EGL_Fixed incR23;
	EGL_Fixed incG23;
	EGL_Fixed incB23;
	EGL_Fixed incA23;
	EGL_Fixed incFog23;
	EGL_Fixed incDepth23;

	// perspective interpolation
	EGL_Fixed incZ23;

	if (deltaY32) {
		invDeltaY23 = EGL_Inverse(deltaY32);

		incX23 = EGL_Mul(pos3.m_WindowCoords.x - pos2.m_WindowCoords.x, invDeltaY23);
		incR23 = EGL_Mul(pos3.m_Color.r - pos2.m_Color.r, invDeltaY23);
		incG23 = EGL_Mul(pos3.m_Color.g - pos2.m_Color.g, invDeltaY23);
		incB23 = EGL_Mul(pos3.m_Color.b - pos2.m_Color.b, invDeltaY23);
		incA23 = EGL_Mul(pos3.m_Color.a - pos2.m_Color.a, invDeltaY23);
		incFog23 = EGL_Mul(pos2.m_FogDensity - pos3.m_FogDensity, invDeltaY23);
		incDepth23 = EGL_Mul(depth3 - depth2, invDeltaY23);

		// perspective interpolation
		incZ23 = EGL_Mul(invZ3 - invZ2, invDeltaY23);

	} else {
		invDeltaY23 = 0;

		incX23 = pos3.m_WindowCoords.x - pos2.m_WindowCoords.x;
		incR23 = 0;
		incG23 = 0;
		incB23 = 0;
		incA23 = 0;
		incFog23 = 0;
		incDepth23 = 0;

		// perspective interpolation
		incZ23 = 0;

	}

	EGL_Fixed incTu23 = EGL_Mul(EGL_Mul(pos3.m_TextureCoords.tu, invZ3) - 
								EGL_Mul(pos2.m_TextureCoords.tu, invZ2), invDeltaY23);

	EGL_Fixed incTv23 = EGL_Mul(EGL_Mul(pos3.m_TextureCoords.tv, invZ3) - 
								EGL_Mul(pos2.m_TextureCoords.tv, invZ2), invDeltaY23);

	I32 yStart = EGL_Round(pos1.m_WindowCoords.y);
	I32 yEnd = EGL_Round(pos2.m_WindowCoords.y);
	I32 y;

	y = yStart;

	if (m_State->m_ScissorTestEnabled) {

		// TO DO: This can be optimized, but we'll address it when we convert the whole
		// function to the plane equation approach

		I32 yScissorStart = m_State->m_ScissorY;
		I32 yScissorEnd = yScissorStart + m_State->m_ScissorHeight;

		if (incX2 < incX3) {

			for (; y < yEnd; ++y) {

				if (y >= yScissorStart && y < yScissorEnd) 
					RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX2;
				start.m_Color.r += incR2;
				start.m_Color.g += incG2;
				start.m_Color.b += incB2;
				start.m_Color.a += incA2;

				start.m_WindowCoords.invZ += incZ2;
				start.m_TextureCoords.tu += incTu2;
				start.m_TextureCoords.tv += incTv2;

				start.m_FogDensity += incFog2;
				start.m_WindowCoords.depth += incDepth2;

				// update end
				end.m_WindowCoords.x += incX3;
				end.m_Color.r += incR3;
				end.m_Color.g += incG3;
				end.m_Color.b += incB3;
				end.m_Color.a += incA3;

				end.m_WindowCoords.invZ += incZ3;
				end.m_TextureCoords.tu += incTu3;
				end.m_TextureCoords.tv += incTv3;

				end.m_FogDensity += incFog3;
				end.m_WindowCoords.depth += incDepth3;

			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			start.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			start.m_WindowCoords.invZ = invZ2;
			start.m_WindowCoords.depth = depth2;
			start.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			start.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			start.m_Color = pos2.m_Color;
			start.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			for (; y < yEnd; ++y) {

				if (y >= yScissorStart && y < yScissorEnd) 
					RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX23;
				start.m_Color.r += incR23;
				start.m_Color.g += incG23;
				start.m_Color.b += incB23;
				start.m_Color.a += incA23;

				start.m_WindowCoords.invZ += incZ23;
				start.m_TextureCoords.tu += incTu23;
				start.m_TextureCoords.tv += incTv23;

				start.m_FogDensity += incFog23;
				start.m_WindowCoords.depth += incDepth23;

				// update end
				end.m_WindowCoords.x += incX3;
				end.m_Color.r += incR3;
				end.m_Color.g += incG3;
				end.m_Color.b += incB3;
				end.m_Color.a += incA3;

				end.m_WindowCoords.invZ += incZ3;
				end.m_TextureCoords.tu += incTu3;
				end.m_TextureCoords.tv += incTv3;

				end.m_FogDensity += incFog3;
				end.m_WindowCoords.depth += incDepth3;
			}
		} else {
			for (; y < yEnd; ++y) {

				if (y >= yScissorStart && y < yScissorEnd) 
					RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX3;
				start.m_Color.r += incR3;
				start.m_Color.g += incG3;
				start.m_Color.b += incB3;
				start.m_Color.a += incA3;

				start.m_WindowCoords.invZ += incZ3;
				start.m_TextureCoords.tu += incTu3;
				start.m_TextureCoords.tv += incTv3;

				start.m_FogDensity += incFog3;
				start.m_WindowCoords.depth += incDepth3;

				// update end
				end.m_WindowCoords.x += incX2;
				end.m_Color.r += incR2;
				end.m_Color.g += incG2;
				end.m_Color.b += incB2;
				end.m_Color.a += incA2;

				end.m_WindowCoords.invZ += incZ2;
				end.m_TextureCoords.tu += incTu2;
				end.m_TextureCoords.tv += incTv2;

				end.m_FogDensity += incFog2;
				end.m_WindowCoords.depth += incDepth2;

			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			end.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			end.m_Color = pos2.m_Color;
			end.m_WindowCoords.invZ = invZ2;
			end.m_WindowCoords.depth = depth2;
			end.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			end.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			end.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			for (; y < yEnd; ++y) {

				if (y >= yScissorStart && y < yScissorEnd) 
					RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX3;
				start.m_Color.r += incR3;
				start.m_Color.g += incG3;
				start.m_Color.b += incB3;
				start.m_Color.a += incA3;

				start.m_WindowCoords.invZ += incZ3;
				start.m_TextureCoords.tu += incTu3;
				start.m_TextureCoords.tv += incTv3;

				start.m_FogDensity += incFog3;
				start.m_WindowCoords.depth += incDepth3;

				// update end
				end.m_WindowCoords.x += incX23;
				end.m_Color.r += incR23;
				end.m_Color.g += incG23;
				end.m_Color.b += incB23;
				end.m_Color.a += incA23;

				end.m_WindowCoords.invZ += incZ23;
				end.m_TextureCoords.tu += incTu23;
				end.m_TextureCoords.tv += incTv23;

				end.m_FogDensity += incFog23;
				end.m_WindowCoords.depth += incDepth23;
			}
		}
	} else {
		if (incX2 < incX3) {

			for (; y < yEnd; ++y) {

				RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX2;
				start.m_Color.r += incR2;
				start.m_Color.g += incG2;
				start.m_Color.b += incB2;
				start.m_Color.a += incA2;

				start.m_WindowCoords.invZ += incZ2;
				start.m_TextureCoords.tu += incTu2;
				start.m_TextureCoords.tv += incTv2;

				start.m_FogDensity += incFog2;
				start.m_WindowCoords.depth += incDepth2;

				// update end
				end.m_WindowCoords.x += incX3;
				end.m_Color.r += incR3;
				end.m_Color.g += incG3;
				end.m_Color.b += incB3;
				end.m_Color.a += incA3;

				end.m_WindowCoords.invZ += incZ3;
				end.m_TextureCoords.tu += incTu3;
				end.m_TextureCoords.tv += incTv3;

				end.m_FogDensity += incFog3;
				end.m_WindowCoords.depth += incDepth3;

			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			start.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			start.m_WindowCoords.invZ = invZ2;
			start.m_WindowCoords.depth = depth2;
			start.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			start.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			start.m_Color = pos2.m_Color;
			start.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			for (; y < yEnd; ++y) {
				
				RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX23;
				start.m_Color.r += incR23;
				start.m_Color.g += incG23;
				start.m_Color.b += incB23;
				start.m_Color.a += incA23;

				start.m_WindowCoords.invZ += incZ23;
				start.m_TextureCoords.tu += incTu23;
				start.m_TextureCoords.tv += incTv23;

				start.m_FogDensity += incFog23;
				start.m_WindowCoords.depth += incDepth23;

				// update end
				end.m_WindowCoords.x += incX3;
				end.m_Color.r += incR3;
				end.m_Color.g += incG3;
				end.m_Color.b += incB3;
				end.m_Color.a += incA3;

				end.m_WindowCoords.invZ += incZ3;
				end.m_TextureCoords.tu += incTu3;
				end.m_TextureCoords.tv += incTv3;

				end.m_FogDensity += incFog3;
				end.m_WindowCoords.depth += incDepth3;
			}
		} else {
			for (; y < yEnd; ++y) {

				RasterScanLine(start, end, y);

				// update start
				start.m_WindowCoords.x += incX3;
				start.m_Color.r += incR3;
				start.m_Color.g += incG3;
				start.m_Color.b += incB3;
				start.m_Color.a += incA3;

				start.m_WindowCoords.invZ += incZ3;
				start.m_TextureCoords.tu += incTu3;
				start.m_TextureCoords.tv += incTv3;

				start.m_FogDensity += incFog3;
				start.m_WindowCoords.depth += incDepth3;

				// update end
				end.m_WindowCoords.x += incX2;
				end.m_Color.r += incR2;
				end.m_Color.g += incG2;
				end.m_Color.b += incB2;
				end.m_Color.a += incA2;

				end.m_WindowCoords.invZ += incZ2;
				end.m_TextureCoords.tu += incTu2;
				end.m_TextureCoords.tv += incTv2;

				end.m_FogDensity += incFog2;
				end.m_WindowCoords.depth += incDepth2;

			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			end.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			end.m_Color = pos2.m_Color;
			end.m_WindowCoords.invZ = invZ2;
			end.m_WindowCoords.depth = depth2;
			end.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			end.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			end.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			for (; y < yEnd; ++y) {
				RasterScanLine(start, end, y);
				// update start
				start.m_WindowCoords.x += incX3;
				start.m_Color.r += incR3;
				start.m_Color.g += incG3;
				start.m_Color.b += incB3;
				start.m_Color.a += incA3;

				start.m_WindowCoords.invZ += incZ3;
				start.m_TextureCoords.tu += incTu3;
				start.m_TextureCoords.tv += incTv3;

				start.m_FogDensity += incFog3;
				start.m_WindowCoords.depth += incDepth3;

				// update end
				end.m_WindowCoords.x += incX23;
				end.m_Color.r += incR23;
				end.m_Color.g += incG23;
				end.m_Color.b += incB23;
				end.m_Color.a += incA23;

				end.m_WindowCoords.invZ += incZ23;
				end.m_TextureCoords.tu += incTu23;
				end.m_TextureCoords.tv += incTv23;

				end.m_FogDensity += incFog23;
				end.m_WindowCoords.depth += incDepth23;
			}
		}
	}
}


