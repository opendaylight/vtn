/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_LIBPFC_IMPL_H
#define	_PFC_LIBPFC_LIBPFC_IMPL_H

/*
 * libpfc internal definitions.
 */

#include <stdarg.h>
#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/event.h>
#include <pfc/clock.h>
#include "ptimer.h"

PFC_C_BEGIN_DECL

#ifdef	_PFC_LIBPFC_BUILD
extern const char	libpfc_conf_options[];
extern const char	libpfc_conf_reap_interval[];
extern const char	*libpfc_progname;
#endif	/* _PFC_LIBPFC_BUILD */

extern void		libpfc_init(void);
extern void		libpfc_setprogname(pfc_refptr_t *rname);
extern void		libpfc_fini(void);
extern void		libpfc_shutdown_start(void);
extern int		libpfc_shutdown_getfd(void);
extern pfc_bool_t	libpfc_is_shutdown(void);
extern pfc_ptimer_t	libpfc_getptimer(void);
extern int		libpfc_post_sysevent(pfc_evtype_t type,
					     const pfc_timespec_t *timeout);

#ifdef	_PFC_LIBPFC_BUILD
extern void	libpfc_ipc_init(void);
extern void	libpfc_ipc_fini(void);
#endif	/* _PFC_LIBPFC_BUILD */

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_LIBPFC_IMPL_H */
