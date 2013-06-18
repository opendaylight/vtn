/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_DEBUG_H
#define	_PFC_DEBUG_H

/*
 * Definitions for debugging.
 */

#include <stdarg.h>
#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * PFC assertion macros.
 */

extern void	__pfc_assfail(const char *ex, const char *file, uint32_t line,
			      const char *func)
	PFC_FATTR_NORETURN PFC_FATTR_NONNULL((1, 2));
extern int	__pfc_assfail_int(const char *ex, int value, int required,
				  const char *file, uint32_t line,
				  const char *func)
	PFC_FATTR_NONNULL((1, 4));

static inline int PFC_FATTR_ALWAYS_INLINE PFC_FATTR_NONNULL((1, 4))
___pfc_assfail_int(const char *ex, int value, int required,
		   const char *file, uint32_t line, const char *func)
{
	if (PFC_EXPECT_TRUE(value == required)) {
		return value;
	}

	return __pfc_assfail_int(ex, value, required, file, line, func);
}

extern void	__pfc_assfail_printf(const char *file, uint32_t line,
				     const char *func, const char *fmt, ...)
	PFC_FATTR_NORETURN PFC_FATTR_NONNULL((1, 4))
	PFC_FATTR_PRINTFLIKE(4, 5);
extern void	__pfc_assfail_vprintf(const char *file, uint32_t line,
				      const char *func, const char *fmt,
				      va_list ap)
	PFC_FATTR_NORETURN PFC_FATTR_NONNULL((1, 4))
	PFC_FATTR_PRINTFLIKE(4, 0);

#ifdef	PFC_FUNCNAME
#define	PFC_ASSERT_FUNCNAME		PFC_FUNCNAME
#else	/* !PFC_FUNCNAME */
#define	PFC_ASSERT_FUNCNAME		NULL
#endif	/* PFC_FUNCNAME */

