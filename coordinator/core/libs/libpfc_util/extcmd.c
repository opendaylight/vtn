/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * extcmd.c - Execute external command.
 *
 * Remarks:
 *	Reaping child process must be done by the code in this file.
 *	The use of pfc_extcmd_t with the following code causes undefined
 *	behavior:
 *
 *	- The call of wait(2) system call.
 *	- The call of waitpid(2) or waitid(2) system call with specifying -1
 *	  as process ID.
 *	- The call of waitpid(2) or waitid(2) system call with specifying
 *	  process ID of a process created by pfc_extcmd_t.
 */

/*
 * Use "extcmd" for log identifier.
 */
static const char	log_ident[] = "extcmd";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pfc/config.h>
#include <pfc/util.h>
#include <pfc/extcmd.h>
#include <pfc/debug.h>
#include <pfc/log.h>
#include <pfc/atomic.h>
#include "extcmd_impl.h"
#include "util_impl.h"

/*
 * Absolute path to exec wrapper command.
 */
#define	EXTCMD_EXEC_PATH	PFC_LIBEXECDIR "/" EXTCMD_EXEC_NAME

/*
 * Number of exec wrapper command arguments.
 */
#define	EXTCMD_EXEC_NARGS	4

/*
 * Initial number of command arguments.
 */
#define	EXTCMD_NARGS_INITIAL	(EXTCMD_EXEC_NARGS + 1)

/*
 * Number of arguments to be added on argv expansion.
 */
#define	EXTCMD_ARGV_INCR	8

#if	EXTCMD_ARGV_INCR <= EXTCMD_NARGS_INITIAL
#error	EXTCMD_ARGV_INCR must be greater than EXTCMD_NARGS_INITIAL.
#endif	/* EXTCMD_ARGV_INCR <= EXTCMD_NARGS_INITIAL */

/*
 * Initial size of path buffer used to resolve path to an executable file.
 */
#define	EXTCMD_PATH_LENGTH	PFC_CONST_U(64)

/*
 * Default external command search path.
 */
static const char	*extcmd_default_path[] = {
	PFC_BINDIR,
#ifdef	PFC_EXTCMD_BINDIR
	PFC_EXTCMD_BINDIR,
#endif	/* PFC_EXTCMD_BINDIR */
	"/bin",
	"/usr/bin",
	NULL,
};

/*
 * External command search path.
 */
static const char	**extcmd_path = extcmd_default_path;

/*
 * External command operations.
 * This is used by pfcd to configure asynchronous process waiting.
 */
static const extcmd_ops_t	*extcmd_ops = NULL;

/*
 * Command context ID for the next allocation.
 */
static pfc_extcmd_t		extcmd_id_next = PFC_EXTCMD_INVALID;

/*
 * Flag which indicates whether internal log should be recorded by PFC logging
 * architecture.
 * This flag is updated by atomic operation.
 */
static volatile uint32_t	extcmd_log_enabled = 0;

/*
 * Internal logging functions.
 */

#ifdef	__PFC_LOG_GNUC

