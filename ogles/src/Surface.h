#ifndef EGL_SURFACE_H
#define EGL_SURFACE_H 1

#pragma once

// ==========================================================================
//
// Surface.h		Drawing Surface Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"
#include "GLES/egl.h"
#include "GLES/gl.h"
#include "Config.h"
#include "fixed.h"
#include "Color.h"
#include "FractionalColor.h"


namespace EGL {

	class OGLES_API Surface {
		friend Context;

	public:

		// Create a PBuffer surface
		Surface(const Config & config, HDC hdc = 0);
		~Surface();

		// Is the depth value re-scaled based on near/far settings?.
		void ClearDepthBuffer(GLclampx depth);
		void ClearColorBuffer(const FractionalColor & rgba);
		void ClearStencilBuffer(U32 value);

		U16 Width();
		U16 Height();
		U32 Pixels();

		void SetCurrentContext(Context * context);
		Context * GetCurrentContext();

		Config * GetConfig();

		U16 * GetColorBuffer();
		I32 * GetDepthBuffer();
		U32 * GetStencilBuffer();

		void Dispose();

		// Windows integration
		HDC GetMemoryDC();
		//HBITMAP GetBitmap();

	private:
		HDC		m_HDC;				// windows device context handle
		//HBITMAP	m_Bitmap;			// windows bitmap handle
		Config	m_Config;			// configuration arguments
		U16 *	m_AlphaBuffer;		// alpha buffer
		U16 *	m_ColorBuffer;		// pointer to frame buffer base address 5-6-5
		I32 *	m_DepthBuffer;		// pointer to Z-buffer base address
		U32 *	m_StencilBuffer;	// stencil buffer

		U16		m_Width;			// number of pixels in x direction
		U16		m_Height;			// number of pixels in y direction

		Context *	m_CurrentContext;
		bool	m_Disposed;			// the surface 
	};


	// --------------------------------------------------------------------------
	// Inline accessors
	// --------------------------------------------------------------------------


	inline Config * Surface :: GetConfig() {
		return &m_Config;
	}

	inline U32 Surface :: Pixels() {
		return m_Width * m_Height;
	}

	inline Context * Surface :: GetCurrentContext() {
		return m_CurrentContext;
	}

	inline HDC Surface :: GetMemoryDC() {
		return m_HDC;
	}

	/*
	inline HBITMAP Surface :: GetBitmap() {
		return m_Bitmap;
	}
	*/

	inline U16 * Surface :: GetColorBuffer() {
		return m_ColorBuffer;
	}

	inline I32 * Surface :: GetDepthBuffer() {
		return m_DepthBuffer;
	}

	inline U32 * Surface :: GetStencilBuffer() {
		return m_StencilBuffer;
	}

	inline U16 Surface :: Width() {
		return m_Width;
	}

	inline U16 Surface :: Height() {
		return m_Height;
	}
}

#endif // ndef EGL_SURFACE_H