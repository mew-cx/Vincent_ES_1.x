// ==========================================================================
//
// Color			Integer 8-8-8-8 representation of an RGBA color
//
// --------------------------------------------------------------------------
//
// 10-09-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Color.h"
#include "Init.h"


using namespace EGL;


EGL_Fixed Color::s_alphaFactor[256];


void Color :: InitAlphaFactorTable() {
	for (int index = 0; index < elementsof(s_alphaFactor); ++index) {
		s_alphaFactor[index] = EGL_FixedFromFloat(index / 0.255f);
	}
}


namespace {
	Init init(Color::InitAlphaFactorTable);
}