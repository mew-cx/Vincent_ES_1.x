// ==========================================================================
//
// context.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
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
	m_LightEnabled(1),				// only light 0 is on
	m_CullFaceEnabled(false),
	m_ColorMaterialEnabled(false),
	m_NormalizeEnabled(false),
	m_RescaleNormalEnabled(false),
	m_PolygonOffsetFillEnabled(false),
	m_MultiSampleEnabled(false),
	m_SampleAlphaToCoverageEnabled(false),
	m_SampleAlphaToOneEnabled(false),
	m_SampleCoverageEnabled(false),

	//m_ScanLineFunction(&gppScanLine_G_Zal_16_32s),

	// client flags
	m_VertexArrayEnabled(false),
	m_NormalArrayEnabled(false),
	m_ColorArrayEnabled(false),
	m_TexCoordArrayEnabled(false),

	// general context state
	m_Current(false),
	m_Disposed(false),
	m_ViewportInitialized(false)
{
	DepthRangex(VIEWPORT_NEAR, VIEWPORT_FAR);
	m_Textures.push_back(new MultiTexture());
	m_Rasterizer = new Rasterizer(GetRasterizerState());
	m_Rasterizer->SetTexture(m_Textures[0]);

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
		m_DrawSurface->ClearDepthBuffer(EGL_MAP_0_1(m_DepthClearValue));
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
		GetRasterizerState()->EnableFog(value);
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

void Context :: Hint(GLenum target, GLenum mode) { }

void Context :: GetIntegerv(GLenum pname, GLint *params) { }


const GLubyte * Context :: GetString(GLenum name) { 
	return 0;
}

void Context :: Finish(void) { }
void Context :: Flush(void) { }

void Context :: FrontFace(GLenum mode) { }

void Context :: CullFace(GLenum mode) { }


// --------------------------------------------------------------------------
// Context Management
// --------------------------------------------------------------------------

static DWORD s_TlsIndexContext = TlsAlloc();


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