#define	EXTCMD_LOG_ERROR(format, ...)			\
	if (extcmd_log_enabled) {			\
		pfc_log_error((format), ##__VA_ARGS__);	\
	}						\

#define	EXTCMD_LOG_WARN(format, ...)			\
	if (extcmd_log_enabled) {			\
		pfc_log_warn((format), ##__VA_ARGS__);	\
	}						\

#define	EXTCMD_LOG_DEBUG(format, ...)			\
	if (extcmd_log_enabled) {			\
		pfc_log_debug((format), ##__VA_ARGS__);	\
	}						\

#else	/* !__PFC_LOG_GNUC */

static void PFC_FATTR_PRINTFLIKE(1, 2)
EXTCMD_LOG_ERROR(const char *format, ...)
{
	if (extcmd_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

static void PFC_FATTR_PRINTFLIKE(1, 2)
EXTCMD_LOG_WARN(const char *format, ...)
{
	if (extcmd_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_warn_v(format, ap);
		va_end(ap);
	}
}

static void PFC_FATTR_PRINTFLIKE(1, 2)
EXTCMD_LOG_DEBUG(const char *format, ...)
{
	if (extcmd_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_debug_v(format, ap);
		va_end(ap);
	}
}

#endif	/* __PFC_LOG_GNUC */

/*
 * I/O handle specific to EXTCMD_IOTYPE_FILE.
 */
typedef struct {
	extcmd_io_t	ecif_io;		/* common data */
	const char	*ecif_path;		/* file path */
	int		ecif_oflags;		/* open flags */
	mode_t		ecif_mode;		/* file mode */
} extcmd_io_file_t;

#define	EXTCMD_IO2FILE(iop)					\
	PFC_CAST_CONTAINER((iop), extcmd_io_file_t, ecif_io)

/*
 * I/O handle specific to EXTCMD_IOTYPE_STREAM.
 */
typedef struct {
	extcmd_io_t	ecis_io;		/* common data */
	FILE		*ecis_stream;		/* I/O stream */
	int		ecis_parent_fd;		/* FD for parent */
	int		ecis_child_fd;		/* FD for child */
} extcmd_io_stream_t;

#define	EXTCMD_IO2STREAM(iop)					\
	PFC_CAST_CONTAINER((iop), extcmd_io_stream_t, ecis_io)

/*
 * I/O handle specific to EXTCMD_IOTYPE_FD.
 */
typedef struct {
	extcmd_io_t	ecid_io;		/* common data */
	int		ecid_fd;		/* file descriptor */
} extcmd_io_fd_t;

#define	EXTCMD_IO2FD(iop)					\
	PFC_CAST_CONTAINER((iop), extcmd_io_fd_t, ecid_io)

/*
 * Internal prototypes.
 */
static pfc_cptr_t	extcmd_keyfunc(pfc_rbnode_t *node);
static extcmd_t		*extcmd_lookup(pfc_extcmd_t ecmd);

static int	extcmd_register(extcmd_t *cmd);
static int	extcmd_expand_argv(extcmd_t *cmd);
static int	extcmd_resolve_path(extcmd_t *PFC_RESTRICT cmd,
				    const char *PFC_RESTRICT command);
static int	extcmd_is_executable(const char *path);
static int	extcmd_check_exec(extcmd_t *cmd);
static int	extcmd_check_async(const extcmd_ops_t *PFC_RESTRICT ops,
				   extcmd_t *PFC_RESTRICT cmd);
static int	extcmd_execute(const extcmd_ops_t *PFC_RESTRICT ops,
			       extcmd_t *PFC_RESTRICT cmd,
			       pid_t *PFC_RESTRICT pidp);
static void	extcmd_child_bind_fd(int ctrlfd, int fd, int target,
				     pfc_bool_t do_close);
static void	extcmd_child_error(int fd, const char *msg, int err)
	PFC_FATTR_NORETURN PFC_FATTR_NOINLINE;
static int	extcmd_child_write(int fd, const uint8_t *src, size_t size);
static int	extcmd_child_do_wait(const extcmd_ops_t *PFC_RESTRICT ops,
				     extcmd_t *PFC_RESTRICT cmd);

static int	extcmd_setio_begin(pfc_extcmd_t ecmd,
				   pfc_extcmd_iodesc_t iodesc,
				   extcmd_t **cmdp);
static void	extcmd_setio_commit(extcmd_t *PFC_RESTRICT cmd,
				    pfc_extcmd_iodesc_t iodesc,
				    extcmd_io_t *PFC_RESTRICT iop);

static int	extcmd_io_prepare(extcmd_t *cmd);
static void	extcmd_io_child(extcmd_t *cmd, int ctrlfd);
static void	extcmd_io_reset(extcmd_t *cmd);

static int	extcmd_io_discard_create(extcmd_io_t **iopp);
static void	extcmd_io_discard_child(extcmd_io_t *iop,
					pfc_extcmd_iodesc_t iodesc,
					int ctrlfd);
static void	extcmd_io_discard_dtor(extcmd_io_t *iop);

static int	extcmd_io_file_create(extcmd_io_t **PFC_RESTRICT iopp,
				      pfc_extcmd_iodesc_t iodesc,
				      const char *PFC_RESTRICT path,
				      int oflags, mode_t mode);
static void	extcmd_io_file_child(extcmd_io_t *iop,
				     pfc_extcmd_iodesc_t iodesc, int ctrlfd);
static void	extcmd_io_file_dtor(extcmd_io_t *iop);

static int	extcmd_io_stream_create(extcmd_io_t **PFC_RESTRICT iopp,
					pfc_extcmd_iodesc_t iodesc);
static int	extcmd_io_stream_prepare(extcmd_io_t *iop,
					 pfc_extcmd_iodesc_t iodesc);
static void	extcmd_io_stream_parent(extcmd_io_t *iop);
static void	extcmd_io_stream_child(extcmd_io_t *iop,
				       pfc_extcmd_iodesc_t iodesc, int ctrlfd);
static void	extcmd_io_stream_reset(extcmd_io_t *iop);
static void	extcmd_io_stream_dtor(extcmd_io_t *iop);

static int	extcmd_io_fd_create(extcmd_io_t **iopp,
				    pfc_extcmd_iodesc_t iodesc, int fd);
static void	extcmd_io_fd_child(extcmd_io_t *iop,
				   pfc_extcmd_iodesc_t iodesc,
				   int ctrlfd);
static void	extcmd_io_fd_dtor(extcmd_io_t *iop);

static int	extcmd_stream_get(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
				  extcmd_t **PFC_RESTRICT cmdp,
				  extcmd_io_stream_t **PFC_RESTRICT iospp);

/*
 * Global context which manages pairs of command context ID and context.
 * Note that the global tree lock must be held before the context lock is held.
 */
typedef struct {
	pfc_rbtree_t	et_tree;		/* context tree */
	pfc_rwlock_t	et_lock;		/* read-write lock */
} extcmd_tree_t;

static extcmd_tree_t	extcmd_tree = {
	.et_tree	= PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare,
						 extcmd_keyfunc),
	.et_lock	= PFC_RWLOCK_INITIALIZER,
};

#define	EXTCMD_TREE_RDLOCK(etp)					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(etp)->et_lock), 0)
#define	EXTCMD_TREE_WRLOCK(etp)					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(etp)->et_lock), 0)
#define	EXTCMD_TREE_UNLOCK(etp)					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(etp)->et_lock), 0)

/*
 * Cast context ID to Red-Black Tree key.
 */
#define	EXTCMD_KEY(id)		((pfc_cptr_t)(uintptr_t)(id))

/*
 * Determine whether pfc_extcmd_iodesc_t value is valid or not.
 */
#define	EXTCMD_IODESC_IS_VALID(iodesc)			\
	((uint32_t)(iodesc) <= PFC_EXTCMD_STDERR)

/*
 * I/O handle operation.
 */
struct extcmd_ioops {
	/*
	 * int
	 * ioops_prepare(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc)
	 *	Prepare I/O handle.
	 *	This function is called just before the call of fork(2).
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 *
	 * Remarks:
	 *	This operation is called with holding the command lock
	 *	associated with the specified I/O handle.
	 */
	int	(*ioops_prepare)(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc);

	/*
	 * void
	 * ioops_parent(extcmd_io_t *iop)
	 *	This function is called just after fork(2) on parent process.
	 *
	 * Remarks:
	 *	This operation is called with holding the command lock
	 *	associated with the specified I/O handle.
	 */
	void	(*ioops_parent)(extcmd_io_t *iop);

	/*
	 * void
	 * ioops_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
	 *	       int ctrlfd)
	 *	This function is called just after fork(2) on child process.
	 */
	void	(*ioops_child)(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
			       int ctrlfd);

	/*
	 * void
	 * ioops_reset(extcmd_io_t *iop)
	 *	Reset I/O handle.
	 *	Any changes made by the call of ioops_prepare() must be
	 *	reverted.
	 *
	 * Remarks:
	 *	This operation is called with holding the command lock
	 *	associated with the specified I/O handle.
	 */
	void	(*ioops_reset)(extcmd_io_t *iop);

	/*
	 * void
	 * ioops_dtor(extcmd_io_t *iop)
	 *	Destructor of I/O handle.
	 */
	void	(*ioops_dtor)(extcmd_io_t *iop);
};

/*
 * Call I/O operation which takes only extcmd_io_t instance, and returns
 * void value. This macro must be used with holding the command lock.
 */
#define	EXTCMD_IO_CALL(iop, method)				\
	do {							\
		if ((iop) != NULL) {				\
			const extcmd_ioops_t	*__ops;		\
								\
			__ops = (iop)->eci_ops;			\
			if (__ops->method != NULL) {		\
				__ops->method(iop);		\
			}					\
		}						\
	} while (0)

#define	EXTCMD_IO_CALL_FOREACH(cmd, method)				\
	do {								\
		extcmd_io_t	**__iopp;				\
									\
		for (__iopp = (cmd)->ec_handles;			\
		     __iopp < PFC_ARRAY_LIMIT((cmd)->ec_handles);	\
		     __iopp++) {					\
			extcmd_io_t	*__iop = *__iopp;		\
									\
			EXTCMD_IO_CALL(__iop, method);			\
		}							\
									\
	} while (0)

/*
 * I/O handle operations.
 */

#define	EXTCMD_IOOPS_METHOD_DECL(type, method)	\
	.ioops_##method		= extcmd_io_##type##_##method

/* For EXTCMD_IOTYPE_DISCARD. */
static const extcmd_ioops_t	extcmd_ioops_discard = {
	EXTCMD_IOOPS_METHOD_DECL(discard, child),
	EXTCMD_IOOPS_METHOD_DECL(discard, dtor),
};

/* For EXTCMD_IOTYPE_FILE. */
static const extcmd_ioops_t	extcmd_ioops_file = {
	EXTCMD_IOOPS_METHOD_DECL(file, child),
	EXTCMD_IOOPS_METHOD_DECL(file, dtor),
};

/* For EXTCMD_IOTYPE_STREAM. */
static const extcmd_ioops_t	extcmd_ioops_stream = {
	EXTCMD_IOOPS_METHOD_DECL(stream, prepare),
	EXTCMD_IOOPS_METHOD_DECL(stream, parent),
	EXTCMD_IOOPS_METHOD_DECL(stream, child),
	EXTCMD_IOOPS_METHOD_DECL(stream, reset),
	EXTCMD_IOOPS_METHOD_DECL(stream, dtor),
};

/* For EXTCMD_IOTYPE_FD. */
static const extcmd_ioops_t	extcmd_ioops_fd = {
	EXTCMD_IOOPS_METHOD_DECL(fd, child),
	EXTCMD_IOOPS_METHOD_DECL(fd, dtor),
};

/*
 * Test I/O handle type.
 */
#define	EXTCMD_IO_ISTYPE(iop, type)					\
	((iop) != NULL && (iop)->eci_ops == &extcmd_ioops_##type)

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * extcmd_free(const extcmd_ops_t *ops, extcmd_t *cmd)
 *	Free buffer specified by `cmd'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
extcmd_free(const extcmd_ops_t *ops, extcmd_t *cmd)
{
	if (ops != NULL) {
		ops->ecops_free(cmd);
	}
	else {
		free(cmd);
	}
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * extcmd_verify_accmode(int fflags, pfc_extcmd_iodesc_t iodesc)
 *	Ensure that the file open flag is suitable for the specified I/O
 *	descriptor.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified open flag is suitable.
 *	Otherwise EACCESS is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
extcmd_verify_accmode(int fflags, pfc_extcmd_iodesc_t iodesc)
{
	int	accmode = fflags & O_ACCMODE;

	if (accmode != O_RDWR) {
		int	reqmode = (iodesc == PFC_EXTCMD_STDIN)
			? O_RDONLY : O_WRONLY;

		if (PFC_EXPECT_FALSE(accmode != reqmode)) {
			return EACCES;
		}
	}

	return 0;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * extcmd_child_wait(const extcmd_ops_t *PFC_RESTRICT ops,
 *		     extcmd_t *PFC_RESTRICT cmd)
 *	Block the calling thread until the process associated with `cmd' quits.
 *
 * Calling/Exit State:
 *	Upon successful completion, process status returned by the call of
 *	waitpid(2) is set to cmd->ec_status, and then zero is returned.
 *
 *	ECHILD is returned of no child process is associated with the command
 *	context, or another thread is already waiting.
 *	ESRCH is returned if no child process is found due to internal error.
 *	ENOENT is returned if the specified context is destroyed.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 *	Note that it will be released for a while.
 */
static int
extcmd_child_wait(const extcmd_ops_t *PFC_RESTRICT ops,
		  extcmd_t *PFC_RESTRICT cmd)
{
	int	err;

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already destroyed. */
		err = ENOENT;
	}
	else {
		/* Obtain exit status of the child. */
		err = extcmd_child_do_wait(ops, cmd);
	}

	return err;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * extcmd_log_arguments(extcmd_t *cmd)
 *	Dump command arguments just for debugging.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
extcmd_log_arguments(extcmd_t *cmd)
{
	pfc_log_burst_verbose_begin();
	{
		const char	**argv;
		int		argc;

		for (argv = cmd->ec_argv + EXTCMD_EXEC_NARGS, argc = 0;
		     argv < cmd->ec_argv + cmd->ec_argc; argv++, argc++) {
			pfc_log_burst_write("argv[%d] = %s", argc, *argv);
		}
	}
	pfc_log_burst_end();
}

/*
 * void
 * __pfc_extcmd_path_init(const char **path)
 *	Initialize directory path list to search external command executable.
 *
 * Remarks:
 *	- This function is designed to be called once while the process is
 *	  single-threaded. So this function doesn't issue any data
 *	  serialization.
 *	- The specified path list is used directly without copy.
 *	  The caller must guarantee that the specified path list is not
 *	  released.
 *	- The last entry of the path list must be NULL.
 */
void
__pfc_extcmd_path_init(const char **path)
{
	if (path == NULL) {
		/* Use default path. */
		extcmd_path = extcmd_default_path;
	}
	else {
		extcmd_path = path;
	}
}

/*
 * int
 * pfc_extcmd_create_async(pfc_extcmd_t *PFC_RESTRICT ecmdp,
 *			   const char *command, const char *name,
 *			   pfc_extcmd_cb_t callback)
 *	Create external command context.
 *
 *	`command' is a path to command or command name. If it is an absolute
 *	path, it is considered as path to executable. If not,
 *	pfc_extcmd_create_async() searches for an executable file in the
 *	external command search path list, previously set by the call of
 *	__pfc_extcmd_path_init().
 *
 *	`name' specifies the process name, which will be set to the first
 *	entry of argument vector. If NULL is specified to `name', `command'
 *	is used as process name.
 *
 *	`callback' is a pointer to callback function. If a non-NULL value is
 *	specified, it will be called when the child process associated with
 *	the command context is terminated. `cbarg' is an arbitrary pointer
 *	value to be passed to callback function.
 *
 *	If a non-NULL callback function is specified, child process must be
 *	reaped by the callback function. The following APIs always returns
 *	EPERM error if it is called with specifying command context with
 *	callback:
 *
 *	  - pfc_extcmd_execute()
 *	  - pfc_extcmd_wait()
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to external command context is
 *	set to `*ecmdp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- pfc_extcmd_create_async() never uses PATH environment variable.
 *
 *	- If `command' does not starts with slash character, it must not
 *	  contain any slash character.
 */
int
pfc_extcmd_create_async(pfc_extcmd_t *PFC_RESTRICT ecmdp,
			const char *command, const char *name,
			pfc_extcmd_cb_t callback)
{
	const extcmd_ops_t	*ops = extcmd_ops;
	extcmd_t	*cmd;
	extcmd_io_t	**iopp;
	const char	**argv, *pname = NULL;
	int		err;

	/* Allocate command context. */
	if (ops != NULL) {
		cmd = ops->ecops_alloc();
	}
	else if (PFC_EXPECT_FALSE(callback != NULL)) {
		/* Asynchronous mode is not supported. */
		return ENOSYS;
	}
	else {
		cmd = (extcmd_t *)malloc(sizeof(*cmd));
	}
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOMEM;
	}

	if (*command == '/') {
		/* Ensure that the specified executable file exists. */
		err = extcmd_is_executable(command);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		cmd->ec_command = strdup(command);
		if (PFC_EXPECT_FALSE(cmd->ec_command == NULL)) {
			err = ENOMEM;
			goto error;
		}
	}
	else {
		/* Resolve path to an executable file. */
		err = extcmd_resolve_path(cmd, command);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}

	cmd->ec_capacity = 0;
	cmd->ec_status = 0;
	cmd->ec_pid = EXTCMD_PID_INVALID;
	cmd->ec_argv = NULL;		/* Set NULL for rollback. */
	cmd->ec_callback = callback;
	cmd->ec_cbarg = NULL;
	cmd->ec_nwaiters = 0;
	cmd->ec_flags = 0;
	PFC_MUTEX_INIT(&cmd->ec_mutex);
	PFC_ASSERT_INT(pfc_cond_init(&cmd->ec_cond), 0);

	/* Expand argument vector for argv[0]. */
	err = extcmd_expand_argv(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_free;
	}
	PFC_ASSERT(cmd->ec_capacity == EXTCMD_ARGV_INCR);

	/* Set exec wrapper path and arguments. */
	argv = cmd->ec_argv;
	*argv = EXTCMD_EXEC_NAME;
	*(argv + 1) = "-c";
	*(argv + 2) = "--";
	*(argv + 3) = cmd->ec_command;
	argv += EXTCMD_EXEC_NARGS;

	/* Determine process name. */
	pname = (name == NULL)
		? (const char *)strdup(command)
		: (const char *)strdup(name);
	if (PFC_EXPECT_FALSE(pname == NULL)) {
		err = ENOMEM;
		goto error_free;
	}
	*argv = pname;

	/* Initialize I/O handle type to retain original handle. */
	for (iopp = cmd->ec_handles; iopp < PFC_ARRAY_LIMIT(cmd->ec_handles);
	     iopp++) {
		*iopp = NULL;
	}

	cmd->ec_argc = EXTCMD_NARGS_INITIAL;
	*(cmd->ec_argv + EXTCMD_NARGS_INITIAL) = NULL;

	/* Register the command context to the global context tree. */
	err = extcmd_register(cmd);
	if (PFC_EXPECT_TRUE(err == 0)) {
		PFC_ASSERT(cmd->ec_id != PFC_EXTCMD_INVALID);
		*ecmdp = cmd->ec_id;

		return 0;
	}

error_free:
	free((void *)pname);
	free((void *)cmd->ec_command);
	free(cmd->ec_argv);

error:
	extcmd_free(ops, cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_destroy(pfc_extcmd_t ecmd)
 *	Destroy the specified external command context if no other thread uses
 *	the context.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	SIGKILL is sent to a child process if it is still running.
 */
int
pfc_extcmd_destroy(pfc_extcmd_t ecmd)
{
	const extcmd_ops_t	*ops = extcmd_ops;
	extcmd_tree_t	*etp = &extcmd_tree;
	pfc_rbtree_t	*tree = &etp->et_tree;
	pfc_rbnode_t	*node;
	extcmd_t	*cmd;
	pid_t		pid;

	/* Search for a context associated with the specified ID. */
	EXTCMD_TREE_WRLOCK(etp);

	node = pfc_rbtree_get(tree, EXTCMD_KEY(ecmd));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		EXTCMD_TREE_UNLOCK(etp);

		return ENOENT;
	}

	cmd = EXTCMD_NODE2PTR(node);
	EXTCMD_LOCK(cmd);

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already passed to pfc_extcmd_destroy(). */
		EXTCMD_UNLOCK(cmd);
		EXTCMD_TREE_UNLOCK(etp);

		return ENOENT;
	}

	/* Disable further command execution on this context. */
	cmd->ec_flags |= ECMDF_DESTROYED;
	EXTCMD_BROADCAST(cmd);
	EXTCMD_TREE_UNLOCK(etp);

	pid = cmd->ec_pid;
	if (PFC_EXPECT_FALSE(pid != EXTCMD_PID_INVALID)) {
		/* Terminate child process by sending SIGKILL. */
		EXTCMD_LOG_WARN("%s: Terminate by sending SIGKILL: pid=%d",
				cmd->ec_command, pid);
		(void)kill(pid, SIGKILL);
	}

	if ((cmd->ec_flags & (ECMDF_CALLBACK | ECMDF_CB_DESTROY)) ||
	    (pid != EXTCMD_PID_INVALID && cmd->ec_callback != NULL)) {
		/* The rest of work will be done by the callback thread. */
		cmd->ec_flags |= ECMDF_CB_DESTROY;
		EXTCMD_UNLOCK(cmd);

		return EINPROGRESS;
	}

	if (PFC_EXPECT_FALSE(pid != EXTCMD_PID_INVALID)) {
		if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_RUNNING)) {
			int	err;

			err = extcmd_child_do_wait(ops, cmd);
			if (PFC_EXPECT_FALSE(err != 0)) {
				EXTCMD_LOG_WARN("%s: Failed to reap "
						"process(%d): %s",
						cmd->ec_command, pid,
						strerror(err));
				cmd->ec_pid = EXTCMD_PID_INVALID;
			}
		}
		else {
			/* Another thread is waiting for this process. */
			do {
				EXTCMD_WAIT(cmd);
			} while (cmd->ec_pid != EXTCMD_PID_INVALID);
		}
	}

	EXTCMD_UNLOCK(cmd);
	__pfc_extcmd_destroy(cmd);

	return 0;
}

/*
 * int
 * pfc_extcmd_add_arguments(pfc_extcmd_t ecmd, ...)
 *	Add arguments to the specified command context.
 *	Just like execl(2), type of each argument must be 'const char *', and
 *	the last argument must be NULL.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_add_arguments(pfc_extcmd_t ecmd, ...)
{
	extcmd_t	*cmd;
	int		err = 0;
	uint16_t	oldargc, argc;
	va_list		ap;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already destroyed. */
		EXTCMD_UNLOCK(cmd);

		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(EXTCMD_IS_RUNNING(cmd))) {
		/* A child process is running, or not yet reaped. */
		EXTCMD_UNLOCK(cmd);

		return EBUSY;
	}

	va_start(ap, ecmd);

	/* Preserve argc for rollback. */
	oldargc = argc = cmd->ec_argc;
	PFC_ASSERT(argc <= cmd->ec_capacity);

	while (1) {
		const char	*buf, *arg = va_arg(ap, const char *);

		if (arg == NULL) {
			break;
		}

		buf = (const char *)strdup(arg);
		if (PFC_EXPECT_FALSE(buf == NULL)) {
			err = ENOMEM;
			break;
		}

		if (argc == cmd->ec_capacity) {
			err = extcmd_expand_argv(cmd);
			if (PFC_EXPECT_FALSE(err != 0)) {
				free((void *)buf);
				break;
			}
		}

		*(cmd->ec_argv + argc) = buf;
		argc++;
	}

	va_end(ap);
	cmd->ec_argc = argc;

	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Append argument terminator. */
		if (argc < cmd->ec_capacity) {
			*(cmd->ec_argv + argc) = NULL;
		}
		else {
			err = extcmd_expand_argv(cmd);
			if (PFC_EXPECT_TRUE(err == 0)) {
				*(cmd->ec_argv + argc) = NULL;
			}
		}
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		int	ac;
		const char	**argv = cmd->ec_argv + oldargc;

		/* Rollback. */
		for (ac = oldargc; ac < argc; ac++, argv++) {
			free((void *)*argv);
		}
		cmd->ec_argc = oldargc;
		*(cmd->ec_argv + oldargc) = NULL;
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_clear_arguments(pfc_extcmd_t ecmd)
 *	Clear all arguments added by the call of pfc_extcmd_add_arguments().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_clear_arguments(pfc_extcmd_t ecmd)
{
	extcmd_t	*cmd;
	const char	**argv;
	int		err;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already destroyed. */
		err = ENOENT;
		goto out;
	}

	if (PFC_EXPECT_FALSE(EXTCMD_IS_RUNNING(cmd))) {
		/* A child process is running, or not yet reaped. */
		err = EBUSY;
		goto out;
	}

	for (argv = cmd->ec_argv + EXTCMD_NARGS_INITIAL;
	     argv < cmd->ec_argv + cmd->ec_argc; argv++) {
		free((void *)*argv);
	}
	cmd->ec_argc = EXTCMD_NARGS_INITIAL;
	*(cmd->ec_argv + EXTCMD_NARGS_INITIAL) = NULL;

	err = 0;

out:
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_execute(pfc_extcmd_t ecmd)
 *	Execute the external command with specifying arguments added by
 *	the call of pfc_extcmd_add_arguments().
 *
 *	The calling thread is blocked until the child process quits.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EPERM is returned if callback function is configured on the command
 *	context specified by `ecmd'.
 *	ENOENT is returned if the specified context is not found.
 *	EDEADLK is returned if at least one I/O handle is configured as
 *	stream mode.
 *	EAGAIN is returned if this function is called on the callback context.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	It doesn't mean the command has been successfully completed even if
 *	pfc_extcmd_execute() returns zero. The command status must be
 *	tested by the call of pfc_extcmd_get_status() or
 *	pfc_extcmd_get_signal().
 */
int
pfc_extcmd_execute(pfc_extcmd_t ecmd)
{
	const extcmd_ops_t	*ops = extcmd_ops;
	extcmd_t	*cmd;
	extcmd_io_t	**iopp;
	int		err;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	/* Reject if the context has callback function. */
	if (PFC_EXPECT_FALSE(cmd->ec_callback != NULL)) {
		err = EPERM;
		goto out;
	}
	PFC_ASSERT((cmd->ec_flags & ECMDF_CALLBACK) == 0);

	for (iopp = cmd->ec_handles; iopp < PFC_ARRAY_LIMIT(cmd->ec_handles);
	     iopp++) {
		extcmd_io_t	*iop = *iopp;

		if (PFC_EXPECT_FALSE(EXTCMD_IO_ISTYPE(iop, stream))) {
			/* Stream mode may causes deadlock. */
			err = EDEADLK;
			goto out;
		}
	}

	if (ops != NULL && ops->ecops_on_callback(cmd) != 0) {
		/* This function can not be called on the callback context. */
		err = EAGAIN;
		goto out;
	}

	/* Ensure that a new command can be executed. */
	err = extcmd_check_exec(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Create a child process, and execute external command on it. */
	cmd->ec_flags |= ECMDF_SYNC;
	err = extcmd_execute(ops, cmd, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	EXTCMD_LOCK(cmd);

	/* Obtain exit status of the child. */
	err = extcmd_child_wait(ops, cmd);
	cmd->ec_flags &= ~ECMDF_SYNC;

out:
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_start(pfc_extcmd_t ecmd, pid_t *pidp, pfc_ptr_t arg)
 *	Execute the external command with specifying arguments added by
 *	the call of pfc_extcmd_add_arguments().
 *
 *	Unlike pfc_extcmd_execute(), pfc_extcmd_start() never waits for the
 *	completion of the child process. The caller must reap child process
 *	by callback function specified to pfc_extcmd_create_async(), or the
 *	call of pfc_extcmd_wait().
 *
 *	`arg' is an arbitrary pointer to be passed to the callback function
 *	configured on the specified command context. It is simply ignored
 *	if the callback function is not configured.
 *
 * Calling/Exit State:
 *	Upon successful completion, the process ID of the created process is set
 *	to `*pidp' if `pidp' is not NULL, and then zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	It doesn't mean the command has been successfully completed even if
 *	pfc_extcmd_start() returns zero. The command status must be
 *	tested by the call of pfc_extcmd_get_status() or
 *	pfc_extcmd_get_signal() after reaping child.
 */
int
pfc_extcmd_start(pfc_extcmd_t ecmd, pid_t *pidp, pfc_ptr_t arg)
{
	const extcmd_ops_t	*ops = extcmd_ops;
	extcmd_t	*cmd;
	int		err;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	/* Ensure that a new async command can be started on this context. */
	err = extcmd_check_async(ops, cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Set callback argument. */
	cmd->ec_cbarg = arg;

	/* Create a child process, and execute external command on it. */
	return extcmd_execute(ops, cmd, pidp);

error:
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_wait(pfc_extcmd_t ecmd)
 *	Block the calling thread until the child process associated with the
 *	specified command context quits.
 *	After the successful completion of this function, another child process
 *	can be started on the specified context, by the call of
 *	pfc_extcmd_execute() or pfc_extcmd_start().
 *
 *	The command context which has callback function can not be specified
 *	to this function. See comments on pfc_extcmd_create_async().
 *
 * Calling/Exit State:
 *	Zero is returned if the termination of the child process is
 *	successfully detected.
 *
 *	ENOENT is returned if the specified context is not found.
 *	ECHILD is returned if no child process is associated with the command
 *	context, or another thread is already waiting.
 *	ESRCH is returned if no child process is found due to internal error.
 *	EPERM is returned if callback function is configured to the specified
 *	command context.
 *	EBADF is returned if the child process is created by the call of
 *	pfc_extcmd_execute().
 *	EAGAIN is returned if this function is called on the callback context.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_wait(pfc_extcmd_t ecmd)
{
	const extcmd_ops_t	*ops = extcmd_ops;
	extcmd_t	*cmd;
	int		err;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	/* Reject if the context has callback function. */
	if (PFC_EXPECT_FALSE(cmd->ec_callback != NULL)) {
		err = EPERM;
		goto out;
	}
	PFC_ASSERT((cmd->ec_flags & ECMDF_CALLBACK) == 0);

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_SYNC)) {
		/*
		 * Another thread is calling pfc_extcmd_execute() on this
		 * context.
		 */
		err = EBADF;
		goto out;
	}

	if (ops != NULL && ops->ecops_on_callback(cmd) != 0) {
		/* This function can not be called on the callback context. */
		err = EAGAIN;
		goto out;
	}

	/* Obtain exit status of the child. */
	err = extcmd_child_wait(ops, cmd);

out:
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_setio_retain(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
 *	Configure the command I/O handle to retain mode.
 *	This function reverts change made by the call of the following
 *	functions, so as to retain I/O handle of parent process.
 *
 *	`iodesc' specifies the target I/O handle. `iodesc' must be one of
 *	PFC_EXTCMD_STDIN, PFC_EXTCMD_STDOUT, and PFC_EXTCMD_STDERR which mean
 *	the standard input, the standard output, and the standard error output
 *	respectively.
 *
 * Calling/Exit State:
 *	Upon successful completion zero is returned.
 *
 *	ENOENT is returned if the command context associated with `ecmd' is
 *	not found.
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	EBUSY is returned if the child process associated with the command
 *	context is still running, or not yet reaped by the callback function
 *	or the call of pfc_extcmd_wait() call.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_setio_retain(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
{
	extcmd_t	*cmd;
	int		err;

	err = extcmd_setio_begin(ecmd, iodesc, &cmd);
	if (PFC_EXPECT_TRUE(err == 0)) {
		extcmd_setio_commit(cmd, iodesc, NULL);
		EXTCMD_UNLOCK(cmd);
	}

	return err;
}

/*
 * int
 * pfc_extcmd_setio_discard(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
 *	Configure the command I/O handle to discard mode, which discards any
 *	input or output.
 *
 *	`iodesc' specifies the target I/O handle. /dev/null is associated with
 *	the standard input, the standard output, or the standard error output
 *	if `iodesc' is respectively PFC_EXTCMD_STDIN, PFC_EXTCMD_STDOUT, or
 *	PFC_EXTCMD_STDERR.
 *
 * Calling/Exit State:
 *	Upon successful completion zero is returned.
 *
 *	ENOENT is returned if the command context associated with `ecmd' is
 *	not found.
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	EBUSY is returned if the child process associated with the command
 *	context is still running, or not yet reaped by the callback function
 *	or the call of pfc_extcmd_wait() call.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_setio_discard(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
{
	extcmd_t	*cmd;
	extcmd_io_t	*iop;
	int		err;

	err = extcmd_setio_begin(ecmd, iodesc, &cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = extcmd_io_discard_create(&iop);
	if (PFC_EXPECT_TRUE(err == 0)) {
		extcmd_setio_commit(cmd, iodesc, iop);
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_setio_file(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
 *			 const char *path, int oflags, mode_t mode)
 *	Configure the command I/O handle to file mode, which binds the
 *	specified file to the command I/O handle.
 *
 *	`iodesc' specifies the target I/O handle. `iodesc' must be one of
 *	PFC_EXTCMD_STDIN, PFC_EXTCMD_STDOUT, and PFC_EXTCMD_STDERR which mean
 *	the standard input, the standard output, and the standard error output
 *	respectively.
 *
 *	`path', `oflags', and `mode' are arguments to be passed to open(2)
 *	system call.
 *
 * Calling/Exit State:
 *	Upon successful completion zero is returned.
 *
 *	ENOENT is returned if the command context associated with `ecmd' is
 *	not found.
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	EBUSY is returned if the child process associated with the command
 *	context is still running, or not yet reaped by the callback function
 *	or the call of pfc_extcmd_wait() call.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_setio_file(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
		      const char *path, int oflags, mode_t mode)
{
	extcmd_t	*cmd;
	extcmd_io_t	*iop;
	int		err;

	err = extcmd_setio_begin(ecmd, iodesc, &cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = extcmd_io_file_create(&iop, iodesc, path, oflags, mode);
	if (PFC_EXPECT_TRUE(err == 0)) {
		extcmd_setio_commit(cmd, iodesc, iop);
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_setio_stream(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
 *	Configure the command I/O handle to stream mode, which associates
 *	a stdio stream with the command I/O handle.
 *	If I/O handle is configured by this function call, a stdio stream
 *	associated with the command I/O handle can be obtained by the call
 *	of pfc_extcmd_stream_get().
 *
 *	- If PFC_EXTCMD_STDIN is passed to `iodesc', data written to the stdio
 *	  stream is sent to the standard input of the child process.
 *	- If PFC_EXTCMD_STDOUT or PFC_EXTCMD_STDERR is passed to `iodesc',
 *	  data written to the standard output or the standard error output,
 *	  respectively, of the child process can be read by reading the stdio
 *	  stream.
 *
 * Calling/Exit State:
 *	Upon successful completion zero is returned.
 *
 *	ENOENT is returned if the command context associated with `ecmd' is
 *	not found.
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	EBUSY is returned if the child process associated with the command
 *	context is still running, or not yet reaped by the callback function
 *	or the call of pfc_extcmd_wait() call.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_setio_stream(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
{
	extcmd_t	*cmd;
	extcmd_io_t	*iop;
	int		err;

	err = extcmd_setio_begin(ecmd, iodesc, &cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = extcmd_io_stream_create(&iop, iodesc);
	if (PFC_EXPECT_TRUE(err == 0)) {
		extcmd_setio_commit(cmd, iodesc, iop);
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_setio_fd(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc, int fd)
 *	Configure the command I/O handle to fd mode, which associates a file
 *	descriptor specified by `fd' with the command I/O handle.
 *
 *	`iodesc' specifies the target I/O handle. `iodesc' must be one of
 *	PFC_EXTCMD_STDIN, PFC_EXTCMD_STDOUT, and PFC_EXTCMD_STDERR which mean
 *	the standard input, the standard output, and the standard error output
 *	respectively.
 *
 * Calling/Exit State:
 *	Upon successful completion zero is returned.
 *
 *	ENOENT is returned if the command context associated with `ecmd' is
 *	not found.
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	EBUSY is returned if the child process associated with the command
 *	context is still running, or not yet reaped by the callback function
 *	or the call of pfc_extcmd_wait() call.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_extcmd_setio_fd(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc, int fd)
{
	extcmd_t	*cmd;
	extcmd_io_t	*iop;
	int		err;

	err = extcmd_setio_begin(ecmd, iodesc, &cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = extcmd_io_fd_create(&iop, iodesc, fd);
	if (PFC_EXPECT_TRUE(err == 0)) {
		extcmd_setio_commit(cmd, iodesc, iop);
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_stream_get(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
 *			 FILE **filep)
 *	Obtain a stdio stream associated with the command.
 *	I/O descriptor specified by `iodesc' must be configured as stream mode
 *	by the call of pfc_extcmd_setio_stream() in advance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to a stdio stream is
 *	set to `*filep', and then zero is returned.
 *
 *	ENOENT is returned if the command context specified by `ecmd' is
 *	not found.
 *	ECHILD is returned if the child process is not started by the call of
 *	pfc_extcmd_start().
 *	EBADF is returned if the specified I/O descriptor is not configured
 *	as stream mode by the call of pfc_extcmd_setio_stream().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	NULL must not be passed to `filep'.
 */
int
pfc_extcmd_stream_get(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
		      FILE **filep)
{
	extcmd_io_stream_t	*iosp;
	extcmd_t	*cmd;
	FILE		*fp;
	int		err;

	err = extcmd_stream_get(ecmd, iodesc, &cmd, &iosp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	fp = iosp->ecis_stream;
	if (PFC_EXPECT_TRUE(fp != NULL)) {
		*filep = fp;
		err = 0;
	}
	else {
		err = ECHILD;
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_stream_close(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
 *	Close a stdio stream associated with the command.
 *	I/O descriptor specified by `iodesc' must be configured as stream mode
 *	by the call of pfc_extcmd_setio_stream() in advance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ENOENT is returned if the command context specified by `ecmd' is
 *	not found.
 *	ECHILD is returned if the child process is not started by the call of
 *	pfc_extcmd_start(), or the stream is already closed.
 *	EBADF is returned if the specified I/O descriptor is not configured
 *	as stream mode by the call of pfc_extcmd_setio_stream().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	After the call of pfc_extcmd_stream_close(), irrespective of result,
 *	any further access to the stdio stream obtained by the call of
 *	pfc_extcmd_stream_get() results in undefined behavior.
 */
int
pfc_extcmd_stream_close(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc)
{
	extcmd_io_stream_t	*iosp;
	extcmd_t	*cmd;
	FILE		*fp;
	int		err;

	err = extcmd_stream_get(ecmd, iodesc, &cmd, &iosp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	fp = iosp->ecis_stream;
	if (PFC_EXPECT_TRUE(fp != NULL)) {
		if (PFC_EXPECT_TRUE(fclose(fp) == 0)) {
			err = 0;
		}
		else if (PFC_EXPECT_FALSE((err = errno) != 0)) {
			/*
			 * fclose(3) did not set errno.
			 * This should never happen.
			 */
			err = EIO;
		}
		iosp->ecis_stream = NULL;
		iosp->ecis_parent_fd = -1;
	}
	else {
		err = ECHILD;
	}

	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * int
 * pfc_extcmd_get_status(pfc_extcmd_t ecmd)
 *	Return exit status of the last command executed on the specified
 *	command context.
 *
 * Calling/Exit State:
 *	Exit status of the last command is returned.
 *
 *	A negative error code is returned on error.
 *
 *	- PFC_EXTCMD_ERR_UNSPEC is returned if the last command has been
 *	  killed by a signal, or no command is executed on the context.
 *	- PFC_EXTCMD_ERR_NOENT is returned if the specified command context is
 *	  not found.
 */
int
pfc_extcmd_get_status(pfc_extcmd_t ecmd)
{
	extcmd_t	*cmd;
	int		ret;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return PFC_EXTCMD_ERR_NOENT;
	}

	if (PFC_EXPECT_FALSE((cmd->ec_flags & ECMDF_EXECUTED) == 0 ||
			     !WIFEXITED(cmd->ec_status))) {
		ret = PFC_EXTCMD_ERR_UNSPEC;
	}
	else {
		ret = WEXITSTATUS(cmd->ec_status);
	}

	EXTCMD_UNLOCK(cmd);

	return ret;
}

/*
 * int
 * pfc_extcmd_get_signal(pfc_extcmd_t ecmd)
 *	Return signal number which killed the last command executed on the
 *	specified command context.
 *
 * Calling/Exit State:
 *	If the last command was killed by a signal, a signal number is returned.
 *
 *	A negative error code is returned on error.
 *
 *	- PFC_EXTCMD_ERR_UNSPEC is returned if the last command has exited,
 *	  or no command is executed on the context.
 *	- PFC_EXTCMD_ERR_NOENT is returned if the specified command context is
 *	  not found.
 */
int
pfc_extcmd_get_signal(pfc_extcmd_t ecmd)
{
	extcmd_t	*cmd;
	int		ret;

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return PFC_EXTCMD_ERR_NOENT;
	}

	if (PFC_EXPECT_FALSE((cmd->ec_flags & ECMDF_EXECUTED) == 0 ||
			     !WIFSIGNALED(cmd->ec_status))) {
		ret = PFC_EXTCMD_ERR_UNSPEC;
	}
	else {
		ret = WTERMSIG(cmd->ec_status);
	}

	EXTCMD_UNLOCK(cmd);

	return ret;
}

/*
 * void
 * pfc_extcmd_enable_log(pfc_bool_t enable)
 *	Enable or disable extcmd internal logging.
 *
 *	If PFC_TRUE is passed to `enable', internal logging by PFC log
 *	architecture is enabled. It is up to the caller for initialization
 *	of PFC log.
 *
 *	If PFC_FALSE is passed to `enable', no internal log is dumped to
 *	PFC log architecture.
 *
 *	Internal logging is disabled by default.
 */
void
pfc_extcmd_enable_log(pfc_bool_t enable)
{
	uint32_t	newval = (enable) ? 1 : 0;

	(void)pfc_atomic_swap_uint32((uint32_t *)&extcmd_log_enabled, newval);
}

/*
 * void
 * __pfc_extcmd_setops(const extcmd_ops_t *ops)
 *	Set external command operations.
 *	This is used by pfcd to configure asynchronous process waiting.
 */
void
__pfc_extcmd_setops(const extcmd_ops_t *ops)
{
	(void)pfc_atomic_swap_ptr((pfc_ptr_t *)&extcmd_ops, (pfc_ptr_t)ops);
}

/*
 * void
 * __pfc_extcmd_destroy(extcmd_t *cmd)
 *	Destroy the specified command context.
 *	The caller must ensure that no child process is associated with the
 *	context.
 */
void
__pfc_extcmd_destroy(extcmd_t *cmd)
{
	extcmd_tree_t	*etp = &extcmd_tree;
	const char	**argv;
	int		argc;

	/* Invalidate command context ID. */
	EXTCMD_TREE_WRLOCK(etp);
	pfc_rbtree_remove_node(&etp->et_tree, &cmd->ec_node);
	EXTCMD_TREE_UNLOCK(etp);

	EXTCMD_LOCK(cmd);
	PFC_ASSERT(cmd->ec_pid == EXTCMD_PID_INVALID);
	PFC_ASSERT(cmd->ec_flags & ECMDF_DESTROYED);

	/* Ensure that no one waits on this context. */
	while (cmd->ec_nwaiters != 0) {
		EXTCMD_WAIT(cmd);
	}

	EXTCMD_UNLOCK(cmd);

	PFC_ASSERT_INT(pfc_mutex_destroy(&cmd->ec_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_destroy(&cmd->ec_cond), 0);
	PFC_ASSERT(cmd->ec_argc > EXTCMD_EXEC_NARGS);

	/* Dispose I/O handles. */
	EXTCMD_IO_CALL_FOREACH(cmd, ioops_dtor);

	argv = cmd->ec_argv + EXTCMD_EXEC_NARGS;
	for (argc = EXTCMD_EXEC_NARGS; argc < cmd->ec_argc;
	     argc++, argv++) {
		free((void *)*argv);
	}

	free((void *)cmd->ec_command);
	free((void *)cmd->ec_argv);
	extcmd_free(extcmd_ops, cmd);
}

/*
 * void
 * __pfc_extcmd_ioreset(extcmd_t *cmd)
 *	Reset all I/O handles associated with the command context specified by
 *	`cmd'.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
void
__pfc_extcmd_ioreset(extcmd_t *cmd)
{
	extcmd_io_reset(cmd);
}

/*
 * static pfc_cptr_t
 * extcmd_keyfunc(pfc_rbnode_t *node)
 *	Return Red-Black Tree key of the specified command context.
 *	`node' must be a pointer to ec_node in extcmd_t.
 */
static pfc_cptr_t
extcmd_keyfunc(pfc_rbnode_t *node)
{
	extcmd_t	*cmd = EXTCMD_NODE2PTR(node);

	return EXTCMD_KEY(cmd->ec_id);
}

/*
 * static extcmd_t *
 * extcmd_lookup(pfc_extcmd_t ecmd)
 *	Search for the external command context associated with the specified
 *	context ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to external command
 *	context is returned. Lock for the external command context is held
 *	on return.
 *	NULL is returned if not found.
 */
static extcmd_t *
extcmd_lookup(pfc_extcmd_t ecmd)
{
	extcmd_tree_t	*etp = &extcmd_tree;
	extcmd_t	*cmd;
	pfc_rbnode_t	*node;

	EXTCMD_TREE_RDLOCK(etp);

	node = pfc_rbtree_get(&etp->et_tree, EXTCMD_KEY(ecmd));
	if (PFC_EXPECT_TRUE(node != NULL)) {
		cmd = EXTCMD_NODE2PTR(node);

		/* Acquire command lock. */
		EXTCMD_LOCK(cmd);
	}
	else {
		cmd = NULL;
	}

	EXTCMD_TREE_UNLOCK(etp);

	return cmd;
}

/*
 * static int
 * extcmd_register(extcmd_t *cmd)
 *	Register the command context specified by `cmd' to the global context
 *	tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_register(extcmd_t *cmd)
{
	extcmd_tree_t	*etp = &extcmd_tree;
	pfc_extcmd_t	id, start;
	int		err;

	EXTCMD_TREE_WRLOCK(etp);

	id = extcmd_id_next + 1;
	if (PFC_EXPECT_FALSE(id == PFC_EXTCMD_INVALID)) {
		id++;
	}
	start = id;

	for (;;) {
		cmd->ec_id = id;
		err = pfc_rbtree_put(&etp->et_tree, &cmd->ec_node);
		if (PFC_EXPECT_TRUE(err == 0)) {
			extcmd_id_next = id;
			break;
		}

		PFC_ASSERT(err == EEXIST);

		/* This ID is not available. Try another ID. */
		id++;
		if (PFC_EXPECT_FALSE(id == PFC_EXTCMD_INVALID)) {
			id++;
		}
		if (PFC_EXPECT_FALSE(id == start)) {
			/* No ID is available. */
			err = ENFILE;
			break;
		}
	}

	EXTCMD_TREE_UNLOCK(etp);

	return err;
}

/*
 * static int
 * extcmd_expand_argv(extcmd_t *cmd)
 *	Expand argument vector.
 */
static int
extcmd_expand_argv(extcmd_t *cmd)
{
	const char	**argv;
	uint32_t	capacity = cmd->ec_capacity + EXTCMD_ARGV_INCR;

	if (PFC_EXPECT_FALSE(cmd->ec_capacity == EXTCMD_MAXARG)) {
		return E2BIG;
	}
	if (PFC_EXPECT_FALSE(capacity > EXTCMD_MAXARG)) {
		capacity = EXTCMD_MAXARG;
	}

	PFC_ASSERT(capacity > cmd->ec_capacity);
	argv = (const char **)realloc(cmd->ec_argv,
				      sizeof(const char *) * capacity);
	if (PFC_EXPECT_FALSE(argv == NULL)) {
		return ENOMEM;
	}

	cmd->ec_capacity = capacity;
	cmd->ec_argv = argv;

	return 0;
}

/*
 * static int
 * extcmd_resolve_path(extcmd_t *PFC_RESTRICT cmd,
 *			   const char *PFC_RESTRICT command)
 *	Search for an executable file in the command search path list.
 *
 * Calling/Exit State:
 *	Upon successful completion, path to an executable file is set to
 *	cmd->ec_command, and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	If a command file is found and it is not executable, the command
 *	search is terminated and an error number, returned by
 *	extcmd_is_executable(), is returned.
 */
static int
extcmd_resolve_path(extcmd_t *PFC_RESTRICT cmd,
		    const char *PFC_RESTRICT command)
{
	const char	**dirpp, *p;
	char	*path;
	size_t	clen, size;
	int	err;

	/*
	 * Ensure that the specified command doesn't contain slash
	 * character.
	 */
	for (clen = 0, p = command; *p != '\0'; clen++, p++) {
		if (PFC_EXPECT_FALSE(*p == '/')) {
			return EINVAL;
		}
	}
	if (PFC_EXPECT_FALSE(clen == 0)) {
		/*
		 * An empty string is specified. This will be expanded to
		 * a directory path.
		 */
		return EISDIR;
	}

	/* Allocate buffer to construct an absolute path to an executable. */
	size = EXTCMD_PATH_LENGTH;
	path = (char *)malloc(size);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		return ENOMEM;
	}

	/* Search for an executable in the command search path list. */
	err = ENOENT;
	for (dirpp = extcmd_path; *dirpp != NULL; dirpp++) {
		const char	*dir = *dirpp;
		size_t	plen, dlen = strlen(dir);

		plen = dlen + clen + 2;
		if (plen > size) {
			/* Expand path buffer. */
			free(path);
			size = plen;
			path = (char *)malloc(size);
			if (PFC_EXPECT_FALSE(path == NULL)) {
				err = ENOMEM;
				break;
			}
		}

		PFC_ASSERT_INT(snprintf(path, size, "%s/%s", dir, command),
			       plen - 1);
		err = extcmd_is_executable(path);
		if (PFC_EXPECT_TRUE(err == 0)) {
			cmd->ec_command = path;

			return 0;
		}
		if (PFC_EXPECT_FALSE(err != ENOENT)) {
			/* Stop command search. */
			break;
		}
	}

	PFC_ASSERT(err != 0);
	free(path);

	return err;
}

/*
 * static int
 * extcmd_is_executable(const char *path)
 *	Determine whether the specified file is an executable file which can be
 *	executed by the current user.
 */
static int
extcmd_is_executable(const char *path)
{
	struct stat	sbuf;
	mode_t	mode;

	if (PFC_EXPECT_FALSE(stat(path, &sbuf) != 0)) {
		return errno;
	}

	mode = sbuf.st_mode;
	if (PFC_EXPECT_FALSE(S_ISDIR(mode))) {
		return EISDIR;
	}
	else if (PFC_EXPECT_FALSE(!S_ISREG(mode))) {
		return EINVAL;
	}

	if (PFC_EXPECT_TRUE((mode & S_IXOTH) ||
			    (sbuf.st_uid == geteuid() && (mode & S_IXUSR)) ||
			    (sbuf.st_gid == getegid() && (mode & S_IXGRP)))) {
		return 0;
	}

	return EACCES;
}

/*
 * static int
 * extcmd_check_exec(extcmd_t *cmd)
 *	Ensure that a new command can be executed on the specified context.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static int
extcmd_check_exec(extcmd_t *cmd)
{
	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context will be destroyed. */
		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(EXTCMD_IS_RUNNING(cmd))) {
		/* A child process is running, or not yet reaped. */
		return EBUSY;
	}

	return 0;
}

/*
 * static int
 * extcmd_execute(const extcmd_ops_t *PFC_RESTRICT ops,
 *		  extcmd_t *PFC_RESTRICT cmd, pid_t *PFC_RESTRICT pidp)
 *	Execute external command on a new child process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If `pidp' is not NULL, the ID of the created process is set to `*pidp'.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the command lock.
 *	  The command lock is always released on return.
 *
 *	- The caller must check the command status atomically by the calling
 *	  extcmd_check_exec().
 *
 *	- This function returns zero if fork(2) is successfully completed.
 *	  So it can't detect exec(2) error.
 *
 *	- This function clears ECMDF_SYNC flag on error.
 */
static int
extcmd_execute(const extcmd_ops_t *PFC_RESTRICT ops,
	       extcmd_t *PFC_RESTRICT cmd, pid_t *PFC_RESTRICT pidp)
{
	pid_t		pid;
	int		err, ctrl[2];
	char		errmsg[EXTCMD_ERRMSG_SIZE], *ep;
	uint8_t		*dst;
	size_t		size;
	ssize_t		nbytes;

	/* Create control pipe with specifying blocking mode. */
	if (PFC_EXPECT_FALSE(pfc_pipe_open(ctrl, PFC_PIPE_CLOEXEC) != 0)) {
		err = errno;
		EXTCMD_LOG_ERROR("Failed to create control pipe: %s",
				 strerror(err));
		goto error;
	}

	/* Prepare I/O handles. */
	err = extcmd_io_prepare(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_ioreset;
	}

	/* Create a child process. */
	pid = fork();
	if (PFC_EXPECT_FALSE(pid == -1)) {
		err = errno;
		EXTCMD_LOG_ERROR("fork() failed: %s", strerror(err));
		goto error;
	}

	if (pid == 0) {
		int	cfd = ctrl[1];

		(void)close(ctrl[0]);

		/* Set up control pipe. */
		extcmd_child_bind_fd(cfd, cfd, EXTCMD_CTRL_FD, PFC_TRUE);

		/* Call child method for each I/O handle. */
		extcmd_io_child(cmd, EXTCMD_CTRL_FD);

		/* Call pre-exec handler. */
		if (ops != NULL) {
			ops->ecops_preexec();
		}

		/* Let extcmd_exec command execute the specified command. */
		execv(EXTCMD_EXEC_PATH, (char *const *)cmd->ec_argv);

		extcmd_child_error(EXTCMD_CTRL_FD, "execv() failed", errno);
		/* NOTREACHED */
	}

	/* Call parent method for each I/O handle. */
	EXTCMD_IO_CALL_FOREACH(cmd, ioops_parent);

	cmd->ec_flags &= ~(ECMDF_EXECUTED | ECMDF_LOST);
	cmd->ec_flags |= ECMDF_RUNNING;
	cmd->ec_pid = pid;
	if (pidp != NULL) {
		*pidp = pid;
	}

	EXTCMD_LOG_DEBUG("Execute command: %s: pid=%d", cmd->ec_command, pid);
	extcmd_log_arguments(cmd);

	EXTCMD_UNLOCK(cmd);

	(void)close(ctrl[1]);

	/* Read error number from child process. */
	dst = (uint8_t *)&err;
	size = sizeof(err);
	for (;;) {
		int	e;

		nbytes = read(ctrl[0], dst, size);
		if (PFC_EXPECT_TRUE(nbytes == 0)) {
			/* EOF has been detected. */
			if (PFC_EXPECT_FALSE(size != sizeof(err))) {
				EXTCMD_LOG_WARN("Unexpected error from child.");
			}
			goto out;
		}

		if (PFC_EXPECT_TRUE(nbytes > 0)) {
			if (size == (size_t)nbytes) {
				break;
			}
			dst += nbytes;
			size -= nbytes;
			continue;
		}

		e = errno;
		if (PFC_EXPECT_FALSE(e != EINTR)) {
			EXTCMD_LOG_WARN("Failed to read errno from child: %s",
					strerror(e));
			goto out;
		}
	}

	/* Read error message until EOF. */
	ep = errmsg;
	size = sizeof(errmsg) - 1;
	while ((nbytes = read(ctrl[0], ep, size)) != 0) {
		if (PFC_EXPECT_FALSE(nbytes == -1)) {
			if ((err = errno) == EINTR) {
				continue;
			}
			break;
		}

		ep += nbytes;
		size -= nbytes;
		if (size == 0) {
			break;
		}
	}
	errmsg[sizeof(errmsg) - 1 - size] = '\0';
	EXTCMD_LOG_WARN("exec failed: %s: %s", errmsg, strerror(err));

out:
	(void)close(ctrl[0]);

	/* Register a new child process. */
	if (ops != NULL) {
		ops->ecops_register(cmd);
	}

	return 0;

error_ioreset:
	extcmd_io_reset(cmd);

	(void)close(ctrl[0]);
	(void)close(ctrl[1]);

error:
	cmd->ec_flags &= ~ECMDF_SYNC;
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * static int
 * extcmd_check_async(const extcmd_ops_t *PFC_RESTRICT ops,
 *		      extcmd_t *PFC_RESTRICT cmd)
 *	Ensure that a new async command can be started on the command context
 *	specified by `cmd'.
 *
 *	If the callback is active on the context, the calling thread waits for
 *	completion of the callback.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 *	The command lock is always released on return.
 */
static int
extcmd_check_async(const extcmd_ops_t *PFC_RESTRICT ops,
		   extcmd_t *PFC_RESTRICT cmd)
{
	int	err;

	/* Ensure that a new command can be executed. */
	err = extcmd_check_exec(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if ((cmd->ec_flags & ECMDF_CALLBACK) == 0) {
		/* The callback is not active. */
		return 0;
	}

	/* The callback function is now being called. */
	PFC_ASSERT(ops != NULL);
	err = ops->ecops_on_callback(cmd);
	if (err == ELOOP) {
		/*
		 * This function is called on the same command callback
		 * context. Restarting the same command by callback is allowed.
		 */
		return 0;
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		/*
		 * This is an attempt to start another command on the callback
		 * context. The callback thread must not be blocked.
		 */
		PFC_ASSERT(err == EAGAIN);

		return err;
	}

	/* Wait for completion of callback. */
	do {
		EXTCMD_WAIT(cmd);
		if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
			break;
		}
	} while (cmd->ec_flags & ECMDF_CALLBACK);

	/*
	 * We need to check the command status again because the command
	 * lock has been released by EXTCMD_WAIT().
	 */
	return extcmd_check_exec(cmd);
}

/*
 * static void
 * extcmd_child_bind_fd(int ctrlfd, int fd, int target, pfc_bool_t do_close)
 *	Bind the file descriptor specified by `fd' to `target'.
 *
 *	`ctrlfd' is a file descriptor associated with control pipe.
 *	Error message will be dumped to it on error.
 *
 *	If `do_close' is PFC_TRUE, file descriptor specified by `fd' is
 *	closed.
 *
 * Calling/Exit State:
 *	The program quits on any error.
 */
static void
extcmd_child_bind_fd(int ctrlfd, int fd, int target, pfc_bool_t do_close)
{
	int	err;

	/* Turn the close-on-exec flag off. */
	err = pfc_set_cloexec(fd, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		extcmd_child_error(ctrlfd,
				   "Failed to clear close-on-exec flag",
				   err);
		/* NOTREACHED */
	}

	/* Reassign file descriptor. */
	if (PFC_EXPECT_FALSE(dup2(fd, target) == -1)) {
		extcmd_child_error(ctrlfd,
				   "Failed to duplicate file descriptor",
				   errno);
		/* NOTREACHED */
	}

	if (do_close) {
		(void)close(fd);
	}
}

/*
 * static void
 * extcmd_child_error(int fd, const char *msg, int err)
 *	Send error number and message to the file descriptor specified by `fd'.
 *
 * Remarks:
 *	This function is called on child process just before exec(),
 *	so only async-signal-safe functions can be used in this function.
 */
static void
extcmd_child_error(int fd, const char *msg, int err)
{
	size_t	len = strlen(msg);

	/* Send error number. */
	if (extcmd_child_write(fd, (const uint8_t *)&err, sizeof(err)) != 0 ||
	    extcmd_child_write(fd, (const uint8_t *)msg, len) != 0) {
		/* Dump error message to the standard error output. */
		(void)extcmd_child_write(2, (const uint8_t *)msg, len);
	}

	(void)close(fd);

	_exit(EXTCMD_ERRST_CHILD);
	/* NOTREACHED */
}

/*
 * static void
 * extcmd_child_write(int fd, const uint8_t *src, size_t size)
 *	Send data to the file descriptor specified by `fd'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is called on child process just before exec(),
 *	so only async-signal-safe functions can be used in this function.
 */
static int
extcmd_child_write(int fd, const uint8_t *src, size_t size)
{
	int	err = 0;

	do {
		ssize_t	nbytes = write(fd, src, size);

		if (PFC_EXPECT_TRUE(nbytes >= 0)) {
			src += nbytes;
			size -= nbytes;
		}
		else if (PFC_EXPECT_FALSE((err = errno) != EINTR)) {
			return err;
		}
		err = 0;
	} while (size > 0);

	return err;
}

/*
 * static int
 * extcmd_child_do_wait(const extcmd_ops_t *PFC_RESTRICT ops,
 *			extcmd_t *PFC_RESTRICT cmd)
 *	Block the calling thread until the process associated with `cmd' quits.
 *
 * Calling/Exit State:
 *	Upon successful completion, process status returned by the call of
 *	waitpid(2) is set to cmd->ec_status, and then zero is returned.
 *
 *	ECHILD is returned of no child process is associated with the command
 *	context, or another thread is already waiting.
 *	ESRCH is returned if no child process is found due to internal error.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 *	Note that it will be released for a while.
 */
static int
extcmd_child_do_wait(const extcmd_ops_t *PFC_RESTRICT ops,
		     extcmd_t *PFC_RESTRICT cmd)
{
	pid_t	pid;
	int	err;

	if (PFC_EXPECT_FALSE((cmd->ec_flags & ECMDF_RUNNING) == 0 ||
			     (pid = cmd->ec_pid) == EXTCMD_PID_INVALID)) {
		return ECHILD;
	}

	/*
	 * Turn the running flag off to guarantee that only one thread waits
	 * for this process to quit.
	 */
	cmd->ec_flags &= ~ECMDF_RUNNING;

	if (ops != NULL) {
		err = ops->ecops_wait(cmd);
		goto out;
	}

	PFC_ASSERT((cmd->ec_flags & (ECMDF_EXECUTED | ECMDF_LOST)) == 0);
	EXTCMD_UNLOCK(cmd);

	/* Below loop must quit with holding the command lock. */
	err = 0;
	for (;;) {
		int	status;

		if (PFC_EXPECT_FALSE(waitpid(pid, &status, 0) == -1)) {
			int	e = errno;

			if (PFC_EXPECT_TRUE(e == EINTR)) {
				continue;
			}

			/*
			 * This can happen if another thread calls wait(2)
			 * unexpectedly.
			 */
			PFC_ASSERT(e == ECHILD);
			EXTCMD_LOG_ERROR("%s: waitpid(%d) failed: %s",
					 cmd->ec_command, pid, strerror(e));
			err = ESRCH;
			EXTCMD_LOCK(cmd);
			cmd->ec_flags |= ECMDF_LOST;
			break;
		}

		if (PFC_EXPECT_TRUE(WIFEXITED(status) ||
				    WIFSIGNALED(status))) {
			EXTCMD_LOCK(cmd);
			EXTCMD_LOG_DEBUG("%s: Reaped: pid=%d, status=0x%x",
					 cmd->ec_command, pid, status);
			cmd->ec_status = status;
			cmd->ec_flags |= ECMDF_EXECUTED;
			break;
		}
	}

out:
	cmd->ec_pid = EXTCMD_PID_INVALID;
	extcmd_io_reset(cmd);

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* Wake up thread blocked in pfc_extcmd_destroy(). */
		EXTCMD_BROADCAST(cmd);
	}

	return err;
}

/*
 * static int
 * extcmd_setio_begin(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
 *		      extcmd_t **cmdp)
 *	Begin the transaction to configure the command I/O handle.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to the command context associated
 *	with `ecmd' is set to `*cmdp', and then zero is returned.
 *	Note that the command context is locked on return.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_setio_begin(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
		   extcmd_t **cmdp)
{
	extcmd_t	*cmd;
	int		err;

	if (PFC_EXPECT_FALSE(!EXTCMD_IODESC_IS_VALID(iodesc))) {
		/* Invalid I/O descriptor is specified. */
		return EINVAL;
	}

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already destroyed. */
		err = ENOENT;
		goto error;
	}

	if (PFC_EXPECT_FALSE(EXTCMD_IS_RUNNING(cmd))) {
		/* A child process is running, or not yet reaped. */
		err = EBUSY;
		goto error;
	}

	*cmdp = cmd;

	return 0;

error:
	EXTCMD_UNLOCK(cmd);

	return err;
}

/*
 * static void
 * extcmd_setio_commit(extcmd_t *PFC_RESTRICT cmd, pfc_extcmd_iodesc_t iodesc,
 *		       extcmd_io_t *PFC_RESTRICT iop)
 *	Install new command I/O handle.
 *	If NULL is passed to `iop', this function disposes I/O handle currently
 *	configured for `iodesc'.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_setio_commit(extcmd_t *PFC_RESTRICT cmd, pfc_extcmd_iodesc_t iodesc,
		    extcmd_io_t *PFC_RESTRICT iop)
{
	extcmd_io_t	*old;

	PFC_ASSERT(EXTCMD_IODESC_IS_VALID(iodesc));

	/* Dispose old handle. */
	old = cmd->ec_handles[iodesc];
	EXTCMD_IO_CALL(old, ioops_dtor);
	cmd->ec_handles[iodesc] = iop;
}

/*
 * static int
 * extcmd_io_prepare(extcmd_t *cmd)
 *	Call prepare operation for each I/O handle associated with the
 *	command context specified by `cmd'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static int
extcmd_io_prepare(extcmd_t *cmd)
{
	extcmd_io_t	**iopp;
	pfc_extcmd_iodesc_t	iodesc = PFC_EXTCMD_STDIN;

	PFC_ASSERT((uint32_t)iodesc == 0U);

	for (iopp = cmd->ec_handles; iopp < PFC_ARRAY_LIMIT(cmd->ec_handles);
	     iopp++, iodesc++) {
		const extcmd_ioops_t	*ioops;
		extcmd_io_t	*iop = *iopp;
		int		err;

		if (iop == NULL) {
			continue;
		}

		/* Reset this handle before preparation. */
		ioops = iop->eci_ops;
		if (ioops->ioops_reset != NULL) {
			ioops->ioops_reset(iop);
		}

		if (ioops->ioops_prepare == NULL) {
			continue;
		}

		err = ioops->ioops_prepare(iop, iodesc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			EXTCMD_LOG_ERROR("Failed to prepare I/O handle[%u]",
					 iodesc);

			return err;
		}
	}

	return 0;
}

