// ==========================================================================
//
// context.cpp	Rendering Context Class for 3D Rendering Library
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
#include "Surface.h"
#include "Rasterizer.h"


using namespace EGL;


// --------------------------------------------------------------------------
// Constructor and destructor
// --------------------------------------------------------------------------


Context :: Context(const Config & config) 
	:
	m_Config(config),
	m_DrawSurface(0),
	m_ReadSurface(0),
	m_LastError(GL_NO_ERROR),

	// transformation matrices
	m_ModelViewMatrixStack(16),
	m_ProjectionMatrixStack(2),
	m_TextureMatrixStack(2),
	m_CurrentMatrixStack(&m_ModelViewMatrixStack),

	// server flags
	m_LightingEnabled(false),
	m_TwoSidedLightning(false),
	m_LightEnabled(1),				// only light 0 is on
	m_CullFaceEnabled(false),
	m_ReverseFaceOrientation(false),
	m_CullMode(CullModeBack),
	m_ColorMaterialEnabled(false),
	m_NormalizeEnabled(false),
	m_RescaleNormalEnabled(false),
	m_PolygonOffsetFillEnabled(false),
	m_MultiSampleEnabled(false),
	m_SampleAlphaToCoverageEnabled(false),
	m_SampleAlphaToOneEnabled(false),
	m_SampleCoverageEnabled(false),

	// fog parameters for setup phase
	m_FogMode(FogModeExp),
	m_FogStart(0),
	m_FogEnd(EGL_ONE),
	m_FogDensity(EGL_ONE),

	// client flags
	m_VertexArrayEnabled(false),
	m_NormalArrayEnabled(false),
	m_ColorArrayEnabled(false),
	m_TexCoordArrayEnabled(false),

	// general context state
	m_Current(false),
	m_Disposed(false),
	m_ViewportInitialized(false),

	// SGIS_generate_mipmap extension
	m_GenerateMipmaps(false)
{
	DepthRangex(VIEWPORT_NEAR, VIEWPORT_FAR);
	m_Rasterizer = new Rasterizer(GetRasterizerState());	
	m_Rasterizer->SetTexture(m_Textures.GetTexture(m_Textures.Allocate()));

	m_LightModelAmbient.r = m_LightModelAmbient.g = m_LightModelAmbient.b = F(0.2f);
	m_LightModelAmbient.a = F(1.0);

	m_Lights[0].SetDiffuseColor(FractionalColor(F(1.0), F(1.0), F(1.0), F(1.0)));
	m_Lights[0].SetSpecularColor(FractionalColor(F(1.0), F(1.0), F(1.0), F(1.0)));
}


Context :: ~Context() {

	if (m_DrawSurface != 0) {
		m_DrawSurface->SetCurrentContext(0);
	}

	if (m_ReadSurface != 0 && m_ReadSurface != m_DrawSurface) {
		m_ReadSurface->SetCurrentContext(0);
	}

	m_ReadSurface = m_DrawSurface = 0;

	if (m_Rasterizer != 0) {

		delete m_Rasterizer;
		m_Rasterizer = 0;
	}
}


void Context :: SetReadSurface(EGL::Surface * surface) {
	if (m_ReadSurface != 0 && m_ReadSurface != m_DrawSurface && m_ReadSurface != surface) {
		m_ReadSurface->SetCurrentContext(0);
	}

	m_ReadSurface = surface;
	m_ReadSurface->SetCurrentContext(this);
}


void Context :: SetDrawSurface(EGL::Surface * surface) {
	if (m_DrawSurface != 0 && m_ReadSurface != m_DrawSurface && m_DrawSurface != surface) {
		m_DrawSurface->SetCurrentContext(0);
	}

	if (surface != 0 && !m_ViewportInitialized) {
		U16 width = surface->GetWidth();
		U16 height = surface->GetHeight();

		Viewport(0, 0, width, height);
		Scissor(0, 0, width, height);
		m_ViewportInitialized = true;
	}

	m_DrawSurface = surface;
	m_DrawSurface->SetCurrentContext(this);
	m_Rasterizer->SetSurface(surface);
}


