// ==========================================================================
//
// ContextMaterial.cpp	Rendering Context Class for Embedded OpenGL Implementation
//
//						RasterizerState related settings
//
// --------------------------------------------------------------------------
//
// 08-02-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "stdafx.h"
#include "Context.h"
#include "RasterizerState.h"

using namespace EGL;


// ==========================================================================
// Material Functions on Context
// ==========================================================================

namespace {
	RasterizerState::ComparisonFunc ComparisonFuncFromEnum(GLenum func) {
		switch (func) {
			case GL_NEVER:		return RasterizerState::CompFuncNever;
			case GL_LESS:		return RasterizerState::CompFuncLess;
			case GL_LEQUAL:		return RasterizerState::CompFuncLEqual;
			case GL_GREATER:	return RasterizerState::CompFuncGreater;
			case GL_GEQUAL:		return RasterizerState::CompFuncGEqual;
			case GL_EQUAL:		return RasterizerState::CompFuncEqual;
			case GL_NOTEQUAL:	return RasterizerState::CompFuncNotEqual;

			default:			// TODO RecordError(GL_INVALID_ENUM);
			case GL_ALWAYS:		return RasterizerState::CompFuncAlways;
		}
	}

	RasterizerState::BlendFuncSrc BlendFuncSrcFromEnum(GLenum func) {
		switch (func) {
			case GL_ONE:					return RasterizerState::BlendFuncSrcOne;
			case GL_DST_COLOR:				return RasterizerState::BlendFuncSrcSrcColor;
			case GL_ONE_MINUS_DST_COLOR:	return RasterizerState::BlendFuncSrcOneMinusSrcColor;
			case GL_SRC_ALPHA:				return RasterizerState::BlendFuncSrcSrcAlpha;
			case GL_ONE_MINUS_SRC_ALPHA:	return RasterizerState::BlendFuncSrcOneMinusSrcAlpha;
			case GL_DST_ALPHA:				return RasterizerState::BlendFuncSrcDstAlpha;
			case GL_ONE_MINUS_DST_ALPHA:	return RasterizerState::BlendFuncSrcOneMinusDstAlpha;
			case GL_SRC_ALPHA_SATURATE:		return RasterizerState::BlendFuncSrcSrcAlphaSaturate;

			default:						// TODO RecordError(GL_INVALID_ENUM);
			case GL_ZERO:					return RasterizerState::BlendFuncSrcZero;
		}
	}

	RasterizerState::BlendFuncDst BlendFuncDstFromEnum(GLenum func) {
		switch (func) {
			case GL_ZERO:					return RasterizerState::BlendFuncDstZero;
			case GL_SRC_COLOR:				return RasterizerState::BlendFuncDstSrcColor;
			case GL_ONE_MINUS_SRC_COLOR:	return RasterizerState::BlendFuncDstOneMinusSrcColor;
			case GL_SRC_ALPHA:				return RasterizerState::BlendFuncDstSrcAlpha;
			case GL_ONE_MINUS_SRC_ALPHA:	return RasterizerState::BlendFuncDstSrcOneMinusSrcAlpha;
			case GL_DST_ALPHA:				return RasterizerState::BlendFuncDstDstAlpha;
			case GL_ONE_MINUS_DST_ALPHA:	return RasterizerState::BlendFuncDstOneMinusDstAlpha;

			default:						// TODO RecordError(GL_INVALID_ENUM);
			case GL_ONE:					return RasterizerState::BlendFuncDstOne;
		}
	}

	RasterizerState::StencilOp StencilOpFromEnum(GLenum op) {
		switch (op) {
			default:
			case GL_KEEP:					return RasterizerState::StencilOpKeep;
			case GL_ZERO:					return RasterizerState::StencilOpZero;
			case GL_REPLACE:				return RasterizerState::StencilOpReplace;
			case GL_INCR:					return RasterizerState::StencilOpIncr;
			case GL_DECR:					return RasterizerState::StencilOpDecr;
			case GL_INVERT:					return RasterizerState::StencilOpInvert;
		}
	}

	RasterizerState::LogicOp LogicOpFromEnum(GLenum op) {
		switch (op) {
			case GL_CLEAR:					return RasterizerState::LogicOpClear;
			case GL_AND:					return RasterizerState::LogicOpAnd;
			case GL_AND_REVERSE:			return RasterizerState::LogicOpAndReverse;
			case GL_COPY:					return RasterizerState::LogicOpCopy;
			case GL_AND_INVERTED:			return RasterizerState::LogicOpAndInverted;
			default:
			case GL_NOOP:					return RasterizerState::LogicOpNoop;
			case GL_XOR:					return RasterizerState::LogicOpXor;
			case GL_OR:						return RasterizerState::LogicOpOr;
			case GL_NOR:					return RasterizerState::LogicOpNor;
			case GL_EQUIV:					return RasterizerState::LogicOpEquiv;
			case GL_INVERT:					return RasterizerState::LogicOpInvert;
			case GL_OR_REVERSE:				return RasterizerState::LogicOpOrReverse;
			case GL_COPY_INVERTED:			return RasterizerState::LogicOpCopyInverted;
			case GL_OR_INVERTED:			return RasterizerState::LogicOpOrInverted;
			case GL_NAND:					return RasterizerState::LogicOpNand;
			case GL_SET:					return RasterizerState::LogicOpSet;
		}
	}
}

void Context :: AlphaFuncx(GLenum func, GLclampx ref) { 
	GetRasterizerState()->SetAlphaFunc(ComparisonFuncFromEnum(func), ref);
}


void Context :: StencilFunc(GLenum func, GLint ref, GLuint mask) {
	GetRasterizerState()->SetStencilFunc(ComparisonFuncFromEnum(func), ref, mask);
}


void Context :: DepthFunc(GLenum func) { 
	GetRasterizerState()->SetDepthFunc(ComparisonFuncFromEnum(func));
}


void Context :: StencilMask(GLuint mask) { 
	GetRasterizerState()->SetStencilMask(mask);
}


void Context :: StencilOp(GLenum fail, GLenum zfail, GLenum zpass) { 
	GetRasterizerState()->SetStencilOp(StencilOpFromEnum(fail), 
		StencilOpFromEnum(zfail), StencilOpFromEnum(zpass));
}


void Context :: DepthMask(GLboolean flag) { 
	GetRasterizerState()->SetDepthMask(flag != 0);
}


void Context :: BlendFunc(GLenum sfactor, GLenum dfactor) { 
	GetRasterizerState()->SetBlendFunc(BlendFuncSrcFromEnum(sfactor), BlendFuncDstFromEnum(dfactor));
}


void Context :: ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { 
	GetRasterizerState()->SetColorMask(red != 0, green != 0, blue != 0, alpha != 0);
}


void Context :: LogicOp(GLenum opcode) { 
	GetRasterizerState()->SetLogicOp(LogicOpFromEnum(opcode));
}


void Context :: PolygonOffsetx(GLfixed factor, GLfixed units) { 
	GetRasterizerState()->SetPolygonOffset(factor, units);
}


void Context :: SampleCoveragex(GLclampx value, GLboolean invert) { 
	GetRasterizerState()->SetSampleCoverage(value, invert != 0);
}


void Context :: ShadeModel(GLenum mode) { 

	switch (mode) {
	case GL_FLAT:		GetRasterizerState()->SetShadeModel(RasterizerState::ShadeModelFlat);	return;
	case GL_SMOOTH:		GetRasterizerState()->SetShadeModel(RasterizerState::ShadeModelSmooth); return;
	default:			RecordError(GL_INVALID_ENUM);											return;
	}
}

