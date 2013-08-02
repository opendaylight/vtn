/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_BASE_H
#define	_PFC_BASE_H

/*
 * Basic definition for PFC.
 * This header assumes to be included from all PFC sources.
 */

#if	defined(__cplusplus) && !defined(_PFC_IN_CFDEFC)
#define	PFC_C_BEGIN_DECL	extern "C" {
#define	PFC_C_END_DECL		}
#else	/* !(defined(__cplusplus) && !defined(_PFC_IN_CFDEFC)) */
#define	PFC_C_BEGIN_DECL
#define	PFC_C_END_DECL
#endif	/* defined(__cplusplus) && !defined(_PFC_IN_CFDEFC) */

#ifdef	_PFC_IN_CFDEFC

/* Do not append suffix for cfdef file. */
#define	PFC_LITERAL_DECL(value, suffix)		value

#else	/* !_PFC_IN_CFDEFC */

#define	PFC_LITERAL_DECL(value, suffix)		(value ## suffix)

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>

/* Import build configuration. */
#include <pfc/config.h>

#ifdef	__GNUC__
#include <pfc/compiler/gcc.h>
#else	/* !__GNUC__ */
#error	Unsupported compiler.
#endif	/* __GNUC__ */

/* Import OS-specific configuration. */
#include <pfc/osdep.h>

/* Import byte order utilities. */
#include <pfc/byteorder.h>

#ifdef	__cplusplus
/* Import common definitions for C++. */
#include <pfcxx/base.hh>
#endif	/* __cplusplus */

PFC_C_BEGIN_DECL

/*
 * Boolean type.
 */
typedef uint8_t		pfc_bool_t;

#define	PFC_TRUE	((pfc_bool_t)1)
#define	PFC_FALSE	((pfc_bool_t)0)

/*
 * Long integer. Its size may be changed by ILP model.
 */
typedef long int		pfc_long_t;
typedef unsigned long int	pfc_ulong_t;

#define	PFC_LONG_MAX		LONG_MAX
#define	PFC_LONG_MIN		LONG_MIN

#define	PFC_ULONG_MAX		ULONG_MAX

/*
 * Abstract pointer type.
 * pfc_ptr_t points to modifiable storage, and pfc_cptr_t points to
 * read-only storage.
 */
typedef void		*pfc_ptr_t;
typedef const void	*pfc_cptr_t;

/*
 * Pointer difference.
 */
#ifdef	PFC_LP64
typedef long		pfc_ptrdiff_t;
#else	/* !PFC_LP64 */
typedef int		pfc_ptrdiff_t;
#endif	/* PFC_LP64 */

/*
 * Define valid integer range if not defined.
 */
#ifndef	INT8_MAX
#define	INT8_MAX		0x7f
#endif	/* !INT8_MAX */

#ifndef	INT8_MIN
#define	INT8_MIN		(-INT8_MAX - 1)
#endif	/* !INT8_MIN */

#ifndef	INT16_MAX
#define	INT16_MAX		0x7fff
#endif	/* !INT16_MAX */

#ifndef	INT16_MIN
#define	INT16_MIN		(-INT16_MAX - 1)
#endif	/* !INT16_MIN */

#ifndef	INT32_MAX
#define	INT32_MAX		0x7fffffff
#endif	/* !INT32_MAX */

#ifndef	INT32_MIN
#define	INT32_MIN		(-INT32_MAX - 1)
#endif	/* !INT32_MIN */

#ifndef	INT64_MAX
#define	INT64_MAX		PFC_CONST_LL(0x7fffffffffffffff)
#endif	/* !INT64_MAX */

#ifndef	INT64_MIN
#define	INT64_MIN		(-INT64_MAX - 1)
#endif	/* !INT64_MIN */

#ifndef	UINT8_MAX
#define	UINT8_MAX		PFC_CONST_U(0xff)
#endif	/* !UINT8_MAX */

#ifndef	UINT16_MAX
#define	UINT16_MAX		PFC_CONST_U(0xffff)
#endif	/* !UINT16_MAX */

#ifndef	UINT32_MAX
#define	UINT32_MAX		PFC_CONST_U(0xffffffff)
#endif	/* !UINT32_MAX */

