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

	// fragment level clipping (for now)
	if (m_State->m_ScissorTestEnabled) {
		if (x < m_State->m_ScissorX || x - m_State->m_ScissorX >= m_State->m_ScissorWidth ||
			y < m_State->m_ScissorY || y - m_State->m_ScissorY >= m_State->m_ScissorHeight) {
			return;
		}
	}

	I32 offset = x + y * m_Surface->GetWidth();

	if (m_State->m_TextureEnabled) {

		EGL_Fixed tu0;
		EGL_Fixed tv0;

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
		I32 texOffset = texX + (texY << texture->GetExponent());

		Fragment(offset, depth, texOffset, baseColor.ConvertToRGBA(), fogDensity);
	} else {
		Fragment(offset, depth, 0, baseColor.ConvertToRGBA(), fogDensity);
	}
}

inline void Rasterizer :: Fragment(I32 offset, EGL_Fixed depth, I32 texOffset, 
								   U32 baseColor, EGL_Fixed fogDensity) {
	bool depthTest;
	
	I32 zBufferValue = m_Surface->GetDepthBuffer()[offset];

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
		color = Color::Blend(color, m_State->m_FogColor, fogDensity);
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
		U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
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

			m_Surface->GetStencilBuffer()[offset] = stencilValue;
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

			m_Surface->GetStencilBuffer()[offset] = stencilValue;
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

			m_Surface->GetStencilBuffer()[offset] = stencilValue;
		}
	}

	if (m_State->m_StencilTestEnabled && !depthTest && m_State->m_DepthTestEnabled) {
		// otherwise we returned at the top
		return;
	}

	// have color, offset 

	// Blending
	if (m_State->m_BlendingEnabled) {

		U16 dstValue = m_Surface->GetColorBuffer()[offset];
		U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];

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

			case RasterizerState::BlendFuncSrcSrcColor:
				srcCoeff = color;
				break;

			case RasterizerState::BlendFuncSrcOneMinusSrcColor:
				srcCoeff = Color(Color::MAX - color.R(), Color::MAX - color.G(), Color::MAX - color.B(), Color::MAX - color.A());
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

			case RasterizerState::BlendFuncDstSrcOneMinusSrcAlpha:
				dstCoeff = Color(Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A(), Color::MAX - color.A());
				break;

			case RasterizerState::BlendFuncDstDstAlpha:
				dstCoeff = Color(dstAlpha, dstAlpha, dstAlpha, dstAlpha);
				break;

			case RasterizerState::BlendFuncDstOneMinusDstAlpha:
				dstCoeff = Color(Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha, Color::MAX - dstAlpha);
				break;

			case RasterizerState::BlendFuncDstSrcAlphaSaturate:
				{
					U8 rev = Color::MAX - dstAlpha;
					U8 f = (rev < color.A() ? rev : color.A());
					dstCoeff = Color(f, f, f, Color::MAX);
				}
				break;
		}

		color = srcCoeff * color + dstCoeff * dstColor;
	}

	// have offset, depth, color

	// Masking and write to framebuffer
	if (m_State->m_MaskDepth) {
		m_Surface->GetDepthBuffer()[offset] = depth;
	}

	Color maskedColor = 
		color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);

	if (m_State->m_LogicOpEnabled) {

		U16 dstValue = m_Surface->GetColorBuffer()[offset];
		U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];

		U32 newValue = maskedColor.ConvertToRGBA();
		U32 oldValue = Color::From565A(dstValue, dstAlpha).ConvertToRGBA();
		U32 value;

		switch (m_State->m_LogicOpcode) {
			default:
			case RasterizerState:: LogicOpClear:		value = 0;						break;
			case RasterizerState:: LogicOpAnd:			value = newValue & oldValue;	break;
			case RasterizerState:: LogicOpAndReverse:	value = newValue & ~oldValue;	break;
			case RasterizerState:: LogicOpCopy:			value = newValue;				break;
			case RasterizerState:: LogicOpAndInverted:	value = ~newValue & oldValue;	break;
			case RasterizerState:: LogicOpNoop:			value = oldValue;				break;
			case RasterizerState:: LogicOpXor:			value = newValue ^ oldValue;	break;
			case RasterizerState:: LogicOpOr:			value = newValue | oldValue;	break;
			case RasterizerState:: LogicOpNor:			value = ~(newValue | oldValue); break;
			case RasterizerState:: LogicOpEquiv:		value = ~(newValue ^ oldValue); break;
			case RasterizerState:: LogicOpInvert:		value = ~oldValue;				break;
			case RasterizerState:: LogicOpOrReverse:	value = newValue | ~oldValue;	break;
			case RasterizerState:: LogicOpCopyInverted:	value = ~newValue;				break;
			case RasterizerState:: LogicOpOrInverted:	value = ~newValue | oldValue;	break;
			case RasterizerState:: LogicOpNand:			value = ~(newValue & oldValue); break;
			case RasterizerState:: LogicOpSet:			value = 0xFFFF;					break;
		}

		maskedColor = Color::FromRGBA(value);
		m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();
		m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();

	} else {
		m_Surface->GetColorBuffer()[offset] = maskedColor.ConvertTo565();

		if (m_State->m_MaskAlpha) {
			m_Surface->GetAlphaBuffer()[offset] = maskedColor.A();
		}
	}
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
		rasterInfo.TextureWidth = texture->GetWidth();
		rasterInfo.TextureHeight = texture->GetHeight();
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


