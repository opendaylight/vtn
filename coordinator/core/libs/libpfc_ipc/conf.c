/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf.c - Accessor of IPC configuration file.
 */

#include <pfc/util.h>
#include <pfc/path.h>
#include <pfc/synch.h>
#include "ipc_impl.h"

/*
 * Internal prototypes.
 */
static pfc_bool_t	ipc_conf_checkdir(const char *path, mode_t perm);
static int		ipc_conf_mkdir(const char *path, mode_t perm);

/*
 * IPC system directory paths.
 */
static const char	ipc_workdir[] = PFC_IPCWORKDIR;
static const char	ipc_channeldir[] =
	IPC_WORKDIR_PATH(IPC_CHANNEL_DIRNAME);

/*
 * Permission bits for IPC directories.
 */
#define	IPC_DIRPERM_workdir						\
	(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)	/* 0755 */
#define	IPC_DIRPERM_channeldir						\
	(S_IRWXU | S_IRWXG | S_IRWXO | S_ISVTX)			/* 01777 */

#define	IPC_CONF_CHECKDIR(name)					\
	(ipc_conf_checkdir(ipc_##name, IPC_DIRPERM_##name))

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * ipc_conf_checkname(const char *channel)
 *	Ensure that the channel name is valid.
 *
 * Calling/Exit State:
 *	Zero is returned if the channel name is valid.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
ipc_conf_checkname(const char *channel)
{
	return pfc_ipc_checkname(IPC_NAME_CHANNEL, channel);
}

/*
 * int
 * pfc_ipc_conf_dirinit(void)
 *	Initialize IPC system directories.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called by root user.
 */
int
pfc_ipc_conf_dirinit(void)
{
	int	err;

	PFC_ASSERT(geteuid() == 0);

	/* Verify that IPC working directory is sane. */
	if (PFC_EXPECT_TRUE(IPC_CONF_CHECKDIR(workdir) &&
			    IPC_CONF_CHECKDIR(channeldir))) {
		return 0;
	}

	/* Create IPC working directory. */
	err = ipc_conf_mkdir(ipc_workdir, IPC_DIRPERM_workdir);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Create IPC channel directory. */
	return ipc_conf_mkdir(ipc_channeldir, IPC_DIRPERM_channeldir);
}

/*
 * int
 * pfc_ipc_conf_getpath(const char *PFC_RESTRICT channel
 *			pfc_refptr_t **PFC_RESTRICT pathpp)
 *	Get path to UNIX domain socket file and temporary directory associated
 *	with the IPC channel name specified by `channel'.
 *
 *	`channel' is a name of IPC channel.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to refptr string which
 *	keeps UNIX domain socket file path is stored to `*pathpp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must decrement the reference counter of the refptr string
 *	in `*pathpp' on successful return.
 */
int
pfc_ipc_conf_getpath(const char *PFC_RESTRICT channel,
		     pfc_refptr_t **PFC_RESTRICT pathpp)
{
	sockaddr_un_t	un;
	pfc_refptr_t	*path;
	int		err, len;

	PFC_ASSERT(channel != NULL);

	/* Verify IPC channel name. */
	err = ipc_conf_checkname(channel);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Construct UNIX domain socket path. */
	len = snprintf(un.sun_path, sizeof(un.sun_path), "%s/.%s",
		       ipc_channeldir, channel);
	if (PFC_EXPECT_FALSE(len < 0 || (size_t)len >= sizeof(un.sun_path))) {
		IPC_LOG_ERROR("Failed to copy IPC channel path: channel=%s",
			      channel);

		return EOVERFLOW;
	}

	/* Create string refptr which keeps UNIX domain socket path. */
	path = pfc_refptr_string_create(un.sun_path);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		IPC_LOG_ERROR("No memory for UNIX domain socket path.");

		return ENOMEM;
	}

	*pathpp = path;

	return 0;
}

/*
 * int
 * pfc_ipc_conf_getsrvconf(const char *PFC_RESTRICT channel,
 *			   ipc_chconf_t *PFC_RESTRICT chcp)
 *	Get IPC channel configuration associated with the given channel name.
 *
 *	This function obtains configuration for server process:
 *	  - Value of "allow_group" is set to icc_allow_group.
 *	  - Value of "max_clients" is set to icc_max_clients.
 *	  - Value of "max_sessions" is set to icc_max_sessions.
 *	  - Value of "max_monitors" is set to icc_max_monitors.
 *	  - Value of "timeout" is set to icc_timeout
 *	  - Value of "permission" is set to icc_permission.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	In this case, ipc_chconf_t buffer specified by `chcp' is filled with
 *	default parameters.
 */
int
pfc_ipc_conf_getsrvconf(const char *PFC_RESTRICT channel,
			ipc_chconf_t *PFC_RESTRICT chcp)
{
	PFC_ASSERT(channel != NULL);

	chcp->icc_max_clients = IPC_CHANNEL_MAX_CLIENTS;
	chcp->icc_max_sessions = IPC_CHANNEL_MAX_SESSIONS;
	chcp->icc_timeout = IPC_CHANNEL_TIMEOUT;
	chcp->icc_permission = IPC_CHANNEL_PERMISSION;

	return 0;
}

/*
 * int
 * pfc_ipc_conf_getcliconf(const char *PFC_RESTRICT channel,
 *			   ipc_chconf_t *PFC_RESTRICT chcp)
 *	Get IPC channel configuration associated with the given channel name.
 *
 *	This function obtains configuration for client processes:
 *	  - Value of "timeout" is set to icc_timeout
 *
 *	Other fields are never changed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	In this case, ipc_chconf_t buffer specified by `chcp' is filled with
 *	default parameters.
 */
int
pfc_ipc_conf_getcliconf(const char *PFC_RESTRICT channel,
			ipc_chconf_t *PFC_RESTRICT chcp)
{
	PFC_ASSERT(channel != NULL);

	chcp->icc_timeout = IPC_CHANNEL_TIMEOUT;

	return 0;
}

/*
 * static pfc_bool_t
 * ipc_conf_checkdir(const char *path, mode_t perm)
 *	Ensure that the file specified by `path' is a directory file,
 *	and its access permission is `perm'.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if all checks are passed.
 *	Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
ipc_conf_checkdir(const char *path, mode_t perm)
{
	struct stat	sbuf;
	mode_t		mode;
	int		ret;

	ret = stat(path, &sbuf);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		return PFC_FALSE;
	}

	mode = sbuf.st_mode;
	if (PFC_EXPECT_FALSE(!S_ISDIR(mode) || (mode & 07777) != perm)) {
		return PFC_FALSE;
	}

	/* Owner and group of IPC system directory must be root. */

	return PFC_TRUE;
}

