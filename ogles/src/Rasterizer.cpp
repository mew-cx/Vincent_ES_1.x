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
#include "arm/FunctionCache.h"

using namespace EGL;


// --------------------------------------------------------------------------
// Local helper functions
// --------------------------------------------------------------------------

// TODO: Add lookup table for multiplication of 0..0xff * 0..0xff

namespace {

	inline U8 MulU8(U8 a, U8 b) {
		U16 prod = a * b;
		return (prod + (prod >> 7)) >> 8;
	}

	inline U8 ClampU8(U16 value) {
		return (value > 0xff) ? (U8) 0xff : (U8) value;
	}


}


// --------------------------------------------------------------------------
// Class Rasterizer
// --------------------------------------------------------------------------


Rasterizer :: Rasterizer(RasterizerState * state):
	m_State(state)
{
	m_FunctionCache = new FunctionCache();

	memset(m_RasterTriangleFunctions, 0, sizeof m_RasterTriangleFunctions);

	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)	] = RasterTriangle_cTdfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleFog)		] = RasterTriangle_cTdFs;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleDepth)	] = RasterTriangle_cTDfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		] = RasterTriangle_cTDFs;

	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	] = RasterTriangle_Ctdfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleFog)		] = RasterTriangle_CtdFs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleDepth)	] = RasterTriangle_CtDfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		] = RasterTriangle_CtDFs;

	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	] = RasterTriangle_CTdfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleFog)		] = RasterTriangle_CTdFs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleDepth)	] = RasterTriangle_CTDfs;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		] = RasterTriangle_CTDFs;

	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_cTdfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_cTdFS;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_cTDfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleTexture)  |
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_cTDFS;

	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CtdfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CtdFS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CtDfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CtDFS;

	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CTdfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CTdFS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CTDfS;
	m_RasterTriangleFunctions[(1 << RasterTriangleColor)	|
							  (1 << RasterTriangleTexture)	|
							  (1 << RasterTriangleDepth)	|
							  (1 << RasterTriangleFog)		|
							  (1 << RasterTriangleScissor)	] = RasterTriangle_CTDFS;
}


