// ==========================================================================
//
// Rasterizer.cpp	Rasterizer Class for 3D Rendering Library
//
//					The rasterizer converts transformed and lit
//					primitives and creates a raster image in the
//					current rendering surface.
//
// --------------------------------------------------------------------------
//
// 10-06-2003		Hans-Martin Will	initial version
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
#include "Utils.h"
#include "arm/FunctionCache.h"

using namespace EGL;


// --------------------------------------------------------------------------
// Local helper functions
// --------------------------------------------------------------------------

namespace {

	inline U8 MulU8(U8 a, U8 b) {
        U16 prod = a * b;
        return (prod + (prod >> 7)) >> 8;
	} 

	inline U8 ClampU8(U16 value) {
		return (value > 0xff) ? (U8) 0xff : (U8) value;
	}

	inline U8 AddU8(U8 arg0, U8 arg1) {
		return ClampU8(arg0 + arg1);
	}

	inline U8 SubU8(U8 arg0, U8 arg1) {
		return arg0 > arg1 ? arg0 - arg1 : 0;
	}

	inline U8 AddSignedU8(U8 arg0, U8 arg1) {
		U16 value = (U16) arg0 + arg1;

		if (value >= 0x80)
			return ClampU8(value - 0x80);
		else
			return 0;
	}

	inline U8 InterpolateU8(U8 arg0, U8 arg1, U8 arg2) {
		return MulU8(arg0, arg2) + MulU8(arg1, 0xff - arg2);
	}

	// bring 0 .. 255 -> -128 .. 128
	inline I16 SignedVal(U8 in) {
		if (in <= 0x80)
			return (I16) in - 0x80;
		else {
			I16 inter = (I16) in - 0x80;
			return inter + (inter >> 6);
		}
	}

	inline U8 Dot3U8(const Color& arg0, const Color& arg1) {

		// each product is in the range of -2^14 .. +2^14
		I32 prodR = SignedVal(arg0.r) * SignedVal(arg1.r);
		I32 prodG = SignedVal(arg0.g) * SignedVal(arg1.g);
		I32 prodB = SignedVal(arg0.b) * SignedVal(arg1.b);

		I32 sum = (prodR + prodG + prodB) * 4;

		if (sum <= 0)
			return 0;
		else if (sum >= 0x10000)
			return 0xff;
		else
			return (sum >> 8) - (sum >> 15);
	}

	inline U8 MulU8(U8 a, U8 b, EGL_Fixed scale) {
		U16 prod = a * b;
		return ClampU8((U16) EGL_Mul((prod + (prod >> 7)) >> 8, scale));
	}

	inline U8 AddU8(U8 arg0, U8 arg1, EGL_Fixed scale) {
		return ClampU8((U16) EGL_Mul((U16) arg0 + (U16) arg1, scale));
	}

	inline U8 SubU8(U8 arg0, U8 arg1, EGL_Fixed scale) {
		return arg0 > arg1 ? ClampU8((U16) EGL_Mul(arg0 - arg1, scale)) : 0;
	}

	inline U8 AddSignedU8(U8 arg0, U8 arg1, EGL_Fixed scale) {
		U16 value = (U16) arg0 + arg1;

		if (value >= 0x80)
			return ClampU8((U16) EGL_Mul(value - 0x80, scale));
		else
			return 0;
	}

	inline U8 InterpolateU8(U8 arg0, U8 arg1, U8 arg2, EGL_Fixed scale) {
		return ClampU8((U16) MulU8(arg0, arg2, scale) + MulU8(arg1, 0xff - arg2, scale));
	}

	inline U8 Dot3U8(const Color& arg0, const Color& arg1, EGL_Fixed scale) {
		// each product is in the range of -2^14 .. +2^14
		I32 prodR = SignedVal(arg0.r) * SignedVal(arg1.r);
		I32 prodG = SignedVal(arg0.g) * SignedVal(arg1.g);
		I32 prodB = SignedVal(arg0.b) * SignedVal(arg1.b);

		I32 sum = (prodR + prodG + prodB) * 4;
		I32 scaledSum = ((sum >> 7) - (sum >> 14) + 1) >> 1;

		if (sum <= 0xff)
			return 0;
		else
			return ClampU8(EGL_Mul(scaledSum, scale));
	}

    const I32 InversionTable[32] = {
	    0,
	    EGL_FixedFromFloat(1.0f / 1.0f),
	    EGL_FixedFromFloat(1.0f / 2.0f),
	    EGL_FixedFromFloat(1.0f / 3.0f),
	    EGL_FixedFromFloat(1.0f / 4.0f),
	    EGL_FixedFromFloat(1.0f / 5.0f),
	    EGL_FixedFromFloat(1.0f / 6.0f),
	    EGL_FixedFromFloat(1.0f / 7.0f),
	    EGL_FixedFromFloat(1.0f / 8.0f),
	    EGL_FixedFromFloat(1.0f / 9.0f),
	    EGL_FixedFromFloat(1.0f / 10.0f),
	    EGL_FixedFromFloat(1.0f / 11.0f),
	    EGL_FixedFromFloat(1.0f / 12.0f),
	    EGL_FixedFromFloat(1.0f / 13.0f),
	    EGL_FixedFromFloat(1.0f / 14.0f),
	    EGL_FixedFromFloat(1.0f / 15.0f),
    };
}

void RasterInfo::Init(Surface * surface, I32 y, I32 x) {

	RasterSurface.Width = surface->GetWidth();
	RasterSurface.Height = surface->GetHeight();
	RasterSurface.Pitch = surface->GetPitch();

	size_t offset, depthStencilOffset;

	if (surface->GetPitch() >= 0) {
		offset = y * surface->GetPitch() + x;
	} else {
		offset = (y + 1 - (I32) surface->GetHeight()) * surface->GetPitch() + x;
	}

	I32 blockX = x & (EGL_RASTER_BLOCK_SIZE - 1);
	I32 blockY = y & (EGL_RASTER_BLOCK_SIZE - 1);
	I32 blockIndexX = x - blockX;
	I32 blockIndexY = y - blockY;

	depthStencilOffset = 
		blockIndexY * surface->GetWidth() + ((blockIndexX + blockY) << EGL_LOG_RASTER_BLOCK_SIZE) + blockX;

	RasterSurface.ColorFormat = surface->GetColorFormat();

	switch (RasterSurface.ColorFormat) {
	case ColorFormatRGB565:
	case ColorFormatRGBA5551:
	case ColorFormatRGBA4444:
		RasterSurface.ColorOffsetShift = 1;
		break;

	case ColorFormatRGBA8: 
		RasterSurface.ColorOffsetShift = 2;
		break;

	default:
		assert(false);
	}

	RasterSurface.DepthStencilFormat = surface->GetDepthStencilFormat();

	switch (RasterSurface.DepthStencilFormat) {
	case DepthStencilFormatDepth16:
		RasterSurface.DepthStencilOffsetShift = 1;
		break;

	case DepthStencilFormatDepth16Stencil16:
		RasterSurface.DepthStencilOffsetShift = 2;
		break;

	default:
		assert(false);
	}

	RasterSurface.ColorBuffer = surface->GetColorBuffer() + (offset << RasterSurface.ColorOffsetShift);	
	RasterSurface.DepthStencilBuffer = surface->GetDepthStencilBuffer() + (depthStencilOffset << RasterSurface.DepthStencilOffsetShift);

	InversionTablePtr = InversionTable;
}

// --------------------------------------------------------------------------
// Class Rasterizer
// --------------------------------------------------------------------------


Rasterizer :: Rasterizer(RasterizerState * state, FunctionCache * cache):
	m_State(state),
	m_FunctionCache(cache)
{
}


