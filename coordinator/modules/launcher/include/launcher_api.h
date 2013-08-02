/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_MODULE_LAUNCHER_INC_LAUNCHER_API_H
#define	_UNC_MODULE_LAUNCHER_INC_LAUNCHER_API_H

/*
 * Definitions for APIs provided by the launcher module.
 */

#include <unc/lnc_types.h>
#include <pfc/module.h>
#include <pfc/event.h>
#include <pfc/clock.h>

UNC_C_BEGIN_DECL

/*
 * Information of a daemon process launched by the launcher module.
 */
#define	LNC_DMINFO_NAME_BUFSIZE		(LNC_DAEMON_NAMELEN_MAX + 1)
#define	LNC_DMINFO_CHANNEL_BUFSIZE	(PFC_IPC_CHANNEL_NAMELEN_MAX + 1)

typedef struct {
	/* Name of the daemon. */
	char		dmi_name[LNC_DMINFO_NAME_BUFSIZE];

	/* IPC channel name provided by the daemon. */
	char		dmi_channel[LNC_DMINFO_CHANNEL_BUFSIZE];

	/* Process type. (LNC_PROCTYPE_XXX) */
	lnc_proctype_t	dmi_type;

	/* Process ID of the daemon. */
	pid_t		dmi_pid;
} lnc_dminfo_t;

/*
 * List of daemon process information.
 */
struct lnc_dmlist;
typedef struct lnc_dmlist	lnc_dmlist_t;

/*
 * PFC event type raised by the launcher module.
 *
 * LNC_EVTYPE_INIT
 *	Indicates that all daemons have been launched successfully.
 *	This event has no private data.
 */
#define	LNC_EVTYPE_INIT			PFC_CONST_U(0)

/*
 * Prototypes.
 */
extern int	lncapi_isinitialized(void);
extern int	lncapi_dmlist_create(lnc_proctype_t type,
				     lnc_dmlist_t **UNC_RESTRICT listpp,
				     const pfc_timespec_t *UNC_RESTRICT tout);
extern void	lncapi_dmlist_destroy(lnc_dmlist_t *listp);
extern int	lncapi_dmlist_get(lnc_dmlist_t *UNC_RESTRICT listp,
				  uint32_t index,
				  lnc_dminfo_t *UNC_RESTRICT infop);
extern uint32_t	lncapi_dmlist_getsize(lnc_dmlist_t *listp);
extern int	lncapi_getdminfo(const char *UNC_RESTRICT name,
				 lnc_dminfo_t *UNC_RESTRICT infop,
				 const pfc_timespec_t *UNC_RESTRICT tout);

extern const char	*lncapi_event_getsource(void);

UNC_C_END_DECL

#endif	/* !_UNC_MODULE_LAUNCHER_INC_LAUNCHER_API_H */
