/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * channel.c - IPC server channel management.
 */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pfc/util.h>
#include <pfc/socket.h>
#include "ipcsrv_impl.h"

/*
 * Permission of channel lock file.
 */
#define	IPCSRV_LKFILE_PERM		(S_IRUSR | S_IWUSR)	/* 0600 */

/*
 * Suffix of channel lock file.
 */
static const char	ipcsrv_lkfile_suffix[] = ".lock";

#define	IPCSRV_LKFILE_SUFFIX_LEN	(sizeof(ipcsrv_lkfile_suffix) - 1)

/*
 * Channel lock handle.
 */
typedef struct {
	char		*cl_path;		/* lock file path */
	pfc_flock_t	cl_lock;		/* lock file handle */
} ch_lockfile_t;

/*
 * Internal prototypes.
 */
static int	ipcsrv_register(const char *channel, int shutfd,
				ipc_srvops_t *ops);
static int	ipcsrv_init(ipc_channel_t *PFC_RESTRICT chp,
			    const char *PFC_RESTRICT channel, int shutfd,
			    ipc_srvops_t *ops);
static int	ipcsrv_shutfd_init(ipc_channel_t *chp, int shutfd);
static void	ipcsrv_apply(ipc_channel_t *PFC_RESTRICT chp,
			     pfc_bool_t do_init, mode_t *PFC_RESTRICT modep);
static int	ipcsrv_open(ipc_channel_t *chp, mode_t mode);
static int	ipcsrv_channel_lock(ipc_channel_t *chp, ch_lockfile_t *lkp,
				    const char *spath, uint32_t spathlen);
static void	ipcsrv_channel_unlock(ch_lockfile_t *lkp);
static int	ipcsrv_clean_socket(ipc_channel_t *PFC_RESTRICT chp,
				    csockaddr_un_t *PFC_RESTRICT unp,
				    uint32_t addrlen);
static int	ipcsrv_check_socket(ipc_channel_t *PFC_RESTRICT chp,
				    csockaddr_un_t *PFC_RESTRICT unp,
				    uint32_t addrlen);
static int	ipcsrv_epoll_init(ipc_channel_t *chp);
static int	ipcsrv_epoll_add(ipc_channel_t *PFC_RESTRICT chp, int fd,
				 uint32_t events,
				 pfc_ephdlr_t *PFC_RESTRICT ephp);
static void	ipcsrv_setops(ipc_channel_t *chp, ipc_srvops_t *ops);

static pfc_cptr_t	ipcsrv_handler_getkey(pfc_rbnode_t *node);

#define	IPC_HANDLER_TREE_DECL						\
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, ipcsrv_handler_getkey)

