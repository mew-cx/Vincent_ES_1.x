// ==========================================================================
//
// RasterizerTraingles.cpp	Rasterizer Class for 3D Rendering Library
//
// The rasterizer converts transformed and lit primitives and creates a 
// raster image in the current rendering surface.
//
// This files contains the triangle rasterization code, which was
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

	inline EGL_Fixed Det2x2NoShift(EGL_Fixed a11, EGL_Fixed a12, EGL_Fixed a21, EGL_Fixed a22) {
		I64 first = static_cast<I64>(a11) * static_cast<I64>(a22);
		I64 second = static_cast<I64>(a12) * static_cast<I64>(a21);

		return (EGL_Fixed) ((first - second)  >> EGL_PRECISION);
	}

}


#define SOLVE_PARAM_XY(val, p1, p2, p3, scale) \
	grad.dx.val = EGL_Mul(																\
			Det2x2NoShift(																		\
				p2 - p1, (pos2.m_WindowCoords.y - pos1.m_WindowCoords.y) >> DET_SHIFT,					\
				p3 - p1, (pos3.m_WindowCoords.y - pos1.m_WindowCoords.y) >> DET_SHIFT) >> DET_SHIFT,				\
			scale);	\
	grad.dy.val = EGL_Mul(																\
			Det2x2NoShift(																		\
				(pos2.m_WindowCoords.x - pos1.m_WindowCoords.x) >> DET_SHIFT, p2 - p1,					\
				(pos3.m_WindowCoords.x - pos1.m_WindowCoords.x) >> DET_SHIFT, p3 - p1) >> DET_SHIFT,				\
			scale)


#define SOLVE_XY(param, scale) \
	SOLVE_PARAM_XY(param, pos1.param, pos2.param, pos3.param, scale)


// ---------------------------------------------------------------------------
// Prepare rasterizer object for triangles
// ---------------------------------------------------------------------------

void Rasterizer :: PrepareTriangle() {
	if (m_State->m_TextureEnabled) {
		SetTexture(m_Texture);
	}

	m_ScanlineFunction = 
		m_FunctionCache->GetFunction(FunctionCache::FunctionTypeScanline, 
									 *m_State);

	m_RasterInfo.MipmapLevel = 0;
}


// --------------------------------------------------------------------------
// number of pixels done with linear interpolation
// --------------------------------------------------------------------------


#define LOG_LINEAR_SPAN 3					// logarithm of value base 2
#define LINEAR_SPAN (1 << LOG_LINEAR_SPAN)	// must be power of 2

//#define NO_COMPILE
#if !defined(NO_COMPILE) && (defined(ARM) || defined(_ARM_))

inline void Rasterizer :: RasterScanLine(RasterInfo & rasterInfo, const EdgePos & start, const EdgePos & end) {
	m_ScanlineFunction(&rasterInfo, &start, &end);
}

#else 

