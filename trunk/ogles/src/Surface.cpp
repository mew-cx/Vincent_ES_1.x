// ==========================================================================
//
// Surface.cpp		Drawing Surface Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Surface.h"
#include "Color.h"
#include <string.h>


using namespace EGL;


Surface :: Surface(const Config & config, HDC hdc) 
:	m_Config(config),
	m_Width (config.GetConfigAttrib(EGL_WIDTH)), 
	m_Height (config.GetConfigAttrib(EGL_HEIGHT)),
//	m_Bitmap(reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE)),
	m_HDC(reinterpret_cast<HDC>(INVALID_HANDLE_VALUE))
{
	m_ColorBuffer = new U16[m_Width * m_Height];
	m_AlphaBuffer = new U8[m_Width * m_Height];
	m_DepthBuffer = new I32[m_Width * m_Height];
	m_StencilBuffer = new U32[m_Width * m_Height];

	if (hdc != INVALID_HANDLE_VALUE) {
		m_HDC = CreateCompatibleDC(hdc);
	}
}


Surface :: ~Surface() {

	/*
	if (m_Bitmap != INVALID_HANDLE_VALUE) {
		DeleteObject(m_Bitmap);
		m_Bitmap = reinterpret_cast<HBITMAP>(INVALID_HANDLE_VALUE);
	}*/

	if (m_HDC != INVALID_HANDLE_VALUE) {
		DeleteDC(m_HDC);
		m_HDC = reinterpret_cast<HDC>(INVALID_HANDLE_VALUE);
	}

	if (m_ColorBuffer != 0) {
		delete [] m_ColorBuffer;
		m_ColorBuffer = 0;
	}

	if (m_AlphaBuffer != 0) {
		delete [] m_AlphaBuffer;
		m_AlphaBuffer = 0;
	}

	if (m_DepthBuffer != 0) {
		delete[] m_DepthBuffer;
		m_DepthBuffer = 0;
	}

	if (m_StencilBuffer != 0) {
		delete[] m_StencilBuffer;
		m_StencilBuffer = 0;
	}
}


void Surface :: Dispose() {
	if (GetCurrentContext() != 0) {
		m_Disposed = true;
	} else {
		delete this;
	}
}


void Surface :: SetCurrentContext(Context * context) {
	m_CurrentContext = context;

	if (context == 0 && m_Disposed) {
		delete this;
	}
}


void Surface :: ClearDepthBuffer(GLclampx depth) {
	I32 depthValue = depth;//DepthBitsFromDepth(depth);

	int count = GetPixels();
	I32 * ptr = m_DepthBuffer;

	while (--count > 0) {
		*ptr++ = depthValue;
	}
}


void Surface :: ClearStencilBuffer(U32 value) {
	int count = GetPixels();
	U32 * ptr = m_StencilBuffer;

	while (--count > 0) {
		*ptr++ = value;
	}
}

/*
I32 Surface :: DepthBitsFromDepth(GLclampx depth) {
	I32 result;
	gppMul_16_32s(EGL_CLAMP(depth, EGL_FIXED_0, EGL_FIXED_1), 0xffffff, &result);
	return result;
}
*/

void Surface :: ClearColorBuffer(const Color & rgba) {
	U16 color = rgba.ConvertTo565();

	int count = GetPixels();
	U16 * ptr = m_ColorBuffer;

	while (--count > 0) {
		*ptr++ = color;
	}
}

