// ==========================================================================
//
// triVMcodegen.cpp
//				triVM Intermediate Language for OpenGL (R) ES Implementation
//				Emit ARM instructions from intermediate code
//				
//				This class is part of the runtime compiler infrastructure
//				used by the OpenGL|ES implementation for compiling
//				shader code at runtime into machine language.
//
// --------------------------------------------------------------------------
//
// 01-19-2004	Hans-Martin Will	initial version
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
#include "CodeGenerator.h"
#include "fixed.h"
#include "trivm.h"
#include "Inst.h"
#include "Sweep.h"
#include "Transform.h"
#include "arm-codegen.h"


using namespace EGL;
using namespace triVM;


namespace {

	ARMShiftType ShiftTypeFromOpcode(Opcode opcode) {
		switch (opcode) {
			default:
				assert(false);

			case lsl:
				return ARMSHIFT_LSL;

			case asr:
				return ARMSHIFT_ASR;

			case lsr:
				return ARMSHIFT_LSR;

		}
	}

	class CodeEmitter: public Sweep {

		arminstr_t * PC;						// instruction pointer

	protected:
		void begin(triVM::Module * module) { 
		}

		void begin(triVM::Procedure * procedure) { 
		}

	private:

		// data structure:
		// for each physical register, maintain a reference to the virtual register that is
		// currently allocated to it
		// maintain clean/dirty flag

		// for each virtual register, maintain life-flag
		// maintain physcial register currently allocated
		// maintain spill location allocated


		int AllocateTempRegister() {
		}

		void ReleaseTempRegister(int physicalRegister) {
		}

		void KillPhysicalRegister(int physicalRegister) {
		}

	public:
		virtual void sweep(triVM::Module * module) {
			Sweep::sweep(module);
		}

		virtual void sweep(triVM::Block * block) {
			Sweep::sweep(block);
			// TO DO: this actually needs to be replaced by the following instruction scheduling code:
			
			// create a priority list of instructions that can be executed
			// start with a virtual clock; an instruction can be executed at clock t if all operands are available
			// at that point in time

			// create a map of virtual registers to its instructions; this will be used to update the
			// priority list of scheduled instructions whenever one of the input registers becomes available

			// each instruction maintains a counter of registers that have not been become available yet; once the
			// counter goes 0 the insturction is inserted into the priority list at the maximum time of the
			// availability of all operands

			// for each virtual register not allocated to a physical register, add a spilling cost to all instructions using
			// this register in the scheduling queue

			// whenever a registers gets allocated, remove the spilling cost from all instructions that are in the ready list
			// and that might use the register value

			// picking of next instruction:
			//	take the instruction with minimum time from the queue
			//	if more than one instruction coul dbe selected, break ties:
			//		- prefer a memory instruction over a non-memory instruction, given that theprevious instructions
			//			was not a memory instruction
			//		- if more than one instruction satisfy the previous tie-break, select the instruction that has a
			//			successor instruction with minimum number of unavailable operands (this ensures locality of reference)

			// spilling of registers:
			//	the operand for an instruction has been spilled and needs to be refetched.
			//	ideally, this refetching would be reflected in the scheduling: If an instruction is in the queue but has
			//	a non-minimum scheduling time because of spilled arguments, emit these fetches earlier in order to prefetch
			//	the required operands
			//		--> how about adding fetch instructions to the set of instructions for the block whenever a virtual
			//			register gets spilled to memory? If enough physical registers are available, this should never
			//			create an endless loop.
			//
			//  Big question: How can we determine as early as possible if a register will need to be spilled?
			//		If it needs to be spilled, this should be done as early as possible, so that we can free up
			//		the physical register avoiding unecessary secondary spills.
			//		Is there any heuristic based on the next use of a register?
			//		Spilling a live register costs 2 memory operations
			//		Recap: Only determining what needs to be written out and when is relevant for the problem,
			//		once it has been decided that a value is to be spilled, the refetch of the register comes
			//		"for free".
			//		We could mark an insertion point for spills, i.e. right after the last use of a register;
			//		or we add a virtual store instruction and remove it later if it's not needed.
			//		This means: Scheduling before actual instruction encoding...
			//
			//	2 situations: no dead register is available for reallocation, so a live register needs to be spilled

		}

