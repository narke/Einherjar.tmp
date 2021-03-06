/*
 * Copyright (c) 2005 Martin Decky
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
#include <arch/msr.h>
#include <arch/istate_struct.h>

.text

.global iret
.global early_putchar

iret:
	
	/* Disable interrupts */
	
	mfmsr r31
	rlwinm r31, r31, 0, 17, 15
	mtmsr r31
	isync
	
	lwz r0, ISTATE_OFFSET_R0(sp)
	lwz r2, ISTATE_OFFSET_R2(sp)
	lwz r3, ISTATE_OFFSET_R3(sp)
	lwz r4, ISTATE_OFFSET_R4(sp)
	lwz r5, ISTATE_OFFSET_R5(sp)
	lwz r6, ISTATE_OFFSET_R6(sp)
	lwz r7, ISTATE_OFFSET_R7(sp)
	lwz r8, ISTATE_OFFSET_R8(sp)
	lwz r9, ISTATE_OFFSET_R9(sp)
	lwz r10, ISTATE_OFFSET_R10(sp)
	lwz r11, ISTATE_OFFSET_R11(sp)
	lwz r13, ISTATE_OFFSET_R13(sp)
	lwz r14, ISTATE_OFFSET_R14(sp)
	lwz r15, ISTATE_OFFSET_R15(sp)
	lwz r16, ISTATE_OFFSET_R16(sp)
	lwz r17, ISTATE_OFFSET_R17(sp)
	lwz r18, ISTATE_OFFSET_R18(sp)
	lwz r19, ISTATE_OFFSET_R19(sp)
	lwz r20, ISTATE_OFFSET_R20(sp)
	lwz r21, ISTATE_OFFSET_R21(sp)
	lwz r22, ISTATE_OFFSET_R22(sp)
	lwz r23, ISTATE_OFFSET_R23(sp)
	lwz r24, ISTATE_OFFSET_R24(sp)
	lwz r25, ISTATE_OFFSET_R25(sp)
	lwz r26, ISTATE_OFFSET_R26(sp)
	lwz r27, ISTATE_OFFSET_R27(sp)
	lwz r28, ISTATE_OFFSET_R28(sp)
	lwz r29, ISTATE_OFFSET_R29(sp)
	lwz r30, ISTATE_OFFSET_R30(sp)
	lwz r31, ISTATE_OFFSET_R31(sp)
	
	lwz r12, ISTATE_OFFSET_CR(sp)
	mtcr r12
	
	lwz r12, ISTATE_OFFSET_PC(sp)
	mtsrr0 r12
	
	lwz r12, ISTATE_OFFSET_SRR1(sp)
	mtsrr1 r12
	
	lwz r12, ISTATE_OFFSET_LR(sp)
	mtlr r12
	
	lwz r12, ISTATE_OFFSET_CTR(sp)
	mtctr r12
	
	lwz r12, ISTATE_OFFSET_XER(sp)
	mtxer r12
	
	lwz r12, ISTATE_OFFSET_R12(sp)
	lwz sp, ISTATE_OFFSET_SP(sp)
	
	rfi


early_putchar:
	blr
