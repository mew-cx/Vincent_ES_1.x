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


#include <stdarg.h>
#include "instruction.h"
#include "bitset.h"
#include "arm-codegen.h"


static cg_virtual_reg_t ** add_reg(cg_virtual_reg_t * reg,
								   cg_virtual_reg_t ** dest, 
								   cg_virtual_reg_t * const * limit)
{
	if (dest != limit) 
		*dest++ = reg;
	
	return dest;
}


static cg_virtual_reg_t ** add_reg_list(cg_virtual_reg_list_t * list,
										cg_virtual_reg_t ** dest, 
										cg_virtual_reg_t * const * limit)
{
	while (list) 
	{
		dest = add_reg(list->reg, dest, limit);
		list = list->next;
	}
	
	return dest;
}

/****************************************************************************/
/* Determine the set of defined registers for the instruction passed in.	*/
/* dest points to a buffer into which the result will be stored, limit		*/
/* specifies an upper limit on this buffer. The return value will be		*/
/* pointing right after the last register stored in the destanation area.   */
/****************************************************************************/
cg_virtual_reg_t ** cg_inst_def(const cg_inst_t * inst,
								cg_virtual_reg_t ** dest, 
								cg_virtual_reg_t * const * limit)
{
	switch (inst->base.kind) 
	{		
		case cg_inst_unary:		
		case cg_inst_arm_unary_immed:	
		case cg_inst_arm_unary_shift_reg:	
		case cg_inst_arm_unary_shift_immed:	
			dest = add_reg(inst->unary.dest_value, dest, limit);
			
			if (inst->unary.dest_flags)
				dest = add_reg(inst->unary.dest_flags, dest, limit);
				
			return dest;
			
		case cg_inst_binary:		
		case cg_inst_arm_binary_immed:	
		case cg_inst_arm_binary_shift_reg:	
		case cg_inst_arm_binary_shift_immed:	
			dest = add_reg(inst->binary.dest_value, dest, limit);
			
			if (inst->binary.dest_flags)
				dest = add_reg(inst->binary.dest_flags, dest, limit);
				
				return dest;
			
		case cg_inst_compare:	
		case cg_inst_arm_compare_immed:	
		case cg_inst_arm_compare_shift_reg:	
		case cg_inst_arm_compare_shift_immed:	
			return add_reg(inst->compare.dest_flags, dest, limit);
			
		case cg_inst_load:	
		case cg_inst_arm_load_immed_offset:	
		case cg_inst_arm_load_reg_offset:	
			return add_reg(inst->load.dest, dest, limit);
			
		case cg_inst_store:	
		case cg_inst_arm_store_immed_offset:	
		case cg_inst_arm_store_reg_offset:	
			return dest;
			
		case cg_inst_load_immed:
			return add_reg(inst->immed.dest, dest, limit);
			
		case cg_inst_branch_label:	
		case cg_inst_branch_cond:	
			return dest;
			
		case cg_inst_phi:	
			return add_reg(inst->phi.dest, dest, limit);
			
		case cg_inst_call:
			if (inst->call.dest)
				return add_reg(inst->call.dest, dest, limit);
			else
				return dest;
			
		case cg_inst_ret:						
			return dest;
		
		// ARM specific formats
		default:
			assert(0);
	}

	return dest;
}