Rasterizer :: ~Rasterizer() {
}


void Rasterizer :: SetState(RasterizerState * state) {
	m_State = state;

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		m_Texture[unit] = 0;
	}
}


RasterizerState * Rasterizer :: GetState() const {
	return m_State;
}


void Rasterizer :: SetTexture(size_t unit, MultiTexture * texture) {
	m_Texture[unit] = texture;
}


void Rasterizer :: AllocateVaryings() {

	size_t unit;							// index variable

	// determine uses features and allocate varying variables

	bool needsRGBForRGB		= true;		// primary RGB used directly
	bool needsAlphaForAlpha = true;		// primary alpha used directly
	bool needsAlphaForRGB	= false;	// primary alpha not used indirectly
	bool needsRGBForAlpha	= false;	// primary RGB not used indirectly

	// determine if texture units either use primary fragment color
	// directly or indirectly propagated through combine operations

	for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (m_State->m_Texture[unit].Enabled && m_Texture[unit]) {

			bool prevNeedsAlphaForAlpha	= needsAlphaForAlpha;
			bool prevNeedsAlphaForRGB	= needsAlphaForRGB;
			bool prevNeedsRGBForRGB		= needsRGBForRGB;
			bool prevNeedsRGBForAlpha	= needsRGBForAlpha;

			needsRGBForRGB		= false;
			needsAlphaForRGB	= false;
			needsAlphaForAlpha	= false;
			needsRGBForAlpha	= false;

			switch (m_State->m_Texture[unit].Mode) {
			case RasterizerState::TextureModeDecal:
				needsAlphaForAlpha = prevNeedsAlphaForAlpha;
				break;

			case RasterizerState::TextureModeReplace:
				switch (m_Texture[unit]->GetInternalFormat()) {
				case ColorFormatAlpha:
					needsRGBForRGB = prevNeedsRGBForRGB;
					break;

				case ColorFormatLuminance:
				case ColorFormatRGB8:
				case ColorFormatRGB565:
					needsAlphaForAlpha = prevNeedsAlphaForAlpha;
					break;

				case ColorFormatLuminanceAlpha:
				case ColorFormatRGBA8:
				case ColorFormatRGBA4444:
				case ColorFormatRGBA5551:
					break;
				}

				break;

			case RasterizerState::TextureModeModulate:
			case RasterizerState::TextureModeAdd:
			case RasterizerState::TextureModeBlend:
				needsRGBForRGB = prevNeedsRGBForRGB;
				needsAlphaForAlpha = prevNeedsAlphaForAlpha;
				break;

			case RasterizerState::TextureModeCombine:
				switch (m_State->m_Texture[unit].CombineFuncRGB) {
				default:
					assert(0);

				case RasterizerState::TextureModeCombineInterpolate:
					needsRGBForRGB |= (prevNeedsRGBForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcColor));

					needsAlphaForRGB |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsAlphaForRGB |= (prevNeedsAlphaForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[2] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[2] == RasterizerState::TextureCombineOpOneMinusSrcColor));

				case RasterizerState::TextureModeCombineModulate:
				case RasterizerState::TextureModeCombineAdd:
				case RasterizerState::TextureModeCombineAddSigned:
				case RasterizerState::TextureModeCombineSubtract:
				case RasterizerState::TextureModeCombineDot3RGB:
				case RasterizerState::TextureModeCombineDot3RGBA:
					needsRGBForRGB |= (prevNeedsRGBForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcColor));

					needsAlphaForRGB |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsAlphaForRGB |= (prevNeedsAlphaForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[1] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[1] == RasterizerState::TextureCombineOpOneMinusSrcColor));

				case RasterizerState::TextureModeCombineReplace:
					needsRGBForRGB |= (prevNeedsRGBForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcColor));

					needsAlphaForRGB |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsAlphaForRGB |= (prevNeedsAlphaForRGB &&
						m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcColor)) ||
						(m_State->m_Texture[unit].CombineSrcRGB[0] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpSrcColor ||
						 m_State->m_Texture[unit].CombineOpRGB[0] == RasterizerState::TextureCombineOpOneMinusSrcColor));

				}

				switch (m_State->m_Texture[unit].CombineFuncAlpha) {
				default:
					assert(0);

				case RasterizerState::TextureModeCombineInterpolate:
					needsAlphaForAlpha |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[2] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcAlpha[2] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsRGBForAlpha |= (prevNeedsRGBForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[2] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[2] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

				case RasterizerState::TextureModeCombineModulate:
				case RasterizerState::TextureModeCombineAdd:
				case RasterizerState::TextureModeCombineAddSigned:
				case RasterizerState::TextureModeCombineSubtract:
					needsAlphaForAlpha |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[1] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcAlpha[1] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsRGBForAlpha |= (prevNeedsRGBForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[1] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[1] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

				case RasterizerState::TextureModeCombineReplace:
					needsAlphaForAlpha |= (prevNeedsAlphaForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[0] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpOneMinusSrcAlpha)) ||
						(m_State->m_Texture[unit].CombineSrcAlpha[0] == RasterizerState::TextureCombineSrcPrimaryColor &&
						(m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

					needsRGBForAlpha |= (prevNeedsRGBForAlpha &&
						m_State->m_Texture[unit].CombineSrcAlpha[0] == RasterizerState::TextureCombineSrcPrevious &&
						(m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpSrcAlpha ||
						 m_State->m_Texture[unit].CombineOpAlpha[0] == RasterizerState::TextureCombineOpOneMinusSrcAlpha));

				}

				if (m_State->m_Texture[unit].CombineFuncRGB == RasterizerState::TextureModeCombineDot3RGBA) {
					needsRGBForAlpha = needsRGBForRGB;
					needsAlphaForAlpha = needsAlphaForRGB;
				}

				break;

			}
		}
	}

	bool needsRGB     = needsRGBForRGB ||
		 needsRGBForAlpha && (m_State->m_Alpha.Enabled || m_State->m_Blend.Enabled || m_State->m_Mask.Alpha);
	bool needsAlpha   = needsAlphaForRGB ||
		 needsAlphaForAlpha && (m_State->m_Alpha.Enabled || m_State->m_Blend.Enabled || m_State->m_Mask.Alpha);

	// Do not have granularity of using only alpha at this point
	bool needsColor	  = needsRGB | needsAlpha;

	bool needsFog 	  = m_State->m_Fog.Enabled;
	bool needsDepth   = m_State->m_DepthTest.Enabled ||
						m_State->m_Mask.Depth 		 ||
						m_State->m_Stencil.Enabled;
	bool needsStencil = m_State->m_Stencil.Enabled;

	// now allocate slots

	I32 nextIndex = 0;

	if (needsColor) {
		m_VaryingInfo.colorIndex = nextIndex;
		nextIndex += 4;							// RGBA
	} else {
		m_VaryingInfo.colorIndex = -1;
	}

	if (needsFog) {
		m_VaryingInfo.fogIndex = nextIndex++;
	} else {
		m_VaryingInfo.fogIndex = -1;
	}

	for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (m_State->m_Texture[unit].Enabled && m_Texture[unit]) {
			m_VaryingInfo.textureBase[unit] = nextIndex;
			nextIndex += 2;						// u, v
		} else {
			m_VaryingInfo.textureBase[unit] = -1;
		}
	}

	m_VaryingInfo.numVarying = nextIndex;
}

