// ==========================================================================
//
// points.cpp	Rendering Context Class for 3D Rendering Library
//
//				Rendering Operations for Points
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
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


#include "stdafx.h"
#include "Context.h"
#include "fixed.h"
#include "Rasterizer.h"

using namespace EGL;


void Context :: PointSizex(GLfixed size) { 
	GetRasterizerState()->SetPointSize(size);
}


void Context :: RenderPoints(GLint first, GLsizei count) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		RasterPos pos0;
		SelectArrayElement(first++);
		CurrentValuesToRasterPos(&pos0);
		RenderPoint(pos0);
	}
}


void Context :: RenderPoints(GLsizei count, const GLubyte * indices) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		RasterPos pos0;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderPoint(pos0);
	}
}


void Context :: RenderPoints(GLsizei count, const GLushort * indices) {

	m_Rasterizer->PreparePoint();

	while (count >= 1) {
		count -= 1;

		RasterPos pos0;
		SelectArrayElement(*indices++);
		CurrentValuesToRasterPos(&pos0);
		RenderPoint(pos0);
	}
}


void Context :: RenderPoint(RasterPos& point) {

	// clip at frustrum
	// in principle, the scissor test can be included in here
	if (point.m_ClipCoords.x() < -point.m_ClipCoords.w() ||
		point.m_ClipCoords.x() >  point.m_ClipCoords.w() ||
		point.m_ClipCoords.y() < -point.m_ClipCoords.w() ||
		point.m_ClipCoords.y() >  point.m_ClipCoords.w() ||
		point.m_ClipCoords.z() < -point.m_ClipCoords.w() ||
		point.m_ClipCoords.z() >  point.m_ClipCoords.w()) 
		return;

	ClipCoordsToWindowCoords(point);
	m_Rasterizer->RasterPoint(point);
}


