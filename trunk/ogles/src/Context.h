#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H 1

#pragma once

// ==========================================================================
//
// context.h	Rendering Context Class for OpenGL (R) ES Implementation
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


#include "OGLES.h"
#include "GLES/gl.h"
#include "linalg.h"
#include "FractionalColor.h"
#include "Config.h"
#include "Material.h"
#include "Light.h"
#include "RasterizerState.h"
#include "Rasterizer.h"
#include "MatrixStack.h"
#include "Texture.h"
#include <vector>

namespace EGL {

	struct OGLES_API VertexArray {
		VertexArray() {
			pointer = 0;
			stride = 0;
			size = 4;
			size = 0;
		};

		GLfixed GetValue(int row, int column) {
			GLsizei rowOffset = row * stride;
			const unsigned char * base = reinterpret_cast<const unsigned char *>(pointer) + rowOffset;

			switch (type) {
			case GL_BYTE:
				return EGL_FixedFromInt(*(reinterpret_cast<const char *>(base) + column));

			case GL_SHORT:
				return EGL_FixedFromInt(*(reinterpret_cast<const short *>(base) + column));

			case GL_FIXED:
				return *(reinterpret_cast<const I32 *>(base) + column);

			default:
				return 0;
			}
		}

		GLint			size;
		GLenum			type;
		GLsizei			stride;
		const GLvoid *	pointer;
	};


	class Surface;
	class MultiTexture;

	#define EGL_NUMBER_LIGHTS 8

	class Context {

		// ----------------------------------------------------------------------
		// Default viewport configuration
		// ----------------------------------------------------------------------
		enum {
			VIEWPORT_X = 0,
			VIEWPORT_Y = 0,
			VIEWPORT_WIDTH = 240,
			VIEWPORT_HEIGHT = 320,
			VIEWPORT_NEAR = 0,
			VIEWPORT_FAR = EGL_ONE
		};

	public:
		Context(const Config & config);

		~Context();

		// ----------------------------------------------------------------------
		// Public OpenGL ES entry points
		// ----------------------------------------------------------------------
		void ActiveTexture(GLenum texture);
		void AlphaFuncx(GLenum func, GLclampx ref);
		void BindTexture(GLenum target, GLuint texture);
		void BlendFunc(GLenum sfactor, GLenum dfactor);
		void Clear(GLbitfield mask);
		void ClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
		void ClearDepthx(GLclampx depth);
		void ClearStencil(GLint s);
		void ClientActiveTexture(GLenum texture);
		void Color4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
		void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		void ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
		void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
		void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		void CullFace(GLenum mode);
		void DeleteTextures(GLsizei n, const GLuint *textures);
		void DepthFunc(GLenum func);
		void DepthMask(GLboolean flag);
		void DepthRangex(GLclampx zNear, GLclampx zFar);
		void Disable(GLenum cap);
		void DisableClientState(GLenum array);
		void DrawArrays(GLenum mode, GLint first, GLsizei count);
		void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
		void Enable(GLenum cap);
		void EnableClientState(GLenum array);
		void Finish(void);
		void Flush(void);
		void Fogx(GLenum pname, GLfixed param);
		void Fogxv(GLenum pname, const GLfixed *params);
		void FrontFace(GLenum mode);
		void Frustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
		void GenTextures(GLsizei n, GLuint *textures);
		GLenum GetError(void);
		void GetIntegerv(GLenum pname, GLint *params);
		const GLubyte * GetString(GLenum name);
		void Hint(GLenum target, GLenum mode);
		void LightModelx(GLenum pname, GLfixed param);
		void LightModelxv(GLenum pname, const GLfixed *params);
		void Lightx(GLenum light, GLenum pname, GLfixed param);
		void Lightxv(GLenum light, GLenum pname, const GLfixed *params);
		void LineWidthx(GLfixed width);
		void LoadIdentity(void);
		void LoadMatrixx(const GLfixed *m);
		void LogicOp(GLenum opcode);
		void Materialx(GLenum face, GLenum pname, GLfixed param);
		void Materialxv(GLenum face, GLenum pname, const GLfixed *params);
		void MatrixMode(GLenum mode);
		void MultMatrixx(const GLfixed *m);
		void MultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
		void Normal3x(GLfixed nx, GLfixed ny, GLfixed nz);
		void NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
		void Orthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
		void PixelStorei(GLenum pname, GLint param);
		void PointSizex(GLfixed size);
		void PolygonOffsetx(GLfixed factor, GLfixed units);
		void PopMatrix(void);
		void PushMatrix(void);
		void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
		void Rotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
		void SampleCoveragex(GLclampx value, GLboolean invert);
		void Scalex(GLfixed x, GLfixed y, GLfixed z);
		void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
		void ShadeModel(GLenum mode);
		void StencilFunc(GLenum func, GLint ref, GLuint mask);
		void StencilMask(GLuint mask);
		void StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
		void TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void TexEnvx(GLenum target, GLenum pname, GLfixed param);
		void TexEnvxv(GLenum target, GLenum pname, const GLfixed *params);
		void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
		void TexParameterf(GLenum target, GLenum pname, GLfloat param);
		void TexParameterx(GLenum target, GLenum pname, GLfixed param);
		void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
		void Translatex(GLfixed x, GLfixed y, GLfixed z);
		void VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

