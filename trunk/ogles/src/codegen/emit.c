/****************************************************************************/
/*																			*/
/* Copyright (c) 2004, Hans-Martin Will. All rights reserved.				*/
/*																			*/
/* Redistribution and use in source and binary forms, with or without		*/
/* modification, are permitted provided that the following conditions are   */
/* met:																		*/
/*																			*/
/* *  Redistributions of source code must retain the above copyright		*/
/*    notice, this list of conditions and the following disclaimer.			*/
/*																			*/
/* *  Redistributions in binary form must reproduce the above copyright		*/
/*    notice, this list of conditions and the following disclaimer in the   */
/*    documentation and/or other materials provided with the distribution.  */
/*																			*/
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS		*/
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT		*/
/* LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A   */
/* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER */
/* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,		*/
/* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR		*/
/* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF   */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING		*/
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS		*/
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.				*/
/*																			*/
/****************************************************************************/


#include "emit.h"
#include "bitset.h"
#include "heap.h"
#include "segment.h"
#include "arm-codegen.h"

#include <malloc.h>


typedef struct reference_t
{
	struct reference_t *	next;
	cg_reference_type_t		ref_type;
	size_t					offset;
}
reference_t;


struct cg_label_t 
{
	cg_label_t *			next;
	reference_t *			refs;
	size_t					offset;
};


typedef enum physical_reg_status_t
{
	reg_status_init,
	reg_status_free,
	reg_status_allocated,
	reg_status_defined,
	reg_status_dirty,
	reg_status_secondary
}
physical_reg_status_t;


typedef enum physical_reg_event_t
{
	reg_event_init,
	reg_event_allocate,
	reg_event_deallocate,
	reg_event_load,
	reg_event_save,
	reg_event_define,
	reg_event_reassign
}
physical_reg_event_t;


/****************************************************************************/
/* This method implements the finite state machine underlying physical		*/
/* register allocation.														*/
/****************************************************************************/
static physical_reg_status_t update_register_status(physical_reg_status_t state, 
													physical_reg_event_t event)
{
	if (event == reg_event_init)
	{
		return reg_status_init;
	}

	switch (state)
	{
	case reg_status_init:
		switch (event)
		{
		case (reg_event_init):
			return reg_status_free;

		default:
			assert(0);
		}
		break;

	case reg_status_free:
		switch (event)
		{
		case (reg_event_allocate):
			return reg_status_allocated;

		default:
			assert(0);
		}
		break;

	case reg_status_allocated:
		switch (event)
		{
		case reg_event_load:
			return reg_status_defined;

		case reg_event_define:
			return reg_status_dirty;

		default:
			assert(0);
		}
		break;

	case reg_status_defined:
		switch (event)
		{
		case reg_event_deallocate:
			return reg_status_free;

		case reg_event_reassign:
			return reg_status_secondary;

		default:
			assert(0);
		}
		break;

	case reg_status_dirty:
		switch (event)
		{
		case reg_event_save:
			return reg_status_defined;

		default:
			assert(0);
		}
		break;

	case reg_status_secondary:
		switch (event)
		{
		case reg_event_define:
			return reg_status_dirty;

		case reg_event_reassign:
			return reg_status_defined;

		default:
			assert(0);
		}
		break;

	default:
		assert(0);
		break;
	}

	return reg_status_init;
}


struct physical_reg_list_t;

typedef struct cg_physical_reg_t 
{
	struct cg_physical_reg_t * prev;				/* for LRU chain			*/
	struct cg_physical_reg_t * next;				/* for LRU chain			*/
	struct physical_reg_list_t * list;
	ARMReg					regno;				/* physical register		*/
	cg_virtual_reg_t *		virtual_reg;		/* assigned virtual reg.	*/
	cg_inst_t *				next_use;			/* next use of ass. value   */
	physical_reg_status_t	state;				/* current register state	*/
	int						dirty: 1;
	int						defined:1;
}
cg_physical_reg_t;


typedef struct physical_reg_list_t
{
	cg_physical_reg_t *		head;
	cg_physical_reg_t *		tail;	
}
physical_reg_list_t;


void reg_list_add(physical_reg_list_t * list, cg_physical_reg_t * reg)
{
	assert(!reg->list);
	assert(!reg->prev);
	assert(!reg->next);
	
	if (list->tail == (cg_physical_reg_t *) 0)
	{
		assert(list->head == (cg_physical_reg_t *) 0);
		list->head = list->tail = reg;
		reg->prev = reg->next = NULL;
	}
	else 
	{
		assert(list->head != (cg_physical_reg_t *) 0);
		reg->prev = NULL;
		reg->next = list->head;
		list->head->prev = reg;
		list->head = reg;
	}

	reg->list = list;
}


void reg_list_remove(physical_reg_list_t * list, cg_physical_reg_t * reg)
{
	assert(reg->list == list);

	if (reg->prev != (cg_physical_reg_t *) 0)
	{
		assert(list->head != reg);
		reg->prev->next = reg->next;
	}
	else 
	{
		assert(list->head == reg);
		list->head = reg->next;
	}
	
	if (reg->next != (cg_physical_reg_t *) 0)
	{
		assert(list->tail != reg);
		reg->next->prev = reg->prev;
	}
	else 
	{
		assert(list->tail == reg);
		list->tail = reg->prev;
	}
	
	reg->prev = reg->next = (cg_physical_reg_t *) 0;
	reg->list = NULL;
}


void reg_list_move_to_front(physical_reg_list_t * list, cg_physical_reg_t * reg)
{
	reg_list_remove(list, reg);
	reg_list_add(list, reg);
}


typedef struct literal_t
{
	struct literal_t *	next;
	U32					value;
	size_t				offset;
}
literal_t;


#define PHYSICAL_REGISTERS  (ARMREG_MAX + 1)

struct cg_codegen_t 
{
	cg_runtime_info_t *		runtime;
	cg_heap_t *				heap;
	cg_label_t *			labels;
	cg_segment_t *			cseg;
	
	cg_physical_reg_t			registers[PHYSICAL_REGISTERS];
	cg_physical_reg_t			flags;

	physical_reg_list_t		free_regs;			/* phys regs free			*/
	physical_reg_list_t		used_regs;			/* phys regs in use			*/
	
	/************************************************************************/
	/* for each block, the following array will be heads to the use chains  */
	/* within the block currently processed.								*/
	/************************************************************************/
	
	cg_block_t *			current_block;
	literal_t *				literals;
	cg_label_t *			literal_base;
	size_t					literal_pool_size;
	cg_inst_list_t **		use_chains;			
};


size_t cg_codegen_emit_literal(cg_codegen_t * gen, U32 value)
{
	literal_t ** literal;

	for (literal = &gen->literals; (*literal) != (literal_t *) 0; literal = &(*literal)->next)
	{
		if ((*literal)->value == value)
			return (*literal)->offset;
	}

	*literal = cg_heap_allocate(gen->heap, sizeof (literal_t));

	(*literal)->value = value;
	(*literal)->offset = gen->literal_pool_size;
	gen->literal_pool_size += sizeof(U32);

	return (*literal)->offset;
}


cg_codegen_t * cg_codegen_create(cg_heap_t * heap, cg_runtime_info_t * runtime) 
{
	size_t regno;
	
	cg_codegen_t * gen = 
		(cg_codegen_t *) cg_heap_allocate(heap, sizeof(cg_codegen_t));
	memset(gen, 0, sizeof *gen);
	
	gen->runtime = runtime;
	gen->heap = heap;
	gen->labels = (cg_label_t *) 0;
	gen->cseg = cg_segment_create("CSEG");
	
	for (regno = 0; regno != PHYSICAL_REGISTERS; ++regno)
	{
		gen->registers[regno].regno = (ARMReg) (ARMREG_R0 + regno);
	}
	
	gen->flags.regno = ARMREG_CPSR;

	return gen;
}


void cg_codegen_destroy(cg_codegen_t * gen)
{
	cg_segment_destroy(gen->cseg);
}


/****************************************************************************/
/* Processing of individual instructions									*/
/****************************************************************************/


static cg_physical_reg_t * allocate_reg(cg_codegen_t * gen, cg_virtual_reg_t * reg,
									 U32 mask);
static void deallocate_reg(cg_codegen_t * gen, cg_physical_reg_t * physical_reg);
static void assign_reg(cg_codegen_t * gen,
					   cg_physical_reg_t * physical_reg, 
					   cg_virtual_reg_t * reg);