void Context :: Dispose() {
	if (m_Current) {
		m_Disposed = true;
	} else {
		delete this;
	}
}

void Context :: SetCurrent(bool current) {
	m_Current = current;

	if (!m_Current && m_Disposed) {
		delete this;
	}
}

// --------------------------------------------------------------------------
// Error handling
// --------------------------------------------------------------------------


GLenum Context :: GetError(void) { 
	GLenum result = m_LastError;
	m_LastError = GL_NO_ERROR;
	return result;
}


void Context :: RecordError(GLenum error) {
	if (error != GL_NO_ERROR && m_LastError == GL_NO_ERROR) {
		m_LastError = error;
	}
}


// --------------------------------------------------------------------------
// Clearing
// --------------------------------------------------------------------------


void Context :: Clear(GLbitfield mask) { 

	if (m_DrawSurface == 0) {
		RecordError(GL_INVALID_OPERATION);
		return;
	}

	if (mask & GL_COLOR_BUFFER_BIT) {
		// is clamping [min, max] or [min, max)
		m_DrawSurface->ClearColorBuffer(m_ColorClearValue);
	}

	if (mask & GL_DEPTH_BUFFER_BIT) {
		// actually need to transform depth to correct value
		EGL_Fixed clearValue = EGL_Mul(EGL_MAP_0_1(m_DepthClearValue),m_DepthRangeFactor)  + m_DepthRangeBase;
		m_DrawSurface->ClearDepthBuffer(clearValue);
	}

	if (mask & GL_STENCIL_BUFFER_BIT) {
		m_DrawSurface->ClearStencilBuffer(m_StencilClearValue);
	}
}


void Context :: ClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) { 
	m_ColorClearValue = FractionalColor(red, green, blue, alpha);
}


void Context :: ClearDepthx(GLclampx depth) { 
	m_DepthClearValue = EGL_CLAMP(depth, 0, EGL_ONE);
}


void Context :: ClearStencil(GLint s) { 
	m_StencilClearValue = s;
}


// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------


void Context :: Toggle(GLenum cap, bool value) { 
	switch (cap) {
	case GL_LIGHTING:
		m_LightingEnabled = value;
		break;

	case GL_FOG:
		GetRasterizerState()->EnableFog(value);
		break;

	case GL_TEXTURE_2D:
		GetRasterizerState()->EnableTexture(value);
		break;

	case GL_CULL_FACE:
		m_CullFaceEnabled = value;
		break;

	case GL_ALPHA_TEST:
		GetRasterizerState()->EnableAlphaTest(value);
		break;

	case GL_BLEND:
		GetRasterizerState()->EnableBlending(value);
		break;

	case GL_COLOR_LOGIC_OP:
		GetRasterizerState()->EnableLogicOp(value);
		break;

	case GL_DITHER:
		//GetRasterizerState()->EnableDither(value);
		break;

	case GL_STENCIL_TEST:
		GetRasterizerState()->EnableStencilTest(value);
		break;

	case GL_DEPTH_TEST:
		GetRasterizerState()->EnableDepthTest(value);
		break;

	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		{
			int mask = 1 << (cap - GL_LIGHT0);

			if (value) {
				m_LightEnabled |= mask;
			} else {
				m_LightEnabled &= ~mask;
			}
		}
		break;

	case GL_POINT_SMOOTH:
		GetRasterizerState()->SetPointSmoothEnabled(value);
		break;

	case GL_LINE_SMOOTH:
		GetRasterizerState()->SetLineSmoothEnabled(value);
		break;

	case GL_SCISSOR_TEST:
		GetRasterizerState()->EnableScissorTest(value);
		break;

	case GL_COLOR_MATERIAL:
		m_ColorMaterialEnabled = value;
		break;

	case GL_NORMALIZE:
		m_NormalizeEnabled = value;
		break;

	case GL_RESCALE_NORMAL:
		m_RescaleNormalEnabled = value;
		break;

	case GL_POLYGON_OFFSET_FILL:
		GetRasterizerState()->EnablePolygonOffsetFill(value);
		break;

	case GL_MULTISAMPLE:
		m_MultiSampleEnabled = value;
		break;

	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		m_SampleAlphaToCoverageEnabled = value;
		break;

	case GL_SAMPLE_ALPHA_TO_ONE:
		m_SampleAlphaToOneEnabled = value;
		break;

	case GL_SAMPLE_COVERAGE:
		m_SampleCoverageEnabled = value;
		break;

	default:
		RecordError(GL_INVALID_ENUM);
		return;
	}
}

