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


#define SOLVE_PARAM_X(dx, dy, p1, p2, p3, scale) \
	EGL_Fixed dx = EGL_Mul(																\
			Det2x2(																		\
				p2 - p1, pos2.m_WindowCoords.y - pos1.m_WindowCoords.y,					\
				p3 - p1, pos3.m_WindowCoords.y - pos1.m_WindowCoords.y),				\
			scale);	

#define SOLVE_PARAM_XY(dx, dy, p1, p2, p3, scale) \
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


#define SOLVE_X(dx, dy, param, scale) \
	SOLVE_PARAM_X(dx, dy, pos1.param, pos2.param, pos3.param, scale)

#define SOLVE_XY(dx, dy, param, scale) \
	SOLVE_PARAM_XY(dx, dy, pos1.param, pos2.param, pos3.param, scale)


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

#define NO_COMPILE
#if !defined(NO_COMPILE) && (defined(ARM) || defined(_ARM_))

inline void Rasterizer :: RasterScanLine(const RasterInfo & rasterInfo, const EdgePos & start, const EdgePos & end) {
	m_ScanlineFunction(&rasterInfo, &start, &end);
}

#else 

inline void Rasterizer :: RasterScanLine(const RasterInfo & rasterInfo, const EdgePos & start, const EdgePos & delta) {

	// In the edge buffer, z, tu and tv are actually divided by w

	FractionalColor baseColor = start.m_Color;

	if (!(delta.m_WindowCoords.x - start.m_WindowCoords.x)) {
		return;
	}

	const FractionalColor& colorIncrement = delta.m_Color;

	EGL_Fixed deltaInvZ = delta.m_WindowCoords.invZ;
	EGL_Fixed deltaInvU = delta.m_TextureCoords.tu;
	EGL_Fixed deltaInvV = delta.m_TextureCoords.tv;

	EGL_Fixed deltaFog = delta.m_FogDensity;
	EGL_Fixed deltaDepth = delta.m_WindowCoords.depth;

	EGL_Fixed invTu = start.m_TextureCoords.tu;
	EGL_Fixed invTv = start.m_TextureCoords.tv;
	EGL_Fixed invZ = start.m_WindowCoords.invZ;

	EGL_Fixed fogDensity = start.m_FogDensity;
	I32 x = EGL_IntFromFixed(start.m_WindowCoords.x);
	I32 xEnd = EGL_IntFromFixed(delta.m_WindowCoords.x);
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
			Fragment(&rasterInfo, x, depth, tu, tv, baseColor, EGL_ONE - fogDensity);

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

		I32 deltaX = xEnd - x;

		EGL_Fixed endZ = EGL_Inverse(invZ + deltaX * deltaInvZ);
		EGL_Fixed endTu = EGL_Mul(invTu + deltaX * deltaInvU, endZ);
		EGL_Fixed endTv = EGL_Mul(invTv + deltaX * deltaInvV, endZ);

		EGL_Fixed invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));

		EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);

		tu += deltaTu >> 1;
		tv += deltaTv >> 1;

		for (; x < xEnd; ++x) {

			Fragment(&rasterInfo, x, depth, tu, tv, baseColor, EGL_ONE - fogDensity);

			baseColor += colorIncrement;
			depth += deltaDepth;
			tu += deltaTu;
			tv += deltaTv;
			fogDensity += deltaFog;
		}
	}
}
#endif


#define TrianglePartScissor(rasterInfo, start, delta, deltaStart, deltaDeltaX, y, yEnd, yScissorStart, yScissorEnd) \
	{																									\
		for (; y < yEnd; ++y) {																			\
			if (y >= yScissorStart && y < yScissorEnd)													\
				RasterScanLine(rasterInfo, start, delta);												\
																										\
			AdvanceScanline(rasterInfo, start, delta, deltaStart, deltaDeltaX);							\
		}																								\
	}


#define TrianglePart(rasterInfo, start, delta, deltaStart, deltaDeltaX, y, yEnd)						\
	{																									\
		for (; y < yEnd; ++y) {																			\
			RasterScanLine(rasterInfo, start, delta);													\
			AdvanceScanline(rasterInfo, start, delta, deltaStart, deltaDeltaX);							\
		}																								\
	}

#define AdvanceScanline(rasterInfo, start, delta, deltaStart, deltaDeltaX)						\
	start.m_WindowCoords.x += deltaStart.m_WindowCoords.x;										\
	start.m_Color += deltaStart.m_Color;														\
	start.m_WindowCoords.invZ += deltaStart.m_WindowCoords.invZ;								\
	start.m_TextureCoords.tu += deltaStart.m_TextureCoords.tu;									\
	start.m_TextureCoords.tv += deltaStart.m_TextureCoords.tv;									\
	start.m_FogDensity += deltaStart.m_FogDensity;												\
	start.m_WindowCoords.depth += deltaStart.m_WindowCoords.depth;								\
																								\
	delta.m_WindowCoords.x += deltaDeltaX;														\
																								\
	rasterInfo.DepthBuffer += rasterInfo.SurfaceWidth;											\
	rasterInfo.ColorBuffer += rasterInfo.SurfaceWidth;											\
	rasterInfo.StencilBuffer += rasterInfo.SurfaceWidth;										\
	rasterInfo.AlphaBuffer += rasterInfo.SurfaceWidth



