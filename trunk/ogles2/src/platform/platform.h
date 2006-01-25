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
** Copyright (c) 2005, Hans-Martin Will. All rights reserved.
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

#define GLES_ASSERT(c) assert(c)


/*
** --------------------------------------------------------------------------
** Data Types
** --------------------------------------------------------------------------
*/

typedef void *	OGLESgpuaddr;		/* Memory in (virtual) CPU		*/

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

void * MallocClient(GLsizeiptr size);
void FreeClient(void * ptr);

OGLESgpuaddr MallocServer(GLsizeiptr size);
void FreeServer(OGLESgpuaddr ptr);


#endif /* ndef GLES_PLATFORM_PLATFORM_H */