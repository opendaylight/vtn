/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_ARCH_X86_PFC_ATOMIC_GCC_H
#define	_PFC_ARCH_X86_PFC_ATOMIC_GCC_H

/*
 * Atomic operation using gcc style inline assembly.
 * i386/x86_64 specific.
 */

#include <pfc/asm_base.h>

#ifndef	_PFC_ATOMIC_H
#error	Do NOT include atomic_gcc.h directly.
#endif	/* !_PFC_ATOMIC_H */

#ifndef	__GNUC__
#error	gcc is required.
#endif	/* !__GNUC__ */

/*
 * Atomic operations which take address, and return void.
 */
#define	__PFC_ATOMIC_OP(inst, suffix, addr)				\
	PFC_ASM_INLINE_V("lock; " inst PFC_X86_INST_##suffix "  %0"	\
			 : "+m" (*addr)					\
			 :						\
			 : "cc")

/*
 * Atomic operations which take address and immediate, and return void.
 */
#define	__PFC_ATOMIC_OP2(inst, suffix, addr, value)			\
	do {								\
		if (PFC_X86_NEED_LOWREG(sizeof(value))) {		\
			/* Low-part of register must be used. */	\
			PFC_ASM_INLINE_V("lock; "			\
					 inst PFC_X86_INST_##suffix	\
					 "  %1, %0"			\
					 : "+m" (*addr)			\
					 : "iq" (value)			\
					 : "cc");			\
		}							\
		else {							\
			PFC_ASM_INLINE_V("lock; "			\
					 inst PFC_X86_INST_##suffix	\
					 "  %1, %0"			\
					 : "+m" (*addr)			\
					 : "ir" (value)			\
					 : "cc");			\
		}							\
	} while (0)

/*
 * Atomic "xadd".
 *
 * Remarks: xaddl doesn't work on CPU older than i486.
 */
#define	__PFC_ATOMIC_XADD(suffix, addr, value)				\
	do {								\
		if (PFC_X86_NEED_LOWREG(sizeof(value))) {		\
			/* Low-part of register must be used. */	\
			PFC_ASM_INLINE_V("lock; xadd" PFC_X86_INST_##suffix \
					 "  %0, %1"			\
					 : "+q" (value), "+m" (*addr)	\
					 :				\
					 : "cc", "memory");		\
		}							\
		else {							\
			PFC_ASM_INLINE_V("lock; xadd" PFC_X86_INST_##suffix \
					 "  %0, %1"			\
					 : "+r" (value), "+m" (*addr)	\
					 :				\
					 : "cc", "memory");		\
		}							\
	} while (0)

/*
 * Atomic "cmpxchg".
 *
 * Remarks: cmpxchgl doesn't work on CPU older than i486.
 */
#define	__PFC_ATOMIC_CMPXCHG(suffix, addr, newval, oldval, result)	\
	do {								\
		if (PFC_X86_NEED_LOWREG(sizeof(result))) {		\
			/* Low-part of register must be used. */	\
			PFC_ASM_INLINE_V("lock; cmpxchg"		\
					 PFC_X86_INST_##suffix "  %2, %1" \
					 : "=a" (result), "=m" (*addr)	\
					 : "q" (newval), "m" (*addr),	\
					   "0" (oldval)			\
					 : "cc", "memory");		\
		}							\
		else {							\
			PFC_ASM_INLINE_V("lock; cmpxchg"		\
					 PFC_X86_INST_##suffix "  %2, %1" \
					 : "=a" (result), "=m" (*addr)	\
					 : "r" (newval), "m" (*addr),	\
					   "0" (oldval)			\
					 : "cc", "memory");		\
		}							\
	} while (0)

/*
 * Atomic "xchg".
 *
 * Remarks: "xchg" never updates status flag.
 */
#define	__PFC_ATOMIC_XCHG(suffix, addr, value, result)			\
	do {								\
		if (PFC_X86_NEED_LOWREG(sizeof(result))) {		\
			/* Low-part of register must be used. */	\
			PFC_ASM_INLINE_V("lock;  xchg"			\
					 PFC_X86_INST_##suffix "  %2, %1" \
					 : "=q" (result), "=m" (*addr)	\
					 : "0" (value), "m" (*addr)	\
					 : "memory");			\
		}							\
		else {							\
			PFC_ASM_INLINE_V("lock;  xchg"			\
					 PFC_X86_INST_##suffix "  %2, %1" \
					 : "=r" (result), "=m" (*addr)	\
					 : "0" (value), "m" (*addr)	\
					 : "memory");			\
		}							\
	} while (0)

/*
 * Declare definition of pfc_atomic_add_XXX().
 */
