#ifndef EGL_MATRIX_STACK_H
#define EGL_MATRIX_STACK_H 1

#pragma once

// ==========================================================================
//
// MatrixStack.h	Matrix Stack Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-08-2003	Hans-Martin Will	initial version
// 10-04-2003	Hans-Martin Will	rework against new matrix class
//
// ==========================================================================



#include "OGLES.h"
#include "fixed.h"
#include "linalg.h"


namespace EGL {

	class OGLES_API MatrixStack {

	public:

		MatrixStack(I32 maxStackElements);
		~MatrixStack();

		bool PopMatrix(void);
		bool PushMatrix(void);

		void MultMatrix(const Matrix4x4 &matrix);

		void LoadIdentity(void);
		void LoadMatrix(const Matrix4x4 &matrix);

		inline Matrix4x4 & CurrentMatrix() {
			return m_Stack[m_StackPointer];
		}

	private:

		Matrix4x4	*m_Stack;
		I32				m_StackPointer;
		I32				m_StackSize;
	};
}

#endif //ndef EGL_MATRIX_STACK_H