void Rasterizer :: PrepareTexture() {
	if (m_State) {
		for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			if (m_State->m_Texture[unit].Enabled && m_Texture[unit]) {
				m_State->SetWrappingModeS(unit, m_Texture[unit]->GetWrappingModeS());
				m_State->SetWrappingModeT(unit, m_Texture[unit]->GetWrappingModeT());
				m_State->SetMinFilterMode(unit, m_Texture[unit]->GetMinFilterMode());
				m_State->SetMagFilterMode(unit, m_Texture[unit]->GetMagFilterMode());
				m_State->SetMipmapFilterMode(unit, m_Texture[unit]->GetMipmapFilterMode());
				m_State->SetInternalFormat(unit, m_Texture[unit]->GetInternalFormat());

				m_RasterInfo.Textures[unit] = m_Texture[unit]->m_TextureLevels;

				m_UseMipmap[unit] = m_Texture[unit]->IsMipMap() && m_Texture[unit]->IsComplete();
				m_RasterInfo.MaxMipmapLevel[unit] = EGL_Max(m_Texture[unit]->GetTexture(0)->GetLogWidth(), m_Texture[unit]->GetTexture(0)->GetLogHeight());
			} else {
				m_RasterInfo.Textures[unit] = 0;
				m_UseMipmap[unit] = false;
			}
		}
	}
}

void Rasterizer :: Prepare() {
	PrepareTexture();
}

#if !EGL_USE_JIT

inline void Rasterizer :: Fragment(I32 x, I32 y, U32 depth, EGL_Fixed tu[], EGL_Fixed tv[],
								   EGL_Fixed fogDensity, const Color& baseColor, EGL_Fixed coverage) {

	// pixel ownership test
	if (x < 0 || x >= m_Surface->GetWidth() ||
		y < 0 || y >= m_Surface->GetHeight()) {
		return;
	}

	// fragment level clipping (for now)
	if (m_State->m_ScissorTest.Enabled) {
		if (x < m_State->m_ScissorTest.X || x - m_State->m_ScissorTest.X >= m_State->m_ScissorTest.Width ||
			y < m_State->m_ScissorTest.Y || y - m_State->m_ScissorTest.Y >= m_State->m_ScissorTest.Height) {
			return;
		}
	}

	m_RasterInfo.Init(m_Surface, y, x);

	if (FragmentDepthStencil(&m_RasterInfo, &m_RasterInfo.RasterSurface, 0, depth)) {
		FragmentColorAlpha(&m_RasterInfo, &m_RasterInfo.RasterSurface, 0, tu, tv, baseColor, fogDensity, coverage);
	}
}


Color Rasterizer :: GetRawTexColor(const RasterizerState::TextureState * state, const Texture * texture, EGL_Fixed tu, EGL_Fixed tv) {
	// retrieve the texture color from a texture plane

	EGL_Fixed tu0;
	EGL_Fixed tv0;

	// for nearest texel

	switch (state->WrappingModeS) {
		case RasterizerState::WrappingModeClampToEdge:
			if (tu < 0)
				tu0 = 0;
			else if (tu >= EGL_ONE)
				tu0 = EGL_ONE - 1;
			else
				tu0 = tu;

			break;

		default:
		case RasterizerState::WrappingModeRepeat:
			tu0 = tu & 0xffff;
			break;
	}

	switch (state->WrappingModeT) {
		case RasterizerState::WrappingModeClampToEdge:
			if (tv < 0)
				tv0 = 0;
			else if (tv >= EGL_ONE)
				tv0 = EGL_ONE - 1;
			else tv0 = tv;

			break;

		default:
		case RasterizerState::WrappingModeRepeat:
			tv0 = tv & 0xffff;
			break;
	}

	// get the pixel color
	I32 texX = EGL_IntFromFixed(texture->GetWidth() * tu0);		// can become a shift
	I32 texY = EGL_IntFromFixed(texture->GetHeight() * tv0);	// can become a shift

	// do wrapping mode here
	I32 texOffset = texX + (texY << texture->GetLogWidth());

	void * data = texture->GetData();

	switch (state->InternalFormat) {
		default:
		case ColorFormatAlpha:
			//return Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);
			return Color(0, 0, 0, reinterpret_cast<const U8 *>(data)[texOffset]);

		case ColorFormatLuminance:
			{
			U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
			return Color (luminance, luminance, luminance, 0xff);
			}

		case ColorFormatRGB565:
			return Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);

		case ColorFormatRGB8:
			{
			texOffset = (texOffset << 1) + texOffset;
			const U8 * ptr = reinterpret_cast<const U8 *>(data) + texOffset;
			return Color(ptr[0], ptr[1], ptr[2], 0xff);
			}

		case ColorFormatLuminanceAlpha:
			{
			U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
			U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
			return Color (luminance, luminance, luminance, alpha);
			}

		case ColorFormatRGBA8:
			//texColor = Color::FromRGBA(reinterpret_cast<const U32 *>(data)[texOffset]);
			{
			texOffset = texOffset << 2;
			const U8 * ptr = reinterpret_cast<const U8 *>(data) + texOffset;
			return Color(ptr[0], ptr[1], ptr[2], ptr[3]);
			}

		case ColorFormatRGBA4444:
			return Color::From4444(reinterpret_cast<const U16 *>(data)[texOffset]);

		case ColorFormatRGBA5551:
			return Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
	}
}


inline Color Rasterizer :: GetTexColor(const RasterizerState::TextureState * state, const Texture * texture, EGL_Fixed tu, EGL_Fixed tv,
								RasterizerState::FilterMode filterMode) {
	// retrieve the texture color from a texture plane

	if (filterMode == RasterizerState::FilterModeNearest) {
		return GetRawTexColor(state, texture, tu, tv);
	} else if (filterMode == RasterizerState::FilterModeLinear) {
		I32 logWidth = texture->GetLogWidth();
		I32 logHeight = texture->GetLogHeight();

		EGL_Fixed tu0 = tu - (0x8000 >> logWidth);
		EGL_Fixed tu1 = tu + (0x7fff >> logWidth);
		EGL_Fixed tv0 = tv - (0x8000 >> logHeight);
		EGL_Fixed tv1 = tv + (0x7fff >> logHeight);

		U32 alpha = EGL_FractionFromFixed(tu0 << logWidth) >> 8;
		U32 beta = EGL_FractionFromFixed(tv0 << logHeight) >> 8;

		return Color::BlendAlpha(Color::BlendAlpha(GetRawTexColor(state, texture, tu1, tv1),
												   GetRawTexColor(state, texture, tu0, tv1), alpha),
								 Color::BlendAlpha(GetRawTexColor(state, texture, tu1, tv0),
												   GetRawTexColor(state, texture, tu0, tv0), alpha),
								 beta);
	} else {
		return Color(0, 0, 0, 0xff);
	}
}

U32 Rasterizer :: UpdateStencilValue(RasterizerState::StencilOp op, U32 stencilValue, U32 stencilRef) {

	int stencilBits;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		// fall through

	case DepthStencilFormatDepth16:
		return stencilValue;

	case DepthStencilFormatDepth16Stencil16:
		stencilBits = 16;
		break;
	}

	switch (op) {
		default:
		case RasterizerState::StencilOpKeep:
			break;

		case RasterizerState::StencilOpZero:
			stencilValue = 0;
			break;

		case RasterizerState::StencilOpReplace:
			stencilValue = stencilRef;
			break;

		case RasterizerState::StencilOpIncr:
			if (stencilValue != (1u << stencilBits) - 1) {
				stencilValue++;
			}

			break;

		case RasterizerState::StencilOpDecr:
			if (stencilValue != 0) {
				stencilValue--;
			}

			break;

		case RasterizerState::StencilOpInvert:
			stencilValue = ~stencilValue;
			break;
	}

	return stencilValue;
}

