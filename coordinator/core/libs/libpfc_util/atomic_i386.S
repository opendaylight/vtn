/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * atomic_i386.S - i386 specific atomic operations.
 */

/*
 * On i386, cmpxchg8b instruction is used to implement 64-bit integer atomic
 * operations. cmpxchg8b clobbers 4 registers, %eax, %ebx, %ecx, and %edx.
 * But PIC on i386 uses %ebx to access GOT. So we can't use cmpxchg8b in
 * inline assembly in C source file.
 */

#ifdef	__x86_64
#error	i386 specific code.
#endif	/* __x86_64 */

#include <pfc/asm_base.h>

PFC_ASM_FILENAME(atomic_i386.S)

/*
 * uint64_t
 * pfc_atomic_cas_uint64(uint64_t *addr, uint64_t newval, uint64_t oldval)
 *	Compare and swap uint64_t value atomically.
 *	Install `newval' into `*addr' if `*addr' equals `oldval'.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_cas_uint64)
	PFC_ASM_PUSH_FP()

	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	/* Read arguments. */
	movl	0x10(%esp), %edi	/* %edi = addr */
	movl	0x14(%esp), %ebx	/* %ecx:%ebx = newval */
	movl	0x18(%esp), %ecx
	movl	0x1c(%esp), %eax	/* %edx:%eax = oldval */
	movl	0x20(%esp), %edx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_cas_uint64)

/*
 * uint64_t
 * pfc_atomic_add_uint64_old(uint64_t *addr, uint64_t value)
 *	Add `value' to `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_add_uint64_old)
	PFC_ASM_PUSH_FP()

	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	movl	0x10(%esp), %edi	/* %edi = addr */
1:
	movl	0x14(%esp), %ebx	/* %ecx:%ebx = value */
	movl	0x18(%esp), %ecx
	movl	(%edi), %eax		/* %edx:%eax = *addr */
	movl	0x4(%edi), %edx
	addl	%eax, %ebx		/* %ecx:%ebx += *addr  */
	adcl	%edx, %ecx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)
	jne	1b

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_add_uint64_old)

/*
 * uint64_t
 * pfc_atomic_inc_uint64_old(uint64_t *addr)
 *	Increment `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_inc_uint64_old)
	PFC_ASM_PUSH_FP()

	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	movl	0x10(%esp), %edi	/* %edi = addr */
1:
	movl	(%edi), %eax		/* %edx:%eax = *addr */
	movl	0x4(%edi), %edx
	movl	%eax, %ebx
	xorl	%ecx, %ecx
	addl	$1, %ebx		/* %ecx:%ebx += *addr + 1  */
	adcl	%edx, %ecx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)
	jne	1b

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_inc_uint64_old)

/*
 * void
 * pfc_atomic_and_uint64(uint64_t *addr, uint64_t bits)
 *	Perform bitwise AND operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_and_uint64)
	PFC_ASM_PUSH_FP()

	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	movl	0x10(%esp), %edi	/* %edi = addr */
1:
	movl	0x14(%esp), %ebx	/* %ecx:%ebx = bits */
	movl	0x18(%esp), %ecx
	movl	(%edi), %eax		/* %edx:%eax = *addr */
	movl	0x4(%edi), %edx
	andl	%eax, %ebx		/* %ecx:%ebx &= *addr  */
	andl	%edx, %ecx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)
	jne	1b

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_and_uint64)

/*
 * void
 * pfc_atomic_or_uint64(uint64_t *addr, uint64_t bits)
 *	Perform bitwise OR operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_or_uint64)
	PFC_ASM_PUSH_FP()

	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	movl	0x10(%esp), %edi	/* %edi = addr */
1:
	movl	0x14(%esp), %ebx	/* %ecx:%ebx = bits */
	movl	0x18(%esp), %ecx
	movl	(%edi), %eax		/* %edx:%eax = *addr */
	movl	0x4(%edi), %edx
	orl	%eax, %ebx		/* %ecx:%ebx |= *addr  */
	orl	%edx, %ecx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)
	jne	1b

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_or_uint64)

/*
 * uint64_t
 * pfc_atomic_swap_uint64(uint64_t *addr, uint64_t value)
 *	Exchange value in `*addr' with `value' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
PFC_ASM_FUNC_ENTRY(pfc_atomic_swap_uint64)
	PFC_ASM_PUSH_FP()
	/* Preserve %ebx and %edi. */
	pushl	%ebx
	pushl	%edi

	movl	0x10(%esp), %edi	/* %edi = addr */
	movl	0x14(%esp), %ebx	/* %ecx:%ebx = value */
	movl	0x18(%esp), %ecx
1:
	movl	(%edi), %eax		/* %edx:%eax = *addr */
	movl	0x4(%edi), %edx

	/* Do compare and exchange. */
	lock
	cmpxchg8b	(%edi)
	jne	1b

	popl	%edi
	popl	%ebx
	PFC_ASM_POP_FP()
	ret
	PFC_ASM_FUNC_END(pfc_atomic_swap_uint64)
