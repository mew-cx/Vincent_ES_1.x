#ifndef GLES_CONFIG_H
#define GLES_CONFIG_H 1

/*
** ==========================================================================
**
** config.h			Configuration settings
**
** --------------------------------------------------------------------------
**
** 06-13-2005		Hans-Martin Will	initial version
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


#ifndef GLES_BUILD_NUMBER
#define GLES_BUILD_NUMBER "#.## Debug"
#endif

/*
** --------------------------------------------------------------------------
** Implementation Limits
** --------------------------------------------------------------------------
*/
#define GLES_MAX_MIPMAP_LEVELS	20		/* maximum number of mipmap levels	*/
#define GLES_MAX_TEXTURES		64		/* maximum number of textures		*/
#define GLES_MAX_VERTEX_ATTRIBS	16		/* maximum number of vertex attr.	*/
#define GLES_MAX_SHADERS		64		/* maximum number of shaders		*/
#define GLES_MAX_PROGRAMS		32		/* maximum number of programs		*/
#define GLES_MAX_BUFFERS		64		/* maximum number of vertex buffers	*/

#define GLES_MAX_VERTEX_UNIFORM_COMPONENTS		512	/* storage for uniforms	*/
#define GLES_MAX_FRAGMENT_UNIFORM_COMPONENTS	64	/* storage for uniforms	*/
#define GLES_MAX_VARYING_FLOATS					32	/* storage for varying	*/

#define GLES_VERTEX_ATTRIB_COMPONENTS			4	/* # components			*/

#define GLES_MAX_VIEWPORT_WIDTH		2048	/* maximum viewport dimensions	*/
#define GLES_MAX_VIEWPORT_HEIGHT	2048

#define GLES_MAX_TEXTURE_WIDTH		2048	/* maximum texture dimensions	*/
#define GLES_MAX_TEXTURE_HEIGHT		2048
#define GLES_MAX_TEXTURE_DEPTH		2048

#define GLES_MAX_STENCIL_BITS	8		/* maximum number of stencil bits	*/

/*
** --------------------------------------------------------------------------
** Internal Limits
** --------------------------------------------------------------------------
*/

#define GLES_MAX_VERTEX_QUEUE	12		/* size of processed vertices queue	*/
										/* should be multiple of 2 and 3	*/


/*
** --------------------------------------------------------------------------
** Version information
** --------------------------------------------------------------------------
*/
#define GLES_VENDOR				"Hans-Martin Will"
#define GLES_RENDERER			"Vincent " GLES_BUILD_NUMBER
#define GLES_VERSION			"2.0"
#define GLES_SHADING_LANGUAGE	"1.10.10"
#define GLES_EXTENSIONS			"OES_read_format OES_compressed_paletted_texture OES_framebuffer_object"

#endif /* ndef GLES_CONFIG_H */
