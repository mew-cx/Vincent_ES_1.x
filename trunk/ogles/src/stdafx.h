// ==========================================================================
//
// stdafx.h			Precompiled headers for 3D Rendering Library
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


#if defined(__SYMBIAN32__)
#	define EGL_ON_SYMBIAN
#elif defined(WIN32) || defined(_WIN32_WCE)
#	define EGL_ON_WINCE
#else
#	error "Unsupported Operating System"
#endif


// --------------------------------------------------------------------------
// Windows Header Files:
// --------------------------------------------------------------------------

#ifdef EGL_ON_WINCE
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#	pragma warning (disable:4786)
#	pragma warning (disable:4244)

#	include <windows.h>
#	include <Cmnintrin.h>

#endif //ndef EGL_ON_WINCE

// --------------------------------------------------------------------------
// Symbian Header Files:
// --------------------------------------------------------------------------

#ifdef EGL_ON_SYMBIAN

#	include <e32def.h>

#endif // ndef EGL_ON_SYMBIAN

// --------------------------------------------------------------------------
// Standard Library Files
// --------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

// --------------------------------------------------------------------------
// Intel Header Files
// --------------------------------------------------------------------------

#ifdef EGL_USE_GPP
#	include <gpp.h>
#endif


