#
# Copyright (c) 2005 Jakub Jermar
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#include <arch/arch.h>
#include <arch/stack.h>
#include <arch/regdef.h>
#include <arch/mm/mmu.h>

.text

.register       %g2, #scratch
.register       %g3, #scratch

.macro WRITE_ALTERNATE_REGISTER reg, bit
	rdpr %pstate, %g1				! save PSTATE.PEF
	wrpr %g0, (\bit | PSTATE_PRIV_BIT), %pstate
	mov %o0, \reg
	wrpr %g0, PSTATE_PRIV_BIT, %pstate
	retl
	wrpr %g1, 0, %pstate				! restore PSTATE.PEF
.endm

.macro READ_ALTERNATE_REGISTER reg, bit
	rdpr %pstate, %g1				! save PSTATE.PEF
	wrpr %g0, (\bit | PSTATE_PRIV_BIT), %pstate
	mov \reg, %o0
	wrpr %g0, PSTATE_PRIV_BIT, %pstate
	retl
	wrpr %g1, 0, %pstate				! restore PSTATE.PEF
.endm

.global write_to_ag_g6
write_to_ag_g6:
	WRITE_ALTERNATE_REGISTER %g6, PSTATE_AG_BIT

.global write_to_ag_g7
write_to_ag_g7:
	WRITE_ALTERNATE_REGISTER %g7, PSTATE_AG_BIT

.global write_to_ig_g6
write_to_ig_g6:
	WRITE_ALTERNATE_REGISTER %g6, PSTATE_IG_BIT

.global read_from_ag_g6
read_from_ag_g6:
	READ_ALTERNATE_REGISTER %g6, PSTATE_AG_BIT

.global read_from_ag_g7
read_from_ag_g7:
	READ_ALTERNATE_REGISTER %g7, PSTATE_AG_BIT

