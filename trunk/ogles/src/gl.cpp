// ==========================================================================
//
// gl.cpp	EGL Client API entry points
//
// --------------------------------------------------------------------------
//
// 08-04-2003	Hans-Martin Will	initial version
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
#include "GLES/gl.h"
#include "Context.h"


using namespace EGL;


GLAPI void APIENTRY glActiveTexture (GLenum texture) { 
	Context::DefaultContext()->ActiveTexture(texture);
}

GLAPI void APIENTRY glAlphaFunc (GLenum func, GLclampf ref) { 
	Context::DefaultContext()->AlphaFunc(func, ref);
}

GLAPI void APIENTRY glAlphaFuncx (GLenum func, GLclampx ref) { 
	Context::DefaultContext()->AlphaFuncx(func, ref);
}

GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture) { 
	Context::DefaultContext()->BindTexture(target, texture);
}

GLAPI void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) { 
	Context::DefaultContext()->BlendFunc(sfactor, dfactor);
}

GLAPI void APIENTRY glClear (GLbitfield mask) { 
	Context::DefaultContext()->Clear(mask);
}

GLAPI void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { 
	Context::DefaultContext()->ClearColor(red, green, blue, alpha);
}

GLAPI void APIENTRY glClearColorx (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) { 
	Context::DefaultContext()->ClearColorx(red, green, blue, alpha);
}

GLAPI void APIENTRY glClearDepthf (GLclampf depth) {
	Context::DefaultContext()->ClearDepthf(depth);
}

GLAPI void APIENTRY glClearDepthx (GLclampx depth) { 
	Context::DefaultContext()->ClearDepthx(depth);
}

GLAPI void APIENTRY glClearStencil (GLint s) { 
	Context::DefaultContext()->ClearStencil(s);
}

GLAPI void APIENTRY glClientActiveTexture (GLenum texture) { 
	Context::DefaultContext()->ClientActiveTexture(texture);
}

