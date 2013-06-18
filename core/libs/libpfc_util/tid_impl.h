/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_TID_IMPL_H
#define	_PFC_LIBPFC_UTIL_TID_IMPL_H

/*
 * Private header that defines utility to obtain system thread identifier.
 */

#include <pfc/base.h>
#include <pfc/debug.h>

#ifdef	PFC_OSTYPE_LINUX

/*
 * Linux-specific definitions.
 */
#include <unistd.h>
#include <sys/syscall.h>

/*
 * Type of system thread identifier.
 */
typedef pid_t	pfc_tid_t;

/*
 * Invalid value of pfc_tid_t.
 */
#define	PFC_TID_INVALID			0

#define	__PFC_UTIL_DEFINE_PROTO_GETTID

#ifdef	_PFC_LIBPFC_UTIL_BUILD

/*
 * Thread ID cache.
 */
extern __thread pfc_tid_t	tid_cache;

/*
 * static inline pfc_tid_t PFC_FATTR_ALWAYS_INLINE
 * __pfc_gettid(void)
 *	Return identifier of system thread for the calling thread.
 */
static inline pfc_tid_t PFC_FATTR_ALWAYS_INLINE
__pfc_gettid(void)
{
	pfc_tid_t	*cachep = &tid_cache;
	pfc_tid_t	tid = *cachep;

	if (PFC_EXPECT_FALSE(tid == PFC_TID_INVALID)) {
		/* Use gettid system call. */
		tid = (pfc_tid_t)syscall(SYS_gettid);
		PFC_ASSERT(tid != PFC_TID_INVALID && tid != (pfc_tid_t)-1);
		*cachep = tid;
	}

	return tid;
}

#ifndef	__PFC_UTIL_DONT_DEFINE_GETTID

/*
 * Use inline version of pfc_gettid().
 */
#define	pfc_gettid()	__pfc_gettid()

#undef	__PFC_UTIL_DEFINE_PROTO_GETTID

#endif	/* !__PFC_UTIL_DONT_DEFINE_GETTID */

/*
 * static void
 * libpfc_tid_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
static inline void
libpfc_tid_fork_child(void)
{
	/* Clear TID cache. */
	tid_cache = PFC_TID_INVALID;
}

#endif	/* _PFC_LIBPFC_UTIL_BUILD */

#ifdef	__PFC_UTIL_DEFINE_PROTO_GETTID
extern pfc_tid_t	pfc_gettid(void);
#endif	/* __PFC_UTIL_DEFINE_PROTO_GETTID */

#else	/* !PFC_OSTYPE_LINUX */
#error	"Unsupported OS."
#endif	/* PFC_OSTYPE_LINUX */

#endif	/* !_PFC_LIBPFC_UTIL_TID_IMPL_H */