/****************************************************************************/
/* Determine the set of used registers for the instruction passed in.		*/
/* dest points to a buffer into which the result will be stored, limit		*/
/* specifies an upper limit on this buffer. The return value will be		*/
/* pointing right after the last register stored in the destanation area.   */
/****************************************************************************/
cg_virtual_reg_t ** cg_inst_use(const cg_inst_t * inst,
								cg_virtual_reg_t ** dest, 
								cg_virtual_reg_t * const * limit)
{
	switch (inst->base.kind) 
	{		
		case cg_inst_unary:		
			return add_reg(inst->unary.operand.source, dest, limit);
			
		case cg_inst_arm_unary_immed:	
			return dest;
			
		case cg_inst_arm_unary_shift_reg:	
			dest = add_reg(inst->unary.operand.shift_reg.source, dest, limit);
			return add_reg(inst->unary.operand.shift_reg.shift, dest, limit);
			
		case cg_inst_arm_unary_shift_immed:	
			return add_reg(inst->unary.operand.shift_immed.source, dest, limit);
			
		case cg_inst_binary:		
			dest = add_reg(inst->binary.source, dest, limit);
			return add_reg(inst->binary.operand.source, dest, limit);
			
		case cg_inst_arm_binary_immed:	
			return add_reg(inst->binary.source, dest, limit);
			
		case cg_inst_arm_binary_shift_reg:	
			dest = add_reg(inst->binary.source, dest, limit);
			dest = add_reg(inst->binary.operand.shift_reg.source, dest, limit);
			return add_reg(inst->binary.operand.shift_reg.shift, dest, limit);
			
		case cg_inst_arm_binary_shift_immed:	
			dest = add_reg(inst->binary.source, dest, limit);
			return add_reg(inst->binary.operand.shift_immed.source, dest, limit);
			
		case cg_inst_compare:	
			dest = add_reg(inst->compare.source, dest, limit);
			return add_reg(inst->compare.operand.source, dest, limit);
			
		case cg_inst_arm_compare_immed:	
			return add_reg(inst->compare.source, dest, limit);
			
		case cg_inst_arm_compare_shift_reg:	
			dest = add_reg(inst->compare.source, dest, limit);
			dest = add_reg(inst->compare.operand.shift_reg.source, dest, limit);
			return add_reg(inst->compare.operand.shift_reg.shift, dest, limit);
			
		case cg_inst_arm_compare_shift_immed:	
			dest = add_reg(inst->compare.source, dest, limit);
			return add_reg(inst->compare.operand.shift_immed.source, dest, limit);
			
		case cg_inst_load:	
			return add_reg(inst->load.mem.base, dest, limit);
			
		case cg_inst_arm_load_immed_offset:	
			return add_reg(inst->load.mem.immed_offset.base, dest, limit);
			
		case cg_inst_arm_load_reg_offset:	
			dest = add_reg(inst->load.mem.reg_offset.base, dest, limit);
			return add_reg(inst->load.mem.reg_offset.offset, dest, limit);
			
		case cg_inst_store:	
			dest = add_reg(inst->store.source, dest, limit);
			return add_reg(inst->store.mem.base, dest, limit);
			
		case cg_inst_arm_store_immed_offset:	
			dest = add_reg(inst->store.source, dest, limit);
			return add_reg(inst->store.mem.immed_offset.base, dest, limit);

		case cg_inst_arm_store_reg_offset:	
			dest = add_reg(inst->store.source, dest, limit);
			dest = add_reg(inst->store.mem.reg_offset.base, dest, limit);
			return add_reg(inst->store.mem.reg_offset.offset, dest, limit);
			
		case cg_inst_load_immed:
			return dest;
			
		case cg_inst_branch_label:	
			return dest;
			
		case cg_inst_branch_cond:	
			return add_reg(inst->branch.cond, dest, limit);
			
		case cg_inst_phi:	
			return add_reg_list(inst->phi.regs, dest, limit);
			
		case cg_inst_call:
			return add_reg_list(inst->call.args, dest, limit);
			
		case cg_inst_ret:			
			if (inst->ret.result) 
				return add_reg(inst->ret.result, dest, limit);
			else
				return dest;
			
			// ARM specific formats
		default:
			assert(0);
	}

	return dest;
}


void mark_instruction_used(cg_inst_t * inst)
{
	cg_virtual_reg_t * buffer[16];
	cg_virtual_reg_t **iter, ** end;
	
	if (inst->base.used) 
		return;

	inst->base.used = 1;

	end = cg_inst_use(inst, buffer, buffer + 16);

	for (iter = buffer; iter != end; ++iter)
	{
		cg_virtual_reg_t * reg = *iter;
		
		if (reg->def && !reg->def->base.used)
			mark_instruction_used(reg->def);
	}
}


void cg_module_eliminate_dead_code(cg_module_t * module)
{
	cg_proc_t * proc;
	cg_block_t * block;
	cg_inst_t * inst, ** pinst;

	/************************************************************************/
	/* Mark all instructions as unused										*/
	/************************************************************************/

	for (proc = module->procs; proc; proc = proc->next)
	{
		for (block = proc->blocks; block; block = block->next)
		{
			for (inst = block->insts; inst; inst = inst->base.next)
			{
				inst->base.used = 0;
			}
		}
	}

	/************************************************************************/
	/* mark used instructions												*/
	/************************************************************************/

	for (proc = module->procs; proc; proc = proc->next)
	{
		for (block = proc->blocks; block; block = block->next)
		{
			for (inst = block->insts; inst; inst = inst->base.next)
			{
				if (inst->base.kind == cg_inst_store ||
					inst->base.kind == cg_inst_arm_store_immed_offset ||
					inst->base.kind == cg_inst_arm_store_reg_offset ||
					inst->base.kind == cg_inst_ret ||
					inst->base.kind == cg_inst_call ||
					inst->base.kind == cg_inst_branch_cond ||
					inst->base.kind == cg_inst_branch_label) 
				{
					mark_instruction_used(inst);
				}
			}
		}
	}

	/************************************************************************/
	/* remove un-used instructions											*/
	/************************************************************************/

	for (proc = module->procs; proc; proc = proc->next)
	{
		for (block = proc->blocks; block; block = block->next)
		{
			for (pinst = &block->insts; *pinst; )
			{
				if ((*pinst)->base.used) 
				{
					pinst = &(*pinst)->base.next;
				}
				else 
				{
					*pinst = (*pinst)->base.next;
				}
			}
		}
	}
}


