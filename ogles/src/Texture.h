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
#include "RasterizerState.h"


namespace EGL {
	class OGLES_API Texture { 
	public:

		Texture();
		~Texture();

		void Initialize(U32 width, U32 height, RasterizerState::TextureFormat format);

		U32 GetWidth() const				{ return m_Width; }
		U32 GetHeight() const				{ return m_Height; }
		U32 GetExponent() const				{ return m_Exponent; }

		RasterizerState::TextureFormat 
			GetInternalFormat() const		{ return m_InternalFormat; }

		U8 GetBytesPerPixel() const			{ return s_BytesPerPixel[m_InternalFormat]; }

		void * GetData() const				{ return m_Data; }

	private:
		void *							m_Data;
		U32								m_Width;
		U32								m_Height;
		U32								m_Exponent;
		RasterizerState::TextureFormat	m_InternalFormat;

		static U8 s_BytesPerPixel[];
	};


	class OGLES_API MultiTexture {
	public:
		enum {
			MAX_LEVELS = 20
		};

		MultiTexture();
		~MultiTexture();

		Texture * GetTexture(int level)				{ return m_TextureLevels[level]; }
		const Texture * GetTexture(int level) const	{ return m_TextureLevels[level]; }

		void SetMinFilterMode(RasterizerState::MinFilterMode mode)	{ m_MinFilterMode = mode; }
		void SetMagFilterMode(RasterizerState::MagFilterMode mode)	{ m_MagFilterMode = mode; }
		void SetWrappingModeS(RasterizerState::WrappingMode mode)	{ m_WrappingModeS = mode; }
		void SetWrappingModeT(RasterizerState::WrappingMode mode)	{ m_WrappingModeT = mode; }

		RasterizerState::MinFilterMode GetMinFilterMode() const		{ return m_MinFilterMode; }
		RasterizerState::MagFilterMode GetMagFilterMode() const		{ return m_MagFilterMode; }
		RasterizerState::WrappingMode GetWrappingModeS() const		{ return m_WrappingModeS; }
		RasterizerState::WrappingMode GetWrappingModeT() const		{ return m_WrappingModeT; }

		RasterizerState::TextureFormat 
			GetInternalFormat() const				{ return m_TextureLevels[0]->GetInternalFormat(); }

		bool IsComplete() const;

		bool IsMipMap() const;

	private:
		Texture	*						m_TextureLevels[MAX_LEVELS];
		RasterizerState::MinFilterMode	m_MinFilterMode;
		RasterizerState::MagFilterMode	m_MagFilterMode;
		RasterizerState::WrappingMode	m_WrappingModeS;
		RasterizerState::WrappingMode	m_WrappingModeT;
		U32								m_Levels;
	};

	inline bool MultiTexture :: IsMipMap() const {
		return 
			m_MinFilterMode == RasterizerState::MinFilterModeNearestMipmapNearest ||
			m_MinFilterMode == RasterizerState::MinFilterModeNearestMipmapLinear ||
			m_MinFilterMode == RasterizerState::MinFilterModeLinearMipmapNearest ||
			m_MinFilterMode == RasterizerState::MinFilterModeLinearMipmapLinear;
	}

}

#endif //ndef EGL_TEXTURE_H