// ==========================================================================
//
// Rasterizer.cpp	Rasterizer Class for OpenGL (R) ES Implementation
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

		return Permutation[
			(((y1 - y0) >> 29) & 4) |
			(((y2 - y0) >> 30) & 2) |
			(((y2 - y1) >> 31) & 1)];
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
	m_IsPrepared(false),
	m_State(state)
{
}


Rasterizer :: ~Rasterizer() {
}


void Rasterizer :: SetState(RasterizerState * state) {
	m_State = state;
}


RasterizerState * Rasterizer :: GetState() const {
	return m_State;
}


void Rasterizer :: InitFogTable() {

	// calculate fog table using floating point
	float start = EGL_FloatFromFixed(m_State->m_FogStart);
	float end = EGL_FloatFromFixed(m_State->m_FogEnd);
	float density = EGL_FloatFromFixed(m_State->m_FogDensity);

	U32 currentIndex = 0;
	U32 upper = (U32) (start * FOG_INTERVAL);

	// clear values before fog starts
	while (currentIndex <= upper) {
		m_FogTable[currentIndex++] = 0x100;
	}

	upper = (U32) (end * FOG_INTERVAL);

	switch (m_State->m_FogMode) {
		default:
		case RasterizerState::FogLinear:
			{
				while (currentIndex <= upper) {
					float distance = currentIndex * (1.0f/FOG_INTERVAL);
					U16 value = (U16) (0x100 * (end - distance) / (end - start));

					m_FogTable[currentIndex++] = value;
				}
			}
			break;

		case RasterizerState::FogModeExp:
			{
				while (currentIndex <= upper) {
					float distance = currentIndex * (1.0f/FOG_INTERVAL);
					U16 value = (U16) (0x100 * exp(-density * distance));

					m_FogTable[currentIndex++] = value;
				}
			}
			break;

		case RasterizerState::FogModeExp2:
			{
				while (currentIndex <= upper) {
					float distance = currentIndex * (1.0f/FOG_INTERVAL);
					float densityDistance = (density * distance);
					U16 value = (U16) (0x100 * exp(-densityDistance * densityDistance));

					m_FogTable[currentIndex++] = value;
				}
			}
			break;
	}

	// max out values after fog ends
	while (currentIndex <= FOG_INTERVAL) {
		m_FogTable[currentIndex++] = 0;
	}

}