inline void Rasterizer :: RasterScanLine(RasterInfo & rasterInfo, const EdgePos & start, const EdgePos & delta) {

	// In the edge buffer, z, tu and tv are actually divided by w

	FractionalColor baseColor = start.m_Color;

	if (!(delta.m_WindowCoords.x - start.m_WindowCoords.x)) {
		return;
	}

	const FractionalColor& colorIncrement = delta.m_Color;

	EGL_Fixed deltaInvZ = delta.m_WindowCoords.invZ;
	EGL_Fixed deltaInvU = delta.m_TextureCoords.tu;
	EGL_Fixed deltaInvV = delta.m_TextureCoords.tv;

	EGL_Fixed deltaInvDu = delta.m_TextureCoords.dtudy;
	EGL_Fixed deltaInvDv = delta.m_TextureCoords.dtvdy;

	EGL_Fixed deltaFog = delta.m_FogDensity;
	EGL_Fixed deltaDepth = delta.m_WindowCoords.depth;

	EGL_Fixed invTu = start.m_TextureCoords.tu;
	EGL_Fixed dTuDxOverInvZ2 = start.m_TextureCoords.dtudx;
	EGL_Fixed dTuDyOverInvZ2 = start.m_TextureCoords.dtudy;

	EGL_Fixed invTv = start.m_TextureCoords.tv;
	EGL_Fixed dTvDxOverInvZ2 = start.m_TextureCoords.dtvdx;
	EGL_Fixed dTvDyOverInvZ2 = start.m_TextureCoords.dtvdy;

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

		// to get started, do mipmap selection at beginning of span

		if (m_UseMipmap) {
			EGL_Fixed z2 = EGL_Mul(z << 4, z << 4);
			EGL_Fixed maxDu = EGL_Max(EGL_Abs(dTuDxOverInvZ2), EGL_Abs(dTuDyOverInvZ2)) >> (16 - m_Texture->GetTexture(0)->GetLogWidth());
			EGL_Fixed maxDv = EGL_Max(EGL_Abs(dTvDxOverInvZ2), EGL_Abs(dTvDyOverInvZ2)) >> (16 - m_Texture->GetTexture(0)->GetLogHeight());

			//EGL_Fixed maxD = EGL_Max(maxDu, maxDv);
			EGL_Fixed maxD = maxDu + maxDv;
			//I64 rho64 = ((I64) EGL_Mul(z2, EGL_FixedFromFloat(1/sqrt(2.0f)) + 1)) * ((I64) maxD);
			EGL_Fixed rho = EGL_Mul(z2, maxD);

			// we start with nearest/minification only selection; will add LINEAR later

			rasterInfo.MipmapLevel = EGL_Min(Log2(rho), rasterInfo.MaxMipmapLevel);

			dTuDyOverInvZ2 += deltaInvDu << LOG_LINEAR_SPAN;
			dTvDyOverInvZ2 += deltaInvDv << LOG_LINEAR_SPAN;
		}

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

		do {
			Fragment(&rasterInfo, x, depth, tu, tv, baseColor, fogDensity);

			baseColor += colorIncrement;
			depth += deltaDepth;
			fogDensity += deltaFog;
			z += deltaZ;
			tu += deltaTu;
			tv += deltaTv;
			++x;
		} while (--count);
	}

	if (x != xEnd) {

		I32 deltaX = xEnd - x;

		if (m_UseMipmap) {
			EGL_Fixed z2 = EGL_Mul(z << 4, z << 4);
			EGL_Fixed maxDu = EGL_Max(EGL_Abs(dTuDxOverInvZ2), EGL_Abs(dTuDyOverInvZ2)) >> (16 - m_Texture->GetTexture(0)->GetLogWidth());
			EGL_Fixed maxDv = EGL_Max(EGL_Abs(dTvDxOverInvZ2), EGL_Abs(dTvDyOverInvZ2)) >> (16 - m_Texture->GetTexture(0)->GetLogHeight());

			//EGL_Fixed maxD = EGL_Max(maxDu, maxDv);
			EGL_Fixed maxD = maxDu + maxDv;
			//I64 rho64 = ((I64) EGL_Mul(z2, EGL_FixedFromFloat(1/sqrt(2.0f)) + 1)) * ((I64) maxD);
			EGL_Fixed rho = EGL_Mul(z2, maxD);

			// we start with nearest/minification only selection; will add LINEAR later

			rasterInfo.MipmapLevel = EGL_Min(Log2(rho), rasterInfo.MaxMipmapLevel);
		}

		EGL_Fixed endZ = EGL_Inverse(invZ + deltaX * deltaInvZ);
		EGL_Fixed endTu = EGL_Mul(invTu + deltaX * deltaInvU, endZ);
		EGL_Fixed endTv = EGL_Mul(invTv + deltaX * deltaInvV, endZ);

		EGL_Fixed invSpan = EGL_Inverse(EGL_FixedFromInt(xEnd - x));

		EGL_Fixed deltaZ = EGL_Mul(endZ - z, invSpan);
		EGL_Fixed deltaTu = EGL_Mul(endTu - tu, invSpan);
		EGL_Fixed deltaTv = EGL_Mul(endTv - tv, invSpan);

		for (; x < xEnd; ++x) {

			Fragment(&rasterInfo, x, depth, tu, tv, baseColor, fogDensity);

			baseColor += colorIncrement;
			depth += deltaDepth;
			tu += deltaTu;
			tv += deltaTv;
			fogDensity += deltaFog;
		}
	}
}
#endif


