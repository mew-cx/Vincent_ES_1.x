#ifndef CODEGEN_INSTRUCTION_H
#define CODEGEN_INSTRUCTION_H 1

#pragma once

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


#include "codegen.h"
#include "heap.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct cg_module_t cg_module_t;
typedef union cg_inst_t cg_inst_t;
typedef struct cg_block_t cg_block_t;
typedef struct cg_proc_t cg_proc_t;
typedef struct cg_virtual_reg_t cg_virtual_reg_t;


struct cg_label_t;
struct cg_bitset_t;


typedef enum cg_inst_kind_t {
	cg_inst_none,
	cg_inst_unary,		
	cg_inst_binary,		
	cg_inst_compare,	
	cg_inst_load,	
	cg_inst_store,	
	cg_inst_load_immed,
	cg_inst_branch_label,	
	cg_inst_branch_cond,	
	cg_inst_phi,	
	cg_inst_call,
	cg_inst_ret,						
	
	// ARM specific formats
	cg_inst_arm_unary_immed,	
	cg_inst_arm_unary_shift_reg,	
	cg_inst_arm_unary_shift_immed,	
	cg_inst_arm_binary_immed,	
	cg_inst_arm_binary_shift_reg,	
	cg_inst_arm_binary_shift_immed,	
	cg_inst_arm_compare_immed,	
	cg_inst_arm_compare_shift_reg,	
	cg_inst_arm_compare_shift_immed,	
	cg_inst_arm_load_immed_offset,	
	cg_inst_arm_load_reg_offset,	
	cg_inst_arm_store_immed_offset,	
	cg_inst_arm_store_reg_offset,	
	
	cg_inst_count
}
cg_inst_kind_t;


typedef enum cg_opcode_t {
	cg_op_nop,					
	cg_op_add,		cg_op_and,		cg_op_asr,		cg_op_cmp,		cg_op_div,		
	cg_op_lsl,		cg_op_lsr,		cg_op_mod,		cg_op_mul,		cg_op_neg,	
	cg_op_not,		cg_op_or,		cg_op_sub,		cg_op_xor,									
	cg_op_fadd,		cg_op_fcmp,		cg_op_fdiv,		cg_op_fmul,		cg_op_fneg,		
	cg_op_fsub,		cg_op_finv,		cg_op_fsqrt,					
	cg_op_trunc,	cg_op_round,	cg_op_fcnv,															
	cg_op_beq,		cg_op_bge,		cg_op_ble,		cg_op_bgt,		cg_op_blt,		
	cg_op_bne,		cg_op_bra,			
	cg_op_ldb,		cg_op_ldh,		cg_op_ldi,		cg_op_ldw,		cg_op_stb,		
	cg_op_sth,		cg_op_stw,							
	cg_op_call,		cg_op_ret,		cg_op_phi
}
cg_opcode_t;


typedef struct cg_inst_base_t 
{
	cg_inst_t *			next;
	cg_block_t *		block;
	cg_inst_kind_t		kind;
	cg_opcode_t			opcode;
	int					used : 1;
}
cg_inst_base_t;


typedef enum cg_shift_op_t
{
	cg_shift_lsl,
	cg_shift_lsr,
	cg_shift_asr,
	cg_shift_ror
}
cg_shift_op_t;


typedef union
{
	cg_virtual_reg_t *  source;
	I32					immed;
	
	struct 
	{
		cg_virtual_reg_t *  source;
		cg_virtual_reg_t *  shift;
		cg_shift_op_t		op;
	}					shift_reg;
	
	struct
	{
		cg_virtual_reg_t *  source;
		U32					shift;
		cg_shift_op_t		op;
	}					shift_immed;
}
cg_data_operand_t;

typedef union
{
	cg_virtual_reg_t *  base;
	
	struct
	{
		cg_virtual_reg_t *  base;
		cg_virtual_reg_t *  offset;
	}					reg_offset;
	
	struct
	{
		cg_virtual_reg_t *  base;
		I32					offset;
	}					immed_offset;
}
cg_mem_operand_t;

typedef struct cg_inst_unary_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  dest_value;
	cg_virtual_reg_t *  dest_flags;
	cg_data_operand_t   operand;
	
}
cg_inst_unary_t;

typedef struct cg_inst_binary_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  dest_value;
	cg_virtual_reg_t *  dest_flags;
	cg_virtual_reg_t *  source;
	cg_data_operand_t   operand;	
}
cg_inst_binary_t;

typedef struct cg_inst_compare_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  dest_flags;
	cg_virtual_reg_t *  source;
	cg_data_operand_t   operand;	
}
cg_inst_compare_t;

typedef struct cg_inst_load_immed_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  dest;
	I32					value;
}
cg_inst_load_immed_t;

typedef struct cg_inst_load_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  dest;
	cg_mem_operand_t	mem;
}
cg_inst_load_t;

