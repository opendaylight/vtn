/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_UTIL_IMPL_H
#define	_PFC_LIBPFC_UTIL_UTIL_IMPL_H

/*
 * Internal definitions for miscellaneous utilities in libpfc_util.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Cached system configurations.
 */
extern size_t		pfc_pagesize;
extern size_t		pfc_pageoffset;
extern size_t		pfc_pagemask;
extern uint32_t		pfc_pageshift;

extern pfc_ulong_t	pfc_clock_tick;

extern uint32_t		pfc_ngroups_max;

/*
 * Macros related to pagesize.
 */
#define	PFC_PAGE_ALIGN(addr)	((addr) & pfc_pagemask)
#define	PFC_PAGE_ROUNDUP(addr)	(((addr) + pfc_pageoffset) & pfc_pagemask)

/*
 * Determine whether we are running on the valgrind.
 */
extern pfc_bool_t	pfc_valgrind_mode;

/*
 * Prototypes.
 */
extern void	sysconf_init(void);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_UTIL_IMPL_H */
