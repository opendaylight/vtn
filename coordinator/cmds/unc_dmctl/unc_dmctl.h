/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_DMCTL_UNC_DMCTL_H
#define	_UNC_DMCTL_UNC_DMCTL_H

/*
 * Common definitions for unc_dmctl command.
 */

#include <unc/base.h>
#include <unc/exstatus.h>
#include <unc/lnc_ipc.h>
#include <pfc/debug.h>
#include <pfc/clock.h>
#include <pfc/util.h>
#include <pfc/listmodel.h>
#include <pfc/ipc_client.h>

/*
 * Exit status.
 */
#define	DMCTL_EX_OK		UNC_EX_OK	/* successfully completed */
#define	DMCTL_EX_FATAL		UNC_EX_FATAL	/* fatal error */
#define	DMCTL_EX_TEMPFAIL	UNC_EX_TEMPFAIL	/* temporary failure */
#define	DMCTL_EX_BUSY		UNC_EX_BUSY	/* resource busy */
#define	DMCTL_EX_NOTRUNNING	4		/* uncd is not running */
#define	DMCTL_EX_AUTH		5		/* authentication failed */
#define	DMCTL_EX_RESET		6		/* connection reset by uncd */
#define	DMCTL_EX_TIMEDOUT	7		/* timed out */
#define	DMCTL_EX_SUBCMD		8		/* subcommand specific */
#define	DMCTL_EX_UNDEFINED	(-1)		/* undefined value */

/*
 * Subcommand specifications.
 */
struct cmdspec;
typedef const struct cmdspec	cmdspec_t;

struct cmdspec {
	const char	*cs_name;		/* subcommand name */
	const char	*cs_fullname;		/* full name */
	uint32_t	cs_timeout;		/* default timeout */
	uint32_t	cs_flags;		/* flags */

	/*
	 * int
	 * cs_func(cmdspec_t *UNC_RESTRICT spec, int argc,
	 *	   char **UNC_RESTRICT argv)
	 *	Run subcommand.
	 *
	 * Calling/Exit State:
	 *	Exit status of the command is returned.
	 */
	int	(*cs_func)(cmdspec_t *UNC_RESTRICT spec, int argc,
			   char **UNC_RESTRICT argv);
};

/*
 * Flags for cs_flags.
 */
#define	CMDF_NOIPC		PFC_CONST_U(0x1)	/* don't use IPC */
#define	CMDF_HIDDEN		PFC_CONST_U(0x2)	/* hidden subcommand */

/*
 * Default timeout for clevent subcommand.
 */
#define	CLEVENT_TIMEOUT		PFC_CONST_U(30)		/* 30 seconds */

/*
 * Width of output columns.
 */
#define	DMCTL_COL_WIDTH		78

extern uint32_t		ipc_timeout;
extern const char	*ipc_channel;

extern const char	str_empty[];
extern const char	str_hyphen[];
extern const char	str_file[];
extern const char	str_secs[];
extern const char	str_msecs[];
extern const char	str_unknown[];
extern const char	str_prompt_prefix[];
extern const char	str_true[];
extern const char	str_false[];
extern const char	str_arg_names[];
extern const char	str_err_EPERM[];

extern uint32_t		debug_level;

/*
 * Prototypes.
 */
extern void	error(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	warning(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	debug_printf(uint32_t level, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);
extern int	uncd_stop(void);

extern const char	*proctype_getname(uint32_t type);

extern int	ipc_init(void);
extern void	ipc_fini(void);
extern int	ipc_getsess(pfc_ipcsess_t **UNC_RESTRICT sessp,
			    pfc_ipcid_t service,
			    const pfc_timespec_t *UNC_RESTRICT timeout);
extern int	ipc_invoke(pfc_ipcsess_t *UNC_RESTRICT sess,
			   pfc_ipcresp_t *UNC_RESTRICT respp);
extern void	ipc_gethostaddr(pfc_hostaddr_t *haddr);

extern int	uniqstr_create(pfc_listm_t *usp);
extern int	uniqstr_append(pfc_listm_t us, const char *string);

extern int	cmd_list(cmdspec_t *UNC_RESTRICT spec, int argc,
			 char **UNC_RESTRICT argv);
extern int	cmd_status(cmdspec_t *UNC_RESTRICT spec, int argc,
			   char **UNC_RESTRICT argv);
extern int	cmd_clevent(cmdspec_t *UNC_RESTRICT spec, int argc,
			    char **UNC_RESTRICT argv);
extern int	cmd_clstate(cmdspec_t *UNC_RESTRICT spec, int argc,
			    char **UNC_RESTRICT argv);

#endif	/* !_UNC_DMCTL_UNC_DMCTL_H */
