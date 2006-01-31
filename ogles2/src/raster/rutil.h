#ifndef GLES_RASTER_RUTIL_H
#define GLES_RASTER_RUTIL_H 1

/*
** ==========================================================================
**
** rutil.h			Rasterizer Utility Declarations
**
** --------------------------------------------------------------------------
**
** 11-28-2005		Hans-Martin Will	initial version
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


#include "platform/platform.h"


/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

/* 1.0f */
#define GLES_ONE			1.0f

/* 1.0f - FLT_EPSILON */
#define GLES_ONE_MINUS_EPS	(1.0f - FLT_EPSILON)

/* 0.5f */
#define GLES_HALF			0.5f

/* 0.5f - FLT_EPSILON/2 */
#define GLES_HALF_MINUS_EPS	(0.5f * (1.0f - FLT_EPSILON))


#endif /* ndef GLES_RASTER_RUTIL_H */