#define	IPC_EPOLL_HANDLER_DECL(func)				\
	PFC_EPOLL_HANDLER_INITIALIZER(pfc_ipcsrv_epoll_##func)

/*
 * IPC channel.
 */
ipc_channel_t	ipc_channel PFC_ATTR_HIDDEN = {
	.ich_option		= IPC_OPTION_INITIALIZER,
	.ich_mutex		= PFC_MUTEX_INITIALIZER,
	.ich_hlock		= PFC_RWLOCK_INITIALIZER,
	.ich_ep_shutdown	= IPC_EPOLL_HANDLER_DECL(shutdown),
	.ich_ep_listener	= IPC_EPOLL_HANDLER_DECL(listener),
	.ich_handlers		= IPC_HANDLER_TREE_DECL,
	.ich_sessions		= PFC_LIST_INITIALIZER(ipc_channel.
						       ich_sessions),
	.ich_evqueues		= PFC_LIST_INITIALIZER(ipc_channel.
						       ich_evqueues),
	.ich_listener		= -1,
	.ich_epfd		= -1,
	.ich_shutfd		= -1,
	.ich_killfd		= -1,
	.ich_evserial_next	= IPC_EVENT_SERIAL_INITIAL,
	.ich_ops		= {
		.isvops_thread_create = pfc_ipc_thread_create,
	},
};

/*
 * int
 * pfc_ipcsrv_init(const char *channel, const pfc_ipcsrvops_t *ops)
 *	Initialize IPC service channel.
 *
 *	`channel' is an IPC channel name to be registered.
 *
 *	`ops' is a pointer to static pfc_ipcsrvops_t buffer, which determines
 *	behavior of the IPC server. Default operation is used if NULL is
 *	specified to `ops'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Only one IPC channel can be registered per a process.
 */
int
pfc_ipcsrv_init(const char *channel, const pfc_ipcsrvops_t *ops)
{
	return ipcsrv_register(channel, -1, ops);
}

/*
 * int
 * pfc_ipcsrv_sysinit(const char *channel, int shutfd,
 *		      const pfc_ipcsrvops_t *ops)
 *	Initialize IPC service channel.
 *	Only the PFC daemon can call this function.
 *
 *	`channel' is an IPC channel name to be registered.
 *
 *	If -1 is specified to `shutfd', an internal pipe is created, and it
 *	is used for shutdown notification. If not, `shutfd' must be a readable
 *	file descriptor which notifies the end of IPC service. If a read
 *	event is detected on `shutfd', IPC service is closed immediately.
 *	Note that libpfc_ipcsrv never closes the file descriptor specified
 *	by `shutfd'.
 *
 *	`ops' is a pointer to static pfc_ipcsrvops_t buffer, which determines
 *	behavior of the IPC server. Default operation is used if NULL is
 *	specified to `ops'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Only one IPC channel can be registered per a process.
 */
int
pfc_ipcsrv_sysinit(const char *PFC_RESTRICT channel, int shutfd,
		   const pfc_ipcsrvops_t *ops)
{
	return ipcsrv_register(channel, shutfd, ops);
}

/*
 * void
 * pfc_ipcsrv_reload(void)
 *	Reload dynamic configurations for IPC server library.
 */
void
pfc_ipcsrv_reload(void)
{
	ipc_channel_t	*chp = &ipc_channel;
	int		err;

	IPCCH_LOCK(chp);

	/* Ensure the IPC channel is initialized. */
	err = pfc_ipcsrv_channel_check(chp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Update IPC channel configuration. */
		ipcsrv_apply(chp, PFC_FALSE, NULL);
	}

	IPCCH_UNLOCK(chp);
}

/*
 * ipc_channel_t *
 * pfc_ipcsrv_getchannel(void)
 *	Return a pointer to ipc_channel.
 *
 * Remarks:
 *	This is non-public interface.
 */
ipc_channel_t *
pfc_ipcsrv_getchannel(void)
{
	return &ipc_channel;
}

/*
 * void
 * pfc_ipcsrv_destroy(ipc_channel_t *chp)
 *	Destroy global IPC channel instance.
 *
 * Remarks:
 *	This function must be called with holding the channel lock.
 *	It will be released on return.
 */
void
pfc_ipcsrv_destroy(ipc_channel_t *chp)
{
	PFC_ASSERT(chp->ich_refcnt == 1);
	chp->ich_refcnt = 0;

	IPCSRV_LOG_INFO("%s: IPC channel has been destroyed.",
			IPCCH_NAME(chp));

	if (chp->ich_epfd != -1) {
		PFC_IPCSRV_CLOSE(chp->ich_epfd);
		chp->ich_epfd = -1;
	}

	if (chp->ich_name != NULL) {
		pfc_refptr_put(chp->ich_name);
		chp->ich_name = NULL;
	}

	if (chp->ich_sockpath != NULL) {
		pfc_refptr_put(chp->ich_sockpath);
		chp->ich_sockpath = NULL;
	}

	if (chp->ich_listener != -1) {
		PFC_IPCSRV_CLOSE(chp->ich_listener);
		chp->ich_listener = -1;
	}

	if (chp->ich_flags & IPCCHF_SELFPIPE) {
		if (chp->ich_killfd != -1) {
			PFC_IPCSRV_CLOSE(chp->ich_killfd);
		}
		if (chp->ich_shutfd != -1) {
			PFC_IPCSRV_CLOSE(chp->ich_shutfd);
		}
	}

	chp->ich_killfd = -1;
	chp->ich_shutfd = -1;

	/* Call additional destructor. */
	if (chp->ich_dtor != NULL) {
		chp->ich_dtor(chp);
	}

	IPCCH_UNLOCK(chp);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_shutdown(ipc_channel_t *chp)
 *	Shutdown the IPC service on the specified channel.
 *
 *	UNIX domain socket file for the given channel is removed.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_shutdown(ipc_channel_t *chp)
{
	const char	*path;
	pfc_refptr_t	*rpath;
	pfc_list_t	*elem;
	int		listener;

	IPCCH_LOCK(chp);

	listener = chp->ich_listener;
	if (PFC_EXPECT_FALSE(listener == -1)) {
		goto out;
	}

	IPCSRV_LOG_INFO("%s: Shutting down the IPC channel.", IPCCH_NAME(chp));
	PFC_IPCSRV_CLOSE(listener);
	chp->ich_listener = -1;

	/* Set shutdown flag to all event queues. */
	PFC_LIST_FOREACH(&chp->ich_evqueues, elem) {
		ipc_evqueue_t	*evq = IPC_EVQUEUE_LIST2PTR(elem);

		pfc_ipcsrv_evqueue_setflags(evq, IPCSRVF_EVQ_SHUTDOWN);
	}

	/* Shutdown all event delivery descriptors. */
	pfc_ipcsrv_evdesc_shutdown();

	/* Remove all IPC service handlers. */
	pfc_ipcsrv_remove_all();

	rpath = chp->ich_sockpath;
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		/* This should never happen. */
		goto out;
	}

	/* Unlink UNIX domain socket file of this channel. */
	path = pfc_refptr_string_value(rpath);
	if (PFC_EXPECT_FALSE(unlink(path) != 0 && errno == EACCES)) {
		(void)chmod(path, 0755);
		(void)unlink(path);
	}

out:
	IPCCH_UNLOCK(chp);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipcsrv_channel_cleanup(void)
 *	Remove the UNIX domain socket file associated with the registered IPC
 *	channel by force.
 *
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipcsrv_channel_cleanup(void)
{
	ipc_channel_t	*chp = &ipc_channel;
	pfc_refptr_t	*rpath;
	int		lock_err;

	lock_err = IPCCH_TRYLOCK(chp);

	rpath = chp->ich_sockpath;
	if (PFC_EXPECT_TRUE(rpath != NULL && chp->ich_listener != -1)) {
		/*
		 * Unlink UNIX domain socket file of unregistered IPC
		 * channel.
		 */
		(void)unlink(pfc_refptr_string_value(rpath));

		/*
		 * Refptr string must not be released here because another
		 * thread may touch them unexpectedly.
		 */
	}

	if (lock_err == 0) {
		IPCCH_UNLOCK(chp);
	}
}

/*
 * static int
 * ipcsrv_register(const char *channel, int shutfd, ipc_srvops_t *ops)
 *	Initialize IPC service channel.
 *
 *	`shutfd' must be a readable file descriptor which notifies the end of
 *	IPC service. If a read event is detected on `shutfd', IPC service
 *	is closed immediately.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- Only one IPC channel can be registered per a process.
 *
 *	- This function will remove UNIX domain socket file associated with
 *	  the specified channel. So specifying channel name used by another
 *	  process results in undefined behavior.
 */
static int
ipcsrv_register(const char *channel, int shutfd, ipc_srvops_t *ops)
{
	ipc_channel_t	*chp = &ipc_channel;
	mode_t		mode;
	int		err;

	/* Load IPC struct information. */
	err = pfc_ipc_struct_load();
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Initialize global IPC channel instance. */
	err = ipcsrv_init(chp, channel, shutfd, ops);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Apply IPC channel configuration. */
	ipcsrv_apply(chp, PFC_TRUE, &mode);

	/* Open IPC channel socket. */
	err = ipcsrv_open(chp, mode);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize IPC event delivery descriptor. */
	pfc_ipcsrv_evdesc_init();

	IPCSRV_LOG_INFO("%s: IPC channel has been registered.", channel);

	return 0;

error:
	IPCCH_LOCK(chp);
	pfc_ipcsrv_destroy(chp);

	return err;
}

/*
 * static int
 * ipcsrv_init(ipc_channel_t *PFC_RESTRICT chp,
 *	       const char *PFC_RESTRICT channel, int shutfd, ipc_srvops_t *ops)
 *	Initialize IPC channel instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_init(ipc_channel_t *PFC_RESTRICT chp, const char *PFC_RESTRICT channel,
	    int shutfd, ipc_srvops_t *ops)
{
	pfc_refptr_t	*rname;
	int		err;

	if (PFC_EXPECT_FALSE(channel == NULL)) {
		return EINVAL;
	}

	rname = pfc_refptr_string_create(channel);
	if (PFC_EXPECT_FALSE(rname == NULL)) {
		IPCSRV_LOG_ERROR("%s: No memory for channel name.", channel);

		return ENOMEM;
	}

	IPCCH_LOCK(chp);

	if (PFC_EXPECT_FALSE(chp->ich_flags & IPCCHF_SHUTDOWN)) {
		IPCSRV_LOG_ERROR("%s: IPC channel is already finalized.",
				 channel);
		IPCCH_UNLOCK(chp);
		err = ECANCELED;
		goto error_rname;
	}

	if (PFC_EXPECT_FALSE(chp->ich_name != NULL)) {
		IPCSRV_LOG_ERROR("%s: IPC channel is already initialized.",
				 channel);
		IPCCH_UNLOCK(chp);

		/* Don't destroy IPC channel instance. */
		err = EBUSY;
		goto error_rname;
	}

	chp->ich_name = rname;
	IPCCH_UNLOCK(chp);

	err = pfc_cond_init(&chp->ich_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		goto error;
	}

	/* Initialize shutdown notification FD. */
	err = ipcsrv_shutfd_init(chp, shutfd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	chp->ich_refcnt = 1;

	ipcsrv_setops(chp, ops);

	return 0;

error:
	IPCCH_LOCK(chp);
	chp->ich_name = NULL;
	IPCCH_UNLOCK(chp);

error_rname:
	pfc_refptr_put(rname);

	return err;
}

/*
 * static int
 * ipcsrv_shutfd_init(ipc_channel_t *chp, int shutfd)
 *	Initialize shutdown notification file descriptor, which is used to
 *	shutdown IPC service on the given channel.
 */
static int
ipcsrv_shutfd_init(ipc_channel_t *chp, int shutfd)
{
	int	err;

	if (shutfd == -1) {
		int	pipefds[2];

		/*
		 * Create an internal pipe, and use it for shutdown
		 * notification.
		 */
		err = pfc_pipe_open(pipefds, PFC_PIPE_CLOEXEC_NB);
		if (PFC_EXPECT_FALSE(err != 0)) {
			err = errno;
			IPCSRV_LOG_ERROR("Unable to create shutdown pipe: %s",
					 strerror(err));
			goto error;
		}

		chp->ich_shutfd = pipefds[0];
		chp->ich_killfd = pipefds[1];
		chp->ich_flags |= IPCCHF_SELFPIPE;

		return 0;
	}

	/* Ensure that the given shutdown FD is valid. */
	err = pfc_ipc_checkshutfd(shutfd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	chp->ich_shutfd = shutfd;
	chp->ich_killfd = -1;
	chp->ich_flags &= ~IPCCHF_SELFPIPE;

	return 0;

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static void
 * ipcsrv_apply(ipc_channel_t *PFC_RESTRICT chp, pfc_bool_t do_init,
 *		mode_t *PFC_RESTRICT modep)
 *	Apply IPC channel configuration.
 *
 *	This function sets permission bits for the socket file to the buffer
 *	pointed by `modep' unless it is NULL.
 *
 *	If `do_init' is PFC_TRUE and it fails to load the configuration file,
 *	default configuration is applied to the IPC channel.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock
 *	except for initialization.
 */
static void
ipcsrv_apply(ipc_channel_t *PFC_RESTRICT chp, pfc_bool_t do_init,
	     mode_t *PFC_RESTRICT modep)
{
	ipc_chconf_t	chconf;
	const char	*channel = IPCCH_NAME(chp);
	int		err;

	/* Obtain IPC channel configuration for server process. */
	err = pfc_ipc_conf_getsrvconf(channel, &chconf);
	if (PFC_EXPECT_FALSE(!do_init && err != 0)) {
		IPCSRV_LOG_ERROR("%s: Failed to reload configuration file: %s",
				 channel, strerror(err));

		return;
	}

	chp->ich_max_clients = chconf.icc_max_clients;
	chp->ich_max_sessions = chconf.icc_max_sessions;
	chp->ich_timeout = chconf.icc_timeout;

	/* max_clients must not exceed max_sessions. */
	if (PFC_EXPECT_FALSE(chp->ich_max_sessions < chp->ich_max_clients)) {
		IPCSRV_LOG_WARN("max_sessions(%u) must be greater than or "
				"equal max_clients(%u).",
				chp->ich_max_sessions, chp->ich_max_clients);
		chp->ich_max_sessions = chp->ich_max_clients;
	}

	IPCSRV_LOG_INFO("max_clients=%u, max_sessions=%u, timeout=%u, "
			"perm=0%o",
			chp->ich_max_clients, chp->ich_max_sessions,
			chp->ich_timeout, chconf.icc_permission);

	if (modep != NULL) {
		*modep = chconf.icc_permission;
	}
}

/*
 * static int
 * ipcsrv_open(ipc_channel_t *chp, mode_t mode)
 *	Open a listener socket for the IPC service.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_open(ipc_channel_t *chp, mode_t mode)
{
	sockaddr_un_t	un;
	const char	*channel = IPCCH_NAME(chp);
	pfc_refptr_t	*path;
	socklen_t	addrlen;
	ch_lockfile_t	lk;
	uint32_t	plen;
	int		err, sock, ret;

	/* Initialize IPC system directories. */
	err = pfc_ipc_conf_dirinit();
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to initialize system directories: %s",
				 strerror(err));

		return err;
	}

	/* Determine UNIX domain socket path and temporary directory. */
	un.sun_family = AF_UNIX;
	err = pfc_ipc_conf_getpath(channel, &path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to determine socket file path: %s",
				 strerror(err));

		return err;
	}

	plen = pfc_strlcpy(un.sun_path, pfc_refptr_string_value(path),
			   sizeof(un.sun_path));
	PFC_ASSERT(plen < sizeof(un.sun_path));
	pfc_refptr_put(path);

	/* Preserve UNIX domain socket file path. */
	chp->ich_sockpath = pfc_refptr_string_create(un.sun_path);
	if (PFC_EXPECT_FALSE(chp->ich_sockpath == NULL)) {
		IPCSRV_LOG_ERROR("No memory for socket file path.");

		return ENOMEM;
	}

	/* Create a socket for the IPC service. */
	sock = pfc_sock_open(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		IPCSRV_LOG_ERROR("Failed to create IPC server socket: %s",
				 strerror(err));
		goto error;
	}

	chp->ich_listener = sock;

	/* Acquire the IPC channel lock. */
	err = ipcsrv_channel_lock(chp, &lk, un.sun_path, plen);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Clean up bogus socket file. */
	addrlen = PFC_SOCK_UNADDR_SIZE(plen);
	err = ipcsrv_clean_socket(chp, &un, addrlen);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_unlock;
	}

	/* Make the listener socket visible to the filesystem.*/
	ret = bind(sock, (csockaddr_t *)&un, addrlen);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPCSRV_LOG_ERROR("Failed to bind listener socket: %s",
				 strerror(err));
		goto error_unlock;
	}

	/* Make this socket as a listener. */
	ret = listen(sock, chp->ich_max_clients);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPCSRV_LOG_ERROR("listen(backlog=%u) failed: %s",
				 chp->ich_max_clients, strerror(err));
		goto error_bind;
	}

	/* Set permission bits for the socket file. */
	ret = chmod(un.sun_path, mode);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPCSRV_LOG_ERROR("chmod(0%o) failed: %s", mode, strerror(err));
		goto error_bind;
	}

	/* Initialize epoll(7) instance to detect events from the socket. */
	err = ipcsrv_epoll_init(chp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_bind;
	}

	IPCSRV_LOG_VERBOSE("%s: socket=%s",
			   channel, un.sun_path);

	ipcsrv_channel_unlock(&lk);

	return 0;

