/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * debug.c - Debugging utilities.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pfc/debug.h>
#include <pfc/atomic.h>

/* Size of static message buffer. */
#define	ASSERT_BUFSIZE		256

/* Internal buffer for assertion message. */
static char	assert_buffer[ASSERT_BUFSIZE];

/* Lock for assertions. */
#define	ASSERT_LOCK_LOCKED	0x1
#define	ASSERT_LOCK_UNLOCKED	0x0

static uint32_t	pfc_assert_lock = ASSERT_LOCK_UNLOCKED;

/* Nesting depth of assertion lock. */
static uint32_t	pfc_assert_lock_depth;

#ifdef	PFC_HAVE_POSIX_THREAD

#include <pthread.h>

/* Owner of assertion lock. */

static volatile pthread_t	pfc_assert_lock_owner = PFC_PTHREAD_INVALID_ID;

/* Save thread ID of assertion lock owner. */
static inline void
assert_lock_owner_save(void)
{
	pfc_assert_lock_owner = pthread_self();
}

/* Clear assertion lock owner. */
static inline void
assert_lock_owner_clear(void)
{
	pfc_assert_lock_owner = PFC_PTHREAD_INVALID_ID;
}

/* Determine whether the current thread already acquires assertion lock. */
static inline pfc_bool_t
assert_lock_is_owned(void)
{
	pfc_bool_t	ret;

	ret = (pthread_equal(pthread_self(), pfc_assert_lock_owner))
		? PFC_TRUE : PFC_FALSE;

	return ret;
}

#else	/* !PFC_HAVE_POSIX_THREAD */
#error	You need to implement code to allow recursive assertion lock.
#endif	/* PFC_HAVE_POSIX_THREAD */

/*
 * Internal prototypes.
 */
