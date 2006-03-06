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

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace {
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

	inline I32 min(I32 a, I32 b, I32 c) {
		I32 d = (a < b) ? a : b;
		return (d < c) ? d : c;
	}

	inline I32 max(I32 a, I32 b, I32 c) {
		I32 d = (a > b) ? a : b;
		return (d > c) ? d : c;
	}

	bool HasAlpha(RasterizerState::TextureFormat format) {
		switch (format) {
		case RasterizerState::TextureFormatAlpha:
		case RasterizerState::TextureFormatLuminanceAlpha:
		case RasterizerState::TextureFormatRGBA8:
		case RasterizerState::TextureFormatRGBA4444:
		case RasterizerState::TextureFormatRGBA5551:
			return true;

		default:
			return false;
		}
	}

	inline I32 Mul24(I32 a, I32 b) {
		I64 product = static_cast<I64>(a) * static_cast<I64>(b);
		return static_cast<I32>((product + 0x400000) >> 23);
	}

	inline I32 MulRoundShift(I32 a, I32 b, I32 shift) {
		I64 product = static_cast<I64>(a) * static_cast<I64>(b);
		return static_cast<I32>((product + (1 << (shift - 1))) >> shift);
	}

# if 0
	I32 Inverse8q24(I32 a) {
		I32 x;

		/* 1/(4x) */
		static const I32 __gl_rcp_tab[] = { /* domain 0.5 .. 1.0-1/16 */
			0x8000, 0x71c7, 0x6666, 0x5d17, 0x5555, 0x4ec4, 0x4924, 0x4444
		};
		bool sign = false;

		if (a == 0) return 0;

		if (a < 0) {
			sign = true;
			a = -a;
		}

		x = __gl_rcp_tab[(a >> 21)&0x7] << 8;

		/* 3 iterations of newton-raphson  x = x(2-ax) */
		int iter = 3;

		do {
			x = Mul24(x,(0x1000000 - Mul24(a,x)));
		} while (--iter);

		if (sign)
			x = -x;

		return x;
	}
#endif

	I32 InvNewtonRaphson4q28(I32 a, I32 x) {
		int iter = 3;

		do {
			x = MulRoundShift(x,(0x20000 - MulRoundShift(a, x, 16)), 12);
		} while (--iter);

		return x;
	}
}


// ---------------------------------------------------------------------------
// Prepare rasterizer object for triangles
// ---------------------------------------------------------------------------

void Rasterizer :: PrepareTriangle() {
	PrepareTexture();

	// initialize block rasterization here

	// could be optimized
	bool needsColor = true;
				//!m_State->m_Texture[0].Enabled || 
				//  m_State->m_Texture[0].Mode != RasterizerState::TextureModeReplace;

	bool needsTexture = false;
	
	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		needsTexture |= m_State->m_Texture[unit].Enabled;
	}

	bool needsFog = m_State->m_Fog.Enabled;
	bool needsDepth = m_State->m_DepthTest.Enabled || m_State->m_Mask.Depth || m_State->m_Stencil.Enabled;
	bool needsScissor = m_State->m_ScissorTest.Enabled;
	bool needsStencil = m_State->m_Stencil.Enabled;

	U32 selector =
		((needsColor	? 1 : 0) << RasterTriangleColor		) |
		((needsTexture	? 1 : 0) << RasterTriangleTexture	) |
		((needsFog		? 1 : 0) << RasterTriangleFog		) |
		((needsDepth	? 1 : 0) << RasterTriangleDepth		) |
		((needsScissor	? 1 : 0) << RasterTriangleScissor	) |
		((needsStencil	? 1 : 0) << RasterTriangleStencil	);

	memset(m_RasterInfo.MipmapLevel, 0, sizeof(m_RasterInfo.MipmapLevel));
}


