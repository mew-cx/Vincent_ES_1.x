// ==========================================================================
//
// light.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//				Lightning Related Operations
//
// --------------------------------------------------------------------------
//
// 08-07-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Light.h"
#include "Material.h"
#include <math.h>


#ifndef M_PI
#	define M_PI       3.14159265358979323846
#endif


using namespace EGL;


// ==========================================================================
// class Light
// ==========================================================================


Light :: Light() 
:	m_AmbientColor(F(0.2f), F(0.2f), F(0.2f), F(1.0f)),
	m_DiffuseColor(F(0.8f), F(0.8f), F(0.8f), F(1.0f)),
	m_SpecularColor(F(0.0f), F(0.0f), F(0.0f), F(1.0f)),
	m_Position(F(0.0f), F(0.0f), F(1.0f), F(0.0f)),
	m_SpotDirection(F(0.0f), F(0.0f), F(-1.0f)),
	m_SpotCutoff(F(180.0f)),
	m_ConstantAttenuation(F(1.0f)),
	m_LinearAttenuation(0),
	m_QuadraticAttenuation(0)
{
}


void Light :: SetAmbientColor(const FractionalColor & color) {
	m_AmbientColor = color;
}


void Light :: SetDiffuseColor(const FractionalColor & color) {
	m_DiffuseColor = color;
}


void Light :: SetSpecularColor(const FractionalColor & color) {
	m_SpecularColor = color;
}


void Light :: SetPosition(const Vec4D & position) {
	m_Position = position;
}


void Light :: SetDirection(const Vec3D & direction) {
	m_SpotDirection = direction;
	m_SpotDirection.Normalize();
}


void Light :: SetConstantAttenuation(EGL_Fixed attenuation) {
	m_ConstantAttenuation = attenuation;
}


void Light :: SetLinearAttenuation(EGL_Fixed attenuation) {
	m_LinearAttenuation = attenuation;
}


void Light :: SetQuadraticAttenuation(EGL_Fixed attenuation) {
	m_QuadraticAttenuation = attenuation;
}


void Light :: SetSpecularExponent(EGL_Fixed exponent) {
	m_SpecularExponent = exponent;
}


void Light :: SetSpotExponent(EGL_Fixed exponent) {
	m_SpotExponent = exponent;
}


void Light :: SetSpotCutoff(EGL_Fixed cutoff) {
	m_SpotCutoff = cutoff;
	EGL_Fixed angle = EGL_Mul(angle, EGL_FixedFromFloat(static_cast<float>(M_PI) / 180.0f));
	m_CosineSpotCutoff = EGL_Cos(angle);
}


void Light :: InitWithMaterial(const Material& material) {

	m_EffectiveAmbientColor = material.GetAmbientColor() * m_AmbientColor;
	m_EffectiveDiffuseColor = material.GetDiffuseColor() * m_DiffuseColor;
	m_EffectiveSpecularColor = material.GetSpecularColor() * m_SpecularColor;
}


void Light :: AccumulateLight(const Vec4D & vertexCoords, const Vec3D& vertexNormal,
							  FractionalColor& result) {
	// Optimize special cases later by using a pointer to member function that is updated
	// whenever one of the parameters is changed
	AccumulateLight(vertexCoords, vertexNormal, FractionalColor(EGL_ONE, EGL_ONE, EGL_ONE, EGL_ONE),
		result);
}


void Light :: AccumulateLight(const Vec4D& vertexCoords, const Vec3D& vertexNormal, 
							  const FractionalColor& currentColor, 
							  FractionalColor& result) {
	// initially, do not support spotlights to simplify calculations,
	// i.e. set spot_i from lightning equation (p. 48) to 1.0

	result += currentColor * m_AmbientColor;

	Vec3D vp_li = EGL_Direction(vertexCoords, m_Position);
	EGL_Fixed sqLength = vp_li.LengthSq();			// keep squared length around for later
	vp_li.Normalize();								// can optimizer factor this out?
	EGL_Fixed diffuseFactor = vertexNormal * vp_li;

	if (diffuseFactor > 0) {
		EGL_Fixed att = EGL_ONE;

		if (m_SpotCutoff != EGL_FixedFromInt(180)) {
			EGL_Fixed cosine = vp_li * m_SpotDirection;

			if (cosine < m_CosineSpotCutoff) {
				return;
			} else {
				att = EGL_Power(cosine, m_SpotExponent);
			}
		}

		if (m_Position.w() != 0) {
			EGL_Fixed length = EGL_Sqrt(sqLength);

			att = EGL_Mul(att, 
				EGL_Inverse(m_ConstantAttenuation + 
					EGL_Mul(m_LinearAttenuation, length) +
					EGL_Mul(m_QuadraticAttenuation, sqLength)));
		}

		result.Accumulate(currentColor * m_DiffuseColor, EGL_Mul(diffuseFactor, att));

		// add specular component
		// calculate h^
		Vec3D h = vp_li + Vec3D(0, 0, EGL_ONE);
		h.Normalize();
		EGL_Fixed specularFactor = vertexNormal * h;

		if (specularFactor > 0) {
			result.Accumulate(currentColor * m_SpecularColor, 
				EGL_Mul(att, EGL_Power(specularFactor, m_SpecularExponent)));
		}
	}
}