Rasterizer :: ~Rasterizer() {
	if (m_FunctionCache) {
		delete m_FunctionCache;
	}
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


void Rasterizer :: PrepareTexture() {
	if (m_State) {
		for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
			if (m_State->m_Texture[unit].Enabled, m_Texture[unit]) {
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


#if !EGL_USE_JIT

inline void Rasterizer :: Fragment(I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv,
								   EGL_Fixed fogDensity, const Color& baseColor) {

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

	m_RasterInfo.Init(m_Surface, y);

	Fragment(&m_RasterInfo, x, depth, tu, tv, baseColor, fogDensity);
}


Color Rasterizer :: GetRawTexColor(const Texture * texture, EGL_Fixed tu, EGL_Fixed tv) {
	// retrieve the texture color from a texture plane

	EGL_Fixed tu0;
	EGL_Fixed tv0;

	// for nearest texel

	switch (m_Texture[TODO]->GetWrappingModeS()) {
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

	switch (m_Texture[TODO]->GetWrappingModeT()) {
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

	switch (m_Texture[TODO]->GetInternalFormat()) {
		default:
		case RasterizerState::TextureFormatAlpha:
			return Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);

		case RasterizerState::TextureFormatLuminance:
			{
			U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
			return Color (luminance, luminance, luminance, 0xff);
			}

		case RasterizerState::TextureFormatRGB565:
			return Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);

		case RasterizerState::TextureFormatRGB8:
			{
			texOffset = (texOffset << 1) + texOffset;
			const U8 * ptr = reinterpret_cast<const U8 *>(data) + texOffset;
			return Color(ptr[0], ptr[1], ptr[2], 0xff);
			}

		case RasterizerState::TextureFormatLuminanceAlpha:
			{
			U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
			U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
			return Color (luminance, luminance, luminance, alpha);
			}

		case RasterizerState::TextureFormatRGBA8:
			//texColor = Color::FromRGBA(reinterpret_cast<const U32 *>(data)[texOffset]);
			{
			texOffset = texOffset << 2;
			const U8 * ptr = reinterpret_cast<const U8 *>(data) + texOffset;
			return Color(ptr[0], ptr[1], ptr[2], ptr[3]);
			}

		case RasterizerState::TextureFormatRGBA4444:
			return Color::From4444(reinterpret_cast<const U16 *>(data)[texOffset]);

		case RasterizerState::TextureFormatRGBA5551:
			return Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);
	}
}


inline Color Rasterizer :: GetTexColor(const Texture * texture, EGL_Fixed tu, EGL_Fixed tv,
								RasterizerState::FilterMode filterMode) {
	// retrieve the texture color from a texture plane

	if (filterMode == RasterizerState::FilterModeNearest) {
		return GetRawTexColor(texture, tu, tv);
	} else if (filterMode == RasterizerState::FilterModeLinear) {
		I32 logWidth = texture->GetLogWidth();
		I32 logHeight = texture->GetLogHeight();

		EGL_Fixed tu0 = tu - (0x8000 >> logWidth);
		EGL_Fixed tu1 = tu + (0x7fff >> logWidth);
		EGL_Fixed tv0 = tv - (0x8000 >> logHeight);
		EGL_Fixed tv1 = tv + (0x7fff >> logHeight);

		U32 alpha = EGL_FractionFromFixed(tu0 << logWidth) >> 8;
		U32 beta = EGL_FractionFromFixed(tv0 << logHeight) >> 8;

		return Color::BlendAlpha(Color::BlendAlpha(GetRawTexColor(texture, tu1, tv1),
												   GetRawTexColor(texture, tu0, tv1), alpha),
								 Color::BlendAlpha(GetRawTexColor(texture, tu1, tv0),
												   GetRawTexColor(texture, tu0, tv0), alpha),
								 beta);
	} else {
		return Color(0, 0, 0, 0xff);
	}
}


void Rasterizer :: Fragment(const RasterInfo * rasterInfo, I32 x, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv,
			  const Color& baseColor, EGL_Fixed fogDensity) {
	// fragment rendering with signature corresponding to function fragment
	// generated by code generator

	bool depthTest;

	// fragment level clipping (for now)
	if (m_State->m_ScissorTest.Enabled) {
		if (x < m_State->m_ScissorTest.X || x - m_State->m_ScissorTest.X >= m_State->m_ScissorTest.Width) {
			return;
		}
	}

	I32 offset = x;

	depth = EGL_CLAMP(depth, 0, 0xffff);
	assert(depth >= 0 && depth <= 0xffff);
	I32 zBufferValue = rasterInfo->DepthBuffer[offset];

	switch (m_State->m_DepthTest.Func) {
		default:
		case RasterizerState::CompFuncNever:	depthTest = false;						break;
		case RasterizerState::CompFuncLess:		depthTest = depth < zBufferValue;		break;
		case RasterizerState::CompFuncEqual:	depthTest = depth == zBufferValue;		break;
		case RasterizerState::CompFuncLEqual:	depthTest = depth <= zBufferValue;		break;
		case RasterizerState::CompFuncGreater:	depthTest = depth > zBufferValue;		break;
		case RasterizerState::CompFuncNotEqual:	depthTest = depth != zBufferValue;		break;
		case RasterizerState::CompFuncGEqual:	depthTest = depth >= zBufferValue;		break;
		case RasterizerState::CompFuncAlways:	depthTest = true;						break;
	}

	if (!m_State->m_Stencil.Enabled && m_State->m_DepthTest.Enabled && !depthTest) {
		return;
	}

	Color color(baseColor);

	// have offset, color, texOffset, texture

	if (m_State->m_Texture[TODO].Enabled) {

		Texture * texture = rasterInfo->Textures[TODO] + rasterInfo->MipmapLevel[TODO];
		Color texColor = GetTexColor(texture, tu, tv, m_State->GetMinFilterMode(TODO));

		switch (m_Texture[TODO]->GetInternalFormat()) {
			default:
			case RasterizerState::TextureFormatAlpha:
				switch (m_State->m_Texture[TODO].Mode) {
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

			case RasterizerState::TextureFormatLuminance:
				switch (m_State->m_Texture[TODO].Mode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[TODO].EnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[TODO].EnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[TODO].EnvColor.b, texColor.b),
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

			case RasterizerState::TextureFormatRGB565:
			case RasterizerState::TextureFormatRGB8:
				switch (m_State->m_Texture[TODO].Mode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[TODO].EnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[TODO].EnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[TODO].EnvColor.b, texColor.b),
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

			case RasterizerState::TextureFormatLuminanceAlpha:
				switch (m_State->m_Texture[TODO].Mode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[TODO].EnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[TODO].EnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[TODO].EnvColor.b, texColor.b),
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

			case RasterizerState::TextureFormatRGBA5551:
			case RasterizerState::TextureFormatRGBA4444:
			case RasterizerState::TextureFormatRGBA8:
				switch (m_State->m_Texture[TODO].Mode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_Texture[TODO].EnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_Texture[TODO].EnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_Texture[TODO].EnvColor.b, texColor.b),
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

	// fog
	if (m_State->m_Fog.Enabled) {
		color = Color::Blend(color, m_State->m_Fog.Color, fogDensity >> 8);
	}

	// have color

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

	// have offset, depth

	if (m_State->m_Stencil.Enabled) {

		bool stencilTest;
		U32 stencilRef = m_State->m_Stencil.Reference & m_State->m_Stencil.ComparisonMask;
		U32 stencilValue = rasterInfo->StencilBuffer[offset];
		U32 stencil = stencilValue & m_State->m_Stencil.ComparisonMask;

		switch (m_State->m_Stencil.Func) {
			default:
			case RasterizerState::CompFuncNever:	stencilTest = false;				break;
			case RasterizerState::CompFuncLess:		stencilTest = stencilRef < stencil;	break;
			case RasterizerState::CompFuncEqual:	stencilTest = stencilRef == stencil;break;
			case RasterizerState::CompFuncLEqual:	stencilTest = stencilRef <= stencil;break;
			case RasterizerState::CompFuncGreater:	stencilTest = stencilRef > stencil;	break;
			case RasterizerState::CompFuncNotEqual:	stencilTest = stencilRef != stencil;break;
			case RasterizerState::CompFuncGEqual:	stencilTest = stencilRef >= stencil;break;
			case RasterizerState::CompFuncAlways:	stencilTest = true;					break;
		}

		if (!stencilTest) {

			switch (m_State->m_Stencil.Fail) {
				default:
				case RasterizerState::StencilOpKeep:
					break;

				case RasterizerState::StencilOpZero:
					stencilValue = 0;
					break;

				case RasterizerState::StencilOpReplace:
					stencilValue = m_State->m_Stencil.Reference;
					break;

				case RasterizerState::StencilOpIncr:
					if (stencilValue != 0xffffffff) {
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

			rasterInfo->StencilBuffer[offset] = 
				rasterInfo->StencilBuffer[offset] & m_State->m_Stencil.Mask |
				stencilValue & m_State->m_Stencil.Mask;

			return;
		}

		if (depthTest) {
			switch (m_State->m_Stencil.ZPass) {
				default:
				case RasterizerState::StencilOpKeep:
					break;

				case RasterizerState::StencilOpZero:
					stencilValue = 0;
					break;

				case RasterizerState::StencilOpReplace:
					stencilValue = m_State->m_Stencil.Reference;
					break;

				case RasterizerState::StencilOpIncr:
					if (stencilValue != 0xffffffff) {
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

			rasterInfo->StencilBuffer[offset] = 
				rasterInfo->StencilBuffer[offset] & m_State->m_Stencil.Mask |
				stencilValue & m_State->m_Stencil.Mask;
		} else {
			switch (m_State->m_Stencil.ZFail) {
				default:
				case RasterizerState::StencilOpKeep:
					break;

				case RasterizerState::StencilOpZero:
					stencilValue = 0;
					break;

				case RasterizerState::StencilOpReplace:
					stencilValue = m_State->m_Stencil.Reference;
					break;

				case RasterizerState::StencilOpIncr:
					if (stencilValue != 0xffffffff) {
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

			rasterInfo->StencilBuffer[offset] = 
				rasterInfo->StencilBuffer[offset] & m_State->m_Stencil.Mask |
				stencilValue & m_State->m_Stencil.Mask;
		}
	}

	if (m_State->m_Stencil.Enabled && !depthTest && m_State->m_DepthTest.Enabled) {
		// otherwise we returned at the top
		return;
	}

	// have color, offset

	// Blending
	if (m_State->m_Blend.Enabled) {

		U16 dstValue = rasterInfo->ColorBuffer[offset];
		U8 dstAlpha = rasterInfo->AlphaBuffer[offset];

		Color dstColor = Color::From565A(dstValue, dstAlpha);

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
				srcCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				break;

			case RasterizerState::BlendFuncSrcOneMinusDstAlpha:
				srcCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				break;

			case RasterizerState::BlendFuncSrcSrcAlphaSaturate:
				{
					U8 rev = Color::MAX - dstAlpha;
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
				dstCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				dstCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				break;

		}

		color = srcCoeff * color + dstCoeff * dstColor;
	}

	// have offset, depth, color

	// Masking and write to framebuffer
	if (m_State->m_Mask.Depth) {
		rasterInfo->DepthBuffer[offset] = depth;
	}

	Color maskedColor =
		color.Mask(m_State->m_Mask.Red, m_State->m_Mask.Green, m_State->m_Mask.Blue, m_State->m_Mask.Alpha);

	if (m_State->m_LogicOp.Enabled) {

		U16 oldValue = rasterInfo->ColorBuffer[offset];
		U8 oldAlpha = rasterInfo->AlphaBuffer[offset];

		U16 newValue = maskedColor.ConvertTo565();
		U8 newAlpha = maskedColor.A();

		U16 value;
		U8 alpha;

		switch (m_State->m_LogicOp.Opcode) {
			default:
			case RasterizerState:: LogicOpClear:
				value = 0;
				alpha = 0;
				break;

			case RasterizerState:: LogicOpAnd:
				value = newValue & oldValue;
				alpha = newAlpha & oldAlpha;
				break;

			case RasterizerState:: LogicOpAndReverse:
				value = newValue & ~oldValue;
				alpha = newAlpha & ~oldAlpha;
				break;

			case RasterizerState:: LogicOpCopy:
				value = newValue;
				alpha = newAlpha;
				break;

			case RasterizerState:: LogicOpAndInverted:
				value = ~newValue & oldValue;
				alpha = ~newAlpha & oldAlpha;
				break;

			case RasterizerState:: LogicOpNoop:
				value = oldValue;
				alpha = oldAlpha;
				break;

			case RasterizerState:: LogicOpXor:
				value = newValue ^ oldValue;
				alpha = newAlpha ^ oldAlpha;
				break;

			case RasterizerState:: LogicOpOr:
				value = newValue | oldValue;
				alpha = newAlpha | oldAlpha;
				break;

			case RasterizerState:: LogicOpNor:
				value = ~(newValue | oldValue);
				alpha = ~(newAlpha | oldAlpha);
				break;

			case RasterizerState:: LogicOpEquiv:
				value = ~(newValue ^ oldValue);
				alpha = ~(newAlpha ^ oldAlpha);
				break;

			case RasterizerState:: LogicOpInvert:
				value = ~oldValue;
				alpha = ~oldAlpha;
				break;

			case RasterizerState:: LogicOpOrReverse:
				value = newValue | ~oldValue;
				alpha = newAlpha | ~oldAlpha;
				break;

			case RasterizerState:: LogicOpCopyInverted:
				value = ~newValue;
				alpha = ~newAlpha;
				break;

			case RasterizerState:: LogicOpOrInverted:
				value = ~newValue | oldValue;
				alpha = ~newAlpha | oldAlpha;
				break;

			case RasterizerState:: LogicOpNand:
				value = ~(newValue & oldValue);
				alpha = ~(newAlpha & oldAlpha);
				break;

			case RasterizerState:: LogicOpSet:
				value = 0xFFFF;
				alpha = 0xFF;
				break;
		}

		rasterInfo->ColorBuffer[offset] = value;
		rasterInfo->AlphaBuffer[offset] = alpha;

	} else {
		rasterInfo->ColorBuffer[offset] = maskedColor.ConvertTo565();

		if (m_State->m_Mask.Alpha) {
			rasterInfo->AlphaBuffer[offset] = maskedColor.A();
		}
	}
}

#endif // !EGL_USE_JIT

// --------------------------------------------------------------------------
// Prepare rasterizer with according to current state settings
// --------------------------------------------------------------------------

void Rasterizer :: PreparePoint() {

	PrepareTexture();

	m_PointFunction = (PointFunction *)
		m_FunctionCache->GetFunction(FunctionCache::FunctionTypePoint,
									 *m_State);

	m_RasterInfo.Init(m_Surface, 0);
	memset(m_RasterInfo.MipmapLevel, 0, sizeof(m_RasterInfo.MipmapLevel));
}


void Rasterizer :: PrepareLine() {
	PrepareTexture();

	m_LineFunction = (LineFunction *)
		m_FunctionCache->GetFunction(FunctionCache::FunctionTypeLine,
									 *m_State);

	m_RasterInfo.Init(m_Surface, 0);
	memset(m_RasterInfo.MipmapLevel, 0, sizeof(m_RasterInfo.MipmapLevel));
}


void Rasterizer :: Finish() {
}


#if !EGL_USE_JIT

void Rasterizer :: RasterLine(const RasterPos& p_from, const RasterPos& p_to) {

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

		const RasterPos *start, *end;

		I32 x;
		I32 endX;
		EGL_Fixed roundedX; //, preStepX;

		if (deltaX < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			roundedX = EGL_NearestInt(p_to.m_WindowCoords.x + 1);
			x = EGL_IntFromFixed(roundedX);
			//preStepX = roundedX + (EGL_ONE/2) - p_to.m_WindowCoords.x;
			endX = EGL_IntFromFixed(p_from.m_WindowCoords.x + ((EGL_ONE)/2));
		} else {
			start = &p_from;
			end = &p_to;
			roundedX = EGL_NearestInt(p_from.m_WindowCoords.x);
			x = EGL_IntFromFixed(roundedX);
			//preStepX = roundedX + (EGL_ONE/2) - p_from.m_WindowCoords.x;
			endX = EGL_IntFromFixed(p_to.m_WindowCoords.x + ((EGL_ONE)/2-1));
		}

		const RasterPos& from = *start;
		const RasterPos& to = *end;

		FractionalColor baseColor = from.m_Color;
		EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
		EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords[TODO].tu, OneOverZ);
		EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords[TODO].tv, OneOverZ);
		EGL_Fixed fogDensity = from.m_FogDensity;
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
		EGL_Fixed invSpan = EGL_Inverse(deltaX);
		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaY), invSpan);

		// -- increments(to, from, invSpan)
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords[TODO].tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords[TODO].tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords[TODO].tv, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords[TODO].tv, OneOverZ), invSpan);

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);
		// -- end increments

#if 0
		baseColor.r += EGL_Mul(colorIncrement.r, preStepX);
		baseColor.g += EGL_Mul(colorIncrement.g, preStepX);
		baseColor.b += EGL_Mul(colorIncrement.b, preStepX);
		baseColor.a += EGL_Mul(colorIncrement.a, preStepX);

		fogDensity  += EGL_Mul(deltaFog, preStepX);
		depth       += EGL_Mul(deltaDepth, preStepX);

		OneOverZ	+= EGL_Mul(deltaZ, preStepX);
		tuOverZ		+= EGL_Mul(deltaU, preStepX);
		tvOverZ		+= EGL_Mul(deltaV, preStepX);
#endif

		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y + ((EGL_ONE)/2-1));

		I32 yIncrement = (deltaY > 0) ? 1 : -1;
		EGL_Fixed error = 0;//EGL_ONE/2;

		for (; x < endX; ++x) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu = EGL_Mul(tuOverZ, z);
			EGL_Fixed tv = EGL_Mul(tvOverZ, z);

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

			error += slope;
			if (error > EGL_ONE) {
				y += yIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			depth += deltaDepth;
			OneOverZ += deltaZ;
			tuOverZ += deltaU;
			tvOverZ += deltaV;
			fogDensity += deltaFog;
		}

	} else {
		// Bresenham along y-axis

		const RasterPos *start, *end;

		I32 y;
		I32 endY;
		EGL_Fixed roundedY; //, preStepY;

		if (deltaY < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			roundedY = EGL_NearestInt(p_to.m_WindowCoords.y + 1);
			y = EGL_IntFromFixed(roundedY);
			//preStepY = roundedY + (EGL_ONE/2) - p_to.m_WindowCoords.y;
			endY = EGL_IntFromFixed(p_from.m_WindowCoords.y + ((EGL_ONE)/2));
		} else {
			start = &p_from;
			end = &p_to;
			roundedY = EGL_NearestInt(p_from.m_WindowCoords.y);
			y = EGL_IntFromFixed(roundedY);
			//preStepY = roundedY + (EGL_ONE/2) - p_from.m_WindowCoords.y;
			endY = EGL_IntFromFixed(p_to.m_WindowCoords.y + ((EGL_ONE)/2-1));
		}

		const RasterPos& from = *start;
		const RasterPos& to = *end;

		FractionalColor baseColor = from.m_Color;
		EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
		EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords[TODO].tu, OneOverZ);
		EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords[TODO].tv, OneOverZ);
		EGL_Fixed fogDensity = from.m_FogDensity;
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
		EGL_Fixed invSpan = EGL_Inverse(deltaY);
		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaX), invSpan);

		// -- increments(to, from, invSpan)
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords[TODO].tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords[TODO].tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords[TODO].tv, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords[TODO].tv, OneOverZ), invSpan);

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
		tuOverZ		+= EGL_Mul(deltaU, preStepY);
		tvOverZ		+= EGL_Mul(deltaV, preStepY);