typedef struct cg_inst_store_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  source;
	cg_mem_operand_t	mem;
}
cg_inst_store_t;

typedef struct cg_virtual_reg_list_t
{
	struct cg_virtual_reg_list_t *  next;
	cg_virtual_reg_t *				reg;
}
cg_virtual_reg_list_t;

typedef struct cg_inst_call_t
{
	cg_inst_base_t			base;
	cg_proc_t *				proc;
	cg_virtual_reg_list_t * args;
	cg_virtual_reg_t *		dest;
}
cg_inst_call_t;

typedef struct cg_inst_ret_t
{
	cg_inst_base_t		base;
	cg_virtual_reg_t *  result;
}
cg_inst_ret_t;

typedef struct cg_inst_phi_t
{
	cg_inst_base_t			base;
	cg_virtual_reg_t *		dest;
	cg_virtual_reg_list_t * regs;
}
cg_inst_phi_t;

typedef struct cg_branch_t
{
	cg_inst_base_t		base;
	cg_block_t *		target;
	cg_virtual_reg_t *  cond;
}
cg_inst_branch_t;

union cg_inst_t
{
	cg_inst_base_t			base;
	cg_inst_unary_t			unary;
	cg_inst_binary_t		binary;
	cg_inst_compare_t		compare;
	cg_inst_load_immed_t	immed;
	cg_inst_load_t			load;
	cg_inst_store_t			store;
	cg_inst_branch_t		branch;
	cg_inst_call_t			call;
	cg_inst_ret_t			ret;
	cg_inst_phi_t			phi;
};


typedef struct cg_block_list_t 
{
	struct cg_block_list_t *	next;
	struct cg_block_t *			block;
}
cg_block_list_t;


struct cg_block_t 
{
	struct cg_block_t *		next;
	cg_proc_t *				proc;
	struct cg_label_t *		label;
	cg_inst_t *				insts;
	cg_inst_t *				last_inst;		/* ptr. to last instruction		*/
	struct cg_bitset_t *	def;			/* set of defined regs			*/
	struct cg_bitset_t *	use;			/* set of used regs				*/
	struct cg_bitset_t *	live_in;		/* set of regs live on entering */
	struct cg_bitset_t *	live_out;		/* set of regs live on leaving  */
	cg_block_list_t *		pred;			/* list of predecessor blocks	*/
	cg_block_list_t *		succ;			/* list of successor blocks		*/
};


typedef struct cg_inst_list_t
{
	struct cg_inst_list_t *		next;
	cg_inst_t *					inst;
}
cg_inst_list_t;


struct cg_physical_reg_t;

typedef enum cg_reg_type_t
{
	cg_reg_type_general,
	cg_reg_type_flags
}
cg_reg_type_t;


struct cg_virtual_reg_t 
{
	cg_virtual_reg_t *  next;
	cg_virtual_reg_t *	representative;		/* for union-find				*/
	struct cg_physical_reg_t *	
						physical_reg;		/* physical register assigned   */
	size_t				reg_no;				/* virtual register number		*/
	int					fp_offset;			/* FP offset for spilling		*/
	cg_inst_t *			def;				/* defining instruction			*/
	cg_inst_list_t *	use;				/* use set						*/
	cg_reg_type_t		type;				/* type of this register		*/
	int					is_global : 1;		/* is this a global register?   */
	int					is_arg : 1;			/* is passed in as argument val.*/
};


struct cg_proc_t 
{
	cg_proc_t *			next;
	cg_module_t *		module;				/* uplink to module				*/
	cg_block_t *		blocks;				/* linked list of blocks		*/
	cg_block_t *		last_block;			/* pointer to last block		*/
	cg_virtual_reg_t *  registers;			/* list of virtual registers	*/
	cg_virtual_reg_t *	last_register;		/* ptr to last register in list */
	size_t				num_registers;		/* number of virtual registers  */
	size_t				num_args;			/* number of arguments			*/
	size_t				local_storage;		/* size of activation record	*/
	struct cg_label_t * prologue;			/* assembly label begin			*/
	struct cg_label_t * epilogue;			/* assembly label end			*/
};


struct cg_module_t
{
	cg_heap_t *			heap;
	cg_proc_t *			procs;
};


/****************************************************************************/
/* Determine the set of defined registers for the instruction passed in.	*/
/* dest points to a buffer into which the result will be stored, limit		*/
/* specifies an upper limit on this buffer. The return value will be		*/
/* pointing right after the last register stored in the destanation area.   */
/****************************************************************************/
cg_virtual_reg_t ** cg_inst_def(const cg_inst_t * inst,
								cg_virtual_reg_t ** dest, 
								cg_virtual_reg_t *const * limit);


