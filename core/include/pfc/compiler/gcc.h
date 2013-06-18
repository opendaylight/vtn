/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_COMPILER_GCC_H
#define	_PFC_COMPILER_GCC_H

/*
 * GCC specific definitions.
 */

#ifndef	__GNUC__
#error	gcc is required.
#endif	/* !__GNUC__ */

#if	__GNUC__ < 4
#error	gcc version 4 or later is required.
#endif	/* __GNUC__ < 4 */

#undef	PFCXX_HAVE_FORCE_UNWIND

#if	__GNUC__ > 4 || (defined(__GNUC_MINOR__) && __GNUC_MINOR__ >= 3)

#ifdef	__cplusplus
/* We have __force_unwind class. */
#define	PFCXX_HAVE_FORCE_UNWIND		1
#endif	/* __cplusplus */

#endif	/* __GNUC__ > 4 || (defined(__GNUC_MINOR__) && __GNUC_MINOR__ >= 3) */

/*
 * Function attributes.
 */
#define	PFC_FATTR_INIT			__attribute__((constructor))
#define	PFC_FATTR_FINI			__attribute__((destructor))
#define	PFC_FATTR_INIT_PRI(pri)		__attribute__((constructor(pri)))
#define	PFC_FATTR_FINI_PRI(pri)		__attribute__((destructor(pri)))
#define	PFC_FATTR_NOINLINE		__attribute__((noinline))
#define	PFC_FATTR_ALWAYS_INLINE		__attribute__((always_inline))
#define	PFC_FATTR_NONNULL(arglist)	__attribute__((nonnull arglist))
#define	PFC_FATTR_NORETURN		__attribute__((noreturn))
#define	PFC_FATTR_PRINTFLIKE(fmtidx, argidx)			\
	__attribute__((format(printf, (fmtidx), (argidx))))

/*
 * Function/Object attributes
 */
#define	PFC_ATTR_HIDDEN			__attribute__((visibility("hidden")))
#define	PFC_ATTR_WEAK			__attribute__((weak))
#define	PFC_ATTR_WEAK_ALIAS(target)	__attribute__((weak, alias(#target)))
#define	PFC_ATTR_PACKED			__attribute__((packed))
#define	PFC_ATTR_UNUSED			__attribute__((unused))
#define	PFC_ATTR_ALIGNED(bytes)		__attribute__((aligned(bytes)))
#define	PFC_ATTR_CLEANUP(handler)	__attribute__((cleanup(handler)))
#define	PFC_ATTR_MAY_ALIAS		__attribute__((may_alias))

/*
 * Branch prediction.
 */
#define	PFC_EXPECT_TRUE(ex)	__builtin_expect((ex) ? 1: 0, 1)
#define	PFC_EXPECT_FALSE(ex)	__builtin_expect((ex) ? 1: 0, 0)

/*
 * Declare inline assembly.
 */
#define	PFC_ASM_INLINE		__asm__
#define	PFC_ASM_INLINE_V	PFC_ASM_INLINE __volatile__

/*
 * C99 keywords.
 */
#define	PFC_RESTRICT		__restrict

/*
 * Return true if the argument is a constant.
 */
#define	PFC_IS_CONSTANT(x)	__builtin_constant_p(x)

/*
 * Determine whether long int and pointer is 64-bit value or not.
 */
#ifdef	__LP64__
#define	PFC_LP64		1
#else	/* !__LP64__ */
#undef	PFC_LP64
#endif	/* __LP64__ */

/*
 * Tell the compiler to drop any cached memory.
 */
#define	PFC_MEMORY_RELOAD()	PFC_ASM_INLINE_V("" : : : "memory")

/*
 * Return size of the object pointed by the given pointer.
 * Note that this may return -1 if object size can't be determined at
 * compile time.
 */
#define	PFC_PTR_OBJSIZE(ptr)	__builtin_object_size((ptr), 1)

/*
 * Variable name which contains function name.
 */
#define	PFC_FUNCNAME		__func__

#endif	/* !_PFC_COMPILER_GCC_H */
