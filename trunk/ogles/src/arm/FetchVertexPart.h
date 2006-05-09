#ifndef EGL_FETCH_VERTEX_PART_H
#define EGL_FETCH_VERTEX_PART_H 1

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


#include "OGLES.h"
#include "PipelinePart.h"
#include "Rasterizer.h"
#include "Arrays.h"


namespace EGL {

	// ----------------------------------------------------------------------
	// Compile-time information for vertex fetch and transformation
	// ----------------------------------------------------------------------

	struct ArrayState {
		GLenum		Type;
		size_t		Size;
		bool		Enabled;
		
		bool operator==(const ArrayState& other) const {
			return
				Enabled == other.Enabled &&
				(!Enabled || (Type == other.Type && Size == other.Size));
		}
		
		void Init(const VertexArray array, bool enabled) {
			Size = array.size;
			Type = array.type;
			Enabled = enabled;
		}
	};

	struct RenderState {
		VaryingInfo	Varying;
		ArrayState	Coord;
		ArrayState	Normal;
		ArrayState	Color;
		ArrayState	TexCoord[EGL_NUM_TEXTURE_UNITS];
		bool		TextureMatrixIdentity[EGL_NUM_TEXTURE_UNITS];
		bool		NeedsNormal;
		bool		NeedsColor;
		bool		NeedsEyeCoords;
		
		bool operator==(const RenderState & other) const {
			if (!(Varying == other.Varying &&
				  Coord == other.Coord &&
				  Normal == other.Normal &&
				  Color == other.Color &&
				  NeedsNormal == other.NeedsNormal &&
				  NeedsColor == other.NeedsColor &&
				  NeedsEyeCoords == other.NeedsEyeCoords))
				return false;
				
			size_t index = EGL_NUM_TEXTURE_UNITS;
			
			while (index-- > 0)
				if (!(TextureMatrixIdentity[index] == other.TextureMatrixIdentity[index] &&
					  TexCoord[index] == other.TexCoord[index]))
					return false;
					
			return true;			
		}
	};

	// ----------------------------------------------------------------------
	// Runtime information for vertex fetch and transformation
	// ----------------------------------------------------------------------

	struct ArrayInfo {
		const void * 	Base;
		GLsizei			Stride;
		
		void Init(VertexArray & array) {
			Base = array.effectivePointer;
			Stride = array.stride;
		}
	};

	struct RenderInfo {
		ArrayInfo	Coord;
		ArrayInfo	Normal;
		ArrayInfo	Color;
		ArrayInfo	TexCoord[EGL_NUM_TEXTURE_UNITS];
		const GLfixed *	ModelviewProjectionMatrix;
		const GLfixed * ModelviewMatrix;
		const GLfixed * InvModelviewMatrix;
		const GLfixed * TextureMatrix[EGL_NUM_TEXTURE_UNITS];
	};

	class FetchVertexPart: public PipelinePart {
	public:
		void CopyState(void * target, const void * source) const;
		bool CompareState(const void * first, const void * second) const;
		void Compile(FunctionCache * target, const VaryingInfo * varyingInfo, const void * state);
		Part GetPart() const;
		
	private:
		// ----------------------------------------------------------------------
		// Code generation of vertex processing
		// ----------------------------------------------------------------------

		void GenerateFetch(const RenderState * state);

		// fetch vertex coordinates and transform by given matrix
		void GenerateCoordFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase,
			GLenum type, size_t size);

		void GenerateCoordFetchMultiply2(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateCoordFetchMultiply3(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateCoordFetchMultiply4(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		// fetch normal and transform by given matrix
		void GenerateNormalFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase,
			GLenum type);

		// fetch color
		void GenerateColorFetch(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			GLenum type);

		// fetch tex coords and transform by given matrix if not null
		void GenerateTexCoordFetchMultiply(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type, size_t size);

		void GenerateTexCoordFetchMultiply2(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateTexCoordFetchMultiply3(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		void GenerateTexCoordFetchMultiply4(cg_block_t * block,
			cg_virtual_reg_t * vectorBase, cg_virtual_reg_t * resultBase, size_t resultOffset,
			cg_virtual_reg_t * matrixBase, GLenum type);

		// load a specific element of a matrix
		cg_virtual_reg_t * LoadMatrixElement(cg_block_t * block,
				cg_virtual_reg_t * base, int row, int column);

		cg_virtual_reg_t * LoadVectorElement(cg_block_t * block, cg_virtual_reg_t * base, int element, GLenum type);

	
	};
	
}

#endif // ndef EGL_FETCH_VERTEX_PART_H