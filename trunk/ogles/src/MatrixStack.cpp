// ==========================================================================
//
// MatrixStack.cpp	Matrix Stack Class for Embedded OpenGL Implementation
//
// --------------------------------------------------------------------------
//
// 08-08-2003	Hans-Martin Will	initial version
// 10-04-2003	Hans-Martin Will	rework against new matrix class
//
// ==========================================================================


#include "stdafx.h"
#include "MatrixStack.h"


using namespace EGL;


MatrixStack :: MatrixStack(I32 maxStackElements)
:	m_StackSize(maxStackElements),
	m_StackPointer(0)
{
	m_Stack = new Matrix4x4[maxStackElements];
}


MatrixStack :: ~MatrixStack() {
	delete[] m_Stack;
}


bool MatrixStack :: PopMatrix(void) {
	if (m_StackPointer > 0) {
		--m_StackPointer;
		return true;
	} else {
		return false;
	}
}


bool MatrixStack :: PushMatrix(void) {

	if (m_StackPointer < m_StackSize - 1) {
		m_Stack[m_StackPointer + 1] = m_Stack[m_StackPointer];
		++m_StackPointer;
		return true;
	} else {
		return false;
	}
}


void MatrixStack :: MultMatrix(const Matrix4x4& matrix) {
	LoadMatrix(CurrentMatrix() * matrix);
}


void MatrixStack :: LoadIdentity(void) {
	CurrentMatrix().MakeIdentity();
}


void MatrixStack :: LoadMatrix(const Matrix4x4& matrix) {
	CurrentMatrix() = matrix;
}