	protected:
		void end(triVM::Procedure * procedure) { 
			// create procedure epilogue
		}

		void end(triVM::Block * block) { 
			// store any global variables into memory locations unless they have been
			// assigned to a register. If that's the case, ensure that the variables
			// are stored in the correct register.
		}

	protected:
		// base formats
		void visit(triVM::InstructionUnaryType * instruction) { 

			bool flags = instruction->rC != -1;

			switch (instruction->opcode) {
				case not:	
					if (flags) {
						ARM_MVNS_REG_REG(PC, instruction->rD, instruction->rS);
					} else {
						ARM_MVN_REG_REG(PC, instruction->rD, instruction->rS);
					}

					break;

				case neg:	
				case fneg:	
					ARM_EOR_REG_REG(PC, instruction->rD, instruction->rD, instruction->rD);

					if (flags) {
						ARM_SUBS_REG_REG(PC, instruction->rD, instruction->rD, instruction->rS);
					} else {
						ARM_SUB_REG_REG(PC, instruction->rD, instruction->rD, instruction->rS);
					}

					break;

				case fcnv:	
					if (flags) {
						ARM_MOVS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASL, EGL_PRECISION);
					} else {
						ARM_MOV_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASL, EGL_PRECISION);
					}

					break;

				case trunc:	
					if (flags) {
						ARM_MOVS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASR, EGL_PRECISION);
					} else {
						ARM_MOV_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASR, EGL_PRECISION);
					}

					break;

				case round:	
					ARM_ADD_REG_IMM8(PC, instruction->rD, instruction->rS, 1);

					if (flags) {
						ARM_MOVS_REG_IMMSHIFT(PC, instruction->rD, instruction->rD, ARMSHIFT_ASR, EGL_PRECISION);
					} else {
						ARM_MOV_REG_IMMSHIFT(PC, instruction->rD, instruction->rD, ARMSHIFT_ASR, EGL_PRECISION);
					}

					break;


				case finv:	
					// Generate Call into gppInv_16_32s
					// generate function call or inline code sequence?

					// Allocate r0, or try to generate argument into r0
					// If argument not in r0, move argument into r0
					
					// Allocate r1
					// add		r1, sp, #offset

					// kill all registers affected by calling conventions
					// as non-preserved

					// bl       gppDiv_16_32s

					// mark result register as being spilled into sp + #offset

				case fsqrt:					
					// generate function call or inline code sequence?

					;

				default:
					assert(false);
			}
		}	

		void visit(triVM::InstructionBinaryType * instruction) { 

			bool flags = instruction->rC != -1;

			switch (instruction->opcode) {
				case add:	
				case fadd:	
					// this assumes that we already mapped those virtual registers to physical registers
					if (flags) {
						ARM_ADDS_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					} else {
						ARM_ADD_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					}

					break;

				case and:	
					if (flags) {
						ARM_ANDS_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					} else {
						ARM_AND_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					}

					break;

				case or:		
					if (flags) {
						ARM_ORRS_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					} else {
						ARM_ORR_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					}

					break;

				case sub:	
				case fsub:	
					if (flags) {
						ARM_SUBS_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					} else {
						ARM_SUB_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					}

					break;

				case xor:									
					if (flags) {
						ARM_EORS_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					} else {
						ARM_EOR_REG_REG(PC, instruction->rD, instruction->rS, instruction->rM);
					}

					break;

				case asr:	
					if (flags) {
						// These (and the following) register assignments are actually correct: 
						// In triVM, rS is the source and rM is the modifier register
						// In ARM assembly language, rm holds the shifter operand base value and rs is the shift register
						ARM_MOVS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASR, instruction->rM);
					} else {
						ARM_MOV_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_ASR, instruction->rM);
					}

					break;

				case lsl:	
					if (flags) {
						ARM_MOVS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_LSL, instruction->rM);
					} else {
						ARM_MOV_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_LSL, instruction->rM);
					}

					break;

				case lsr:	
					if (flags) {
						ARM_MOVS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_LSR, instruction->rM);
					} else {
						ARM_MOV_REG_REGSHIFT(PC, instruction->rD, instruction->rS, ARMSHIFT_LSR, instruction->rM);
					}

					break;


				case mul:	
				case fmul:	

				case mod:	
				case umod:	

				case triVM::div:	
				case udiv:	
				case fdiv:	
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionCompareType * instruction) { 
			switch (instruction->opcode) {
				case cmp:	
				case fcmp:	
					ARM_CMP_REG_REG(PC, instruction->rS, instruction->rC);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionLoadType * instruction) { 
			switch (instruction->opcode) {
				case ldb:	
					ARM_LDRB_IMM(PC, instruction->rD, instruction->rS, 0);
					break;

				case ldh:	
					ARM_LDRH_IMM(PC, instruction->rD, instruction->rS, 0);
					break;

				case ldw:	
					ARM_LDR_IMM(PC, instruction->rD, instruction->rS, 0);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionStoreType * instruction) { 
			switch (instruction->opcode) {
				case stb:	
					ARM_STRB_IMM(PC, instruction->rS, instruction->rD, 0);
					break;

				case sth:	
					ARM_STRH_IMM(PC, instruction->rS, instruction->rD, 0);
					break;

				case stw:					
					ARM_STR_IMM(PC, instruction->rS, instruction->rD, 0);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionLoadImmediateType * instruction) { 
			switch (instruction->opcode) {
				case ldi:	
					// if immediate value fits in imm8/rot format emit MOV instruction
					// otherwise, create
					//	BLR		PC + 2
					//  dw		literal
					//	LDR		rd, lr

					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionBranchRegType * instruction) { 
			switch (instruction->opcode) {
				case bra:
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionBranchLabelType * instruction) { 
			switch (instruction->opcode) {
				case bra:
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionBranchConditionallyType * instruction) { 
			switch (instruction->opcode) {
				case bae:	
				case bbl:	
				case beq:	
				case bge:	
				case ble:	
				case bgt:	
				case blt:	
				case bne:	
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionPhiType * instruction) { 
			// ignore?
		}

		void visit(triVM::InstructionCallType * instruction) { 
			assert(0);
		}

		void visit(triVM::InstructionRetType * instruction) { 
			// create procedure epilogue/return code
		}


		// ARM-specific formats
		void visit(triVM::InstructionArmUnaryImmediateType * instruction) { 
			switch (instruction->opcode) {
				case neg:	
				case not:	
				case fneg:	
				case finv:	
				case fsqrt:					
				case trunc:	
				case round:	
				case fcnv:									
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmUnaryShiftRegType * instruction) { 
			switch (instruction->opcode) {
				case neg:	
				case not:	
				case fneg:	
				case finv:	
				case fsqrt:					
				case trunc:	
				case round:	
				case fcnv:															
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmUnaryShiftImmedType * instruction) { 
			switch (instruction->opcode) {
				case neg:	
				case not:	
				case fneg:	
				case finv:	
				case fsqrt:					
				case trunc:	
				case round:	
				case fcnv:													
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmBinaryImmediateType * instruction) { 
			switch (instruction->opcode) {
				case add:	
				case and:	
				case asr:	
				case triVM::div:	
				case lsl:	
				case lsr:	
				case mod:	
				case mul:	
				case or:		
				case sub:	
				case udiv:	
				case umod:	
				case xor:									
				case fadd:	
				case fdiv:	
				case fmul:	
				case fsub:	
					;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmBinaryShiftRegType * instruction) { 
			bool flags = instruction->rC != -1;

			ARMShiftType shiftType = ShiftTypeFromOpcode(instruction->opShift);

			switch (instruction->opcode) {
				case add:	
				case fadd:	
					// this assumes that we already mapped those virtual registers to physical registers
					if (flags) {
						ARM_ADDS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					} else {
						ARM_ADD_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					}

					break;

				case and:	
					if (flags) {
						ARM_ANDS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					} else {
						ARM_AND_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					}

					break;

				case or:		
					if (flags) {
						ARM_ORRS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					} else {
						ARM_ORR_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					}

					break;

				case sub:	
				case fsub:	
					if (flags) {
						ARM_SUBS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					} else {
						ARM_SUB_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					}

					break;

				case xor:									
					if (flags) {
						ARM_EORS_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					} else {
						ARM_EOR_REG_REGSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->rShift);
					}

					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmBinaryShiftImmedType * instruction) { 
			bool flags = instruction->rC != -1;

			ARMShiftType shiftType = ShiftTypeFromOpcode(instruction->opShift);

			switch (instruction->opcode) {
				case add:	
				case fadd:	
					// this assumes that we already mapped those virtual registers to physical registers
					if (flags) {
						ARM_ADDS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					} else {
						ARM_ADD_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					}

					break;

				case and:	
					if (flags) {
						ARM_ANDS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					} else {
						ARM_AND_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					}

					break;

				case or:		
					if (flags) {
						ARM_ORRS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					} else {
						ARM_ORR_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					}

					break;

				case sub:	
				case fsub:	
					if (flags) {
						ARM_SUBS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					} else {
						ARM_SUB_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					}

					break;

				case xor:									
					if (flags) {
						ARM_EORS_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					} else {
						ARM_EOR_REG_IMMSHIFT(PC, instruction->rD, instruction->rS, instruction->rM, shiftType, instruction->shiftBits);
					}

					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmLoadImmedOffsetType * instruction) { 

			int offset = instruction->immed12;

			switch (instruction->opcode) {
				case ldb:	
					ARM_LDRB_IMM(PC, instruction->rD, instruction->rS, offset);
					break;

				case ldh:	
					ARM_LDRH_IMM(PC, instruction->rD, instruction->rS, offset);
					break;

				case ldw:	
					ARM_LDR_IMM(PC, instruction->rD, instruction->rS, offset);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmLoadRegOffsetType * instruction) { 
			switch (instruction->opcode) {
				case ldb:	
					ARM_LDRB_REG_REG(PC, instruction->rD, instruction->rS, instruction->rOffset);
					break;

				case ldh:	
					ARM_LDRH_REG_REG(PC, instruction->rD, instruction->rS, instruction->rOffset);
					break;

				case ldw:	
					ARM_LDR_REG_REG(PC, instruction->rD, instruction->rS, instruction->rOffset);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmStoreImmedOffsetType * instruction) { 
			switch (instruction->opcode) {
				case stb:	
					ARM_STRB_IMM(PC, instruction->rS, instruction->rD, instruction->immed12);
					break;

				case sth:	
					ARM_STRH_IMM(PC, instruction->rS, instruction->rD, instruction->immed12);
					break;

				case stw:					
					ARM_STR_IMM(PC, instruction->rS, instruction->rD, instruction->immed12);
					break;

				default:
					assert(false);
			}
		}

		void visit(triVM::InstructionArmStoreRegOffsetType * instruction) { 
			switch (instruction->opcode) {
				case stb:	
					ARM_STRB_REG_REG(PC, instruction->rS, instruction->rD, instruction->rOffset);
					break;

				case sth:	
					ARM_STRH_REG_REG(PC, instruction->rS, instruction->rD, instruction->rOffset);
					break;

				case stw:					
					ARM_STR_REG_REG(PC, instruction->rS, instruction->rD, instruction->rOffset);
					break;

				default:
					assert(false);
			}
		}

	};
}

void CodeGenerator :: EmitCode(Module * module) {

	CodeEmitter emitter;
	emitter.sweep(module);
}