static void block_dataflow(cg_block_t * block)
{
	cg_inst_t * inst;

	block->def =		cg_bitset_create(block->proc->module->heap, block->proc->num_registers);
	block->use =		cg_bitset_create(block->proc->module->heap, block->proc->num_registers);
	block->live_in =	cg_bitset_create(block->proc->module->heap, block->proc->num_registers);
	block->live_out =	cg_bitset_create(block->proc->module->heap, block->proc->num_registers);

	for (inst = block->insts; inst; inst = inst->base.next)
	{
		cg_virtual_reg_t * buffer[64];
		cg_virtual_reg_t **iter, ** end = cg_inst_use(inst, buffer, buffer + 64);

		for (iter = buffer; iter != end; ++iter)
		{
			cg_virtual_reg_t * use = *iter;

			if (!CG_BITSET_TEST(block->def, use->reg_no))
			{
				/* if the register has not been defined yet it means we received it as	*/
				/* input to the block													*/

				CG_BITSET_SET(block->use, use->reg_no);
			}
		}

		end = cg_inst_def(inst, buffer, buffer + 64);

		for (iter = buffer; iter != end; ++iter)
		{
			cg_virtual_reg_t * def = *iter;
			CG_BITSET_SET(block->def, def->reg_no);
		}
	}

	cg_bitset_assign(block->live_in, block->use);
}


static void proc_dataflow(cg_proc_t * proc)
{
	int changed;
	cg_block_t * block;
	cg_block_list_t * list;

	// for each block establish def and use sets

	for (block = proc->blocks; block; block = block->next)
		block_dataflow(block);

	// repeat until no further change:
	//	live_out(b) := U live_in(b), b in succ(b)
	//  live_in(b) := live_in(b) U (live_out(b) \ def(b))
	do
	{
		changed = 0;

		for (block = proc->blocks; block; block = block->next)
		{
			for (list = block->succ; list; list = list->next)
				changed |= cg_bitset_union(block->live_out, list->block->live_in);

			changed |= cg_bitset_union_minus(block->live_in, block->live_out, block->def);
		}

	}
	while (changed);
}


static void add_successor(cg_block_t * block, cg_block_t * succ)
{
	cg_block_list_t * list = cg_heap_allocate(block->proc->module->heap, sizeof(cg_block_list_t));
	list->block = succ;
	list->next = block->succ;
	block->succ = list;
}


static void add_predecessor(cg_block_t * block, cg_block_t * pred)
{
	cg_block_list_t * list = cg_heap_allocate(block->proc->module->heap, sizeof(cg_block_list_t));
	list->block = pred;
	list->next = block->pred;
	block->pred = list;
}


static void proc_controlflow(cg_proc_t * proc)
{
	// for each block determine list of preedecessor and successor blocks
	cg_block_t * block;
	cg_inst_t * inst;

	for (block = proc->blocks; block; block = block->next)
	{
		int last_inst_is_bra = 0;

		for (inst = block->insts; inst; inst = inst->base.next)
		{
			if (inst->base.kind == cg_inst_branch_cond ||
				inst->base.kind == cg_inst_branch_label) 
			{
				cg_block_t * target = inst->branch.target->block;

				add_successor(block, target);
				add_predecessor(target, block);
			}

			last_inst_is_bra = inst->base.opcode != cg_op_bra;
		}

		if (!last_inst_is_bra && block->next) 
		{
			add_successor(block, block->next);
			add_predecessor(block->next, block);
		}
	}
}


void cg_module_dataflow(cg_module_t * module)
{
	// perform def/use and liveliness analysis
	cg_proc_t * proc;

	for (proc = module->procs; proc != (cg_proc_t *) 0; proc = proc->next)
	{
		proc_controlflow(proc);
		proc_dataflow(proc);
	}
}


static int is_arm_unary_data_opcode(cg_opcode_t opcode) {
	switch (opcode) {
		case cg_op_neg:
		case cg_op_fneg:
			return 1;

		default:
			return 0;
	}
}

