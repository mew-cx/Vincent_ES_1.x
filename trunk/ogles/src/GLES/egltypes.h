#ifndef __egl_types_h_
#define __egl_types_h_

/*
** egltypes.h is platform dependent. It defines:
**
**     - EGL types and resources
**     - Native types
**     - EGL and native handle values
**
** EGL types and resources are to be typedef'ed with appropriate platform
** dependent resource handle types. EGLint must be an integer of at least
** 32-bit.
**
** NativeDisplayType, NativeWindowType and NativePixmapType are to be
** replaced with corresponding types of the native window system in egl.h.
**
** EGL and native handle values must match their types.
*/


//#include <sys/types.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>


class Context;
class Config;
class Surface;


typedef DWORD	int32_t;
typedef HDC		NativeDisplayType;
typedef HWND	NativeWindowType;
typedef HBITMAP NativePixmapType;


/*
** Types and resources
*/
typedef int				EGLBoolean;
typedef int32_t			EGLint;
typedef void *			EGLDisplay;
typedef Config *		EGLConfig;
typedef Surface *		EGLSurface;
typedef Context  *		EGLContext;

/*
** EGL and native handle values
*/
#define EGL_DEFAULT_DISPLAY ((NativeDisplayType)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_SURFACE ((EGLSurface)0)


#endif //ndef __egl_types_h_