/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_EXTCMD_IMPL_H
#define	_PFC_LIBPFC_UTIL_EXTCMD_IMPL_H

/*
 * Internal definitions for pfc_extcmd_t.
 */

#include <pfc/base.h>
#include <pfc/synch.h>
#include <pfc/rbtree.h>
#include <pfc/extcmd.h>

/*
 * Invalid process ID.
 */
#define	EXTCMD_PID_INVALID	((pid_t)0)

/*
 * Maximum number of arguments.
 */
#define	EXTCMD_MAXARG		UINT16_MAX

/*
 * Program name of exec wrapper command.
 */
#define	EXTCMD_EXEC_NAME	"extcmd_exec"

/*
 * Control file descriptor, which is used to send error message from
 * exec wrapper command.
 */
#define	EXTCMD_CTRL_FD		3

/*
 * Maximum size of error log.
 */
#define	EXTCMD_ERRMSG_SIZE	PFC_CONST_U(128)

/*
 * Exit status which means internal error.
 */
#define	EXTCMD_ERRST_CHILD	126		/* error on child process */
#define	EXTCMD_ERRST_EXEC	127		/* error on extcmd_exec */

/*
 * Number of external command I/O handles.
 */
#define	EXTCMD_NHANDLES		((uint32_t)PFC_EXTCMD_STDERR + 1)

/*
 * Type of external command I/O handle.
 */
typedef enum {
	EXTCMD_IOTYPE_RETAIN	= 0,	/* retain original I/O handle */
	EXTCMD_IOTYPE_DISCARD	= 1,	/* discard input or output */
	EXTCMD_IOTYPE_FILE	= 2,	/* redirect from/to file */
	EXTCMD_IOTYPE_STREAM	= 3,	/* redirect from/to stdio stream */
	EXTCMD_IOTYPE_FD	= 4,	/* redirect from/to file descriptor */
} extcmd_iotype_t;

/*
 * I/O handle operation for external command.
 */
struct extcmd_ioops;
typedef struct extcmd_ioops	extcmd_ioops_t;

/*
 * I/O handle associated with external command.
 */
typedef struct {
	const extcmd_ioops_t	*eci_ops;	/* I/O operation */
} extcmd_io_t;

/*
 * Context to execute external command.
 *
 * Remarks:
 *	ec_refcnt must be modified by atomic operation.
 */
typedef struct {
	const char	*ec_command;		/* command path */
	const char	**ec_argv;		/* arguments */
	pfc_mutex_t	ec_mutex;		/* mutex */
	pfc_cond_t	ec_cond;		/* condition variable */
	pfc_extcmd_cb_t	ec_callback;		/* callback function */
	pfc_ptr_t	ec_cbarg;		/* argument for callback */
	pfc_rbnode_t	ec_node;		/* tree node */
	pfc_extcmd_t	ec_id;			/* context ID */
	uint16_t	ec_argc;		/* number of arguments */
	uint16_t	ec_capacity;		/* current capacity of argv */
	int		ec_status;		/* status of the last command */
	pid_t		ec_pid;			/* child process ID */
	uint32_t	ec_flags;		/* flags */
	uint32_t	ec_nwaiters;		/* number of waiting threads */

	/* I/O handles associated with the command. */
	extcmd_io_t	*ec_handles[EXTCMD_NHANDLES];
} extcmd_t;

#define	EXTCMD_NODE2PTR(node)	PFC_CAST_CONTAINER((node), extcmd_t, ec_node)

#define	EXTCMD_LOCK(cmd)	pfc_mutex_lock(&(cmd)->ec_mutex)
#define	EXTCMD_UNLOCK(cmd)	pfc_mutex_unlock(&(cmd)->ec_mutex)

#define	EXTCMD_WAIT(cmd)						\
	do {								\
		(cmd)->ec_nwaiters++;					\
		pfc_cond_wait(&(cmd)->ec_cond, &(cmd)->ec_mutex);	\
		(cmd)->ec_nwaiters--;					\
	} while (0)
