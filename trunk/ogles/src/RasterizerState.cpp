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


RasterizerState :: RasterizerState(const RasterizerState& other):
	m_Version(other.m_Version),

	// fog
	m_FogEnabled(other.m_FogEnabled),
	m_FogMode(other.m_FogMode),
	m_FogColor(other.m_FogColor),
	m_FogStart(other.m_FogStart),
	m_FogEnd(other.m_FogEnd),
	m_FogDensity(other.m_FogDensity),

	// initial values for alpha test
	m_AlphaTestEnabled(other.m_AlphaTestEnabled),
	m_AlphaFunc(other.m_AlphaFunc),
	m_AlphaReference(other.m_AlphaReference),

	// initial values for stencil test
	m_StencilTestEnabled(other.m_StencilTestEnabled),
	m_StencilFunc(other.m_StencilFunc),
	m_StencilMask(other.m_StencilMask),
	m_StencilReference(other.m_StencilReference),
	m_StencilFail(other.m_StencilFail),
	m_StencilZFail(other.m_StencilZFail),
	m_StencilZPass(other.m_StencilZPass),

	// inititial values for depth test
	m_DepthFunc(other.m_DepthFunc),
	m_DepthTestEnabled(other.m_DepthTestEnabled),

	// initial values for blending
	m_BlendingEnabled(other.m_BlendingEnabled),
	m_BlendFuncSrc(other.m_BlendFuncSrc),
	m_BlendFuncDst(other.m_BlendFuncDst),

	// initial values for masks
	m_MaskDepth(other.m_MaskDepth),
	m_MaskRed(other.m_MaskRed),
	m_MaskGreen(other.m_MaskGreen),
	m_MaskBlue(other.m_MaskBlue),
	m_MaskAlpha(other.m_MaskAlpha),

	// initial values for logic op
	m_LogicOpEnabled(other.m_LogicOpEnabled),
	m_LogicOpcode(other.m_LogicOpcode)
{
}


RasterizerState :: ~RasterizerState() {
}


RasterizerState& RasterizerState :: operator=(const RasterizerState& other) {
	m_Version = other.m_Version;

	// fog
	m_FogEnabled = other.m_FogEnabled;
	m_FogMode = other.m_FogMode;
	m_FogColor = other.m_FogColor;
	m_FogStart = other.m_FogStart;
	m_FogEnd = other.m_FogEnd;
	m_FogDensity = other.m_FogDensity;

	// initial values for alpha test
	m_AlphaTestEnabled = other.m_AlphaTestEnabled;
	m_AlphaFunc = other.m_AlphaFunc;
	m_AlphaReference = other.m_AlphaReference;

	// initial values for stencil test
	m_StencilTestEnabled = other.m_StencilTestEnabled;
	m_StencilFunc = other.m_StencilFunc;
	m_StencilMask = other.m_StencilMask;
	m_StencilReference = other.m_StencilReference;
	m_StencilFail = other.m_StencilFail;
	m_StencilZFail = other.m_StencilZFail;
	m_StencilZPass = other.m_StencilZPass;

	// inititial values for depth test
	m_DepthFunc = other.m_DepthFunc;
	m_DepthTestEnabled = other.m_DepthTestEnabled;

	// initial values for blending
	m_BlendingEnabled = other.m_BlendingEnabled;
	m_BlendFuncSrc = other.m_BlendFuncSrc;
	m_BlendFuncDst = other.m_BlendFuncDst;

	// initial values for masks
	m_MaskDepth = other.m_MaskDepth;
	m_MaskRed = other.m_MaskRed;
	m_MaskGreen = other.m_MaskGreen;
	m_MaskBlue = other.m_MaskBlue;
	m_MaskAlpha = other.m_MaskAlpha;

	// initial values for logic op
	m_LogicOpEnabled = other.m_LogicOpEnabled;
	m_LogicOpcode = other.m_LogicOpcode;

	return *this;
}


// -1 if less, 0 if equal, 1 if greater
int RasterizerState :: Compare(const RasterizerState& other) const {
	return memcmp(this, &other, sizeof(*this));
}


size_t RasterizerState :: HashCode() const {
	size_t hashVal = 0;
	const U8 * ptr = reinterpret_cast<const U8 *>(this);

	for (int index = 0; index < sizeof(*this); ++index) {
		hashVal = (hashVal << 5) |
			(hashVal >> 27) |
			*ptr++;
	}

	return hashVal;
}