static cg_physical_reg_t * load_reg(cg_codegen_t * gen, cg_virtual_reg_t * reg,
									 U32 mask);


static ARMShiftType arm_shift_type(cg_shift_op_t shift_op)
{
	switch (shift_op)
	{
		default:
			assert(0);

		case cg_shift_lsl:
			return ARMSHIFT_LSL;
			
		case cg_shift_lsr:
			return ARMSHIFT_LSR;
			
		case cg_shift_asr:
			return ARMSHIFT_ASR;
			
		case cg_shift_ror:
			return ARMSHIFT_ROR;
	}
}


static void branch_cond(cg_codegen_t * gen, cg_label_t * target, ARMCond cond)
{
	/* insert a conditional branch to the specified target label */
	cg_codegen_reference(gen, target, cg_reference_branch24);
	/* compensate for PC pointing 2 instruction words ahead */
	ARM_B_COND(gen->cseg, cond, -8);
}


static void branch(cg_codegen_t * gen, cg_label_t * target)
{
	branch_cond(gen, target, ARMCOND_AL);
}


static void save_flags(cg_codegen_t * gen, cg_physical_reg_t * physical_reg)
{
	ARM_MRS_CPSR(gen->cseg, physical_reg->regno);
}


static void restore_flags(cg_codegen_t * gen, cg_physical_reg_t * reg)
{
	ARM_MSR_REG(gen->cseg, ARM_PSR_F, reg->regno, ARM_CPSR);
}


static void call_store_additional_args(cg_codegen_t * gen, cg_virtual_reg_list_t * args, cg_inst_t * inst)
{
	/* In case we have more than 4 arguments: store all remaining args onto the stack */

	ARMReg regno;

	for (regno = ARMREG_A1; regno <= ARMREG_A4 && args != (cg_virtual_reg_list_t *) 0; 
		 ++regno, args = args->next)
		;										/* skip register arguments */

	if (args != (cg_virtual_reg_list_t *) 0) 
	{
		size_t argument_base_offset = 0;

		do
		{
			cg_virtual_reg_t * reg = args->reg;
			cg_physical_reg_t * physical_reg;
			cg_inst_list_t ** plist = &gen->use_chains[reg->reg_no];
			
			while (*plist != (cg_inst_list_t *) 0 &&
				(*plist)->inst != inst)
			{
				plist = &(*plist)->next;
			}
			
			if (*plist != 0)
			{
				*plist = (*plist)->next;
			}
			
			physical_reg = load_reg(gen, reg, 0);
		
			/* save the register into the parameter area */
			ARM_STR_IMM(gen->cseg, physical_reg->regno, ARMREG_FP, argument_base_offset);

			if (gen->use_chains[reg->reg_no] == (cg_inst_list_t *) 0)
			{
				deallocate_reg(gen, physical_reg);
			}

			args = args->next;
			argument_base_offset += sizeof(U32);
		}
		while (args != (cg_virtual_reg_list_t *) 0);
	}
}


static void load_register_arg(cg_codegen_t * gen, cg_virtual_reg_t * reg, ARMReg regno, cg_inst_t * inst)
{
	U32 mask = 1u << regno;

	/* force argument into the specified register */
	cg_physical_reg_t * physical_reg;
	cg_inst_list_t ** plist = &gen->use_chains[reg->reg_no];
	
	while (*plist != (cg_inst_list_t *) 0 &&
		(*plist)->inst != inst)
	{
		plist = &(*plist)->next;
	}
	
	if (*plist != 0)
	{
		*plist = (*plist)->next;
	}
	
	physical_reg = load_reg(gen, reg, mask);
	assert(physical_reg->virtual_reg == reg);
}


static void kill_argument_registers(cg_codegen_t * gen)
{
	ARMReg regno;

	for (regno = ARMREG_A1; regno <= ARMREG_A4; ++regno)
	{
		cg_physical_reg_t * physical_reg = gen->registers + regno;
		cg_virtual_reg_t * reg = physical_reg->virtual_reg;

		if (reg != NULL)
		{
			if (reg->physical_reg == physical_reg)
			{
				deallocate_reg(gen, physical_reg);
			}
			else
			{
				/* register is a duplicate of another register, just free it up */
				reg_list_remove(&gen->used_regs, physical_reg);
				reg_list_add(&gen->free_regs, physical_reg);
				
				physical_reg->virtual_reg = 0;
			}
		}
	}
}


static void kill_flags(cg_codegen_t * gen)
{
	if (gen->flags.virtual_reg != NULL &&
		gen->flags.virtual_reg->physical_reg == &gen->flags)
	{
		cg_virtual_reg_t * reg = gen->flags.virtual_reg;

		 int used = CG_BITSET_TEST(gen->current_block->live_out, reg->reg_no) ||
			gen->use_chains[reg->reg_no];

		if (used) 
		{
			cg_physical_reg_t * physical_reg = allocate_reg(gen, reg, 0);
			assign_reg(gen, physical_reg, reg);
			save_flags(gen, physical_reg);
			physical_reg->dirty = physical_reg->defined = 1;
		}
	}
}


static void call_load_register_args(cg_codegen_t * gen, cg_virtual_reg_list_t * begin_args, cg_inst_t * inst)
{
	ARMReg regno;
	cg_virtual_reg_list_t * args;

	for (regno = ARMREG_A1, args = begin_args; regno <= ARMREG_A4 && args != (cg_virtual_reg_list_t *) 0; 
		 ++regno, args = args->next)
	{
		load_register_arg(gen, args->reg, regno, inst);
	}

	kill_flags(gen);
	kill_argument_registers(gen);
}

	

static void call_runtime(cg_codegen_t * gen, void * target)
{
	/* create the necessary code sequence to call into a procedure that is  */
	/* part of the runtime library											*/

	ARM_MOV_REG_REG(gen->cseg, ARMREG_LR, ARMREG_PC);
	ARM_LDR_IMM(gen->cseg, ARMREG_PC, ARMREG_PC, 
		cg_codegen_emit_literal(gen, (U32) target));
}


static void call(cg_codegen_t * gen, cg_label_t * target,
				 cg_virtual_reg_list_t * args, cg_inst_t * inst)
{
	/* create the necessary code sequence to call into a procedure */

	call_store_additional_args(gen, args, inst);
	call_load_register_args(gen, args, inst);
	cg_codegen_reference(gen, target, cg_reference_branch24);
	ARM_BL(gen->cseg, -8);
}


static void emit_unary_negate(cg_codegen_t * gen, cg_inst_unary_t * inst,
							  int update_flags)
{
	switch (inst->base.kind)
	{
		case cg_inst_unary:
			if (update_flags)
				ARM_MVNS_REG_REG(gen->cseg,
								 inst->dest_value->physical_reg->regno,
								 inst->operand.source->physical_reg->regno);
			else
				ARM_MVN_REG_REG(gen->cseg,
								inst->dest_value->physical_reg->regno,
								inst->operand.source->physical_reg->regno);

			break;
			
		case cg_inst_arm_unary_immed:
			assert(inst->operand.immed >= 0 && inst->operand.immed <= 0xff);

			if (update_flags)
				ARM_MVNS_REG_IMM(gen->cseg,
								 inst->dest_value->physical_reg->regno,
								 inst->operand.immed, 0);
			else
				ARM_MVN_REG_IMM(gen->cseg,
								inst->dest_value->physical_reg->regno,
								inst->operand.immed, 0);

			break;
			
		case cg_inst_arm_unary_shift_reg:
			if (update_flags)
				ARM_MVNS_REG_REGSHIFT(gen->cseg,
									 inst->dest_value->physical_reg->regno,
									 inst->operand.shift_reg.source->physical_reg->regno, 
									 arm_shift_type(inst->operand.shift_reg.op),
									 inst->operand.shift_reg.shift->physical_reg->regno);
			else
				ARM_MVN_REG_REGSHIFT(gen->cseg,
									inst->dest_value->physical_reg->regno,
									inst->operand.shift_reg.source->physical_reg->regno, 
									arm_shift_type(inst->operand.shift_reg.op),
									inst->operand.shift_reg.shift->physical_reg->regno);
			break;
			
		case cg_inst_arm_unary_shift_immed:
			if (update_flags)
				ARM_MVNS_REG_IMMSHIFT(gen->cseg,
									 inst->dest_value->physical_reg->regno,
									 inst->operand.shift_immed.source->physical_reg->regno, 
									 arm_shift_type(inst->operand.shift_immed.op),
									 inst->operand.shift_immed.shift);
			else
				ARM_MVN_REG_IMMSHIFT(gen->cseg,
									inst->dest_value->physical_reg->regno,
									inst->operand.shift_immed.source->physical_reg->regno, 
									arm_shift_type(inst->operand.shift_immed.op),
									inst->operand.shift_immed.shift);

			break;
			
		default:
			assert(0);
	}
}


