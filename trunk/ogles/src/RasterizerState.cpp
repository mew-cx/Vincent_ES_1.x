// ==========================================================================
//
// RasterizerState	Rasterizer State Class for Embedded OpenGL Implementation
//
//					The rasterizer state maintains all the state information
//					that is necessary to determine the details of the
//					next primitive to be scan-converted.
//
// --------------------------------------------------------------------------
//
// 10-10-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "RasterizerState.h"


using namespace EGL;


RasterizerState :: RasterizerState():
	m_Version(0),

	// fog
	m_FogEnabled(false),
	m_FogMode(FogModeExp),
	m_FogColor(Color(0, 0, 0, 0)),
	m_FogStart(0),
	m_FogEnd(EGL_ONE),
	m_FogDensity(EGL_ONE),

	// initial values for alpha test
	m_AlphaTestEnabled(false),
	m_AlphaFunc(CompFuncAlways),
	m_AlphaReference(0),

	// initial values for stencil test
	m_StencilTestEnabled(false),
	m_StencilFunc(CompFuncAlways),
	m_StencilMask(0xFFFFFFFFu),
	m_StencilReference(0),
	m_StencilFail(StencilOpKeep),
	m_StencilZFail(StencilOpKeep),
	m_StencilZPass(StencilOpKeep),

	// inititial values for depth test
	m_DepthFunc(CompFuncLess),
	m_DepthTestEnabled(false),

	// initial values for blending
	m_BlendingEnabled(false),
	m_BlendFuncSrc(BlendFuncSrcOne),
	m_BlendFuncDst(BlendFuncDstZero),

	// initial values for masks
	m_MaskDepth(true),
	m_MaskRed(true),
	m_MaskGreen(true),
	m_MaskBlue(true),
	m_MaskAlpha(true),

	// initial values for logic op
	m_LogicOpEnabled(false),
	m_LogicOpcode(LogicOpCopy)
{
}


RasterizerState :: ~RasterizerState() {
}