void Rasterizer::WriteDepthStencil(void * depthStencilAddr, U32 oldDepth, U32 newDepth, U32 oldStencil, U32 newStencil) {
	int stencilBits;
	U32 stencilBitMask, stencilWriteMask = m_State->GetStencilMask();
	U32 depthWriteMask = m_State->GetDepthMask() ? 0xffffffff : 0;
	int stencilShift, depthShift;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		return;

	case DepthStencilFormatDepth16:
		stencilBits = 0;
		stencilBitMask = 0;
		stencilWriteMask = 0;
		depthWriteMask &= 0xffff;
		stencilShift = 0;
		depthShift = 0;
		break;

	case DepthStencilFormatDepth16Stencil16:
		stencilBits = 16;
		stencilBitMask = 0xffff;
		stencilWriteMask &= stencilBitMask;
		depthWriteMask &= 0xffff;
		stencilShift = 16;
		depthShift = 0;
		break;
	}

	if (!m_State->GetDepthMask() &&!stencilWriteMask) {
			// no depth value to write
			// no stencil value to write
			// => Don't write anything
			return;
	}

	U32 stencilWriteValue =
		((oldStencil & (stencilBitMask & ~stencilWriteMask)) | (newStencil & stencilWriteMask)) << stencilShift;

	U32 depthWriteValue = 
		(m_State->GetDepthMask() ? newDepth : oldDepth) << depthShift;

	U32 depthStencilWriteValue = stencilWriteValue | depthWriteValue;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		return;

	case DepthStencilFormatDepth16:
		*(U16 *) depthStencilAddr = depthStencilWriteValue;
		break;

	case DepthStencilFormatDepth16Stencil16:
		*(U32 *) depthStencilAddr = depthStencilWriteValue;
		break;
	}
}

bool Rasterizer::FragmentDepthStencil(const RasterInfo * rasterInfo,
									  const SurfaceInfo * surfaceInfo,
									  U32 offset, U32 depth)
	// fragment rendering with signature corresponding to function fragment
	// generated by code generator
	// return true if the fragment passed depth and stencil tests
{
	void * depthStencilAddr =
		surfaceInfo->DepthStencilBuffer + (offset << surfaceInfo->DepthStencilOffsetShift);

	U32 depthStencilValue, stencilValue, zBufferValue;
	bool stencilPassAlways = false;

	switch (m_State->GetDepthStencilFormat()) {
	default:
		assert(false);
		return false;

	case DepthStencilFormatDepth16:
		depthStencilValue = zBufferValue = *(U16 *) depthStencilAddr;
		stencilValue = 0;
		stencilPassAlways = true;
		break;

	case DepthStencilFormatDepth16Stencil16:
		depthStencilValue = *(U32 *) depthStencilAddr;
		zBufferValue = depthStencilValue & 0xffff;
		stencilValue = depthStencilValue >> 16;
		break;
	}

	if (m_State->m_Stencil.Enabled) {
		U32 stencilRef = m_State->m_Stencil.Reference & m_State->m_Stencil.ComparisonMask;
		U32 stencil = stencilValue & m_State->m_Stencil.ComparisonMask;
		bool stencilTest;

		switch (m_State->m_Stencil.Func) {
			default:
			case RasterizerState::CompFuncNever:	stencilTest = false;				break;
			case RasterizerState::CompFuncLess:		stencilTest = stencilRef < stencil;	break;
			case RasterizerState::CompFuncEqual:	stencilTest = stencilRef == stencil;break;
			case RasterizerState::CompFuncLEqual:	stencilTest = stencilRef <= stencil;break;
			case RasterizerState::CompFuncGreater:	stencilTest = stencilRef > stencil;	break;
			case RasterizerState::CompFuncNotEqual:	stencilTest = stencilRef != stencil;break;
			case RasterizerState::CompFuncGEqual:	stencilTest = stencilRef >= stencil;break;
			case RasterizerState::CompFuncAlways:	                					break;
		}

		if (!(stencilTest || stencilPassAlways)) {

			U32 newStencilValue = UpdateStencilValue(m_State->m_Stencil.Fail, stencilValue, m_State->m_Stencil.Reference);
			WriteDepthStencil(depthStencilAddr, zBufferValue, zBufferValue, stencilValue, newStencilValue);

			return false;
		}

		bool depthTest;

		if (m_State->m_DepthTest.Enabled) {
			depth = depth > 0xffff ? 0xffff : depth;
			assert(depth >= 0 && depth <= 0xffff);

			switch (m_State->m_DepthTest.Func) {
				default:
				case RasterizerState::CompFuncNever:	depthTest = false;						break;
				case RasterizerState::CompFuncLess:		depthTest = depth < zBufferValue;		break;
				case RasterizerState::CompFuncEqual:	depthTest = depth == zBufferValue;		break;
				case RasterizerState::CompFuncLEqual:	depthTest = depth <= zBufferValue;		break;
				case RasterizerState::CompFuncGreater:	depthTest = depth > zBufferValue;		break;
				case RasterizerState::CompFuncNotEqual:	depthTest = depth != zBufferValue;		break;
				case RasterizerState::CompFuncGEqual:	depthTest = depth >= zBufferValue;		break;
				case RasterizerState::CompFuncAlways:	                                        break;
			}
		} else {
			depthTest = true;
		}

		if (depthTest) {
			U32 newStencilValue = UpdateStencilValue(m_State->m_Stencil.ZPass, stencilValue, m_State->m_Stencil.Reference);
			WriteDepthStencil(depthStencilAddr, zBufferValue, depth, stencilValue, newStencilValue);

			return true;

		} else {
			U32 newStencilValue = UpdateStencilValue(m_State->m_Stencil.ZFail, stencilValue, m_State->m_Stencil.Reference);
			WriteDepthStencil(depthStencilAddr, zBufferValue, zBufferValue, stencilValue, newStencilValue);

			return false;
		}
	} else if (m_State->m_DepthTest.Enabled) {
		depth = depth > 0xffff ? 0xffff : depth;
	    assert(depth >= 0 && depth <= 0xffff);
		bool depthTest;

	    switch (m_State->m_DepthTest.Func) {
		    default:
		    case RasterizerState::CompFuncNever:	depthTest = false;						break;
		    case RasterizerState::CompFuncLess:		depthTest = depth < zBufferValue;		break;
		    case RasterizerState::CompFuncEqual:	depthTest = depth == zBufferValue;		break;
		    case RasterizerState::CompFuncLEqual:	depthTest = depth <= zBufferValue;		break;
		    case RasterizerState::CompFuncGreater:	depthTest = depth > zBufferValue;		break;
		    case RasterizerState::CompFuncNotEqual:	depthTest = depth != zBufferValue;		break;
		    case RasterizerState::CompFuncGEqual:	depthTest = depth >= zBufferValue;		break;
		    case RasterizerState::CompFuncAlways:	                                        break;
	    }

	    if (!depthTest)
		    return false;
	}

	WriteDepthStencil(depthStencilAddr, zBufferValue, depth, stencilValue, stencilValue);

	return true;
}

namespace {
	Color FetchColor(const SurfaceInfo * surfaceInfo, size_t offset) {
		switch (surfaceInfo->ColorFormat) {
		case ColorFormatRGBA4444:
			return Color::From4444(*(U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift)));

		case ColorFormatRGBA5551:
			return Color::From5551(*(U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift)));

		case ColorFormatRGB565:
			return Color::From565(*(U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift)));

		case ColorFormatRGBA8:
			return Color::FromRGBA(*(U32 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift)));

		default:
			assert(false);
			return Color(0, 0, 0, 0);
		}
	}

	void WriteColor(const SurfaceInfo * surfaceInfo, size_t offset, Color color) {
		switch (surfaceInfo->ColorFormat) {
		case ColorFormatRGBA4444:
			*((U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift))) = color.ConvertTo4444();
			break;

		case ColorFormatRGBA5551:
			*((U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift))) = color.ConvertTo5551();
			break;

		case ColorFormatRGB565:
			*((U16 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift))) = color.ConvertTo565();
			break;

		case ColorFormatRGBA8:
			*((U32 *) (surfaceInfo->ColorBuffer + (offset << surfaceInfo->ColorOffsetShift))) = color.ConvertToRGBA();
			break;

		default:
			assert(false);
			break;
		}
	}
}