		// ----------------------------------------------------------------------
		// Floating Point API
		// ----------------------------------------------------------------------

		void AlphaFunc(GLenum func, GLclampf ref);
		void ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
		void ClearDepthf(GLclampf depth);
		void Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		void DepthRangef(GLclampf zNear, GLclampf zFar);
		void Fogf(GLenum pname, GLfloat param);
		void Fogfv(GLenum pname, const GLfloat *params);
		void Frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
		void LightModelf(GLenum pname, GLfloat param);
		void LightModelfv(GLenum pname, const GLfloat *params);
		void Lightf(GLenum light, GLenum pname, GLfloat param);
		void Lightfv(GLenum light, GLenum pname, const GLfloat *params);
		void LineWidth(GLfloat width);
		void LoadMatrixf(const GLfloat *m);
		void Materialf(GLenum face, GLenum pname, GLfloat param);
		void Materialfv(GLenum face, GLenum pname, const GLfloat *params);
		void MultMatrixf(const GLfloat *m);
		void MultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		void Normal3f(GLfloat nx, GLfloat ny, GLfloat nz);
		void Orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
		void PointSize(GLfloat size);
		void PolygonOffset(GLfloat factor, GLfloat units);
		void Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		void SampleCoverage(GLclampf value, GLboolean invert);
		void Scalef(GLfloat x, GLfloat y, GLfloat z);
		void TexEnvf(GLenum target, GLenum pname, GLfloat param);
		void TexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
		void Translatef(GLfloat x, GLfloat y, GLfloat z);

		// ----------------------------------------------------------------------
		// Context Management Functions
		// ----------------------------------------------------------------------

		void Dispose();
		void SetCurrent(bool current);
		Config * GetConfig();

		RasterizerState * GetRasterizerState() {
			return &m_RasterizerState;
		}

		const RasterizerState * GetRasterizerState() const {
			return &m_RasterizerState;
		}

		Rasterizer * GetRasterizer() {
			return m_Rasterizer;
		}

		MultiTexture * GetCurrentTexture() {
			return m_Rasterizer->GetTexture();
		}

		const MultiTexture * GetCurrentTexture() const {
			return m_Rasterizer->GetTexture();
		}

		static void SetCurrentContext(Context * context);
		static Context * GetCurrentContext();
		static Context * DefaultContext();

		void SetReadSurface(Surface * surface);
		void SetDrawSurface(Surface * surface);
		Surface * GetReadSurface() const;
		Surface * GetDrawSurface() const;

	private:
		// ----------------------------------------------------------------------
		// Private Functions
		// ----------------------------------------------------------------------

		inline MatrixStack * CurrentMatrixStack() {
			return m_CurrentMatrixStack;
		}

		inline const Matrix4x4& CurrentMatrix() {
			return CurrentMatrixStack()->CurrentMatrix();
		}

		void UpdateInverseModelViewMatrix(void);

		void RecordError(GLenum error);
		void Toggle(GLenum cap, bool value);
		void ToggleClientState(GLenum array, bool value);

		// ----------------------------------------------------------------------
		// Private Functions - Rendering of collections
		// ----------------------------------------------------------------------

		void RenderPoints(GLint first, GLsizei count);
		void RenderPoints(GLsizei count, const GLubyte * indices);
		void RenderPoints(GLsizei count, const GLushort * indices);

		void RenderLines(GLint first, GLsizei count);
		void RenderLines(GLsizei count, const GLubyte * indices);
		void RenderLines(GLsizei count, const GLushort * indices);

		void RenderLineStrip(GLint first, GLsizei count);
		void RenderLineStrip(GLsizei count, const GLubyte * indices);
		void RenderLineStrip(GLsizei count, const GLushort * indices);

		void RenderLineLoop(GLint first, GLsizei count);
		void RenderLineLoop(GLsizei count, const GLubyte * indices);
		void RenderLineLoop(GLsizei count, const GLushort * indices);

		void RenderTriangles(GLint first, GLsizei count);
		void RenderTriangles(GLsizei count, const GLubyte * indices);
		void RenderTriangles(GLsizei count, const GLushort * indices);

		void RenderTriangleStrip(GLint first, GLsizei count);
		void RenderTriangleStrip(GLsizei count, const GLubyte * indices);
		void RenderTriangleStrip(GLsizei count, const GLushort * indices);

