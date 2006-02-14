#ifndef GLES_PLATFORM_PLATFORM_H
#define GLES_PLATFORM_PLATFORM_H 1

/*
** ==========================================================================
**
** platform.h		Bindings to underlying OS platform
**
** --------------------------------------------------------------------------
**
** 10-21-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/


#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#define GLES_ASSERT(c) assert(c)

#ifdef _MSC_VER
#	define GLES_INLINE	__inline
#else /* GNU CC - what's the identifier? */
#	define GLES_INLINE	__inline__
#endif

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

#define GLES_NIL ((const void *) 4)		/* platform dependent invalid		*/
										/* pointer != NULL					*/

#define GLES_UBYTE_MAX	255

/*
** --------------------------------------------------------------------------
** Macros
** --------------------------------------------------------------------------
*/

#define GLES_ELEMENTSOF(a)	(sizeof(a) / sizeof((a)[0]))

/*
** --------------------------------------------------------------------------
** Data Types
** --------------------------------------------------------------------------
*/


/*
** --------------------------------------------------------------------------
** Inline functions
** --------------------------------------------------------------------------
*/

GLclampf GLES_INLINE GlesClampf(GLfloat value) {
	return value > 1.0f ? 1.0f : value < 0.0f ? 0.0f : value;
}

GLfloat GLES_INLINE GlesFloatx(GLfixed value) {
	return (GLfloat) ldexp((double) value, -16);
}

/*
** --------------------------------------------------------------------------
** Memory Functions
** --------------------------------------------------------------------------
*/

void GLES_MEMSET(void * ptr, GLubyte value, GLsizeiptr size);
void GLES_MEMCPY(void * dst, const void * src, GLsizeiptr size);
void GLES_MEMMOVE(void * dst, const void * src, GLsizeiptr size);

/*
** --------------------------------------------------------------------------
** Memory Management
** --------------------------------------------------------------------------
*/

void * GlesMalloc(GLsizeiptr size);
void GlesFree(void * ptr);

#endif /* ndef GLES_PLATFORM_PLATFORM_H */