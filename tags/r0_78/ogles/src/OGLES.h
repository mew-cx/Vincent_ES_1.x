#ifndef EGL_OGLES_H
#define EGL_OGLES_H 1

#pragma once


// ==========================================================================
//
// OGLES.h		Base include file for OpenGL (R) ES library
//
// --------------------------------------------------------------------------
//
// 10-02-2003	Hans-Martin Will	initial version
// 12-17-2003	Hans-Martin Will	added configuration information
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


#ifdef OGLES_EXPORTS
#	define OGLES_API __declspec(dllexport)
#else
#	define OGLES_API __declspec(dllimport)
#endif


#pragma warning (disable:4786)


#ifdef EGL_USE_GPP
#	include <gpp.h>
#endif


#define EGL_CONFIG_VENDOR			"Hans-Martin Will"
#define EGL_CONFIG_VERSION			"OpenGL ES-CL 1.0"
#define EGL_CONFIG_EXTENSIONS		"GL_OES_fixed_point "\
									"GL_OES_single_precision "\
									"GL_OES_read_format "\
									"GL_OES_query_matrix "\
									"GL_OES_compressed_paletted_texture "\
									"GL_SGIS_generate_mipmap"

#ifdef EGL_USE_GPP
#	define EGL_CONFIG_RENDERER		"Software GPP"
#else
#	define EGL_CONFIG_RENDERER		"Software"
#endif

#define EGL_VERSION_NUMBER			"0.77 " EGL_CONFIG_RENDERER


// --------------------------------------------------------------------------
// type definitions
// --------------------------------------------------------------------------

#ifndef EGL_USE_GPP		// define only if not including Intel's GPP library

typedef signed char 		I8;
typedef unsigned char 		U8;
typedef short 				I16;
typedef unsigned short 		U16;
typedef int 				I32;
typedef unsigned int 		U32;

#endif

typedef unsigned __int64	U64;
typedef __int64				I64;


// --------------------------------------------------------------------------
// Memory helper macros
// --------------------------------------------------------------------------

// number of elements of an array
#define elementsof(a) (sizeof(a) / sizeof(*(a)))

// offset in bytes of a structure member from the base address
#ifndef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

// --------------------------------------------------------------------------
// Common variables
// --------------------------------------------------------------------------

extern DWORD s_TlsIndexContext;	// thread local storage slot for current context
extern DWORD s_TlsIndexError;	// thread local storage slot for EGL error


#endif // ndef EGL_OGLES_H