/*
 * static void
 * extcmd_io_child(extcmd_t *cmd, int ctrlfd)
 *	Call child handler for each I/O handle associated with the command
 *	context specified by `cmd'.
 *
 *	`ctrlfd' is a file descriptor associated with control pipe.
 *	Error message will be dumped to it on error.
 *
 * Calling/Exit State:
 *	The program quits on any error.
 */
static void
extcmd_io_child(extcmd_t *cmd, int ctrlfd)
{
	extcmd_io_t	**iopp;
	pfc_extcmd_iodesc_t	iodesc = PFC_EXTCMD_STDIN;

	for (iopp = cmd->ec_handles; iopp < PFC_ARRAY_LIMIT(cmd->ec_handles);
	     iopp++, iodesc++) {
		const extcmd_ioops_t	*ioops;
		extcmd_io_t	*iop = *iopp;

		if (iop == NULL) {
			continue;
		}

		ioops = iop->eci_ops;
		if (ioops->ioops_child == NULL) {
			continue;
		}

		ioops->ioops_child(iop, iodesc, ctrlfd);
	}
}

/*
 * static void
 * extcmd_io_reset(extcmd_t *cmd)
 *	Reset all I/O handles associated with the command context specified
 *	by `cmd'.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_reset(extcmd_t *cmd)
{
	EXTCMD_IO_CALL_FOREACH(cmd, ioops_reset);
}

/*
 * static int
 * extcmd_io_discard_create(extcmd_io_t **iopp)
 *	Constructor of I/O handle which discards any input or output.
 *
 * Calling/Exit State:
 *	Upon successful completion, I/O handle is set to `*iopp', and then
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_io_discard_create(extcmd_io_t **iopp)
{
	extcmd_io_t	*iop;

	iop = (extcmd_io_t *)malloc(sizeof(*iop));
	if (PFC_EXPECT_FALSE(iop == NULL)) {
		return ENOMEM;
	}

	iop->eci_ops = &extcmd_ioops_discard;
	*iopp = iop;

	return 0;
}

/*
 * static void
 * extcmd_io_discard_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
 *			   int ctrlfd)
 *	Configure I/O handle to drop any input or output.
 *	This function is called just after fork(2) on child process.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_discard_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
			int ctrlfd)
{
	int	fd;

	/* Redirect to /dev/null. */
	fd = open("/dev/null", O_RDWR);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		extcmd_child_error(ctrlfd, "Failed to open /dev/null", errno);
		/* NOTREACHED */
	}

	extcmd_child_bind_fd(ctrlfd, fd, (int)iodesc, PFC_TRUE);
}

