#ifndef EGL_COLOR_H
#define EGL_COLOR_H 1

#pragma once

// ==========================================================================
//
// Color			Integer 8-8-8-8 representation of an RGBA color
//
// --------------------------------------------------------------------------
//
// 10-09-2003		Hans-Martin Will	initial version
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


#include "OGLES.h"
#include "fixed.h"


namespace EGL {

	class OGLES_API Color {

	public:
		enum {
			MAX = 0xff
		};

	public:
		U8	r, g, b, a;							// rgba components

	public:
		Color() {
			r = g = b = a = 0;
		}


		Color(U8 R, U8 G, U8 B, U8 A) {
			r = R;
			g = G;
			b = B;
			a = A;
		}

		Color(const Color& other) {
			r = other.r;
			b = other.b;
			g = other.g;
			a = other.a;
		}

		Color& operator=(const Color& other) {
			r = other.r;
			b = other.b;
			g = other.g;
			a = other.a;
			return *this;
		}

		inline U8 A() const {
			return a;
		}

		inline U8 R() const {
			return r;
		}

		inline U8 G() const {
			return g;
		}

		inline U8 B() const {
			return b;
		}

		inline Color Mask(bool maskR, bool maskG, bool maskB, bool maskA) const {
			return Color(maskR ? r : 0, maskG ? g : 0, maskB ? b : 0, maskA ? a : 0);
		}

		inline U16 ConvertTo565() const {
			return r >> 3 | (g & 0xFC) << 3 | (b & 0xF8) << 8;
		}

		inline U32 ConvertToRGBA() const {
			return r << 24 | b << 16 | g << 8 | a;
		}

		inline U16 ConvertTo5551() const {
			return r >> 3 | (g & 0xF8) << 2 | (b & 0xF8) << 7 | (a & 0x80) << 8;
		}

		inline U16 ConvertTo4444() const {
			return (r & 0xf0) << 8 |
				(g & 0xf0) << 4 |
				(b & 0xf0) |
				a >> 4;
		}

		static inline Color FromRGBA(U32 rgba) {
			U8 r = (rgba & 0xFF000000) >> 24;
			U8 g = (rgba & 0x00FF0000) >> 16;
			U8 b = (rgba & 0x0000FF00) >>  8;
			U8 a = (rgba & 0x000000FF);

			return Color(r, g, b, a);
		}

		static inline Color From4444(U16 u4444) {
			U8 r = (u4444 & 0xF000u) << 8;
			U8 g = (u4444 & 0x0F00u) >> 4;
			U8 b = (u4444 & 0x00F0u);
			U8 a = (u4444 & 0x000Fu) << 4;

			return Color(r, g, b, a);
		}

		static inline Color From565(U16 u565) {
			U8 r = (u565 & 0x001Fu) << 3;
			U8 g = (u565 & 0x07E0u) >> 3;
			U8 b = (u565 & 0xF800u) >> 8;

			return Color(r, g, b, 0xFF);
		}

		static inline Color From565A(U16 u565, U8 alpha) {
			U8 r = (u565 & 0x001Fu) << 3;
			U8 g = (u565 & 0x07E0u) >> 3;
			U8 b = (u565 & 0xF800u) >> 8;

			return Color(r, g, b, alpha);
		}

		static inline Color From5551(U16 u5551) {
			U8 r = (u5551 & 0x001Fu) << 3;
			U8 g = (u5551 & 0x03E0u) >> 2;
			U8 b = (u5551 & 0x7C00u) >> 7;
			U8 a = (u5551 & 0x8000u) >> 8;

			return Color(r, g, b, a);
		}

		Color operator+(const Color& other) const {
			return Color(clamp(r + other.r), clamp(g + other.g), 
				clamp(b + other.b), clamp(a + other.a));
		}

		Color operator*(const Color& factor) const {
			return Color(mul(r, factor.r), mul(g, factor.g), mul(b, factor.b), mul(a, factor.a));
		}

		// -------------------------------------------------------------------------
		// return the blend of src and dst, where alpha is a value between 0 and 0x100
		// -------------------------------------------------------------------------
		static inline Color Blend(const Color& src, const Color& dst, U32 alpha) {
			U32 oneMinusAlpha = 0x100 - alpha;

			return Color((src.R() * alpha + dst.R() * oneMinusAlpha) >> 8,
				(src.G() * alpha + dst.G() * oneMinusAlpha) >> 8,
				(src.B() * alpha + dst.B() * oneMinusAlpha) >> 8,
				src.A());
		}

		static inline Color Blend(const Color& src, const Color& dst) {
			return Blend(src, dst, s_alphaFactor[src.A()]);
		}

		static void InitAlphaFactorTable();

	private:
		static U8 clamp(U16 value) {
			return (value > MAX) ? (U8) MAX : (U8) value;
		}

		static U8 mul(U8 color, U8 factor) {
			if (factor & 0x80) {
				return (color * (factor + 1)) >> 8;
			} else {
				return (color * factor) >> 8;
			}
		}

		static EGL_Fixed s_alphaFactor[256];		// lookup table to convert 0..255 into 0 .. 1.0
	};

}

#endif //ndef EGL_COLOR_H