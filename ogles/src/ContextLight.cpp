// ==========================================================================
//
// ContextLight.cpp	Rendering Context Class for OpenGL (R) ES Implementation
//
//					Lighting and Shading related settings
//
// --------------------------------------------------------------------------
//
// 08-02-2003	Hans-Martin Will	initial version
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
#include "RasterizerState.h"

using namespace EGL;


// ==========================================================================
// class Context
// ==========================================================================

void Context :: Fogx(GLenum pname, GLfixed param) { 
	switch (pname) {
		case GL_FOG_MODE:
			switch (param) {
				default:
					RecordError(GL_INVALID_ENUM);
					break;

				case GL_LINEAR:
					m_FogMode = FogLinear;
					break;

				case GL_EXP:
					m_FogMode = FogModeExp;
					break;

				case GL_EXP2:
					m_FogMode = FogModeExp2;
					break;

			}

			break;

		case GL_FOG_START:
			m_FogStart = param;
			break;

		case GL_FOG_END:
			m_FogEnd = param;
			break;

		case GL_FOG_DENSITY:
			m_FogDensity = param;
			break;

		default:
			RecordError(GL_INVALID_ENUM);
	}
}

void Context :: Fogxv(GLenum pname, const GLfixed *params) { 
	switch (pname) {
		case GL_FOG_COLOR:
			GetRasterizerState()->SetFogColor(FractionalColor(params));
			break;

		default:
			Fogx(pname, *params);
			break;
	}
}

void Context :: LightModelx(GLenum pname, GLfixed param) { 
}

void Context :: LightModelxv(GLenum pname, const GLfixed *params) { 

	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		m_LightModelAmbient = FractionalColor::Clamp(params);
		break;

	default:
		LightModelx(pname, *params);
		return;
	}
}

void Context :: Lightx(GLenum light, GLenum pname, GLfixed param) { 

	if (light < GL_LIGHT0 || light > GL_LIGHT7) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	Light * pLight = m_Lights + (light - light);

	switch (pname) {
	case GL_SPOT_EXPONENT:
		pLight->SetSpotExponent(param);
		break;

	case GL_SPOT_CUTOFF:
		pLight->SetSpotCutoff(param);
		break;

	case GL_CONSTANT_ATTENUATION:
		pLight->SetConstantAttenuation(param);
		break;

	case GL_LINEAR_ATTENUATION:
		pLight->SetLinearAttenuation(param);
		break;

	case GL_QUADRATIC_ATTENUATION:
		pLight->SetQuadraticAttenuation(param);
		break;

	default:
		RecordError(GL_INVALID_ENUM);
		return;
	}
}

void Context :: Lightxv(GLenum light, GLenum pname, const GLfixed *params) { 
	if (light < GL_LIGHT0 || light > GL_LIGHT7) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	Light * pLight = m_Lights + (light - light);

	switch (pname) {
	case GL_AMBIENT:
		pLight->SetAmbientColor(FractionalColor::Clamp(params));
		break;

	case GL_DIFFUSE:
		pLight->SetDiffuseColor(FractionalColor::Clamp(params));
		break;

	case GL_SPECULAR:
		pLight->SetSpecularColor(FractionalColor::Clamp(params));
		break;

	case GL_POSITION:
		pLight->SetPosition(m_ModelViewMatrixStack.CurrentMatrix() * Vec4D(params));
		break;

	case GL_SPOT_DIRECTION:
		// TODO
		{/*
			GPP_VEC3D directionIn;
			GPP_VEC4D directionTransformed;
			directionIn.x = params[0];
			directionIn.y = params[1];
			directionIn.z = params[2];
			gppVec3DTransform_16_32s(&directionIn, &directionTransformed,
				m_ModelViewMatrixStack.CurrentMatrix());
			pLight->SetDirection(*reinterpret_cast<GPP_VEC3D *>(&directionTransformed));
			*/
		}
		break;

	default:
		Lightx(light, pname, *params);
		return;
	}
}


void Context :: Materialx(GLenum face, GLenum pname, GLfixed param) { 

	if (face != GL_FRONT_AND_BACK) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	Material * material = &m_FrontMaterial;

	switch (pname) {
	case GL_SHININESS:
		material->SetSpecularExponent(param);
		break;

	default:
		RecordError(GL_INVALID_ENUM);
		return;
	}
}

void Context :: Materialxv(GLenum face, GLenum pname, const GLfixed *params) { 

	if (face != GL_FRONT_AND_BACK) {
		RecordError(GL_INVALID_ENUM);
		return;
	}

	Material * material = &m_FrontMaterial;

	switch (pname) {
	case GL_AMBIENT:
		material->SetAmbientColor(FractionalColor::Clamp(params));
		break;

	case GL_DIFFUSE:
		material->SetDiffuseColor(FractionalColor::Clamp(params));
		break;

	case GL_AMBIENT_AND_DIFFUSE:
		material->SetAmbientColor(FractionalColor::Clamp(params));
		material->SetDiffuseColor(FractionalColor::Clamp(params));
		break;

	case GL_SPECULAR:
		material->SetSpecularColor(FractionalColor::Clamp(params));
		break;

	case GL_EMISSION:
		material->SetEmisiveColor(FractionalColor::Clamp(params));
		break;

	default:
		Materialx(face, pname, *params);
		break;
	}
}


