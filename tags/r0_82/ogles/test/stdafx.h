// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define EGL_ON_WINCE

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#ifndef _WIN32_WCE
#include <gdiplus.h>
#else
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <gx.h>
#endif