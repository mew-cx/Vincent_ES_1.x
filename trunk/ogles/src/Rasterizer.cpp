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
		// TODO: use lookup table and treat ranges correctly
		return (a * b) >> 8; 
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
}


Rasterizer :: ~Rasterizer() {
	if (m_FunctionCache) {
		delete m_FunctionCache;
	}
}


void Rasterizer :: SetState(RasterizerState * state) {
	m_State = state;
	m_Texture = 0;
}


RasterizerState * Rasterizer :: GetState() const {
	return m_State;
}


void Rasterizer :: SetTexture(MultiTexture * texture) {
	m_Texture = texture; 

	if (m_Texture && m_State) {
		m_State->SetWrappingModeS(m_Texture->GetWrappingModeS());
		m_State->SetWrappingModeT(m_Texture->GetWrappingModeT());
		m_State->SetMinFilterMode(m_Texture->GetMinFilterMode());
		m_State->SetMagFilterMode(m_Texture->GetMagFilterMode());
		m_State->SetInternalFormat(m_Texture->GetInternalFormat());
	}
}


inline void Rasterizer :: Fragment(I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, 
								   EGL_Fixed fogDensity, const Color& baseColor) {

	// pixel ownership test
	if (x < 0 || x >= m_Surface->GetWidth() ||
		y < 0 || y >= m_Surface->GetHeight()) {
		return;
	}

	// fragment level clipping (for now)
	if (m_State->m_ScissorTestEnabled) {
		if (x < m_State->m_ScissorX || x - m_State->m_ScissorX >= m_State->m_ScissorWidth ||
			y < m_State->m_ScissorY || y - m_State->m_ScissorY >= m_State->m_ScissorHeight) {
			return;
		}
	}

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

	if (m_State->m_TextureEnabled) {

		EGL_Fixed tu0;
		EGL_Fixed tv0;

		// for nearest texel
		tu += ((EGL_ONE/2) >> rasterInfo.TextureLogWidth) - 1;
		tv += ((EGL_ONE/2) >> rasterInfo.TextureLogHeight) - 1; 

		switch (m_Texture->GetWrappingModeS()) {
			case RasterizerState::WrappingModeClampToEdge:
				tu0 = EGL_CLAMP(tu, 0, EGL_ONE);
				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				tu0 = tu & 0xffff;
				break;
		}

		switch (m_Texture->GetWrappingModeT()) {
			case RasterizerState::WrappingModeClampToEdge:
				tv0 = EGL_CLAMP(tv, 0, EGL_ONE);
				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				tv0 = tv & 0xffff;
				break;
		}


		// get the pixel color
		Texture * texture = m_Texture->GetTexture(m_MipMapLevel);

		I32 texX = EGL_IntFromFixed(texture->GetWidth() * tu0);		// can become a shift
		I32 texY = EGL_IntFromFixed(texture->GetHeight() * tv0);	// can become a shift

		// do wrapping mode here
		I32 texOffset = texX + (texY << texture->GetLogWidth());

		Fragment(&rasterInfo, x, depth, texOffset, baseColor.ConvertToRGBA(), fogDensity);
	} else {
		Fragment(&rasterInfo, x, depth, 0, baseColor.ConvertToRGBA(), fogDensity);
	}
}


void Rasterizer :: Fragment(const RasterInfo * rasterInfo, I32 x, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv,
			  const Color& baseColor, EGL_Fixed fogDensity) {
	// fragment rendering with signature corresponding to function fragment
	// generated by code generator

	// fragment level clipping (for now)
	if (m_State->m_ScissorTestEnabled) {
		if (x < m_State->m_ScissorX || x - m_State->m_ScissorX >= m_State->m_ScissorWidth) {
			return;
		}
	}

	I32 offset = x;

	if (m_State->m_TextureEnabled) {

		EGL_Fixed tu0;
		EGL_Fixed tv0;

		// for nearest texel
//		tu += ((EGL_ONE/2) >> rasterInfo->TextureLogWidth) - 1;
//		tv += ((EGL_ONE/2) >> rasterInfo->TextureLogHeight) - 1;

		switch (m_Texture->GetWrappingModeS()) {
			case RasterizerState::WrappingModeClampToEdge:
				if (tu < 0)
					tu0 = 0;
				else if (tu >= EGL_ONE)
					tu0 = EGL_ONE - 1;

				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				tu0 = tu & 0xffff;
				break;
		}

		switch (m_Texture->GetWrappingModeT()) {
			case RasterizerState::WrappingModeClampToEdge:
				if (tv < 0)
					tv0 = 0;
				else if (tv >= EGL_ONE)
					tv0 = EGL_ONE - 1;

				break;

			default:
			case RasterizerState::WrappingModeRepeat:
				tv0 = tv & 0xffff;
				break;
		}


		// get the pixel color
		Texture * texture = m_Texture->GetTexture(m_MipMapLevel);

		I32 texX = EGL_IntFromFixed(texture->GetWidth() * tu0);		// can become a shift
		I32 texY = EGL_IntFromFixed(texture->GetHeight() * tv0);	// can become a shift

		// do wrapping mode here
		I32 texOffset = texX + (texY << texture->GetLogWidth());

		Fragment(rasterInfo, offset, depth, texOffset, baseColor.ConvertToRGBA(), fogDensity);
	} else {
		Fragment(rasterInfo, offset, depth, 0, baseColor.ConvertToRGBA(), fogDensity);
	}
}