static void	assert_lock(void);
static void	assert_unlock(void);
static void	assert_printf(const char *file, uint32_t line,
			      const char *func, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(4, 5) PFC_FATTR_NONNULL((1, 4));
static void	assert_vprintf(const char *file, uint32_t line,
			       const char *func, const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(4, 0) PFC_FATTR_NONNULL((1, 4));

/*
 * void
 * __pfc_assfail(const char *ex, const char *file, uint32_t line,
 *		 const char *func)
 *	Called when PFC_ASSERT() or PFC_VERIFY() fails.
 *	Print error message to the standard error output, and then raise
 *	SIGABRT.
 */
void
__pfc_assfail(const char *ex, const char *file, uint32_t line, const char *func)
{
	assert_printf(file, line, func, "%s", ex);
	abort();
	/* NOTREACHED */
}

/*
 * int
 * __pfc_assfail_int(const char *ex, int value, int required,
 *		       const char *file, uint32_t line, const char *func)
 *	Called via PFC_ASSERT_INT().
 *	If `value' and `required' doesn't equal, print error message to the
 *	standard error output, and then raise SIGABRT.
 */
int
__pfc_assfail_int(const char *ex, int value, int required, const char *file,
		  uint32_t line, const char *func)
{
	if (PFC_EXPECT_FALSE(value != required)) {
		assert_printf(file, line, func, "\"%s\" must be %d, but %d",
			      ex, required, value);
		abort();
		/* NOTREACHED */
	}

	return value;
}

/*
 * void
 * __pfc_assfail_printf(const char *file, uint32_t line, const char *func,
 *			const char *fmt, ...)
 *	Print error message specified by printf(3) format to the standard
 *	error output, and then raise SIGABRT.
 */
void
__pfc_assfail_printf(const char *file, uint32_t line, const char *func,
		     const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	assert_vprintf(file, line, func, fmt, ap);
	va_end(ap);

	abort();
	/* NOTREACHED */
}

/*
 * void
 * __pfc_assfail_vprintf(const char *file, uint32_t line, const char *func,
 *			const char *fmt, va_list ap)
 *	Print error message specified by vprintf(3) format to the standard
 *	error output, and then raise SIGABRT.
 */
void
__pfc_assfail_vprintf(const char *file, uint32_t line, const char *func,
		      const char *fmt, va_list ap)
{
	assert_vprintf(file, line, func, fmt, ap);

	abort();
	/* NOTREACHED */
}

#ifdef	PFC_ATTR_CLEANUP

/*
 * void
 * __pfc_return_assert(__pfc_disret_frame_t *frame)
 *	Called when the thread leaves from local scope defined by
 *	PFC_DISABLE_RETURN_BEGIN() macro.
 *
 *	If the current thread reaches the end of local scope,
 *	frame->flag is updated to __PFC_DISABLE_RETURN_DONE by
 *	PFC_DISABLE_RETURN_END() macro. If it is about to leave local scope
 *	by "return" clause, frame->flag keeps initial value.
 */
void
__pfc_return_assert(__pfc_disret_frame_t *frame)
{
	if (PFC_EXPECT_FALSE(frame->flag != __PFC_DISABLE_RETURN_DONE)) {
		assert_printf(frame->file, frame->line, NULL,
			      "Unexpected premature leaving from the scope.");
		abort();
		/* NOTREACHED */
	}
}

#endif	/* PFC_ATTR_CLEANUP */

/*
 * static void
 * assert_lock(void)
 *	Acquire assertion lock.
 */
static void
assert_lock(void)
{
	uint32_t	old;

	do {
		if (assert_lock_is_owned()) {
			/* Allow recursive lock. */
			break;
		}
		old = pfc_atomic_cas_acq_uint32(&pfc_assert_lock,
						ASSERT_LOCK_LOCKED,
						ASSERT_LOCK_UNLOCKED);
	} while (old != ASSERT_LOCK_UNLOCKED);

	pfc_assert_lock_depth++;
	assert_lock_owner_save();
}

/*
 * static void
 * assert_unlock(void)
 *	Release assertion lock.
 */
static void
assert_unlock(void)
{
	pfc_assert_lock_depth--;
	if (pfc_assert_lock_depth == 0) {
		assert_lock_owner_clear();
		(void)pfc_atomic_swap_rel_uint32(&pfc_assert_lock,
						 ASSERT_LOCK_UNLOCKED);
	}
}

/*
 * static void
 * assert_printf(const char *file, uint32_t line, const char *func,
 *		 const char *fmt, ...)
 *	Print assertion message to the standard error output.
 *	Newline character is appended to the message automatically.
 *
 *	`file' is a string which represents source file name, and `line' is
 *	the line number in `file' to be reported.
 *
 *	`func' is a string which represents function name where assertion
 *	fails. It may be NULL if C preprocessor does not have ability to
 *	obtain function name.
 *
 *	`fmt' and `ap' specifies error message to be dumped, specified
 *	by printf(3) format.
 */
static void
assert_printf(const char *file, uint32_t line, const char *func,
	      const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	assert_vprintf(file, line, func, fmt, ap);
	va_end(ap);
}

/*
 * static void
 * assert_vprintf(const char *file, uint32_t line, const char *func,
 *		  const char *fmt, va_list ap)
 *	Print assertion message to the standard error output.
 *	Newline character is appended to the message automatically.
 *
 *	`file' is a string which represents source file name, and `line' is
 *	the line number in `file' to be reported.
 *
 *	`func' is a string which represents function name where assertion
 *	fails. It may be NULL if C preprocessor does not have ability to
 *	obtain function name.
 *
 *	`fmt' and `ap' specifies error message to be dumped, specified
 *	by vprintf(3) format.
 */
static void
assert_vprintf(const char *file, uint32_t line, const char *func,
	       const char *fmt, va_list ap)
{
	const char	*sep, *funcname;
	char		*buf = assert_buffer;
	size_t		bufsize = sizeof(assert_buffer);
	int		len;

	assert_lock();

	/*
	 * Dump file name, line number, and function name into message
	 * buffer.
	 */
	if (func == NULL) {
		sep = "";
		funcname = "";
	}
	else {
		sep = ": ";
		funcname = func;
	}
	len = snprintf(buf, bufsize, "%s:%u%s%s: Assertion Failed: ",
		       file, line, sep, funcname);
	if (PFC_EXPECT_FALSE(len < 0 || (size_t)len >= bufsize)) {
		/* This should not happen. */
		snprintf(buf, bufsize, fmt, ap);
	}
	else {
		bufsize -= len;
		buf += len;
		vsnprintf(buf, bufsize, fmt, ap);
	}

	assert_buffer[ASSERT_BUFSIZE - 1] = '\0';
	fputs(assert_buffer, stderr);
	putc('\n', stderr);
	fflush(stderr);

	assert_unlock();
}
