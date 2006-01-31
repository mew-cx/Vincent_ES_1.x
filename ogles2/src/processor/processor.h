#ifndef GLES_PROCESSOR_PROCESSOR_H
#define GLES_PROCESSOR_PROCESSOR_H 1

/*
** ==========================================================================
**
** processor.h		A virtual vertex/fragment processor
**
** --------------------------------------------------------------------------
**
** 10-21-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/


#include "gl/state.h"


typedef struct Processor {
	State *		state;					/* reference to GL state		*/
} Processor;


typedef enum InstCode {

/*  ----------------------------------------------------------------------  */
/*  Instruction    C S   Inputs  Output   Description						*/
/*  -----------    - -   ------  ------   --------------------------------	*/

	InstABS,	/* X X   v       v        absolute value					*/
	InstADD,	/* X X   v,v     v        add								*/
	InstARA,	/* X -   a       a        address register add				*/
	InstARL,	/* X -   s       a        address register load				*/
	InstARR,	/* X -   v       a        address register load (round)		*/
	InstBRA,	/* - -   c       -        branch							*/
	InstCAL,	/* - -   c       -        subroutine call					*/
	InstCOS,	/* X X   s       ssss     cosine							*/
	InstDP3,	/* X X   v,v     ssss     3-component dot product			*/
	InstDP4,	/* X X   v,v     ssss     4-component dot product			*/
	InstDPH,	/* X X   v,v     ssss     homogeneous dot product			*/
	InstDST,	/* X X   v,v     v        distance vector					*/
	InstEX2,	/* X X   s       ssss     exponential base 2				*/
	InstEXP,    /* X X   s       v        exponential base 2 (approximate)	*/
	InstFLR,    /* X X   v       v        floor								*/
	InstFRC,    /* X X   v       v        fraction							*/
	InstLG2,    /* X X   s       ssss     logarithm base 2					*/
	InstLIT,    /* X X   v       v        compute light coefficients		*/
	InstLOG,    /* X X   s       v        logarithm base 2 (approximate)	*/
	InstMAD,    /* X X   v,v,v   v        multiply and add					*/
	InstMAX,    /* X X   v,v     v        maximum							*/
	InstMIN,    /* X X   v,v     v        minimum							*/
	InstMOV,    /* X X   v       v        move								*/
	InstMUL,    /* X X   v,v     v        multiply							*/
	InstPOPA,   /* - -   -       a        pop address register				*/
	InstPOW,    /* X X   s,s     ssss     exponentiate						*/
	InstPUSHA,  /* - -   a       -        push address register				*/
	InstRCC,    /* X X   s       ssss     reciprocal (clamped)				*/
	InstRCP,    /* X X   s       ssss     reciprocal						*/
	InstRET,    /* - -   c       -        subroutine return					*/
	InstRSQ,    /* X X   s       ssss     reciprocal square root			*/
	InstSEQ,    /* X X   v,v     v        set on equal						*/
	InstSFL,    /* X X   v,v     v        set on false						*/
	InstSGE,    /* X X   v,v     v        set on greater than or equal		*/
	InstSGT,    /* X X   v,v     v        set on greater than				*/
	InstSIN,    /* X X   s       ssss     sine								*/
	InstSLE,    /* X X   v,v     v        set on less than or equal			*/
	InstSLT,    /* X X   v,v     v        set on less than					*/
	InstSNE,    /* X X   v,v     v        set on not equal					*/
	InstSSG,    /* X X   v       v        set sign							*/
	InstSTR,    /* X X   v,v     v        set on true						*/
	InstSUB,    /* X X   v,v     v        subtract							*/
	InstSWZ,    /* X X   v       v        extended swizzle					*/
	InstTEX,    /* X X   v       v        texture lookup					*/
	InstTXB,    /* X X   v       v        texture lookup with LOD bias		*/
	InstTXL,    /* X X   v       v        texture lookup with explicit LOD	*/
	InstTXP,    /* X X   v       v        projective texture lookup			*/
	InstXPD     /* X X   v,v     v        cross product						*/

	/* -------------------------------------------------------------------- */
	/* The columns "C" and "S" indicate whether the "C", "C0", and "C1"		*/
	/* condition code update modifiers, and the "_SAT" saturation modifiers,*/
	/* respectively, are supported for the opcode.  "v" indicates a			*/
	/* floating-point vector input or output, "s" indicates a floating-		*/
	/* point scalar input, "ssss" indicates a scalar output replicated		*/
	/* across a 4-component result vector, "a" indicates a vector address	*/
	/* register, and "c" indicates a condition code test.					*/
	/* -------------------------------------------------------------------- */
} InstCode;


void ProcInitialize(Processor * proc,		/* processor to initialize		*/
					State * state,			/* GL state to be associated	*/
					GLsizeiptr tempSpace);	/* Temporary space needed		*/

void ProcExecute(Processor * proc,			/* the processor object			*/
				 GLES_gpumem_t ip,			/* instruction pointer			*/
				 GLES_gpumem_t input,		/* input data area base			*/
				 GLES_gpumem_t output,		/* output data area base		*/
				 GLES_gpumem_t params,		/* parameter area base			*/
				 GLES_gpumem_t constants);	/* constant area base			*/

#endif /* ndef GLES_PROCESSOR_PROCESSOR_H */