GLAPI void APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { 
	Context::DefaultContext()->Color4f(red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4x (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) { 
	Context::DefaultContext()->Color4x(red, green, blue, alpha);
}

GLAPI void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { 
	Context::DefaultContext()->ColorMask(red, green , blue, alpha);
}

GLAPI void APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { 
	Context::DefaultContext()->ColorPointer(size, type, stride, pointer);
}

GLAPI void APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { 
	Context::DefaultContext()->CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { 
	Context::DefaultContext()->CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLAPI void APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { 
	Context::DefaultContext()->CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

GLAPI void APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { 
	Context::DefaultContext()->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

GLAPI void APIENTRY glCullFace (GLenum mode) { 
	Context::DefaultContext()->CullFace(mode);
}

GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) { 
	Context::DefaultContext()->DeleteTextures(n, textures);
}

GLAPI void APIENTRY glDepthFunc (GLenum func) { 
	Context::DefaultContext()->DepthFunc(func);
}

GLAPI void APIENTRY glDepthMask (GLboolean flag) { 
	Context::DefaultContext()->DepthMask(flag);
}

GLAPI void APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) { 
	Context::DefaultContext()->DepthRangef(zNear, zFar);
}

GLAPI void APIENTRY glDepthRangex (GLclampx zNear, GLclampx zFar) { 
	Context::DefaultContext()->DepthRangex(zNear, zFar);
}

GLAPI void APIENTRY glDisable (GLenum cap) { 
	Context::DefaultContext()->Disable(cap);
}

GLAPI void APIENTRY glDisableClientState (GLenum array) { 
	Context::DefaultContext()->DisableClientState(array);
}

GLAPI void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) { 
	Context::DefaultContext()->DrawArrays(mode, first, count);
}

GLAPI void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) { 
	Context::DefaultContext()->DrawElements(mode, count, type, indices);
}

GLAPI void APIENTRY glEnable (GLenum cap) { 
	Context::DefaultContext()->Enable(cap);
}

GLAPI void APIENTRY glEnableClientState (GLenum array) { 
	Context::DefaultContext()->EnableClientState(array);
}

GLAPI void APIENTRY glFinish (void) { 
	Context::DefaultContext()->Finish();
}

GLAPI void APIENTRY glFlush (void) { 
	Context::DefaultContext()->Flush();
}

GLAPI void APIENTRY glFogf (GLenum pname, GLfloat param) { 
	Context::DefaultContext()->Fogf(pname, param);
}

GLAPI void APIENTRY glFogfv (GLenum pname, const GLfloat *params) { 
	Context::DefaultContext()->Fogfv(pname, params);
}

GLAPI void APIENTRY glFogx (GLenum pname, GLfixed param) { 
	Context::DefaultContext()->Fogx(pname, param);
}

GLAPI void APIENTRY glFogxv (GLenum pname, const GLfixed *params) { 
	Context::DefaultContext()->Fogxv(pname, params);
}

GLAPI void APIENTRY glFrontFace (GLenum mode) { 
	Context::DefaultContext()->FrontFace(mode);
}

GLAPI void APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) { 
	Context::DefaultContext()->Frustumf(left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) { 
	Context::DefaultContext()->Frustumx(left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures) { 
	Context::DefaultContext()->GenTextures(n, textures);
}

GLAPI GLenum APIENTRY glGetError (void) { 
	return Context::DefaultContext()->GetError();
}

GLAPI void APIENTRY glGetIntegerv (GLenum pname, GLint *params) { 
	Context::DefaultContext()->GetIntegerv(pname, params);
}

GLAPI const GLubyte * APIENTRY glGetString (GLenum name) { 
	return Context::DefaultContext()->GetString(name);
}

GLAPI void APIENTRY glHint (GLenum target, GLenum mode) { 
	Context::DefaultContext()->Hint(target, mode);
}

GLAPI void APIENTRY glLightModelf (GLenum pname, GLfloat param) { 
	Context::DefaultContext()->LightModelf(pname, param);
}

GLAPI void APIENTRY glLightModelfv (GLenum pname, const GLfloat *params) { 
	Context::DefaultContext()->LightModelfv(pname, params);
}

GLAPI void APIENTRY glLightModelx (GLenum pname, GLfixed param) { 
	Context::DefaultContext()->LightModelx(pname, param);
}

GLAPI void APIENTRY glLightModelxv (GLenum pname, const GLfixed *params) { 
	Context::DefaultContext()->LightModelxv(pname, params);
}

GLAPI void APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param) { 
	Context::DefaultContext()->Lightf(light, pname, param);
}

GLAPI void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params) { 
	Context::DefaultContext()->Lightfv(light, pname, params);
}

GLAPI void APIENTRY glLightx (GLenum light, GLenum pname, GLfixed param) { 
	Context::DefaultContext()->Lightx(light, pname, param);
}

GLAPI void APIENTRY glLightxv (GLenum light, GLenum pname, const GLfixed *params) { 
	Context::DefaultContext()->Lightxv(light, pname, params);
}

GLAPI void APIENTRY glLineWidth (GLfloat width) { 
	Context::DefaultContext()->LineWidth(width);
}

GLAPI void APIENTRY glLineWidthx (GLfixed width) { 
	Context::DefaultContext()->LineWidthx(width);
}

GLAPI void APIENTRY glLoadIdentity (void) { 
	Context::DefaultContext()->LoadIdentity();
}

GLAPI void APIENTRY glLoadMatrixf (const GLfloat *m) { 
	Context::DefaultContext()->LoadMatrixf(m);
}

GLAPI void APIENTRY glLoadMatrixx (const GLfixed *m) { 
	Context::DefaultContext()->LoadMatrixx(m);
}

GLAPI void APIENTRY glLogicOp (GLenum opcode) { 
	Context::DefaultContext()->LogicOp(opcode);
}

GLAPI void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param) { 
	Context::DefaultContext()->Materialf(face, pname, param);
}

GLAPI void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params) { 
	Context::DefaultContext()->Materialfv(face, pname, params);
}

GLAPI void APIENTRY glMaterialx (GLenum face, GLenum pname, GLfixed param) { 
	Context::DefaultContext()->Materialx(face, pname, param);
}

GLAPI void APIENTRY glMaterialxv (GLenum face, GLenum pname, const GLfixed *params) { 
	Context::DefaultContext()->Materialxv(face, pname, params);
}

GLAPI void APIENTRY glMatrixMode (GLenum mode) { 
	Context::DefaultContext()->MatrixMode(mode);
}

GLAPI void APIENTRY glMultMatrixf (const GLfloat *m) { 
	Context::DefaultContext()->MultMatrixf(m);
}

GLAPI void APIENTRY glMultMatrixx (const GLfixed *m) { 
	Context::DefaultContext()->MultMatrixx(m);
}

GLAPI void APIENTRY glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) { 
	Context::DefaultContext()->MultiTexCoord4f(target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4x (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) { 
	Context::DefaultContext()->MultiTexCoord4x(target, s, t, r, q);
}

GLAPI void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz) { 
	Context::DefaultContext()->Normal3f(nx, ny, nz);
}

GLAPI void APIENTRY glNormal3x (GLfixed nx, GLfixed ny, GLfixed nz) { 
	Context::DefaultContext()->Normal3x(nx, ny, nz);
}

GLAPI void APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer) { 
	Context::DefaultContext()->NormalPointer(type, stride, pointer);
}