error_bind:
	(void)unlink(un.sun_path);

error_unlock:
	ipcsrv_channel_unlock(&lk);

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static int
 * ipcsrv_channel_lock(ipc_channel_t *chp, ch_lockfile_t *lkp,
 *		       const char *spath, uint32_t spathlen)
 *	Acquire IPC channel lock.
 *	IPC channel lock is implemented by file advisory lock.
 *
 *	`spath' must be a pointer to IPC channel socket file path, and
 *	`spathlen' must be its length.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	After successful return of this function call, the caller must call
 *	ipcsrv_channel_unlock().
 */
static int
ipcsrv_channel_lock(ipc_channel_t *chp, ch_lockfile_t *lkp, const char *spath,
		    uint32_t spathlen)
{
	uint32_t	lkpathlen = spathlen + IPCSRV_LKFILE_SUFFIX_LEN + 1;
	int		err;

	/* Construct lock file path. */
	lkp->cl_path = (char *)malloc(lkpathlen);
	if (PFC_EXPECT_FALSE(lkp->cl_path == NULL)) {
		IPCSRV_LOG_ERROR("No memory for channel lock file path.");

		return ENOMEM;
	}

	PFC_ASSERT_INT(snprintf(lkp->cl_path, lkpathlen, "%s%s", spath,
				ipcsrv_lkfile_suffix),
		       (int)(lkpathlen - 1));

	/* Acquire writer lock of the lock file. */
	err = pfc_flock_open(&lkp->cl_lock, lkp->cl_path, O_RDWR | O_CREAT,
			     IPCSRV_LKFILE_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPCSRV_LOG_ERROR("Failed to open channel lock file: %s",
				 strerror(err));
		goto error;
	}

	for (;;) {
		err = pfc_flock_wrlock(lkp->cl_lock, NULL);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/* Succeeded. */
			return 0;
		}

		if (PFC_EXPECT_FALSE(err != EINTR)) {
			IPCSRV_LOG_ERROR("Failed to acquire channel lock: %s",
					 strerror(err));
			break;
		}
	}

	PFC_ASSERT_INT(pfc_flock_close(lkp->cl_lock), 0);

