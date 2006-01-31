/*
** ==========================================================================
**
** points.c			Point rendering functions
**
** --------------------------------------------------------------------------
**
** 06-14-2005		Hans-Martin Will	initial version
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "render/render.h"
#include "raster/raster.h"


void GlesPreparePoints(State * state) {
}

void GlesRenderPoint(State * state, const Vertex * p1) {
	RasterVertex a;

	if (p1->v.position.x < -p1->v.position.w ||
		p1->v.position.x >  p1->v.position.w ||
		p1->v.position.y < -p1->v.position.w ||
		p1->v.position.y >  p1->v.position.w ||
		p1->v.position.z < -p1->v.position.w ||
		p1->v.position.z >  p1->v.position.w) 
		return;

	GlesViewportTransform(state, &a, p1);
#if 0
	point.m_Color = point.m_FrontColor;

	if (m_PointSizeAttenuate) {
		EGL_Fixed eyeDistance = EGL_Abs(point.m_EyeCoords.z());

		EGL_Fixed factor =
			EGL_InvSqrt(m_PointDistanceAttenuation[0] +
						EGL_Mul(m_PointDistanceAttenuation[1], eyeDistance) +
						EGL_Mul(m_PointDistanceAttenuation[2], EGL_Mul(eyeDistance, eyeDistance)));

		size = EGL_Mul(size, factor);
	}

	// as long as we do not have anti-aliasing, determining the effective point size here is fine
	EGL_Fixed pointSize = EGL_Max(size, EGL_ONE);
#endif

	GlesRasterPoint(state, &a);
}

void GlesRenderPoints(State * state, GLint first, GLsizei count, GLenum type, const void * indices) {

	Vertex pos;
	GlesPreparePoints(state);

	while (count >= 1) {
		GlesSelectArrayElement(state, NEXT_INDEX);
		GlesProcessVertex(state, &pos);
		GlesRenderPoint(state, &pos);
		--count;
	}
}

