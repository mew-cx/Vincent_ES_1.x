#ifndef EGL_TEXTURE_H
#define EGL_TEXTURE_H 1

#pragma once

// ==========================================================================
//
// Texture.h		Texture Class for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 09-14-2003		Hans-Martin Will	initial version
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
#include "GLES/gl.h"
#include "Config.h"
#include "Color.h"


namespace EGL {
	class OGLES_API Texture { 
	public:
		enum TextureFormat {
			TextureFormatInvalid = -1,
			TextureFormatAlpha = 0,				// 8
			TextureFormatLuminance = 1,			// 8
			TextureFormatLuminanceAlpha = 2,	// 8-8
			TextureFormatRGB = 3,				// 5-6-5
			TextureFormatRGBA = 4				// 5-5-5-1
		};

	public:

		Texture();
		~Texture();

		void Initialize(U32 width, U32 height, TextureFormat format);

		U32 GetWidth() const				{ return m_Width; }
		U32 GetHeight() const				{ return m_Height; }
		U32 GetExponent() const				{ return m_Exponent; }

		TextureFormat 
			GetInternalFormat() const		{ return m_InternalFormat; }

		U8 GetBytesPerPixel() const			{ return s_BytesPerPixel[m_InternalFormat]; }

		void * GetData() const				{ return m_Data; }

	private:
		void *			m_Data;
		U32				m_Width;
		U32				m_Height;
		U32				m_Exponent;
		TextureFormat	m_InternalFormat;

		static U8 s_BytesPerPixel[];
	};


	class OGLES_API MultiTexture {
	public:
		enum WrappingMode {
			WrappingModeInvalid = -1,
			WrappingModeClampToEdge,
			WrappingModeRepeat
		};

		enum MinFilterMode {
			MinFilterModeInvalid = -1,
			MinFilterModeNearest,
			MinFilterModeLinear,
			MinFilterModeNearestMipmapNearest,
			MinFilterModeNearestMipmapLinear,
			MinFilterModeLinearMipmapNearest,
			MinFilterModeLinearMipmapLinear,
		};

		enum MagFilterMode {
			MagFilterModeInvalid = -1,
			MagFilterModeNearest,
			MagFilterModeLinear
		};

	public:
		enum {
			MAX_LEVELS = 20
		};

		MultiTexture();
		~MultiTexture();

		Texture * GetTexture(int level)				{ return m_TextureLevels[level]; }
		const Texture * GetTexture(int level) const	{ return m_TextureLevels[level]; }

		void SetMinFilterMode(MinFilterMode mode)	{ m_MinFilterMode = mode; }
		void SetMagFilterMode(MagFilterMode mode)	{ m_MagFilterMode = mode; }
		void SetWrappingModeS(WrappingMode mode)	{ m_WrappingModeS = mode; }
		void SetWrappingModeT(WrappingMode mode)	{ m_WrappingModeT = mode; }

		MinFilterMode GetMinFilterMode() const		{ return m_MinFilterMode; }
		MagFilterMode GetMagFilterMode() const		{ return m_MagFilterMode; }
		WrappingMode GetWrappingModeS() const		{ return m_WrappingModeS; }
		WrappingMode GetWrappingModeT() const		{ return m_WrappingModeT; }

		Texture::TextureFormat 
			GetInternalFormat() const				{ return m_TextureLevels[0]->GetInternalFormat(); }

		bool IsComplete() const;

		bool IsMipMap() const;

	private:
		Texture	*		m_TextureLevels[MAX_LEVELS];
		MinFilterMode	m_MinFilterMode;
		MagFilterMode	m_MagFilterMode;
		WrappingMode	m_WrappingModeS;
		WrappingMode	m_WrappingModeT;
		U32				m_Levels;
	};

	inline bool MultiTexture :: IsMipMap() const {
		return 
			m_MinFilterMode == MinFilterModeNearestMipmapNearest ||
			m_MinFilterMode == MinFilterModeNearestMipmapLinear ||
			m_MinFilterMode == MinFilterModeLinearMipmapNearest ||
			m_MinFilterMode == MinFilterModeLinearMipmapLinear;
	}

}

#endif //ndef EGL_TEXTURE_H