/*
 * static int
 * ipc_conf_mkdir(const char *path, mode_t perm)
 *	Create the specified directory.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called by root user.
 */
static int
ipc_conf_mkdir(const char *path, mode_t perm)
{
	int	err, ret;

	PFC_ASSERT(geteuid() == 0);

	err = pfc_mkdir(path, perm);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != EEXIST) {
			IPC_LOG_ERROR("Failed to create directory(%s): %s",
				      path, strerror(err));

			return err;
		}

		/* Try to remove unexpected file. */
		err = pfc_rmpath(path);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_LOG_ERROR("Failed to clean up directory(%s): %s",
				      path, strerror(err));

			return err;
		}

		/* Try to create directory again. */
		err = pfc_mkdir(path, perm);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_LOG_ERROR("Failed to create directory(%s): %s",
				      path, strerror(err));

			return err;
		}
	}

	if (geteuid() == 0) {
		/* Change owner and group to root explicitly. */
		ret = chown(path, 0, 0);
		if (PFC_EXPECT_FALSE(ret != 0)) {
			err = errno;
			IPC_LOG_ERROR("Failed to change owner of directory(%s)"
				      ": %s", path, strerror(err));
			goto error;
		}
	}

	/*
	 * Permission bits must be changed explicitly, because some permission
	 * bits may be dropped by umask.
	 */
	ret = chmod(path, perm);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPC_LOG_ERROR("Failed to change permission for directory(%s)"
			      ": %s", path, strerror(err));
		goto error;
	}

	return err;

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}
