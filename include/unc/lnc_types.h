/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_LNC_TYPES_H
#define	_UNC_LNC_TYPES_H

/*
 * Common data types and constants used by UNC daemon launcher.
 */

#include <unc/base.h>
#include <unc/clstat_types.h>

UNC_C_BEGIN_DECL

/*
 * Maximum length of daemon name, excluding terminator.
 */
#define	LNC_DAEMON_NAMELEN_MAX		PFC_CONST_U(31)

/*
 * Daemon process type identifier.
 */
#define	LNC_PROCTYPE_UNSPEC		PFC_CONST_U(0)	/* unspecified */
#define	LNC_PROCTYPE_LOGICAL		PFC_CONST_U(1)	/* logical network */
#define	LNC_PROCTYPE_PHYSICAL		PFC_CONST_U(2)	/* physical network */
#define	LNC_PROCTYPE_DRIVER		PFC_CONST_U(3)	/* network driver */

#define	LNC_PROCTYPE_MAX		LNC_PROCTYPE_DRIVER

/*
 * The minimum and invalid value of order attribute.
 */
#define	LNC_ORDER_MIN			PFC_CONST_U(1)
#define	LNC_ORDER_INVALID		PFC_CONST_U(0)

/*
 * Order value which represents the UNC daemon.
 *   - This value is used as cluster event order for the UNC daemon.
 *   - This value is used as threshold of "stop_order" to determine the
 *     timing when to stop the daemon.
 */
#define	LNC_ORDER_UNCD			PFC_CONST_U(1000)

/*
 * Valid range of parameters in launcher.conf.
 */

/* options.event_timeout */
#define	LNC_CONF_MIN_event_timeout	PFC_CONST_U(100)
#define	LNC_CONF_MAX_event_timeout	PFC_CONST_U(3600000)

/* options.clact_timeout */
#define	LNC_CONF_MIN_clact_timeout	PFC_CONST_U(100)
#define	LNC_CONF_MAX_clact_timeout	PFC_CONST_U(3600000)

/* options.startup_timeout */
#define	LNC_CONF_MIN_startup_timeout	PFC_CONST_U(100)
#define	LNC_CONF_MAX_startup_timeout	PFC_CONST_U(3600000)

#define	LNC_CONF_MIN(name)		LNC_CONF_MIN_##name
#define	LNC_CONF_MAX(name)		LNC_CONF_MAX_##name

/*
 * Valid range of parameters in daemon configuration file. (XXX.daemon)
 */

/* daemon.start_timeout */
#define	LNC_DMCONF_MIN_start_timeout	PFC_CONST_U(100)
#define	LNC_DMCONF_MAX_start_timeout	PFC_CONST_U(3600000)

/* daemon.stop_timeout */
#define	LNC_DMCONF_MIN_stop_timeout	PFC_CONST_U(100)
#define	LNC_DMCONF_MAX_stop_timeout	PFC_CONST_U(3600000)

/* daemon.min_lifetime */
#define	LNC_DMCONF_MIN_min_lifetime	PFC_CONST_U(100)
#define	LNC_DMCONF_MAX_min_lifetime	PFC_CONST_U(3600000)

/* daemon.stderr_rotate */
#define	LNC_DMCONF_MAX_stderr_rotate	PFC_CONST_U(1000)

#define	LNC_DMCONF_MIN(name)		LNC_DMCONF_MIN_##name
#define	LNC_DMCONF_MAX(name)		LNC_DMCONF_MAX_##name

#if	!defined(_UNC_IN_CFDEFC) && !defined(_PFC_IN_CFDEFC)

/*
 * Number of process types.
 */
#define	LNC_NPROCTYPES			(LNC_PROCTYPE_MAX + 1)

/*
 * Data type of process type identifier.
 */
typedef uint32_t	lnc_proctype_t;

/*
 * Type of daemon order.
 */
typedef enum {
	LNC_ORDTYPE_START	= 0,		/* starting order */
	LNC_ORDTYPE_STOP,			/* stopping order */
	LNC_ORDTYPE_CLEVENT,			/* cluster event order */

	__LNC_ORDTYPE_MIN	= LNC_ORDTYPE_START,
	__LNC_ORDTYPE_INDEX_MIN	= LNC_ORDTYPE_CLEVENT,
} lnc_ordtype_t;

#define	LNC_ORDER_NTYPES	(LNC_ORDTYPE_CLEVENT + CLSTAT_NEVTYPES)

/*
 * Determine whether the order type takes an array index or not.
 */
#define	LNC_ORDTYPE_HASINDEX(type)	((type) >= __LNC_ORDTYPE_INDEX_MIN)

/*
 * System property key associated with the UNC daemon process ID.
 * This property is set only if the daemon is spawned by the launcher module.
 */
#define	LNC_PROP_PID		"core.uncd.pid"

/*
 * Extension status in lnc_clstate_t.
 */
#define	LNC_CLSTAT_EX_STABLE	PFC_CONST_U(0)		/* state is stable */
#define	LNC_CLSTAT_EX_TRANS	PFC_CONST_U(1)		/* in transition */
#define	LNC_CLSTAT_EX_ERROR	PFC_CONST_U(2)		/* in error state */
#define	LNC_CLSTAT_EX_NUM	(LNC_CLSTAT_EX_ERROR + PFC_CONST_U(1))

#define	LNC_CLSTAT_EX_NSHIFT	PFC_CONST_U(8)
#define	LNC_CLSTAT_EVTYPE_MASK						\
	((PFC_CONST_U(1) << LNC_CLSTAT_EX_NSHIFT) - PFC_CONST_U(1))

/*
 * Get extension status in in lnc_clstate_t.
 */
#define	LNC_CLSTAT_GETEXSTATUS(state)	((state) >> LNC_CLSTAT_EX_NSHIFT)

/*
 * Get event type embedded in lnc_clstate_t.
 */
#define	LNC_CLSTAT_GETEVTYPE(state)	((state) & LNC_CLSTAT_EVTYPE_MASK)

#define	LNC_CLSTAT_MKSTATE(evtype, exstatus)				\
	((LNC_CLSTAT_EX_##exstatus << LNC_CLSTAT_EX_NSHIFT) | (evtype))

#define	LNC_CLSTAT_DEFSTATE(type, exstate)				\
	LNC_CLSTAT_MKSTATE(CLSTAT_EVTYPE_##type, exstate)

/*
 * Cluster node state.
 */
typedef enum {
	LNC_CLSTATE_INITIAL	= LNC_CLSTAT_DEFSTATE(INVALID, STABLE),
	LNC_CLSTATE_ACT		= LNC_CLSTAT_DEFSTATE(ACT, STABLE),
	LNC_CLSTATE_ACT_TRANS	= LNC_CLSTAT_DEFSTATE(ACT, TRANS),
	LNC_CLSTATE_ACT_ERROR	= LNC_CLSTAT_DEFSTATE(ACT, ERROR),
} lnc_clstate_t;

#endif	/* !defined(_UNC_IN_CFDEFC) && !defined(_PFC_IN_CFDEFC) */

UNC_C_END_DECL

#endif	/* !_UNC_LNC_TYPES_H */