/*
 * static void
 * extcmd_io_discard_dtor(extcmd_io_t *iop)
 *	Destructor of I/O handle created by extcmd_io_discard_create().
 */
static void
extcmd_io_discard_dtor(extcmd_io_t *iop)
{
	free(iop);
}

/*
 * static int
 * extcmd_io_file_create(extcmd_io_t **PFC_RESTRICT iopp,
 *			 pfc_extcmd_iodesc_t iodesc,
 *			 const char *PFC_RESTRICT path, int oflags, mode_t mode)
 *	Constructor of I/O handle which represents redirection from/to the
 *	specified file.
 *
 * Calling/Exit State:
 *	Upon successful completion, I/O handle is set to `*iopp', and then
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_io_file_create(extcmd_io_t **PFC_RESTRICT iopp,
		      pfc_extcmd_iodesc_t iodesc,
		      const char *PFC_RESTRICT path, int oflags, mode_t mode)
{
	extcmd_io_file_t	*iofp;
	int	err;

	PFC_ASSERT(EXTCMD_IODESC_IS_VALID(iodesc));

	/* Verify arguments. */
	if (PFC_EXPECT_FALSE(path == NULL || *path == '\0')) {
		return EINVAL;
	}

#ifdef	PFC_HAVE_O_CLOEXEC
	oflags &= ~O_CLOEXEC;