GLAPI void APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) { 
	Context::DefaultContext()->Orthof(left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) { 
	Context::DefaultContext()->Orthox(left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY glPixelStorei (GLenum pname, GLint param) { 
	Context::DefaultContext()->PixelStorei(pname, param);
}

GLAPI void APIENTRY glPointSize (GLfloat size) { 
	Context::DefaultContext()->PointSize(size);
}

GLAPI void APIENTRY glPointSizex (GLfixed size) { 
	Context::DefaultContext()->PointSizex(size);
}

GLAPI void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) { 
	Context::DefaultContext()->PolygonOffset(factor, units);
}

GLAPI void APIENTRY glPolygonOffsetx (GLfixed factor, GLfixed units) { 
	Context::DefaultContext()->PolygonOffsetx(factor, units);
}

GLAPI void APIENTRY glPopMatrix (void) { 
	Context::DefaultContext()->PopMatrix();
}

GLAPI void APIENTRY glPushMatrix (void) { 
	Context::DefaultContext()->PushMatrix();
}

GLAPI void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) { 
	Context::DefaultContext()->ReadPixels(x, y, width, height, format, type, pixels);
}

GLAPI void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) { 
	Context::DefaultContext()->Rotatef(angle, x, y, z);
}

GLAPI void APIENTRY glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z) { 
	Context::DefaultContext()->Rotatex(angle, x, y, z);
}

GLAPI void APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) { 
	Context::DefaultContext()->SampleCoverage(value, invert);
}

GLAPI void APIENTRY glSampleCoveragex (GLclampx value, GLboolean invert) { 
	Context::DefaultContext()->SampleCoveragex(value, invert);
}

GLAPI void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z) { 
	Context::DefaultContext()->Scalef(x, y, z);
}

GLAPI void APIENTRY glScalex (GLfixed x, GLfixed y, GLfixed z) { 
	Context::DefaultContext()->Scalex(x, y, z);
}

GLAPI void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) { 
	Context::DefaultContext()->Scissor(x, y, width, height);
}

GLAPI void APIENTRY glShadeModel (GLenum mode) { 
	Context::DefaultContext()->ShadeModel(mode);
}

GLAPI void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) { 
	Context::DefaultContext()->StencilFunc(func, ref, mask);
}

GLAPI void APIENTRY glStencilMask (GLuint mask) { 
	Context::DefaultContext()->StencilMask(mask);
}

GLAPI void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) { 
	Context::DefaultContext()->StencilOp(fail, zfail, zpass);
}

GLAPI void APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { 
	Context::DefaultContext()->TexCoordPointer(size, type, stride, pointer);
}

GLAPI void APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param) { 
	Context::DefaultContext()->TexEnvf(target, pname, param);
}

GLAPI void APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params) { 
	Context::DefaultContext()->TexEnvfv(target, pname, params);
}

GLAPI void APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param) { 
	Context::DefaultContext()->TexEnvx(target, pname, param);
}

GLAPI void APIENTRY glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params) { 
	Context::DefaultContext()->TexEnvxv(target, pname, params);
}

GLAPI void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { 
	Context::DefaultContext()->TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

GLAPI void APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) { 
	Context::DefaultContext()->TexParameterf(target, pname, param);
}

GLAPI void APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param) { 
	Context::DefaultContext()->TexParameterx(target, pname, param);
}

GLAPI void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { 
	Context::DefaultContext()->TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLAPI void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z) { 
	Context::DefaultContext()->Translatef(x, y, z);
}

GLAPI void APIENTRY glTranslatex (GLfixed x, GLfixed y, GLfixed z) { 
	Context::DefaultContext()->Translatex(x, y, z);
}

GLAPI void APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) { 
	Context::DefaultContext()->VertexPointer(size, type, stride, pointer);
}

GLAPI void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) { 
	Context::DefaultContext()->Viewport(x, y, width, height);
}

GLAPI void APIENTRY glDrawMeshHM(GLsizei count, GLenum type, GLsizei stride,
								 GLsizei offsetVertex, GLsizei strideVertex,
								 GLsizei offsetNormal, GLsizei strideNormal,
								 GLsizei offsetTexture, GLsizei strideTexture,
								 GLsizei offsetColor, GLsizei strideColor, 
								 const GLvoid *pointer) {
	Context::DefaultContext()->DrawMesh(count, type, stride,
										offsetVertex, strideVertex,
										offsetNormal, strideNormal,
										offsetTexture, strideTexture,
										offsetColor, strideColor, pointer);
}
