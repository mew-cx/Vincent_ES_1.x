// ==========================================================================
//
// material.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Coloring and Material Related Operations
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "material.h"


using namespace EGL;


// ==========================================================================
// Material Class Members
// ==========================================================================


void Material :: SetAmbientColor(const FractionalColor & color) {
	m_AmbientColor = color;
}


void Material :: SetDiffuseColor(const FractionalColor & color) {
	m_DiffuseColor = color;
}


void Material :: SetSpecularColor(const FractionalColor & color) {
	m_SpecularColor = color;
}


void Material :: SetEmisiveColor(const FractionalColor & color) {
	m_EmisiveColor = color;
}


void Material :: SetSpecularExponent(EGL_Fixed exponent) {
	m_SpecularExponent = EGL_CLAMP(exponent, 0, EGL_FixedFromInt(128));
}