#define	__PFC_ASSERT(ex)						\
	(PFC_EXPECT_TRUE(ex)						\
	 ? ((void)0)							\
	 : __pfc_assfail(#ex, __FILE__, __LINE__, PFC_ASSERT_FUNCNAME))

#ifdef	PFC_VERBOSE_DEBUG

#ifndef	PFC_DEBUG
#define	PFC_DEBUG
#endif	/* !PFC_DEBUG */

#define	PFC_ASSERT(ex)		__PFC_ASSERT(ex)

#endif	/* PFC_VERBOSE_DEBUG */

#ifdef	PFC_DEBUG
#define	PFC_VERIFY(ex)		__PFC_ASSERT(ex)
#endif	/* PFC_DEBUG */

#ifndef	PFC_VERIFY
#define	PFC_VERIFY(ex)		((void)0)
#endif	/* !PFC_VERIFY */

#ifndef	PFC_ASSERT
#define	PFC_ASSERT(ex)		((void)0)
#endif	/* !PFC_ASSERT */

/*
 * PFC_ASSERT_INT(ex, required)
 *	Ensure that the expression returns the required value.
 *	This macro assumes that the specified expression returns int value.
 *
 *	If PFC_VERBOSE_DEBUG is defined, the program will abort unless the
 *	expression returns the required value. If not defined, this macro
 *	simply executes the specified expression.
 *
 *	This macro is evaluated as the value returned by the expression.
 */
#define	__PFC_ASSERT_INT(ex, required)					\
	(___pfc_assfail_int(#ex, (ex), (required),  __FILE__, __LINE__,	\
			    PFC_ASSERT_FUNCNAME))
#ifdef	PFC_VERBOSE_DEBUG
#define	PFC_ASSERT_INT(ex, required)	__PFC_ASSERT_INT(ex, required)
#else	/* !PFC_VERBOSE_DEBUG */
#define	PFC_ASSERT_INT(ex, required)	(ex)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * PFC_VERIFY_INT(ex, required)
 *	Same as PFC_ASSERT_INT(), but the assertion is not invalidated
 *	as long as PFC_DEBUG is defined.
 */
#ifdef	PFC_DEBUG
#define	PFC_VERIFY_INT(ex, required)	__PFC_ASSERT_INT(ex, required)
#else	/* !PFC_DEBUG */
#define	PFC_VERIFY_INT(ex, required)	(ex)
#endif	/* PFC_DEBUG */

/*
 * PFC_ASSERT_PRINTF(ex, fmt, ...)
 *	Ensure that the expression specified by `ex' is true.
 *
 *	If PFC_VERBOSE_DEBUG is defined, the program will abort unless `ex'
 *	is true. The printf(3) style message, specified by `fmt' and the rest
 *	of arguments, is printed out to the standard error output.
 *
 *	This macro does nothing unless PFC_VERBOSE_DEBUG is defined.
 *
 * PFC_VERIFY_PRINTF(ex, fmt, ...)
 *	Same as PFC_ASSERT_PRINTF(), but the assertion is not invalidated
 *	as long as PFC_DEBUG is defined.
 *
 * Remarks:
 *	If the compiler is GNU C compiler, incorrect filename, file line
 *	number, and function name are embedded into the error message.
 */
#ifdef	__GNUC__

#define	__PFC_ASSERT_PRINTF(ex, fmt, ...)				\
	(PFC_EXPECT_TRUE(ex)						\
	 ? ((void)0)							\
	 : __pfc_assfail_printf(__FILE__, __LINE__, PFC_ASSERT_FUNCNAME, \
				fmt, ##__VA_ARGS__))

#ifdef	PFC_VERBOSE_DEBUG
#define	PFC_ASSERT_PRINTF(ex, fmt, ...)			\
	__PFC_ASSERT_PRINTF(ex, fmt, ##__VA_ARGS__)
#else	/* !PFC_VERBOSE_DEBUG */
#define	PFC_ASSERT_PRINTF(ex, fmt, ...)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

#ifdef	PFC_DEBUG
#define	PFC_VERIFY_PRINTF(ex, fmt, ...)			\
	__PFC_ASSERT_PRINTF(ex, fmt, ##__VA_ARGS__)
#else	/* !PFC_DEBUG */
#define	PFC_VERIFY_PRINTF(ex, fmt, ...)		((void)0)
#endif	/* PFC_DEBUG */

#else	/* !__GNUC__ */

/*
 * We can't pass correct filename, line number, and function name to
 * __pfc_assfail_vprintf() without variable argument macro support...
 */
static inline void PFC_FATTR_PRINTFLIKE(2, 3)
PFC_ASSERT_PRINTF(int ex, const char *fmt, ...)
{
#ifdef	PFC_VERBOSE_DEBUG
	if (PFC_EXPECT_FALSE(!ex)) {
		va_list	ap;

		va_start(ap, fmt);
		__pfc_assfail_vprintf(__FILE__, __LINE__, PFC_ASSERT_FUNCNAME,
				      fmt, ap);
		/* NOTREACHED */
	}
#endif	/* PFC_VERBOSE_DEBUG */
}

static inline void PFC_FATTR_PRINTFLIKE(2, 3)
PFC_VERIFY_PRINTF(int ex, const char *fmt, ...)
{
#ifdef	PFC_DEBUG
	if (PFC_EXPECT_FALSE(!ex)) {
		va_list	ap;

		va_start(ap, fmt);
		__pfc_assfail_vprintf(__FILE__, __LINE__, PFC_ASSERT_FUNCNAME,
				      fmt, ap);
		/* NOTREACHED */
	}
#endif	/* PFC_DEBUG */
}

#endif	/* __GNUC__ */

#ifdef	PFC_PTR_OBJSIZE

/*
 * Ensure that the object pointed by the given pointer has at least the
 * given size.
 *
 * Note that this assertion may not detect assertion if the object size
 * can't be determined at compile time.
 */
#define	PFC_PTR_OBJSIZE_ASSERT(ptr, size)				\
	PFC_ASSERT(PFC_PTR_OBJSIZE(ptr) == (size_t)-1 ||		\
		   PFC_PTR_OBJSIZE(ptr) >= (size))

#else	/* !PFC_PTR_OBJSIZE */
#define	PFC_PTR_OBJSIZE_ASSERT(ptr, size)
#endif	/* PFC_PTR_OBJSIZE */

#ifdef	PFC_ATTR_CLEANUP

/*
 * Assertion which detects unexpected leaving from local scope.
 */
typedef struct {
	const char	*file;
	uint32_t	line;
	int		flag;
} __pfc_disret_frame_t;

extern void	__pfc_return_assert(__pfc_disret_frame_t *frame);

#define	__PFC_DISABLE_RETURN_DONE	1

#ifdef	PFC_VERBOSE_DEBUG
#define	PFC_DISABLE_RETURN_BEGIN()					\
	do {								\
		__pfc_disret_frame_t __frame				\
			PFC_ATTR_CLEANUP(__pfc_return_assert) = {	\
			__FILE__, __LINE__, 0,				\
		};							\

#define	PFC_DISABLE_RETURN_END()					\
		__frame.flag = __PFC_DISABLE_RETURN_DONE;		\
	} while (0)
#endif	/* PFC_VERBOSE_DEBUG */

#endif	/* PFC_ATTR_CLEANUP */

#ifndef	PFC_DISABLE_RETURN_BEGIN
#define	PFC_DISABLE_RETURN_BEGIN()
#define	PFC_DISABLE_RETURN_END()
#endif	/* !PFC_DISABLE_RETURN_BEGIN */

PFC_C_END_DECL

#endif	/* !_PFC_DEBUG_H */