namespace {
	inline void InitScanlineStart(EdgePos & start, const Gradients & grad,
		EGL_Fixed xPreStep, EGL_Fixed yPreStep,
		const FractionalColor& color, EGL_Fixed fog, EGL_Fixed depth, EGL_Fixed invZ, EGL_Fixed tuOverZ, EGL_Fixed tvOverZ) {
		start.m_WindowCoords.invZ = 
			invZ		+ EGL_Mul(grad.dx.m_WindowCoords.invZ, xPreStep)	
						+ EGL_Mul(grad.dy.m_WindowCoords.invZ, yPreStep); 

		start.m_WindowCoords.depth = 
			depth		+ EGL_Mul(grad.dx.m_WindowCoords.depth, xPreStep)	
						+ EGL_Mul(grad.dy.m_WindowCoords.depth, yPreStep); 

		start.m_Color.r	= 
			color.r		+ EGL_Mul(grad.dx.m_Color.r, xPreStep)				
						+ EGL_Mul(grad.dy.m_Color.r, yPreStep);

		start.m_Color.g	= 
			color.g		+ EGL_Mul(grad.dx.m_Color.g, xPreStep)				
						+ EGL_Mul(grad.dy.m_Color.g, yPreStep);

		start.m_Color.b	= 
			color.b		+ EGL_Mul(grad.dx.m_Color.b, xPreStep)				
						+ EGL_Mul(grad.dy.m_Color.b, yPreStep);

		start.m_Color.a	= 
			color.a		+ EGL_Mul(grad.dx.m_Color.a, xPreStep)				
						+ EGL_Mul(grad.dy.m_Color.a, yPreStep);
		
		start.m_TextureCoords.tu = 
			tuOverZ		+ EGL_Mul(grad.dx.m_TextureCoords.tu, xPreStep)		
						+ EGL_Mul(grad.dy.m_TextureCoords.tu, yPreStep);

		start.m_TextureCoords.tv = 
			tvOverZ		+ EGL_Mul(grad.dx.m_TextureCoords.tv, xPreStep)		
						+ EGL_Mul(grad.dy.m_TextureCoords.tv, yPreStep);

		// ------------------------------------------------------------------
		// Determine partial derivatives of texture functions, 
		// see eq. (2) and (3) in
		// J. P. Ewins et al (1998), 
		// "MIP-Map Level Selection for Texture Mapping",
		// IEEE Transactions on Visualization and Computer Graphics, 
		// Vol 4, No. 4
		// ------------------------------------------------------------------

		start.m_TextureCoords.dtudx = 
			Det2x2(grad.dx.m_TextureCoords.tu, tuOverZ, grad.dx.m_WindowCoords.invZ, invZ) 
						+ EGL_Mul(grad.dy.m_TextureCoords.dtudx, yPreStep);

		start.m_TextureCoords.dtudy = 
			Det2x2(grad.dx.m_TextureCoords.tv, tvOverZ, grad.dx.m_WindowCoords.invZ, invZ)
						+ EGL_Mul(grad.dx.m_TextureCoords.dtudy, xPreStep);

		start.m_TextureCoords.dtvdx = 
			Det2x2(grad.dy.m_TextureCoords.tu, tuOverZ, grad.dy.m_WindowCoords.invZ, invZ)
						+ EGL_Mul(grad.dy.m_TextureCoords.dtvdx, yPreStep);

		start.m_TextureCoords.dtvdy = 
			Det2x2(grad.dy.m_TextureCoords.tv, tvOverZ, grad.dy.m_WindowCoords.invZ, invZ)
						+ EGL_Mul(grad.dx.m_TextureCoords.dtvdy, xPreStep);

		start.m_FogDensity = 
			fog			+ EGL_Mul(grad.dx.m_FogDensity, xPreStep)			
						+ EGL_Mul(grad.dy.m_FogDensity, yPreStep);
	}