static void emit_unary_complement(cg_codegen_t * gen, cg_inst_unary_t * inst,
								  int update_flags)
{
	emit_unary_negate(gen, inst, 0);

	if (update_flags)
		ARM_SUBS_REG_IMM(gen->cseg,
						 inst->dest_value->physical_reg->regno,
						 inst->dest_value->physical_reg->regno,
						 1, 0);
	else
		ARM_SUB_REG_IMM(gen->cseg,
						inst->dest_value->physical_reg->regno,
						inst->dest_value->physical_reg->regno,
						1, 0);

}


static void emit_unary_trunc(cg_codegen_t * gen, cg_inst_unary_t * inst,
							 int update_flags)
{
	assert(inst->base.kind == cg_inst_unary);
	
	if (update_flags)
		ARM_MOVS_REG_IMMSHIFT(gen->cseg,
							  inst->dest_value->physical_reg->regno,
							  inst->operand.source->physical_reg->regno,
							  ARMSHIFT_ASR, 16);						 
	else
		ARM_MOV_REG_IMMSHIFT(gen->cseg,
							 inst->dest_value->physical_reg->regno,
							 inst->operand.source->physical_reg->regno,
							 ARMSHIFT_ASR, 16);						 

}


static void emit_unary_round(cg_codegen_t * gen, cg_inst_unary_t * inst,
							 int update_flags)
{
	assert(inst->base.kind == cg_inst_unary);
	
	ARM_ADD_REG_IMM(gen->cseg,
					inst->dest_value->physical_reg->regno,
					inst->operand.source->physical_reg->regno,
					1, 0);
	
	if (update_flags)
		ARM_MOVS_REG_IMMSHIFT(gen->cseg,
							  inst->dest_value->physical_reg->regno,
							  inst->dest_value->physical_reg->regno,
							  ARMSHIFT_ASR, 16);						 
	else
		ARM_MOV_REG_IMMSHIFT(gen->cseg,
							 inst->dest_value->physical_reg->regno,
							 inst->operand.source->physical_reg->regno,
							 ARMSHIFT_ASR, 16);						 

}


static void emit_unary_fcnv(cg_codegen_t * gen, cg_inst_unary_t * inst,
							int update_flags)
{
	assert(inst->base.kind == cg_inst_unary);
	
	if (update_flags)
		ARM_MOVS_REG_IMMSHIFT(gen->cseg,
							  inst->dest_value->physical_reg->regno,
							  inst->operand.source->physical_reg->regno,
							  ARMSHIFT_LSL, 16);						 
	else
		ARM_MOV_REG_IMMSHIFT(gen->cseg,
							 inst->dest_value->physical_reg->regno,
							 inst->operand.source->physical_reg->regno,
							 ARMSHIFT_LSL, 16);						 
}


static void emit_unary(cg_codegen_t * gen, cg_inst_unary_t * inst,
					   int update_flags)
{
	// distinguish operations that map directly to ARM instructions
	// operations that require sequence of instructions
	// operations that map into a function call

	switch (inst->base.opcode)
	{
		/* regular unary operation */
		case cg_op_neg:
		case cg_op_fneg:
			emit_unary_negate(gen, inst, update_flags);
			break;
			
		case cg_op_not:		
			emit_unary_complement(gen, inst, update_flags);
			break;
			
		/* sequences */
		case cg_op_trunc:	
			emit_unary_trunc(gen, inst, update_flags);
			break;
			
		case cg_op_round:	
			emit_unary_round(gen, inst, update_flags);
			break;
			
		case cg_op_fcnv:															
			emit_unary_fcnv(gen, inst, update_flags);
			break;

		default:
			assert(0);
	}
}


static void emit_binary_shifter(cg_codegen_t * gen, cg_inst_binary_t * inst,
								int update_flags)
{
	ARMShiftType shift_type;
	
	switch (inst->base.opcode)
	{
		default:
			assert(0);
			
		case cg_op_lsl:
			shift_type = ARMSHIFT_LSL;
			
		case cg_op_lsr:
			shift_type = ARMSHIFT_LSR;
			
		case cg_op_asr:
			shift_type = ARMSHIFT_ASR;
	}
	
	switch (inst->base.kind)
	{
		case cg_inst_binary:
			if (update_flags)
				ARM_MOVS_REG_REGSHIFT(gen->cseg,
									  inst->dest_value->physical_reg->regno,
									  inst->source->physical_reg->regno,
									  shift_type,
									  inst->operand.source->physical_reg->regno);
			else
				ARM_MOV_REG_REGSHIFT(gen->cseg,
									 inst->dest_value->physical_reg->regno,
									 inst->source->physical_reg->regno,
									 shift_type,
									 inst->operand.source->physical_reg->regno);

			break;
			
		case cg_inst_arm_binary_immed:
			if (update_flags)
				ARM_MOVS_REG_IMMSHIFT(gen->cseg,
									  inst->dest_value->physical_reg->regno,
									  inst->source->physical_reg->regno,
									  shift_type,
									  inst->operand.immed);
			else
				ARM_MOV_REG_IMMSHIFT(gen->cseg,
									 inst->dest_value->physical_reg->regno,
									 inst->source->physical_reg->regno,
									 shift_type,
									 inst->operand.immed);

			break;
			
		default:
			assert(0);
	}
}


static void emit_binary_regular(cg_codegen_t * gen, cg_inst_binary_t * inst,
								ARMOpcode opcode, int update_flags)
{
	switch (inst->base.kind)
	{
		case cg_inst_binary:
			if (update_flags)
				ARM_DPIOP_S_REG_REG_COND(gen->cseg, 
										 opcode, 
										 inst->dest_value->physical_reg->regno, 
										 inst->source->physical_reg->regno, 
										 inst->operand.source->physical_reg->regno, 
										 ARMCOND_AL);
			else
				ARM_DPIOP_REG_REG_COND(gen->cseg, 
									   opcode, 
									   inst->dest_value->physical_reg->regno, 
									   inst->source->physical_reg->regno, 
									   inst->operand.source->physical_reg->regno, 
									   ARMCOND_AL);

			break;
			
		case cg_inst_arm_binary_immed:
			assert(inst->operand.immed >= 0 && inst->operand.immed <= 0xff);
			if (update_flags)
				ARM_DPIOP_S_REG_IMM8ROT_COND(gen->cseg, 
											 opcode, 
											 inst->dest_value->physical_reg->regno, 
											 inst->source->physical_reg->regno, 
											 inst->operand.immed, 0,
											 ARMCOND_AL);
			else
				ARM_DPIOP_REG_IMM8ROT_COND(gen->cseg, 
										   opcode, 
										   inst->dest_value->physical_reg->regno, 
										   inst->source->physical_reg->regno, 
										   inst->operand.immed, 0,
										   ARMCOND_AL);

			break;
			
		case cg_inst_arm_binary_shift_reg:
			if (update_flags)
				ARM_DPIOP_S_REG_REGSHIFT_COND(gen->cseg, 
											  opcode, 
											  inst->dest_value->physical_reg->regno, 
											  inst->source->physical_reg->regno, 
											  inst->operand.shift_reg.source->physical_reg->regno, 
											  arm_shift_type(inst->operand.shift_reg.op),
											  inst->operand.shift_reg.shift->physical_reg->regno, 
											  ARMCOND_AL);
			else
				ARM_DPIOP_REG_REGSHIFT_COND(gen->cseg, 
											opcode, 
											inst->dest_value->physical_reg->regno, 
											inst->source->physical_reg->regno, 
											inst->operand.shift_reg.source->physical_reg->regno, 
											arm_shift_type(inst->operand.shift_reg.op),
											inst->operand.shift_reg.shift->physical_reg->regno, 
											ARMCOND_AL);

			break;
			
		case cg_inst_arm_binary_shift_immed:
			if (update_flags)
				ARM_DPIOP_S_REG_IMMSHIFT_COND(gen->cseg, 
											  opcode, 
											  inst->dest_value->physical_reg->regno, 
											  inst->source->physical_reg->regno, 
											  inst->operand.shift_immed.source->physical_reg->regno, 
											  arm_shift_type(inst->operand.shift_immed.op),
											  inst->operand.shift_immed.shift, 
											  ARMCOND_AL);
			else
				ARM_DPIOP_REG_IMMSHIFT_COND(gen->cseg, 
											opcode, 
											inst->dest_value->physical_reg->regno, 
											inst->source->physical_reg->regno, 
											inst->operand.shift_immed.source->physical_reg->regno, 
											arm_shift_type(inst->operand.shift_immed.op),
											inst->operand.shift_immed.shift, 
											ARMCOND_AL);

			break;
			
		default:
			assert(0);
	}
}


