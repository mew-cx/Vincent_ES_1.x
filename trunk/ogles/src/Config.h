#ifndef EGL_CONFIG_H
#define EGL_CONFIG_H 1

#pragma once

// ==========================================================================
//
// Config.h			Configuration Management Class
//					for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-14-2003		Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"
#include "GLES/egl.h"


namespace EGL {

	class OGLES_API Config {
	public:
		Config(
			EGLint	bufferSize,
			EGLint	redSize,
			EGLint	greenSize,
			EGLint	blueSize,
			EGLint	alphaSize,
			EGLint	configCaveat,
			EGLint	configID,
			EGLint	depthSize,
			EGLint	level,
			EGLint	maxPBufferWidth,
			EGLint	maxPBufferHeight,
			EGLint	mxPBufferPixels,
			EGLint	nativeRenderable,
			EGLint	nativeVisualID,
			EGLint	nativeVisualType,
			EGLint	sampleBuffers,
			EGLint	samples,
			EGLint	stencilSize,
			EGLint	surfaceType,
			EGLint	transparentType,
			EGLint	transparentRedValue,
			EGLint	transparentGreenValue,
			EGLint	transparentBlueValue,
			EGLint  width,
			EGLint	height
			);

		Config (const Config &other, const EGLint * attribList = 0, const EGLint * validAttributes = 0);
		Config & operator=(const Config & other);

		EGLint GetConfigAttrib(EGLint attribute) const;
		void SetConfigAttrib(EGLint attribute, EGLint value);

		static EGLBoolean GetConfigs(EGLConfig * result, EGLint configSize, EGLint * numConfig);
		static EGLBoolean ChooseConfig(const EGLint * attribList, EGLConfig * result, EGLint configSize, EGLint * numConfig);

	private:
		bool Matches(EGLint attribList);

		static bool IsValidAttribute(const EGLint * validAttributes, EGLint attribute);

	private:
		EGLint	m_BufferSize;
		EGLint	m_RedSize;
		EGLint	m_GreenSize;
		EGLint	m_BlueSize;
		EGLint	m_AlphaSize;
		EGLint	m_ConfigCaveat;
		EGLint	m_ConfigID;
		EGLint	m_DepthSize;
		EGLint	m_Level;
		EGLint	m_MaxPBufferWidth;
		EGLint	m_MaxPBufferHeight;
		EGLint	m_MaxPBufferPixels;
		EGLint	m_NativeRenderable;
		EGLint	m_NativeVisualID;
		EGLint	m_NativeVisualType;
		EGLint	m_SampleBuffers;
		EGLint	m_Samples;
		EGLint	m_StencilSize;
		EGLint	m_SurfaceType;
		EGLint	m_TransparentType;
		EGLint	m_TransparentRedValue;
		EGLint	m_TransparentGreenValue;
		EGLint	m_TransparentBlueValue;
		EGLint  m_Width;
		EGLint	m_Height;

		static Config s_AllConfigurations[];
		static int s_NumConfigurations;
	};

}

#endif // ndef EGL_CONFIG_H