/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFC_TASKQ_H
#define _PFC_TASKQ_H

/*
 * Public definitions for PFC task queue implementation.
 */

#include <pfc/base.h>
#include <pfc/clock.h>
#include <pfc/log.h>
#include <stdint.h>

PFC_C_BEGIN_DECL

/*
 * Task queue identifier.
 */
typedef uint32_t	pfc_taskq_t;

/*
 * Task identifier.
 */
typedef uint32_t	pfc_task_t;

/*
 * Prototype for task function.
 */
typedef void		(*pfc_taskfunc_t)(void *arg);

/*
 * Prototype for argument destructor.
 */
typedef void		(*pfc_taskdtor_t)(void *arg);

/*
 * Flags for pfc_taskq_dispatch()
 */
#define	PFC_TASKQ_JOINABLE	PFC_CONST_U(0x1)

/*
 * Invalid ID number for taskq.
 */
#define	PFC_TASKQ_INVALID_ID		(pfc_taskq_t)0
#define	PFC_TASKQ_INVALID_TASKID	(pfc_task_t)0

/*
 * Prototypes.
 */
extern int	pfc_taskq_create_named(pfc_taskq_t *PFC_RESTRICT,
				       const char *PFC_RESTRICT, uint32_t,
				       const char *PFC_RESTRICT);
extern int	pfc_taskq_destroy(pfc_taskq_t);
extern int	pfc_taskq_dispatch(pfc_taskq_t, pfc_taskfunc_t,
				   void *, uint32_t, pfc_task_t *PFC_RESTRICT);
extern int	pfc_taskq_dispatch_dtor(pfc_taskq_t, pfc_taskfunc_t,
					void *, pfc_taskdtor_t, uint32_t,
					pfc_task_t *PFC_RESTRICT);
extern int	pfc_taskq_cancel(pfc_taskq_t, pfc_task_t);
extern int	pfc_taskq_join(pfc_taskq_t, pfc_task_t);
extern int	pfc_taskq_timedjoin(pfc_taskq_t, pfc_task_t,
				    const pfc_timespec_t *PFC_RESTRICT);
extern int	pfc_taskq_set_free(pfc_taskq_t, uint32_t);
extern int	pfc_taskq_flush(pfc_taskq_t,
				const pfc_timespec_t *PFC_RESTRICT);
extern int	pfc_taskq_clear(pfc_taskq_t,
				const pfc_timespec_t *PFC_RESTRICT);

#define	pfc_taskq_create(tqidp, poolname, concurrency)	\
	pfc_taskq_create_named(tqidp, poolname, concurrency, PFC_LOG_IDENT)

PFC_C_END_DECL

#endif /* _PFC_TASKQ_H */