static int is_arm_binary_data_opcode(cg_opcode_t opcode) {
	switch (opcode) {
		case cg_op_add:
		case cg_op_and:
		case cg_op_or:
		case cg_op_sub:
		case cg_op_xor:
		case cg_op_fadd:
		case cg_op_fsub:
			return 1;

		default:
			return 0;
	}
}

static int is_shift_or_rotate(cg_opcode_t opcode) {
	switch (opcode) {
		case cg_op_asr:
		case cg_op_lsr:
		case cg_op_lsl:
			return 1;

		default:
			return 0;
	}
}

static int is_arm_data_proc_immediate(U32 value) {
	// preliminary rule
	int shift;

	if ((value & 0xff) == value) {
		return 1;
	}

	for (shift = 2; shift <= 12; shift += 2) {
		if (((0xff << shift) & value) == value)
			return 1;
	}

	return 0;
}


typedef enum
{
	amode_register,
	amode_immed,
	amode_shift_immed,
	amode_shift_reg
}
amode_t;


typedef enum
{
	dmode_register,
	dmode_immed_offset,
	dmode_reg_offset
}
dmode_t;



static amode_t consolidate_amode(cg_data_operand_t * operand)
{
	cg_inst_t * source_inst;

	if (!operand->source->def)
		return amode_register;

	source_inst = operand->source->def;

	switch (source_inst->base.kind)
	{
	case cg_inst_load_immed:
		// constant
		{
			if (is_arm_const(source_inst->immed.value))
			{
				operand->immed = source_inst->immed.value;
				return amode_immed;
			}
		}

		break;

	case cg_inst_binary:
		// check for general shift -> shift reg

		{
			switch (source_inst->base.opcode)
			{
			case cg_op_asr:
				operand->shift_reg.source = source_inst->binary.source;
				operand->shift_reg.op = cg_shift_asr;
				operand->shift_reg.shift = source_inst->binary.operand.source;
				return amode_shift_reg;

			case cg_op_lsr:
				operand->shift_reg.source = source_inst->binary.source;
				operand->shift_reg.op = cg_shift_lsr;
				operand->shift_reg.shift = source_inst->binary.operand.source;
				return amode_shift_reg;

			case cg_op_lsl:
				operand->shift_reg.source = source_inst->binary.source;
				operand->shift_reg.op = cg_shift_lsl;
				operand->shift_reg.shift = source_inst->binary.operand.source;
				return amode_shift_reg;
			}
		}

		break;

	case cg_inst_arm_binary_immed:
		// maybe we already consolidated the shift -> shift immed

		{
			switch (source_inst->base.opcode)
			{
			case cg_op_asr:
				operand->shift_immed.source = source_inst->binary.source;
				operand->shift_immed.op = cg_shift_asr;
				operand->shift_immed.shift = source_inst->binary.operand.immed;
				return amode_shift_immed;

			case cg_op_lsr:
				operand->shift_immed.source = source_inst->binary.source;
				operand->shift_immed.op = cg_shift_lsr;
				operand->shift_immed.shift = source_inst->binary.operand.immed;
				return amode_shift_immed;

			case cg_op_lsl:
				operand->shift_immed.source = source_inst->binary.source;
				operand->shift_immed.op = cg_shift_lsl;
				operand->shift_immed.shift = source_inst->binary.operand.immed;
				return amode_shift_immed;
			}
		}

		break;

	default:
		break;
	}

	return amode_register;
}


static dmode_t consolidate_dmode(cg_mem_operand_t * operand)
{
	cg_inst_t * source_inst = operand->base->def;

	if (!source_inst)
		return dmode_register;

	if (source_inst->base.opcode == cg_op_add &&
		source_inst->base.kind == cg_inst_binary)
	{
		operand->reg_offset.base = source_inst->binary.source;
		operand->reg_offset.offset = source_inst->binary.operand.source;
		return dmode_reg_offset;
	}
	else if (source_inst->base.opcode == cg_op_add &&
		source_inst->base.kind == cg_inst_arm_binary_immed &&
		source_inst->binary.operand.immed >= 0 &&
		source_inst->binary.operand.immed <= 0xff)
	{
		operand->immed_offset.base = source_inst->binary.source;
		operand->immed_offset.offset = source_inst->binary.operand.immed;
		return dmode_immed_offset;
	}

	return dmode_register;
}


