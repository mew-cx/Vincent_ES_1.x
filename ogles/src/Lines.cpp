// ==========================================================================
//
// lines.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Rendering Operations for Lines
//
// --------------------------------------------------------------------------
//
// 08-12-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include <string.h>
#include "Context.h"

using namespace EGL;


void Context :: LineWidthx(GLfixed width) { 
	GetRasterizerState()->SetLineWidth(width);
}


// --------------------------------------------------------------------------
// Lines
// --------------------------------------------------------------------------


void Context :: RenderLines(GLint first, GLsizei count) {
}


void Context :: RenderLines(GLsizei count, const GLubyte * indices) {
}


void Context :: RenderLines(GLsizei count, const GLushort * indices) {
}


// --------------------------------------------------------------------------
// Line Strips
// --------------------------------------------------------------------------


void Context :: RenderLineStrip(GLint first, GLsizei count) {
}


void Context :: RenderLineStrip(GLsizei count, const GLubyte * indices) {
}


void Context :: RenderLineStrip(GLsizei count, const GLushort * indices) {
}


// --------------------------------------------------------------------------
// Line Loops
// --------------------------------------------------------------------------


void Context :: RenderLineLoop(GLint first, GLsizei count) {
}


void Context :: RenderLineLoop(GLsizei count, const GLubyte * indices) {
}


void Context :: RenderLineLoop(GLsizei count, const GLushort * indices) {
}