#endif

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x + ((EGL_ONE)/2-1));

		I32 xIncrement = (deltaX > 0) ? 1 : -1;
		EGL_Fixed error = 0;//EGL_ONE/2;

		// can have xIncrement; yIncrement; xBaseIncrement, yBaseIncrement
		// then both x/y loops become the same
		// question: how to add correct mipmap selection?

		for (; y < endY; ++y) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu = EGL_Mul(tuOverZ, z);
			EGL_Fixed tv = EGL_Mul(tvOverZ, z);

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

			error += slope;
			if (error > EGL_ONE) {
				x += xIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			depth += deltaDepth;
			OneOverZ += deltaZ;
			tuOverZ += deltaU;
			tvOverZ += deltaV;
			fogDensity += deltaFog;
		}
	}
}

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
}

void Rasterizer :: RasterPoint(const RasterPos& point, EGL_Fixed size) {

	EGL_Fixed halfSize = size / 2;

	I32 xmin = EGL_IntFromFixed(point.m_WindowCoords.x - halfSize + EGL_HALF);
	I32 xmax = xmin + ((size - EGL_HALF) >> EGL_PRECISION);
	I32 ymin = EGL_IntFromFixed(point.m_WindowCoords.y - halfSize + EGL_HALF);
	I32 ymax = ymin + ((size - EGL_HALF) >> EGL_PRECISION);

	EGL_Fixed depth = point.m_WindowCoords.depth;
	FractionalColor baseColor = point.m_Color;
	EGL_Fixed fogDensity = point.m_FogDensity;


	if (!m_State->m_Point.SpriteEnabled && !m_State->m_Point.CoordReplaceEnabled) {
		EGL_Fixed tu = point.m_TextureCoords[TODO].tu;
		EGL_Fixed tv = point.m_TextureCoords[TODO].tv;

		for (I32 y = ymin; y <= ymax; y++) {
			for (I32 x = xmin; x <= xmax; x++) {
				Fragment(x, y, depth, tu, tv, fogDensity, baseColor);
			}
		}
	} else {
		EGL_Fixed delta = EGL_Inverse(size);

		if (m_UseMipmap) {
			EGL_Fixed maxDu = delta >> (16 - m_Texture[TODO]->GetTexture(0)->GetLogWidth());
			EGL_Fixed maxDv = delta >> (16 - m_Texture[TODO]->GetTexture(0)->GetLogHeight());

			EGL_Fixed rho = maxDu + maxDv;

			// we start with nearest/minification only selection; will add LINEAR later

			m_RasterInfo.MipmapLevel[TODO] = EGL_Min(Log2(rho), m_RasterInfo.MaxMipmapLevel[TODO]);
		}

		for (I32 y = ymin, tv = delta / 2; y <= ymax; y++, tv += delta) {
			for (I32 x = xmin, tu = delta / 2; x <= xmax; x++, tu += delta) {
				Fragment(x, y, depth, tu, tv, fogDensity, baseColor);
			}
		}
	}
}

#endif // !EGL_USE_JIT


const I32 RasterInfo::InversionTable[32] = {
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