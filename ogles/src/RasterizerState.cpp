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
	m_DepthFunc(CompFuncLess),
	m_DepthTestEnabled(false)

{
}


RasterizerState :: ~RasterizerState() {
}


