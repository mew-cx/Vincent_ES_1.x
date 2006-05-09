// ==========================================================================
//
// FetchVertexPart.h	JIT Class for 3D Rendering Library
//
//						This class encapsulates the specifics for managing
//						code generation of vertex fetching
//
// --------------------------------------------------------------------------
//
// 04-24-2006	Hans-Martin Will	initial version
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
#include "FetchVertexPart.h"
#include "inline.h"


using namespace EGL;


// ----------------------------------------------------------------------
// Code generation of vertex processing
// ----------------------------------------------------------------------

// fetch vertex coordinates and transform by given matrix
void FetchVertexPart :: GenerateCoordFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type, size_t size) {
	switch (size) {
	case 2:
		GenerateCoordFetchMultiply2(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 3:
		GenerateCoordFetchMultiply3(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 4:
		GenerateCoordFetchMultiply4(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;
	}
}

void FetchVertexPart :: GenerateCoordFetchMultiply2(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);

		DECL_REG	(s01);
		DECL_REG	(s013);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);
		FADD		(s013, s01, m3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s013);
	}
}

void FetchVertexPart :: GenerateCoordFetchMultiply3(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {


	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);

		DECL_REG	(s01);
		DECL_REG	(s012);
		DECL_REG	(s0123);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);
		FADD		(s0123, s012, m3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s0123);
	}
}

void FetchVertexPart :: GenerateCoordFetchMultiply4(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
	cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

	for (int row = 0; row < 4; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);
		DECL_REG	(p3);

		DECL_REG	(s01);
		DECL_REG	(s012);
		DECL_REG	(s0123);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);

		cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);
		FMUL		(p3, m3, v3);
		FADD		(s0123, s012, p3);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s0123);
	}
}

// fetch normal and transform by given matrix
void FetchVertexPart :: GenerateNormalFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type) {

	cg_proc_t * procedure = block->proc;

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

	for (int row = 0; row < 3; ++row) {
		DECL_REG	(p0);
		DECL_REG	(p1);
		DECL_REG	(p2);

		DECL_REG	(s01);
		DECL_REG	(s012);

		cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
		cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

		FMUL		(p0, m0, v0);

		cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

		FMUL		(p1, m1, v1);
		FADD		(s01, p0, p1);
		FMUL		(p2, m2, v2);
		FADD		(s012, s01, p2);

		STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), s012);
	}
}

// fetch color
void FetchVertexPart :: GenerateColorFetch(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	GLenum type) {

	cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
	cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
	STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
	cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
	STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);
	STORE_DATA(block, resultBase, resultOffset + 2 * sizeof(EGL_Fixed), v2);
	STORE_DATA(block, resultBase, resultOffset + 3 * sizeof(EGL_Fixed), v3);
}

