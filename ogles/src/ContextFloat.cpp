// ==========================================================================
//
// ContextFloat.cpp	Rendering Context Class for 3D Rendering Library
//
//					Emulation of EGL Floating Point Primitives
//
// --------------------------------------------------------------------------
//
// 08-02-2003	Hans-Martin Will	initial version
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


using namespace EGL;


void Context :: AlphaFunc (GLenum func, GLclampf ref) {
	AlphaFuncx(func, EGL_FixedFromFloat(ref));
}

void Context :: ClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	ClearColorx(EGL_FixedFromFloat(red), EGL_FixedFromFloat(green), 
		EGL_FixedFromFloat(blue), EGL_FixedFromFloat(alpha));
}

void Context :: ClearDepthf (GLclampf depth) {
	ClearDepthx(EGL_FixedFromFloat(depth));
}

void Context :: Color4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	Color4x(EGL_FixedFromFloat(red), EGL_FixedFromFloat(green), 
		EGL_FixedFromFloat(blue), EGL_FixedFromFloat(alpha));
}

void Context :: DepthRangef (GLclampf zNear, GLclampf zFar) {
	DepthRangex(EGL_FixedFromFloat(zNear), EGL_FixedFromFloat(zFar));
}

void Context :: Fogf (GLenum pname, GLfloat param) {
	Fogx(pname, EGL_FixedFromFloat(param));
}

void Context :: Fogfv (GLenum pname, const GLfloat *params) {
	switch (pname) {
	case GL_FOG_MODE:
	case GL_FOG_DENSITY:
	case GL_FOG_START:
	case GL_FOG_END:
	//case GL_FOG_INDEX:
		GLfixed param;
		param = EGL_FixedFromFloat(*params);
		Fogxv(pname, &param);
		break;

	case GL_FOG_COLOR:
		GLfixed fixed_params[4];

		for (int index = 0; index < 4; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		Fogxv(pname, fixed_params);
		break;
	}
}

void Context :: Frustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
	Frustumx(EGL_FixedFromFloat(left), EGL_FixedFromFloat(right),
		EGL_FixedFromFloat(bottom), EGL_FixedFromFloat(top), 
		EGL_FixedFromFloat(zNear), EGL_FixedFromFloat(zFar));
}

void Context :: LightModelf (GLenum pname, GLfloat param) {
	LightModelx(pname, EGL_FixedFromFloat(param));
}

void Context :: LightModelfv (GLenum pname, const GLfloat *params) {

	int index;

	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		GLfixed fixed_params[4];

		for (index = 0; index < 4; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		LightModelxv(pname, fixed_params);
		break;

	//case GL_LIGHT_MODEL_LOCAL_VIEWER:
	case GL_LIGHT_MODEL_TWO_SIDE:
	//case GL_LIGHT_MODEL_COLOR_CONTROL:
		GLfixed param;
		param = EGL_FixedFromFloat(*params);
		LightModelxv(pname, &param);
		break;
	}
}

void Context :: Lightf (GLenum light, GLenum pname, GLfloat param) {
	Lightx(light, pname, EGL_FixedFromFloat(param));
}

void Context :: Lightfv (GLenum light, GLenum pname, const GLfloat *params) {
	//void Context :: Lightxv (GLenum light, GLenum pname, const GLfixed *params);
	GLfixed fixed_params[4];
	int index;

	switch (pname) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
		for (index = 0; index < 4; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		Lightxv(light, pname, fixed_params);
		break;

	case GL_SHININESS:
		fixed_params[0] = EGL_FixedFromFloat(params[0]);
		Lightxv(light, pname, fixed_params);
		break;

	/*
	case GL_COLOR_INDEXES:
		for (index = 0; index < 3; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		glLightxv(light, pname, fixed_params);
		break;
	*/
	}
}

void Context :: LineWidth (GLfloat width) {
	LineWidthx(EGL_FixedFromFloat(width));
}

