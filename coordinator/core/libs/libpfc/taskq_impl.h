/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_TASKQ_IMPL_H
#define	_PFC_LIBPFC_TASKQ_IMPL_H

/*
 * Internal definitions for task queue.
 */

#include <pfc/taskq.h>
#include <pfc/atomic.h>
#include <stdio.h>

PFC_C_BEGIN_DECL

/*
 * Prototypes.
 */
extern void	pfc_taskq_init(void);
extern void	pfc_taskq_fini(void);

extern void	pfc_timer_init(void);
extern void	pfc_timer_fini(void);

extern void	pfc_wdt_init(void);
extern void	pfc_wdt_fini(void);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_TASKQ_IMPL_H */
