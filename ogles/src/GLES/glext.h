#ifndef __glext_h_
#define __glext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright 1992-2003, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2004, Hans-Martin Will. All rights reserved.
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
*/

#ifndef WIN32
#define APIENTRY
#endif

#define GL_OES_querymatrix		    1

/* OES_query_matrix */
GLbitfield APIENTRY glQueryMatrixxOES(GLfixed *mantissa, GLint *exponent);

/****************************************************************************/
/* Extension: SGIS_generate_mipmap											*/
/****************************************************************************/


#define GL_GENERATE_MIPMAP_SGIS			0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS	0x8192


/****************************************************************************/
/* Extension: HM_render_mesh												*/
/****************************************************************************/

GLAPI void APIENTRY glDrawMeshHM(GLsizei count, GLenum type, GLsizei stride,
								 GLsizei offsetVertex, GLsizei strideVertex,
								 GLsizei offsetNormal, GLsizei strideNormal,
								 GLsizei offsetTexture, GLsizei strideTexture,
								 GLsizei offsetColor, GLsizei strideColor, 
								 const GLvoid *pointer);

#ifdef __cplusplus
}
#endif

#endif /* __glext_h_ */
