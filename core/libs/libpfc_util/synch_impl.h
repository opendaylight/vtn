/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_SYNCH_IMPL_H
#define	_PFC_LIBPFC_UTIL_SYNCH_IMPL_H

/*
 * Internal definitions for thread synchronization.
 */

#include <pfc/debug.h>

/*
 * Internal assertion.
 */
#ifdef	__GNUC__
#define	PFC_SYNC_ASSERT(ex, file, line, func, fmt, ...)			\
	(PFC_EXPECT_TRUE(ex)						\
	 ? ((void)0)							\
	 : __pfc_assfail_printf(file, line, func, fmt, ##__VA_ARGS__))
#else	/* !__GNUC__ */
extern void	PFC_SYNC_ASSERT(int ex, const char *file, uint32_t line,
				const char *func, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(5, 6);
#endif	/* __GNUC__ */

#endif	/* !_PFC_LIBPFC_UTIL_SYNCH_IMPL_H */
