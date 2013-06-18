/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CONTROL_PFC_CONTROL_H
#define	_PFC_CONTROL_PFC_CONTROL_H

/*
 * Common definitions for pfc_control command.
 */

#include <errno.h>
#include <sys/time.h>
#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/util.h>
#include <pfc/exstatus.h>
#include <pfc/debug.h>
#include <pfc/iostream.h>
#include <pfc/conf.h>
#include <cmdutil.h>
#include <ctrl_client.h>

/*
 * The name of the target daemon.
 */
extern const char	*daemon_name;

/*
 * Return value of subcommand operations.
 */
typedef enum {
	CMDRET_COMPLETE,		/* successfully completed */
	CMDRET_CONT,			/* proceed to subcommand sequence */
	CMDRET_FAIL,			/* failed */
} ctrlcmd_ret_t;

struct ctrlcmd_spec;
typedef struct ctrlcmd_spec	ctrlcmd_spec_t;

/*
 * Subcommand specifications.
 */
struct ctrlcmd_spec {
	const char	*cs_name;		/* subcommand name */
	pfc_refptr_t	*cs_fullname;		/* subcommand full name */
	ctrl_cmdtype_t	cs_command;		/* protocol command type */
	uint16_t	cs_flags;		/* flags */

	/*
	 * Below are subcommand operations.
	 * Each method which returns ctrlcmd_ret_t must return one of the
	 * followings.
	 *
	 * CMDRET_COMPLETE
	 *	All subcommand work has been finished.
	 *
	 * CMDRET_CONT
	 *	Proceed to subcommand sequence.
	 *
	 * CMDRET_FAIL
	 *	Subcommand sequence has been terminated due to error.
	 */

	/*
	 * Subcommand constructor.
	 * Constructor must not be NULL.
	 */
	ctrlcmd_ret_t	(*cs_ctor)(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				   int argc, char **PFC_RESTRICT argv);

	/*
	 * Subcommand destructor.
	 */
	void		(*cs_dtor)(const ctrlcmd_spec_t *spec);

	/*
	 * Prepare execution of the command.
	 */
	ctrlcmd_ret_t	(*cs_prepare)(const ctrlcmd_spec_t *spec,
				      cproto_sess_t *PFC_RESTRICT sess);

	/*
	 * Send optional argument for command.
	 */
	int		(*cs_send)(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				   cproto_sess_t *PFC_RESTRICT sess);

	/*
	 * Receive optional response, and process command.
	 */
	int		(*cs_receive)(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				      cproto_sess_t *PFC_RESTRICT sess);

	/*
	 * Initialize pfc_iowait_t which determines behavior of I/O wait on
	 * the control protocol socket.
	 */
	int		(*cs_iowait)(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				     pfc_iowait_t *PFC_RESTRICT iowait);
};

#define	CTRLCMD_FULLNAME(spec)				\
	pfc_refptr_string_value((spec)->cs_fullname)

/*
 * Flags for cs_flags.
 */
#define	CTRLCMDF_NEED_PRIV		0x1U	/* need privilege */

/*
 * Subcommands.
 */

/* stop */
extern ctrlcmd_ret_t	cmd_ctor_stop(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				      int argc, char **PFC_RESTRICT argv);
#define	cmd_dtor_stop		NULL
#define	cmd_prepare_stop	NULL
#define	cmd_send_stop		NULL
#define	cmd_receive_stop	NULL
#define	cmd_iowait_stop		NULL

/* pid */
extern ctrlcmd_ret_t	cmd_ctor_pid(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				     int argc, char **PFC_RESTRICT argv);
#define	cmd_dtor_pid		NULL
#define	cmd_prepare_pid		NULL
#define	cmd_send_pid		NULL
#define	cmd_receive_pid		NULL
#define	cmd_iowait_pid		NULL

/* loglevel */
extern ctrlcmd_ret_t	cmd_ctor_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT
					  spec, int argc,
					  char **PFC_RESTRICT argv);
extern int		cmd_send_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT
					  spec,
					  cproto_sess_t *PFC_RESTRICT sess);
extern int		cmd_receive_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT
					     spec,
					     cproto_sess_t *PFC_RESTRICT sess);
extern void		cmd_dtor_loglevel(const ctrlcmd_spec_t *spec);

#define	cmd_prepare_loglevel	NULL
#define	cmd_iowait_loglevel	NULL

/* modlist */
extern ctrlcmd_ret_t	cmd_ctor_modlist(const ctrlcmd_spec_t *PFC_RESTRICT
					 spec, int argc,
					 char **PFC_RESTRICT argv);
