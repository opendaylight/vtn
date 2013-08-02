/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_X86_64_PFC_ATOMIC_GCC_ARCH_H
#define	_PFC_ARCH_X86_64_PFC_ATOMIC_GCC_ARCH_H

/*
 * x86_64 specific atomic operation using gcc style inline assembly.
 */

#ifndef	_PFC_ATOMIC_H
#error	Do NOT include atomic_gcc_arch.h directly.
#endif	/* !_PFC_ATOMIC_H */

#ifndef	__GNUC__
#error	gcc is required.
#endif	/* !__GNUC__ */

PFC_C_BEGIN_DECL

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint64(uint64_t *addr, uint64_t value)
 *	Add `value' to `*addr' atomically.
 */
__PFC_ATOMIC_ADD_DECL(64)

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint64(uint64_t *addr)
 *	Increment `*addr' atomically.
 */
__PFC_ATOMIC_INC_DECL(64)

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint64_old(uint64_t *addr, uint64_t value)
 *	Add `value' to `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_ADD_OLD_DECL(64)

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint64_old(uint64_t *addr)
 *	Increment `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_INC_OLD_DECL(64)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint64(uint64_t *addr, uint64_t value)
 *	Subtract `value' from `*addr' atomically.
 */
__PFC_ATOMIC_SUB_DECL(64)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint64(uint64_t *addr)
 *	Decrement `*addr' atomically.
 */
__PFC_ATOMIC_DEC_DECL(64)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_and_uint64(uint64_t *addr, uint64_t bits)
 *	Perform bitwise AND operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
__PFC_ATOMIC_AND_DECL(64)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_or_uint64(uint64_t *addr, uint64_t bits)
 *	Perform bitwise OR operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
__PFC_ATOMIC_OR_DECL(64)

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_uint64(uint64_t *addr, uint64_t newval, uint64_t oldval)
 *	Compare and swap uint64_t value atomically.
 *	Install `newval' into `*addr' if `*addr' equals `oldval'.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_CAS_DECL(64)

/*
 * static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_ptr(pfc_ptr_t *addr, pfc_ptr_t newval, pfc_ptr_t oldval)
 *	Compare and swap pointer value atomically.
 *	Install `newval' into `*addr' if `*addr' equals `oldval'.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
pfc_atomic_cas_ptr(pfc_ptr_t *addr, pfc_ptr_t newval, pfc_ptr_t oldval)
{
	uint64_t	ret;

	ret = pfc_atomic_cas_uint64((uint64_t *)addr, (uint64_t)newval,
				    (uint64_t)oldval);

	return (pfc_ptr_t)ret;
}

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_uint64(uint64_t *addr, uint64_t value)
 *	Exchange value in `*addr' with `value' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_SWAP_DECL(64)

/*
 * static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_ptr(pfc_ptr_t *addr, pfc_ptr_t value)
 *	Exchange pointer value in `*addr' with `value' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
static inline pfc_ptr_t PFC_FATTR_ALWAYS_INLINE
pfc_atomic_swap_ptr(pfc_ptr_t *addr, pfc_ptr_t value)
{
	uint64_t	ret;

	ret = pfc_atomic_swap_uint64((uint64_t *)addr, (uint64_t)value);

	return (pfc_ptr_t)ret;
}

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_read_uint64(uint64_t *addr)
 *	Read value in `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value in `*addr' is returned.
 */
static inline uint64_t PFC_FATTR_ALWAYS_INLINE
pfc_atomic_read_uint64(uint64_t *addr)
{
	return *((uint64_t volatile *)addr);
}

PFC_C_END_DECL

#endif	/* !_PFC_ARCH_X86_64_PFC_ATOMIC_GCC_ARCH_H */