static void emit_binary_multiply_int(cg_codegen_t * gen, cg_inst_binary_t * inst,
									 int update_flags)
{
	assert(inst->base.kind == cg_inst_binary);
	
	if (update_flags)
		ARM_MULS(gen->cseg, 
				 inst->dest_value->physical_reg->regno, 
				 inst->source->physical_reg->regno, 
				 inst->operand.source->physical_reg->regno);
	else
		ARM_MUL(gen->cseg, 
				inst->dest_value->physical_reg->regno, 
				inst->source->physical_reg->regno, 
				inst->operand.source->physical_reg->regno);

}


static void emit_binary_multiply_fixed(cg_codegen_t * gen, cg_inst_binary_t * inst,
									   int update_flags)
{
	// alloacte a temporary register
	cg_virtual_reg_t temp_reg;
	cg_physical_reg_t * temp_physical_reg;
	U32 mask;

	assert(inst->base.kind == cg_inst_binary);
	
	memset(&temp_reg, 0, sizeof temp_reg);
	
	/* may need to create a mask based on the registers allocated for		*/
	/* the other operands													*/
	mask = ~(inst->dest_value->physical_reg->regno  |
			 inst->source->physical_reg->regno		|
			 inst->operand.source->physical_reg->regno);
	
	temp_physical_reg = allocate_reg(gen, &temp_reg, mask);
	assign_reg(gen, temp_physical_reg, &temp_reg);
	
	ARM_SMULL(gen->cseg, 
			  temp_physical_reg->regno, 
			  inst->dest_value->physical_reg->regno, 
			  inst->source->physical_reg->regno, 
			  inst->operand.source->physical_reg->regno);
	
	ARM_MOV_REG_IMMSHIFT(gen->cseg,
						 inst->dest_value->physical_reg->regno,
						 inst->dest_value->physical_reg->regno,
						 ARMSHIFT_LSR,
						 16);
	
	if (update_flags)
		ARM_ORRS_REG_IMMSHIFT(gen->cseg,
							  inst->dest_value->physical_reg->regno,
							  inst->dest_value->physical_reg->regno,
							  temp_physical_reg->regno, 
							  ARMSHIFT_LSL,
							  16);
	else
		ARM_ORR_REG_IMMSHIFT(gen->cseg,
							inst->dest_value->physical_reg->regno,
							inst->dest_value->physical_reg->regno,
							temp_physical_reg->regno, 
							ARMSHIFT_LSL,
							16);

	
	// release the temporary register
	deallocate_reg(gen, temp_physical_reg);
}


static void emit_binary(cg_codegen_t * gen, cg_inst_binary_t * inst, int update_flags)
{
	// distinguish operations that map directly to ARM instructions
	// shifter-type instructions (MOV plus shifter operand)
	// operations that require sequence of instructions
	// operations that map into a function call

	switch (inst->base.opcode)
	{
		/* shifter type */
		case cg_op_asr:
		case cg_op_lsl:
		case cg_op_lsr:
			emit_binary_shifter(gen, inst, update_flags);
			break;
	
		/* regular binary operation */
		case cg_op_add:
		case cg_op_fadd:
			emit_binary_regular(gen, inst, ARMOP_ADD, update_flags);
			break;
			
		case cg_op_sub:
		case cg_op_fsub:
			emit_binary_regular(gen, inst, ARMOP_SUB, update_flags);
			break;
			
		case cg_op_and:
			emit_binary_regular(gen, inst, ARMOP_AND, update_flags);
			break;
			
		case cg_op_or:
			emit_binary_regular(gen, inst, ARMOP_ORR, update_flags);
			break;
			
		case cg_op_xor:
			emit_binary_regular(gen, inst, ARMOP_EOR, update_flags);
			break;
	
		/* multiplication */
		case cg_op_mul:
			emit_binary_multiply_int(gen, inst, update_flags);
			break;
			
		case cg_op_fmul:
			emit_binary_multiply_fixed(gen, inst, update_flags);
			break;
			
		default:
			assert(0);
	}
}


static void emit_compare(cg_codegen_t * gen, cg_inst_compare_t * inst)
{
	ARMShiftType shift_type;
	
	assert(inst->base.opcode == cg_op_cmp ||
		   inst->base.opcode == cg_op_fcmp);
	
	switch (inst->base.kind)
	{
		case cg_inst_compare:
			ARM_CMP_REG_REG(gen->cseg, 
							inst->source->physical_reg->regno,
							inst->operand.source->physical_reg->regno);
			break;
			
		case cg_inst_arm_compare_immed:
			assert(inst->operand.immed <= 0xff &&
				   inst->operand.immed >= 0);
			
			ARM_CMP_REG_IMM8(gen->cseg,
							inst->source->physical_reg->regno,
							inst->operand.immed);
			break;
			
		case cg_inst_arm_compare_shift_reg:
			shift_type = arm_shift_type(inst->operand.shift_reg.op);
			
			ARM_CMP_REG_REGSHIFT(gen->cseg, 
								 inst->source->physical_reg->regno, 
								 inst->operand.shift_reg.source->physical_reg->regno, 
								 shift_type, 
								 inst->operand.shift_reg.shift->physical_reg->regno);
			break;
			
		case cg_inst_arm_compare_shift_immed:
			shift_type = arm_shift_type(inst->operand.shift_immed.op);
			
			ARM_CMP_REG_IMMSHIFT(gen->cseg, 
								 inst->source->physical_reg->regno, 
								 inst->operand.shift_immed.source->physical_reg->regno, 
								 shift_type, 
								 inst->operand.shift_immed.shift);
			break;
			
		default:
			assert(0);
	}
}


