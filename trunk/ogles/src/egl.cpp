// ==========================================================================
//
// gl.cpp	EGL Client API entry points
//
// --------------------------------------------------------------------------
//
// 08-04-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "GLES/egl.h"
#include "Context.h"
#include "Rasterizer.h"
#include "Config.h"
#include "Surface.h"
//#include "gx.h"


using namespace EGL;


// version numbers
#define EGL_VERSION_MAJOR 1
#define EGL_VERSION_MINOR 0


// Index into thread local storage for active EGL context
// This allocation needs to change when we move the code into a DLL
static DWORD s_TlsIndexError = TlsAlloc();


static void eglRecordError(EGLint error) 
	// Save an error code for the current thread
	//
	// error		-		The error code to be recorded in thread local storage
{
	TlsSetValue(s_TlsIndexError, reinterpret_cast<void *>(error));
}


static NativeDisplayType GetNativeDisplay (EGLDisplay display) {
	return reinterpret_cast<NativeDisplayType>(display);
}

GLAPI EGLint APIENTRY eglGetError (void) {
	return reinterpret_cast<EGLint> (TlsGetValue(s_TlsIndexError));
}

GLAPI EGLDisplay APIENTRY eglGetDisplay (NativeDisplayType display) {
	return reinterpret_cast<EGLDisplay>(display);
}

GLAPI EGLBoolean APIENTRY eglInitialize (EGLDisplay dpy, EGLint *major, EGLint *minor) {

	if (major != 0) {
		*major = EGL_VERSION_MAJOR;
	}

	if (minor != 0) {
		*minor = EGL_VERSION_MINOR;
	}

	return TRUE;
}

GLAPI EGLBoolean APIENTRY eglTerminate (EGLDisplay dpy) {
	return EGL_TRUE;
}

GLAPI const char * APIENTRY eglQueryString (EGLDisplay dpy, EGLint name) {
	return 0;
}

GLAPI void (* APIENTRY eglGetProcAddress (const char *procname))() {
	return 0;
}


GLAPI EGLBoolean APIENTRY eglGetConfigs (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
	return Config::GetConfigs(configs, config_size, num_config);
}

GLAPI EGLBoolean APIENTRY eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
	return Config::ChooseConfig(attrib_list, configs, config_size, num_config);
}

GLAPI EGLBoolean APIENTRY eglGetConfigAttrib (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
	*value = config->GetConfigAttrib(attribute);
	return EGL_TRUE;
}


GLAPI EGLSurface APIENTRY eglCreateWindowSurface (EGLDisplay dpy, EGLConfig config, NativeWindowType window, const EGLint *attrib_list) {
	RECT rect;

	GetClientRect(window, &rect);
	
	Config surfaceConfig(*config);
	surfaceConfig.SetConfigAttrib(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
	surfaceConfig.SetConfigAttrib(EGL_WIDTH, rect.right - rect.left);
	surfaceConfig.SetConfigAttrib(EGL_HEIGHT, rect.bottom - rect.top);
	return new EGL::Surface(surfaceConfig, GetWindowDC(window));
}

GLAPI EGLSurface APIENTRY eglCreatePixmapSurface (EGLDisplay dpy, EGLConfig config, NativePixmapType pixmap, const EGLint *attrib_list) {
	return 0;
}

GLAPI EGLSurface APIENTRY eglCreatePbufferSurface (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
	static EGLint validAttributes[] = {
		EGL_WIDTH,
		EGL_HEIGHT,
		EGL_NONE
	};

	Config surfaceConfig(*config, attrib_list, validAttributes);
	surfaceConfig.SetConfigAttrib(EGL_SURFACE_TYPE, EGL_PBUFFER_BIT);
	return new EGL::Surface(surfaceConfig, GetNativeDisplay(dpy));
}

GLAPI EGLBoolean APIENTRY eglDestroySurface (EGLDisplay dpy, EGLSurface surface) {
	surface->Dispose();
	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglQuerySurface (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
	*value = surface->GetConfig()->GetConfigAttrib(attribute);
	return EGL_TRUE;
}

GLAPI EGLContext APIENTRY eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_list, const EGLint *attrib_list) {
	return new Context(*config);
}

GLAPI EGLBoolean APIENTRY eglDestroyContext (EGLDisplay dpy, EGLContext ctx) {
	ctx->Dispose();
	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglMakeCurrent (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {

	Context::SetCurrentContext(ctx);
	ctx->SetDrawSurface(draw);
	ctx->SetReadSurface(read);

	return EGL_TRUE;

}

GLAPI EGLContext APIENTRY eglGetCurrentContext (void) {
	return Context::GetCurrentContext();
}

GLAPI EGLSurface APIENTRY eglGetCurrentSurface (EGLint readdraw) {
	EGLContext currentContext = eglGetCurrentContext();

	if (currentContext != 0) {
		switch (readdraw) {
		case EGL_DRAW:
			return currentContext->GetDrawSurface();

		case EGL_READ:
			return currentContext->GetReadSurface();

		default: 
			return 0;
		}
	} else {
		return 0;
	}
}

GLAPI EGLDisplay APIENTRY eglGetCurrentDisplay (void) {
	return 0;
}

GLAPI EGLBoolean APIENTRY eglQueryContext (EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
	*value = ctx->GetConfig()->GetConfigAttrib(attribute);
	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglWaitGL (void) {
	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglWaitNative (EGLint engine) {
	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglSwapBuffers (EGLDisplay dpy, EGLSurface draw) {

	Context::GetCurrentContext()->Flush();

	HDC nativeDisplay = GetNativeDisplay(dpy);
	HDC memoryDC = draw->GetMemoryDC();

	if (memoryDC == INVALID_HANDLE_VALUE) {
		memoryDC = CreateCompatibleDC(nativeDisplay);
	}

	// TO DO: Make this more efficient, either using hardware accelaration or
	// by using optimized memory copy code
	HBITMAP tempBitMap = CreateBitmap(draw->Width(), draw->Height(), 1,	16, draw->GetColorBuffer());
	SelectObject(memoryDC, tempBitMap);

	BitBlt(nativeDisplay, 0, 0, draw->Width(), draw->Height(), memoryDC, 0, 0, SRCCOPY);
	DeleteObject(tempBitMap);

	if (memoryDC != draw->GetMemoryDC()) {
		DeleteDC(memoryDC);
	}

	return EGL_TRUE;
}

GLAPI EGLBoolean APIENTRY eglCopyBuffers (EGLDisplay dpy, EGLSurface surface, NativePixmapType target) {
	Context::GetCurrentContext()->Flush();

	return EGL_TRUE;
}


