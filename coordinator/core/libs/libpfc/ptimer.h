/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_PTIMER_H
#define	_PFC_LIBPFC_PTIMER_H

/*
 * Common definition for periodic timer.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Periodic timer handle.
 */
struct __pfc_ptimer;
typedef struct __pfc_ptimer	*pfc_ptimer_t;

/*
 * Invalid value of pfc_ptimer_t.
 */
#define	PFC_PTIMER_INVALID		((pfc_ptimer_t)NULL)

/*
 * Timeout ID for periodic timer.
 */
typedef uint32_t	pfc_ptimeout_t;

/*
 * Invalid timeout ID, which will never be used.
 */
#define	PFC_PTIMEOUT_INVALID		PFC_CONST_U(0)

/*
 * Prototype for timeout function.
 */
typedef void		(*pfc_ptmfunc_t)(pfc_ptr_t arg);

/*
 * Timeout attributes.
 */
typedef struct {
	/*
	 * Interval between periodic timeout in milliseconds.
	 */
	uint32_t	ptma_interval;

	/*
	 * If PFC_TRUE is set, timeout is enabled.
	 * Otherwise timeout is disabled.
	 */
	pfc_bool_t	ptma_enabled;
} pfc_ptmattr_t;

/*
 * Prototypes.
 */
extern int	pfc_ptimer_create(pfc_ptimer_t *timerp);
extern int	pfc_ptimer_destroy(pfc_ptimer_t timer);
extern int	pfc_ptimer_timeout(pfc_ptimer_t timer, pfc_ptmfunc_t func,
				   pfc_ptr_t arg,
				   const pfc_ptmattr_t *PFC_RESTRICT attrp,
				   pfc_ptimeout_t *PFC_RESTRICT idp);
extern int	pfc_ptimer_setattr(pfc_ptimer_t timer, pfc_ptimeout_t id,
				   const pfc_ptmattr_t *PFC_RESTRICT attrp);
extern int	pfc_ptimer_oneshot(pfc_ptimer_t timer, pfc_ptimeout_t id);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_PTIMER_H */
