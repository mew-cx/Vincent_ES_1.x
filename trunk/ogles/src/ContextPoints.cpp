// ==========================================================================
//
// points.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Rendering Operations for Points
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "context.h"
#include <string.h>
#include "fixed.h"


using namespace EGL;


void Context :: PointSizex(GLfixed size) { 
	GetRasterizerState()->SetPointSize(size);
}


void Context :: RenderPoints(GLint first, GLsizei count) {
}


void Context :: RenderPoints(GLsizei count, const GLubyte * indices) {
}


void Context :: RenderPoints(GLsizei count, const GLushort * indices) {
}