static void inst_amode(cg_inst_t * inst)
{
	switch (inst->base.kind)
	{
	case cg_inst_unary:
		if (is_arm_unary_data_opcode(inst->base.opcode))
		{
			amode_t amode = consolidate_amode(&inst->unary.operand);

			switch (amode)
			{
			case amode_immed:
				inst->base.kind = cg_inst_arm_unary_immed;
				break;

			case amode_shift_immed:
				inst->base.kind = cg_inst_arm_unary_shift_immed;
				break;

			case amode_shift_reg:
				inst->base.kind = cg_inst_arm_unary_shift_reg;
				break;

			default:
				break;
			}

		}

		break;

	case cg_inst_binary:
		if (is_arm_binary_data_opcode(inst->base.opcode))
		{
			amode_t amode = consolidate_amode(&inst->binary.operand);

			switch (amode)
			{
			case amode_immed:
				inst->base.kind = cg_inst_arm_binary_immed;
				break;

			case amode_shift_immed:
				inst->base.kind = cg_inst_arm_binary_shift_immed;
				break;

			case amode_shift_reg:
				inst->base.kind = cg_inst_arm_binary_shift_reg;
				break;

			default:
				break;
			}

		} 
		else switch (inst->base.opcode)
		{
		case cg_op_asr:
		case cg_op_lsr:
		case cg_op_lsl:
			{
				cg_inst_t * source_inst = inst->binary.operand.source->def;

				if (source_inst && source_inst->base.kind == cg_inst_load_immed)
				{
					inst->binary.operand.immed = source_inst->immed.value;
					inst->base.kind = cg_inst_arm_binary_immed;
				}
			}

			break;

		}

		break;

	case cg_inst_compare:
		{
			amode_t amode = consolidate_amode(&inst->compare.operand);

			switch (amode)
			{
			case amode_immed:
				inst->base.kind = cg_inst_arm_compare_immed;
				break;

			case amode_shift_immed:
				inst->base.kind = cg_inst_arm_compare_shift_immed;
				break;

			case amode_shift_reg:
				inst->base.kind = cg_inst_arm_compare_shift_reg;
				break;

			default:
				break;
			}
		}

		break;

	case cg_inst_load:
		{
			dmode_t dmode = consolidate_dmode(&inst->load.mem);

			switch (dmode)
			{
			case dmode_reg_offset:
				inst->base.kind = cg_inst_arm_load_reg_offset;
				break;

			case dmode_immed_offset:
				inst->base.kind = cg_inst_arm_load_immed_offset;
				break;

			default:
				break;
			}
		}

		break;

	case cg_inst_store:
		{
			dmode_t dmode = consolidate_dmode(&inst->store.mem);

			switch (dmode)
			{
			case dmode_reg_offset:
				inst->base.kind = cg_inst_arm_store_reg_offset;
				break;

			case dmode_immed_offset:
				inst->base.kind = cg_inst_arm_store_immed_offset;
				break;

			default:
				break;
			}
		}

		break;

	default:
		break;
	}
}


void cg_module_amode(cg_module_t * module)
{
	cg_proc_t * proc;
	cg_block_t * block;
	cg_inst_t * inst;

	for (proc = module->procs; proc; proc = proc->next)
	{
		for (block = proc->blocks; block; block = block->next)
		{
			for (inst = block->insts; inst; inst = inst->base.next)
			{
				inst_amode(inst);
			}
		}
	}
}


#define SAVE_AREA_SIZE (10 * sizeof(U32))		/* this really depends on the function prolog */


static void proc_allocate_variables(cg_proc_t * proc)
{
	cg_virtual_reg_t * reg;
	size_t arg_count = proc->num_args;
	size_t arg_offset = 0;

	proc->num_registers = 0;
	proc->local_storage = 0;

	for (reg = proc->registers; reg; reg = reg->next)
	{
		reg->fp_offset = ~0;
	}

	for (reg = proc->registers; reg && arg_count; reg = reg->next, arg_count--)
	{
		reg->reg_no = reg->representative->reg_no = proc->num_registers++;
		reg->fp_offset = reg->representative->fp_offset = arg_offset;
		arg_offset += sizeof(U32);
	}

	for (; reg; reg = reg->next)
	{
		if (reg->representative->fp_offset == ~0)
		{
			reg->representative->reg_no = proc->num_registers++;
			proc->local_storage += sizeof(U32);
			reg->representative->fp_offset = - (int) proc->local_storage - SAVE_AREA_SIZE;	
		}

		reg->reg_no = reg->representative->reg_no;
		reg->fp_offset = reg->representative->fp_offset;
	}
}


void cg_module_allocate_variables(cg_module_t * module)
{
	// allocate local variables in stack frame
	cg_proc_t * proc;

	cg_module_unify_registers(module);

	for (proc = module->procs; proc; proc = proc->next)
		proc_allocate_variables(proc);
}