void Rasterizer::FragmentColorAlpha(const RasterInfo * rasterInfo,
									const SurfaceInfo * surfaceInfo,
									U32 offset, EGL_Fixed tu[], EGL_Fixed tv[],
									const Color& baseColor, EGL_Fixed fog, 
									EGL_Fixed coverage)
	// fragment rendering with signature corresponding to function fragment
	// generated by code generator
{
	Color color(baseColor);

	// have offset, color, texOffset, texture

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (m_State->m_Texture[unit].Enabled) {

			const Texture * texture = m_RasterInfo.Textures[unit] ;//+ m_RasterInfo.MipmapLevel[unit];
			Color texColor = GetTexColor(&m_State->m_Texture[unit], texture, tu[unit], tv[unit], m_State->GetMinFilterMode(unit));

			if (m_State->m_Texture[unit].Mode == RasterizerState::TextureModeCombine) {
				Color arg[3];

				for (size_t idx = 0; idx < 3; ++idx) {
					Color rgbIn;
					U8 alphaIn;

					switch (m_State->m_Texture[unit].CombineSrcRGB[idx]) {
					case RasterizerState::TextureCombineSrcTexture:
						rgbIn = texColor;
						break;

					case RasterizerState::TextureCombineSrcConstant:
						rgbIn = m_State->m_Texture[unit].EnvColor;
						break;

					case RasterizerState::TextureCombineSrcPrimaryColor:
						rgbIn = baseColor;
						break;

					case RasterizerState::TextureCombineSrcPrevious:
						rgbIn = color;
						break;
					}

					switch (m_State->m_Texture[unit].CombineSrcAlpha[idx]) {
					case RasterizerState::TextureCombineSrcTexture:
						alphaIn = texColor.a;
						break;

					case RasterizerState::TextureCombineSrcConstant:
						alphaIn = m_State->m_Texture[unit].EnvColor.a;
						break;

					case RasterizerState::TextureCombineSrcPrimaryColor:
						alphaIn = baseColor.a;
						break;

					case RasterizerState::TextureCombineSrcPrevious:
						alphaIn = color.a;
						break;
					}

					U8 alphaOut;

					if (m_State->m_Texture[unit].CombineOpAlpha[idx] == RasterizerState::TextureCombineOpSrcAlpha) {
						alphaOut = alphaIn;
					} else {
						alphaOut = 0xFF - alphaIn;
					}

					switch (m_State->m_Texture[unit].CombineOpRGB[idx]) {
					case RasterizerState::TextureCombineOpSrcColor:
						arg[idx] = Color(rgbIn.r, rgbIn.g, rgbIn.b, alphaOut);
						break;

					case RasterizerState::TextureCombineOpOneMinusSrcColor:
						arg[idx] = Color(0xFF - rgbIn.r, 0xFF - rgbIn.g, 0xFF - rgbIn.b, alphaOut);
						break;

					case RasterizerState::TextureCombineOpSrcAlpha:
						arg[idx] = Color(rgbIn.a, rgbIn.a, rgbIn.a, alphaOut);
						break;

					case RasterizerState::TextureCombineOpOneMinusSrcAlpha:
						arg[idx] = Color(0xFF - rgbIn.a, 0xFF - rgbIn.a, 0xFF - rgbIn.a, alphaOut);
						break;
					}
				}

				U8 combineAlpha;
				EGL_Fixed scaleAlpha = m_State->m_Texture[unit].ScaleAlpha;

				switch (m_State->m_Texture[unit].CombineFuncAlpha) {
				case RasterizerState::TextureModeCombineReplace:
					combineAlpha = MulU8(arg[0].a, 0xFF, scaleAlpha);
					break;

				case RasterizerState::TextureModeCombineModulate:
					combineAlpha = MulU8(arg[0].a, arg[1].a, scaleAlpha);
					break;

				case RasterizerState::TextureModeCombineAdd:
					combineAlpha = AddU8(arg[0].a, arg[1].a, scaleAlpha);
					break;

				case RasterizerState::TextureModeCombineAddSigned:
					combineAlpha = AddSignedU8(arg[0].a, arg[1].a, scaleAlpha);
					break;

				case RasterizerState::TextureModeCombineInterpolate:
					combineAlpha = InterpolateU8(arg[0].a, arg[1].a, arg[2].a, scaleAlpha);
					break;

				case RasterizerState::TextureModeCombineSubtract:
					combineAlpha = SubU8(arg[0].a, arg[1].a, scaleAlpha);
					break;
				}

				EGL_Fixed scaleRGB = m_State->m_Texture[unit].ScaleRGB;

				switch (m_State->m_Texture[unit].CombineFuncRGB) {
				case RasterizerState::TextureModeCombineReplace:
					color = Color(
							MulU8(arg[0].r, 0xFF, scaleRGB),
							MulU8(arg[0].g, 0xFF, scaleRGB),
							MulU8(arg[0].b, 0xFF, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineModulate:
					color =
						Color(
							MulU8(arg[0].r, arg[1].r, scaleRGB),
							MulU8(arg[0].g, arg[1].g, scaleRGB),
							MulU8(arg[0].b, arg[1].b, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineAdd:
					color =
						Color(
							AddU8(arg[0].r, arg[1].r, scaleRGB),
							AddU8(arg[0].g, arg[1].g, scaleRGB),
							AddU8(arg[0].b, arg[1].b, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineAddSigned:
					color =
						Color(
							AddSignedU8(arg[0].r, arg[1].r, scaleRGB),
							AddSignedU8(arg[0].g, arg[1].g, scaleRGB),
							AddSignedU8(arg[0].b, arg[1].b, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineInterpolate:
					color =
						Color(
							InterpolateU8(arg[0].r, arg[1].r, arg[2].r, scaleRGB),
							InterpolateU8(arg[0].g, arg[1].g, arg[2].g, scaleRGB),
							InterpolateU8(arg[0].b, arg[1].b, arg[2].b, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineSubtract:
					color =
						Color(
							SubU8(arg[0].r, arg[1].r, scaleRGB),
							SubU8(arg[0].g, arg[1].g, scaleRGB),
							SubU8(arg[0].b, arg[1].b, scaleRGB),
							combineAlpha);
					break;

				case RasterizerState::TextureModeCombineDot3RGB:
					{
						U8 dotRGB = Dot3U8(arg[0], arg[1], scaleRGB);
						color = Color(dotRGB, dotRGB, dotRGB, combineAlpha);
					}

					break;

				case RasterizerState::TextureModeCombineDot3RGBA:
					{
						U8 dotRGB = Dot3U8(arg[0], arg[1], scaleRGB);
						U8 dotAlpha = Dot3U8(arg[0], arg[1], scaleAlpha);
						color = Color(dotRGB, dotRGB, dotRGB, dotAlpha);
					}

					break;
				}
			} else {
				switch (m_Texture[unit]->GetInternalFormat()) {
					default:
					case ColorFormatAlpha:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeReplace:
								color = Color(color.r, color.g, color.b, texColor.a);
								break;

							case RasterizerState::TextureModeModulate:
							case RasterizerState::TextureModeBlend:
							case RasterizerState::TextureModeAdd:
								color = Color(color.r, color.g, color.b, MulU8(color.a, texColor.a));
								break;
						}
						break;

					case ColorFormatLuminance:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeDecal:
							case RasterizerState::TextureModeReplace:
								color = Color(texColor.r, texColor.g, texColor.b, color.a);
								break;

							case RasterizerState::TextureModeModulate:
								color = Color(MulU8(color.r, texColor.r),
									MulU8(color.g, texColor.g), MulU8(color.b, texColor.b), color.a);
								break;

							case RasterizerState::TextureModeBlend:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[unit].EnvColor.r, texColor.r),
										MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[unit].EnvColor.g, texColor.g),
										MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[unit].EnvColor.b, texColor.b),
										color.a);
								break;

							case RasterizerState::TextureModeAdd:
								color =
									Color(
										ClampU8(color.r + texColor.r),
										ClampU8(color.g + texColor.g),
										ClampU8(color.b + texColor.b),
										color.a);
								break;
						}
						break;

					case ColorFormatRGB565:
					case ColorFormatRGB8:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeDecal:
							case RasterizerState::TextureModeReplace:
								color = Color(texColor.r, texColor.g, texColor.b, color.a);
								break;

							case RasterizerState::TextureModeModulate:
								color = Color(MulU8(color.r, texColor.r),
									MulU8(color.g, texColor.g), MulU8(color.b, texColor.b), color.a);
								break;

							case RasterizerState::TextureModeBlend:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[unit].EnvColor.r, texColor.r),
										MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[unit].EnvColor.g, texColor.g),
										MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[unit].EnvColor.b, texColor.b),
										color.a);
								break;

							case RasterizerState::TextureModeAdd:
								color =
									Color(
										ClampU8(color.r + texColor.r),
										ClampU8(color.g + texColor.g),
										ClampU8(color.b + texColor.b),
										color.a);
								break;
						}
						break;

					case ColorFormatLuminanceAlpha:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeReplace:
								color = texColor;
								break;

							case RasterizerState::TextureModeModulate:
								color = color * texColor;
								break;

							case RasterizerState::TextureModeDecal:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.a) + MulU8(texColor.r, texColor.a),
										MulU8(color.g, 0xff - texColor.a) + MulU8(texColor.g, texColor.a),
										MulU8(color.b, 0xff - texColor.a) + MulU8(texColor.b, texColor.a),
										color.a);
								break;

							case RasterizerState::TextureModeBlend:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[unit].EnvColor.r, texColor.r),
										MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[unit].EnvColor.g, texColor.g),
										MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[unit].EnvColor.b, texColor.b),
										MulU8(color.a, texColor.a));
								break;

							case RasterizerState::TextureModeAdd:
								color =
									Color(
										ClampU8(color.r + texColor.r),
										ClampU8(color.g + texColor.g),
										ClampU8(color.b + texColor.b),
										MulU8(color.a, texColor.a));
								break;
						}
						break;

					case ColorFormatRGBA5551:
					case ColorFormatRGBA4444:
					case ColorFormatRGBA8:
						switch (m_State->m_Texture[unit].Mode) {
							case RasterizerState::TextureModeReplace:
								color = texColor;
								break;

							case RasterizerState::TextureModeModulate:
								color = color * texColor;
								break;

							case RasterizerState::TextureModeDecal:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.a) + MulU8(texColor.r, texColor.a),
										MulU8(color.g, 0xff - texColor.a) + MulU8(texColor.g, texColor.a),
										MulU8(color.b, 0xff - texColor.a) + MulU8(texColor.b, texColor.a),
										color.a);
								break;

							case RasterizerState::TextureModeBlend:
								color =
									Color(
										MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[unit].EnvColor.r, texColor.r),
										MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[unit].EnvColor.g, texColor.g),
										MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[unit].EnvColor.b, texColor.b),
										MulU8(color.a, texColor.a));
								break;

							case RasterizerState::TextureModeAdd:
								color =
									Color(
										ClampU8(color.r + texColor.r),
										ClampU8(color.g + texColor.g),
										ClampU8(color.b + texColor.b),
										MulU8(color.a, texColor.a));
								break;
						}
						break;
				}
			}
		}
	}

	// fog
	if (m_State->m_Fog.Enabled) {
		color = Color::Blend(color, m_State->m_Fog.Color, fog >> 8);
	}

	// have color; apply coverage value if anti-aliasing is enabled
	color.ScaleA(coverage);

	if (m_State->m_Alpha.Enabled) {
		bool alphaTest;
		U8 alpha = color.A();
		U8 alphaRef = EGL_IntFromFixed(m_State->m_Alpha.Reference * 255);

		switch (m_State->m_Alpha.Func) {
			default:
			case RasterizerState::CompFuncNever:	alphaTest = false;					break;
			case RasterizerState::CompFuncLess:		alphaTest = alpha < alphaRef;		break;
			case RasterizerState::CompFuncEqual:	alphaTest = alpha == alphaRef;		break;
			case RasterizerState::CompFuncLEqual:	alphaTest = alpha <= alphaRef;		break;
			case RasterizerState::CompFuncGreater:	alphaTest = alpha > alphaRef;		break;
			case RasterizerState::CompFuncNotEqual:	alphaTest = alpha != alphaRef;		break;
			case RasterizerState::CompFuncGEqual:	alphaTest = alpha >= alphaRef;		break;
			case RasterizerState::CompFuncAlways:	alphaTest = true;					break;
		}

		if (!alphaTest) {
			return;
		}
	}

	// have color, offset

	// Blending
	if (m_State->m_Blend.Enabled) {

		Color dstColor = FetchColor(surfaceInfo, offset);
		Color srcCoeff, dstCoeff;

		switch (m_State->m_Blend.FuncSrc) {
			default:
			case RasterizerState::BlendFuncSrcZero:
				srcCoeff = Color(0, 0, 0, 0);
				break;

			case RasterizerState::BlendFuncSrcOne:
				srcCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);
				break;

			case RasterizerState::BlendFuncSrcDstColor:
				srcCoeff = dstColor;
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstColor:
				srcCoeff = Color(Color::MAX - dstColor.R(), Color::MAX - dstColor.G(), Color::MAX - dstColor.B(), Color::MAX - dstColor.A());
				break;

			case RasterizerState::BlendFuncSrcSrcAlpha:
				srcCoeff = Color(color.A(), color.A(), color.A(), color.A());
				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcAlpha:
				srcCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncSrcDstAlpha:
				srcCoeff = Color(dstColor.A(), dstColor.A(), dstColor.A(), dstColor.A());
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstAlpha:
				srcCoeff = Color(Color::MAX - dstColor.A(), Color::MAX - dstColor.A(), Color::MAX - dstColor.A(), Color::MAX - dstColor.A());
				break;

			case RasterizerState::BlendFuncSrcSrcAlphaSaturate:
				{
					U8 rev = Color::MAX - dstColor.A();
					U8 f = (rev < color.A() ? rev : color.A());
					srcCoeff = Color(f, f, f, Color::MAX);
				}
				break;
		}

		switch (m_State->m_Blend.FuncDst) {
			default:
			case RasterizerState::BlendFuncDstZero:
				dstCoeff = Color(0, 0, 0, 0);
				break;

			case RasterizerState::BlendFuncDstOne:
				dstCoeff = Color(Color::MAX, Color::MAX, Color::MAX, Color::MAX);
				break;

			case RasterizerState::BlendFuncDstSrcColor:
				dstCoeff = color;
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcColor:
				dstCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncDstSrcAlpha:
				dstCoeff = Color(color.A(), color.A(), color.A(), color.A());
				break;

			case RasterizerState::BlendFuncDstOneMinusSrcAlpha:
				dstCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncDstDstAlpha:
				dstCoeff = Color(dstColor.A(), dstColor.A(), dstColor.A(), dstColor.A());
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				dstCoeff = Color(Color::MAX - dstColor.A(), Color::MAX - dstColor.A(), Color::MAX - dstColor.A(), Color::MAX - dstColor.A());
				break;

		}

		color = srcCoeff * color + dstCoeff * dstColor;
	}

	// have offset, depth, color

	Color maskedColor =
		color.Mask(m_State->m_Mask.Red, m_State->m_Mask.Green, m_State->m_Mask.Blue, m_State->m_Mask.Alpha);

	if (m_State->m_LogicOp.Enabled) {

		U32 oldValue = FetchColor(surfaceInfo, offset).ConvertToRGBA();
		U32 newValue = maskedColor.ConvertToRGBA();

		U32 value;

		switch (m_State->m_LogicOp.Opcode) {
			default:
			case RasterizerState:: LogicOpClear:
				value = 0;
				break;

			case RasterizerState:: LogicOpAnd:
				value = newValue & oldValue;
				break;

			case RasterizerState:: LogicOpAndReverse:
				value = newValue & ~oldValue;
				break;

			case RasterizerState:: LogicOpCopy:
				value = newValue;
				break;

			case RasterizerState:: LogicOpAndInverted:
				value = ~newValue & oldValue;
				break;

			case RasterizerState:: LogicOpNoop:
				value = oldValue;
				break;

			case RasterizerState:: LogicOpXor:
				value = newValue ^ oldValue;
				break;

			case RasterizerState:: LogicOpOr:
				value = newValue | oldValue;
				break;

			case RasterizerState:: LogicOpNor:
				value = ~(newValue | oldValue);
				break;

			case RasterizerState:: LogicOpEquiv:
				value = ~(newValue ^ oldValue);
				break;

			case RasterizerState:: LogicOpInvert:
				value = ~oldValue;
				break;

			case RasterizerState:: LogicOpOrReverse:
				value = newValue | ~oldValue;
				break;

			case RasterizerState:: LogicOpCopyInverted:
				value = ~newValue;
				break;

			case RasterizerState:: LogicOpOrInverted:
				value = ~newValue | oldValue;
				break;

			case RasterizerState:: LogicOpNand:
				value = ~(newValue & oldValue);
				break;

			case RasterizerState:: LogicOpSet:
				value = Color(0xff, 0xff, 0xff, 0xff).ConvertToRGBA();
				break;
		}

		WriteColor(surfaceInfo, offset, Color::FromRGBA(value));
	} else {
		WriteColor(surfaceInfo, offset, maskedColor);
	}
}

