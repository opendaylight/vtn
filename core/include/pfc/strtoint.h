/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_STRTOINT_H
#define	_PFC_STRTOINT_H

/*
 * Definitions for string to integer converter.
 */

#include <errno.h>
#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Prototypes.
 */
extern int	pfc_strtoi32(const char *PFC_RESTRICT string,
			     int32_t *PFC_RESTRICT valuep);
extern int	pfc_strtou32(const char *PFC_RESTRICT string,
			     uint32_t *PFC_RESTRICT valuep);
extern int	pfc_strtoi64(const char *PFC_RESTRICT string,
			     int64_t *PFC_RESTRICT valuep);
extern int	pfc_strtou64(const char *PFC_RESTRICT string,
			     uint64_t *PFC_RESTRICT valuep);

PFC_C_END_DECL

#endif	/* !_PFC_STRTOINT_H */