void Rasterizer :: RasterTriangle(const RasterPos& a, const RasterPos& b,
								  const RasterPos& c) {

	// 16.16 -> 28.4 fixed-point coordinates
	I32 Y1 = (a.m_WindowCoords.y + (1 << 11)) >> 12;
	I32 Y2 = (b.m_WindowCoords.y + (1 << 11)) >> 12;
	I32 Y3 = (c.m_WindowCoords.y + (1 << 11)) >> 12;

	I32 X1 = (a.m_WindowCoords.x + (1 << 11)) >> 12;
	I32 X2 = (b.m_WindowCoords.x + (1 << 11)) >> 12;
	I32 X3 = (c.m_WindowCoords.x + (1 << 11)) >> 12;

	I32 IX = (X1 + X2 + X3) / 3;
	I32 IY = (Y1 + Y2 + Y3) / 3;

	I32 OX = (IX + 8) >> 4;
	I32 OY = (IY + 8) >> 4;

    // Deltas
    I32 DX12 = X1 - X2, DX23 = X2 - X3, DX31 = X3 - X1;
    I32 DY12 = Y1 - Y2, DY23 = Y2 - Y3, DY31 = Y3 - Y1;

    I32 DW12 = a.m_WindowCoords.invW - b.m_WindowCoords.invW;
    I32 DW23 = b.m_WindowCoords.invW - c.m_WindowCoords.invW;
    I32 DW31 = c.m_WindowCoords.invW - a.m_WindowCoords.invW;

    I32 DD12 = a.m_WindowCoords.depth - b.m_WindowCoords.depth;
    I32 DD23 = b.m_WindowCoords.depth - c.m_WindowCoords.depth;
    I32 DD31 = c.m_WindowCoords.depth - a.m_WindowCoords.depth;

    // 24.8 Fixed-point deltas
    I32 FDX12 = DX12 << 4, FDX23 = DX23 << 4, FDX31 = DX31 << 4;
    I32 FDY12 = DY12 << 4, FDY23 = DY23 << 4, FDY31 = DY31 << 4;

    // Bounding rectangle; round lower bound down to block size
    I32 minx = ((min(X1, X2, X3) + 0x7) >> 4) & ~(EGL_RASTER_BLOCK_SIZE - 1);
    I32 miny = ((min(Y1, Y2, Y3) + 0x7) >> 4) & ~(EGL_RASTER_BLOCK_SIZE - 1);
    I32 maxx = (max(X1, X2, X3) + 0x8) >> 4;
    I32 maxy = (max(Y1, Y2, Y3) + 0x8) >> 4;
	I32 span = maxx - minx;

	m_RasterInfo.Init(m_Surface, miny, minx);
	I32 stride = m_RasterInfo.SurfaceWidth - maxx + minx;
	I32 blockStride = EGL_RASTER_BLOCK_SIZE * m_RasterInfo.SurfaceWidth;

    // Half-edge constants
    //I32 C1 = DY12 * X1 - DX12 * Y1;
    //I32 C2 = DY23 * X2 - DX23 * Y2;
    //I32 C3 = DY31 * X3 - DX31 * Y3;
    I32 C1 = Y2 * X1 - X2 * Y1;
    I32 C2 = Y3 * X2 - X3 * Y2;
    I32 C3 = Y1 * X3 - X1 * Y3;

	I32 x, y, ix, iy; // loop variables
	U32 index, unit;

	I32 dVaryingInvWdX[EGL_MAX_NUM_VARYING];
	I32 dVaryingInvWdY[EGL_MAX_NUM_VARYING];
	I32 dVaryingInvWBlockLine[EGL_MAX_NUM_VARYING];
	I32 varyingInvW[EGL_MAX_NUM_VARYING];

	I32 dInvWdX, dInvWdY, dInvWBlockLine, invW;
	I32 dDepthDx, dDepthDy, dDepthBlockLine, depth;

	I32 area, invArea;

	U32 numVarying = m_VaryingInfo.numVarying, usedNumVarying = 0;

    // Correct for fill convention
	if (DY12 > 0 || (DY12 == 0 && DX12 < 0)) C1++;
	if (DY23 > 0 || (DY23 == 0 && DX23 < 0)) C2++;
	if (DY31 > 0 || (DY31 == 0 && DX31 < 0)) C3++;

	GLint CIY1 = C1 + DY12 * IX - DX12 * IY;
	GLint CIY2 = C2 + DY23 * IX - DX23 * IY;
	GLint CIY3 = C3 + DY31 * IX - DX31 * IY;

	// setup of interpolation of varying variables goes here
	// area in 24.8
	area = DX31 * DY12 - DX12 * DY31;
	//assert(area >= 0);

	if (area <= 0xf)
		return;

	//assert(CIY1 >= 0 && CIY2 >= 0 && CIY3 >= 0);

	// inv arera as 8.24
	invArea = EGL_Inverse(area >> 4);

	dDepthDx = MulRoundShift(EGL_Mul(DY12, DD31) - EGL_Mul(DD12, DY31), invArea, 12); 
	dDepthDy = MulRoundShift(EGL_Mul(DX12, DD31) - EGL_Mul(DD12, DX31), invArea, 12); 
	dDepthBlockLine = dDepthDx * EGL_RASTER_BLOCK_SIZE - dDepthDy * span; 

	// dWdX, dWdY is 4.28
	dInvWdX = MulRoundShift(EGL_Mul(DY12, DW31) - EGL_Mul(DW12, DY31), invArea, 12);
	dInvWdY = MulRoundShift(EGL_Mul(DX12, DW31) - EGL_Mul(DW12, DX31), invArea, 12);
	dInvWBlockLine = dInvWdX * EGL_RASTER_BLOCK_SIZE - dInvWdY * span;

	// invW is 4.28
	{
		// calculate coordinate of upper left corner incl. 1/2 pixel pre-step
		I32 XMin1 = (minx << 4) + (1 << 3) - X1;
		I32 YMin1 = (miny << 4) + (1 << 3) - Y1;

		depth = a.m_WindowCoords.depth 
				+ MulRoundShift(XMin1, dDepthDx, 4) 
				+ MulRoundShift(YMin1, dDepthDy, 4);

		invW = a.m_WindowCoords.invW 
				+ MulRoundShift(XMin1, dInvWdX, 4) 
				+ MulRoundShift(YMin1, dInvWdY, 4);
	}

    // Loop through blocks
    for (y = miny; y < maxy; y += EGL_RASTER_BLOCK_SIZE) {

		SurfaceInfo outerInfo;

		outerInfo.ColorBuffer = m_RasterInfo.SurfaceInfo.ColorBuffer;
		outerInfo.AlphaBuffer = m_RasterInfo.SurfaceInfo.AlphaBuffer;
		outerInfo.DepthBuffer = m_RasterInfo.SurfaceInfo.DepthBuffer;
		outerInfo.StencilBuffer = m_RasterInfo.SurfaceInfo.StencilBuffer;

        for (x = minx; x < maxx; x += EGL_RASTER_BLOCK_SIZE) {

			// interpolation values for the four corners
			I32 depth0;
			I32 w[2][2];				// w in all four corners of block 4.28 (?)

			I32 varying[2][2][EGL_MAX_NUM_VARYING];	//
			I32 dVaryingDy[2][EGL_MAX_NUM_VARYING]; // gradients along y

			// information to use per block
			SurfaceInfo surfaceInfo;
			const Texture *	texture[EGL_NUM_TEXTURE_UNITS];		// texture ptr (potentially 3-linear)

            // Corners of block
            GLint x0 = (x << 4) | (1 << 3);
            GLint x1 = ((x + EGL_RASTER_BLOCK_SIZE - 1) << 4) | (1 << 3);
            GLint y0 = (y << 4) | (1 << 3);
            GLint y1 = ((y + EGL_RASTER_BLOCK_SIZE - 1) << 4) | (1 << 3);

            // Evaluate half-space functions
            GLboolean a00 = C1 + DY12 * x0 - DX12 * y0 > 0;
            GLboolean a10 = C1 + DY12 * x1 - DX12 * y0 > 0;
            GLboolean a01 = C1 + DY12 * x0 - DX12 * y1 > 0;
            GLboolean a11 = C1 + DY12 * x1 - DX12 * y1 > 0;
            GLint eqa = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
    
            GLboolean b00 = C2 + DY23 * x0 - DX23 * y0 > 0;
            GLboolean b10 = C2 + DY23 * x1 - DX23 * y0 > 0;
            GLboolean b01 = C2 + DY23 * x0 - DX23 * y1 > 0;
            GLboolean b11 = C2 + DY23 * x1 - DX23 * y1 > 0;
            GLint eqb = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
    
            GLboolean c00 = C3 + DY31 * x0 - DX31 * y0 > 0;
            GLboolean c10 = C3 + DY31 * x1 - DX31 * y0 > 0;
            GLboolean c01 = C3 + DY31 * x0 - DX31 * y1 > 0;
            GLboolean c11 = C3 + DY31 * x1 - DX31 * y1 > 0;
            GLint eqc = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

#if EGL_RASTER_BLOCK_SIZE <= 8
			typedef U8 PixelMask;
#elif EGL_RASTER_BLOCK_SIZE <= 16
			typedef U16 PixelMask;
#else
			typedef U32 PixelMask;
#endif
			PixelMask pixelMask[EGL_RASTER_BLOCK_SIZE], *mask;
			PixelMask totalMask;

            // Skip block when outside an edge
			if (eqa == 0x0 || eqb == 0x0 || eqc == 0x0) {
				goto cont;
			}

			depth0 = depth;

			// initialize surface pointers in local info block
			surfaceInfo.ColorBuffer = outerInfo.ColorBuffer;
			surfaceInfo.AlphaBuffer = outerInfo.AlphaBuffer;
			surfaceInfo.DepthBuffer = outerInfo.DepthBuffer;
			surfaceInfo.StencilBuffer = outerInfo.StencilBuffer;

			totalMask = 0;
			mask = pixelMask;

			// Accept whole block when totally covered
            if (eqa == 0xF && eqb == 0xF && eqc == 0xF) {
                for (iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {

					PixelMask rowMask = 0;

                    for (ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {

                        // test and write depth and stencil
						bool written = FragmentDepthStencil(&surfaceInfo, ix, depth0);

						rowMask = (rowMask >> 1) | (written ? (1 << (EGL_RASTER_BLOCK_SIZE - 1)) : 0);

						depth0 += dDepthDx;
                    }

					*mask++ = rowMask;
					totalMask |= rowMask;

					depth0 += dDepthDy - (dDepthDx << EGL_LOG_RASTER_BLOCK_SIZE);

					surfaceInfo.DepthBuffer += m_RasterInfo.SurfaceWidth;
					surfaceInfo.StencilBuffer += m_RasterInfo.SurfaceWidth;
                }
            }
            else {
				// Partially covered block
                GLint CY1 = C1 + DY12 * x0 - DX12 * y0;
                GLint CY2 = C2 + DY23 * x0 - DX23 * y0;
                GLint CY3 = C3 + DY31 * x0 - DX31 * y0;

                for (iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {
                    GLint CX1 = CY1, CX2 = CY2, CX3 = CY3;
					PixelMask rowMask = 0;

                    for(ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {
                        if (CX1 > 0 && CX2 > 0 && CX3 > 0) {
							// test and write depth and stencil
							bool written = FragmentDepthStencil(&surfaceInfo, ix, depth0);
							rowMask = (rowMask >> 1) | (written ? (1 << (EGL_RASTER_BLOCK_SIZE - 1)) : 0);
                        } else {
							rowMask >>= 1;
						}

                        CX1 += FDY12; CX2 += FDY23; CX3 += FDY31;
						depth0 += dDepthDx;
                    }

					*mask++ = rowMask;
					totalMask |= rowMask;

                    CY1 -= FDX12; CY2 -= FDX23; CY3 -= FDX31;

					depth0 += dDepthDy - (dDepthDx << EGL_LOG_RASTER_BLOCK_SIZE);

					surfaceInfo.DepthBuffer += m_RasterInfo.SurfaceWidth;
					surfaceInfo.StencilBuffer += m_RasterInfo.SurfaceWidth;
                }
            }

			if (totalMask) {
				if (numVarying != usedNumVarying) {
					I32 XMin2 = x0 - X1;
					I32 YMin2 = y0 - Y1;

					for (index = 0; index < numVarying; ++index) {
						// 4.28
						I32 V1OverW = MulRoundShift(a.m_Varying[index], a.m_WindowCoords.invW, 16);
						I32 V2OverW = MulRoundShift(b.m_Varying[index], b.m_WindowCoords.invW, 16);
						I32 V3OverW = MulRoundShift(c.m_Varying[index], c.m_WindowCoords.invW, 16);

						I32 IVW12 = V1OverW - V2OverW;
						I32 IVW31 = V3OverW - V1OverW;

						// dVaryingDx, dVaryingDy is 4.28
						dVaryingInvWdX[index] = MulRoundShift(EGL_Mul(DY12, IVW31) - EGL_Mul(IVW12, DY31), invArea, 12);
						dVaryingInvWdY[index] = MulRoundShift(EGL_Mul(DX12, IVW31) - EGL_Mul(IVW12, DX31), invArea, 12);
						dVaryingInvWBlockLine[index] = dVaryingInvWdY[index] * EGL_RASTER_BLOCK_SIZE - dVaryingInvWdY[index] * span;

						// varyingStart is 4.28
						varyingInvW[index] = 
							V1OverW 
								+ MulRoundShift(XMin2, dInvWdX, 4) 
								+ MulRoundShift(YMin2, dInvWdY, 4);
					}

					usedNumVarying = numVarying;
				}

				// compute w in all four corners
				w[0][0] = EGL_Inverse(invW >> 12);		// 4.28
				w[0][1] = InvNewtonRaphson4q28(invW + dInvWdX * EGL_RASTER_BLOCK_SIZE, w[0][0]);
				w[1][0] = InvNewtonRaphson4q28(invW + dInvWdY * EGL_RASTER_BLOCK_SIZE, w[0][0]);
				w[1][1] = InvNewtonRaphson4q28(invW + dInvWdX * EGL_RASTER_BLOCK_SIZE
													+ dInvWdY * EGL_RASTER_BLOCK_SIZE, w[1][0]);

				// compute values of varying at all four corners
				for (index = 0; index < numVarying; ++index) {
					varying[0][0][index] = MulRoundShift(varyingInvW[index], w[0][0], 16);
					varying[0][1][index] = MulRoundShift(varyingInvW[index] + dVaryingInvWdX[index] * EGL_RASTER_BLOCK_SIZE, w[0][1], 16);
					dVaryingDy[0][index] = (varying[0][1][index] - varying[0][0][index]) >> EGL_LOG_RASTER_BLOCK_SIZE;
					varying[1][0][index] = MulRoundShift(varyingInvW[index] + dVaryingInvWdY[index] * EGL_RASTER_BLOCK_SIZE, w[1][0], 16);
					varying[1][1][index] = MulRoundShift(varyingInvW[index] + dVaryingInvWdX[index] * EGL_RASTER_BLOCK_SIZE 
																			+ dVaryingInvWdY[index] * EGL_RASTER_BLOCK_SIZE, w[1][1], 16);
					dVaryingDy[1][index] = (varying[1][1][index] - varying[1][0][index]) >> EGL_LOG_RASTER_BLOCK_SIZE;
				}			

				// perform Mipmap selection; initialize local RasterInfo structure
				for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
					size_t textureBase = m_VaryingInfo.textureBase[unit];

					if (textureBase >= 0 && m_UseMipmap[unit]) {

						I32 dUdX = (varying[0][1][textureBase] + (dVaryingDy[1][textureBase] << (EGL_LOG_RASTER_BLOCK_SIZE - 1)) -
									varying[0][0][textureBase] - (dVaryingDy[0][textureBase] << (EGL_LOG_RASTER_BLOCK_SIZE - 1)))
									<< EGL_LOG_RASTER_BLOCK_SIZE;
						I32 dUdY = (dVaryingDy[0][textureBase] + dVaryingDy[1][textureBase]) >> 1;
						I32 dVdX = (varying[0][1][textureBase + 1] + (dVaryingDy[1][textureBase + 1] << (EGL_LOG_RASTER_BLOCK_SIZE - 1)) -
									varying[0][0][textureBase + 1] - (dVaryingDy[0][textureBase + 1] << (EGL_LOG_RASTER_BLOCK_SIZE - 1)))
									<< EGL_LOG_RASTER_BLOCK_SIZE;
						I32 dVdY = (dVaryingDy[0][textureBase + 1] + dVaryingDy[1][textureBase + 1]) >> 1;

						EGL_Fixed maxDu = EGL_Max(EGL_Abs(dUdX), EGL_Abs(dUdY)) >> (16 - m_Texture[unit]->GetTexture(0)->GetLogWidth());
						EGL_Fixed maxDv = EGL_Max(EGL_Abs(dVdX), EGL_Abs(dVdY)) >> (16 - m_Texture[unit]->GetTexture(0)->GetLogHeight());

						EGL_Fixed rho = EGL_Max(maxDu, maxDv);

						// we start with nearest/minification only selection; will add LINEAR later
						I32 level = EGL_Min(Log2(rho), m_RasterInfo.MaxMipmapLevel[unit]);
						texture[unit] = m_RasterInfo.Textures[level];
					} else {
						texture[unit] = m_RasterInfo.Textures[0];
					}
				}

				mask = pixelMask;

                for (iy = 0; iy < EGL_RASTER_BLOCK_SIZE; iy++) {

					I32 varying0[EGL_MAX_NUM_VARYING];
					I32 dVarying[EGL_MAX_NUM_VARYING];
					PixelMask rowMask = *mask++;

					for (index = 0; index < numVarying; ++index) {
						varying0[index] = varying[0][0][index];
						dVarying[index] = (varying[0][1][index] - varying[0][0][index]) >> EGL_LOG_RASTER_BLOCK_SIZE;
					}

                    for (ix = 0; ix < EGL_RASTER_BLOCK_SIZE; ix++) {

						if (rowMask & 1) {
							I32 tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];
							Color baseColor;
							I32 fog;

							for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
								size_t textureBase = m_VaryingInfo.textureBase[unit];

								if (textureBase >= 0) {
									tu[unit] = varying0[textureBase];
									tv[unit] = varying0[textureBase + 1];
								}
							}

							if (m_VaryingInfo.colorIndex >= 0) {
								//baseColor = FractionalColor(varying0 + m_VaryingInfo.colorIndex);
								baseColor = Color(255, 255, 255, 255);
							}

							if (m_VaryingInfo.fogIndex >= 0) {
								fog = varying0[m_VaryingInfo.fogIndex];
							}

							FragmentColorAlpha(&surfaceInfo, ix, texture, tu, tv, baseColor, fog);
						}

						rowMask >>= 1;

						for (index = 0; index < usedNumVarying; ++index) {
							varying0[index] += dVarying[index];
						}
                    }

					for (index = 0; index < numVarying; ++index) {
						varying[0][0][index] += dVaryingDy[0][index];
						varying[0][1][index] += dVaryingDy[1][index];
					}

					surfaceInfo.ColorBuffer += m_RasterInfo.SurfaceWidth;
					surfaceInfo.AlphaBuffer += m_RasterInfo.SurfaceWidth;
                }
			}

cont:
			depth += dDepthDx << EGL_LOG_RASTER_BLOCK_SIZE;
			invW += dInvWdX << EGL_LOG_RASTER_BLOCK_SIZE;

			for (index = 0; index < usedNumVarying; ++index) {
				varyingInvW[index] += dVaryingInvWdX[index] << EGL_LOG_RASTER_BLOCK_SIZE;
			}

			outerInfo.ColorBuffer   += EGL_RASTER_BLOCK_SIZE;
			outerInfo.DepthBuffer   += EGL_RASTER_BLOCK_SIZE;
			outerInfo.AlphaBuffer   += EGL_RASTER_BLOCK_SIZE;
			outerInfo.StencilBuffer += EGL_RASTER_BLOCK_SIZE;
        }

		depth += dDepthBlockLine;
		invW += dInvWBlockLine;

		for (index = 0; index < usedNumVarying; ++index) {
			varyingInvW[index] += dVaryingInvWBlockLine[index];
		}

		m_RasterInfo.SurfaceInfo.ColorBuffer   += blockStride;
		m_RasterInfo.SurfaceInfo.DepthBuffer   += blockStride;
		m_RasterInfo.SurfaceInfo.AlphaBuffer   += blockStride;
		m_RasterInfo.SurfaceInfo.StencilBuffer += blockStride;
    }
}