#endif // !EGL_USE_JIT

// --------------------------------------------------------------------------
// Prepare rasterizer with according to current state settings
// --------------------------------------------------------------------------

void Rasterizer :: PreparePoint() {
	Prepare();
	m_FunctionCache->PrepareFunction(PipelinePart::PartRasterPoint,
									 m_State, &m_VaryingInfo);
}


void Rasterizer :: PrepareLine() {
	Prepare();
	m_FunctionCache->PrepareFunction(PipelinePart::PartRasterLine,
									 m_State, &m_VaryingInfo);
}

void Rasterizer :: BeginPoint() {

	m_PointFunction = (PointFunction *)
		m_FunctionCache->GetFunction(PipelinePart::PartRasterPoint,
									 m_State);

	m_RasterInfo.Init(m_Surface, 0);
	memset(m_RasterInfo.MipmapLevel, 0, sizeof(m_RasterInfo.MipmapLevel));
}


void Rasterizer :: BeginLine() {
	m_LineFunction = (LineFunction *)
		m_FunctionCache->GetFunction(PipelinePart::PartRasterLine,
									 m_State);

	m_RasterInfo.Init(m_Surface, 0);
	memset(m_RasterInfo.MipmapLevel, 0, sizeof(m_RasterInfo.MipmapLevel));
}

