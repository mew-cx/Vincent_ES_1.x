#ifndef EGL_TEXTURE_H
#define EGL_TEXTURE_H 1

#pragma once

// ==========================================================================
//
// Texture.h		Texture Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 09-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"
#include "GLES/gl.h"
#include "Config.h"


namespace EGL {
	class OGLES_API Texture { 
	public:
		enum TextureFormatInternal {
			TextureFormatLuminance = 1,			// 8
			TextureFormatLuminanceAlpha = 2,	// 7-1
			TextureFormatRGB = 3,				// 5-6-5
			TextureFormatRGBA = 4				// 5-5-5-1
		};

	public:

		Texture();
		~Texture();

		void Initialize(U32 width, U32 height, TextureFormatInternal format);

		U32 GetWidth() const				{ return m_Width; }
		U32 GetHeight() const				{ return m_Height; }
		U32 GetExponent() const				{ return m_Exponent; }

		TextureFormatInternal 
			GetInternalFormat() const		{ return m_InternalFormat; }

		U8 GetBytesPerPixel() const			{ return s_BytesPerPixel[m_InternalFormat]; }

		void * GetData() const				{ return m_Data; }

	private:
		void *					m_Data;
		U32						m_Width;
		U32						m_Height;
		U32						m_Exponent;
		TextureFormatInternal	m_InternalFormat;

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

	private:
		Texture	*		m_TextureLevels[MAX_LEVELS];
		U32				m_BaseLevel;
		U32				m_Levels;
	};

}

#endif //ndef EGL_TEXTURE_H