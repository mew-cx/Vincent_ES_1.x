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
// ==========================================================================


#include "OGLES.h"
#include "fixed.h"


namespace EGL {

	class OGLES_API Color {

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

		inline U16 ConvertTo565() const {
			return b >> 3 | (g & 0xFC) << 3 | (b & 0xF8) << 8;
		}

		inline U16 ConvertTo5551() const {
			return b >> 3 | (g & 0xF8) << 2 | (b & 0xF8) << 7 | (a & 0x80) << 8;
		}

		static inline Color From565(U16 u565) {
			U8 r = (u565 & 0x001Fu) << 3;
			U8 g = (u565 & 0x07E0u) >> 3;
			U8 b = (u565 & 0xF800u) >> 8;

			return Color(r, g, b, 0xFF);
		}

		static inline Color From5551(U16 u5551) {
			U8 r = (u5551 & 0x001Fu) << 3;
			U8 g = (u5551 & 0x03E0u) >> 2;
			U8 b = (u5551 & 0x7C00u) >> 7;
			U8 a = (u5551 & 0x8000u) >> 8;

			return Color(r, g, b, a);
		}

		// -------------------------------------------------------------------------
		// return the blend of src and dst, where alpha is a value between 0 and 1
		// -------------------------------------------------------------------------
		static inline Color Blend(const Color& src, const Color& dst, EGL_Fixed alpha) {
			EGL_Fixed oneMinusAlpha = EGL_ONE - alpha;

			return Color((src.R() * alpha + dst.R() * oneMinusAlpha) >> EGL_PRECISION,
				(src.G() * alpha + dst.G() * oneMinusAlpha) >> EGL_PRECISION,
				(src.B() * alpha + dst.B() * oneMinusAlpha) >> EGL_PRECISION,
				src.A());
		}

		static inline Color Blend(const Color& src, const Color& dst) {
			return Blend(src, dst, s_alphaFactor[src.A()]);
		}

		static void InitAlphaFactorTable();

	private:
		static EGL_Fixed s_alphaFactor[256];		// lookup table to convert 0..255 into 0 .. 1.0
	};

}

#endif //ndef EGL_COLOR_H