void Rasterizer :: End() {
}


#if !EGL_USE_JIT

void Rasterizer :: RasterLine(Vertex& p_from, Vertex& p_to) {

	if (EGL_Round(p_from.m_WindowCoords.x) == EGL_Round(p_to.m_WindowCoords.x) &&
		EGL_Round(p_from.m_WindowCoords.y) == EGL_Round(p_to.m_WindowCoords.y)) {
		// both ends of line on same pixel
		RasterPoint(p_from, EGL_ONE);
		return;
	}

	EGL_Fixed deltaX = p_to.m_WindowCoords.x - p_from.m_WindowCoords.x;
	EGL_Fixed deltaY = p_to.m_WindowCoords.y - p_from.m_WindowCoords.y;

	if (EGL_Abs(deltaX) > EGL_Abs(deltaY)) {
		// Bresenham along x-axis

		const Vertex *start, *end;

		I32 x;
		I32 endX;

		if (deltaX < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			x = EGL_IntFromFixed(p_to.m_WindowCoords.x) + 1;
			endX = EGL_IntFromFixed(p_from.m_WindowCoords.x) + 1;
		} else {
			start = &p_from;
			end = &p_to;
			x = EGL_IntFromFixed(p_from.m_WindowCoords.x);
			endX = EGL_IntFromFixed(p_to.m_WindowCoords.x);
		}

		const Vertex& from = *start;
		const Vertex& to = *end;

		FractionalColor baseColor(from.m_Varying + m_VaryingInfo.colorIndex);
		EGL_Fixed OneOverZ = from.m_WindowCoords.invW;
		EGL_Fixed tuOverZ[EGL_NUM_TEXTURE_UNITS];
		EGL_Fixed tvOverZ[EGL_NUM_TEXTURE_UNITS];

		size_t unit;

		for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			I32 textureBase = m_VaryingInfo.textureBase[unit];
			tuOverZ[unit] = EGL_Mul(from.m_Varying[textureBase], OneOverZ);
			tvOverZ[unit] = EGL_Mul(from.m_Varying[textureBase + 1], OneOverZ);
		}

		EGL_Fixed fogDensity = from.m_Varying[m_VaryingInfo.fogIndex];
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed OneOverZTo = to.m_WindowCoords.invW;
		EGL_Fixed invSpan = EGL_Inverse(deltaX);
		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaY), invSpan);

		// -- increments(to, from, invSpan)
		FractionalColor colorIncrement = (FractionalColor(to.m_Varying + m_VaryingInfo.colorIndex) -
			FractionalColor(from.m_Varying + m_VaryingInfo.colorIndex)) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_Varying[m_VaryingInfo.fogIndex] - from.m_Varying[m_VaryingInfo.fogIndex], invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU[EGL_NUM_TEXTURE_UNITS], deltaV[EGL_NUM_TEXTURE_UNITS];

		for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			I32 textureBase = m_VaryingInfo.textureBase[unit];
			deltaU[unit] = EGL_Mul(EGL_Mul(to.m_Varying[textureBase], OneOverZTo) -
									   EGL_Mul(from.m_Varying[textureBase], OneOverZ), invSpan);

			deltaV[unit] = EGL_Mul(EGL_Mul(to.m_Varying[textureBase + 1], OneOverZTo) -
									   EGL_Mul(from.m_Varying[textureBase + 1], OneOverZ), invSpan);
		}

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);
		// -- end increments

		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y);

		I32 yIncrement = (deltaY > 0) ? 1 : -1;
		EGL_Fixed error = 0;//EGL_ONE/2;

		for (; x < endX; ++x) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];
			size_t unit;

			for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
				tu[unit] = EGL_Mul(tuOverZ[unit], z);
				tv[unit] = EGL_Mul(tvOverZ[unit], z);
			}

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor, EGL_ONE);

			error += slope;
			if (error > EGL_ONE) {
				y += yIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			depth += deltaDepth;
			OneOverZ += deltaZ;

			for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
				tuOverZ[unit] += deltaU[unit];
				tvOverZ[unit] += deltaV[unit];
			}

			fogDensity += deltaFog;
		}

	} else {
		// Bresenham along y-axis

		const Vertex *start, *end;

		I32 y;
		I32 endY;

		if (deltaY < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			y = EGL_IntFromFixed(p_to.m_WindowCoords.y) + 1;
			endY = EGL_IntFromFixed(p_from.m_WindowCoords.y) + 1;
		} else {
			start = &p_from;
			end = &p_to;
			y = EGL_IntFromFixed(p_from.m_WindowCoords.y);
			endY = EGL_IntFromFixed(p_to.m_WindowCoords.y);
		}

		const Vertex& from = *start;
		const Vertex& to = *end;

		FractionalColor baseColor(from.m_Varying + m_VaryingInfo.colorIndex);
		EGL_Fixed OneOverZ = from.m_WindowCoords.invW;
		EGL_Fixed tuOverZ[EGL_NUM_TEXTURE_UNITS];
		EGL_Fixed tvOverZ[EGL_NUM_TEXTURE_UNITS];

		size_t unit;

		for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			I32 textureBase = m_VaryingInfo.textureBase[unit];
			tuOverZ[unit] = EGL_Mul(from.m_Varying[textureBase], OneOverZ);
			tvOverZ[unit] = EGL_Mul(from.m_Varying[textureBase + 1], OneOverZ);
		}

		EGL_Fixed fogDensity = from.m_Varying[m_VaryingInfo.fogIndex];
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed OneOverZTo = to.m_WindowCoords.invW;
		EGL_Fixed invSpan = EGL_Inverse(deltaY);
		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaX), invSpan);

		// -- increments(to, from, invSpan)
		FractionalColor colorIncrement = (FractionalColor(to.m_Varying + m_VaryingInfo.colorIndex)
			- FractionalColor(from.m_Varying + m_VaryingInfo.colorIndex)) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_Varying[m_VaryingInfo.fogIndex] - from.m_Varying[m_VaryingInfo.fogIndex], invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU[EGL_NUM_TEXTURE_UNITS], deltaV[EGL_NUM_TEXTURE_UNITS];

		for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			I32 textureBase = m_VaryingInfo.textureBase[unit];
			deltaU[unit] = EGL_Mul(EGL_Mul(to.m_Varying[textureBase], OneOverZTo) -
									   EGL_Mul(from.m_Varying[textureBase], OneOverZ), invSpan);

			deltaV[unit] = EGL_Mul(EGL_Mul(to.m_Varying[textureBase + 1], OneOverZTo) -
									   EGL_Mul(from.m_Varying[textureBase + 1], OneOverZ), invSpan);
		}

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);
		// -- end increments