extern int		cmd_receive_modlist(const ctrlcmd_spec_t *PFC_RESTRICT
					    spec,
					    cproto_sess_t *PFC_RESTRICT sess);
#define	cmd_prepare_modlist	NULL
#define	cmd_send_modlist	NULL
#define	cmd_dtor_modlist	NULL
#define	cmd_iowait_modlist	NULL

/* ping */
extern ctrlcmd_ret_t	cmd_ctor_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				      int argc, char **PFC_RESTRICT argv);
extern void		cmd_dtor_ping(const ctrlcmd_spec_t *spec);
extern int		cmd_iowait_ping(const ctrlcmd_spec_t *PFC_RESTRICT spec,
					pfc_iowait_t *PFC_RESTRICT iowait);

#define	cmd_prepare_ping	NULL
#define	cmd_send_ping		NULL
#define	cmd_receive_ping	NULL

/* event */
extern ctrlcmd_ret_t	cmd_ctor_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				       int argc, char **PFC_RESTRICT argv);
extern ctrlcmd_ret_t	cmd_prepare_event(const ctrlcmd_spec_t *PFC_RESTRICT
					  spec,
					  cproto_sess_t *PFC_RESTRICT sess);
extern int		cmd_send_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
				       cproto_sess_t *PFC_RESTRICT sess);
extern int		cmd_receive_event(const ctrlcmd_spec_t *PFC_RESTRICT
					  spec,
					  cproto_sess_t *PFC_RESTRICT sess);
extern void		cmd_dtor_event(const ctrlcmd_spec_t *spec);

#define	cmd_iowait_event	NULL

/* checkconf */
extern ctrlcmd_ret_t	cmd_ctor_checkconf(const ctrlcmd_spec_t *PFC_RESTRICT
					   spec,
					   int argc, char **PFC_RESTRICT argv);
#define	cmd_dtor_checkconf	NULL
#define	cmd_prepare_checkconf	NULL
#define	cmd_send_checkconf	NULL
#define	cmd_receive_checkconf	NULL
#define	cmd_iowait_checkconf	NULL

/*
 * Set interval timer.
 * We use one shot timer only, so it_interval is always set to zero.
 */
#define	CTRL_ITIMER_SET(it, sec, usec)			\
	do {						\
		(it)->it_interval.tv_sec = 0;		\
		(it)->it_interval.tv_usec = 0;		\
		(it)->it_value.tv_sec = (sec);		\
		(it)->it_value.tv_usec = (usec);	\
	} while (0)

extern const char	*pfcd_work_dir;
extern const char	*pfcd_pidfile;
extern void		(*not_running_hook)(void);
extern pfc_cfblk_t	pfcd_options;
extern int		ctrl_debug;
extern uint32_t		ctrl_timeout;
extern int		ctrl_err_status;

extern const char	str_timeout[];
extern const char	str_number[];
extern const char	str_size[];
extern const char	str_interval[];
extern const char	str_empty[];
extern const char	str_comma[];
extern const char	str_s[];
extern const char	str_prompt_prefix[];

#define	ENGLISH_PLUAL(value)	(((value) <= 1) ? str_empty : str_s)

/*
 * Prototypes.
 */
extern void	fatal(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2) PFC_FATTR_NORETURN;
extern void	error(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
extern void	error_v(const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(1, 0);
extern void	warning(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2);
extern void	debug_printf(int level, const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);
extern void	not_running_ipc(const char *chaddr);
extern void	not_allowed(void);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * not_running(void)
 *	Print error message that informs the PFC daemon is not running.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
not_running(void)
{
	not_running_ipc(NULL);
}

extern int	open_pidfile(pfc_pidf_t *pfp);
extern int	get_daemon_pid(pfc_pidf_t pf, pid_t *pidp);

extern int		client_create(cproto_sess_t *PFC_RESTRICT sess,
				      pfc_iowait_t *PFC_RESTRICT iwp);
extern ctrlcmd_ret_t	client_execute(cproto_sess_t *PFC_RESTRICT sess,
				       const ctrlcmd_spec_t *PFC_RESTRICT spec);

extern int	prompt(void);
extern int	as_bool(const char *PFC_RESTRICT str, pfc_bool_t *boolp);
extern void	pretty_format_seconds(char *buf, size_t bufsize, uint32_t sec);
extern void	pretty_format_bytes(char *buf, size_t bufsize, uint64_t bytes);

#endif	/* !_PFC_CONTROL_PFC_CONTROL_H */