error:
	free(lkp->cl_path);

	return err;
}

/*
 * static void
 * ipcsrv_channel_unlock(ch_lockfile_t *lkp)
 *	Release IPC channel lock, and free up resources held by the specified
 *	lock handle.
 */
static void
ipcsrv_channel_unlock(ch_lockfile_t *lkp)
{
	PFC_ASSERT_INT(pfc_flock_close(lkp->cl_lock), 0);
	free(lkp->cl_path);
}

/*
 * static int
 * ipcsrv_clean_socket(ipc_channel_t *PFC_RESTRICT chp,
 *		       csockaddr_un_t *PFC_RESTRICT unp, uint32_t addrlen)
 *	Clean up bogus IPC channel socket file.
 *
 *	If the IPC channel socket file specified by `unp' exists,
 *	this function checks whether another process listens on the socket
 *	file.
 *
 *	`unp' must be a pointer to valid UNIX domain socket address which
 *	contains the IPC channel socket file path.
 *	`addrlen' must be the length of the socket address specified by `unp'.
 *
 * Calling/Exit State:
 *	Zero is returned if no process listens on the socket file.
 *	EADDRINUSE is returned if another process listens on the socket file.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock
 *	by the call of ipcsrv_channel_lock().
 */
static int
ipcsrv_clean_socket(ipc_channel_t *PFC_RESTRICT chp,
		    csockaddr_un_t *PFC_RESTRICT unp, uint32_t addrlen)
{
	struct stat	sbuf;
	int		ret, err;

	/* Check status of the IPC channel socket file. */
	ret = lstat(unp->sun_path, &sbuf);
	if (ret != 0) {
		err = errno;
		if (PFC_EXPECT_TRUE(err == ENOENT)) {
			/* The IPC channel socket file does not exist. */
			return 0;
		}

		IPCSRV_LOG_ERROR("Failed to obtain socket file status: %s",
				 strerror(err));
		goto error;
	}

	if (PFC_EXPECT_TRUE(S_ISSOCK(sbuf.st_mode))) {
		/*
		 * Check whether another process listens on this channel
		 * or not.
		 */
		err = ipcsrv_check_socket(chp, unp, addrlen);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	/* Try to unlink bogus IPC channel socket file. */
	if (!S_ISLNK(sbuf.st_mode)) {
		(void)chmod(unp->sun_path, 0755);
	}
	err = pfc_rmpath(unp->sun_path);
	if (PFC_EXPECT_FALSE(err != 0 && err != ENOENT)) {
		IPCSRV_LOG_ERROR("Failed to unlink socket file: %s",
				 strerror(err));
		goto error;
	}

	IPCSRV_LOG_NOTICE("Bogus IPC channel socket file was removed.");

	return 0;

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static int
 * ipcsrv_check_socket(ipc_channel_t *PFC_RESTRICT chp,
 *		       csockaddr_un_t *PFC_RESTRICT unp, uint32_t addrlen)
 *	Check whether another process listens on the IPC channel socket file
 *	or not.
 *
 *	`unp' must be a pointer to valid UNIX domain socket address which
 *	contains the IPC channel socket file path.
 *	`addrlen' must be the length of the socket address specified by `unp'.
 *
 * Calling/Exit State:
 *	Zero is returned if no process listens on the socket file.
 *	EADDRINUSE is returned if another process listens on the socket file.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC channel lock
 *	by the call of ipcsrv_channel_lock().
 */
static int
ipcsrv_check_socket(ipc_channel_t *PFC_RESTRICT chp,
		    csockaddr_un_t *PFC_RESTRICT unp, uint32_t addrlen)
{
	pfc_timespec_t	timeout;
	int		sock, err;

	/*
	 * The IPC channel socket file exists.
	 * So we try to connect to this channel with specifying zero timeout.
	 * If no other process listens this channel, pfc_sock_connect()
	 * will returns ECONNREFUSED.
	 */
	sock = pfc_sock_open(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		IPCSRV_LOG_ERROR("Failed to create socket for channel test: "
				 "%s", strerror(err));
		goto error;
	}

	timeout.tv_sec = 0;
	timeout.tv_nsec = 0;
	err = pfc_sock_connect(sock, (csockaddr_t *)unp, addrlen, &timeout,
			       NULL);
	if (PFC_EXPECT_TRUE(err == ECONNREFUSED)) {
		return 0;
	}

	if (PFC_EXPECT_FALSE(err == 0 || err == EINPROGRESS)) {
		IPCSRV_LOG_ERROR("Another IPC server is running: channel=%s",
				 IPCCH_NAME(chp));
		if (err == 0) {
			PFC_ASSERT_INT(close(sock), 0);
		}
		err = EADDRINUSE;
	}
	else {
		IPCSRV_LOG_ERROR("Failed to connect existing IPC "
				 "channel(%s) socket: %s", IPCCH_NAME(chp),
				 strerror(err));
	}

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * static int
 * ipcsrv_epoll_init(ipc_channel_t *chp)
 *	Initialize epoll instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_epoll_init(ipc_channel_t *chp)
{
	int	err;

	/* Create epoll instance. */
	chp->ich_epfd = pfc_epoll_create();
	if (PFC_EXPECT_FALSE(chp->ich_epfd == -1)) {
		err = errno;
		IPCSRV_LOG_ERROR("Failed to create epoll instance: %s",
				 strerror(err));
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}

		return err;
	}

	/* Register listener socket to the epoll instance. */
	err = ipcsrv_epoll_add(chp, chp->ich_listener, EPOLLIN,
			       &chp->ich_ep_listener);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Register shutdown FD to the epoll instance. */
		err = ipcsrv_epoll_add(chp, chp->ich_shutfd, EPOLLIN,
				       &chp->ich_ep_shutdown);
	}

	return err;
}

/*
 * static int
 * ipcsrv_epoll_add(ipc_channel_t *PFC_RESTRICT chp, int fd, uint32_t events,
 *		    pfc_ephdlr_t *PFC_RESTRICT ephp)
 *	Register file descriptor to the epoll instance associated with the
 *	given IPC channel.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipcsrv_epoll_add(ipc_channel_t *PFC_RESTRICT chp, int fd, uint32_t events,
		 pfc_ephdlr_t *PFC_RESTRICT ephp)
{
	int	err;

	err = pfc_epoll_ctl(chp->ich_epfd, EPOLL_CTL_ADD, fd, events, ephp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		err = errno;
		IPCSRV_LOG_ERROR("Failed to register FD to epoll instance: %s",
				 strerror(err));
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}
		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static pfc_cptr_t
 * ipcsrv_handler_getkey(pfc_rbnode_t *node)
 *	Return the key of the given service handler node.
 *	`node' must be a pointer to ihd_node in ipc_handler_t.
 */
static pfc_cptr_t
ipcsrv_handler_getkey(pfc_rbnode_t *node)
{
	ipc_handler_t	*hdp = IPC_HANDLER_NODE2PTR(node);

	return (pfc_cptr_t)pfc_refptr_string_value(hdp->ihd_name);
}

/*
 * static void
 * ipcsrv_setops(ipc_channel_t *chp, ipc_srvops_t *ops)
 *	Set IPC server operations to the IPC channel.
 */
static void
ipcsrv_setops(ipc_channel_t *chp, ipc_srvops_t *ops)
{
	if (ops != NULL && ops->isvops_thread_create != NULL) {
		chp->ich_ops.isvops_thread_create = ops->isvops_thread_create;
	}
}
