// ==========================================================================
//
// matrix.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Texturing Functions
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "context.h"
#include <string.h>
#include "fixed.h"


using namespace EGL;


void Context :: ActiveTexture(GLenum texture) { 
}

void Context :: BindTexture(GLenum target, GLuint texture) { 
}

void Context :: ClientActiveTexture(GLenum texture) { 
}

void Context :: CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { 
}

void Context :: CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { 
}

void Context :: CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { 
}

void Context :: CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { 
}

void Context :: DeleteTextures(GLsizei n, const GLuint *textures) { 
}

void Context :: GenTextures(GLsizei n, GLuint *textures) { 
}

void Context :: TexEnvx(GLenum target, GLenum pname, GLfixed param) { 
}

void Context :: TexEnvxv(GLenum target, GLenum pname, const GLfixed *params) { 
}

void Context :: TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { 

	// Let's start very simple: only one kind of 2d texture for level 0

	m_RasterParameters.m_texture[0].m_pTexBuffer = (U16 *) pixels;
	m_RasterParameters.m_texture[0].m_Width = width;
	m_RasterParameters.m_texture[0].m_Height = height;
}

void Context :: TexParameterx(GLenum target, GLenum pname, GLfixed param) { 
}

void Context :: TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { 
}

