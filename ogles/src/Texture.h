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
#include "internal.h"


namespace EGL {
	class OGLES_API Texture { // this is actually RGB 565

	public:


	private:
		U16	*		m_RGBA;
		U32			m_Width;
		U32			m_Height;
		U32			m_ExpSize;
	};

}

#endif //ndef EGL_TEXTURE_H