/****************************************************************************/
/* Determine the set of used registers for the instruction passed in.		*/
/* dest points to a buffer into which the result will be stored, limit		*/
/* specifies an upper limit on this buffer. The return value will be		*/
/* pointing right after the last register stored in the destanation area.   */
/****************************************************************************/
cg_virtual_reg_t ** cg_inst_use(const cg_inst_t * inst,
								cg_virtual_reg_t ** dest, 
								cg_virtual_reg_t * const * limit);


cg_module_t * cg_module_create(cg_heap_t * heap);


cg_proc_t * cg_proc_create(cg_module_t * module);

cg_block_t * cg_block_create(cg_proc_t * proc);

cg_virtual_reg_t * cg_virtual_reg_create(cg_proc_t * proc, cg_reg_type_t type);

// create a register list
cg_virtual_reg_list_t * cg_create_virtual_reg_list(cg_heap_t * heap, ...);

// add an instruction to a block

cg_inst_t * cg_create_inst_unary(cg_block_t * block, 
								 cg_opcode_t op, 
								 cg_virtual_reg_t * dest, 
								 cg_virtual_reg_t * source);

cg_inst_t * cg_create_inst_unary_s(cg_block_t * block, 
								   cg_opcode_t op, 
								   cg_virtual_reg_t * dest, 
								   cg_virtual_reg_t * flags, 
								   cg_virtual_reg_t * source);

cg_inst_t * cg_create_inst_binary(cg_block_t * block, 
								  cg_opcode_t op, 
								  cg_virtual_reg_t * dest, 
								  cg_virtual_reg_t * source, 
								  cg_virtual_reg_t * operand);

cg_inst_t * cg_create_inst_binary_s(cg_block_t * block, 
									cg_opcode_t op, 
									cg_virtual_reg_t * dest, 
									cg_virtual_reg_t * flags, 
									cg_virtual_reg_t * source, 
									cg_virtual_reg_t * operand);

cg_inst_t * cg_create_inst_compare(cg_block_t * block, 
								   cg_opcode_t op, 
								   cg_virtual_reg_t * dest, 
								   cg_virtual_reg_t * source, 
								   cg_virtual_reg_t * operand);

cg_inst_t * cg_create_inst_load(cg_block_t * block, 
								cg_opcode_t op, 
								cg_virtual_reg_t * dest, 
								cg_virtual_reg_t * mem);

cg_inst_t * cg_create_inst_store(cg_block_t * block, 
								 cg_opcode_t op, 
								 cg_virtual_reg_t * source, 
								 cg_virtual_reg_t * mem);

cg_inst_t * cg_create_inst_load_immed(cg_block_t * block, 
									  cg_opcode_t op, 
									  cg_virtual_reg_t * dest, 
									  U32 value);

cg_inst_t * cg_create_inst_branch_label(cg_block_t * block, 
										cg_opcode_t op, 
										cg_block_t * target);

cg_inst_t * cg_create_inst_branch_cond(cg_block_t * block, 
									   cg_opcode_t op, 
									   cg_virtual_reg_t * flags, 
									   cg_block_t * target);

cg_inst_t * cg_create_inst_phi(cg_block_t * block, 
							   cg_opcode_t op, 
							   cg_virtual_reg_t * dest, 
							   cg_virtual_reg_list_t * regs);

cg_inst_t * cg_create_inst_call_proc(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_proc_t * proc, 
									 cg_virtual_reg_list_t * args);

cg_inst_t * cg_create_inst_call_func(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_virtual_reg_t * dest, 
									 cg_proc_t * proc, 
									 cg_virtual_reg_list_t * args);

cg_inst_t * cg_create_inst_ret(cg_block_t * block, 
							   cg_opcode_t op);

cg_inst_t * cg_create_inst_ret_value(cg_block_t * block, 
									 cg_opcode_t op, 
									 cg_virtual_reg_t * value);

// constructor functions for basic instruction formats:
/*	cg_inst_unary,		
	cg_inst_binary,		
	cg_inst_compare,	
	cg_inst_load,	
	cg_inst_store,	
	cg_inst_load_immed,
	cg_inst_branch_label,	
	cg_inst_branch_cond,	
	cg_inst_phi,	
	cg_inst_call,
	cg_inst_ret,						
*/

/****************************************************************************/
/* The intermediate code needs to be processed by the following				*/
/* functions in this given order											*/
/****************************************************************************/

void cg_module_inst_def(cg_module_t * module);
void cg_module_amode(cg_module_t * module);
void cg_module_eliminate_dead_code(cg_module_t * module);
void cg_module_unify_registers(cg_module_t * module);
void cg_module_allocate_variables(cg_module_t * module);
void cg_module_inst_use_chains(cg_module_t * module);
void cg_module_dataflow(cg_module_t * module);
void cg_module_unify_registers(cg_module_t * module);

#ifdef __cplusplus
}
#endif

#endif //ndef CODEGEN_INSTRUCTION_H