// ==========================================================================
//
// viewport.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Viewport Configuration
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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

	GetRasterizerState()->SetDepthRange(zNear, zFar);
}