#define	__PFC_ATOMIC_ADD_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_add_uint##bits(uint##bits##_t *addr,			\
				  uint##bits##_t value)			\
	{								\
		__PFC_ATOMIC_OP2("add", UINT##bits, addr, value);	\
	}

/*
 * Declare definition of pfc_atomic_inc_XXX().
 */
#define	__PFC_ATOMIC_INC_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_inc_uint##bits(uint##bits##_t *addr)			\
	{								\
		__PFC_ATOMIC_OP("inc", UINT##bits, addr);		\
	}

/*
 * Declare definition of pfc_atomic_add_XXX_old().
 */
#define	__PFC_ATOMIC_ADD_OLD_DECL(bits)				\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE	\
	pfc_atomic_add_uint##bits##_old(uint##bits##_t *addr,	\
					uint##bits##_t value)	\
	{							\
		__PFC_ATOMIC_XADD(UINT##bits, addr, value);	\
								\
		return value;					\
	}

/*
 * Declare definition of pfc_atomic_inc_XXX_old().
 */
#define	__PFC_ATOMIC_INC_OLD_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_inc_uint##bits##_old(uint##bits##_t *addr)		\
	{								\
		return pfc_atomic_add_uint##bits##_old(addr, 1);	\
	}

/*
 * Declare definition of pfc_atomic_sub_XXX().
 */
#define	__PFC_ATOMIC_SUB_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_sub_uint##bits(uint##bits##_t *addr,			\
				  uint##bits##_t value)			\
	{								\
		__PFC_ATOMIC_OP2("sub", UINT##bits, addr, value);	\
	}

/*
 * Declare definition of pfc_atomic_sub_XXX_old().
 */
#define	__PFC_ATOMIC_SUB_OLD_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_sub_uint##bits##_old(uint##bits##_t *addr,		\
					uint##bits##_t value)		\
	{								\
		int##bits##_t	v = (int##bits##_t)value;		\
		uint##bits##_t	old, res;				\
									\
		if (v >= 0) {						\
			return pfc_atomic_add_uint##bits##_old		\
				(addr, (uint##bits##_t)-v);		\
		}							\
									\
		do {							\
			uint##bits##_t	required;			\
									\
			old = *addr;					\
			required = old - value;				\
			res = pfc_atomic_cas_uint##bits(addr, required, \
							old);		\
		} while (res != old);					\
									\
		return old;						\
	}

/*
 * Declare definition of pfc_atomic_dec_XXX().
 */
#define	__PFC_ATOMIC_DEC_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_dec_uint##bits(uint##bits##_t *addr)			\
	{								\
		__PFC_ATOMIC_OP("dec", UINT##bits, addr);		\
	}

/*
 * Declare definition of pfc_atomic_dec_XXX_old().
 */
#define	__PFC_ATOMIC_DEC_OLD_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_dec_uint##bits##_old(uint##bits##_t *addr)		\
	{								\
		return pfc_atomic_sub_uint##bits##_old(addr, 1);	\
	}

/*
 * Declare definition of pfc_atomic_cas_XXX().
 */
#define	__PFC_ATOMIC_CAS_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_cas_uint##bits(uint##bits##_t *addr,			\
				  uint##bits##_t newval,		\
				  uint##bits##_t oldval)		\
	{								\
		uint##bits##_t	result;					\
									\
		__PFC_ATOMIC_CMPXCHG(UINT##bits, addr, newval, oldval,	\
				     result);				\
									\
		return result;						\
	}

/*
 * Declare definition of pfc_atomic_cas_acq_XXX().
 */
#define	__PFC_ATOMIC_CAS_ACQ_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_cas_acq_uint##bits(uint##bits##_t *addr,		\
				      uint##bits##_t newval,		\
				      uint##bits##_t oldval)		\
	{								\
		return pfc_atomic_cas_uint##bits(addr, newval, oldval);	\
	}

/*
 * Declare definition of pfc_atomic_swap_XXX().
 */
#define	__PFC_ATOMIC_SWAP_DECL(bits)					\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_swap_uint##bits(uint##bits##_t *addr,		\
				   uint##bits##_t value)		\
	{								\
		uint##bits##_t	result;					\
									\
		__PFC_ATOMIC_XCHG(UINT##bits, addr, value, result);	\
									\
		return result;						\
	}

/*
 * Declare definition of pfc_atomic_swap_rel_XXX().
 */
#define	__PFC_ATOMIC_SWAP_REL_DECL(bits)				\
	static inline uint##bits##_t PFC_FATTR_ALWAYS_INLINE		\
	pfc_atomic_swap_rel_uint##bits(uint##bits##_t *addr,		\
				       uint##bits##_t value)		\
	{								\
		return pfc_atomic_swap_uint##bits(addr, value);		\
	}

/*
 * Declare definition of pfc_atomic_and_XXX().
 */
#define	__PFC_ATOMIC_AND_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_and_uint##bits(uint##bits##_t *addr,			\
				  uint##bits##_t b)			\
	{								\
		__PFC_ATOMIC_OP2("and", UINT##bits, addr, b);		\
	}

/*
 * Declare definition of pfc_atomic_or_XXX().
 */
#define	__PFC_ATOMIC_OR_DECL(bits)					\
	static inline void PFC_FATTR_ALWAYS_INLINE			\
	pfc_atomic_or_uint##bits(uint##bits##_t *addr,			\
				 uint##bits##_t b)			\
	{								\
		__PFC_ATOMIC_OP2("or", UINT##bits, addr, b);		\
	}

#include <pfc/atomic_gcc_arch.h>

PFC_C_BEGIN_DECL

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint8(uint8_t *addr, uint8_t value)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint16(uint16_t *addr, uint16_t value)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint32(uint32_t *addr, uint32_t value)
 *	Add `value' to `*addr' atomically.
 */
__PFC_ATOMIC_ADD_DECL(8)
__PFC_ATOMIC_ADD_DECL(16)
__PFC_ATOMIC_ADD_DECL(32)

/*
 * static inline void PFC_FATTR_ALWAYS_INLIEN
 * pfc_atomic_inc_uint8(uint8_t *addr)
 *
 * static inline void PFC_FATTR_ALWAYS_INLIEN
 * pfc_atomic_inc_uint16(uint16_t *addr)
 *
 * static inline void PFC_FATTR_ALWAYS_INLIEN
 * pfc_atomic_inc_uint32(uint32_t *addr)
 *	Increment `*addr' atomically.
 */
__PFC_ATOMIC_INC_DECL(8)
__PFC_ATOMIC_INC_DECL(16)
__PFC_ATOMIC_INC_DECL(32)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint8_old(uint8_t *addr, uint8_t value)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint16_old(uint16_t *addr, uint16_t value)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_add_uint32_old(uint32_t *addr, uint32_t value)
 *	Add `value' to `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_ADD_OLD_DECL(8)
__PFC_ATOMIC_ADD_OLD_DECL(16)
__PFC_ATOMIC_ADD_OLD_DECL(32)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint8_old(uint8_t *addr)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint16_old(uint16_t *addr)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_inc_uint32_old(uint32_t *addr)
 *	Increment `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_INC_OLD_DECL(8)
__PFC_ATOMIC_INC_OLD_DECL(16)
__PFC_ATOMIC_INC_OLD_DECL(32)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint8(uint82_t *addr, uint82_t value)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint16(uint16_t *addr, uint16_t value)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint32(uint32_t *addr, uint32_t value)
 *	Subtract `value' from `*addr' atomically.
 */
__PFC_ATOMIC_SUB_DECL(8)
__PFC_ATOMIC_SUB_DECL(16)
__PFC_ATOMIC_SUB_DECL(32)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint8(uint8_t *addr)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint16(uint16_t *addr)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint32(uint32_t *addr)
 *	Decrement `*addr' atomically.
 */
__PFC_ATOMIC_DEC_DECL(8)
__PFC_ATOMIC_DEC_DECL(16)
__PFC_ATOMIC_DEC_DECL(32)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_and_uint8(uint8_t *addr, uint8_t bits)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_and_uint16(uint16_t *addr, uint16_t bits)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_and_uint32(uint32_t *addr, uint32_t bits)
 *	Perform bitwise AND operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
__PFC_ATOMIC_AND_DECL(8)
__PFC_ATOMIC_AND_DECL(16)
__PFC_ATOMIC_AND_DECL(32)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_or_uint8(uint8_t *addr, uint8_t bits)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_or_uint16(uint16_t *addr, uint16_t bits)
 *
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_or_uint32(uint32_t *addr, uint32_t bits)
 *	Perform bitwise OR operation of `bits' to the value stored in
 *	`*addr' atomically.
 */
__PFC_ATOMIC_OR_DECL(8)
__PFC_ATOMIC_OR_DECL(16)
__PFC_ATOMIC_OR_DECL(32)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_uint8(uint8_t *addr, uint8_t newval, uint8_t oldval)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_uint16(uint16_t *addr, uint16_t newval, uint16_t oldval)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_uint32(uint32_t *addr, uint32_t newval, uint32_t oldval)
 *	Compare and swap value atomically.
 *	Install `newval' into `*addr' if `*addr' equals `oldval'.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_CAS_DECL(8)
__PFC_ATOMIC_CAS_DECL(16)
__PFC_ATOMIC_CAS_DECL(32)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_acq_uint8(uint8_t *addr, uint8_t newval, uint8_t oldval)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_acq_uint16(uint16_t *addr, uint16_t newval, uint16_t oldval)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_acq_uint32(uint32_t *addr, uint32_t newval, uint32_t oldval)
 *
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_cas_acq_uint64(uint64_t *addr, uint64_t newval, uint64_t oldval)
 *	Same as pfc_atomic_cas_XXX(), but acquires memory fence to serialize
 *	order of memory reference.
 *
 *	On x86, no special operation is needed because "lock" implies
 *	read/write memory barrier.
 */
__PFC_ATOMIC_CAS_ACQ_DECL(8)
__PFC_ATOMIC_CAS_ACQ_DECL(16)
__PFC_ATOMIC_CAS_ACQ_DECL(32)
__PFC_ATOMIC_CAS_ACQ_DECL(64)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint8_old(uint8_t *addr, uint8_t value)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint16_old(uint16_t *addr, uint16_t value)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint32_old(uint32_t *addr, uint32_t value)
 *
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_sub_uint64_old(uint64_t *addr, uint64_t value)
 *	Subtract `value' from `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_SUB_OLD_DECL(8)
__PFC_ATOMIC_SUB_OLD_DECL(16)
__PFC_ATOMIC_SUB_OLD_DECL(32)
__PFC_ATOMIC_SUB_OLD_DECL(64)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint8_old(uint8_t *addr)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint16_old(uint16_t *addr)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint32_old(uint32_t *addr)
 *
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_dec_uint64_old(uint64_t *addr)
 *	Decrement `*addr' atomically.
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_DEC_OLD_DECL(8)
__PFC_ATOMIC_DEC_OLD_DECL(16)
__PFC_ATOMIC_DEC_OLD_DECL(32)
__PFC_ATOMIC_DEC_OLD_DECL(64)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_uint8(uint8_t *addr, uint8_t value)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_uint16(uint16_t *addr, uint16_t value)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_uint32(uint32_t *addr, uint32_t value)
 *
 * Calling/Exit State:
 *	Value previously set in `*addr' is returned.
 */
__PFC_ATOMIC_SWAP_DECL(8)
__PFC_ATOMIC_SWAP_DECL(16)
__PFC_ATOMIC_SWAP_DECL(32)

/*
 * static inline uint8_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_rel_uint8(uint8_t *addr, uint8_t value)
 *
 * static inline uint16_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_rel_uint16(uint16_t *addr, uint16_t value)
 *
 * static inline uint32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_rel_uint32(uint32_t *addr, uint32_t value)
 *
 * static inline uint64_t PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_swap_rel_uint64(uint64_t *addr, uint64_t value)
 *	Same as pfc_atomic_swap_XXX(), but releases memory fence
 *	previously acquired.
 *
 *	On x86, no special operation is needed because "lock" implies
 *	read/write memory barrier.
 */
__PFC_ATOMIC_SWAP_REL_DECL(8)
__PFC_ATOMIC_SWAP_REL_DECL(16)
__PFC_ATOMIC_SWAP_REL_DECL(32)
__PFC_ATOMIC_SWAP_REL_DECL(64)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_read_barrier(void)
 *	Issue read memory barrier.
 *
 *	This function serializes all load-from-memory instructions in program
 *	order.
 *	The call of pfc_atomic_read_barrier() never returns unless all prior
 *	load instructions have completed. And no later load instructions
 *	begins until the call of this function completes.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_read_barrier(void)
{
	/* Remarks: lfence doesn't work on CPU older than Pentium 4. */
	PFC_ASM_INLINE_V("lfence" ::: "memory");
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_write_barrier(void)
 *	Issue write memory barrier.
 *
 *	This function serializes all store-to-memory instructions in program
 *	order.
 *	The call of pfc_atomic_write_barrier() never returns unless all prior
 *	store instructions have completed. And no later store instructions
 *	begins until the call of this function completes.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_write_barrier(void)
{
	/* Remarks: sfence doesn't work on CPU older than Pentium III. */
	PFC_ASM_INLINE_V("sfence" ::: "memory");
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_atomic_memory_barrier(void)
 *	Issue memory barrier.
 *
 *	This function serializes all memory access instructions in program
 *	order.
 *	The call of pfc_atomic_memory_barrier() never returns unless all prior
 *	memory access instructions have completed. And no later memory access
 *	instructions begins until the call of this function completes.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_atomic_memory_barrier(void)
{
	/* Remarks: mfence doesn't work on CPU older than Pentium 4. */
	PFC_ASM_INLINE_V("mfence" ::: "memory");
}

PFC_C_END_DECL

#endif	/* !_PFC_ARCH_X86_PFC_ATOMIC_GCC_H */