		void RenderTriangleFan(GLint first, GLsizei count);
		void RenderTriangleFan(GLsizei count, const GLubyte * indices);
		void RenderTriangleFan(GLsizei count, const GLushort * indices);

		// ----------------------------------------------------------------------
		// Private Functions - Rendering of individual elements
		// ----------------------------------------------------------------------

		void SelectArrayElement(int index);
		void CurrentValuesToRasterPos(RasterPos * rasterPos);
		void InterpolateRasterPos(RasterPos * a, RasterPos * b, GLfixed x, RasterPos * result);

	private:
		GLenum				m_LastError;

		// ----------------------------------------------------------------------
		// Matrix stacks
		// ----------------------------------------------------------------------
		MatrixStack			m_ModelViewMatrixStack;
		MatrixStack			m_ProjectionMatrixStack;
		MatrixStack			m_TextureMatrixStack;

		MatrixStack *		m_CurrentMatrixStack;
		Matrix4x4			m_InverseModelViewMatrix;

		// ----------------------------------------------------------------------
		// Viewport configuration
		// ----------------------------------------------------------------------
		GLint				m_ViewportX;
		GLint				m_ViewportY;
		GLsizei				m_ViewportWidth;
		GLsizei				m_ViewportHeight;
		GLclampx			m_ViewportNear;
		GLclampx			m_ViewportFar;

		// origin and scale for actual transformation
		Vec3D				m_ViewportOrigin;
		Vec3D				m_ViewportScale;

		// ----------------------------------------------------------------------
		// Client state variables
		// ----------------------------------------------------------------------
		bool				m_VertexArrayEnabled;
		bool				m_NormalArrayEnabled;
		bool				m_ColorArrayEnabled;
		bool				m_TexCoordArrayEnabled;

		VertexArray			m_VertexArray;
		VertexArray			m_NormalArray;
		VertexArray			m_ColorArray;
		VertexArray			m_TexCoordArray;


		// ----------------------------------------------------------------------
		// Default values if arrays are disabled
		// ----------------------------------------------------------------------
		Vec3D				m_DefaultNormal;
		FractionalColor		m_DefaultRGBA;
		TexCoord			m_DefaultTextureCoords;

		// ----------------------------------------------------------------------
		// Current values for setup
		// ----------------------------------------------------------------------
		Vec3D				m_CurrentVertex;	// don't support vertex at inf
		Vec3D				m_CurrentNormal;
		FractionalColor		m_CurrentRGBA;
		TexCoord			m_CurrentTextureCoords;

		// ----------------------------------------------------------------------
		// Rendering State
		// ----------------------------------------------------------------------
		Light				m_Lights[EGL_NUMBER_LIGHTS];
		Material			m_FrontMaterial;
		FractionalColor		m_LightModelAmbient;
		GLint				m_LightEnabled;

		EGL_Fixed			m_DepthClearValue;
		FractionalColor		m_ColorClearValue;
		U32					m_StencilClearValue;

		bool				m_LightingEnabled;	// is lightning enabled?
		bool				m_CullFaceEnabled;
		bool				m_ColorMaterialEnabled;
		bool				m_NormalizeEnabled;
		bool				m_RescaleNormalEnabled;
		bool				m_PolygonOffsetFillEnabled;
		bool				m_MultiSampleEnabled;
		bool				m_SampleAlphaToCoverageEnabled;
		bool				m_SampleAlphaToOneEnabled;
		bool				m_SampleCoverageEnabled;

		I32					m_PixelStorePackAlignment;
		I32					m_PixelStoreUnpackAlignment;

		RasterizerState		m_RasterizerState;
		Rasterizer *		m_Rasterizer;

		// ----------------------------------------------------------------------
		// texturing related state
		// ----------------------------------------------------------------------
		typedef std::vector<MultiTexture *> TextureVector;

		TextureVector		m_Textures;

		// ----------------------------------------------------------------------
		// Object-Life Cycle State
		// ----------------------------------------------------------------------

		Surface *			m_DrawSurface;		// current surface for drawing
		Surface *			m_ReadSurface;		// current surface for reading

		Config				m_Config;			// copy of configuration args
		bool				m_Current;			// this context has been selected
												// as a current context
		bool				m_Disposed;			// this context has been deleted,
												// but is stil selected into a 
												// thread
		bool				m_ViewportInitialized;	// if true, the viewport has been
													// initialized
	};


	inline Config * Context :: GetConfig() {
		return &m_Config;
	}


	inline Context * Context :: DefaultContext() {
		return Context::GetCurrentContext();
	}


	inline Surface * Context :: GetDrawSurface() const {
		return m_DrawSurface;
	}


	inline Surface * Context :: GetReadSurface() const {
		return m_ReadSurface;
	}

}

#endif //ndef EGL_CONTEXT_H
