#ifndef EGL_FRACTIONAL_COLOR_H
#define EGL_FRACTIONAL_COLOR_H 1

#pragma once

// ==========================================================================
//
// FractionalColor	Fixed point representation of an RGBA color
//
// --------------------------------------------------------------------------
//
// 10-05-2003		Hans-Martin Will	initial version
//
// ==========================================================================



#include "OGLES.h"
#include "fixed.h"
#include "Color.h"


namespace EGL {
	class OGLES_API FractionalColor {

	public:
		EGL_Fixed	r, g, b, a;			// rgba components

	public:
		FractionalColor() {
			r = g = b = a = 0;
		}


		FractionalColor(EGL_Fixed R, EGL_Fixed G, EGL_Fixed B, EGL_Fixed A) {
			r = R;
			g = G;
			b = B;
			a = A;
		}

		FractionalColor(const EGL_Fixed * rgba) {
			r = rgba[0];
			g = rgba[1];
			b = rgba[2];
			a = rgba[3];
		}

		static FractionalColor Clamp(const EGL_Fixed * rgba) {
			return FractionalColor(
				EGL_CLAMP(rgba[0], 0, EGL_ONE),
				EGL_CLAMP(rgba[1], 0, EGL_ONE),
				EGL_CLAMP(rgba[2], 0, EGL_ONE),
				EGL_CLAMP(rgba[3], 0, EGL_ONE));
		}

		FractionalColor(const FractionalColor& other) {
			r = other.r;
			b = other.b;
			g = other.g;
			a = other.a;
		}

		FractionalColor& operator=(const FractionalColor& other) {
			r = other.r;
			b = other.b;
			g = other.g;
			a = other.a;
			return *this;
		}

		// TODO; Decide: Is Fatcional color 8.16 or (1).16 format
		inline U16 ConvertTo565() const {
			return 
				 EGL_IntFromFixed(b * 0xFF) >> 3 | 
				(EGL_IntFromFixed(g * 0xFF) & 0xFC) << 3 | 
				(EGL_IntFromFixed(b * 0xFF) & 0xF8) << 8;
		}

		inline U16 ConvertTo5551() const {
			return 
				 EGL_IntFromFixed(b * 0xFF) >> 3 | 
				(EGL_IntFromFixed(g * 0xFF) & 0xF8) << 2 | 
				(EGL_IntFromFixed(b * 0xFF) & 0xF8) << 7 | 
				(EGL_IntFromFixed(a * 0xFF) & 0x80) << 8;
		}

#if 0
		static inline FractionalColor From565(U16 u565) {
			U8 r = (u565 & 0x001Fu) << 3;
			U8 g = (u565 & 0x07E0u) >> 3;
			U8 b = (u565 & 0xF800u) >> 8;

			return FractionalColor(
				EGL_FixedFromInt(r), 
				EGL_FixedFromInt(g), 
				EGL_FixedFromInt(b), 
				EGL_FixedFromInt(0xFF));
		}

		static inline FractionalColor From5551(U16 u5551) {
			U8 r = (u5551 & 0x001Fu) << 3;
			U8 g = (u5551 & 0x03E0u) >> 2;
			U8 b = (u5551 & 0x7C00u) >> 7;
			U8 a = (u5551 & 0x8000u) >> 8;

			return FractionalColor(
				EGL_FixedFromInt(r), 
				EGL_FixedFromInt(g), 
				EGL_FixedFromInt(b), 
				EGL_FixedFromInt(a));
		}
#endif
	
		// convert fixed point to byte format
		inline operator Color() const {
			return Color(
				EGL_IntFromFixed(0xFF * r),
				EGL_IntFromFixed(0xFF * g),
				EGL_IntFromFixed(0xFF * b),
				EGL_IntFromFixed(0xFF * a));
		}

		inline void Clamp() {
			r = EGL_CLAMP(r, 0, EGL_ONE);
			g = EGL_CLAMP(g, 0, EGL_ONE);
			b = EGL_CLAMP(b, 0, EGL_ONE);
			a = EGL_CLAMP(a, 0, EGL_ONE);
		}

		inline FractionalColor operator*(const FractionalColor& other) const {
			return FractionalColor(EGL_Mul(r, other.r),
				EGL_Mul(g, other.g), EGL_Mul(b, other.b), a);
		}


		inline FractionalColor& operator*=(const FractionalColor& other) {
			r = EGL_Mul(r, other.r);
			g = EGL_Mul(g, other.g);
			b = EGL_Mul(b, other.b);
			return *this;
		}

		inline FractionalColor operator*(EGL_Fixed scale) const {
			return FractionalColor(EGL_Mul(r, scale),
				EGL_Mul(g, scale), EGL_Mul(b, scale), a);
		}


		inline FractionalColor& operator*=(EGL_Fixed scale) {
			r = EGL_Mul(r, scale);
			g = EGL_Mul(g, scale);
			b = EGL_Mul(b, scale);
			return *this;
		}

		inline FractionalColor operator-(const FractionalColor& other) const {
			return FractionalColor(r - other.r, g - other.g, b - other.b, a - other.a);
		}

		inline FractionalColor operator+(const FractionalColor& other) const {
			return FractionalColor(r + other.r, g + other.g, b + other.b, a + other.a);
		}

		inline void Accumulate(const FractionalColor& color,
							EGL_Fixed scale) {
			r  += EGL_Mul(color.r, scale);
			g += EGL_Mul(color.g, scale);
			b += EGL_Mul(color.b, scale);
		}


		inline FractionalColor& operator+=(const FractionalColor& color) {
			r += color.r;
			g += color.g;
			b += color.b;
			//a += color.a;

			return *this;
		}
	};
}


#endif //ndef EGL_FRACTIONAL_COLOR_H