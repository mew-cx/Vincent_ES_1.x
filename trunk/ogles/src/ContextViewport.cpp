// ==========================================================================
//
// viewport.cpp	Rendering Context Class for OpenGL (R) ES Implementation
//
//				Viewport Configuration
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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


using namespace EGL;


// --------------------------------------------------------------------------
// Viewport configuration
// --------------------------------------------------------------------------


void Context :: Viewport(GLint x, GLint y, GLsizei width, GLsizei height) { 
	// TODO: clamp to frame buffer limitations
	m_ViewportX = x;
	m_ViewportY = y;
	m_ViewportWidth = width;
	m_ViewportHeight = height;

	m_ViewportOrigin = Vec3D(EGL_FixedFromInt(x + (width / 2)), EGL_FixedFromInt(y + (height / 2)), m_ViewportOrigin.z());
	m_ViewportScale = Vec3D(EGL_FixedFromInt(width / 2), EGL_FixedFromInt(height / 2), m_ViewportScale.z());
}


void Context :: Scissor(GLint x, GLint y, GLsizei width, GLsizei height) { 

	GetRasterizerState()->SetScissor(x, y, width, height);
}

void Context :: DepthRangex(GLclampx zNear, GLclampx zFar) { 

	zNear = EGL_CLAMP(zNear, 0, EGL_ONE);
	zFar = EGL_CLAMP(zFar, 0, EGL_ONE);

	m_DepthRangeBase = (zNear + zFar) / 2;
	m_DepthRangeFactor = (zFar - zNear) / 2;
}