#define	EXTCMD_BROADCAST(cmd)	pfc_cond_broadcast(&(cmd)->ec_cond)

/*
 * Flags for ec_flags.
 */
#define	ECMDF_EXECUTED		0x1U		/* command was executed */
#define	ECMDF_RUNNING		0x2U		/* command is now running */
#define	ECMDF_DESTROYED		0x4U		/* already destroyed */
#define	ECMDF_LOST		0x8U		/* child was lost */
#define	ECMDF_CALLBACK		0x10U		/* callbacking */
#define	ECMDF_CB_DESTROY	0x20U		/* destroy on callback */
#define	ECMDF_SYNC		0x40U		/* sync exec mode */

/*
 * Determine whether a child process associated with the specified context
 * is started, and not yet reaped.
 * This macro must be used with holding the command lock.
 */
#define	EXTCMD_IS_RUNNING(cmd)				\
	(((cmd)->ec_flags & ECMDF_RUNNING) ||		\
	 (cmd)->ec_pid != EXTCMD_PID_INVALID)

/*
 * External command operation.
 * This is used by pfcd to configure asynchronous process waiting.
 */
typedef struct {
	/*
	 * extcmd_t *
	 * ecops_alloc()
	 *	Allocate buffer for command context.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, a non-NULL pointer to command
	 *	context is returned.
	 *	NULL is returned on failure.
	 */
	extcmd_t	*(*ecops_alloc)(void);

	/*
	 * void
	 * ecops_free(extcmd_t *cmd)
	 *	Free buffer pointed by `cmd'.
	 */
	void	(*ecops_free)(extcmd_t *cmd);

	/*
	 * int
	 * ecops_register(extcmd_t *cmd)
	 *	Register external command which has active process.
	 *	Process ID of the child process must be set in cmd->ec_pid
	 *	in advance.
	 *
	 * Remarks:
	 *	This operation must NOT be called with holding the command
	 *	lock.
	 */
	void	(*ecops_register)(extcmd_t *cmd);

	/*
	 * int
	 * ecops_wait(extcmd_t *cmd)
	 *	Block the calling thread until the process associated with
	 *	`cmd' quits.
	 *
	 *	This operation must unregister the process information
	 *	registered by the call of ecops_register().
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, process status returned by the
	 *	call of wait(2) is set to cmd->ec_status, and then zero is
	 *	returned.
	 *	ECHILD is returned if no child process is associated with
	 *	`cmd'.
	 *
	 * Remarks:
	 *	This operation must be called with holding the command lock.
	 *	Note that this operation may release the lock for a while.
	 */
	int	(*ecops_wait)(extcmd_t *cmd);

	/*
	 * int
	 * ecops_on_callback(extcmd_t *cmd)
	 *	Determine whether that the caller is called on callback
	 *	context.
	 *
	 * Calling/Exit State:
	 *	Zero is returned if this function is not called on callback
	 *	context.
	 *
	 *	An error number is returned if this function is called on the
	 *	callback context. ELOOP is returned if `cmd' equals the context
	 *	currently callbacked, EDEADLK if it does not equal.
	 *
	 * Remarks:
	 *	This function must be called with holding the command lock.
	 */
	int	(*ecops_on_callback)(extcmd_t *cmd);

	/*
	 * void
	 * ecops_preexec(void)
	 *	Handler which is called on child process just before it
	 *	executes the command.
	 *
	 * Remarks:
	 *	Only fork-safe or async-signal-safe functions can be used
	 *	in this handler.
	 */
	void	(*ecops_preexec)(void);
} extcmd_ops_t;

/*
 * Prototypes.
 */
extern void	__pfc_extcmd_setops(const extcmd_ops_t *ops);
extern void	__pfc_extcmd_destroy(extcmd_t *cmd);
extern void	__pfc_extcmd_ioreset(extcmd_t *cmd);

#endif	/* _PFC_LIBPFC_UTIL_EXTCMD_IMPL_H */
