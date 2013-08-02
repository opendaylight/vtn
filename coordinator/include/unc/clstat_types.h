/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_CLSTAT_TYPES_H
#define	_UNC_CLSTAT_TYPES_H

/*
 * Common data types and constants used by cluster state management.
 */

#include <unc/base.h>

UNC_C_BEGIN_DECL

/*
 * PFC event type which represents change of cluster state.
 *
 * CLSTAT_EVTYPE_ACT
 *	Indicates that the cluster node state is going to be changed to
 *	active.
 */
#define	CLSTAT_EVTYPE_ACT	PFC_CONST_U(0)

/* Invalid cluster event type. */
#define	CLSTAT_EVTYPE_INVALID	PFC_CONST_U(0xff)

/*
 * Number of cluster event types.
 */
#define	CLSTAT_NEVTYPES		PFC_CONST_U(1)

#if	CLSTAT_NEVTYPES != (CLSTAT_EVTYPE_ACT + 1)
#error	CLSTAT_NEVTYPES value is inconsistent.
#endif	/* !CLSTAT_NEVTYPES != (CLSTAT_EVTYPE_ACT + 1) */

/*
 * Valid range of parameters in clstat.conf.
 */

/* options.init_timeout */
#define	CLSTAT_CONF_MIN_init_timeout	PFC_CONST_U(100)
#define	CLSTAT_CONF_MAX_init_timeout	PFC_CONST_U(3600000)

/* options.ack_timeout */
#define	CLSTAT_CONF_MIN_ack_timeout	PFC_CONST_U(100)
#define	CLSTAT_CONF_MAX_ack_timeout	PFC_CONST_U(3600000)

#define	CLSTAT_CONF_MIN(name)		CLSTAT_CONF_MIN_##name
#define	CLSTAT_CONF_MAX(name)		CLSTAT_CONF_MAX_##name

UNC_C_END_DECL

#endif	/* !_UNC_CLSTAT_TYPES_H */