// fetch tex coords and transform by given matrix if not null
void FetchVertexPart :: GenerateTexCoordFetchMultiply(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase,
	GLenum type, size_t size) {
	switch (size) {
	case 2:
		GenerateTexCoordFetchMultiply2(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 3:
		GenerateTexCoordFetchMultiply3(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;

	case 4:
		GenerateTexCoordFetchMultiply4(block, vectorBase, resultBase, resultOffset, matrixBase, type);
		break;
	}
}

void FetchVertexPart :: GenerateTexCoordFetchMultiply2(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);

			DECL_REG	(s01);
			DECL_REG	(s013);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);
			FADD		(s013, s01, m3);

			FINV		(inverse, s013);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);

			DECL_REG	(s01);
			DECL_REG	(s013);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);
			FADD		(s013, s01, m3);
			FMUL		(prod, s013, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

void FetchVertexPart :: GenerateTexCoordFetchMultiply3(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);

		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0);
		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, 3, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FADD		(s0123, s012, m3);

			FINV		(inverse, s0123);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FADD		(s0123, s012, m3);

			FMUL		(prod, s0123, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

void FetchVertexPart :: GenerateTexCoordFetchMultiply4(cg_block_t * block,
	cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
	cg_virtual_reg_t * matrixBase, GLenum type) {

	cg_proc_t * procedure = block->proc;

	if (!matrixBase) {
		cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

		DECL_REG		(invV3);
		DECL_REG		(v0InvV3);
		DECL_REG		(v1InvV3);

		FINV			(invV3, v3);

		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);

		FMUL			(v0InvV3, v0, invV3);

		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		STORE_DATA(block, resultBase, resultOffset + 0 * sizeof(EGL_Fixed), v0InvV3);

		FMUL			(v1InvV3, v1, invV3);

		STORE_DATA(block, resultBase, resultOffset + 1 * sizeof(EGL_Fixed), v1InvV3);
	} else {
		cg_virtual_reg_t * v0  = LoadVectorElement(block, vectorBase, 0, type);
		cg_virtual_reg_t * v1  = LoadVectorElement(block, vectorBase, 1, type);
		cg_virtual_reg_t * v2  = LoadVectorElement(block, vectorBase, 2, type);
		cg_virtual_reg_t * v3  = LoadVectorElement(block, vectorBase, 3, type);

		DECL_REG		(inverse);

		{
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);
			DECL_REG	(p3);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, 3, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, 3, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, 3, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, 3, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FMUL		(p3, m3, v3);
			FADD		(s0123, s012, p3);

			FINV		(inverse, s0123);
		}

		for (int row = 0; row < 2; ++row) {
			DECL_REG	(p0);
			DECL_REG	(p1);
			DECL_REG	(p2);
			DECL_REG	(p3);

			DECL_REG	(s01);
			DECL_REG	(s012);
			DECL_REG	(s0123);

			DECL_REG	(prod);

			cg_virtual_reg_t * m0 = LoadMatrixElement(block, matrixBase, row, 0);
			cg_virtual_reg_t * m1 = LoadMatrixElement(block, matrixBase, row, 1);

			FMUL		(p0, m0, v0);

			cg_virtual_reg_t * m2 = LoadMatrixElement(block, matrixBase, row, 2);

			FMUL		(p1, m1, v1);
			FADD		(s01, p0, p1);

			cg_virtual_reg_t * m3 = LoadMatrixElement(block, matrixBase, row, 3);

			FMUL		(p2, m2, v2);
			FADD		(s012, s01, p2);
			FMUL		(p3, m3, v3);
			FADD		(s0123, s012, p3);

			FMUL		(prod, s0123, inverse);

			STORE_DATA(block, resultBase, resultOffset + row * sizeof(EGL_Fixed), prod);
		}
	}
}

// load a specific element of a matrix
cg_virtual_reg_t * FetchVertexPart :: LoadMatrixElement(cg_block_t * block,
													  cg_virtual_reg_t * base, int row, int column) {
	int index = sizeof(EGL_Fixed) * (row + column * 4);	// offset of element in 4 by 4 matrix

	cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
	cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
	cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

	LDI(offset, index);
	ADD(addr, base, offset);
	LDW(value, addr);

	return value;
}

cg_virtual_reg_t * FetchVertexPart :: LoadVectorElement(cg_block_t * block, 
													  cg_virtual_reg_t * base, int element, GLenum type) {
	cg_proc_t * procedure = block->proc;

	switch (type) {
	case GL_BYTE:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedRight = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num24, 24);
			DECL_CONST_REG	(num8, 8);

			LDI		(offset, element);
			ADD		(addr, base, offset);
			LDB		(value, addr);
			LSL		(shiftedLeft, value, num24);			/* sign extend and convert to 16.16 */
			ASR		(shiftedRight, shiftedLeft, num8);

			return shiftedRight;
		}
		break;

	case GL_SHORT:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num16, 16);

			LDI		(offset, element * sizeof(GLshort));
			ADD		(addr, base, offset);
			LDH		(value, addr);
			LSL		(shiftedLeft, value, num16);			/* sign extend and convert to 16.16 */

			return shiftedLeft;
		}
		break;

	case GL_FIXED:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			LDI		(offset, element * sizeof(EGL_Fixed));
			ADD		(addr, base, offset);
			LDW		(value, addr);

			return value;
		}
		break;

	case GL_FLOAT:
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * result = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			LDI		(offset, element * sizeof(EGL_Fixed));
			ADD		(addr, base, offset);
			LDW		(value, addr);
			FCONVFLT(result, value);

			return result;
		}
		break;

	case GL_UNSIGNED_BYTE:
		/* for color coordinates! */
		{
			cg_virtual_reg_t * offset = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * addr = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * value = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedLeft = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * shiftedRight = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * ored = cg_virtual_reg_create(block->proc, cg_reg_type_general);
			cg_virtual_reg_t * sum = cg_virtual_reg_create(block->proc, cg_reg_type_general);

			DECL_CONST_REG	(num8, 8);
			DECL_CONST_REG	(num7, 7);

			LDI		(offset, element);
			ADD		(addr, base, offset);
			LDB		(value, addr);
			LSL		(shiftedLeft, value, num8);			/* sign extend and convert to 16.16 */
			LSR		(shiftedRight, value, num7);
			OR		(ored, value, shiftedLeft);
			ADD		(sum, ored, shiftedRight);

			return sum;
		}
		break;

	}

	return 0;
}