inline void Rasterizer :: Fragment(I32 x, I32 y, EGL_Fixed depth, EGL_Fixed tu, EGL_Fixed tv, const Color& baseColor) {

	Color color = baseColor;

	if (m_State->m_TextureEnabled) {

		EGL_Fixed tu0 = m_Texture->GetWrappedS(tu);
		EGL_Fixed tv0 = m_Texture->GetWrappedT(tv);

		Color texColor = 
			m_Texture->GetTexture(m_MipMapLevel)->GetPixel(tu0, tv0);

		switch (m_Texture->GetInternalFormat()) {
			default:
			case Texture::TextureFormatAlpha:
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

			case Texture::TextureFormatLuminance:
			case Texture::TextureFormatRGB:
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

			case Texture::TextureFormatLuminanceAlpha:
			case Texture::TextureFormatRGBA:
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

	bool depthTest;
	
	// fragment level clipping (for now)
	if (m_Surface->GetWidth() <= x || x < 0 ||
		m_Surface->GetHeight() <= y || y < 0) {
		return;
	}

	// fog
	if (m_State->m_FogEnabled) {
		U16 blendingFactor = m_FogTable[depth >> (EGL_PRECISION - FOG_INTERVAL_BITS)];
		color = Color::Blend(color, m_State->m_FogColor, blendingFactor);
	}

	U32 offset = x + y * m_Surface->GetWidth();
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

	if (m_State->m_StencilTestEnabled) {

		bool stencilTest;
		U32 stencilRef = m_State->m_StencilReference & m_State->m_StencilMask;
		U32 stencilValue = m_Surface->GetStencilBuffer()[offset];
		U32 stencil = stencilValue & m_State->m_StencilMask;

		switch (m_State->m_StencilFunc) {
			default:
			case RasterizerState::CompFuncNever:	stencilTest = false;				break;
			case RasterizerState::CompFuncLess:		stencilTest = stencil < stencilRef;	break;
			case RasterizerState::CompFuncEqual:	stencilTest = stencil == stencilRef;break;
			case RasterizerState::CompFuncLEqual:	stencilTest = stencil <= stencilRef;break;
			case RasterizerState::CompFuncGreater:	stencilTest = stencil > stencilRef;	break;
			case RasterizerState::CompFuncNotEqual:	stencilTest = stencil != stencilRef;break;
			case RasterizerState::CompFuncGEqual:	stencilTest = stencil >= stencilRef;break;
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

	if (!depthTest && m_State->m_DepthTestEnabled) {
		return;
	}

	U16 dstValue = m_Surface->GetColorBuffer()[offset];
	U8 dstAlpha = m_Surface->GetAlphaBuffer()[offset];

	// Blending
	if (m_State->m_BlendingEnabled) {

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

	// Masking and write to framebuffer
	if (m_State->m_MaskDepth) {
		m_Surface->GetDepthBuffer()[offset] = depth;
	}

	Color maskedColor = 
		color.Mask(m_State->m_MaskRed, m_State->m_MaskGreen, m_State->m_MaskBlue, m_State->m_MaskAlpha);

	if (m_State->m_LogicOpEnabled) {

		U32 newValue = maskedColor.ConvertToRGBA();
		U32 oldValue = Color::From565A(dstValue, dstAlpha).ConvertToRGBA();
		U32 value;

		switch (m_State->m_LogicOpcode) {
			default:
			case RasterizerState:: LogicOpClear:		value = 0;						break;
			case RasterizerState:: LogicOpAnd:			value = newValue & dstValue;	break;
			case RasterizerState:: LogicOpAndReverse:	value = newValue & ~dstValue;	break;
			case RasterizerState:: LogicOpCopy:			value = newValue;				break;
			case RasterizerState:: LogicOpAndInverted:	value = ~newValue & dstValue;	break;
			case RasterizerState:: LogicOpNoop:			value = dstValue;				break;
			case RasterizerState:: LogicOpXor:			value = newValue ^ dstValue;	break;
			case RasterizerState:: LogicOpOr:			value = newValue | dstValue;	break;
			case RasterizerState:: LogicOpNor:			value = ~(newValue | dstValue); break;
			case RasterizerState:: LogicOpEquiv:		value = ~(newValue ^ dstValue); break;
			case RasterizerState:: LogicOpInvert:		value = ~dstValue;				break;
			case RasterizerState:: LogicOpOrReverse:	value = newValue | ~dstValue;	break;
			case RasterizerState:: LogicOpCopyInverted:	value = ~newValue;				break;
			case RasterizerState:: LogicOpOrInverted:	value = ~newValue | dstValue;	break;
			case RasterizerState:: LogicOpNand:			value = ~(newValue & dstValue); break;
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


inline void Rasterizer :: RasterScanLine(const EdgePos& start, const EdgePos& end, U32 y) {

	// TODO: The depth coordinate should really be interpolated perspectively

	FractionalColor baseColor = start.m_Color;
	EGL_Fixed invSpan = EGL_Inverse(end.m_WindowCoords.x - start.m_WindowCoords.x);

	FractionalColor colorIncrement = (end.m_Color - start.m_Color) * invSpan;
	EGL_Fixed w = start.m_WindowCoords.w;
	EGL_Fixed deltaW = EGL_Mul(end.m_WindowCoords.w - start.m_WindowCoords.w, invSpan);
	EGL_Fixed deltaU = EGL_Mul(end.m_TextureCoords.tu - start.m_TextureCoords.tu, invSpan);
	EGL_Fixed deltaV = EGL_Mul(end.m_TextureCoords.tv - start.m_TextureCoords.tv, invSpan);
	EGL_Fixed tu = start.m_TextureCoords.tu;
	EGL_Fixed tv = start.m_TextureCoords.tv;
	I32 x = EGL_IntFromFixed(start.m_WindowCoords.x);
	I32 xEnd = EGL_IntFromFixed(end.m_WindowCoords.x);

	for (; x < xEnd; ++x) {

		Fragment(x, y, w, tu, tv, baseColor);
		baseColor += colorIncrement;
		w += deltaW;
		tu += deltaU;
		tv += deltaV;
	}
}


// --------------------------------------------------------------------------
// Prepare rasterizer with according to current state settings
// --------------------------------------------------------------------------

void Rasterizer :: PreparePoint() {
	if (!m_IsPrepared) {
		InitFogTable();
	}

	m_MipMapLevel = 0;
	m_IsPrepared = true;
}


void Rasterizer :: PrepareLine() {
	if (!m_IsPrepared) {
		InitFogTable();
	}

	m_MipMapLevel = 0;
	m_IsPrepared = true;
}


void Rasterizer :: PrepareTriangle() {
	if (!m_IsPrepared) {
		InitFogTable();
	}

	m_MipMapLevel = 0;
	m_IsPrepared = true;
}




void Rasterizer :: Finish() {
}


void Rasterizer :: RasterPoint(const RasterPos& point) {

	I32 x = EGL_IntFromFixed(point.m_WindowCoords.x);
	I32 y = EGL_IntFromFixed(point.m_WindowCoords.y);
	EGL_Fixed w = point.m_WindowCoords.w;
	EGL_Fixed tu = point.m_TextureCoords.tu;
	EGL_Fixed tv = point.m_TextureCoords.tv;
	FractionalColor baseColor = point.m_Color;

	Fragment(x, y, w, tu, tv, baseColor);
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
	EGL_Fixed tu = from.m_TextureCoords.tu;
	EGL_Fixed tv = from.m_TextureCoords.tv;
	EGL_Fixed w = from.m_WindowCoords.w;

	if (EGL_Abs(deltaX) > EGL_Abs(deltaY)) {
		// Bresenheim along x-axis

		EGL_Fixed invSpan = EGL_Inverse(deltaX);

		EGL_Fixed slope = EGL_Abs(EGL_Mul(deltaY, invSpan));
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaW = EGL_Mul(to.m_WindowCoords.w - from.m_WindowCoords.w, invSpan);
		EGL_Fixed deltaU = EGL_Mul(to.m_TextureCoords.tu - from.m_TextureCoords.tu, invSpan);
		EGL_Fixed deltaV = EGL_Mul(to.m_TextureCoords.tv - from.m_TextureCoords.tv, invSpan);

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x);
		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y);
		I32 endX = EGL_IntFromFixed(to.m_WindowCoords.x);
		I32 yIncrement = (deltaY > 0) ? 1 : -1;
		EGL_Fixed error = 0;

		for (; x < endX; ++x) {

			Fragment(x, y, w, tu, tv, baseColor);

			error += slope;
			if (error > EGL_FixedFromFloat(0.5f)) {
				y += yIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			w += deltaW;
			tu += deltaU;
			tv += deltaV;
		}

	} else {
		// Bresenheim along y-axis

		EGL_Fixed invSpan = EGL_Inverse(deltaY);

		EGL_Fixed slope = EGL_Abs(EGL_Mul(deltaX, invSpan));
		FractionalColor colorIncrement = (to.m_Color - from.m_Color) * invSpan;
		EGL_Fixed deltaW = EGL_Mul(to.m_WindowCoords.w - from.m_WindowCoords.w, invSpan);
		EGL_Fixed deltaU = EGL_Mul(to.m_TextureCoords.tu - from.m_TextureCoords.tu, invSpan);
		EGL_Fixed deltaV = EGL_Mul(to.m_TextureCoords.tv - from.m_TextureCoords.tv, invSpan);

		I32 x = EGL_IntFromFixed(from.m_WindowCoords.x);
		I32 y = EGL_IntFromFixed(from.m_WindowCoords.y);
		I32 endY = EGL_IntFromFixed(to.m_WindowCoords.y);
		I32 xIncrement = (deltaX > 0) ? 1 : -1;
		EGL_Fixed error = 0;

		for (; y < endY; ++y) {

			Fragment(x, y, w, tu, tv, baseColor);

			error += slope;
			if (error > EGL_FixedFromFloat(0.5f)) {
				x += xIncrement;
				error -= EGL_ONE;
			}

			baseColor += colorIncrement;
			w += deltaW;
			tu += deltaU;
			tv += deltaV;
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
}


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
			EGL_Fixed textureArea = 
				TriangleArea(a.m_TextureCoords.tu, a.m_TextureCoords.tv,
							 b.m_TextureCoords.tu, b.m_TextureCoords.tv,
							 c.m_TextureCoords.tu, c.m_TextureCoords.tv);

			int logWidth = Log2(m_Texture->GetTexture(0)->GetWidth());
			int logHeight = Log2(m_Texture->GetTexture(0)->GetHeight());

			EGL_Fixed screenArea = 
				TriangleArea(a.m_WindowCoords.x >> logWidth, a.m_WindowCoords.y >> logHeight,
							 b.m_WindowCoords.x >> logWidth, b.m_WindowCoords.y >> logHeight,
							 c.m_WindowCoords.x >> logWidth, c.m_WindowCoords.y >> logHeight);
			EGL_Fixed invScreenArea = EGL_Inverse(screenArea);

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

	EdgePos start, end;
	start.m_WindowCoords.x = end.m_WindowCoords.x = pos1.m_WindowCoords.x;
	start.m_WindowCoords.w = end.m_WindowCoords.w = pos1.m_WindowCoords.w;
	start.m_Color = end.m_Color = pos1.m_Color;
	start.m_TextureCoords = end.m_TextureCoords = pos1.m_TextureCoords;

	// set up the triangle
	// init start, end, deltas
	EGL_Fixed invDeltaY2 = EGL_Inverse(pos2.m_WindowCoords.y - pos1.m_WindowCoords.y);

	EGL_Fixed incX2 = EGL_Mul(pos2.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY2);
	EGL_Fixed incR2 = EGL_Mul(pos2.m_Color.r - pos1.m_Color.r, invDeltaY2);
	EGL_Fixed incG2 = EGL_Mul(pos2.m_Color.g - pos1.m_Color.g, invDeltaY2);
	EGL_Fixed incB2 = EGL_Mul(pos2.m_Color.b - pos1.m_Color.b, invDeltaY2);
	EGL_Fixed incA2 = EGL_Mul(pos2.m_Color.a - pos1.m_Color.a, invDeltaY2);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW2 = EGL_Mul(pos2.m_WindowCoords.w - pos1.m_WindowCoords.w, invDeltaY2);
	EGL_Fixed incTu2 = EGL_Mul(pos2.m_TextureCoords.tu - pos1.m_TextureCoords.tu, invDeltaY2);
	EGL_Fixed incTv2 = EGL_Mul(pos2.m_TextureCoords.tv - pos1.m_TextureCoords.tv, invDeltaY2);

	EGL_Fixed invDeltaY3 = EGL_Inverse(pos3.m_WindowCoords.y - pos1.m_WindowCoords.y);

	EGL_Fixed incX3 = EGL_Mul(pos3.m_WindowCoords.x - pos1.m_WindowCoords.x, invDeltaY3);
	EGL_Fixed incR3 = EGL_Mul(pos3.m_Color.r - pos1.m_Color.r, invDeltaY3);
	EGL_Fixed incG3 = EGL_Mul(pos3.m_Color.g - pos1.m_Color.g, invDeltaY3);
	EGL_Fixed incB3 = EGL_Mul(pos3.m_Color.b - pos1.m_Color.b, invDeltaY3);
	EGL_Fixed incA3 = EGL_Mul(pos3.m_Color.a - pos1.m_Color.a, invDeltaY3);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW3 = EGL_Mul(pos3.m_WindowCoords.w - pos1.m_WindowCoords.w, invDeltaY3);
	EGL_Fixed incTu3 = EGL_Mul(pos3.m_TextureCoords.tu - pos1.m_TextureCoords.tu, invDeltaY3);
	EGL_Fixed incTv3 = EGL_Mul(pos3.m_TextureCoords.tv - pos1.m_TextureCoords.tv, invDeltaY3);

	EGL_Fixed invDeltaY23 = EGL_Inverse(pos3.m_WindowCoords.y - pos2.m_WindowCoords.y);

	EGL_Fixed incX23 = EGL_Mul(pos3.m_WindowCoords.x - pos2.m_WindowCoords.x, invDeltaY23);
	EGL_Fixed incR23 = EGL_Mul(pos3.m_Color.r - pos2.m_Color.r, invDeltaY23);
	EGL_Fixed incG23 = EGL_Mul(pos3.m_Color.g - pos2.m_Color.g, invDeltaY23);
	EGL_Fixed incB23 = EGL_Mul(pos3.m_Color.b - pos2.m_Color.b, invDeltaY23);
	EGL_Fixed incA23 = EGL_Mul(pos3.m_Color.a - pos2.m_Color.a, invDeltaY23);

	// initially, use linear interpolation, change to perspective later:
	EGL_Fixed incW23 = EGL_Mul(pos3.m_WindowCoords.w - pos2.m_WindowCoords.w, invDeltaY23);
	EGL_Fixed incTu23 = EGL_Mul(pos3.m_TextureCoords.tu - pos2.m_TextureCoords.tu, invDeltaY23);
	EGL_Fixed incTv23 = EGL_Mul(pos3.m_TextureCoords.tv - pos2.m_TextureCoords.tv, invDeltaY23);

	I32 yStart = EGL_IntFromFixed(pos1.m_WindowCoords.y);
	I32 yEnd = EGL_IntFromFixed(pos2.m_WindowCoords.y);
	I32 y;

	if (incX2 < incX3) {
		for (y = yStart; y < yEnd; ++y) {
			RasterScanLine(start, end, y);

			// update start
			start.m_WindowCoords.x += incX2;
			start.m_Color.r += incR2;
			start.m_Color.g += incG2;
			start.m_Color.b += incB2;
			start.m_Color.a += incA2;

			start.m_WindowCoords.w += incW2;
			start.m_TextureCoords.tu += incTu2;
			start.m_TextureCoords.tv += incTv2;

			// update end
			end.m_WindowCoords.x += incX3;
			end.m_Color.r += incR3;
			end.m_Color.g += incG3;
			end.m_Color.b += incB3;
			end.m_Color.a += incA3;

			end.m_WindowCoords.w += incW3;
			end.m_TextureCoords.tu += incTu3;
			end.m_TextureCoords.tv += incTv3;

		}

		yEnd = EGL_IntFromFixed(pos3.m_WindowCoords.y);

		start.m_WindowCoords.x = pos2.m_WindowCoords.x;
		start.m_WindowCoords.w = pos2.m_WindowCoords.w;
		start.m_Color = pos2.m_Color;
		start.m_TextureCoords = pos2.m_TextureCoords;

		for (; y < yEnd; ++y) {
			RasterScanLine(start, end, y);
			// update start
			start.m_WindowCoords.x += incX23;
			start.m_Color.r += incR23;
			start.m_Color.g += incG23;
			start.m_Color.b += incB23;
			start.m_Color.a += incA23;

			start.m_WindowCoords.w += incW23;
			start.m_TextureCoords.tu += incTu23;
			start.m_TextureCoords.tv += incTv23;

			// update end
			end.m_WindowCoords.x += incX3;
			end.m_Color.r += incR3;
			end.m_Color.g += incG3;
			end.m_Color.b += incB3;
			end.m_Color.a += incA3;

			end.m_WindowCoords.w += incW3;
			end.m_TextureCoords.tu += incTu3;
			end.m_TextureCoords.tv += incTv3;

		}
	} else {
		for (y = yStart; y < yEnd; ++y) {
			RasterScanLine(start, end, y);

			// update start
			start.m_WindowCoords.x += incX3;
			start.m_Color.r += incR3;
			start.m_Color.g += incG3;
			start.m_Color.b += incB3;
			start.m_Color.a += incA3;

			start.m_WindowCoords.w += incW3;
			start.m_TextureCoords.tu += incTu3;
			start.m_TextureCoords.tv += incTv3;

			// update end
			end.m_WindowCoords.x += incX2;
			end.m_Color.r += incR2;
			end.m_Color.g += incG2;
			end.m_Color.b += incB2;
			end.m_Color.a += incA2;

			end.m_WindowCoords.w += incW2;
			end.m_TextureCoords.tu += incTu2;
			end.m_TextureCoords.tv += incTv2;

		}

		yEnd = EGL_IntFromFixed(pos3.m_WindowCoords.y);

		end.m_WindowCoords.x = pos2.m_WindowCoords.x;
		end.m_WindowCoords.w = pos2.m_WindowCoords.w;
		end.m_Color = pos2.m_Color;
		end.m_TextureCoords = pos2.m_TextureCoords;

		for (; y < yEnd; ++y) {
			RasterScanLine(start, end, y);
			// update start
			start.m_WindowCoords.x += incX3;
			start.m_Color.r += incR3;
			start.m_Color.g += incG3;
			start.m_Color.b += incB3;
			start.m_Color.a += incA3;

			start.m_WindowCoords.w += incW3;
			start.m_TextureCoords.tu += incTu3;
			start.m_TextureCoords.tv += incTv3;

			// update end
			end.m_WindowCoords.x += incX23;
			end.m_Color.r += incR23;
			end.m_Color.g += incG23;
			end.m_Color.b += incB23;
			end.m_Color.a += incA23;

			end.m_WindowCoords.w += incW23;
			end.m_TextureCoords.tu += incTu23;
			end.m_TextureCoords.tv += incTv23;

		}
	}
}