#if 0
		baseColor.r += EGL_Mul(colorIncrement.r, preStepY);
		baseColor.g += EGL_Mul(colorIncrement.g, preStepY);
		baseColor.b += EGL_Mul(colorIncrement.b, preStepY);
		baseColor.a += EGL_Mul(colorIncrement.a, preStepY);

		fogDensity  += EGL_Mul(deltaFog, preStepY);
		depth       += EGL_Mul(deltaDepth, preStepY);

		OneOverZ	+= EGL_Mul(deltaZ, preStepY);

		for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			tuOverZ[unit]		+= EGL_Mul(deltaU[unit], preStepX);
			tvOverZ[unit]		+= EGL_Mul(deltaV[unit], preStepX);
		}
#endif

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x); // + ((EGL_ONE)/2-1));

		I32 xIncrement = (deltaX > 0) ? 1 : -1;
		EGL_Fixed error = 0;//EGL_ONE/2;

		// can have xIncrement; yIncrement; xBaseIncrement, yBaseIncrement
		// then both x/y loops become the same
		// question: how to add correct mipmap selection?

		for (; y < endY; ++y) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];
			size_t unit;

			for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
				tu[unit] = EGL_Mul(tuOverZ[unit], z);
				tv[unit] = EGL_Mul(tvOverZ[unit], z);
			}

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor, EGL_ONE);

			error += slope;
			if (error > EGL_ONE) {
				x += xIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			depth += deltaDepth;
			OneOverZ += deltaZ;

			for (unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
				tuOverZ[unit] += deltaU[unit];
				tvOverZ[unit] += deltaV[unit];
			}

			fogDensity += deltaFog;
		}
	}
}


void Rasterizer :: RasterPoint(const Vertex& point, EGL_Fixed size) {

	EGL_Fixed halfSize = size / 2;

	I32 xmin = EGL_IntFromFixed(point.m_WindowCoords.x - halfSize + EGL_HALF);
	I32 xmax = xmin + ((size - EGL_HALF) >> EGL_PRECISION);
	I32 ymin = EGL_IntFromFixed(point.m_WindowCoords.y - halfSize + EGL_HALF);
	I32 ymax = ymin + ((size - EGL_HALF) >> EGL_PRECISION);

	EGL_Fixed depth = point.m_WindowCoords.depth;
	FractionalColor baseColor(point.m_Varying + m_VaryingInfo.colorIndex);
	EGL_Fixed fogDensity = point.m_Varying[m_VaryingInfo.fogIndex];


	if (!m_State->m_Point.SpriteEnabled) {
		EGL_Fixed tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];

		for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			I32 textureBase = m_VaryingInfo.textureBase[unit];
			tu[unit] = point.m_Varying[textureBase];
			tv[unit] = point.m_Varying[textureBase + 1];
		}

		for (I32 y = ymin; y <= ymax; y++) {
			for (I32 x = xmin; x <= xmax; x++) {
				Fragment(x, y, depth, tu, tv, fogDensity, baseColor, EGL_ONE);
			}
		}
	} else {
		EGL_Fixed delta = EGL_Inverse(size);

		for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			if (m_UseMipmap[unit] && m_State->m_Texture[unit].CoordReplaceEnabled) {
				EGL_Fixed maxDu = delta >> (16 - m_Texture[unit]->GetTexture(0)->GetLogWidth());
				EGL_Fixed maxDv = delta >> (16 - m_Texture[unit]->GetTexture(0)->GetLogHeight());

				EGL_Fixed rho = maxDu + maxDv;

				// we start with nearest/minification only selection; will add LINEAR later

				m_RasterInfo.MipmapLevel[unit] = EGL_Min(Log2(rho), m_RasterInfo.MaxMipmapLevel[unit]);
			} else {
				m_RasterInfo.MipmapLevel[unit] = 0;
			}
		}

		EGL_Fixed tu[EGL_NUM_TEXTURE_UNITS], tv[EGL_NUM_TEXTURE_UNITS];

		for (I32 y = ymin, tv0 = delta / 2; y <= ymax; y++, tv0 += delta) {

			for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
				I32 textureBase = m_VaryingInfo.textureBase[unit];

				if (m_State->m_Texture[unit].CoordReplaceEnabled)
					tv[unit] = tv0;
				else
					tv[unit] = point.m_Varying[textureBase + 1];
			}

			for (I32 x = xmin, tu0 = delta / 2; x <= xmax; x++, tu0 += delta) {
				for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
					I32 textureBase = m_VaryingInfo.textureBase[unit];

					if (m_State->m_Texture[unit].CoordReplaceEnabled)
						tu[unit] = tu0;
					else
						tu[unit] = point.m_Varying[textureBase];
				}

				Fragment(x, y, depth, tu, tv, fogDensity, baseColor, EGL_ONE);
			}
		}
	}
}

#endif // !EGL_USE_JIT