static void emit_load(cg_codegen_t * gen, cg_inst_load_t * inst) 
{
	switch (inst->base.opcode)
	{
		case cg_op_ldb:
			switch (inst->base.kind)
			{
				case cg_inst_load:
					ARM_LDRB_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								 inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_load_immed_offset:
					ARM_LDRB_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								 inst->mem.immed_offset.base->physical_reg->regno, 
								 inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_load_reg_offset:
					ARM_LDRB_REG_REG(gen->cseg, inst->dest->physical_reg->regno, 
									 inst->mem.reg_offset.base->physical_reg->regno, 
									 inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		case cg_op_ldh:
			switch (inst->base.kind)
			{
				case cg_inst_load:
					ARM_LDRH_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								 inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_load_immed_offset:
					ARM_LDRH_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								 inst->mem.immed_offset.base->physical_reg->regno, 
								 inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_load_reg_offset:
					ARM_LDRH_REG_REG(gen->cseg, inst->dest->physical_reg->regno, 
									 inst->mem.reg_offset.base->physical_reg->regno, 
									 inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		case cg_op_ldw:
			switch (inst->base.kind)
			{
				case cg_inst_load:
					ARM_LDR_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_load_immed_offset:
					ARM_LDR_IMM(gen->cseg, inst->dest->physical_reg->regno, 
								inst->mem.immed_offset.base->physical_reg->regno, 
								inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_load_reg_offset:
					ARM_LDR_REG_REG(gen->cseg, inst->dest->physical_reg->regno, 
									inst->mem.reg_offset.base->physical_reg->regno, 
									inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		default:
			assert(0);
	}
}


static void emit_store(cg_codegen_t * gen, cg_inst_store_t * inst)
{
	switch (inst->base.opcode)
	{
		case cg_op_stb:
			switch (inst->base.kind)
			{
				case cg_inst_store:
					ARM_STRB_IMM(gen->cseg, inst->source->physical_reg->regno, 
								 inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_store_immed_offset:
					ARM_STRB_IMM(gen->cseg, inst->source->physical_reg->regno, 
								 inst->mem.immed_offset.base->physical_reg->regno, 
								 inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_store_reg_offset:
					ARM_STRB_REG_REG(gen->cseg, inst->source->physical_reg->regno, 
									 inst->mem.reg_offset.base->physical_reg->regno, 
									 inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		case cg_op_sth:
			switch (inst->base.kind)
			{
				case cg_inst_store:
					ARM_STRH_IMM(gen->cseg, inst->source->physical_reg->regno, 
								 inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_store_immed_offset:
					ARM_STRH_IMM(gen->cseg, inst->source->physical_reg->regno, 
								 inst->mem.immed_offset.base->physical_reg->regno, 
								 inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_store_reg_offset:
					ARM_STRH_REG_REG(gen->cseg, inst->source->physical_reg->regno, 
									 inst->mem.reg_offset.base->physical_reg->regno, 
									 inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		case cg_op_stw:
			switch (inst->base.kind)
			{
				case cg_inst_store:
					ARM_STR_IMM(gen->cseg, inst->source->physical_reg->regno, 
								inst->mem.base->physical_reg->regno, 0);
					break;
					
				case cg_inst_arm_store_immed_offset:
					ARM_STR_IMM(gen->cseg, inst->source->physical_reg->regno, 
								inst->mem.immed_offset.base->physical_reg->regno, 
								inst->mem.immed_offset.offset);
					break;
					
				case cg_inst_arm_store_reg_offset:
					ARM_STR_REG_REG(gen->cseg, inst->source->physical_reg->regno, 
									inst->mem.reg_offset.base->physical_reg->regno, 
									inst->mem.reg_offset.offset->physical_reg->regno);
					break;
					
				default:
					assert(0);
			}
			
			break;
			
		default:
			assert(0);
	}
}


static void emit_load_immediate(cg_codegen_t * gen, 
								cg_inst_load_immed_t * inst)
{
	assert(inst->base.opcode == cg_op_ldi);
	
	arm_mov_reg_imm32(gen->cseg, inst->dest->physical_reg->regno,
					  inst->value);
}


static void flush_dirty_regs(cg_codegen_t * gen);
static void flush_dirty_args(cg_codegen_t * gen);

static void emit_branch(cg_codegen_t * gen, cg_inst_branch_t * inst)
{
	ARMCond cond;
	flush_dirty_regs(gen);
	
	switch (inst->base.kind) 
	{
		case cg_inst_branch_label:
			assert(inst->base.opcode == cg_op_bra);
			branch(gen, inst->target->block->label);
			return;
			
		case cg_inst_branch_cond:
			/* ensure condition flag is bound to virtual reg.				*/
			
			switch (inst->base.opcode) 
			{
				case cg_op_beq:		cond = ARMCOND_EQ; break;
				case cg_op_bge:		cond = ARMCOND_GE; break;
				case cg_op_ble:		cond = ARMCOND_LE; break;
				case cg_op_bgt:		cond = ARMCOND_GT; break;
				case cg_op_blt:		cond = ARMCOND_LT; break;
				case cg_op_bne:		cond = ARMCOND_NE; break;
					
				default:
					assert(0);
			}
			
			branch_cond(gen, inst->target->block->label, cond);
			break;
			
		default:
			assert(0);
	}
}


static void emit_call(cg_codegen_t * gen, cg_inst_call_t * inst)
{
	call(gen, inst->proc->prologue, inst->args, (cg_inst_t *) inst);
	
	// deal with results
	if (inst->dest)
	{
		cg_physical_reg_t * physical_reg = allocate_reg(gen, inst->dest, 1 << ARMREG_A1);
		assign_reg(gen, physical_reg, inst->dest);
		physical_reg->dirty = physical_reg->defined = 1;
	}
}


static void emit_ret(cg_codegen_t * gen, cg_inst_ret_t * inst)
{
	if (inst->result)
	{
		/********************************************************************/
		/* Make sure that result is in correct register						*/
		/********************************************************************/
		
		if (inst->result->physical_reg->regno != ARMREG_A1)
		{
			ARM_MOV_REG_REG(gen->cseg, ARMREG_A1, 
							inst->result->physical_reg->regno);
		}
	}
	
	branch(gen, inst->base.block->proc->epilogue);
}


static void dispatch_inst(cg_codegen_t * gen, cg_inst_t * inst, int update_flags)
{
	switch (inst->base.kind) 
	{		
		case cg_inst_unary:		
		case cg_inst_arm_unary_immed:	
		case cg_inst_arm_unary_shift_reg:	
		case cg_inst_arm_unary_shift_immed:	
			emit_unary(gen, &inst->unary, update_flags);
			break;
			
		case cg_inst_binary:		
		case cg_inst_arm_binary_immed:	
		case cg_inst_arm_binary_shift_reg:	
		case cg_inst_arm_binary_shift_immed:	
			emit_binary(gen, &inst->binary, update_flags);
			break;
			
		case cg_inst_compare:	
		case cg_inst_arm_compare_immed:	
		case cg_inst_arm_compare_shift_reg:	
		case cg_inst_arm_compare_shift_immed:	
			emit_compare(gen, &inst->compare);
			break;
			
		case cg_inst_load:	
		case cg_inst_arm_load_immed_offset:	
		case cg_inst_arm_load_reg_offset:	
			emit_load(gen, &inst->load);
			break;
			
		case cg_inst_store:	
		case cg_inst_arm_store_immed_offset:	
		case cg_inst_arm_store_reg_offset:	
			emit_store(gen, &inst->store);
			break;
			
		case cg_inst_load_immed:
			emit_load_immediate(gen, &inst->immed);
			break;
			
		case cg_inst_branch_label:	
		case cg_inst_branch_cond:	
			emit_branch(gen, &inst->branch);
			break;
			
		case cg_inst_call:
			emit_call(gen, &inst->call);
			break;
			
		case cg_inst_ret:			
			emit_ret(gen, &inst->ret);
			break;
			
			// ARM specific formats
		case cg_inst_phi:	
		default:
			assert(0);
	}
	
}


static cg_physical_reg_t * spill_candidate(cg_codegen_t * gen, 
										cg_virtual_reg_t * reg, U32 mask)
	/************************************************************************/
	/* Determine a spill candidate because we need a register for the given */
	/* virtual register														*/
	/************************************************************************/
{
	// pick the register whose next use is most far away
	// if possible, select a register that is not dirty
	
	cg_physical_reg_t * physical_reg = gen->used_regs.tail;
	
	while (physical_reg != 0 && 
		   !((1u << physical_reg->regno) & mask)) 
	{
		physical_reg = physical_reg->prev;
	}
	
	return physical_reg;
}


static void save_reg(cg_codegen_t * gen, cg_physical_reg_t * physical_reg,
					 cg_virtual_reg_t * reg)
	/************************************************************************/
	/* Save a given physical register										*/
	/************************************************************************/
{
	assert(physical_reg->dirty);
	
	// generate code to save the register; reg -> FP + offset
	ARM_STR_IMM(gen->cseg, physical_reg->regno, ARMREG_FP, reg->representative->fp_offset);
	
	physical_reg->dirty = 0;
	
}


static void restore_reg(cg_codegen_t * gen, cg_physical_reg_t * physical_reg,
						cg_virtual_reg_t * reg)
	/************************************************************************/
	/* Restore a given physical register									*/
	/************************************************************************/
{
	assert(!physical_reg->defined);
	
	// generate code to save the register; FP + offset -> reg
	ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_FP, reg->representative->fp_offset);
	
	physical_reg->defined = 1;
	
}


static void deallocate_reg(cg_codegen_t * gen, cg_physical_reg_t * physical_reg)
	/************************************************************************/
	/* free up a register because it is no longer used						*/
	/************************************************************************/
{
	cg_virtual_reg_t * reg = physical_reg->virtual_reg;

	int used;
	
	if (reg != 0 && reg->physical_reg == physical_reg)
	{
		//reg->physical_reg = (cg_physical_reg_t *) 0;
		
		used = CG_BITSET_TEST(gen->current_block->live_out, reg->reg_no) ||
			gen->use_chains[reg->reg_no];
		
		if (physical_reg->dirty && used)
		{
			save_reg(gen, physical_reg, reg);
		}
	}
		
	physical_reg->virtual_reg = (cg_virtual_reg_t *) 0;

	if (physical_reg->list == &gen->used_regs) 
	{
		reg_list_remove(&gen->used_regs, physical_reg);
		reg_list_add(&gen->free_regs, physical_reg);
	}

	assert(physical_reg->list == &gen->free_regs);

	physical_reg->dirty = physical_reg->defined = 0;
}


static cg_physical_reg_t * allocate_reg(cg_codegen_t * gen, cg_virtual_reg_t * reg,
									 U32 mask)
	/************************************************************************/
	/* Allocate a specific physical register for a virtual register			*/
	/************************************************************************/
{
	cg_physical_reg_t * physical_reg;
	
	if (mask == 0)
	{
		mask = ~0u;
	}
	
	// check if the virtual register is already present in a matching
	// physical register
	// if so, return that register
	
	physical_reg = reg->physical_reg;
	
	if (physical_reg != 0 && physical_reg->virtual_reg == reg &&
		((1u << physical_reg->regno) & mask))
	{
		reg_list_move_to_front(&gen->used_regs, physical_reg);
		return physical_reg;
	}
	
	if (gen->free_regs.tail != (cg_physical_reg_t *) 0)
	{
		physical_reg = gen->free_regs.tail;
		reg_list_remove(&gen->free_regs, physical_reg);
		reg_list_add(&gen->used_regs, physical_reg);
		return physical_reg;
	}

	// determine a spill candidate
	physical_reg = spill_candidate(gen, reg, mask);
	assert(physical_reg->list == &gen->used_regs);
	deallocate_reg(gen, physical_reg);
	assert(physical_reg->list == &gen->free_regs);
	reg_list_remove(&gen->free_regs, physical_reg);
	reg_list_add(&gen->used_regs, physical_reg);
		
	return physical_reg;
}


static void assign_reg(cg_codegen_t * gen,
					   cg_physical_reg_t * physical_reg, 
					   cg_virtual_reg_t * reg)
	/************************************************************************/
	/* Bind a specific physical register to a virtual register				*/
	/************************************************************************/
{
	// associate the physical register with the virtual register
	// but mark the register as no holding the value yet

	if (physical_reg->virtual_reg != reg)
	{
		physical_reg->virtual_reg = reg;

		physical_reg->defined = physical_reg->dirty = 0;
	}

	if (reg->physical_reg == (cg_physical_reg_t *) 0 ||
		reg->physical_reg->virtual_reg != reg)
	{
		reg->physical_reg = physical_reg;
	}
		
}


static void ensure_loaded(cg_codegen_t * gen,
						  cg_physical_reg_t * physical_reg, 
						  cg_virtual_reg_t * reg)
	/************************************************************************/
	/* Allocate a specific physical register for a virtual register			*/
	/************************************************************************/
{
	assert(physical_reg->virtual_reg == reg);
	//assert(reg->physical_reg == physical_reg);
	
	if (!physical_reg->defined) 
	{
		if (reg->physical_reg != physical_reg && reg->physical_reg->defined)
		{
			reg_list_move_to_front(&gen->used_regs, reg->physical_reg);
			ARM_MOV_REG_REG(gen->cseg, physical_reg->regno, reg->physical_reg->regno);
			physical_reg->defined = 1;
		} 
		else 
		{
			assert(!physical_reg->dirty);
			restore_reg(gen, physical_reg, reg);
			assert(physical_reg->defined);
		}
	}
}


static cg_physical_reg_t * load_reg(cg_codegen_t * gen, cg_virtual_reg_t * reg,
									 U32 mask)
{
	cg_physical_reg_t * physical_reg;

	physical_reg = allocate_reg(gen, reg, mask);
	assign_reg(gen, physical_reg, reg);
	ensure_loaded(gen, physical_reg, reg);

	return physical_reg;
}


void cg_codegen_emit_simple_inst(cg_codegen_t * gen, cg_inst_t * inst)
{
	/************************************************************************/
	/* Allocate registers for virtual registers in use set					*/
	/************************************************************************/

	cg_virtual_reg_t * buffer[64];
	cg_virtual_reg_t **iter, ** end = cg_inst_use(inst, buffer, buffer + 64);
	int update_flags = 0;

	for (iter = buffer; iter != end; ++iter)
	{
		cg_virtual_reg_t * reg = *iter;
		cg_physical_reg_t * physical_reg;
		cg_inst_list_t ** plist = &gen->use_chains[reg->reg_no];
		
		while (*plist != (cg_inst_list_t *) 0 &&
			   (*plist)->inst != inst)
		{
			plist = &(*plist)->next;
		}
		
		if (*plist != 0)
		{
			*plist = (*plist)->next;
		}
		
		if (reg->type == cg_reg_type_flags)
		{
			/****************************************************************/
			/* do not allocate a register for flags, however we will have	*/
			/* to add spill code later										*/
			/****************************************************************/

			if (gen->flags.virtual_reg == reg)
				continue;			/* OK, current value is what we need	*/

			if (reg->physical_reg != NULL &&
				reg->physical_reg->virtual_reg == reg)
			{
				// value is in a regular register
				restore_flags(gen, reg->physical_reg);
				gen->flags.virtual_reg = reg;
				continue;
			}

			physical_reg = load_reg(gen, reg, 0);
			restore_flags(gen, physical_reg);
			deallocate_reg(gen, physical_reg);
			gen->flags.virtual_reg = reg;

			continue;
		}

		physical_reg = load_reg(gen, reg, 0);
	}
	
	/************************************************************************/
	/* Free up any register that won't be needed after the completion of	*/
	/* the current instruction												*/
	/************************************************************************/
	
	for (iter = buffer; iter != end; ++iter)
	{
		cg_virtual_reg_t * reg = *iter;
		
		if (reg->type != cg_reg_type_flags &&
			gen->use_chains[reg->reg_no] == (cg_inst_list_t *) 0)
		{
			deallocate_reg(gen, reg->physical_reg);
		}
	}

	/************************************************************************/
	/* Allocate registers for all registers in def set						*/
	/************************************************************************/

	end = cg_inst_def(inst, buffer, buffer + 64);
		
	for (iter = buffer; iter != end; ++iter)
	{
		//  for each def:
		//		allocate a new register
		cg_virtual_reg_t * reg = *iter;
		cg_physical_reg_t * physical_reg;
		
		if (reg->type == cg_reg_type_flags)
		{
			/****************************************************************/
			/* do not allocate a register for flags, but save flags if they	*/
			/* are needed later												*/
			/****************************************************************/
			update_flags = 1;

			if (gen->flags.virtual_reg != NULL)
			{
				cg_virtual_reg_t * old_reg = gen->flags.virtual_reg;

				int used = CG_BITSET_TEST(gen->current_block->live_out, old_reg->reg_no) ||
					gen->use_chains[old_reg->reg_no];

				if (used) 
				{
					physical_reg = allocate_reg(gen, gen->flags.virtual_reg, 0);
					assign_reg(gen, physical_reg, gen->flags.virtual_reg);
					save_flags(gen, physical_reg);
					physical_reg->dirty = physical_reg->defined = 1;
				}
			}

			gen->flags.virtual_reg = reg;
			reg->physical_reg = &gen->flags;

			continue;
		}

		physical_reg = allocate_reg(gen, reg, 0);
		assign_reg(gen, physical_reg, reg);
	}

	dispatch_inst(gen, inst, update_flags);

	/************************************************************************/
	/* Mark all registers from the def set as dirty and defined				*/
	/************************************************************************/
	
	for (iter = buffer; iter != end; ++iter)
	{
		cg_virtual_reg_t * reg = *iter;
		cg_physical_reg_t * physical_reg;
		
		physical_reg = reg->physical_reg;
		physical_reg->dirty = physical_reg->defined = 1;
	}
}


void cg_codegen_emit_inst(cg_codegen_t * gen, cg_inst_t * inst)
{
	switch (inst->base.opcode)
	{
		case cg_op_finv:
			{
				cg_physical_reg_t * physical_reg;

				load_register_arg(gen, inst->unary.operand.source, ARMREG_A1, inst);
				kill_flags(gen);
				kill_argument_registers(gen);
				ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
				ARM_MOV_REG_REG(gen->cseg, ARMREG_A2, ARMREG_SP);
				call_runtime(gen, gen->runtime->inv_LP_16_32s);

				/* load result from stack position */
				physical_reg = allocate_reg(gen, inst->unary.dest_value, 0);
				assign_reg(gen, physical_reg, inst->unary.dest_value);
				physical_reg->dirty = physical_reg->defined = 1;
				ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_SP, 0);
				ARM_ADD_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
			}
			break;

		case cg_op_fdiv:
			{
				cg_physical_reg_t * physical_reg;

				load_register_arg(gen, inst->binary.source, ARMREG_A1, inst);
				load_register_arg(gen, inst->binary.operand.source, ARMREG_A2, inst);
				kill_flags(gen);
				kill_argument_registers(gen);
				ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
				ARM_MOV_REG_REG(gen->cseg, ARMREG_A3, ARMREG_SP);
				call_runtime(gen, gen->runtime->div_LP_16_32s);

				/* load result from stack position */
				physical_reg = allocate_reg(gen, inst->unary.dest_value, 0);
				assign_reg(gen, physical_reg, inst->unary.dest_value);
				physical_reg->dirty = physical_reg->defined = 1;
				ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_SP, 0);
				ARM_ADD_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
			}
			break;


		case cg_op_fsqrt:
			{
				cg_physical_reg_t * physical_reg;

				load_register_arg(gen, inst->unary.operand.source, ARMREG_A1, inst);
				kill_flags(gen);
				kill_argument_registers(gen);
				ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
				ARM_MOV_REG_REG(gen->cseg, ARMREG_A2, ARMREG_SP);
				call_runtime(gen, gen->runtime->sqrt_LP_16_32s);

				/* load result from stack position */
				physical_reg = allocate_reg(gen, inst->unary.dest_value, 0);
				assign_reg(gen, physical_reg, inst->unary.dest_value);
				physical_reg->dirty = physical_reg->defined = 1;
				ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_SP, 0);
				ARM_ADD_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, 4);
			}
			break;


		case cg_op_div:
			{
				cg_physical_reg_t * physical_reg;

				load_register_arg(gen, inst->binary.source, ARMREG_A2, inst);
				load_register_arg(gen, inst->binary.operand.source, ARMREG_A3, inst);
				kill_flags(gen);
				kill_argument_registers(gen);
				ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, sizeof(div_t));
				ARM_MOV_REG_REG(gen->cseg, ARMREG_A1, ARMREG_SP);
				call_runtime(gen, gen->runtime->div);

				/* load result from stack position */
				physical_reg = allocate_reg(gen, inst->unary.dest_value, 0);
				assign_reg(gen, physical_reg, inst->unary.dest_value);
				physical_reg->dirty = physical_reg->defined = 1;
				ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_SP, offsetof(div_t, quot));
				ARM_ADD_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, sizeof(div_t));
			}
			break;

		case cg_op_mod:
			{
				cg_physical_reg_t * physical_reg;

				load_register_arg(gen, inst->binary.source, ARMREG_A2, inst);
				load_register_arg(gen, inst->binary.operand.source, ARMREG_A3, inst);
				kill_flags(gen);
				kill_argument_registers(gen);
				ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, sizeof(div_t));
				ARM_MOV_REG_REG(gen->cseg, ARMREG_A1, ARMREG_SP);
				call_runtime(gen, gen->runtime->div);

				/* load result from stack position */
				physical_reg = allocate_reg(gen, inst->unary.dest_value, 0);
				assign_reg(gen, physical_reg, inst->unary.dest_value);
				physical_reg->dirty = physical_reg->defined = 1;
				ARM_LDR_IMM(gen->cseg, physical_reg->regno, ARMREG_SP, offsetof(div_t, rem));
				ARM_ADD_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, sizeof(div_t));
			}
			break;

		case cg_op_call:
			emit_call(gen, &inst->call);
			break;
			
		default:
			cg_codegen_emit_simple_inst(gen, inst);
	}
}


/****************************************************************************/
/* Processing of individual basic blocks									*/
/****************************************************************************/


static void init_flags(cg_codegen_t * gen)
{
	/* clean out any association between flags and virtual registers		*/

	gen->flags.prev = gen->flags.next = (cg_physical_reg_t *) 0;
	gen->flags.virtual_reg = (cg_virtual_reg_t *) 0;
	gen->flags.next_use = (cg_inst_t *) 0;
	gen->flags.dirty = gen->flags.defined = 0;	
}


static void begin_block(cg_codegen_t * gen)
{
	/* these assertions will change as soon as we preserve globals			*/
	/* across basic blocks in registers										*/
	
	assert(gen->used_regs.head == (cg_physical_reg_t *) 0);
	assert(gen->used_regs.tail == (cg_physical_reg_t *) 0);
}


static void end_block(cg_codegen_t * gen)
{
	/* deallocate all registers												*/
	while (gen->used_regs.tail != (cg_physical_reg_t *) 0)
	{
		cg_physical_reg_t * physical_reg = gen->used_regs.tail;

		if (physical_reg->virtual_reg != NULL &&
			physical_reg->virtual_reg->physical_reg == physical_reg) 
		{
			deallocate_reg(gen, physical_reg);
		}
		else
		{
			reg_list_remove(&gen->used_regs, physical_reg);
			reg_list_add(&gen->free_regs, physical_reg);
			physical_reg->defined = physical_reg->dirty = 0;
			physical_reg->virtual_reg = NULL;
		}
	}

	init_flags(gen);
}


static void flush_dirty_reg_set(cg_codegen_t * gen, size_t min_reg,
								size_t max_reg)
{
	size_t regno;
	
	for (regno = min_reg; regno <= max_reg; ++regno)
	{
		if (gen->registers[regno].dirty)
		{
			save_reg(gen, &gen->registers[regno], 
					 gen->registers[regno].virtual_reg);
		}
	}
}


static void flush_dirty_regs(cg_codegen_t * gen)
{
	flush_dirty_reg_set(gen, 0, PHYSICAL_REGISTERS - 1);
}


static void flush_dirty_args(cg_codegen_t * gen)
{
	flush_dirty_reg_set(gen, ARMREG_A1, ARMREG_A4);
}


static void process_phi(cg_codegen_t * gen, cg_inst_phi_t * inst)
{
	/* Any processing necessary for phi instructions */
}


static void init_use_chains(cg_codegen_t * gen, cg_block_t * block) 
{
	cg_inst_t * inst;
	size_t num_registers = block->proc->num_registers;
	size_t regno;
	
	cg_inst_list_t *** p_head =
		(cg_inst_list_t ***) _alloca(sizeof(cg_inst_list_t **) * num_registers);
	
	for (regno = 0; regno != num_registers; ++regno) 
	{
		p_head[regno] = &gen->use_chains[regno];
		gen->use_chains[regno] = (cg_inst_list_t *) 0;
	}
	
	for (inst = block->insts; inst != (cg_inst_t *) 0; inst = inst->base.next)
	{
		cg_virtual_reg_t * buffer[64];
		cg_virtual_reg_t **iter, ** end = cg_inst_use(inst, buffer, buffer + 64);
		
		for (iter = buffer; iter != end; ++iter)
		{
			cg_inst_list_t * node = (cg_inst_list_t *)
				cg_heap_allocate(gen->heap, sizeof(cg_inst_list_t));
			
			regno = (*iter)->reg_no;
			
			*p_head[regno] = node;
			node->inst = inst;
			p_head[regno] = &node->next;
		}
	}
}


void cg_codegen_emit_block(cg_codegen_t * gen, cg_block_t * block, int reinit)
{
	cg_inst_t * inst;
	
	gen->current_block = block;
	cg_codegen_define(gen, block->label);
	init_use_chains(gen, block);
	
	if (reinit)
		begin_block(gen);
	
	/************************************************************************/
	/* Process and skip any phi mappings at beginning of block				*/
	/************************************************************************/
	
	for (inst = block->insts; 
		 inst != (cg_inst_t *) 0 && inst->base.kind == cg_inst_phi; 
		 inst = inst->base.next)
	{
		process_phi(gen, &inst->phi);
	}
	
	/************************************************************************/
	/* Emit actual basic block body											*/
	/* This part will be replaced with a more sophisticed instruction		*/
	/* scheduler															*/
	/************************************************************************/
	
	for (; inst != (cg_inst_t *) 0; inst = inst->base.next)
	{
		cg_codegen_emit_inst(gen, inst);
	}
	
	flush_dirty_regs(gen);
	end_block(gen);
	gen->current_block = (cg_block_t *) 0;
}


/****************************************************************************/
/* Processing of procedures													*/
/****************************************************************************/


static void init_arg(cg_codegen_t * gen, cg_physical_reg_t * physical_reg,
					 cg_virtual_reg_t * reg)
{
	/************************************************************************/
	/* init physical reg as physical register bound to reg, marked dirty	*/
	/************************************************************************/

	physical_reg->list = NULL;
	physical_reg->prev = physical_reg->next = (cg_physical_reg_t *) 0;
	physical_reg->virtual_reg = reg;
	reg->physical_reg = physical_reg;
	physical_reg->next_use = (cg_inst_t *) 0;
	physical_reg->dirty = physical_reg->defined = 1;
	
	reg_list_add(&gen->used_regs, physical_reg);
}


static void init_free(cg_codegen_t * gen, cg_physical_reg_t * physical_reg)
{
	/************************************************************************/
	/* init physical_reg as unsed register									*/
	/************************************************************************/

	physical_reg->list = NULL;
	physical_reg->prev = physical_reg->next = (cg_physical_reg_t *) 0;
	physical_reg->virtual_reg = (cg_virtual_reg_t *) 0;
	physical_reg->next_use = (cg_inst_t *) 0;
	physical_reg->dirty = physical_reg->defined = 0;
	
	reg_list_add(&gen->free_regs, physical_reg);
}


static void begin_proc(cg_codegen_t * gen, cg_proc_t * proc)
{
	ARMReg regno;
	cg_virtual_reg_t * reg = proc->registers;
	size_t reg_args = 4, index;
	
	/************************************************************************/
	/* Initialize literal pool for this procedure							*/
	/************************************************************************/

	gen->literal_pool_size = 0;
	gen->literals = (literal_t *) 0;
	gen->literal_base = cg_codegen_create_label(gen);

	/************************************************************************/
	/* Initialize register free lists										*/
	/************************************************************************/

	gen->used_regs.head = gen->used_regs.tail = (cg_physical_reg_t *) 0;
	gen->free_regs.head = gen->free_regs.tail = (cg_physical_reg_t *) 0;

	/************************************************************************/
	/* argument registers - up to 4 arguments can be passed in as registers */
	/* assign them to corresponding physical register marked dirty			*/
	/************************************************************************/
	
	if (proc->num_args <= 4)
	{
		reg_args = proc->num_args;
	}

	for (index = 0; index < reg_args; ++index, reg = reg->next)
	{
		init_arg(gen, &gen->registers[ARMREG_A1 + index], reg);
	}
	
	/************************************************************************/
	/* initialize remaining argument registers as free						*/
	/************************************************************************/
	
	for (; index < 4; ++index)
	{
		init_free(gen, &gen->registers[ARMREG_A1 + index]);
	}
	
	/************************************************************************/
	/* general registers - set them up as free								*/
	/************************************************************************/
	
	for (regno = ARMREG_V1; regno <= ARMREG_V7; ++regno)
	{
		init_free(gen, &gen->registers[regno]);
	}

	init_flags(gen);
}


static void emit_prolog(cg_codegen_t * gen, cg_proc_t * proc)
{
	size_t local_storage;

	ARM_MOV_REG_REG(gen->cseg,	ARMREG_IP, ARMREG_SP);
	ARM_SUB_REG_IMM8(gen->cseg, ARMREG_SP, ARMREG_SP, sizeof(U32) * 4);
	ARM_STMDB(gen->cseg, ARMREG_SP, 
		(1 << ARMREG_V1) | (1 << ARMREG_V2) | (1 << ARMREG_V3) | (1 << ARMREG_V4) |
		(1 << ARMREG_V5) | (1 << ARMREG_V6) | (1 << ARMREG_V7) | (1 << ARMREG_IP) |
		(1 << ARMREG_FP) | (1 << ARMREG_LR));
	ARM_SUB_REG_IMM8(gen->cseg, ARMREG_FP, ARMREG_IP, sizeof(U32) * 4);
	// SP := SP - #local storage

	local_storage = proc->local_storage >> 2;		/* we know it's a multiple of 4	*/

	while (local_storage) 
	{
		/* while generally inefficient, for the type of procedures we are	*/
		/* generating here we'll typically only create 1 or 2 iterations,	*/
		/* in which case the code is optimal.								*/
		size_t decrement = local_storage > 0xff ? 0xff : local_storage;
		ARM_SUB_REG_IMM(gen->cseg, ARMREG_SP, ARMREG_SP, decrement, 0xf);
		local_storage -= decrement;
	}

}


static void emit_epilog(cg_codegen_t * gen, cg_proc_t * proc)
{
	ARM_LDMDB(gen->cseg, ARMREG_FP,
		(1 << ARMREG_V1) | (1 << ARMREG_V2) | (1 << ARMREG_V3) | (1 << ARMREG_V4) |
		(1 << ARMREG_V5) | (1 << ARMREG_V6) | (1 << ARMREG_V7) | (1 << ARMREG_SP) |
		(1 << ARMREG_PC));
}

void cg_codegen_emit_proc(cg_codegen_t * gen, cg_proc_t * proc)
{
	cg_block_t * block;
	literal_t * literal;
	int reinit = 0;						/* flag to distinguish first block  */
	
	gen->use_chains = (cg_inst_list_t **)
		cg_heap_allocate(gen->heap, 
						 sizeof(cg_inst_list_t *) * proc->num_registers);
	
	/************************************************************************/
	/* Create the function prologue											*/
	/************************************************************************/
	
	cg_codegen_define(gen, proc->prologue);
	emit_prolog(gen, proc);
	
	/************************************************************************/
	/* Create labels for all the blocks of the procedure					*/
	/************************************************************************/
	
	for (block = proc->blocks; block != (cg_block_t *) 0; block = block->next)
	{
		block->label = cg_codegen_create_label(gen);
	}
	
	/************************************************************************/
	/* Emit the code for each block of the procedure						*/
	/************************************************************************/
	
	begin_proc(gen, proc);
	
	for (block = proc->blocks; block != (cg_block_t *) 0; block = block->next)
	{
		cg_codegen_emit_block(gen, block, reinit);
		reinit = 1;
	}
	
	/************************************************************************/
	/* Create the function epilogue											*/
	/************************************************************************/
	
	cg_codegen_define(gen, proc->epilogue);
	emit_epilog(gen, proc);

	/************************************************************************/
	/* Append the literal pool												*/
	/************************************************************************/

	cg_segment_align(gen->cseg, sizeof(U32));
	cg_codegen_define(gen, gen->literal_base);

	for (literal = gen->literals; literal != (literal_t *) 0; literal = literal->next)
	{
		cg_segment_emit_u32(gen->cseg, literal->value);
	}

	gen->literal_base = 0;
	gen->literals = 0;
}


/****************************************************************************/
/* Processing of complete modules											*/
/****************************************************************************/


void cg_codegen_emit_module(cg_codegen_t * gen, cg_module_t * module)
{
	cg_proc_t * proc;
	
	for (proc = module->procs; proc != (cg_proc_t *) 0; proc = proc->next)
	{
		proc->prologue = cg_codegen_create_label(gen);
		proc->epilogue = cg_codegen_create_label(gen);
	}
	
	for (proc = module->procs; proc != (cg_proc_t *) 0; proc = proc->next)
	{
		cg_codegen_emit_proc(gen, proc);
	}
}


/****************************************************************************/
/* Assembly output cross-reference handling									*/
/****************************************************************************/


static void fix_ref(cg_segment_t * seg, size_t source, size_t target,
					cg_reference_type_t type) 
{
	switch(type) 
	{
		case cg_reference_branch24:
		{
			U32 instruction = cg_segment_get_u32(seg, target);
			
			// add the correct displacement
			U32 offset = instruction & 0x00FFFFFFu;
			U32 opcode = instruction & 0xFF000000u;
			
			offset = offset + source - target;
			instruction = opcode | (0x00FFFFFFu & offset);
			
			cg_segment_set_u32(seg, target, instruction);
		}
			break;
			
		default:
			assert(0);
	}
}


void cg_codegen_fix_refs(cg_codegen_t * gen)
{
	cg_label_t * label;
	reference_t * ref;
	
	for (label = gen->labels; label != (cg_label_t *) 0; label = label->next) 
	{
		for (ref = label->refs; ref != (reference_t *) 0; ref = ref->next) 
		{
			fix_ref(gen->cseg, label->offset, ref->offset, ref->ref_type);
		}
	}
}


cg_label_t * cg_codegen_create_label(cg_codegen_t * gen) 
{
	cg_label_t * result = (cg_label_t *)
		cg_heap_allocate(gen->heap, sizeof(cg_label_t));
	
	result->refs = (reference_t *) 0;
	result->offset = ~0u;
	result->next = gen->labels;
	gen->labels = result;
	
	return result;
}


void cg_codegen_define(cg_codegen_t * gen, cg_label_t * label)
{
	assert(label->offset == ~0u);
	label->offset = cg_segment_size(gen->cseg);
}


void cg_codegen_reference(cg_codegen_t * gen, cg_label_t * label, 
						  cg_reference_type_t ref_type)
{
	reference_t * ref = (reference_t *)
		cg_heap_allocate(gen->heap, sizeof(reference_t));
	
	ref->offset = cg_segment_size(gen->cseg);
	ref->next = label->refs;
	ref->ref_type = ref_type;
}
									 

cg_segment_t * cg_codegen_segment(cg_codegen_t * gen)
{
	return gen->cseg;
}