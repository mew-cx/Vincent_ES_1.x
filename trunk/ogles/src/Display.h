#ifndef EGL_DISPLAY_H
#define EGL_DISPLAY_H 1

#pragma once

// ==========================================================================
//
// Display.h		Display Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"
#include "GLES/egl.h"


namespace EGL {
	class OGLES_API Display {
	public:


	private:
		NativeDisplayType		m_NativeDisplay;
	};
}



#endif // ndef EGL_DISPLAY_H