/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_TPOOL_H
#define	_PFC_TPOOL_H

/*
 * Public definitions for PFC thread pool implementation.
 */

#include <pfc/base.h>
#include <pfc/clock.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Maximum length of thread pool name.
 */
#define	PFC_TPOOL_NAME_MAX	PFC_CONST_U(31)

/*
 * Prototypes.
 */
extern int	pfc_tpool_create(const char *name);
extern int	pfc_tpool_destroy(const char *PFC_RESTRICT name,
				  const pfc_timespec_t *PFC_RESTRICT timeout);

PFC_C_END_DECL

#endif	/* !_PFC_TPOOL_H */