	inline void InitScanlineDeltas(EdgePos & deltaSmall, EdgePos & deltaBig,
								   const Gradients & gradients, 
								   EGL_Fixed dxdy, I32 dXdYStepInt) {

		deltaSmall.m_WindowCoords.x		= dxdy;	// x offset is stepped for each line (could consider removing fractional part from scanline function)

		deltaSmall.m_WindowCoords.depth	= gradients.dx.m_WindowCoords.depth * dXdYStepInt	+ gradients.dy.m_WindowCoords.depth; 

		deltaSmall.m_Color.r			= gradients.dx.m_Color.r * dXdYStepInt		+ gradients.dy.m_Color.r;
		deltaSmall.m_Color.g			= gradients.dx.m_Color.g * dXdYStepInt		+ gradients.dy.m_Color.g;
		deltaSmall.m_Color.b			= gradients.dx.m_Color.b * dXdYStepInt		+ gradients.dy.m_Color.b;
		deltaSmall.m_Color.a			= gradients.dx.m_Color.a * dXdYStepInt		+ gradients.dy.m_Color.a;
		
		deltaSmall.m_WindowCoords.invZ	= gradients.dx.m_WindowCoords.invZ * dXdYStepInt	+ gradients.dy.m_WindowCoords.invZ; 
		deltaSmall.m_TextureCoords.tu	= gradients.dx.m_TextureCoords.tu * dXdYStepInt		+ gradients.dy.m_TextureCoords.tu;
		deltaSmall.m_TextureCoords.tv	= gradients.dx.m_TextureCoords.tv * dXdYStepInt		+ gradients.dy.m_TextureCoords.tv;
		deltaSmall.m_TextureCoords.dtudx=													  gradients.dy.m_TextureCoords.dtudx;; 
		deltaSmall.m_TextureCoords.dtudy= gradients.dx.m_TextureCoords.dtudy * dXdYStepInt;
		deltaSmall.m_TextureCoords.dtvdx=													  gradients.dy.m_TextureCoords.dtvdx;
		deltaSmall.m_TextureCoords.dtvdy= gradients.dx.m_TextureCoords.dtvdy * dXdYStepInt;

		deltaSmall.m_FogDensity			= gradients.dx.m_FogDensity * dXdYStepInt	+ gradients.dy.m_FogDensity;

		deltaBig.m_WindowCoords.x		= dxdy;

		if (dxdy >= 0) {
			deltaBig.m_WindowCoords.depth	= deltaSmall.m_WindowCoords.depth	+ gradients.dx.m_WindowCoords.depth; 

			deltaBig.m_Color.r				= deltaSmall.m_Color.r				+ gradients.dx.m_Color.r;
			deltaBig.m_Color.g				= deltaSmall.m_Color.g				+ gradients.dx.m_Color.g;
			deltaBig.m_Color.b				= deltaSmall.m_Color.b				+ gradients.dx.m_Color.b;
			deltaBig.m_Color.a				= deltaSmall.m_Color.a				+ gradients.dx.m_Color.a;
			
			deltaBig.m_WindowCoords.invZ	= deltaSmall.m_WindowCoords.invZ	+ gradients.dx.m_WindowCoords.invZ; 
			deltaBig.m_TextureCoords.tu		= deltaSmall.m_TextureCoords.tu		+ gradients.dx.m_TextureCoords.tu;
			deltaBig.m_TextureCoords.tv		= deltaSmall.m_TextureCoords.tv		+ gradients.dx.m_TextureCoords.tv;
			deltaBig.m_TextureCoords.dtudx	= deltaSmall.m_TextureCoords.dtudx;
			deltaBig.m_TextureCoords.dtudy	= deltaSmall.m_TextureCoords.dtudy	+ gradients.dx.m_TextureCoords.dtudy;
			deltaBig.m_TextureCoords.dtvdx	= deltaSmall.m_TextureCoords.dtvdx;
			deltaBig.m_TextureCoords.dtvdy	= deltaSmall.m_TextureCoords.dtvdy	+ gradients.dx.m_TextureCoords.dtvdy;

			deltaBig.m_FogDensity			= deltaSmall.m_FogDensity			+ gradients.dx.m_FogDensity;
		} else {
			deltaBig.m_WindowCoords.depth	= deltaSmall.m_WindowCoords.depth	- gradients.dx.m_WindowCoords.depth; 

			deltaBig.m_Color.r				= deltaSmall.m_Color.r				- gradients.dx.m_Color.r;
			deltaBig.m_Color.g				= deltaSmall.m_Color.g				- gradients.dx.m_Color.g;
			deltaBig.m_Color.b				= deltaSmall.m_Color.b				- gradients.dx.m_Color.b;
			deltaBig.m_Color.a				= deltaSmall.m_Color.a				- gradients.dx.m_Color.a;
			
			deltaBig.m_WindowCoords.invZ	= deltaSmall.m_WindowCoords.invZ	- gradients.dx.m_WindowCoords.invZ; 
			deltaBig.m_TextureCoords.tu		= deltaSmall.m_TextureCoords.tu		- gradients.dx.m_TextureCoords.tu;
			deltaBig.m_TextureCoords.tv		= deltaSmall.m_TextureCoords.tv		- gradients.dx.m_TextureCoords.tv;
			deltaBig.m_TextureCoords.dtudx	= deltaSmall.m_TextureCoords.dtudx;
			deltaBig.m_TextureCoords.dtudy	= deltaSmall.m_TextureCoords.dtudy	- gradients.dx.m_TextureCoords.dtudy;
			deltaBig.m_TextureCoords.dtvdx	= deltaSmall.m_TextureCoords.dtvdx;
			deltaBig.m_TextureCoords.dtvdy	= deltaSmall.m_TextureCoords.dtvdy	+ gradients.dx.m_TextureCoords.dtvdy;

			deltaBig.m_FogDensity			= deltaSmall.m_FogDensity			- gradients.dx.m_FogDensity;
		}
	}

#define ScanlineDelta(rasterInfo, start, delta, deltaSmall, deltaBig, xError, xStepError, dxdy) \
		xError += xStepError;													\
		if (xError >= EGL_ONE) {												\
			xError -= EGL_ONE;													\
			start.m_WindowCoords.x		+= deltaBig.m_WindowCoords.x;			\
			start.m_Color				+= deltaBig.m_Color;					\
			start.m_WindowCoords.invZ	+= deltaBig.m_WindowCoords.invZ;		\
			start.m_TextureCoords.tu	+= deltaBig.m_TextureCoords.tu;			\
			start.m_TextureCoords.tv	+= deltaBig.m_TextureCoords.tv;			\
			start.m_TextureCoords.dtudx	+= deltaBig.m_TextureCoords.dtudx;		\
			start.m_TextureCoords.dtudy	+= deltaBig.m_TextureCoords.dtudy;		\
			start.m_TextureCoords.dtvdx	+= deltaBig.m_TextureCoords.dtvdx;		\
			start.m_TextureCoords.dtvdy	+= deltaBig.m_TextureCoords.dtvdy;		\
			start.m_FogDensity			+= deltaBig.m_FogDensity;				\
			start.m_WindowCoords.depth	+= deltaBig.m_WindowCoords.depth;		\
		} else {																\
			start.m_WindowCoords.x		+= deltaSmall.m_WindowCoords.x;			\
			start.m_Color				+= deltaSmall.m_Color;					\
			start.m_WindowCoords.invZ	+= deltaSmall.m_WindowCoords.invZ;		\
			start.m_TextureCoords.tu	+= deltaSmall.m_TextureCoords.tu;		\
			start.m_TextureCoords.tv	+= deltaSmall.m_TextureCoords.tv;		\
			start.m_TextureCoords.dtudx	+= deltaSmall.m_TextureCoords.dtudx;	\
			start.m_TextureCoords.dtudy	+= deltaSmall.m_TextureCoords.dtudy;	\
			start.m_TextureCoords.dtvdx	+= deltaSmall.m_TextureCoords.dtvdx;	\
			start.m_TextureCoords.dtvdy	+= deltaSmall.m_TextureCoords.dtvdy;	\
			start.m_FogDensity			+= deltaSmall.m_FogDensity;				\
			start.m_WindowCoords.depth	+= deltaSmall.m_WindowCoords.depth;		\
		}																		\
		delta.m_WindowCoords.x			+= dxdy;								\
		rasterInfo.DepthBuffer			+= rasterInfo.SurfaceWidth;				\
		rasterInfo.ColorBuffer			+= rasterInfo.SurfaceWidth;				\
		rasterInfo.StencilBuffer		+= rasterInfo.SurfaceWidth;				\
		rasterInfo.AlphaBuffer			+= rasterInfo.SurfaceWidth;
}


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

	// ----------------------------------------------------------------------
	// calculate all gradients for interpolation 
	// ----------------------------------------------------------------------

	EGL_Fixed invDenominator = 0;

	Gradients grad;

	EGL_Fixed tuOverZ1 = EGL_Mul(pos1.m_TextureCoords.tu, pos1.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ2 = EGL_Mul(pos2.m_TextureCoords.tu, pos2.m_WindowCoords.invZ);
	EGL_Fixed tuOverZ3 = EGL_Mul(pos3.m_TextureCoords.tu, pos3.m_WindowCoords.invZ);

	EGL_Fixed tvOverZ1 = EGL_Mul(pos1.m_TextureCoords.tv, pos1.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ2 = EGL_Mul(pos2.m_TextureCoords.tv, pos2.m_WindowCoords.invZ);
	EGL_Fixed tvOverZ3 = EGL_Mul(pos3.m_TextureCoords.tv, pos3.m_WindowCoords.invZ);

	if (!denominator) {
		memset(&grad, 0, sizeof grad);
	} else {
		invDenominator = EGL_Inverse(denominator);

		SOLVE_XY(m_Color.r, invDenominator);
		SOLVE_XY(m_Color.g, invDenominator);
		SOLVE_XY(m_Color.b, invDenominator);
		SOLVE_XY(m_Color.a, invDenominator);

		SOLVE_XY(m_FogDensity, invDenominator);
		SOLVE_XY(m_WindowCoords.depth, invDenominator);

		SOLVE_XY(m_WindowCoords.invZ, invDenominator);

		SOLVE_PARAM_XY(m_TextureCoords.tu, tuOverZ1, tuOverZ2, tuOverZ3, invDenominator);
		SOLVE_PARAM_XY(m_TextureCoords.tv, tvOverZ1, tvOverZ2, tvOverZ3, invDenominator);

		// ------------------------------------------------------------------
		// Determine partial derivatives of texture functions, 
		// see eq. (2) and (3) in
		// J. P. Ewins et al (1998), 
		// "MIP-Map Level Selection for Texture Mapping",
		// IEEE Transactions on Visualization and Computer Graphics, 
		// Vol 4, No. 4
		// ------------------------------------------------------------------

		EGL_Fixed A = grad.dx.m_TextureCoords.tu;
		EGL_Fixed B = grad.dy.m_TextureCoords.tu;
		EGL_Fixed D = grad.dx.m_WindowCoords.invZ;
		EGL_Fixed E = grad.dy.m_WindowCoords.invZ;
		EGL_Fixed F = pos1.m_WindowCoords.invZ;
		EGL_Fixed G = grad.dx.m_TextureCoords.tv;
		EGL_Fixed H = grad.dy.m_TextureCoords.tv;

		EGL_Fixed K1 = Det2x2(A, B, D, E);
		EGL_Fixed K2 = Det2x2(G, H, D, E);

		grad.dy.m_TextureCoords.dtudx = K1;
		grad.dy.m_TextureCoords.dtudy = 0;
		grad.dy.m_TextureCoords.dtvdx = K2;
		grad.dy.m_TextureCoords.dtvdy = 0;

		grad.dx.m_TextureCoords.dtudx = 0;
		grad.dx.m_TextureCoords.dtudy = -K1;
		grad.dx.m_TextureCoords.dtvdx = 0;
		grad.dx.m_TextureCoords.dtvdy = -K2;
	}

	// ----------------------------------------------------------------------
	// Constants to determine partial derivatives, see eq. (2) and (3) in
	// J. P. Ewins et al (1998), "MIP-Map Level Selection for Texture Mapping",
	// IEEE Transactions on Visualization and Computer Graphics, Vol 4, No. 4
	// ----------------------------------------------------------------------

	// Share the gradient in x direction for scanline function

	EdgePos& delta = grad.dx;

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

		EGL_Fixed gradX = EGL_Abs(grad.dx.m_WindowCoords.depth);
		EGL_Fixed gradY = EGL_Abs(grad.dy.m_WindowCoords.depth);

		EGL_Fixed depthSlope = gradX > gradY ? gradX : gradY;

		I32 offset = EGL_Mul(factor, depthSlope) + EGL_IntFromFixed(units * PolygonOffsetUnitSize);

		if (offset > 0) {
			depth1 = depth1 < DepthRangeMax - offset ? depth1 + offset : DepthRangeMax;
			depth2 = depth2 < DepthRangeMax - offset ? depth2 + offset : DepthRangeMax;
			depth3 = depth3 < DepthRangeMax - offset ? depth3 + offset : DepthRangeMax;
		} else {
			depth1 = depth1 > -offset ? depth1 + offset : 0;
			depth2 = depth2 > -offset ? depth2 + offset : 0;
			depth3 = depth3 > -offset ? depth3 + offset : 0;
		}
	}

	// ----------------------------------------------------------------------
	// determine the appropriate mipmapping level
	// ----------------------------------------------------------------------

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

	m_RasterInfo.Init(m_Surface, y);

	I32 yScissorStart = m_State->m_ScissorY;
	I32 yScissorEnd = yScissorStart + m_State->m_ScissorHeight;

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

			InitScanlineStart(start, grad, xPreStep1, yPreStep1,
							  pos1.m_Color, pos1.m_FogDensity, depth1, invZ1, tuOverZ1, tvOverZ1);

			// ------------------------------------------------------------------
			// initialize edge buffer delta2Int & delta2Frac
			// ------------------------------------------------------------------

			// determine integer x step/y
			I32 dXdYStep1Int = dXdY3 >= 0 ? EGL_IntFromFixed(dXdY3) : -EGL_IntFromFixed(-dXdY3);

			EdgePos delta3Small, delta3Big;
			InitScanlineDeltas(delta3Small, delta3Big, grad, dXdY3, dXdYStep1Int);

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
				assert(xStepError >= 0 && xStepError < EGL_ONE);
				xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			} else {
				xStepError = -dXdY3 + EGL_FixedFromInt(dXdYStep1Int);
				assert(xStepError >= 0 && xStepError < EGL_ONE);
				xError = (EGL_ONE-1) - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			}

			// ------------------------------------------------------------------
			// Raster the top part of the triangle
			// ------------------------------------------------------------------

			for (; y < y3; ++y) {

				if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
					RasterScanLine(m_RasterInfo, start, delta);												

				ScanlineDelta(m_RasterInfo, start, delta, delta3Small, delta3Big,
							  xError, xStepError, dXdY23);
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

			InitScanlineStart(start, grad, xPreStep2, yPreStep1,
							  pos2.m_Color, pos2.m_FogDensity, depth2, invZ2, tuOverZ2, tvOverZ2);

			// ------------------------------------------------------------------
			// initialize edge buffer delta2Int & delta2Frac
			// ------------------------------------------------------------------

			// determine integer x step/y
			I32 dXdYStep2Int = dXdY23 >= 0 ? EGL_IntFromFixed(dXdY23) : -EGL_IntFromFixed(-dXdY23);

			EdgePos delta23Small, delta23Big;
			InitScanlineDeltas(delta23Small, delta23Big, grad, dXdY23, dXdYStep2Int);

			// ------------------------------------------------------------------
			// initialize the x coordinate for right edge
			// ------------------------------------------------------------------

			EGL_Fixed xStepped1R = pos1.m_WindowCoords.x + EGL_Mul(pos2.m_WindowCoords.y - pos1.m_WindowCoords.y + yPreStep2, dXdY3);
			delta.m_WindowCoords.x = 
				xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

			// ------------------------------------------------------------------
			// initialize the x-step error
			// ------------------------------------------------------------------

			EGL_Fixed xStepError, xError;

			if (dXdY23 >= 0) {
				xStepError = dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
				assert(xStepError >= 0 && xStepError < EGL_ONE);
				xError = EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
			} else {
				xStepError = -dXdY23 + EGL_FixedFromInt(dXdYStep2Int);
				assert(xStepError >= 0 && xStepError < EGL_ONE);
				xError = (EGL_ONE-1) - EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
			}

			// ------------------------------------------------------------------
			// Raster the triangle
			// ------------------------------------------------------------------

			for (; y < y3; ++y) {
				if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
					RasterScanLine(m_RasterInfo, start, delta);												

				ScanlineDelta(m_RasterInfo, start, delta, delta23Small, delta23Big,
							  xError, xStepError, dXdY3);
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

		InitScanlineStart(start, grad, xPreStep1, yPreStep1,
						  pos1.m_Color, pos1.m_FogDensity, depth1, invZ1, tuOverZ1, tvOverZ1);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep1Int = dXdY2 >= 0 ? EGL_IntFromFixed(dXdY2) : -EGL_IntFromFixed(-dXdY2);

		EdgePos delta2Small, delta2Big;
		InitScanlineDeltas(delta2Small, delta2Big, grad, dXdY2, dXdYStep1Int);

		// ------------------------------------------------------------------
		// initialize the x coordinate for right edge
		// ------------------------------------------------------------------

		delta.m_WindowCoords.x = 
			xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest


		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		EGL_Fixed xStepError, xError;

		if (dXdY2 >= 0) {
			xStepError = dXdY2 - EGL_FixedFromInt(dXdYStep1Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			assert(xError >= 0 && xError < EGL_ONE);
		} else {
			xStepError = -dXdY2 + EGL_FixedFromInt(dXdYStep1Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = (EGL_ONE-1) - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
			assert(xError >= 0 && xError < EGL_ONE);
		}

		// ------------------------------------------------------------------
		// Raster the top part of the triangle
		// ------------------------------------------------------------------

		for (; y < y2; ++y) {
			if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
				RasterScanLine(m_RasterInfo, start, delta);												

			ScanlineDelta(m_RasterInfo, start, delta, delta2Small, delta2Big,
						  xError, xStepError, dXdY3);
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

		InitScanlineStart(start, grad, xPreStep2, yPreStep2,
						  pos2.m_Color, pos2.m_FogDensity, depth2, invZ2, tuOverZ2, tvOverZ2);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep2Int = dXdY23 >= 0 ? EGL_IntFromFixed(dXdY23) : -EGL_IntFromFixed(-dXdY23);

		EdgePos delta23Small, delta23Big;
		InitScanlineDeltas(delta23Small, delta23Big, grad, dXdY23, dXdYStep2Int);

		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		if (dXdY23 >= 0) {
			xStepError = dXdY23 - EGL_FixedFromInt(dXdYStep2Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
		} else {
			xStepError = -dXdY23 + EGL_FixedFromInt(dXdYStep2Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = (EGL_ONE-1) - EGL_FractionFromFixed(xStepped2L + EGL_ONE/2);
		}


		// ------------------------------------------------------------------
		// Raster the bottom part of the triangle
		// ------------------------------------------------------------------

		for (; y < y3; ++y) {
			if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
				RasterScanLine(m_RasterInfo, start, delta);												

			ScanlineDelta(m_RasterInfo, start, delta, delta23Small, delta23Big,
						  xError, xStepError, dXdY3);
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

		InitScanlineStart(start, grad, xPreStep1, yPreStep1,
						  pos1.m_Color, pos1.m_FogDensity, depth1, invZ1, tuOverZ1, tvOverZ1);

		// ------------------------------------------------------------------
		// initialize edge buffer delta2Int & delta2Frac
		// ------------------------------------------------------------------

		// determine integer x step/y
		I32 dXdYStep1Int = dXdY3 >= 0 ? EGL_IntFromFixed(dXdY3) : -EGL_IntFromFixed(-dXdY3);

		EdgePos delta3Small, delta3Big;
		InitScanlineDeltas(delta3Small, delta3Big, grad, dXdY3, dXdYStep1Int);

		// ------------------------------------------------------------------
		// initialize the x coordinate for right edge
		// ------------------------------------------------------------------

		EGL_Fixed xStepped1R = pos1.m_WindowCoords.x + EGL_Mul(yPreStep1, dXdY2);
		delta.m_WindowCoords.x = 
			xStepped1R + ((EGL_ONE/2) - 1);	// added offset so round down will be round to nearest

		// ------------------------------------------------------------------
		// initialize the x-step error
		// ------------------------------------------------------------------

		EGL_Fixed xStepError, xError;

		if (dXdY3 >= 0) {
			xStepError = dXdY3 - EGL_FixedFromInt(dXdYStep1Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		} else {
			xStepError = -dXdY3 + EGL_FixedFromInt(dXdYStep1Int);
			assert(xStepError >= 0 && xStepError < EGL_ONE);
			xError = (EGL_ONE-1) - EGL_FractionFromFixed(xStepped1L + EGL_ONE/2);
		}

		// ------------------------------------------------------------------
		// Raster the top part of the triangle
		// ------------------------------------------------------------------

		for (; y < y2; ++y) {
			if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
				RasterScanLine(m_RasterInfo, start, delta);												

			ScanlineDelta(m_RasterInfo, start, delta, delta3Small, delta3Big,
						  xError, xStepError, dXdY2);
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
			if (!m_State->m_ScissorTestEnabled || (y >= yScissorStart && y < yScissorEnd))
				RasterScanLine(m_RasterInfo, start, delta);												

			ScanlineDelta(m_RasterInfo, start, delta, delta3Small, delta3Big,
						  xError, xStepError, dXdY23);
		}
	}
}