#endif	/* PFC_HAVE_O_CLOEXEC */

	err = extcmd_verify_accmode(oflags, iodesc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	iofp = (extcmd_io_file_t *)malloc(sizeof(*iofp));
	if (PFC_EXPECT_FALSE(iofp == NULL)) {
		return ENOMEM;
	}

	iofp->ecif_path = strdup(path);
	if (PFC_EXPECT_FALSE(iofp->ecif_path == NULL)) {
		free(iofp);

		return ENOMEM;
	}

	iofp->ecif_io.eci_ops = &extcmd_ioops_file;
	iofp->ecif_oflags = oflags;
	iofp->ecif_mode = mode;

	*iopp = &iofp->ecif_io;

	return 0;
}

/*
 * static void
 * extcmd_io_file_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
 *			int ctrlfd)
 *	File redirection fork(2) handler for child process.
 *	This function is called just after fork(2) on child process.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_file_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc, int ctrlfd)
{
	extcmd_io_file_t	*iofp = EXTCMD_IO2FILE(iop);
	struct stat	sbuf;
	int	fd;

	/* Open the specified file. */
	fd = open(iofp->ecif_path, iofp->ecif_oflags, iofp->ecif_mode);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		extcmd_child_error(ctrlfd, "Failed to open file", errno);
		/* NOTREACHED */
	}

	/* Reject directory. */
	if (PFC_EXPECT_FALSE(fstat(fd, &sbuf) != 0)) {
		extcmd_child_error(ctrlfd, "fstat() failed", errno);
		/* NOTREACHED */
	}
	if (PFC_EXPECT_FALSE(S_ISDIR(sbuf.st_mode))) {
		extcmd_child_error(ctrlfd,
				   "Can not redirect from/to directory",
				   EISDIR);
		/* NOTREACHED */
	}

	extcmd_child_bind_fd(ctrlfd, fd, (int)iodesc, PFC_TRUE);
}

