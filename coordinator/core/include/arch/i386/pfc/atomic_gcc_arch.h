/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_I386_PFC_ATOMIC_GCC_ARCH_H
#define	_PFC_ARCH_I386_PFC_ATOMIC_GCC_ARCH_H

/*
 * i386 specific atomic operation using gcc style inline assembly.
 */

#ifndef	_PFC_ATOMIC_H
#error	Do NOT include atomic_gcc_arch.h directly.
#endif	/* !_PFC_ATOMIC_H */

#ifndef	__GNUC__
#error	gcc is required.
#endif	/* !__GNUC__ */

PFC_C_BEGIN_DECL

/*
 * Forward declaration for functions defined in atomic_gcc.h.
 */
static inline uint32_t	pfc_atomic_cas_uint32(uint32_t *addr, uint32_t newval,
					      uint32_t oldval);
static inline uint32_t	pfc_atomic_swap_uint32(uint32_t *addr, uint32_t value);
static inline uint64_t	pfc_atomic_sub_uint64_old(uint64_t *addr,
						  uint64_t value);

/*
 * Prototypes.
 */
extern uint64_t	pfc_atomic_cas_uint64(uint64_t *addr, uint64_t newval,
				      uint64_t oldval);
extern uint64_t	pfc_atomic_add_uint64_old(uint64_t *addr, uint64_t value);
extern uint64_t	pfc_atomic_inc_uint64_old(uint64_t *addr);
extern void	pfc_atomic_and_uint64(uint64_t *addr, uint64_t bits);
extern void	pfc_atomic_or_uint64(uint64_t *addr, uint64_t bits);
extern uint64_t	pfc_atomic_swap_uint64(uint64_t *addr, uint64_t value);

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
	uint32_t	ret;

	ret = pfc_atomic_cas_uint32((uint32_t *)addr, (uint32_t)newval,
				    (uint32_t)oldval);

	return (pfc_ptr_t)ret;
}

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
	uint32_t	ret;

	ret = pfc_atomic_swap_uint32((uint32_t *)addr, (uint32_t)value);

	return (pfc_ptr_t)ret;
}

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint64(uint64_t *addr, uint64_t value)
 *	Add `value' to `*addr' atomically.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_add_uint64(uint64_t *addr, uint64_t value)
{
	(void)pfc_atomic_add_uint64_old(addr, value);
}

/*
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint64(uint64_t *addr)
 *	Increment `*addr' atomically.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_inc_uint64(uint64_t *addr)
{
	(void)pfc_atomic_inc_uint64_old(addr);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint64(uint64_t *addr, uint64_t value)
 *	Subtract `value' from `*addr' atomically.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_sub_uint64(uint64_t *addr, uint64_t value)
{
	(void)pfc_atomic_sub_uint64_old(addr, value);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint64(uint64_t *addr)
 *	Decrement `*addr' atomically.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_dec_uint64(uint64_t *addr)
{
	pfc_atomic_sub_uint64(addr, 1);
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
	uint64_t	value;

	/*
	 * On i386 architecture, we need to use locked cmpxchg8b to read
	 * 64-bit value atomically.
	 * This function copies (%ecx, %ebx) to (%edx, %eax), and then issues
	 * cmpxchg8b. If (%edx, %eax) doesn't equal `*addr', its value is
	 * stored to (%edx, %eax) by cmpxchg8b. Although (%ecx, %ebx) is
	 * installed to `*addr' if equals, it's harmless because those values
	 * are the same.
	 *
	 * Note that this code can be used in PIC code because it only clobbers
	 * %eax and %edx and never breaks %ebx, which is used for GOT access.
	 *
	 * Remarks: cmpxchg8b doesn't work on CPU older than Pentium.
	 */
	PFC_ASM_INLINE_V("movl  %%ecx, %%edx\n"
			 "movl  %%ebx, %%eax\n"
			 "lock; cmpxchg8b  %1\n"
			 : "=&A" (value), "=m"(*addr)
			 : "m" (*addr)
			 : "cc", "memory");

	return value;
}

PFC_C_END_DECL

#endif	/* !_PFC_ARCH_I386_PFC_ATOMIC_GCC_ARCH_H */
