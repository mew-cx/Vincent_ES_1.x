// ==========================================================================
//
// Texture.cpp		Texture Class for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 10-15-2003		Hans-Martin Will	initial version
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
#include "Texture.h"
#include <stdlib.h>

using namespace EGL;


// --------------------------------------------------------------------------
// Class Texture
// --------------------------------------------------------------------------


U8 Texture :: s_BytesPerPixel[] = {
	1,				// ALPHA
	1,				// LUMINANCE
	2,				// LUMINANCE_ALPHA
	2,				// RGB
	2				// RGBA
};


Texture :: Texture():
	m_Data(0), m_InternalFormat(static_cast<RasterizerState::TextureFormat>(0))
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


void Texture :: Initialize(U32 width, U32 height, RasterizerState::TextureFormat format) {

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
	size_t bytes = pixels * GetBytesPerPixel();
	m_Data = malloc(bytes);
}


U32 Texture :: GetLogWidth() const {
	return Log(m_Width);
}


U32 Texture :: GetLogHeight() const {
	return Log(m_Height);
}


U32 Texture :: GetLogBytesPerPixel() const {
	return Log(GetBytesPerPixel());
}


// --------------------------------------------------------------------------
// Class MultiTexture
// --------------------------------------------------------------------------


MultiTexture :: MultiTexture():
	m_MinFilterMode(RasterizerState::MinFilterModeNearest),
	m_MagFilterMode(RasterizerState::MagFilterModeNearest),
	m_WrappingModeS(RasterizerState::WrappingModeRepeat),
	m_WrappingModeT(RasterizerState::WrappingModeRepeat)
{
	for (int index = 0; index < MAX_LEVELS; ++index) {
		m_TextureLevels[index] = new Texture();
	}
}


MultiTexture :: ~MultiTexture() {
	for (int index = 0; index < MAX_LEVELS; ++index) {
		delete m_TextureLevels[index];
	}
}


// --------------------------------------------------------------------------
// Verify that this texture is consistently defined across all mipmapping 
// levels (as specified in section 3.8.9)
//
// Returns:
//	true		-	if this texture is defined consistently across all
//					levels
// --------------------------------------------------------------------------
bool MultiTexture :: IsComplete() const {

	if (m_TextureLevels[0] == 0) {
		return false;
	}

	U32 width = m_TextureLevels[0]->GetWidth();
	U32 height = m_TextureLevels[0]->GetHeight();
	RasterizerState::TextureFormat format = m_TextureLevels[0]->GetInternalFormat();

	for (int index = 1; index < MAX_LEVELS; ++index) {
		if (width == 1 && height == 1) {
			return true;
		}

		if (width > 1) {
			width = width / 2;
		}

		if (height > 1) {
			height = height / 2;
		}

		if (m_TextureLevels[index] == 0) {
			return false;
		}

		if (m_TextureLevels[index]->GetWidth() != width ||
			m_TextureLevels[index]->GetHeight() != height ||
			m_TextureLevels[index]->GetInternalFormat() != format) {
			return false;
		}
	}

	return true;
}