/*
 * static void
 * extcmd_io_file_dtor(extcmd_io_t *iop)
 *	Destructor of I/O handle created by extcmd_io_file_create().
 */
static void
extcmd_io_file_dtor(extcmd_io_t *iop)
{
	extcmd_io_file_t	*iofp = EXTCMD_IO2FILE(iop);

	free((void *)iofp->ecif_path);
	free(iofp);
}

/*
 * static int
 * extcmd_io_stream_create(extcmd_io_t **PFC_RESTRICT iopp,
 *			   pfc_extcmd_iodesc_t iodesc)
 *	Constructor of I/O handle which is connected to stdio stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, I/O handle is set to `*iopp', and then
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_io_stream_create(extcmd_io_t **PFC_RESTRICT iopp,
			pfc_extcmd_iodesc_t iodesc)
{
	extcmd_io_stream_t	*iosp;

	PFC_ASSERT(EXTCMD_IODESC_IS_VALID(iodesc));

	iosp = (extcmd_io_stream_t *)malloc(sizeof(*iosp));
	if (PFC_EXPECT_FALSE(iosp == NULL)) {
		return ENOMEM;
	}

	iosp->ecis_io.eci_ops = &extcmd_ioops_stream;
	iosp->ecis_stream = NULL;
	iosp->ecis_parent_fd = iosp->ecis_child_fd = -1;

	*iopp = &iosp->ecis_io;

	return 0;
}

/*
 * static int
 * extcmd_io_stream_prepare(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc)
 *	Prepare stdio stream redirection.
 *	This function is called just before the call of fork(2).
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static int
extcmd_io_stream_prepare(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc)
{
	extcmd_io_stream_t	*iosp = EXTCMD_IO2STREAM(iop);
	int		pp[2], err, parent_fd, child_fd;
	const char	*mode;

	/* Create pipe with close-on-exec flag. */
	if (PFC_EXPECT_FALSE(pfc_pipe_open(pp,  PFC_PIPE_CLOEXEC) != 0)) {
		err = errno;
		EXTCMD_LOG_ERROR("Failed to create pipe for fd[%u]: %s",
				 iodesc, strerror(err));

		return err;
	}

	if (iodesc == PFC_EXTCMD_STDIN) {
		/* For sending to the child process. */
		parent_fd = pp[1];
		child_fd = pp[0];
		mode = "w";
	}
	else {
		/* For receiving from the child process. */
		parent_fd = pp[0];
		child_fd = pp[1];
		mode = "r";
	}

	iosp->ecis_stream = fdopen(parent_fd, mode);
	if (PFC_EXPECT_FALSE(iosp->ecis_stream == NULL)) {
		err = errno;
		EXTCMD_LOG_ERROR("Failed to create stream for fd[%u]: %s",
				 iodesc, strerror(err));
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}

		(void)close(pp[0]);
		(void)close(pp[1]);

		return err;
	}

	iosp->ecis_parent_fd = parent_fd;
	iosp->ecis_child_fd = child_fd;

	return 0;
}

