/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_X86_PFC_ASM_BASE_H
#define	_PFC_ARCH_X86_PFC_ASM_BASE_H

/*
 * Common definitions for code written in i386/x86_64 assembly language.
 */

#ifdef	_PFC_ASM

/*
 * Declare source file name.
 */
#define	PFC_ASM_FILENAME(name)		\
	.file	#name

/*
 * Declare object size for the symbol table.
 */
#define	PFC_ASM_SET_SIZE(name)		\
	.size	name, [.-name]

/*
 * Alignment required for a function address.
 */
#define	PFC_ASM_FUNC_ALIGN	16

/*
 * PFC_ASM_ENTRY()
 *	Declare a global function.
 */
#define	PFC_ASM_FUNC_ENTRY(name)			\
	.text;					\
	.align	PFC_ASM_FUNC_ALIGN;		\
	.globl	name;				\
	.type	name, @function;		\
name:

/*
 * PFC_ASM_FUNC_END(name)
 *	Declare the end of the function code.
 */
#define	PFC_ASM_FUNC_END(name)		PFC_ASM_SET_SIZE(name)

/*
 * Preserve and restore frame pointer.
 */
#define	PFC_ASM_PUSH_FP()		\
	pushl	%ebp;			\
	mov	%esp, %ebp

#define	PFC_ASM_POP_FP()		\
	popl	%ebp

#else	/* !_PFC_ASM */

/*
 * Instruction suffixes which determine access size.
 */
#define	PFC_X86_INST_UINT8		"b"
#define	PFC_X86_INST_UINT16		"w"
#define	PFC_X86_INST_UINT32		"l"
#ifdef	PFC_LP64
#define	PFC_X86_INST_UINT64		"q"
#endif	/* PFC_LP64 */

/*
 * Determine whether low-part of general register must be used or not.
 * `size' must be the access size.
 */
#define	PFC_X86_NEED_LOWREG(size)	((size) <= 2)

#endif	/* _PFC_ASM */

#endif	/* !_PFC_ARCH_X86_PFC_ASM_BASE_H */
