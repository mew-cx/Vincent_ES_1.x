// ==========================================================================
//
// RasterizerState	Rasterizer State Class for 3D Rendering Library
//
//					The rasterizer state maintains all the state information
//					that is necessary to determine the details of the
//					next primitive to be scan-converted.
//
// --------------------------------------------------------------------------
//
// 10-10-2003		Hans-Martin Will	initial version
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
#include "RasterizerState.h"


using namespace EGL;


RasterizerState :: RasterizerState():

	// point and line rasterization
	m_PointSize(EGL_ONE),
	m_LineWidth(EGL_ONE),
	m_PointSmoothEnabled(false),
	m_LineSmoothEnabled(false),

	// fog
	m_FogEnabled(false),
	m_FogColor(Color(0, 0, 0, 0)),

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

	// polygon offset
	m_PolygonOffsetFactor(0),
	m_PolygonOffsetUnits(0),
	m_PolygonOffsetFillEnabled(false),

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
	m_LogicOpcode(LogicOpCopy),

	// texture
	m_TexEnvColor(Color(0, 0, 0, 0)),
	m_TextureEnabled(false),
	m_TextureMode(TextureModeModulate),
	m_MinFilterMode(FilterModeNearest),
	m_MagFilterMode(FilterModeNearest),
	m_MipmapFilterMode(FilterModeNone),
	m_WrappingModeS(WrappingModeRepeat),
	m_WrappingModeT(WrappingModeRepeat),
	m_InternalFormat(TextureFormatLuminance),

	m_ShadingModel(ShadeModelSmooth),
	m_ScissorWidth(1024),			// should be determined by entry in config
	m_ScissorHeight(1024),
	m_ScissorTestEnabled(false)

{
}


RasterizerState :: RasterizerState(const RasterizerState& other):

	m_PointSize(other.m_PointSize),
	m_LineWidth(other.m_LineWidth),
	m_PointSmoothEnabled(other.m_PointSmoothEnabled),
	m_LineSmoothEnabled(other.m_LineSmoothEnabled),

	// fog
	m_FogEnabled(other.m_FogEnabled),
	m_FogColor(other.m_FogColor),

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

	// polygon offset
	m_PolygonOffsetFactor(other.m_PolygonOffsetFactor),
	m_PolygonOffsetUnits(other.m_PolygonOffsetUnits),
	m_PolygonOffsetFillEnabled(other.m_PolygonOffsetFillEnabled),

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
	m_LogicOpcode(other.m_LogicOpcode),

	// texture
	m_TexEnvColor(other.m_TexEnvColor),
	m_TextureEnabled(other.m_TextureEnabled),
	m_TextureMode(other.m_TextureMode),
	m_MinFilterMode(other.m_MinFilterMode),
	m_MagFilterMode(other.m_MagFilterMode),
	m_MipmapFilterMode(other.m_MipmapFilterMode),
	m_WrappingModeS(other.m_WrappingModeS),
	m_WrappingModeT(other.m_WrappingModeT),
	m_InternalFormat(other.m_InternalFormat),

	m_ShadingModel(other.m_ShadingModel),
	m_ScissorWidth(other.m_ScissorWidth),
	m_ScissorHeight(other.m_ScissorHeight),
	m_ScissorTestEnabled(other.m_ScissorTestEnabled)
{
}


RasterizerState :: ~RasterizerState() {
}


RasterizerState& RasterizerState :: operator=(const RasterizerState& other) {

	m_PointSize = other.m_PointSize;
	m_LineWidth = other.m_LineWidth;
	m_PointSmoothEnabled = other.m_PointSmoothEnabled;
	m_LineSmoothEnabled = other.m_LineSmoothEnabled;

	// fog
	m_FogEnabled = other.m_FogEnabled;
	m_FogColor = other.m_FogColor;

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

	// polygon offset
	m_PolygonOffsetFactor= other.m_PolygonOffsetFactor;
	m_PolygonOffsetUnits = other.m_PolygonOffsetUnits;
	m_PolygonOffsetFillEnabled = other.m_PolygonOffsetFillEnabled;

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

	// texture
	m_TexEnvColor = other.m_TexEnvColor;
	m_TextureEnabled = other.m_TextureEnabled;
	m_TextureMode = other.m_TextureMode;
	m_MinFilterMode = other.m_MinFilterMode;
	m_MagFilterMode = other.m_MagFilterMode;
	m_MipmapFilterMode = other.m_MipmapFilterMode;
	m_WrappingModeS = other.m_WrappingModeS;
	m_WrappingModeT = other.m_WrappingModeT;
	m_InternalFormat = other.m_InternalFormat;

	m_ShadingModel = other.m_ShadingModel;
	m_ScissorWidth = other.m_ScissorWidth;
	m_ScissorHeight = other.m_ScissorHeight;
	m_ScissorTestEnabled = other.m_ScissorTestEnabled;

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