void FetchVertexPart :: GenerateFetch(const RenderState * state) {

	cg_proc_t * procedure = cg_proc_create(m_Module);

	// signature is (const RenderInfo * info, int index, Vertx * result)

	DECL_REG	(regRenderInfo);// RenderInfo structure pointer
	DECL_REG	(regIndex);		// Index of element to fetch
	DECL_REG	(regResult);	// Pointer to resulting vertex

	procedure->num_args = 3;	// the previous three declarations make up the arguments

	cg_block_t * block = cg_block_create(procedure, 1);

	cg_virtual_reg_t *	coordStride	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COORD_STRIDE);
	cg_virtual_reg_t *	coordBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COORD_BASE);

	DECL_REG	(coordOffset);
	DECL_REG	(coordVector);

	MUL			(coordOffset, coordStride, regIndex);

	cg_virtual_reg_t *	mvp			= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_MVP);
	ADD			(coordVector, coordBase, coordOffset);

	GenerateCoordFetchMultiply(block, coordVector, regResult, OFFSET_RASTER_POS_CLIP, mvp, state->Coord.Type, state->Coord.Size);

	if (state->NeedsEyeCoords) {
		// we might be able to optimize this to avoid reloading coords
		cg_virtual_reg_t *	mv		= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_MV);
		GenerateCoordFetchMultiply(block, coordVector, regResult, OFFSET_RASTER_POS_EYE, mv, state->Coord.Type, state->Coord.Size);
	}

	if (state->NeedsNormal && state->Normal.Enabled) {
		cg_virtual_reg_t *	normalStride= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_NORMAL_STRIDE);
		cg_virtual_reg_t *	normalBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_NORMAL_BASE);

		DECL_REG	(normalOffset);
		DECL_REG	(normalVector);

		MUL			(normalOffset, normalStride, regIndex);

		cg_virtual_reg_t *	invMv			= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_INV_MV);
		ADD			(normalVector, normalBase, normalOffset);

		GenerateNormalFetchMultiply(block, normalVector, regResult, OFFSET_RASTER_POS_NORMAL, invMv, state->Normal.Type);
	}

	if (state->NeedsColor && state->Color.Enabled) {
		cg_virtual_reg_t *	colorStride= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COLOR_STRIDE);
		cg_virtual_reg_t *	colorBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_COLOR_BASE);

		DECL_REG	(colorOffset);
		DECL_REG	(colorVector);

		MUL			(colorOffset, colorStride, regIndex);
		ADD			(colorVector, colorBase, colorOffset);

		GenerateColorFetch(block, colorVector, regResult, OFFSET_RASTER_POS_RAW_COLOR, state->Color.Type);
	}

	for (size_t unit = 0; unit < EGL_NUM_TEXTURE_UNITS; ++unit) {
		if (state->Varying.textureBase[unit] >= 0 && state->TexCoord[unit].Enabled) {
			cg_virtual_reg_t *	texCoordStride	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX_COORD_STRIDE(unit));
			cg_virtual_reg_t *	texCoordBase	= LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX_COORD_BASE(unit));

			DECL_REG	(texCoordOffset);
			DECL_REG	(texCoordVector);

			MUL			(texCoordOffset, texCoordStride, regIndex);

			cg_virtual_reg_t *	tcm = 0;

			if (!state->TextureMatrixIdentity[unit]) {
				tcm = LOAD_DATA(block, regRenderInfo, OFFSET_RENDER_INFO_TEX(unit));
			}

			ADD			(texCoordVector, texCoordBase, texCoordOffset);

			GenerateTexCoordFetchMultiply(block, texCoordVector, regResult, 
				offsetof(Vertex, m_Varying) + state->Varying.textureBase[unit] * sizeof(U32),
				tcm, state->TexCoord[unit].Type, state->TexCoord[unit].Size);
		}
	}

	RET();
}

// --------------------------------------------------------------------------
// Implementation of virtual functions in PipelinePart
// --------------------------------------------------------------------------

void FetchVertexPart :: CopyState(void * target, const void * source) const {
	memmove(target, source, sizeof(RenderState));
}

bool FetchVertexPart :: CompareState(const void * first, const void * second) const {
	const RenderState * firstState = static_cast<const RenderState *>(first);
	const RenderState * secondState = static_cast<const RenderState *>(second);
	
	return !memcmp(firstState, secondState, sizeof(RenderState));
}

void FetchVertexPart :: Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state) {
	BeginGenerateCode();
	GenerateFetch(static_cast<const RenderState *>(state));
	EndGenerateCode(target, state);
}

PipelinePart::Part FetchVertexPart :: GetPart() const {
	return PipelinePart::PartFetchVertex;
}


