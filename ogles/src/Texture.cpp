// ==========================================================================
//
// Texture.h		Texture Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 10-15-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Texture.h"
#include <stdlib.h>

using namespace EGL;


// --------------------------------------------------------------------------
// Class Texture
// --------------------------------------------------------------------------


U8 Texture :: s_BytesPerPixel[] = {
	0,				// undefined
	1,				// LUMINANCE
	1,				// LUMINANCE_ALPHA
	2,				// RGB
	2				// RGBA
};


Texture :: Texture():
	m_Data(0), m_InternalFormat(static_cast<TextureFormatInternal>(0))
{
}


Texture :: ~Texture() {
	if (m_Data != 0) {
		free(m_Data);
		m_Data = 0;
	}
}


namespace {
	U32 Log(U32 value) {
		U32 result = 0;
		U32 mask = 1;

		while ((value & mask) != value) {
			++result;
			mask = (mask << 1) | 1;
		}

		return result;
	}
}


void Texture :: Initialize(U32 width, U32 height, TextureFormatInternal format) {

	if (m_Data != 0) {
		free(m_Data);
	}

	m_Width = width;
	m_Height = height;

	if (width > height) {
		m_Exponent = Log(width);
	} else {
		m_Exponent = Log(height);
	}

	U32 pixels = width * height;
	m_InternalFormat = format;
	m_Data = malloc(pixels * GetBytesPerPixel());
}


// --------------------------------------------------------------------------
// Class MultiTexture
// --------------------------------------------------------------------------


MultiTexture :: MultiTexture() {
}


MultiTexture :: ~MultiTexture() {
}