#ifndef	UINT64_MAX
#define	UINT64_MAX		PFC_CONST_ULL(0xffffffffffffffff)
#endif	/* !UINT64_MAX */

/*
 * Number of bits in a byte.
 */
#define	PFC_NBITS_PER_BYTE		PFC_CONST_U(8)

/*
 * Return the number of bits in the given number of bytes.
 */
#define	PFC_NBITS(bytes)		((bytes) << PFC_CONST_U(3))

/*
 * Convert the number of bits into bytes.
 */
#define	PFC_BIT2BYTE(bits)		((bits) >> PFC_CONST_U(3))

/*
 * Returns number of elements in the given array.
 */
#define	PFC_ARRAY_CAPACITY(array)	(sizeof(array) / sizeof(array[0]))

/*
 * Returns end boundary address of the given array.
 */
#define	PFC_ARRAY_LIMIT(array)		((array) + PFC_ARRAY_CAPACITY(array))

/*
 * Convert pointer to a struct member into pointer to a struct which contains
 * the given member.
 */
#define	PFC_CAST_CONTAINER(ptr, type, member)			\
	((type *)((uintptr_t)(ptr) - offsetof(type, member)))

/*
 * Determine whether the given value is a power of 2.
 */
#define	PFC_IS_POW2(value)	(((value) & ((value) - 1)) == 0)

/*
 * Determine whether the given address is aligned to the given value.
 * Alignment must be a power of 2.
 */
#define	PFC_IS_POW2_ALIGNED(addr, align)			\
	(((uintptr_t)(addr) & ((uintptr_t)(align) - 1)) == 0)

/*
 * Adjusting alignment and rounding macros.
 * Alignment must be a power of 2.
 */
#define	PFC_POW2_ALIGN(v, align)	((v) & ~((align) - 1))
#define	PFC_POW2_ROUNDUP(v, align)	(((v) + ((align) - 1)) & ~((align) - 1))

/*
 * Common counting and rounding macros.
 */
#define	PFC_HOWMANY(v, align)		(((v) + ((align) - 1)) / (align))
#define	PFC_ROUNDUP(v, align)						\
	((PFC_IS_CONSTANT(align) && PFC_IS_POW2(align))			\
	 ? PFC_POW2_ROUNDUP(v, align) : (PFC_HOWMANY(v, align) * (align)))

/*
 * Determine whether the given address can be accessed directly using
 * the access size specified by `size'. `size' must be a power of 2.
 */
#ifdef	PFC_UNALIGNED_ACCESS
#define	PFC_CAN_ACCESS(addr, size)	(PFC_TRUE)
#else	/* !PFC_UNALIGNED_ACCESS */
#define	PFC_CAN_ACCESS(addr, size)					\
	PFC_IS_POW2_ALIGNED((addr),					\
			    ((size) == sizeof(int64_t))			\
			    ? PFC_ALIGNOF_INT64 : (size))
#endif	/* PFC_UNALIGNED_ACCESS */

/*
 * Stringify argument without macro expansion.
 */
#define	PFC_STRINGIFY(arg)		#arg

/*
 * Stringify the result of macro expansion of argument.
 */
#define	PFC_XSTRINGIFY(arg)		PFC_STRINGIFY(arg)

/*
 * String literals for printf(3) format.
 */
#ifdef	PFC_LP64

/* 64-bit integer is defined as long int. */
#define	PFC_PFMT_o64		"lo"
#define	PFC_PFMT_d64		"ld"
#define	PFC_PFMT_u64		"lu"
#define	PFC_PFMT_x64		"lx"
#define	PFC_PFMT_X64		"lX"

/* size_t and ssize_t are defined as long int. */
#define	PFC_PFMT_SIZE_T		"lu"
#define	PFC_PFMT_xSIZE_T	"lx"
#define	PFC_PFMT_XSIZE_T	"lX"
#define	PFC_PFMT_SSIZE_T	"ld"
#define	PFC_PFMT_xSSIZE_T	"lx"
#define	PFC_PFMT_XSSIZE_T	"lX"