static cg_virtual_reg_t * find_set(cg_virtual_reg_t * reg)
{
	cg_virtual_reg_t * result;

	if (!reg->representative || reg->representative == reg) 
		return reg;

	result  = find_set(reg->representative);

	if (result != reg->representative)
		reg->representative = result;

	return result;
}


static void find_union(cg_virtual_reg_t * first, cg_virtual_reg_t * second)
{
	cg_virtual_reg_t * first_set = find_set(first);
	cg_virtual_reg_t * second_set = find_set(second);

	second_set->representative = first_set;
}


void cg_module_unify_registers(cg_module_t * module)
{
	cg_proc_t * proc;
	cg_block_t * block;
	cg_inst_t * inst;
	cg_virtual_reg_t * reg;

	for (proc = module->procs; proc; proc = proc->next)
	{
		/********************************************************************/
		/* Initialize all registers as being distinct						*/
		/********************************************************************/

		for (reg = proc->registers; reg; reg = reg->next)
		{
			reg->representative = reg;
		}

		/********************************************************************/
		/* Perform union-find for all registers that occur in a phi			*/
		/* instruction														*/
		/********************************************************************/

		for (block = proc->blocks; block; block = block->next)
		{
			cg_virtual_reg_list_t * list;

			for (inst = block->insts; inst && inst->base.opcode == cg_op_phi; 
				 inst = inst->base.next)
			{
				for (list = inst->phi.regs; list; list = list->next)
				{
					find_union(inst->phi.dest, list->reg);
				}
			}
		}

		/********************************************************************/
		/* Flatten out indirections so that there is at most one level of	*/
		/* indirection between any register and its representative			*/
		/********************************************************************/

		for (reg = proc->registers; reg; reg = reg->next)
		{
			find_set(reg);
		}
	}
}



static void proc_inst_def(cg_proc_t * proc)
{
	cg_virtual_reg_t * reg;
	cg_block_t * block;
	cg_inst_t * inst;

	/************************************************************************/
	/* Clear out def and use information associated with virtual registers	*/
	/************************************************************************/

	for (reg = proc->registers; reg; reg = reg->next)
	{
		reg->def = (cg_inst_t *) 0;
		reg->use = (cg_inst_list_t *) 0;
	}

	/************************************************************************/
	/* Add def qand use information for each register						*/
	/************************************************************************/

	for (block = proc->blocks; proc; block = block->next)
	{
		for (inst = block->insts; inst; inst = inst->base.next)
		{
			cg_virtual_reg_t * buffer[64];
			cg_virtual_reg_t **iter, ** end = cg_inst_def(inst, buffer, buffer + 64);

			for (iter = buffer; iter != end; ++iter)
			{
				cg_virtual_reg_t * reg = *iter;
				assert(reg->def == (cg_inst_t *) 0);
				reg->def = inst;
			}
		}
	}
}


void cg_module_inst_def(cg_module_t * module)
{
	cg_proc_t * proc;

	for (proc = module->procs; proc; proc = proc->next)
		proc_inst_def(proc);
}


static void proc_inst_use_chains(cg_proc_t * proc)
{
	cg_virtual_reg_t * reg;
	cg_block_t * block;
	cg_inst_t * inst;

	/************************************************************************/
	/* Clear out def and use information associated with virtual registers	*/
	/************************************************************************/

	for (reg = proc->registers; reg; reg = reg->next)
	{
		reg->def = (cg_inst_t *) 0;
		reg->use = (cg_inst_list_t *) 0;
	}

	/************************************************************************/
	/* Add def qand use information for each register						*/
	/************************************************************************/

	for (block = proc->blocks; proc; block = block->next)
	{
		for (inst = block->insts; inst; inst = inst->base.next)
		{
			cg_virtual_reg_t * buffer[64];
			cg_virtual_reg_t **iter, ** end = cg_inst_use(inst, buffer, buffer + 64);

			for (iter = buffer; iter != end; ++iter)
			{
				cg_virtual_reg_t * reg = *iter;
				cg_inst_list_t * list = cg_heap_allocate(proc->module->heap, sizeof(cg_inst_list_t));
				list->inst = inst;
				list->next = reg->use;
				reg->use = list;
			}
		}
	}
}


void cg_module_inst_use_chains(cg_module_t * module)
{
	cg_proc_t * proc;

	for (proc = module->procs; proc; proc = proc->next)
		proc_inst_use_chains(proc);
}


/****************************************************************************/

