/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_DMCTL_DMCONF_H
#define	_UNC_DMCTL_DMCONF_H

/*
 * Definitions for daemon configuration.
 */

#include <stdio.h>
#include <unc/liblauncher.h>
#include <unc/lnc_types.h>
#include <pfc/refptr.h>
#include <pfc/rbtree.h>

/*
 * Daemon configuration.
 */
typedef struct {
	pfc_refptr_t	*dc_name;		/* daemon name */
	pfc_refptr_t	*dc_desc;		/* description */
	lnc_cmdmap_t	*dc_command;		/* daemon command */
	lnc_cmdmap_t	*dc_stop;		/* command to stop daemon */
	pfc_refptr_t	*dc_stop_signal;	/* signal to stop daemon */
	pfc_ptr_t	*dc_data;		/* arbitrary data */
	pfc_rbnode_t	dc_node;		/* tree node (dc_name) */
	pfc_bool_t	dc_uncd;		/* UNC daemon or not */
	pfc_bool_t	dc_start_wait;		/* wait for start up signal */
	int		dc_stop_sig;		/* parsed stop_signal */
	lnc_proctype_t	dc_type;		/* process type */
	uint32_t	dc_start_timeout;	/* start timeout */
	uint32_t	dc_stop_timeout;	/* stop timeout */
	uint32_t	dc_start_order;		/* starting order */
	uint32_t	dc_stop_order;		/* stopping order */
	int		dc_stderr_rotate;	/* stderr log rotation */

	/* Order of cluster state event delivery. */
	uint32_t	dc_clevent_order[CLSTAT_NEVTYPES];
} dmconf_t;

#define	DMCONF_NODE2PTR(node)					\
	PFC_CAST_CONTAINER((node), dmconf_t, dc_node)

#define	DMCONF_NAME(dcp)	pfc_refptr_string_value((dcp)->dc_name)
#define	DMCONF_DESC(dcp)	pfc_refptr_string_value((dcp)->dc_desc)
#define	DMCONF_STOP_SIGNAL(dcp)	pfc_refptr_string_value((dcp)->dc_stop_signal)
#define	DMCONF_INTKEY(v)	((pfc_cptr_t)(uintptr_t)(v))

/*
 * Fetch uint32_t value in dmconf_t at the given offset.
 */
#define	DMCONF_FETCH_UINT32(dcp, offset)		\
	(*((uint32_t *)((uintptr_t)(dcp) + (offset))))

/*
 * Intermediate tree node for the Red-Black tree node which keeps daemon
 * configuration indexed by UINT32 order value.
 */
typedef struct {
	uint32_t	dco_order;		/* order value */
	dmconf_t	*dco_conf;		/* daemon configuration */
	pfc_rbnode_t	dco_node;		/* Red-Black tree node */
} dmorder_t;

#define	DMORDER_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), dmorder_t, dco_node)

/*
 * Prototype of daemon configuration iterator function.
 */
typedef void	(*dmconf_iter_t)(dmconf_t *dcp, pfc_ptr_t arg);

/*
 * Prototypes.
 */
extern uint32_t	dmconf_load(void);
extern void	dmconf_cleanup(void);
extern dmconf_t	*dmconf_get(const char *name);
extern void	dmconf_iterate(dmconf_iter_t iterator, pfc_ptr_t arg);
extern void	dmconf_order_iterate(dmconf_iter_t iterator,
				     lnc_ordtype_t type, int index,
				     pfc_ptr_t arg);
extern void	dmconf_start_iterate(dmconf_iter_t iterator, pfc_ptr_t arg);
extern void	dmconf_stop_iterate(dmconf_iter_t iterator, pfc_ptr_t arg);

#endif	/* !_UNC_DMCTL_DMCONF_H */