/*
 * static void
 * extcmd_io_stream_parent(extcmd_io_t *iop)
 *	The standard I/O stream redirection fork(2) handler for parent process.
 *	This function is called just after fork(2) on parent process.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_stream_parent(extcmd_io_t *iop)
{
	extcmd_io_stream_t	*iosp = EXTCMD_IO2STREAM(iop);

	/* Close for child side of the pipe. */
	(void)close(iosp->ecis_child_fd);
	iosp->ecis_child_fd = -1;
}

/*
 * static void
 * extcmd_io_stream_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
 *			 int ctrlfd)
 *	The standard I/O stream redirection fork(2) handler for child process.
 *	This function is called just after fork(2) on child process.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_stream_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
		       int ctrlfd)
{
	extcmd_io_stream_t	*iosp = EXTCMD_IO2STREAM(iop);

	/*
	 * Close for parent side of the pipe.
	 * We don't need to initialize ecis_parent_fd because reset operation
	 * is never called on child process.
	 */
	(void)close(iosp->ecis_parent_fd);

	extcmd_child_bind_fd(ctrlfd, iosp->ecis_child_fd, (int)iodesc,
			     PFC_TRUE);
}

/*
 * static void
 * extcmd_io_stream_reset(extcmd_io_t *iop)
 *	Reset I/O handler for standard I/O redirection.
 *	The stream created by the call of extcmd_io_stream_prepare() is closed.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_stream_reset(extcmd_io_t *iop)
{
	extcmd_io_stream_t	*iosp = EXTCMD_IO2STREAM(iop);

	if (iosp->ecis_child_fd != -1) {
		(void)close(iosp->ecis_child_fd);
		iosp->ecis_child_fd = -1;
	}

	if (iosp->ecis_stream != NULL) {
		(void)fclose(iosp->ecis_stream);
		iosp->ecis_stream = NULL;

		/* ecis_parent_fd is also closed by fclose(). */
		iosp->ecis_parent_fd = -1;
	}
	else if (iosp->ecis_parent_fd != -1) {
		(void)close(iosp->ecis_parent_fd);
		iosp->ecis_parent_fd = -1;
	}
}