/* intptr_t and uintptr_t are defined as long int. */
#define	PFC_PFMT_INTPTR_T	"ld"
#define	PFC_PFMT_xINTPTR_T	"lx"
#define	PFC_PFMT_XINTPTR_T	"lX"
#define	PFC_PFMT_UINTPTR_T	"lu"
#define	PFC_PFMT_xUINTPTR_T	"lx"
#define	PFC_PFMT_XUINTPTR_T	"lX"

/* Pointer difference must be signed long integer. */
#define	PFC_PFMT_PTRDIFF_T	"ld"
#define	PFC_PFMT_uPTRDIFF_T	"lu"
#define	PFC_PFMT_xPTRDIFF_T	"lx"
#define	PFC_PFMT_XPTRDIFF_T	"lX"

#else	/* !PFC_LP64 */

/* 64-bit integer is defined as long long int. */
#define	PFC_PFMT_o64		"llo"
#define	PFC_PFMT_d64		"lld"
#define	PFC_PFMT_u64		"llu"
#define	PFC_PFMT_x64		"llx"
#define	PFC_PFMT_X64		"llX"

/* size_t and ssize_t are defined as int. */
#define	PFC_PFMT_SIZE_T		"u"
#define	PFC_PFMT_xSIZE_T	"x"
#define	PFC_PFMT_XSIZE_T	"X"
#define	PFC_PFMT_SSIZE_T	"d"
#define	PFC_PFMT_xSSIZE_T	"x"
#define	PFC_PFMT_XSSIZE_T	"X"

/* intptr_t and uintptr_t are defined as int. */
#define	PFC_PFMT_INTPTR_T	"d"
#define	PFC_PFMT_xINTPTR_T	"x"
#define	PFC_PFMT_XINTPTR_T	"X"
#define	PFC_PFMT_UINTPTR_T	"u"
#define	PFC_PFMT_xUINTPTR_T	"x"
#define	PFC_PFMT_XUINTPTR_T	"X"

/* Pointer difference must be signed integer. */
#define	PFC_PFMT_PTRDIFF_T	"d"
#define	PFC_PFMT_uPTRDIFF_T	"u"
#define	PFC_PFMT_xPTRDIFF_T	"x"
#define	PFC_PFMT_XPTRDIFF_T	"X"

#endif	/* PFC_LP64 */

/*
 * Determine whether the system is LP64 system or not.
 */
#ifdef	PFC_LP64
#define	PFC_IS_LP64_SYSTEM()		(PFC_TRUE)
#else	/* !PFC_LP64 */
#define	PFC_IS_LP64_SYSTEM()		(PFC_FALSE)
#endif	/* PFC_LP64 */

/*
 * True if the specified error number is EWOULDBLOCK.
 */
#if	EAGAIN == EWOULDBLOCK
#define	PFC_IS_EWOULDBLOCK(err)		((err) == EAGAIN)
#else	/* EAGAIN != EWOULDBLOCK */
#define	PFC_IS_EWOULDBLOCK(err)				\
	((err) == EAGAIN || (err) == EWOULDBLOCK)
#endif	/* EAGAIN == EWOULDBLOCK */

/*
 * PFC_TYPE_SIZE_ASSERT(type, size)
 *	Define assertion that verifies the size of data type.
 *
 *	`type' must be the name of data type to be tested.
 *	`size' is the required size of the data type.
 *
 *	Test failure causes the build error.
 */
#define	PFC_TYPE_SIZE_ASSERT(type, size)				\
	typedef int	__pfc_size_assert_##type[((sizeof(type) == (size)) \
						  ? 1 : -1)]

PFC_C_END_DECL

#endif	/* _PFC_IN_CFDEFC */

/*
 * Declare integer constant.
 */
#define	PFC_CONST_U(value)		PFC_LITERAL_DECL(value, U)
#define	PFC_CONST_L(value)		PFC_LITERAL_DECL(value, L)
#define	PFC_CONST_UL(value)		PFC_LITERAL_DECL(value, UL)
#define	PFC_CONST_LL(value)		PFC_LITERAL_DECL(value, LL)
#define	PFC_CONST_ULL(value)		PFC_LITERAL_DECL(value, ULL)

#endif	/* !_PFC_BASE_H */
