#ifndef EGL_OGLES_H
#define EGL_OGLES_H 1

#pragma once


// ==========================================================================
//
// OGLES.h		Base include file for OpenGL|ES library
//
// --------------------------------------------------------------------------
//
// 10-02-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#ifdef OGLES_EXPORTS
#define OGLES_API __declspec(dllexport)
#else
#define OGLES_API __declspec(dllimport)
#endif


#ifdef EGL_USE_GPP
#	include <gpp.h>
#endif


// --------------------------------------------------------------------------
// type definitions
// --------------------------------------------------------------------------

#ifndef EGL_USE_GPP		// define only if not including Intel's GPP library

typedef signed char 	I8;
typedef unsigned char 	U8;
typedef short 			I16;
typedef unsigned short 	U16;
typedef int 			I32;
typedef unsigned int 	U32;

#endif

typedef unsigned __int64 U64;
typedef __int64			I64;


#define elementsof(a) (sizeof(a) / sizeof(*(a)))


#if 0
// This class is exported from the OGLES.dll
class OGLES_API COGLES {
public:
	COGLES(void);
	// TODO: add your methods here.
};

extern OGLES_API int nOGLES;

OGLES_API int fnOGLES(void);
#endif


#endif // ndef EGL_OGLES_H