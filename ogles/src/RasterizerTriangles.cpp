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

//#define NO_COMPILE
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


#define SetupEdge(from, fromDepth, to, toDepth, result)													\
	{																									\
		EGL_Fixed delta = to.m_WindowCoords.y - from.m_WindowCoords.y;									\
		EGL_Fixed invDelta;																				\
																										\
		if (delta) {																					\
			invDelta = EGL_Inverse(delta);																\
			result.m_WindowCoords.x = EGL_Mul(to.m_WindowCoords.x - from.m_WindowCoords.x, invDelta);	\
			result.m_Color = (to.m_Color - from.m_Color) * invDelta;									\
			result.m_FogDensity = EGL_Mul(from.m_FogDensity - to.m_FogDensity, invDelta);				\
			result.m_WindowCoords.depth = EGL_Mul(toDepth - fromDepth, invDelta);						\
			result.m_WindowCoords.invZ = EGL_Mul(to.m_WindowCoords.invZ - from.m_WindowCoords.invZ, invDelta);	\
		} else {																						\
			invDelta = 0;																				\
			result.m_WindowCoords.x = to.m_WindowCoords.x - from.m_WindowCoords.x;						\
			result.m_Color = FractionalColor(0, 0, 0, 0);												\
			result.m_FogDensity = 0;																	\
			result.m_WindowCoords.depth = 0;															\
		}																								\
																										\
		result.m_TextureCoords.tu =																		\
			EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, to.m_WindowCoords.invZ) -							\
					EGL_Mul(from.m_TextureCoords.tu, from.m_WindowCoords.invZ), invDelta);				\
		result.m_TextureCoords.tv =																		\
			EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, to.m_WindowCoords.invZ) -							\
					EGL_Mul(from.m_TextureCoords.tv, from.m_WindowCoords.invZ), invDelta);				\
	}																				


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

	SOLVE_X(dRdX, dRdY, m_Color.r, invDenominator);
	SOLVE_X(dGdX, dGdY, m_Color.g, invDenominator);
	SOLVE_X(dBdX, dBdY, m_Color.b, invDenominator);
	SOLVE_X(dAdX, dAdY, m_Color.a, invDenominator);

	SOLVE_X(dFogdX, dFogdY, m_FogDensity, invDenominator);
	SOLVE_XY(dDepthdX, dDepthdY, m_WindowCoords.depth, invDenominator);

	SOLVE_X(dInvZdX, dInvZdY, m_WindowCoords.invZ, invDenominator);

	EGL_Fixed tuOverZ1 = EGL_Mul(pos1.m_TextureCoords.tu, pos1.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ2 = EGL_Mul(pos2.m_TextureCoords.tu, pos2.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ3 = EGL_Mul(pos3.m_TextureCoords.tu, pos3.m_WindowCoords.invZ);

	EGL_Fixed tvOverZ1 = EGL_Mul(pos1.m_TextureCoords.tv, pos1.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ2 = EGL_Mul(pos2.m_TextureCoords.tv, pos2.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ3 = EGL_Mul(pos3.m_TextureCoords.tv, pos3.m_WindowCoords.invZ);

	SOLVE_PARAM_X(dTuOverZdX, dTuOverZdY, tuOverZ1, tuOverZ2, tuOverZ3, invDenominator);
	SOLVE_PARAM_X(dTvOverZdX, dTvOverZdY, tvOverZ1, tvOverZ2, tvOverZ3, invDenominator);

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
	
	EGL_Fixed invZ1 = pos1.m_WindowCoords.invZ;
	EGL_Fixed invZ2 = pos2.m_WindowCoords.invZ;
	EGL_Fixed invZ3 = pos3.m_WindowCoords.invZ;

	EdgePos start;
	start.m_WindowCoords.x = pos1.m_WindowCoords.x + (EGL_ONE/2);
	delta.m_WindowCoords.x = pos1.m_WindowCoords.x + (EGL_ONE/2);
	start.m_WindowCoords.invZ = invZ1;
	start.m_WindowCoords.depth = depth1;
	start.m_Color = pos1.m_Color;
	start.m_TextureCoords.tu = EGL_Mul(pos1.m_TextureCoords.tu, invZ1);
	start.m_TextureCoords.tv = EGL_Mul(pos1.m_TextureCoords.tv, invZ1);
	start.m_FogDensity = EGL_ONE - pos1.m_FogDensity;

	// set up the triangle
	// init start, end, deltas
	EdgePos delta2, delta3, delta23;

	SetupEdge(pos1, depth1, pos2, depth2, delta2);
	SetupEdge(pos1, depth1, pos3, depth3, delta3);
	SetupEdge(pos2, depth2, pos3, depth3, delta23);

	I32 yStart = EGL_Round(pos1.m_WindowCoords.y);
	I32 yEnd = EGL_Round(pos2.m_WindowCoords.y);
	I32 y;

	y = yStart;

	RasterInfo rasterInfo(m_Surface, y);

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

	if (m_State->m_ScissorTestEnabled) {

		// TO DO: This can be optimized, but we'll address it when we convert the whole
		// function to the plane equation approach

		I32 yScissorStart = m_State->m_ScissorY;
		I32 yScissorEnd = yScissorStart + m_State->m_ScissorHeight;

		if (delta2.m_WindowCoords.x < delta3.m_WindowCoords.x) {

			if (y < yEnd) {
				AdvanceScanline(rasterInfo, start, delta, delta2, delta3.m_WindowCoords.x);
				++y;
				TrianglePartScissor(rasterInfo, start, delta, delta2, delta3.m_WindowCoords.x,
					y, yEnd, yScissorStart, yScissorEnd);
			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			start.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			start.m_WindowCoords.invZ = invZ2;
			start.m_WindowCoords.depth = depth2;
			start.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			start.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			start.m_Color = pos2.m_Color;
			start.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			TrianglePartScissor(rasterInfo, start, delta, delta23, delta3.m_WindowCoords.x,
				y, yEnd, yScissorStart, yScissorEnd);

		} else {
			if (y < yEnd) {
				AdvanceScanline(rasterInfo, start, delta, delta3, delta2.m_WindowCoords.x);
				++y;
				TrianglePartScissor(rasterInfo, start, delta, delta3, delta2.m_WindowCoords.x, 
					y, yEnd, yScissorStart, yScissorEnd);
			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);
			delta.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);

			TrianglePartScissor(rasterInfo, start, delta, delta3, delta23.m_WindowCoords.x, 
				y, yEnd, yScissorStart, yScissorEnd);

		}
	} else {
		if (delta2.m_WindowCoords.x < delta3.m_WindowCoords.x) {

			if (y < yEnd) {
				AdvanceScanline(rasterInfo, start, delta, delta2, delta3.m_WindowCoords.x);
				++y;
				TrianglePart(rasterInfo, start, delta, delta2, delta3.m_WindowCoords.x, y, yEnd);
			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);

			start.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);
			start.m_WindowCoords.invZ = invZ2;
			start.m_WindowCoords.depth = depth2;
			start.m_TextureCoords.tu = EGL_Mul(pos2.m_TextureCoords.tu, invZ2);
			start.m_TextureCoords.tv = EGL_Mul(pos2.m_TextureCoords.tv, invZ2);
			start.m_Color = pos2.m_Color;
			start.m_FogDensity = EGL_ONE - pos2.m_FogDensity;

			TrianglePart(rasterInfo, start, delta, delta23, delta3.m_WindowCoords.x, y, yEnd);
		} else {

			if (y < yEnd) {
				AdvanceScanline(rasterInfo, start, delta, delta3, delta2.m_WindowCoords.x);
				++y;
				TrianglePart(rasterInfo, start, delta, delta3, delta2.m_WindowCoords.x, y, yEnd);
			}

			yEnd = EGL_Round(pos3.m_WindowCoords.y);
			delta.m_WindowCoords.x = pos2.m_WindowCoords.x + (EGL_ONE/2);

			TrianglePart(rasterInfo, start, delta, delta3, delta23.m_WindowCoords.x, y, yEnd);
		}
	}
}