inline void Rasterizer :: Fragment(const RasterInfo * rasterInfo,
								   I32 offset, EGL_Fixed depth, I32 texOffset, 
								   U32 baseColor, EGL_Fixed fogDensity) {
	bool depthTest;
	
	I32 zBufferValue = rasterInfo->DepthBuffer[offset];

	switch (m_State->m_DepthFunc) {
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

	if (!m_State->m_StencilTestEnabled && m_State->m_DepthTestEnabled && !depthTest) {
		return;
	}

	Color color(baseColor);

	// have offset, color, texOffset, texture

	if (m_State->m_TextureEnabled) {

		Texture * texture = m_Texture->GetTexture(m_MipMapLevel);
		void * data = texture->GetData();
		Color texColor;

		switch (m_Texture->GetInternalFormat()) {
			default:
			case RasterizerState::TextureFormatAlpha:
				texColor = Color(0xff, 0xff, 0xff, reinterpret_cast<const U8 *>(data)[texOffset]);

				switch (m_State->m_TextureMode) {
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
				{
				U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset];
				texColor = Color (luminance, luminance, luminance, 0xff);
				}

				switch (m_State->m_TextureMode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
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

			case RasterizerState::TextureFormatRGB:
				texColor = Color::From565(reinterpret_cast<const U16 *>(data)[texOffset]);

				switch (m_State->m_TextureMode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
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
				{
				U8 luminance = reinterpret_cast<const U8 *>(data)[texOffset * 2];
				U8 alpha = reinterpret_cast<const U8 *>(data)[texOffset * 2 + 1];
				texColor = Color (luminance, luminance, luminance, alpha);
				}

				switch (m_State->m_TextureMode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
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

			case RasterizerState::TextureFormatRGBA:
				texColor = Color::From5551(reinterpret_cast<const U16 *>(data)[texOffset]);

				switch (m_State->m_TextureMode) {
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
								MulU8(color.r, 0xff - texColor.r) + MulU8(m_State->m_TexEnvColor.r, texColor.r),
								MulU8(color.g, 0xff - texColor.g) + MulU8(m_State->m_TexEnvColor.g, texColor.g),
								MulU8(color.b, 0xff - texColor.b) + MulU8(m_State->m_TexEnvColor.b, texColor.b),
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
	if (m_State->m_FogEnabled) {
		color = Color::Blend(color, m_State->m_FogColor, fogDensity >> 8);
	}

	// have color

	if (m_State->m_AlphaTestEnabled) {
		bool alphaTest;
		U8 alpha = color.A();
		U8 alphaRef = EGL_IntFromFixed(m_State->m_AlphaReference * 255);

		switch (m_State->m_AlphaFunc) {
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

	if (m_State->m_StencilTestEnabled) {

		bool stencilTest;
		U32 stencilRef = m_State->m_StencilReference & m_State->m_StencilMask;
		U32 stencilValue = rasterInfo->StencilBuffer[offset];
		U32 stencil = stencilValue & m_State->m_StencilMask;

		switch (m_State->m_StencilFunc) {
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

			switch (m_State->m_StencilFail) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					stencilValue = m_State->m_StencilReference; 
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

			rasterInfo->StencilBuffer[offset] = stencilValue;
			return;
		}

		if (depthTest) {
			switch (m_State->m_StencilZPass) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					stencilValue = m_State->m_StencilReference; 
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

			rasterInfo->StencilBuffer[offset] = stencilValue;
		} else {
			switch (m_State->m_StencilZFail) {
				default:
				case RasterizerState::StencilOpKeep: 
					break;

				case RasterizerState::StencilOpZero: 
					stencilValue = 0; 
					break;

				case RasterizerState::StencilOpReplace: 
					stencilValue = m_State->m_StencilReference; 
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

			rasterInfo->StencilBuffer[offset] = stencilValue;
		}
	}

	if (m_State->m_StencilTestEnabled && !depthTest && m_State->m_DepthTestEnabled) {
		// otherwise we returned at the top
		return;
	}

	// have color, offset 

	// Blending
	if (m_State->m_BlendingEnabled) {

		U16 dstValue = rasterInfo->ColorBuffer[offset];
		U8 dstAlpha = rasterInfo->AlphaBuffer[offset];

		Color dstColor = Color::From565A(dstValue, dstAlpha);

		Color srcCoeff, dstCoeff;

		switch (m_State->m_BlendFuncSrc) {
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

		switch (m_State->m_BlendFuncDst) {
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
	if (m_State->m_MaskDepth) {
		rasterInfo->DepthBuffer[offset] = depth;
	}

	Color maskedColor = 
		color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);

	if (m_State->m_LogicOpEnabled) {

		U16 oldValue = rasterInfo->ColorBuffer[offset];
		U8 oldAlpha = rasterInfo->AlphaBuffer[offset];

		U16 newValue = maskedColor.ConvertTo565();
		U8 newAlpha = maskedColor.A();

		U16 value;
		U8 alpha;

		switch (m_State->m_LogicOpcode) {
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

		if (m_State->m_MaskAlpha) {
			rasterInfo->AlphaBuffer[offset] = maskedColor.A();
		}
	}
}


// --------------------------------------------------------------------------
// Prepare rasterizer with according to current state settings
// --------------------------------------------------------------------------

void Rasterizer :: PreparePoint() {

	if (m_State->m_TextureEnabled) {
		SetTexture(m_Texture);
	}

	m_MipMapLevel = 0;
}


void Rasterizer :: PrepareLine() {
	if (m_State->m_TextureEnabled) {
		SetTexture(m_Texture);
	}

	m_MipMapLevel = 0;
}


void Rasterizer :: Finish() {
}


void Rasterizer :: RasterPoint(const RasterPos& point) {

	I32 x = EGL_IntFromFixed(point.m_WindowCoords.x);
	I32 y = EGL_IntFromFixed(point.m_WindowCoords.y);
	EGL_Fixed depth = point.m_WindowCoords.depth;
	EGL_Fixed tu = point.m_TextureCoords.tu;
	EGL_Fixed tv = point.m_TextureCoords.tv;
	FractionalColor baseColor = point.m_Color;
	EGL_Fixed fogDensity = point.m_FogDensity;

	Fragment(x, y, depth, tu, tv, fogDensity, baseColor);
}


void Rasterizer :: RasterLine(const RasterPos& p_from, const RasterPos& p_to) {

	if (EGL_Round(p_from.m_WindowCoords.x) == EGL_Round(p_to.m_WindowCoords.x) &&
		EGL_Round(p_from.m_WindowCoords.y) == EGL_Round(p_to.m_WindowCoords.y)) {
		// both ends of line on same pixel
		RasterPoint(p_from);
		return;
	}

	EGL_Fixed deltaX = p_to.m_WindowCoords.x - p_from.m_WindowCoords.x;
	EGL_Fixed deltaY = p_to.m_WindowCoords.y - p_from.m_WindowCoords.y;

	if (EGL_Abs(deltaX) > EGL_Abs(deltaY)) {
		// Bresenheim along x-axis

		const RasterPos *start, *end;

		I32 x;
		I32 endX;

		if (deltaX < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			x = EGL_IntFromFixed(p_to.m_WindowCoords.x + ((EGL_ONE)/2));
			endX = EGL_IntFromFixed(p_from.m_WindowCoords.x + ((EGL_ONE)/2));
		} else {
			start = &p_from;
			end = &p_to;
			x = EGL_IntFromFixed(p_from.m_WindowCoords.x + ((EGL_ONE)/2-1));
			endX = EGL_IntFromFixed(p_to.m_WindowCoords.x + ((EGL_ONE)/2-1));
		}

		const RasterPos& from = *start;
		const RasterPos& to = *end;

		FractionalColor baseColor = from.m_Color;
		EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
		EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
		EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords.tu, OneOverZ);
		EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords.tv, OneOverZ);
		EGL_Fixed fogDensity = from.m_FogDensity;
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed invSpan = EGL_Inverse(deltaX);

		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaY), invSpan);
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) - 
								   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);

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
		// Bresenheim along y-axis

		const RasterPos *start, *end;

		I32 y;
		I32 endY;

		if (deltaY < 0) {
			deltaY = -deltaY;
			deltaX = -deltaX;
			start = &p_to;
			end = &p_from;
			y = EGL_IntFromFixed(p_to.m_WindowCoords.y + ((EGL_ONE)/2));
			endY = EGL_IntFromFixed(p_from.m_WindowCoords.y + ((EGL_ONE)/2));
		} else {
			start = &p_from;
			end = &p_to;
			y = EGL_IntFromFixed(p_from.m_WindowCoords.y + ((EGL_ONE)/2-1));
			endY = EGL_IntFromFixed(p_to.m_WindowCoords.y + ((EGL_ONE)/2-1));
		}

		const RasterPos& from = *start;
		const RasterPos& to = *end;

		FractionalColor baseColor = from.m_Color;
		EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
		EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
		EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords.tu, OneOverZ);
		EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords.tv, OneOverZ);
		EGL_Fixed fogDensity = from.m_FogDensity;
		EGL_Fixed depth = from.m_WindowCoords.depth;

		EGL_Fixed invSpan = EGL_Inverse(deltaY);

		EGL_Fixed slope = EGL_Mul(EGL_Abs(deltaX), invSpan);
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) - 
								   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x + ((EGL_ONE)/2-1));

		I32 xIncrement = (deltaX > 0) ? 1 : -1;
		EGL_Fixed error = 0;//EGL_ONE/2;

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


I32 RasterInfo::InversionTable[] = {
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