void Context :: Disable(GLenum cap) { 
	Toggle(cap, false);
}

void Context :: Enable(GLenum cap) { 
	Toggle(cap, true);
}

void Context :: Hint(GLenum target, GLenum mode) { 
	// in principle, we have to fail for any target/mode combination 
	// that is not defined
}

void Context :: GetIntegerv(GLenum pname, GLint *params) { 
	switch (pname) {
	case GL_ALPHA_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_ALPHA_SIZE);
		break;

	case GL_BLUE_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_BLUE_SIZE);
		break;

	case GL_DEPTH_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_DEPTH_SIZE);
		break;

	case GL_GREEN_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_GREEN_SIZE);
		break;

	case GL_RED_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_RED_SIZE);
		break;

	case GL_STENCIL_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_STENCIL_SIZE);
		break;

	case GL_SUBPIXEL_BITS:
		params[0] = m_Config.GetConfigAttrib(EGL_SAMPLES);
		break;

	case GL_ALIASED_LINE_WIDTH_RANGE:
	case GL_ALIASED_POINT_SIZE_RANGE:
	case GL_SMOOTH_LINE_WIDTH_RANGE:
	case GL_SMOOTH_POINT_SIZE_RANGE:
		params[0] = 1;
		break;


	case GL_COMPRESSED_TEXTURE_FORMATS:
		break;

	case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
		params[0] = 0;
		break;

	case GL_MAX_ELEMENTS_INDICES:
	case GL_MAX_ELEMENTS_VERTICES:
		params[0] = INT_MAX;
		break;

	case GL_MAX_LIGHTS:
		params[0] = EGL_NUMBER_LIGHTS;
		break;

	case GL_MAX_MODELVIEW_STACK_DEPTH:
		params[0] = m_ModelViewMatrixStack.GetStackSize();
		break;

	case GL_MAX_PROJECTION_STACK_DEPTH:
		params[0] = m_ProjectionMatrixStack.GetStackSize();
		break;

	case GL_MAX_TEXTURE_STACK_DEPTH:
		params[0] = m_TextureMatrixStack.GetStackSize();
		break;

	case GL_MAX_TEXTURE_SIZE:
		params[0] = 4096;
		break;

	case GL_MAX_TEXTURE_UNITS:
		params[0] = 1;
		break;

	case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
		params[0] = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
		params[0] = GL_RGB;
		break;

	case GL_MAX_VIEWPORT_DIMS:
		params[0] = m_Config.m_Width;
		params[1] = m_Config.m_Height;
		break;

	default:
		RecordError(GL_INVALID_ENUM);
	}
}


const GLubyte * Context :: GetString(GLenum name) { 

	switch (name) {
	case GL_VENDOR:
		return (GLubyte *) EGL_CONFIG_VENDOR;

	case GL_VERSION:
		return (GLubyte *) EGL_CONFIG_VERSION;

	case GL_RENDERER:
		return (GLubyte *) EGL_CONFIG_RENDERER;

	case GL_EXTENSIONS:
		return (GLubyte *) EGL_CONFIG_EXTENSIONS;

	default:
		RecordError(GL_INVALID_ENUM);
		return 0;
	}
}

void Context :: Finish(void) { }
void Context :: Flush(void) { }

// --------------------------------------------------------------------------
// Context Management
// --------------------------------------------------------------------------


void Context :: SetCurrentContext(Context * context) {

	Context * oldContext = GetCurrentContext();

	if (oldContext != context) {

		if (oldContext != 0) 
			oldContext->SetCurrent(false);

		TlsSetValue(s_TlsIndexContext, reinterpret_cast<void *>(context));

		if (context != 0)
			context->SetCurrent(true);
	}
}


Context * Context :: GetCurrentContext() {
	return reinterpret_cast<EGLContext> (TlsGetValue(s_TlsIndexContext));
}

