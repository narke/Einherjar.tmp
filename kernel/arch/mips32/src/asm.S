/*
 * Copyright (c) 2003 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/asm/regname.h>
#include <arch/fpu_context_struct.h>

.text

.macro cp0_read reg
	mfc0 $2, \reg
	j $31
	nop
.endm

.macro cp0_write reg
	mtc0 $4, \reg
	j $31
	nop
.endm

.set noat
.set noreorder
.set nomacro

.global asm_delay_loop
asm_delay_loop:
	j $31
	nop

.global cpu_halt
cpu_halt:
	j cpu_halt
	nop

.macro fpu_gp_save reg ctx
	mfc1 $t0, $\reg
	sw $t0, FPU_CONTEXT_OFFSET_DREGS + \reg * FPU_CONTEXT_DREGS_ITEM_SIZE(\ctx)
.endm

.macro fpu_gp_restore reg ctx
	lw $t0, FPU_CONTEXT_OFFSET_DREGS + \reg * FPU_CONTEXT_DREGS_ITEM_SIZE(\ctx)
	mtc1 $t0, $\reg
.endm

.macro fpu_ct_save reg ctx
	cfc1 $t0, $1
	sw $t0, FPU_CONTEXT_OFFSET_CREGS + \reg * FPU_CONTEXT_CREGS_ITEM_SIZE(\ctx)
.endm

.macro fpu_ct_restore reg ctx
	lw $t0, FPU_CONTEXT_OFFSET_CREGS + \reg * FPU_CONTEZT_CREGS_ITEM_SIZE(\ctx)
	ctc1 $t0, $\reg
.endm

.global fpu_context_save
fpu_context_save:
#ifdef CONFIG_FPU
	fpu_gp_save 0, $a0
	fpu_gp_save 1, $a0
	fpu_gp_save 2, $a0
	fpu_gp_save 3, $a0
	fpu_gp_save 4, $a0
	fpu_gp_save 5, $a0
	fpu_gp_save 6, $a0
	fpu_gp_save 7, $a0
	fpu_gp_save 8, $a0
	fpu_gp_save 9, $a0
	fpu_gp_save 10, $a0
	fpu_gp_save 11, $a0
	fpu_gp_save 12, $a0
	fpu_gp_save 13, $a0
	fpu_gp_save 14, $a0
	fpu_gp_save 15, $a0
	fpu_gp_save 16, $a0
	fpu_gp_save 17, $a0
	fpu_gp_save 18, $a0
	fpu_gp_save 19, $a0
	fpu_gp_save 20, $a0
	fpu_gp_save 21, $a0
	fpu_gp_save 22, $a0
	fpu_gp_save 23, $a0
	fpu_gp_save 24, $a0
	fpu_gp_save 25, $a0
	fpu_gp_save 26, $a0
	fpu_gp_save 27, $a0
	fpu_gp_save 28, $a0
	fpu_gp_save 29, $a0
	fpu_gp_save 30, $a0
	fpu_gp_save 31, $a0
	
	fpu_ct_save 1, $a0
	fpu_ct_save 2, $a0
	fpu_ct_save 3, $a0
	fpu_ct_save 4, $a0
	fpu_ct_save 5, $a0
	fpu_ct_save 6, $a0
	fpu_ct_save 7, $a0
	fpu_ct_save 8, $a0
	fpu_ct_save 9, $a0
	fpu_ct_save 10, $a0
	fpu_ct_save 11, $a0
	fpu_ct_save 12, $a0
	fpu_ct_save 13, $a0
	fpu_ct_save 14, $a0
	fpu_ct_save 15, $a0
	fpu_ct_save 16, $a0
	fpu_ct_save 17, $a0
	fpu_ct_save 18, $a0
	fpu_ct_save 19, $a0
	fpu_ct_save 20, $a0
	fpu_ct_save 21, $a0
	fpu_ct_save 22, $a0
	fpu_ct_save 23, $a0
	fpu_ct_save 24, $a0
	fpu_ct_save 25, $a0
	fpu_ct_save 26, $a0
	fpu_ct_save 27, $a0
	fpu_ct_save 28, $a0
	fpu_ct_save 29, $a0
	fpu_ct_save 30, $a0
	fpu_ct_save 31, $a0
#endif
	j $ra
	nop

.global fpu_context_restore
fpu_context_restore:
#ifdef CONFIG_FPU
	fpu_gp_restore 0, $a0
	fpu_gp_restore 1, $a0
	fpu_gp_restore 2, $a0
	fpu_gp_restore 3, $a0
	fpu_gp_restore 4, $a0
	fpu_gp_restore 5, $a0
	fpu_gp_restore 6, $a0
	fpu_gp_restore 7, $a0
	fpu_gp_restore 8, $a0
	fpu_gp_restore 9, $a0
	fpu_gp_restore 10, $a0
	fpu_gp_restore 11, $a0
	fpu_gp_restore 12, $a0
	fpu_gp_restore 13, $a0
	fpu_gp_restore 14, $a0
	fpu_gp_restore 15, $a0
	fpu_gp_restore 16, $a0
	fpu_gp_restore 17, $a0
	fpu_gp_restore 18, $a0
	fpu_gp_restore 19, $a0
	fpu_gp_restore 20, $a0
	fpu_gp_restore 21, $a0
	fpu_gp_restore 22, $a0
	fpu_gp_restore 23, $a0
	fpu_gp_restore 24, $a0
	fpu_gp_restore 25, $a0
	fpu_gp_restore 26, $a0
	fpu_gp_restore 27, $a0
	fpu_gp_restore 28, $a0
	fpu_gp_restore 29, $a0
	fpu_gp_restore 30, $a0
	fpu_gp_restore 31, $a0
	
	fpu_ct_restore 1, $a0
	fpu_ct_restore 2, $a0
	fpu_ct_restore 3, $a0
	fpu_ct_restore 4, $a0
	fpu_ct_restore 5, $a0
	fpu_ct_restore 6, $a0
	fpu_ct_restore 7, $a0
	fpu_ct_restore 8, $a0
	fpu_ct_restore 9, $a0
	fpu_ct_restore 10, $a0
	fpu_ct_restore 11, $a0
	fpu_ct_restore 12, $a0
	fpu_ct_restore 13, $a0
	fpu_ct_restore 14, $a0
	fpu_ct_restore 15, $a0
	fpu_ct_restore 16, $a0
	fpu_ct_restore 17, $a0
	fpu_ct_restore 18, $a0
	fpu_ct_restore 19, $a0
	fpu_ct_restore 20, $a0
	fpu_ct_restore 21, $a0
	fpu_ct_restore 22, $a0
	fpu_ct_restore 23, $a0
	fpu_ct_restore 24, $a0
	fpu_ct_restore 25, $a0
	fpu_ct_restore 26, $a0
	fpu_ct_restore 27, $a0
	fpu_ct_restore 28, $a0
	fpu_ct_restore 29, $a0
	fpu_ct_restore 30, $a0
	fpu_ct_restore 31, $a0
#endif
	j $ra
	nop

.global early_putchar
early_putchar:
	j $ra
	nop