// ---------------------------------------------------------------------------
// Ultimately, we have the following variantions on rastering a triangle:
//
//	- With or without color
//  - With or without texture
//  - With or without depth
//  - With or without fog
//  - With or without alpha buffer
//  - With or without stencil buffer
//  - With or without scissor test
//
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// Render the triangle specified by the three transformed and lit vertices
// passed as arguments. 
//
// Parameters:
//		a, b, c		The three vertices of the triangle
//
// Returns:
//		N/A
// --------------------------------------------------------------------------
void Rasterizer :: RasterTriangle(const RasterPos& a, const RasterPos& b,
								  const RasterPos& c) {
	
	// ----------------------------------------------------------------------
	// sort vertices by y
	// ----------------------------------------------------------------------

	const RasterPos * pos[3];
	pos[0] = &a;
	pos[1] = &b;
	pos[2] = &c;

	I8 * permutation = SortPermutation(a.m_WindowCoords.y, b.m_WindowCoords.y, c.m_WindowCoords.y);
	const RasterPos &pos1 = *pos[permutation[0]];
	const RasterPos &pos2 = *pos[permutation[1]];
	const RasterPos &pos3 = *pos[permutation[2]];

	// ----------------------------------------------------------------------
	// Calculate the screen area of the triangle
	// ----------------------------------------------------------------------

	EGL_Fixed denominator = 
		Det2x2(
			pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, pos2.m_WindowCoords.y - pos1.m_WindowCoords.y,
			pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, pos3.m_WindowCoords.y - pos1.m_WindowCoords.y);

	if (!denominator) {
		// invisible -> done
		return;
	}

	// ----------------------------------------------------------------------
	// calculate all gradients for interpolation 
	// ----------------------------------------------------------------------

	EGL_Fixed invDenominator = EGL_Inverse(denominator);

	SOLVE_XY(dRdX, dRdY, m_Color.r, invDenominator);
	SOLVE_XY(dGdX, dGdY, m_Color.g, invDenominator);
	SOLVE_XY(dBdX, dBdY, m_Color.b, invDenominator);
	SOLVE_XY(dAdX, dAdY, m_Color.a, invDenominator);

	SOLVE_XY(dFogdX, dFogdY, m_FogDensity, invDenominator);
	SOLVE_XY(dDepthdX, dDepthdY, m_WindowCoords.depth, invDenominator);

	SOLVE_XY(dInvZdX, dInvZdY, m_WindowCoords.invZ, invDenominator);

	EGL_Fixed tuOverZ1 = EGL_Mul(pos1.m_TextureCoords.tu, pos1.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ2 = EGL_Mul(pos2.m_TextureCoords.tu, pos2.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ3 = EGL_Mul(pos3.m_TextureCoords.tu, pos3.m_WindowCoords.invZ);

	EGL_Fixed tvOverZ1 = EGL_Mul(pos1.m_TextureCoords.tv, pos1.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ2 = EGL_Mul(pos2.m_TextureCoords.tv, pos2.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ3 = EGL_Mul(pos3.m_TextureCoords.tv, pos3.m_WindowCoords.invZ);

	SOLVE_PARAM_XY(dTuOverZdX, dTuOverZdY, tuOverZ1, tuOverZ2, tuOverZ3, invDenominator);
	SOLVE_PARAM_XY(dTvOverZdX, dTvOverZdY, tvOverZ1, tvOverZ2, tvOverZ3, invDenominator);

	// Fill in the gradient in x direction for scanline function

	EdgePos delta;
	delta.m_Color = FractionalColor(dRdX, dGdX, dBdX, dAdX);
	delta.m_TextureCoords.tu = dTuOverZdX;
	delta.m_TextureCoords.tv = dTvOverZdX;
	delta.m_FogDensity = -dFogdX;
	delta.m_WindowCoords.depth = dDepthdX;
	delta.m_WindowCoords.invZ = dInvZdX;

	// ----------------------------------------------------------------------
	// determine if the depth coordinate needs to be adjusted to
	// support polygon-offset
	// ----------------------------------------------------------------------

	EGL_Fixed depth1 = pos1.m_WindowCoords.depth;
	EGL_Fixed depth2 = pos2.m_WindowCoords.depth;
	EGL_Fixed depth3 = pos3.m_WindowCoords.depth;

	if (m_State->m_PolygonOffsetFillEnabled) {

		EGL_Fixed factor = m_State->m_PolygonOffsetFactor;
		EGL_Fixed units = m_State->m_PolygonOffsetUnits;

		// calculation here

		EGL_Fixed gradX = dDepthdX > 0 ? dDepthdX : -dDepthdX;
		EGL_Fixed gradY = dDepthdY > 0 ? dDepthdY : -dDepthdY;

		EGL_Fixed depthSlope = gradX > gradY ? gradX : gradY;

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

	// ----------------------------------------------------------------------
	// determine the appropriate mipmapping level
	// ----------------------------------------------------------------------

	if (m_State->m_TextureEnabled) {
		if (m_Texture->IsMipMap()) {
			int logWidth = Log2(m_Texture->GetTexture(0)->GetWidth());
			int logHeight = Log2(m_Texture->GetTexture(0)->GetHeight());

			EGL_Fixed textureArea = 
				Det2x2(
					(pos2.m_TextureCoords.tu - pos1.m_TextureCoords.tu) << logWidth, 
					(pos2.m_TextureCoords.tv - pos1.m_TextureCoords.tv) << logHeight,
					(pos3.m_TextureCoords.tu - pos1.m_TextureCoords.tu) << logWidth,
					(pos3.m_TextureCoords.tv - pos1.m_TextureCoords.tv) << logHeight);
			
			EGL_Fixed ratio = EGL_Mul(textureArea, invDenominator) >> EGL_PRECISION;
			
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
	
	EGL_Fixed invZ1 = pos1.m_WindowCoords.invZ;
	EGL_Fixed invZ2 = pos2.m_WindowCoords.invZ;
	EGL_Fixed invZ3 = pos3.m_WindowCoords.invZ;

	// ----------------------------------------------------------------------
	//
	// - Raster top part of triangle
	//		determine integer (i.e. each line) and fractional increments per edgebuffer
	//		determine y-pre step and x-pre step for first pixel of first scanline
	//		determine error for x-stepping.
	//
	// - Raster bottom part of triangle
	// ----------------------------------------------------------------------

	EGL_Fixed yRounded1 = EGL_NearestInt(pos1.m_WindowCoords.y);
	EGL_Fixed yPreStep1 = yRounded1 + (EGL_ONE/2) - pos1.m_WindowCoords.y;
	I32 y = EGL_IntFromFixed(yRounded1);

	EGL_Fixed yRounded2 = EGL_NearestInt(pos2.m_WindowCoords.y);
	EGL_Fixed yPreStep2 = yRounded2 + (EGL_ONE/2) - pos2.m_WindowCoords.y;
	I32 y2 = EGL_IntFromFixed(yRounded2);

	EGL_Fixed yRounded3 = EGL_NearestInt(pos3.m_WindowCoords.y);
	I32 y3 = EGL_IntFromFixed(yRounded3);

	RasterInfo rasterInfo(m_Surface, y);

	// ----------------------------------------------------------------------
	// texture info
	// ----------------------------------------------------------------------

	Texture * texture = m_Texture->GetTexture(m_MipMapLevel);

	if (texture)
	{
		rasterInfo.TextureLogWidth = texture->GetLogWidth();
		rasterInfo.TextureLogHeight = texture->GetLogHeight();
		rasterInfo.TextureLogBytesPerPixel = texture->GetLogBytesPerPixel();
		rasterInfo.TextureExponent = texture->GetExponent();
		rasterInfo.TextureData = texture->GetData();
	}

	// ----------------------------------------------------------------------
	// Determine edge increments and scanline starting point
	// ----------------------------------------------------------------------

	EGL_Fixed deltaY2	= pos2.m_WindowCoords.y - pos1.m_WindowCoords.y;
	EGL_Fixed deltaY3	= pos3.m_WindowCoords.y - pos1.m_WindowCoords.y;
	EGL_Fixed deltaY23	= pos3.m_WindowCoords.y - pos2.m_WindowCoords.y;

	if (y >= y3) {
		// Always, the triangle is empty
		return;
	}

	if (y == y2) {
		// do special case here

		EGL_Fixed invDeltaY3 = EGL_Inverse(deltaY3);	// deltaY3 == 0 should not occur, triangle would already have been skipped
		EGL_Fixed invDeltaY23 = EGL_Inverse(deltaY23);

		EGL_Fixed deltaX3	= pos3.m_WindowCoords.x - pos1.m_WindowCoords.x;
		EGL_Fixed deltaX23	= pos3.m_WindowCoords.x - pos2.m_WindowCoords.x;

		EGL_Fixed dXdY3		= EGL_Mul(deltaX3, invDeltaY3);
		EGL_Fixed dXdY23	= EGL_Mul(deltaX23, invDeltaY23);

		if (dXdY23 < dXdY3) {
			// move beginning of scanline along p1->p3

			// ------------------------------------------------------------------
			// initialize start onto first pixel right off line p1->p3
			// ------------------------------------------------------------------

			EGL_Fixed xStepped1L = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY3);
			EGL_Fixed xRounded1 = EGL_NearestInt(xStepped1L);
			EGL_Fixed xPreStep1 = xRounded1 + (EGL_ONE/2) - pos1.m_WindowCoords.x;

			EdgePos start;
			start.m_WindowCoords.x = 
				xStepped1L + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

			start.m_WindowCoords.invZ	= invZ1					+ EGL_Mul(dInvZdX, xPreStep1)	+ EGL_Mul(dInvZdY, yPreStep1); 
			start.m_WindowCoords.depth	= depth1				+ EGL_Mul(dDepthdX, xPreStep1)	+ EGL_Mul(dDepthdY, yPreStep1); 

			start.m_Color.r				= pos1.m_Color.r		+ EGL_Mul(dRdX, xPreStep1)		+ EGL_Mul(dRdY, yPreStep1);
			start.m_Color.g				= pos1.m_Color.g		+ EGL_Mul(dGdX, xPreStep1)		+ EGL_Mul(dGdY, yPreStep1);
			start.m_Color.b				= pos1.m_Color.b		+ EGL_Mul(dBdX, xPreStep1)		+ EGL_Mul(dBdY, yPreStep1);
			start.m_Color.a				= pos1.m_Color.a		+ EGL_Mul(dAdX, xPreStep1)		+ EGL_Mul(dAdY, yPreStep1);
			
			start.m_TextureCoords.tu	= tuOverZ1				+ EGL_Mul(dTuOverZdX, xPreStep1) + EGL_Mul(dTuOverZdY, yPreStep1);
			start.m_TextureCoords.tv	= tvOverZ1				+ EGL_Mul(dTvOverZdX, xPreStep1) + EGL_Mul(dTvOverZdY, yPreStep1);

			start.m_FogDensity			= pos1.m_FogDensity		+ EGL_Mul(dFogdX, xPreStep1)	+ EGL_Mul(dFogdY, yPreStep1);

			// ------------------------------------------------------------------
			// initialize edge buffer delta2Int & delta2Frac
			// ------------------------------------------------------------------

			// determine integer x step/y
			I32 dXdYStep1Int = dXdY3 >= 0 ? EGL_IntFromFixed(dXdY3) : -EGL_IntFromFixed(-dXdY3);

			EdgePos delta3Small;

			delta3Small.m_WindowCoords.x	= dXdY3;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

			delta3Small.m_WindowCoords.invZ	= dInvZdX * dXdYStep1Int	+ dInvZdY; 
			delta3Small.m_WindowCoords.depth= dDepthdX * dXdYStep1Int	+ dDepthdY; 

			delta3Small.m_Color.r			= dRdX * dXdYStep1Int		+ dRdY;
			delta3Small.m_Color.g			= dGdX * dXdYStep1Int		+ dGdY;
			delta3Small.m_Color.b			= dBdX * dXdYStep1Int		+ dBdY;
			delta3Small.m_Color.a			= dAdX * dXdYStep1Int		+ dAdY;
			
			delta3Small.m_TextureCoords.tu	= dTuOverZdX * dXdYStep1Int + dTuOverZdY;
			delta3Small.m_TextureCoords.tv	= dTvOverZdX * dXdYStep1Int + dTvOverZdY;

			delta3Small.m_FogDensity		= dFogdX * dXdYStep1Int		+ dFogdY;

			EdgePos delta3Big;

			delta3Big.m_WindowCoords.x		= dXdY3;

			if (dXdY3 >= 0) {
				delta3Big.m_WindowCoords.invZ	= delta3Small.m_WindowCoords.invZ	+ dInvZdX; 
				delta3Big.m_WindowCoords.depth	= delta3Small.m_WindowCoords.depth	+ dDepthdX; 

				delta3Big.m_Color.r				= delta3Small.m_Color.r				+ dRdX;
				delta3Big.m_Color.g				= delta3Small.m_Color.g				+ dGdX;
				delta3Big.m_Color.b				= delta3Small.m_Color.b				+ dBdX;
				delta3Big.m_Color.a				= delta3Small.m_Color.a				+ dAdX;
				
				delta3Big.m_TextureCoords.tu	= delta3Small.m_TextureCoords.tu	+ dTuOverZdX;
				delta3Big.m_TextureCoords.tv	= delta3Small.m_TextureCoords.tv	+ dTvOverZdX;

				delta3Big.m_FogDensity			= delta3Small.m_FogDensity			+ dFogdX;
			} else {
				delta3Big.m_WindowCoords.invZ	= delta3Small.m_WindowCoords.invZ	- dInvZdX; 
				delta3Big.m_WindowCoords.depth	= delta3Small.m_WindowCoords.depth	- dDepthdX; 

				delta3Big.m_Color.r				= delta3Small.m_Color.r				- dRdX;
				delta3Big.m_Color.g				= delta3Small.m_Color.g				- dGdX;
				delta3Big.m_Color.b				= delta3Small.m_Color.b				- dBdX;
				delta3Big.m_Color.a				= delta3Small.m_Color.a				- dAdX;
				
				delta3Big.m_TextureCoords.tu	= delta3Small.m_TextureCoords.tu	- dTuOverZdX;
				delta3Big.m_TextureCoords.tv	= delta3Small.m_TextureCoords.tv	- dTvOverZdX;

				delta3Big.m_FogDensity			= delta3Small.m_FogDensity			- dFogdX;
			}

			// ------------------------------------------------------------------
			// Stepping for right edge
			// ------------------------------------------------------------------

			EGL_Fixed xStepped1R = pos2.m_WindowCoords.x + EGL_Mul(pos1.m_WindowCoords.y - pos2.m_WindowCoords.y + yPreStep1, dXdY23);

			delta.m_WindowCoords.x = 
				xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

			// ------------------------------------------------------------------
			// initialize the x-step error
			// ------------------------------------------------------------------

			EGL_Fixed xStepError, xError;

			if (dXdY3 >= 0) {
				xStepError = dXdY3 - EGL_FixedFromInt(dXdYStep1Int);
				xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			} else {
				xStepError = -dXdY3 - EGL_FixedFromInt(dXdYStep1Int);
				xError = EGL_ONE - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			}

			// ------------------------------------------------------------------
			// Raster the top part of the triangle
			// ------------------------------------------------------------------

			for (; y < y3; ++y) {
				RasterScanLine(rasterInfo, start, delta);												

				xError += xStepError;

				if (xError >= EGL_ONE) {
					// do a big step

					xError -= EGL_ONE;

					start.m_WindowCoords.x		+= delta3Big.m_WindowCoords.x;
					start.m_Color				+= delta3Big.m_Color;
					start.m_WindowCoords.invZ	+= delta3Big.m_WindowCoords.invZ;	
					start.m_TextureCoords.tu	+= delta3Big.m_TextureCoords.tu;	
					start.m_TextureCoords.tv	+= delta3Big.m_TextureCoords.tv;	
					start.m_FogDensity			+= delta3Big.m_FogDensity;		
					start.m_WindowCoords.depth	+= delta3Big.m_WindowCoords.depth;	
				} else {
					// do a small step

					start.m_WindowCoords.x		+= delta3Small.m_WindowCoords.x;
					start.m_Color				+= delta3Small.m_Color;
					start.m_WindowCoords.invZ	+= delta3Small.m_WindowCoords.invZ;
					start.m_TextureCoords.tu	+= delta3Small.m_TextureCoords.tu;
					start.m_TextureCoords.tv	+= delta3Small.m_TextureCoords.tv;
					start.m_FogDensity			+= delta3Small.m_FogDensity;	
					start.m_WindowCoords.depth	+= delta3Small.m_WindowCoords.depth;	
				}

				delta.m_WindowCoords.x			+= dXdY23;														
																											
				rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
				rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
				rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
				rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
			}

		} else {
			// move beginning of scanline along p2->p3

			// ------------------------------------------------------------------
			// now do second part of triangle
			//
			// ------------------------------------------------------------------
			//
			// initialize start onto first pixel right off line p2->p3
			// ------------------------------------------------------------------

			EGL_Fixed xStepped2L = pos2.m_WindowCoords.x + EGL_Mul(yPreStep2, dXdY23);
			EGL_Fixed xRounded2 = EGL_NearestInt(xStepped2L);
			EGL_Fixed xPreStep2 = xRounded2 + (EGL_ONE/2) - pos2.m_WindowCoords.x;

			EdgePos start;
			start.m_WindowCoords.x = 
				xStepped2L + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

			start.m_WindowCoords.invZ	= invZ2				+ EGL_Mul(dInvZdX, xPreStep2)	+ EGL_Mul(dInvZdY, yPreStep2); 
			start.m_WindowCoords.depth	= depth2			+ EGL_Mul(dDepthdX, xPreStep2)	+ EGL_Mul(dDepthdY, yPreStep2); 

			start.m_Color.r				= pos2.m_Color.r	+ EGL_Mul(dRdX, xPreStep2)		+ EGL_Mul(dRdY, yPreStep2);
			start.m_Color.g				= pos2.m_Color.g	+ EGL_Mul(dGdX, xPreStep2)		+ EGL_Mul(dGdY, yPreStep2);
			start.m_Color.b				= pos2.m_Color.b	+ EGL_Mul(dBdX, xPreStep2)		+ EGL_Mul(dBdY, yPreStep2);
			start.m_Color.a				= pos2.m_Color.a	+ EGL_Mul(dAdX, xPreStep2)		+ EGL_Mul(dAdY, yPreStep2);
			
			start.m_TextureCoords.tu	= tuOverZ2			+ EGL_Mul(dTuOverZdX, xPreStep2) + EGL_Mul(dTuOverZdY, yPreStep2);
			start.m_TextureCoords.tv	= tvOverZ2			+ EGL_Mul(dTvOverZdX, xPreStep2) + EGL_Mul(dTvOverZdY, yPreStep2);

			start.m_FogDensity			= pos2.m_FogDensity + EGL_Mul(dFogdX, xPreStep2)	+ EGL_Mul(dFogdY, yPreStep2);

			// ------------------------------------------------------------------
			// initialize edge buffer delta2Int & delta2Frac
			// ------------------------------------------------------------------

			// determine integer x step/y
			I32 dXdYStep2Int = dXdY23 >= 0 ? EGL_IntFromFixed(dXdY23) : -EGL_IntFromFixed(-dXdY23);

			EdgePos delta23Small;

			delta23Small.m_WindowCoords.x		= dXdY23;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

			delta23Small.m_WindowCoords.invZ	= dInvZdX * dXdYStep2Int + dInvZdY; 
			delta23Small.m_WindowCoords.depth	= dDepthdX * dXdYStep2Int + dDepthdY; 

			delta23Small.m_Color.r				= dRdX * dXdYStep2Int + dRdY;
			delta23Small.m_Color.g				= dGdX * dXdYStep2Int + dGdY;
			delta23Small.m_Color.b				= dBdX * dXdYStep2Int + dBdY;
			delta23Small.m_Color.a				= dAdX * dXdYStep2Int + dAdY;
			
			delta23Small.m_TextureCoords.tu		= dTuOverZdX * dXdYStep2Int + dTuOverZdY;
			delta23Small.m_TextureCoords.tv		= dTvOverZdX * dXdYStep2Int + dTvOverZdY;

			delta23Small.m_FogDensity			= dFogdX * dXdYStep2Int + dFogdY;

			EdgePos delta23Big;

			delta23Big.m_WindowCoords.x		= dXdY23;

			if (dXdY23 >= 0) {
				delta23Big.m_WindowCoords.depth	= delta23Small.m_WindowCoords.depth		+ dDepthdX; 

				delta23Big.m_Color.r			= delta23Small.m_Color.r				+ dRdX;
				delta23Big.m_Color.g			= delta23Small.m_Color.g				+ dGdX;
				delta23Big.m_Color.b			= delta23Small.m_Color.b				+ dBdX;
				delta23Big.m_Color.a			= delta23Small.m_Color.a				+ dAdX;
				
				delta23Big.m_WindowCoords.invZ	= delta23Small.m_WindowCoords.invZ		+ dInvZdX; 
				delta23Big.m_TextureCoords.tu	= delta23Small.m_TextureCoords.tu		+ dTuOverZdX;
				delta23Big.m_TextureCoords.tv	= delta23Small.m_TextureCoords.tv		+ dTvOverZdX;

				delta23Big.m_FogDensity			= delta23Small.m_FogDensity				+ dFogdX;
			} else {
				delta23Big.m_WindowCoords.depth	= delta23Small.m_WindowCoords.depth		- dDepthdX; 

				delta23Big.m_Color.r			= delta23Small.m_Color.r				- dRdX;
				delta23Big.m_Color.g			= delta23Small.m_Color.g				- dGdX;
				delta23Big.m_Color.b			= delta23Small.m_Color.b				- dBdX;
				delta23Big.m_Color.a			= delta23Small.m_Color.a				- dAdX;
				
				delta23Big.m_WindowCoords.invZ	= delta23Small.m_WindowCoords.invZ		- dInvZdX; 
				delta23Big.m_TextureCoords.tu	= delta23Small.m_TextureCoords.tu		- dTuOverZdX;
				delta23Big.m_TextureCoords.tv	= delta23Small.m_TextureCoords.tv		- dTvOverZdX;

				delta23Big.m_FogDensity			= delta23Small.m_FogDensity				- dFogdX;
			}

			// ------------------------------------------------------------------
			// initialize the x coordinate for right edge
			// ------------------------------------------------------------------

			EGL_Fixed xStepped1R = pos1.m_WindowCoords.x + EGL_Mul(pos2.m_WindowCoords.y - pos1.m_WindowCoords.y + yPreStep2, dXdY3);
			delta.m_WindowCoords.x = 
				xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

			// ------------------------------------------------------------------
			// initialize the x-step error
			// ------------------------------------------------------------------

			//EGL_Fixed dXdYStep2Frac = dXdY23 >= 0 ? dXdY23 - EGL_FixedFromInt(dXdYStep2Int) : dXdY23 + EGL_FixedFromInt(dXdYStep2Int);

			//EGL_Fixed xStepError = EGL_Abs(dXdYStep2Frac);
			//EGL_Fixed xError = EGL_FractionFromFixed(xStepped2L);

			EGL_Fixed xStepError, xError;

			if (dXdY23 >= 0) {
				xStepError = dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
				xError = EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
			} else {
				xStepError = -dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
				xError = EGL_ONE - EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
			}

			// ------------------------------------------------------------------
			// Raster the triangle
			// ------------------------------------------------------------------

			for (; y < y3; ++y) {
				RasterScanLine(rasterInfo, start, delta);												

				xError += xStepError;

				if (xError >= EGL_ONE) {
					// do a big step

					xError -= EGL_ONE;

					start.m_WindowCoords.x		+= delta23Big.m_WindowCoords.x;
					start.m_Color				+= delta23Big.m_Color;
					start.m_WindowCoords.invZ	+= delta23Big.m_WindowCoords.invZ;	
					start.m_TextureCoords.tu	+= delta23Big.m_TextureCoords.tu;	
					start.m_TextureCoords.tv	+= delta23Big.m_TextureCoords.tv;	
					start.m_FogDensity			+= delta23Big.m_FogDensity;		
					start.m_WindowCoords.depth	+= delta23Big.m_WindowCoords.depth;	
				} else {
					// do a small step

					start.m_WindowCoords.x		+= delta23Small.m_WindowCoords.x;
					start.m_Color				+= delta23Small.m_Color;
					start.m_WindowCoords.invZ	+= delta23Small.m_WindowCoords.invZ;
					start.m_TextureCoords.tu	+= delta23Small.m_TextureCoords.tu;
					start.m_TextureCoords.tv	+= delta23Small.m_TextureCoords.tv;
					start.m_FogDensity			+= delta23Small.m_FogDensity;	
					start.m_WindowCoords.depth	+= delta23Small.m_WindowCoords.depth;	
				}

				delta.m_WindowCoords.x			+= dXdY3;	
																											
				rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
				rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
				rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
				rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
			}

		}

		return;
	}

	EGL_Fixed invDeltaY2 = EGL_Inverse(deltaY2);	
	EGL_Fixed invDeltaY3 = EGL_Inverse(deltaY3);	// deltaY3 == 0 should not occur, triangle would already have been skipped

	EGL_Fixed deltaX2	= pos2.m_WindowCoords.x - pos1.m_WindowCoords.x;
	EGL_Fixed deltaX3	= pos3.m_WindowCoords.x - pos1.m_WindowCoords.x;

	EGL_Fixed dXdY2		= EGL_Mul(deltaX2, invDeltaY2);
	EGL_Fixed dXdY3		= EGL_Mul(deltaX3, invDeltaY3);

	if (dXdY2 < dXdY3) {

		// ------------------------------------------------------------------
		// initialize start onto first pixel right off line p1->p2
		// ------------------------------------------------------------------

		EGL_Fixed xStepped1L = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY2);
		EGL_Fixed xStepped1R = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY3);
		EGL_Fixed xRounded1 = EGL_NearestInt(xStepped1L);
		EGL_Fixed xPreStep1 = xRounded1 + (EGL_ONE/2) - pos1.m_WindowCoords.x;

		EdgePos start;
		start.m_WindowCoords.x = 
			xStepped1L + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		start.m_WindowCoords.depth = depth1 + EGL_Mul(dDepthdX, xPreStep1) + EGL_Mul(dDepthdY, yPreStep1); 

		start.m_Color.r = EGL_ONE;//pos1.m_Color.r + EGL_Mul(dRdX, xPreStep1) + EGL_Mul(dRdY, yPreStep1);
		start.m_Color.g = EGL_ONE;//pos1.m_Color.g + EGL_Mul(dGdX, xPreStep1) + EGL_Mul(dGdY, yPreStep1);
		start.m_Color.b = 0;//pos1.m_Color.b + EGL_Mul(dBdX, xPreStep1) + EGL_Mul(dBdY, yPreStep1);
		start.m_Color.a = pos1.m_Color.a + EGL_Mul(dAdX, xPreStep1) + EGL_Mul(dAdY, yPreStep1);
		
		start.m_WindowCoords.invZ = invZ1 + EGL_Mul(dInvZdX, xPreStep1) + EGL_Mul(dInvZdY, yPreStep1); 
		start.m_TextureCoords.tu = tuOverZ1 + EGL_Mul(dTuOverZdX, xPreStep1) + EGL_Mul(dTuOverZdY, yPreStep1);
		start.m_TextureCoords.tv = tvOverZ1 + EGL_Mul(dTvOverZdX, xPreStep1) + EGL_Mul(dTvOverZdY, yPreStep1);

		start.m_FogDensity = pos1.m_FogDensity + EGL_Mul(dFogdX, xPreStep1) + EGL_Mul(dFogdY, yPreStep1);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep1Int = dXdY2 >= 0 ? EGL_IntFromFixed(dXdY2) : -EGL_IntFromFixed(-dXdY2);

		EdgePos delta2Small;

		delta2Small.m_WindowCoords.x = dXdY2;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

		delta2Small.m_WindowCoords.depth	= dDepthdX * dXdYStep1Int	+ dDepthdY; 

		delta2Small.m_Color.r				= dRdX * dXdYStep1Int		+ dRdY;
		delta2Small.m_Color.g				= dGdX * dXdYStep1Int		+ dGdY;
		delta2Small.m_Color.b				= dBdX * dXdYStep1Int		+ dBdY;
		delta2Small.m_Color.a				= dAdX * dXdYStep1Int		+ dAdY;
		
		delta2Small.m_WindowCoords.invZ		= dInvZdX * dXdYStep1Int	+ dInvZdY; 
		delta2Small.m_TextureCoords.tu		= dTuOverZdX * dXdYStep1Int + dTuOverZdY;
		delta2Small.m_TextureCoords.tv		= dTvOverZdX * dXdYStep1Int + dTvOverZdY;

		delta2Small.m_FogDensity			= dFogdX * dXdYStep1Int		+ dFogdY;

		EdgePos delta2Big;

		delta2Big.m_WindowCoords.x		= dXdY2;

		if (dXdY2 >= 0) {
			delta2Big.m_WindowCoords.depth	= delta2Small.m_WindowCoords.depth	+ dDepthdX; 

			delta2Big.m_Color.r				= delta2Small.m_Color.r				+ dRdX;
			delta2Big.m_Color.g				= delta2Small.m_Color.g				+ dGdX;
			delta2Big.m_Color.b				= delta2Small.m_Color.b				+ dBdX;
			delta2Big.m_Color.a				= delta2Small.m_Color.a				+ dAdX;
			
			delta2Big.m_WindowCoords.invZ	= delta2Small.m_WindowCoords.invZ	+ dInvZdX; 
			delta2Big.m_TextureCoords.tu	= delta2Small.m_TextureCoords.tu	+ dTuOverZdX;
			delta2Big.m_TextureCoords.tv	= delta2Small.m_TextureCoords.tv	+ dTvOverZdX;

			delta2Big.m_FogDensity			= delta2Small.m_FogDensity			+ dFogdX;
		} else {
			delta2Big.m_WindowCoords.depth	= delta2Small.m_WindowCoords.depth	- dDepthdX; 

			delta2Big.m_Color.r				= delta2Small.m_Color.r				- dRdX;
			delta2Big.m_Color.g				= delta2Small.m_Color.g				- dGdX;
			delta2Big.m_Color.b				= delta2Small.m_Color.b				- dBdX;
			delta2Big.m_Color.a				= delta2Small.m_Color.a				- dAdX;
			
			delta2Big.m_WindowCoords.invZ	= delta2Small.m_WindowCoords.invZ	- dInvZdX; 
			delta2Big.m_TextureCoords.tu	= delta2Small.m_TextureCoords.tu	- dTuOverZdX;
			delta2Big.m_TextureCoords.tv	= delta2Small.m_TextureCoords.tv	- dTvOverZdX;

			delta2Big.m_FogDensity			= delta2Small.m_FogDensity			- dFogdX;
		}

		// ------------------------------------------------------------------
		// initialize the x coordinate for right edge
		// ------------------------------------------------------------------

		delta.m_WindowCoords.x = 
			xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest


		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		//EGL_Fixed dXdYStep1Frac = dXdY2 >= 0 ? dXdY2 - EGL_FixedFromInt(dXdYStep1Int) : dXdY2 + EGL_FixedFromInt(dXdYStep1Int);

		//EGL_Fixed xStepError = EGL_Abs(dXdYStep1Frac);
		//EGL_Fixed xError = EGL_FractionFromFixed(xStepped1L);

		EGL_Fixed xStepError, xError;

		if (dXdY2 >= 0) {
			xStepError = dXdY2 - EGL_FixedFromInt(dXdYStep1Int);
			xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		} else {
			xStepError = -dXdY2 - EGL_FixedFromInt(dXdYStep1Int);
			xError = EGL_ONE - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		}

		// ------------------------------------------------------------------
		// Raster the top part of the triangle
		// ------------------------------------------------------------------

		for (; y < y2; ++y) {
			RasterScanLine(rasterInfo, start, delta);												

			xError += xStepError;

			if (xError >= EGL_ONE) {
				// do a big step

				xError -= EGL_ONE;

				start.m_WindowCoords.x		+= delta2Big.m_WindowCoords.x;
				start.m_Color				+= delta2Big.m_Color;
				start.m_WindowCoords.invZ	+= delta2Big.m_WindowCoords.invZ;	
				start.m_TextureCoords.tu	+= delta2Big.m_TextureCoords.tu;	
				start.m_TextureCoords.tv	+= delta2Big.m_TextureCoords.tv;	
				start.m_FogDensity			+= delta2Big.m_FogDensity;		
				start.m_WindowCoords.depth	+= delta2Big.m_WindowCoords.depth;	
			} else {
				// do a small step

				start.m_WindowCoords.x		+= delta2Small.m_WindowCoords.x;
				start.m_Color				+= delta2Small.m_Color;
				start.m_WindowCoords.invZ	+= delta2Small.m_WindowCoords.invZ;
				start.m_TextureCoords.tu	+= delta2Small.m_TextureCoords.tu;
				start.m_TextureCoords.tv	+= delta2Small.m_TextureCoords.tv;
				start.m_FogDensity			+= delta2Small.m_FogDensity;	
				start.m_WindowCoords.depth	+= delta2Small.m_WindowCoords.depth;	
			}

			delta.m_WindowCoords.x			+= dXdY3;														
																										
			rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
			rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
		}

		if (y >= y3)
			return;

		EGL_Fixed invDeltaY23 = EGL_Inverse(deltaY23);
		EGL_Fixed deltaX23	= pos3.m_WindowCoords.x - pos2.m_WindowCoords.x;
		EGL_Fixed dXdY23	= EGL_Mul(deltaX23, invDeltaY23);

		// ------------------------------------------------------------------
		// now do second part of triangle
		//
		// ------------------------------------------------------------------
		//
		// initialize start onto first pixel right off line p2->p3
		// ------------------------------------------------------------------

		EGL_Fixed xStepped2L = pos2.m_WindowCoords.x + EGL_Mul(yPreStep2, dXdY23);
		EGL_Fixed xRounded2 = EGL_NearestInt(xStepped2L);
		EGL_Fixed xPreStep2 = xRounded2 + (EGL_ONE/2) - pos2.m_WindowCoords.x;

		start.m_WindowCoords.x = 
			xStepped2L + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		start.m_WindowCoords.invZ = invZ2 + EGL_Mul(dInvZdX, xPreStep2) + EGL_Mul(dInvZdY, yPreStep2); 
		start.m_WindowCoords.depth = depth2 + EGL_Mul(dDepthdX, xPreStep2) + EGL_Mul(dDepthdY, yPreStep2); 

		start.m_Color.r = 0;//pos2.m_Color.r + EGL_Mul(dRdX, xPreStep2) + EGL_Mul(dRdY, yPreStep2);
		start.m_Color.g = 0;//pos2.m_Color.g + EGL_Mul(dGdX, xPreStep2) + EGL_Mul(dGdY, yPreStep2);
		start.m_Color.b = EGL_ONE;//pos2.m_Color.b + EGL_Mul(dBdX, xPreStep2) + EGL_Mul(dBdY, yPreStep2);
		start.m_Color.a = pos2.m_Color.a + EGL_Mul(dAdX, xPreStep2) + EGL_Mul(dAdY, yPreStep2);
		
		start.m_TextureCoords.tu = tuOverZ2 + EGL_Mul(dTuOverZdX, xPreStep2) + EGL_Mul(dTuOverZdY, yPreStep2);
		start.m_TextureCoords.tv = tvOverZ2 + EGL_Mul(dTvOverZdX, xPreStep2) + EGL_Mul(dTvOverZdY, yPreStep2);

		start.m_FogDensity = pos2.m_FogDensity + EGL_Mul(dFogdX, xPreStep2) + EGL_Mul(dFogdY, yPreStep2);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep2Int = dXdY23 >= 0 ? EGL_IntFromFixed(dXdY23) : -EGL_IntFromFixed(-dXdY23);

		EdgePos delta23Small;

		delta23Small.m_WindowCoords.x		= dXdY23;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

		delta23Small.m_WindowCoords.depth	= dDepthdX * dXdYStep2Int	+ dDepthdY; 

		delta23Small.m_Color.r				= dRdX * dXdYStep2Int		+ dRdY;
		delta23Small.m_Color.g				= dGdX * dXdYStep2Int		+ dGdY;
		delta23Small.m_Color.b				= dBdX * dXdYStep2Int		+ dBdY;
		delta23Small.m_Color.a				= dAdX * dXdYStep2Int		+ dAdY;
		
		delta23Small.m_WindowCoords.invZ	= dInvZdX * dXdYStep2Int	+ dInvZdY; 
		delta23Small.m_TextureCoords.tu		= dTuOverZdX * dXdYStep2Int + dTuOverZdY;
		delta23Small.m_TextureCoords.tv		= dTvOverZdX * dXdYStep2Int + dTvOverZdY;

		delta23Small.m_FogDensity			= dFogdX * dXdYStep2Int		+ dFogdY;

		EdgePos delta23Big;

		delta23Big.m_WindowCoords.x			= dXdY23;

		if (dXdY23 >= 0) {
			delta23Big.m_WindowCoords.depth		= delta23Small.m_WindowCoords.depth	+ dDepthdX; 

			delta23Big.m_Color.r				= delta23Small.m_Color.r			+ dRdX;
			delta23Big.m_Color.g				= delta23Small.m_Color.g			+ dGdX;
			delta23Big.m_Color.b				= delta23Small.m_Color.b			+ dBdX;
			delta23Big.m_Color.a				= delta23Small.m_Color.a			+ dAdX;
			
			delta23Big.m_WindowCoords.invZ		= delta23Small.m_WindowCoords.invZ	+ dInvZdX; 
			delta23Big.m_TextureCoords.tu		= delta23Small.m_TextureCoords.tu	+ dTuOverZdX;
			delta23Big.m_TextureCoords.tv		= delta23Small.m_TextureCoords.tv	+ dTvOverZdX;

			delta23Big.m_FogDensity				= delta23Small.m_FogDensity			+ dFogdX;
		} else {
			delta23Big.m_WindowCoords.depth		= delta23Small.m_WindowCoords.depth	- dDepthdX; 

			delta23Big.m_Color.r				= delta23Small.m_Color.r			- dRdX;
			delta23Big.m_Color.g				= delta23Small.m_Color.g			- dGdX;
			delta23Big.m_Color.b				= delta23Small.m_Color.b			- dBdX;
			delta23Big.m_Color.a				= delta23Small.m_Color.a			- dAdX;
			
			delta23Big.m_WindowCoords.invZ		= delta23Small.m_WindowCoords.invZ	- dInvZdX; 
			delta23Big.m_TextureCoords.tu		= delta23Small.m_TextureCoords.tu	- dTuOverZdX;
			delta23Big.m_TextureCoords.tv		= delta23Small.m_TextureCoords.tv	- dTvOverZdX;

			delta23Big.m_FogDensity				= delta23Small.m_FogDensity			- dFogdX;
		}

		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		//EGL_Fixed dXdYStep2Frac = dXdY23 >= 0 ? dXdY23 - EGL_FixedFromInt(dXdYStep2Int) : dXdY23 + EGL_FixedFromInt(dXdYStep2Int);

		//xStepError = EGL_Abs(dXdYStep2Frac);
		//xError = EGL_FractionFromFixed(xStepped2L);

		if (dXdY23 >= 0) {
			xStepError = dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
			xError = EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
		} else {
			xStepError = -dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
			xError = EGL_ONE - EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
		}


		// ------------------------------------------------------------------
		// Raster the bottom part of the triangle
		// ------------------------------------------------------------------

		for (; y < y3; ++y) {
			RasterScanLine(rasterInfo, start, delta);												

			xError += xStepError;

			if (xError >= EGL_ONE) {
				// do a big step

				xError -= EGL_ONE;

				start.m_WindowCoords.x		+= delta23Big.m_WindowCoords.x;
				start.m_Color				+= delta23Big.m_Color;
				start.m_WindowCoords.invZ	+= delta23Big.m_WindowCoords.invZ;	
				start.m_TextureCoords.tu	+= delta23Big.m_TextureCoords.tu;	
				start.m_TextureCoords.tv	+= delta23Big.m_TextureCoords.tv;	
				start.m_FogDensity			+= delta23Big.m_FogDensity;		
				start.m_WindowCoords.depth	+= delta23Big.m_WindowCoords.depth;	
			} else {
				// do a small step

				start.m_WindowCoords.x		+= delta23Small.m_WindowCoords.x;
				start.m_Color				+= delta23Small.m_Color;
				start.m_WindowCoords.invZ	+= delta23Small.m_WindowCoords.invZ;
				start.m_TextureCoords.tu	+= delta23Small.m_TextureCoords.tu;
				start.m_TextureCoords.tv	+= delta23Small.m_TextureCoords.tv;
				start.m_FogDensity			+= delta23Small.m_FogDensity;	
				start.m_WindowCoords.depth	+= delta23Small.m_WindowCoords.depth;	
			}

			delta.m_WindowCoords.x			+= dXdY3;	
																										
			rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
			rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
		}

	} else /* dXdY2 >= dXdY3) */ {

		// ------------------------------------------------------------------
		// initialize start onto first pixel right off line p1->p3
		// ------------------------------------------------------------------

		EGL_Fixed xStepped1L = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY3);
		EGL_Fixed xRounded1 = EGL_NearestInt(xStepped1L);
		EGL_Fixed xPreStep1 = xRounded1 + (EGL_ONE/2) - pos1.m_WindowCoords.x;

		EdgePos start;

		start.m_WindowCoords.x		= xStepped1L + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		start.m_WindowCoords.depth	= depth1			+ EGL_Mul(dDepthdX, xPreStep1)	+ EGL_Mul(dDepthdY, yPreStep1); 

		start.m_Color.r				= EGL_ONE;//pos1.m_Color.r + EGL_Mul(dRdX, xPreStep1) + EGL_Mul(dRdY, yPreStep1);
		start.m_Color.g				= 0;//pos1.m_Color.g + EGL_Mul(dGdX, xPreStep1) + EGL_Mul(dGdY, yPreStep1);
		start.m_Color.b				= EGL_ONE;//pos1.m_Color.b + EGL_Mul(dBdX, xPreStep1) + EGL_Mul(dBdY, yPreStep1);
		start.m_Color.a				= pos1.m_Color.a	+ EGL_Mul(dAdX, xPreStep1)	+ EGL_Mul(dAdY, yPreStep1);
		
		start.m_WindowCoords.invZ	= invZ1				+ EGL_Mul(dInvZdX, xPreStep1)	+ EGL_Mul(dInvZdY, yPreStep1); 
		start.m_TextureCoords.tu	= tuOverZ1			+ EGL_Mul(dTuOverZdX, xPreStep1) + EGL_Mul(dTuOverZdY, yPreStep1);
		start.m_TextureCoords.tv	= tvOverZ1			+ EGL_Mul(dTvOverZdX, xPreStep1) + EGL_Mul(dTvOverZdY, yPreStep1);

		start.m_FogDensity			= pos1.m_FogDensity + EGL_Mul(dFogdX, xPreStep1) + EGL_Mul(dFogdY, yPreStep1);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep1Int = dXdY3 >= 0 ? EGL_IntFromFixed(dXdY3) : -EGL_IntFromFixed(-dXdY3);

		EdgePos delta3Small;

		delta3Small.m_WindowCoords.x		= dXdY3;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

		delta3Small.m_WindowCoords.depth	= dDepthdX * dXdYStep1Int	+ dDepthdY; 

		delta3Small.m_Color.r				= dRdX * dXdYStep1Int		+ dRdY;
		delta3Small.m_Color.g				= dGdX * dXdYStep1Int		+ dGdY;
		delta3Small.m_Color.b				= dBdX * dXdYStep1Int		+ dBdY;
		delta3Small.m_Color.a				= dAdX * dXdYStep1Int		+ dAdY;
		
		delta3Small.m_WindowCoords.invZ		= dInvZdX * dXdYStep1Int	+ dInvZdY; 
		delta3Small.m_TextureCoords.tu		= dTuOverZdX * dXdYStep1Int + dTuOverZdY;
		delta3Small.m_TextureCoords.tv		= dTvOverZdX * dXdYStep1Int + dTvOverZdY;

		delta3Small.m_FogDensity			= dFogdX * dXdYStep1Int		+ dFogdY;

		EdgePos delta3Big;

		delta3Big.m_WindowCoords.x		= dXdY3;

		if (dXdY3 >= 0) {
			delta3Big.m_WindowCoords.depth	= delta3Small.m_WindowCoords.depth	+ dDepthdX; 

			delta3Big.m_Color.r				= delta3Small.m_Color.r				+ dRdX;
			delta3Big.m_Color.g				= delta3Small.m_Color.g				+ dGdX;
			delta3Big.m_Color.b				= delta3Small.m_Color.b				+ dBdX;
			delta3Big.m_Color.a				= delta3Small.m_Color.a				+ dAdX;
			
			delta3Big.m_WindowCoords.invZ	= delta3Small.m_WindowCoords.invZ	+ dInvZdX; 
			delta3Big.m_TextureCoords.tu	= delta3Small.m_TextureCoords.tu	+ dTuOverZdX;
			delta3Big.m_TextureCoords.tv	= delta3Small.m_TextureCoords.tv	+ dTvOverZdX;

			delta3Big.m_FogDensity			= delta3Small.m_FogDensity			+ dFogdX;
		} else {
			delta3Big.m_WindowCoords.depth	= delta3Small.m_WindowCoords.depth	- dDepthdX; 

			delta3Big.m_Color.r				= delta3Small.m_Color.r				- dRdX;
			delta3Big.m_Color.g				= delta3Small.m_Color.g				- dGdX;
			delta3Big.m_Color.b				= delta3Small.m_Color.b				- dBdX;
			delta3Big.m_Color.a				= delta3Small.m_Color.a				- dAdX;
			
			delta3Big.m_WindowCoords.invZ	= delta3Small.m_WindowCoords.invZ	- dInvZdX; 
			delta3Big.m_TextureCoords.tu	= delta3Small.m_TextureCoords.tu	- dTuOverZdX;
			delta3Big.m_TextureCoords.tv	= delta3Small.m_TextureCoords.tv	- dTvOverZdX;

			delta3Big.m_FogDensity			= delta3Small.m_FogDensity			- dFogdX;
		}

		// ------------------------------------------------------------------
		// initialize the x coordinate for right edge
		// ------------------------------------------------------------------

		EGL_Fixed xStepped1R = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY2);
		delta.m_WindowCoords.x = 
			xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		//EGL_Fixed dXdYStep1Frac = dXdY3 >= 0 ? dXdY3 - EGL_FixedFromInt(dXdYStep1Int) : dXdY3 + EGL_FixedFromInt(dXdYStep1Int);

		//EGL_Fixed xStepError = EGL_Abs(dXdYStep1Frac);
		//EGL_Fixed xError = EGL_FractionFromFixed(xStepped1L);

		EGL_Fixed xStepError, xError;

		if (dXdY3 >= 0) {
			xStepError = dXdY3 - EGL_FixedFromInt(dXdYStep1Int);
			xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		} else {
			xStepError = -dXdY3 - EGL_FixedFromInt(dXdYStep1Int);
			xError = EGL_ONE - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		}

		// ------------------------------------------------------------------
		// Raster the top part of the triangle
		// ------------------------------------------------------------------

		for (; y < y2; ++y) {
			RasterScanLine(rasterInfo, start, delta);												

			xError += xStepError;

			if (xError >= EGL_ONE) {
				// do a big step

				xError -= EGL_ONE;

				start.m_WindowCoords.x		+= delta3Big.m_WindowCoords.x;
				start.m_Color				+= delta3Big.m_Color;
				start.m_WindowCoords.invZ	+= delta3Big.m_WindowCoords.invZ;	
				start.m_TextureCoords.tu	+= delta3Big.m_TextureCoords.tu;	
				start.m_TextureCoords.tv	+= delta3Big.m_TextureCoords.tv;	
				start.m_FogDensity			+= delta3Big.m_FogDensity;		
				start.m_WindowCoords.depth	+= delta3Big.m_WindowCoords.depth;	
			} else {
				// do a small step

				start.m_WindowCoords.x		+= delta3Small.m_WindowCoords.x;
				start.m_Color				+= delta3Small.m_Color;
				start.m_WindowCoords.invZ	+= delta3Small.m_WindowCoords.invZ;
				start.m_TextureCoords.tu	+= delta3Small.m_TextureCoords.tu;
				start.m_TextureCoords.tv	+= delta3Small.m_TextureCoords.tv;
				start.m_FogDensity			+= delta3Small.m_FogDensity;	
				start.m_WindowCoords.depth	+= delta3Small.m_WindowCoords.depth;	
			}

			delta.m_WindowCoords.x			+= dXdY2;														
																										
			rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
			rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
		}

		if (y >= y3)
			return;

		EGL_Fixed invDeltaY23 = EGL_Inverse(deltaY23);
		EGL_Fixed deltaX23	= pos3.m_WindowCoords.x - pos2.m_WindowCoords.x;
		EGL_Fixed dXdY23	= EGL_Mul(deltaX23, invDeltaY23);

		// ------------------------------------------------------------------
		// now do second part of triangle
		//
		// ------------------------------------------------------------------
		//
		// initialize end behind first pixel right off line p2->p3
		// ------------------------------------------------------------------

		EGL_Fixed xStepped2R = pos2.m_WindowCoords.x + EGL_Mul(yPreStep2, dXdY23);

		delta.m_WindowCoords.x = 
			xStepped2R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		for (; y < y3; ++y) {
			RasterScanLine(rasterInfo, start, delta);												

			xError += xStepError;

			if (xError >= EGL_ONE) {
				// do a big step

				xError -= EGL_ONE;

				start.m_WindowCoords.x		+= delta3Big.m_WindowCoords.x;
				start.m_Color				+= delta3Big.m_Color;
				start.m_WindowCoords.invZ	+= delta3Big.m_WindowCoords.invZ;	
				start.m_TextureCoords.tu	+= delta3Big.m_TextureCoords.tu;	
				start.m_TextureCoords.tv	+= delta3Big.m_TextureCoords.tv;	
				start.m_FogDensity			+= delta3Big.m_FogDensity;		
				start.m_WindowCoords.depth	+= delta3Big.m_WindowCoords.depth;	
			} else {
				// do a small step

				start.m_WindowCoords.x		+= delta3Small.m_WindowCoords.x;
				start.m_Color				+= delta3Small.m_Color;
				start.m_WindowCoords.invZ	+= delta3Small.m_WindowCoords.invZ;
				start.m_TextureCoords.tu	+= delta3Small.m_TextureCoords.tu;
				start.m_TextureCoords.tv	+= delta3Small.m_TextureCoords.tv;
				start.m_FogDensity			+= delta3Small.m_FogDensity;	
				start.m_WindowCoords.depth	+= delta3Small.m_WindowCoords.depth;	
			}

			delta.m_WindowCoords.x			+= dXdY23;	
																										
			rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;											
			rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;										
			rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
		}
	}

}


