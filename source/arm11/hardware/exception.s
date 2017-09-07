/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "asmfunc.h"

.arm
.cpu mpcore
.fpu vfpv2

.extern deinitCpu
.extern guruMeditation
.extern privIrqHandlerTable
.extern irqHandlerTable



ASM_FUNC undefInstrHandler
	msr cpsr_f, #(0<<29)        @ Abuse conditional flags in cpsr for temporary exception type storage
	b exceptionHandler
ASM_FUNC prefetchAbortHandler
	msr cpsr_f, #(1<<29)
	b exceptionHandler
ASM_FUNC dataAbortHandler
	msr cpsr_f, #(2<<29)
ASM_FUNC exceptionHandler
	sub sp, #68
	stmia sp, {r0-r14}^            @ Save all user/system mode regs except pc
	mrs r2, spsr                   @ Get saved cpsr
	mrs r3, cpsr
	lsr r0, r3, #29                @ Get back the exception type from cpsr
	and r1, r2, #0x1F
	cmp r1, #0x10                  @ User mode
	beq exceptionHandler_skip_other_mode
	add r4, sp, #32
	msr cpsr_c, r2
	stmia r4!, {r8-r14}            @ Some regs are written twice but we don't care
	msr cpsr_c, r3
exceptionHandler_skip_other_mode:
	str lr, [sp, #60]              @ Save lr (pc) on exception stack
	str r2, [sp, #64]              @ Save spsr (cpsr) on exception stack
	mov r4, r0
	mov r5, sp
	bl deinitCpu
	mov r0, r4
	mov sp, r5
	mov r1, r5
	b guruMeditation               @ r0 = exception type, r1 = reg dump ptr {r0-r14, pc (unmodified), cpsr}


ASM_FUNC irqHandler
	sub lr, lr, #4
	srsfd sp!, #31               @ Store lr and spsr on system mode stack
	cps #31                      @ Switch to system mode
	stmfd sp!, {r0-r3, r12, lr}
	mov r12, #0x17000000
	orr r12, r12, #0xE00000
	ldr r0, [r12, #0x10C]        @ REG_CPU_II_AKN
	and r1, r0, #0x7F
	cmp r1, #32
	ldrlo r2, =privIrqHandlerTable
	mrclo p15, 0, r3, c0, c0, 5  @ Get CPU ID
	andlo r3, r3, #3
	addlo r2, r2, r3, lsl #7
	ldrhs r2, =irqHandlerTable
	subhs r1, r1, #32
	ldr r3, [r2, r1, lsl #2]
	cmp r3, #0
	beq irqHandler_skip_processing
	stmfd sp!, {r0, r12}
	cpsie i
	blx r3
	cpsid i
	ldmfd sp!, {r0, r12}
irqHandler_skip_processing:
	str r0, [r12, #0x110]        @ REG_CPU_II_EOI
	ldmfd sp!, {r0-r3, r12, lr}
	rfefd sp!                    @ Restore lr (pc) and spsr (cpsr)