void Rasterizer :: PrepareTriangle() {
	if (m_State->m_TextureEnabled) {
		SetTexture(m_Texture);
	}

	m_ScanlineFunction = m_FunctionCache->GetFunction(*m_State);
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


void Rasterizer :: RasterLine(const RasterPos& from, const RasterPos& to) {

	if (from.m_WindowCoords.x == to.m_WindowCoords.x &&
		from.m_WindowCoords.y == to.m_WindowCoords.y) {
		RasterPoint(from);
		return;
	}

	EGL_Fixed deltaX = to.m_WindowCoords.x - from.m_WindowCoords.x;
	EGL_Fixed deltaY = to.m_WindowCoords.y - from.m_WindowCoords.y;

	FractionalColor baseColor = from.m_Color;
	EGL_Fixed OneOverZ = from.m_WindowCoords.invZ;
	EGL_Fixed OneOverZTo = to.m_WindowCoords.invZ;
	EGL_Fixed tuOverZ = EGL_Mul(from.m_TextureCoords.tu, OneOverZ);
	EGL_Fixed tvOverZ = EGL_Mul(from.m_TextureCoords.tv, OneOverZ);
	EGL_Fixed fogDensity = from.m_FogDensity;
	EGL_Fixed depth = from.m_WindowCoords.depth;

	if (EGL_Abs(deltaX) > EGL_Abs(deltaY)) {
		// Bresenheim along x-axis

		EGL_Fixed invSpan = EGL_Inverse(deltaX);

		EGL_Fixed slope = EGL_Abs(EGL_Mul(deltaY, invSpan));
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) - 
								   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x);
		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y);
		I32 endX = EGL_IntFromFixed(to.m_WindowCoords.x);
		I32 yIncrement = (deltaY > 0) ? 1 : -1;
		EGL_Fixed error = 0;

		for (; x < endX; ++x) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu = EGL_Mul(tuOverZ, z);
			EGL_Fixed tv = EGL_Mul(tvOverZ, z);

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

			error += slope;
			if (error > EGL_FixedFromFloat(0.5f)) {
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

		EGL_Fixed invSpan = EGL_Inverse(deltaY);

		EGL_Fixed slope = EGL_Abs(EGL_Mul(deltaX, invSpan));
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaFog = EGL_Mul(to.m_FogDensity - from.m_FogDensity, invSpan);

		EGL_Fixed deltaZ = EGL_Mul(OneOverZTo - OneOverZ, invSpan);

		EGL_Fixed deltaU = EGL_Mul(EGL_Mul(to.m_TextureCoords.tu, OneOverZTo) -
								   EGL_Mul(from.m_TextureCoords.tu, OneOverZ), invSpan);

		EGL_Fixed deltaV = EGL_Mul(EGL_Mul(to.m_TextureCoords.tv, OneOverZTo) - 
								   EGL_Mul(from.m_TextureCoords.tv, OneOverZ), invSpan);

		EGL_Fixed deltaDepth = EGL_Mul(to.m_WindowCoords.depth - from.m_WindowCoords.depth, invSpan);

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x);
		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y);
		I32 endY = EGL_IntFromFixed(to.m_WindowCoords.y);
		I32 xIncrement = (deltaX > 0) ? 1 : -1;
		EGL_Fixed error = 0;

		for (; y < endY; ++y) {

			EGL_Fixed z = EGL_Inverse(OneOverZ);
			EGL_Fixed tu = EGL_Mul(tuOverZ, z);
			EGL_Fixed tv = EGL_Mul(tvOverZ, z);

			Fragment(x, y, depth, tu, tv, fogDensity, baseColor);

			error += slope;
			if (error > EGL_FixedFromFloat(0.5f)) {
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