cg_module_t * cg_module_create(cg_heap_t * heap)
{
	cg_module_t * module = cg_heap_allocate(heap, sizeof(cg_module_t));
	module->heap = heap;

	return module;
}


cg_proc_t * cg_proc_create(cg_module_t * module)
{
	cg_proc_t * proc = cg_heap_allocate(module->heap, sizeof (cg_proc_t));
	proc->next = module->procs;
	module->procs = proc;
	proc->module = module;

	return proc;
}

cg_block_t * cg_block_create(cg_proc_t * proc)
{
	cg_block_t * block = cg_heap_allocate(proc->module->heap, sizeof (cg_block_t));

	if (proc->blocks)
	{
		proc->last_block->next = block;
		proc->last_block = block;
	}
	else 
	{
		proc->blocks = proc->last_block = block;
	}

	block->proc = proc;

	return block;
}


cg_virtual_reg_t * cg_virtual_reg_create(cg_proc_t * proc, cg_reg_type_t type)
{
	cg_virtual_reg_t * reg = cg_heap_allocate(proc->module->heap, sizeof (cg_virtual_reg_t));
	reg->reg_no = proc->num_registers++;
	reg->type = type;

	if (proc->registers)
	{
		proc->last_register->next = reg;
		proc->last_register = reg;
	}
	else
	{
		proc->registers = proc->last_register = reg;
	}

	return reg;
}


cg_virtual_reg_list_t * cg_create_virtual_reg_list(cg_heap_t * heap, ...)
{
	va_list marker;
	cg_virtual_reg_list_t * result = NULL, ** current = &result;

	va_start(marker, heap);     /* Initialize variable arguments. */

	do
	{
		cg_virtual_reg_t * reg = va_arg(marker, cg_virtual_reg_t *);

		if (!reg)
			break;

		*current = cg_heap_allocate(heap, sizeof(cg_virtual_reg_list_t));
		(*current)->reg = reg;

		current = &(*current)->next;
	}
	while (1);

	va_end(marker);              /* Reset variable arguments.      */

	return result;
}


// add an instruction to a block
static void block_add(cg_block_t * block, cg_inst_t * inst)
{
	inst->base.block = block;

	if (block->insts)
	{
		block->last_inst->base.next = inst;
		block->last_inst = inst;
	}
	else 
	{
		block->insts = block->last_inst = inst;
	}
}


static cg_inst_t * inst_create(cg_block_t * block, size_t size, 
							   cg_inst_kind_t kind, cg_opcode_t op)
{
	cg_inst_t * inst = (cg_inst_t *)
		cg_heap_allocate(block->proc->module->heap, size);
	block_add(block, inst);

	inst->base.kind = cg_inst_unary;
	inst->base.opcode = op;

	return inst;
}


cg_inst_t * cg_create_inst_unary(cg_block_t * block, 
								 cg_opcode_t op, 
								 cg_virtual_reg_t * dest, 
								 cg_virtual_reg_t * source)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_unary_t), cg_inst_unary, op);

	assert(dest->type == cg_reg_type_general);
	assert(source->type == cg_reg_type_general);

	inst->unary.dest_value = dest;
	inst->unary.operand.source = source;

	return inst;
}


cg_inst_t * cg_create_inst_unary_s(cg_block_t * block, 
								   cg_opcode_t op, 
								   cg_virtual_reg_t * dest, 
								   cg_virtual_reg_t * flags, 
								   cg_virtual_reg_t * source)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_unary_t), cg_inst_unary, op);

	assert(dest->type == cg_reg_type_general);
	assert(flags->type == cg_reg_type_flags);
	assert(source->type == cg_reg_type_general);

	inst->unary.dest_value = dest;
	inst->unary.dest_flags = flags;
	inst->unary.operand.source = source;

	return inst;
}


cg_inst_t * cg_create_inst_binary(cg_block_t * block, 
								  cg_opcode_t op, 
								  cg_virtual_reg_t * dest, 
								  cg_virtual_reg_t * source, 
								  cg_virtual_reg_t * operand)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_binary_t), cg_inst_binary, op);

	assert(dest->type == cg_reg_type_general);
	assert(source->type == cg_reg_type_general);
	assert(operand->type == cg_reg_type_general);

	inst->binary.dest_value = dest;
	inst->binary.source = source;
	inst->binary.operand.source = operand;

	return inst;
}


