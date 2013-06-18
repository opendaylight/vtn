/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_X86_PFC_BITPOS_GCC_H
#define	_PFC_ARCH_X86_PFC_BITPOS_GCC_H

/*
 * Bit position utilities using gcc style inline assembly.
 * i386/x86_64 specific.
 */

#ifndef	_PFC_BITPOS_H
#error	Do NOT include bitpos_gcc.h directly.
#endif	/* !_PFC_BITPOS_H */

#ifndef	__GNUC__
#error	gcc is required.
#endif	/* !__GNUC__ */

PFC_C_BEGIN_DECL

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_highbit_uint32(uint32_t value)
 *	Find highest bit set in the given 32-bit value.
 *
 * Calling/Exit State:
 *	Bit number of highest bit set is returned if the given value is
 *	not zero. The high order bit is 31.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_highbit_uint32(uint32_t value)
{
	int	position;

	PFC_ASM_INLINE_V("bsrl	%1, %0;\n"
			 "jnz	1f;\n"
			 "movl	%2, %0;\n"
			 "1:"
			 : "=r" (position)
			 :  "rm" (value), "i" (-1)
			 : "cc");

	return position;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_lowbit_uint32(uint32_t value)
 *	Find lowest bit set in the given 32-bit value.
 *
 * Calling/Exit State:
 *	Bit number of lowest bit set is returned if the given value is
 *	not zero. The low order bit is 0.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_lowbit_uint32(uint32_t value)
{
	int	position;

	PFC_ASM_INLINE_V("bsfl	%1, %0;\n"
			 "jnz	1f;\n"
			 "movl	%2, %0;\n"
			 "1:"
			 : "=r" (position)
			 : "rm" (value), "i" (-1)
			 : "cc");

	return position;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_highbit_uint64(uint64_t value)
 *	Find highest bit set in the given 64-bit value.
 *
 * Calling/Exit State:
 *	Bit number of highest bit set is returned if the given value is
 *	not zero. The high order bit is 63.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_highbit_uint64(uint64_t value)
{
#ifdef	PFC_LP64
	int64_t	position;

	PFC_ASM_INLINE_V("bsrq	%1, %0;\n"
			 "jnz	1f;\n"
			 "movq	%2, %0;\n"
			 "1:"
			 : "=r" (position)
			 :  "rm" (value), "i" (-1)
			 : "cc");

	return (int)position;
#else	/* !PFC_LP64 */
	int	position;
	uint32_t	u32;

	u32 = (uint32_t)(value >> 32);
	if ((position = pfc_highbit_uint32(u32)) >= 0) {
		return position + 32;
	}

	u32 = (uint32_t)value;

	return pfc_highbit_uint32(u32);
#endif	/* PFC_LP64 */
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_lowbit_uint64(uint64_t value)
 *	Find lowest bit set in the given 64-bit value.
 *
 * Calling/Exit State:
 *	Bit number of lowest bit set is returned if the given value is
 *	not zero. The low order bit is 0.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_lowbit_uint64(uint64_t value)
{
#ifdef	PFC_LP64
	int64_t	position;

	PFC_ASM_INLINE_V("bsfq	%1, %0;\n"
			 "jnz	1f;\n"
			 "movq	%2, %0;\n"
			 "1:"
			 : "=r" (position)
			 :  "rm" (value), "i" (-1)
			 : "cc");

	return (int)position;
#else	/* !PFC_LP64 */
	int	position;
	uint32_t	u32;

	u32 = (uint32_t)value;
	if ((position = pfc_lowbit_uint32(u32)) >= 0) {
		return position;
	}

	u32 = (uint32_t)(value >> 32);
	if ((position = pfc_lowbit_uint32(u32)) >= 0) {
		return position + 32;
	}

	return -1;
#endif	/* PFC_LP64 */
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_highbit_ulong(pfc_ulong_t value)
 *	Find highest bit set in the given unsigned long value.
 *
 * Calling/Exit State:
 *	Bit number of highest bit set is returned if the given value is
 *	not zero. The high order bit is 31, or 63 on LP64 system.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_highbit_ulong(pfc_ulong_t value)
{
#ifdef	PFC_LP64
	return pfc_highbit_uint64((uint64_t)value);
#else	/* !PFC_LP64 */
	return pfc_highbit_uint32((uint32_t)value);
#endif	/* PFC_LP64 */
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_lowbit_ulong(pfc_ulong_t value)
 *	Find lowest bit set in the given unsigned long value.
 *
 * Calling/Exit State:
 *	Bit number of lowest bit set is returned if the given value is
 *	not zero. The low order bit is 0.
 *
 *	Negative value is returned if no bit is set.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_lowbit_ulong(pfc_ulong_t value)
{
#ifdef	PFC_LP64
	return pfc_lowbit_uint64((uint64_t)value);
#else	/* !PFC_LP64 */
	return pfc_lowbit_uint32((uint32_t)value);
#endif	/* PFC_LP64 */
}

PFC_C_END_DECL

#endif	/* !_PFC_ARCH_X86_PFC_BITPOS_GCC_H */