void Context :: LoadMatrixf (const GLfloat *m) {
	GLfixed mx[16];

	for (int index = 0; index < 16; ++index) {
		mx[index] = EGL_FixedFromFloat(m[index]);
	}

	LoadMatrixx(mx);
}

void Context :: Materialf (GLenum face, GLenum pname, GLfloat param) {
	Materialx(face, pname, EGL_FixedFromFloat(param));
}

void Context :: Materialfv (GLenum face, GLenum pname, const GLfloat *params) {
	GLfixed fixed_params[4];
	int index;

	switch (pname) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_AMBIENT_AND_DIFFUSE:
	case GL_SPECULAR:
	case GL_EMISSION:
		for (index = 0; index < 4; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		Materialxv(face, pname, fixed_params);
		break;

	case GL_SHININESS:
		fixed_params[0] = EGL_FixedFromFloat(params[0]);
		Materialxv(face, pname, fixed_params);
		break;

	/*
	case GL_COLOR_INDEXES:
		for (index = 0; index < 3; ++index) {
			fixed_params[index] = EGL_FixedFromFloat(params[index]);
		}

		glMaterialxv(light, pname, fixed_params);
		break;
	*/
	}
}

void Context :: MultMatrixf (const GLfloat *m) {
	GLfixed mx[16];

	for (int index = 0; index < 16; ++index) {
		mx[index] = EGL_FixedFromFloat(m[index]);
	}

	MultMatrixx(mx);
}

void Context :: MultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
	MultiTexCoord4x(target, 
		EGL_FixedFromFloat(s), EGL_FixedFromFloat(t),
		EGL_FixedFromFloat(r), EGL_FixedFromFloat(q));
}

void Context :: Normal3f (GLfloat nx, GLfloat ny, GLfloat nz) {
	Normal3x(EGL_FixedFromFloat(nx), EGL_FixedFromFloat(ny), EGL_FixedFromFloat(nz));
}

void Context :: Orthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
	Orthox(EGL_FixedFromFloat(left), EGL_FixedFromFloat(right),
		EGL_FixedFromFloat(bottom), EGL_FixedFromFloat(top),
		EGL_FixedFromFloat(zNear), EGL_FixedFromFloat(zFar));
}

void Context :: PointSize (GLfloat size) {
	PointSizex(EGL_FixedFromFloat(size));
}

void Context :: PolygonOffset (GLfloat factor, GLfloat units) {
	PolygonOffsetx(EGL_FixedFromFloat(factor), EGL_FixedFromFloat(units));
}

void Context :: Rotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	Rotatex(EGL_FixedFromFloat(angle), EGL_FixedFromFloat(x),
		EGL_FixedFromFloat(y), EGL_FixedFromFloat(z));
}

void Context :: SampleCoverage (GLclampf value, GLboolean invert) {
	SampleCoveragex(EGL_FixedFromFloat(value), invert);
}

void Context :: Scalef (GLfloat x, GLfloat y, GLfloat z) {
	Scalex(EGL_FixedFromFloat(x), EGL_FixedFromFloat(y), EGL_FixedFromFloat(z));
}

void Context :: TexEnvf (GLenum target, GLenum pname, GLfloat param) {
	TexEnvx(target, pname, EGL_FixedFromFloat(param));
}

void Context :: TexEnvfv (GLenum target, GLenum pname, const GLfloat *params) {
	//void Context :: TexEnvxv (GLenum target, GLenum pname, const GLfixed *params);
}


void Context :: TexParameterf (GLenum target, GLenum pname, GLfloat param) {
	TexParameterx(target, pname, EGL_FixedFromFloat(param));
}

void Context :: Translatef (GLfloat x, GLfloat y, GLfloat z) {
	Translatex(EGL_FixedFromFloat(x), EGL_FixedFromFloat(y), EGL_FixedFromFloat(z));
}