cg_inst_t * cg_create_inst_binary_s(cg_block_t * block, 
									cg_opcode_t op, 
									cg_virtual_reg_t * dest, 
									cg_virtual_reg_t * flags, 
									cg_virtual_reg_t * source, 
									cg_virtual_reg_t * operand)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_binary_t), cg_inst_binary, op);

	assert(dest->type == cg_reg_type_general);
	assert(flags->type == cg_reg_type_flags);
	assert(source->type == cg_reg_type_general);
	assert(operand->type == cg_reg_type_general);

	inst->binary.dest_value = dest;
	inst->binary.dest_flags = flags;
	inst->binary.source = source;
	inst->binary.operand.source = operand;

	return inst;
}


cg_inst_t * cg_create_inst_compare(cg_block_t * block, 
								   cg_opcode_t op, 
								   cg_virtual_reg_t * dest, 
								   cg_virtual_reg_t * source, 
								   cg_virtual_reg_t * operand)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_compare_t), cg_inst_compare, op);

	assert(dest->type == cg_reg_type_flags);
	assert(source->type == cg_reg_type_general);
	assert(operand->type == cg_reg_type_general);

	inst->compare.dest_flags = dest;
	inst->compare.source = source;
	inst->compare.operand.source = operand;

	return inst;
}


cg_inst_t * cg_create_inst_load(cg_block_t * block, 
								cg_opcode_t op, 
								cg_virtual_reg_t * dest, 
								cg_virtual_reg_t * mem)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_load_t), cg_inst_load, op);

	assert(dest->type == cg_reg_type_general);
	assert(mem->type == cg_reg_type_general);

	inst->load.dest = dest;
	inst->load.mem.base = mem;

	return inst;
}


cg_inst_t * cg_create_inst_store(cg_block_t * block, 
								 cg_opcode_t op, 
								 cg_virtual_reg_t * source, 
								 cg_virtual_reg_t * mem)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_store_t), cg_inst_store, op);

	assert(source->type == cg_reg_type_general);
	assert(mem->type == cg_reg_type_general);

	inst->store.source = source;
	inst->store.mem.base = mem;

	return inst;
}


cg_inst_t * cg_create_inst_load_immed(cg_block_t * block, 
									  cg_opcode_t op, 
									  cg_virtual_reg_t * dest, 
									  U32 value)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_load_immed_t), cg_inst_load_immed, op);

	assert(dest->type == cg_reg_type_general);

	inst->immed.dest = dest;
	inst->immed.value = value;

	return inst;
}


cg_inst_t * cg_create_inst_branch_label(cg_block_t * block, 
										cg_opcode_t op, 
										cg_block_ref_t * target)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_branch_t), cg_inst_branch_label, op);

	inst->branch.target = target;

	return inst;
}


cg_inst_t * cg_create_inst_branch_cond(cg_block_t * block, 
									   cg_opcode_t op, 
									   cg_virtual_reg_t * flags, 
									   cg_block_ref_t * target)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_branch_t), cg_inst_branch_cond, op);

	assert(flags->type == cg_reg_type_flags);

	inst->branch.cond = flags;
	inst->branch.target = target;

	return inst;
}


cg_inst_t * cg_create_inst_phi(cg_block_t * block, 
							   cg_opcode_t op, 
							   cg_virtual_reg_t * dest, 
							   cg_virtual_reg_list_t * regs)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_phi_t), cg_inst_phi, op);

	inst->phi.dest = dest;
	inst->phi.regs = regs;

	return inst;
}


cg_inst_t * cg_create_inst_call_proc(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_proc_t * proc, 
									 cg_virtual_reg_list_t * args)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_call_t), cg_inst_call, op);

	inst->call.proc = proc;
	inst->call.args = args;

	return inst;
}


cg_inst_t * cg_create_inst_call_func(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_virtual_reg_t * dest, 
									 cg_proc_t * proc, 
									 cg_virtual_reg_list_t * args)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_call_t), cg_inst_call, op);

	assert(dest->type == cg_reg_type_general);

	inst->call.dest = dest;
	inst->call.proc = proc;
	inst->call.args = args;

	return inst;
}


cg_inst_t * cg_create_inst_ret(cg_block_t * block, 
							   cg_opcode_t op)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_ret_t), cg_inst_ret, op);

	/* nothing */

	return inst;
}


cg_inst_t * cg_create_inst_ret_value(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_virtual_reg_t * value)
{
	cg_inst_t * inst = inst_create(block, sizeof(cg_inst_ret_t), cg_inst_ret, op);

	assert(value->type == cg_reg_type_general);

	inst->ret.result = value;

	return inst;
}


cg_block_ref_t * cg_block_ref_create(cg_proc_t * proc)
{
	cg_block_ref_t * result = (cg_block_ref_t *) cg_heap_allocate(proc->module->heap, sizeof(cg_block_ref_t));
	return result;
}
