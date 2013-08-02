/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_MODULE_CLSTAT_LAUNCHER_H
#define	_UNC_MODULE_CLSTAT_LAUNCHER_H

/*
 * APIs provided only for launcher module.
 */

#include <unc/base.h>
#include <pfc/clock.h>
#include <pfc/synch.h>
#include <pfc/list.h>
#include <pfc/debug.h>
#include <pfc/ipc_client.h>

/*
 * IPC connection and client session.
 */
typedef struct {
	pfc_ipcsess_t	*cis_sess;		/* IPC client session */
	pfc_ipcconn_t	cis_conn;		/* alternative connection */
	pfc_list_t	cis_list;		/* link for session list */
} clst_ipcsess_t;

#define	CLST_IPCSESS_LIST2PTR(list)				\
	PFC_CAST_CONTAINER((list), clst_ipcsess_t, cis_list)

/*
 * List of clst_ipcsess_t.
 */
typedef struct {
	pfc_list_t	cil_sessions;		/* list of clst_ipcsess_t */
	pfc_mutex_t	cil_mutex;		/* mutex */
	pfc_cond_t	cil_cond;		/* condition variable */
	pfc_bool_t	cil_disabled;		/* true if disabled */
} clst_ipclist_t;

#define	CLST_IPCLIST_LOCK(ilp)		pfc_mutex_lock(&(ilp)->cil_mutex)
#define	CLST_IPCLIST_UNLOCK(ilp)	pfc_mutex_unlock(&(ilp)->cil_mutex)

#define	CLST_IPCLIST_BROADCAST(ilp)	pfc_cond_broadcast(&(ilp)->cil_cond)
#define	CLST_IPCLIST_TIMEDWAIT_ABS(ilp, abstime)			\
	pfc_cond_timedwait_abs(&(ilp)->cil_cond, &(ilp)->cil_mutex, (abstime))

/*
 * Prototypes.
 */
extern int	clstat_raise(uint8_t type, pfc_bool_t sysact,
			     const pfc_timespec_t *abstime);

extern int	clstat_sess_create(clst_ipclist_t *UNC_RESTRICT ilp,
				   clst_ipcsess_t **UNC_RESTRICT csessp,
				   const char *UNC_RESTRICT channel,
				   const char *UNC_RESTRICT name,
				   pfc_ipcid_t service);
extern void	clstat_sess_destroy(clst_ipclist_t *UNC_RESTRICT ilp,
				    clst_ipcsess_t *UNC_RESTRICT csess);
extern int	clstat_ipclist_fini(clst_ipclist_t *UNC_RESTRICT ilp,
				    const pfc_timespec_t *UNC_RESTRICT abstime);

/*
 * static inline void
 * clstat_ipclist_init(clst_ipclist_t *ilp)
 *	Initialize list of IPC client sessions.
 */
static inline void
clstat_ipclist_init(clst_ipclist_t *ilp)
{
	pfc_list_init(&ilp->cil_sessions);
	PFC_ASSERT_INT(PFC_MUTEX_INIT(&ilp->cil_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&ilp->cil_cond), 0);
	ilp->cil_disabled = PFC_FALSE;
}

#endif	/* !_UNC_MODULE_CLSTAT_LAUNCHER_H */
