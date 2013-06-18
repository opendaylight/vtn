/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_EXTCMD_H
#define	_PFC_EXTCMD_H

/*
 * Definitions for external command context, which enables to execute
 * external command in multi-threaded application.
 */

#include <stdio.h>
#include <errno.h>
#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Identifier of external command I/O handle.
 */
typedef enum {
	PFC_EXTCMD_STDIN	= 0,		/* standard input */
	PFC_EXTCMD_STDOUT	= 1,		/* standard output */
	PFC_EXTCMD_STDERR	= 2,		/* standard error output */
} pfc_extcmd_iodesc_t;

/*
 * Context to execute external command.
 */
typedef uint32_t	pfc_extcmd_t;

/*
 * Invalid external command context which is never assigned.
 */
#define	PFC_EXTCMD_INVALID	((uint32_t)0)

/*
 * Reaped process information to be passed to pfc_extcmd_t callback.
 */
typedef struct {
	pid_t		pep_pid;	/* process ID */
	int		pep_status;	/* status value passed to exit(2) */
	int		pep_signal;	/* signal number sent to child */
} pfc_extcmd_proc_t;

/*
 * Error number which will be returned by pfc_extcmd_get_status() and
 * pfc_extcmd_get_signal().
 */
#define	PFC_EXTCMD_ERR_UNSPEC	(-1)	/* unspecified status */
#define	PFC_EXTCMD_ERR_NOENT	(-2)	/* command context is not found */

/*
 * Determine whether a return value of pfc_extcmd_get_status() or
 * pfc_extcmd_get_signal() represents an error state.
 */
#define	PFC_EXTCMD_ISERR(ret)		((ret) < 0)

/*
 * Prototype of callback which notifies termination of child process created
 * by pfc_extcmd_t. Currently, only pfcd supports pfc_extcmd_t callback.
 */
typedef void	(*pfc_extcmd_cb_t)(pfc_extcmd_t ecmd,
				   const pfc_extcmd_proc_t *proc,
				   pfc_ptr_t arg);

/* Prototypes */
extern int	pfc_extcmd_create_async(pfc_extcmd_t *PFC_RESTRICT ecmdp,
					const char *command, const char *name,
					pfc_extcmd_cb_t callback);
extern int	pfc_extcmd_destroy(pfc_extcmd_t ecmd);
extern int	pfc_extcmd_add_arguments(pfc_extcmd_t ecmd, ...);
extern int	pfc_extcmd_clear_arguments(pfc_extcmd_t ecmd);
extern int	pfc_extcmd_execute(pfc_extcmd_t ecmd);
extern int	pfc_extcmd_start(pfc_extcmd_t ecmd, pid_t *pidp,
				 pfc_ptr_t arg);
extern int	pfc_extcmd_wait(pfc_extcmd_t ecmd);

extern int	pfc_extcmd_setio_retain(pfc_extcmd_t ecmd,
					pfc_extcmd_iodesc_t iodesc);
extern int	pfc_extcmd_setio_discard(pfc_extcmd_t ecmd,
					 pfc_extcmd_iodesc_t iodesc);
extern int	pfc_extcmd_setio_file(pfc_extcmd_t ecmd,
				      pfc_extcmd_iodesc_t iodesc,
				      const char *path, int oflags,
				      mode_t mode);
extern int	pfc_extcmd_setio_stream(pfc_extcmd_t ecmd,
					pfc_extcmd_iodesc_t iodesc);
extern int	pfc_extcmd_setio_fd(pfc_extcmd_t ecmd,
				    pfc_extcmd_iodesc_t iodesc, int fd);

extern int	pfc_extcmd_stream_get(pfc_extcmd_t ecmd,
				      pfc_extcmd_iodesc_t iodesc,
				      FILE **filep);
extern int	pfc_extcmd_stream_close(pfc_extcmd_t ecmd,
					pfc_extcmd_iodesc_t iodesc);

extern int	pfc_extcmd_get_status(pfc_extcmd_t ecmd);
extern int	pfc_extcmd_get_signal(pfc_extcmd_t ecmd);

#ifndef	_PFC_PFCD_MAINT_DISABLE
extern void	__pfc_extcmd_path_init(const char **path);
extern void	pfc_extcmd_enable_log(pfc_bool_t enable);
#endif	/* !_PFC_PFCD_MAINT_DISABLE */

/*
 * int
 * pfc_extcmd_create(pfc_extcmd_t *PFC_RESTRICT ecmdp,
 *		     const char *command, const char *name)
 *	Create external command context without callback function.
 *
 *	`command' is a path to command or command name. If it is an absolute
 *	path, it is considered as path to executable. If not,
 *	pfc_extcmd_create() searches for an executable file in the external
 *	command search path list, previously set by the call of
 *	__pfc_extcmd_path_init().
 *
 *	`name' specifies the process name, which will be set to the first
 *	entry of argument vector. If NULL is specified to `name', `command'
 *	is used as process name.
 *
 *	If a child process associated with the context is created by the
 *	call of pfc_extcmd_start(), the caller must call pfc_extcmd_wait()
 *	to reap child process.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to external command context is
 *	set to `*ecmdp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- pfc_extcmd_create() never uses PATH environment variable.
 *
 *	- If `command' does not starts with slash character, it must not
 *	  contain any slash character.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_extcmd_create(pfc_extcmd_t *PFC_RESTRICT ecmdp,
		  const char *command, const char *name)
{
	return pfc_extcmd_create_async(ecmdp, command, name, NULL);
}

PFC_C_END_DECL

#endif	/* !_PFC_EXTCMD_H */