/*
 * static void
 * extcmd_io_stream_dtor(extcmd_io_t *iop)
 *	Destructor of I/O handle created by extcmd_io_stream_create().
 */
static void
extcmd_io_stream_dtor(extcmd_io_t *iop)
{
	extcmd_io_stream_t	*iosp = EXTCMD_IO2STREAM(iop);

	extcmd_io_stream_reset(iop);
	free(iosp);
}

/*
 * static int
 * extcmd_io_fd_create(extcmd_io_t **iopp, pfc_extcmd_iodesc_t iodesc, int fd)
 *	Constructor of I/O handle which is bound to the specified file
 *	descriptor
 *
 * Calling/Exit State:
 *	Upon successful completion, I/O handle is set to `*iopp', and then
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
extcmd_io_fd_create(extcmd_io_t **iopp, pfc_extcmd_iodesc_t iodesc, int fd)
{
	extcmd_io_fd_t	*iodp;
	struct stat	sbuf;
	int		err, fflags;

	/* Suppress compiler's warning. */
	*iopp = NULL;

	/* Verify access mode of the specified file descriptor. */
	fflags = fcntl(fd, F_GETFL);
	if (PFC_EXPECT_FALSE(fflags == -1)) {
		return errno;
	}

	err = extcmd_verify_accmode(fflags, iodesc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Reject file descriptor associated with a directory. */
	if (PFC_EXPECT_FALSE(fstat(fd, &sbuf) != 0)) {
		return errno;
	}
	if (PFC_EXPECT_FALSE(S_ISDIR(sbuf.st_mode))) {
		return EISDIR;
	}

	iodp = (extcmd_io_fd_t *)malloc(sizeof(*iodp));
	if (PFC_EXPECT_FALSE(iodp == NULL)) {
		return ENOMEM;
	}

	iodp->ecid_io.eci_ops = &extcmd_ioops_fd;
	iodp->ecid_fd = fd;

	*iopp = &iodp->ecid_io;

	return 0;
}

/*
 * static void
 * extcmd_io_fd_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
 *			   int ctrlfd)
 *	Bind I/O handle to the file descriptor specified by user.
 *	This function is called just after fork(2) on child process.
 *
 * Remarks:
 *	This function must be called with holding the command lock.
 */
static void
extcmd_io_fd_child(extcmd_io_t *iop, pfc_extcmd_iodesc_t iodesc,
		   int ctrlfd)
{
	extcmd_io_fd_t	*iodp = EXTCMD_IO2FD(iop);

	extcmd_child_bind_fd(ctrlfd, iodp->ecid_fd, (int)iodesc, PFC_FALSE);
}

/*
 * static void
 * extcmd_io_fd_dtor(extcmd_io_t *iop)
 *	Destructor of I/O handle created by extcmd_io_fd_create().
 */
static void
extcmd_io_fd_dtor(extcmd_io_t *iop)
{
	extcmd_io_fd_t	*iodp = EXTCMD_IO2FD(iop);

	free(iodp);
}

/*
 * static int
 * extcmd_stream_get(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
 *		     extcmd_t **PFC_RESTRICT cmdp,
 *		     extcmd_io_stream_t **PFC_RESTRICT iospp)
 *	Obtain stream mode I/O handle associated with the specified command
 *	and I/O descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to the command instance
 *	associated with `ecmd' is set to `*cmdp', and a non-NULL pointer to
 *	stream I/O handle is set to `*iospp', and then zero is returned.
 *
 *	EINVAL is returned if the I/O descriptor specified by `iodesc' is
 *	invalid.
 *	ENOENT is returned if the command context specified by `ecmd' is
 *	not found.
 *	EBADF is returned if the specified I/O descriptor is not configured
 *	as stream mode by the call of pfc_extcmd_setio_stream().
 *
 * Remarks:
 *	The command lock is acquired on successful return. The caller is
 *	responsible for releasing the lock.
 */
static int
extcmd_stream_get(pfc_extcmd_t ecmd, pfc_extcmd_iodesc_t iodesc,
		  extcmd_t **PFC_RESTRICT cmdp,
		  extcmd_io_stream_t **PFC_RESTRICT iospp)
{
	extcmd_io_t	*iop;
	extcmd_t	*cmd;
	int		err;

	if (PFC_EXPECT_FALSE(!EXTCMD_IODESC_IS_VALID(iodesc))) {
		/* Invalid I/O descriptor is specified. */
		return EINVAL;
	}

	cmd = extcmd_lookup(ecmd);
	if (PFC_EXPECT_FALSE(cmd == NULL)) {
		return ENOENT;
	}

	if (PFC_EXPECT_FALSE(cmd->ec_flags & ECMDF_DESTROYED)) {
		/* This context is already destroyed. */
		err = ENOENT;
		goto error;
	}

	iop = cmd->ec_handles[iodesc];
	if (PFC_EXPECT_FALSE(!EXTCMD_IO_ISTYPE(iop, stream))) {
		err = EBADF;
		goto error;
	}

	*cmdp = cmd;
	*iospp = EXTCMD_IO2STREAM(iop);

	return 0;

error:
	EXTCMD_UNLOCK(cmd);

	return err;
}
