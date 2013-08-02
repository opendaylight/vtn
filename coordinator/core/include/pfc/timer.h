/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFC_TIMER_H
#define _PFC_TIMER_H

/*
 * Public definitions for PFC timer implementation.
 */

#include <pfc/base.h>
#include <pfc/clock.h>
#include <pfc/taskq.h>
#include <stdint.h>

PFC_C_BEGIN_DECL

/*
 * Timer identifier.
 */
typedef uint32_t	pfc_timer_t;

/*
 * Timeout identifier.
 */
typedef uint32_t	pfc_timeout_t;

/*
 * Invalid ID number for timer.
 */
#define	PFC_TIMER_INVALID_ID		(pfc_timer_t)0
#define	PFC_TIMER_INVALID_TIMEOUTID	(pfc_timeout_t)0

/*
 * Exclusive upper boundary of timer resolution in seconds.
 */
#define	PFC_TIMER_MAXRES		PFC_CONST_U(0x1000000)

/*
 * Prototypes.
 */
extern int	pfc_timer_create(pfc_timer_t *PFC_RESTRICT tidp,
				 const char *PFC_RESTRICT poolname,
				 pfc_taskq_t tqid,
				 const pfc_timespec_t *resolution);
extern int	pfc_timer_destroy(pfc_timer_t tid);
extern int	pfc_timer_post(pfc_timer_t tid, const pfc_timespec_t *timeout,
			       pfc_taskfunc_t func, void *PFC_RESTRICT arg,
			       pfc_timeout_t *toidp);
extern int	pfc_timer_post_dtor(pfc_timer_t tid,
				    const pfc_timespec_t *timeout,
				    pfc_taskfunc_t func, void *PFC_RESTRICT arg,
				    pfc_taskdtor_t dtor, pfc_timeout_t *toidp);
extern int	pfc_timer_cancel(pfc_timer_t tid, pfc_timeout_t toid);

PFC_C_END_DECL

#endif /* _PFC_TIMER_H */
