/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cred.c - Utilities to handle process credentials.
 */

#include <pfc/util.h>
#include <pfc/path.h>
#include <pfc/debug.h>
#include <pfc/refptr.h>
#include "pwd_impl.h"
#include "conf_impl.h"

#ifdef	PFC_HAVE_PRCTL_DUMPABLE
#include <sys/prctl.h>
#endif	/* PFC_HAVE_PRCTL_DUMPABLE */

/*
 * Set user ID of the calling process.
 */
#ifdef	PFC_HAVE_SETRESUID
#define	PFC_SETUID(uid)		setresuid((uid), (uid), (uid))
#else	/* !PFC_HAVE_SETRESUID */
#define	PFC_SETUID(uid)		setreuid((uid), (uid))
#endif	/* PFC_HAVE_SETRESUID */

/*
 * Set group ID of the calling process.
 */
#ifdef	PFC_HAVE_SETRESGID
#define	PFC_SETGID(gid)		setresgid((gid), (gid), (gid))
#else	/* !PFC_HAVE_SETRESGID */
#define	PFC_SETGID(gid)		setregid((gid), (gid))
#endif	/* PFC_HAVE_SETRESGID */

/*
 * Internal prototypes.
 */
static int	cred_loadconf(void);
static int	cred_enable_coredump(void);

/*
 * int
 * pfc_cred_setup(void)
 *	Set up credentials for the calling process.
 *
 *	This function set the user and group ID of the calling process
 *	according to the system configuration file. This function loads the
 *	system configuration file if it is not loaded yet.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called while the calling process is
 *	single-threaded.
 */
int
pfc_cred_setup(void)
{
	pfc_cfblk_t	options;
	const char	*user, *group;
	int		err;

	if (pfc_sysconf_open() == PFC_CONF_INVALID) {
		/* Load the configuration file. */
		err = cred_loadconf();
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	/* Determine user and group name. */
	options = pfc_sysconf_get_block("options");
	user = pfc_conf_get_string(options, "admin_user", NULL);
	group = pfc_conf_get_string(options, "admin_group", NULL);

	/*
	 * Set user and group ID, and initialize the supplementary access
	 * group list.
	 */
	return pfc_cred_setbyname(user, group, PFC_TRUE);
}

/*
 * int
 * pfc_cred_set(uid_t uid, gid_t gid, pfc_bool_t initgrp)
 *	Set user and group ID of the calling process.
 *
 *	`uid' is a user ID to be applied to the calling process.
 *	If `uid' is not -1, the real and the effective user ID of the calling
 *	process is changed to the specified ID. If setresuid(2) is supported
 *	by the system, the saved set-user-ID of the calling process is also
 *	changed to the specified ID.
 *	The user ID is not changed if `uid' is -1.
 *
 *	`gid' is a group ID to be applied to the calling process.
 *	If `gid' is not -1, the real and the effective group ID of the calling
 *	process is changed to the specified ID. If setresgid(2) is supported
 *	by the system, the saved set-group-ID of the calling process is also
 *	changed to the specified ID.
 *	The group ID is not changed if `gid' is -1.
 *
 *	If `uid' is not -1, and `uid' differs from the real user ID of the
 *	calling process,  and `initgrp' is PFC_TRUE, this function also
 *	initializes the supplementary access group list. In this case
 *	the calling process must be a single-threaded process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- pfc_cred_set() always tries to change group ID before user ID.
 *	  And it returns a non-zero value without reverting group ID if it
 *	  fails to change user ID of the calling process.
 *
 *	- On Linux platform, this function always set 1 to the suid core
 *	  dumpable flag for the current process on successful return.
 */
int
pfc_cred_set(uid_t uid, gid_t gid, pfc_bool_t initgrp)
{
	pwd_user_t	*usp = NULL, uinfo;
	int		err;

	if (initgrp && uid != (uid_t)-1 && uid != getuid()) {
		/*
		 * Determine user information used to initialize the
		 * supplementary group access list.
		 */
		usp = &uinfo;
		err = pfc_pwd_getuser(usp, uid);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	if (gid != (gid_t)-1) {
		/* Change group ID. */
		if (PFC_EXPECT_FALSE(PFC_SETGID(gid) != 0)) {
			err = errno;
			goto error;
		}
	}

	if (uid != (uid_t)-1) {
		/*
		 * Initialize the supplementary group access list only if
		 * the real user ID is changed.
		 */
		if (usp != NULL) {
			err = pfc_pwd_initgroups(&uinfo, gid);
			pfc_pwd_destroyuser(usp);
			usp = NULL;

			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}

		/* Change the user ID of the calling process. */
		if (PFC_EXPECT_FALSE(PFC_SETUID(uid) != 0)) {
			err = errno;
			goto error;
		}
	}

	/*
	 * Setting UID or GID may disable the calling process for dumping
	 * core file. So we need to enable explicitly.
	 */
	err = cred_enable_coredump();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	return 0;

error:
	if (usp != NULL) {
		pfc_pwd_destroyuser(usp);
	}

	if (PFC_EXPECT_FALSE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * int
 * pfc_cred_setbyname(const char *user, const char *group, pfc_bool_t initgrp)
 *	Set user and group of the calling process.
 *
 *	`user' is either a string representation of user ID or a user name.
 *	If `user' is a non-NULL value, the real and the effective user ID of
 *	the calling process is changed to the specified user.
 *	If setresuid(2) is supported by the system, the saved set-user-ID of
 *	the calling process is also changed to the specified user.
 *	The user ID is not changed if `user' is NULL.
 *
 *	`group' is either a string representation of group ID or a group name.
 *	If `group' is a non-NULL value, the real and the effective group ID of
 *	the calling process is changed to the specified group.
 *	If setresgid(2) is supported by the system, the saved set-group-ID of
 *	the calling process is also changed to the specified group.
 *	The group ID is not changed if `group' is NULL.
 *
 *	If `user' is not NULL, and the user ID associated with `user' differs
 *	from the real user ID of the calling process, and `initgrp' is
 *	PFC_TRUE, this function also initializes the supplementary access
 *	group list. In this case the calling process must be a single-threaded
 *	process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- pfc_cred_setbyname() always tries to change group ID before user ID.
 *	  And it returns a non-zero value without reverting group ID if it
 *	  fails to change user ID of the calling process.
 *
 *	- On Linux platform, this function always set 1 to the suid core
 *	  dumpable flag for the current process on successful return.
 */
int
pfc_cred_setbyname(const char *user, const char *group, pfc_bool_t initgrp)
{
	int		err;
	uid_t		uid;
	gid_t		gid = (gid_t)-1;
	pwd_user_t	uinfo;

	if (user != NULL) {
		/* Determine user ID. */
		err = pfc_pwd_getuserbyname(&uinfo, user);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		pfc_pwd_inituser(&uinfo, (uid_t)-1);
	}
	uid = uinfo.pu_uid;

	if (group != NULL) {
		/* Determine group ID. */
		err = pfc_pwd_strtogid(group, &gid);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		/* Chagne the group ID of the calling process. */
		if (PFC_EXPECT_FALSE(PFC_SETGID(gid) != 0)) {
			err = errno;
			goto error;
		}
	}

	if (uid != (uid_t)-1) {
		/*
		 * Initialize the supplementary group access list only if
		 * the real user ID is changed.
		 */
		if (initgrp && uid != getuid()) {
			err = pfc_pwd_initgroups(&uinfo, gid);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}

		/* Change the user ID of the calling process. */
		if (PFC_EXPECT_FALSE(PFC_SETUID(uid) != 0)) {
			err = errno;
			goto error;
		}
	}

	/*
	 * Setting UID or GID may disable the calling process for dumping
	 * core file. So we need to enable explicitly.
	 */
	err = cred_enable_coredump();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_pwd_destroyuser(&uinfo);

	return 0;

error:
	pfc_pwd_destroyuser(&uinfo);

	if (PFC_EXPECT_FALSE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * static int
 * cred_loadconf(void)
 *	Load the system configuration file for the given daemon process.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
cred_loadconf(void)
{
	pfc_refptr_t	*rpath;
	char		*path;
	int		err;

	/* Ensure that the configuration file is safe. */
	path = strdup(PFC_PFCD_CONF_PATH);
	if (PFC_EXPECT_FALSE(path == NULL)) {
		return ENOMEM;
	}

	err = pfc_is_safepath(path);
	free(path);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	rpath = pfc_refptr_string_create(PFC_PFCD_CONF_PATH);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		return ENOMEM;
	}

	err = pfc_sysconf_init(rpath);
	pfc_refptr_put(rpath);
	PFC_ASSERT(err != 0 || pfc_sysconf_open() != PFC_CONF_INVALID);

	return err;
}

/*
 * static int
 * cred_enable_coredump(void)
 *	Allow the calling process to dump core file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Currently, this function works only on Linux.
 */
static int
cred_enable_coredump(void)
{
#ifdef	PFC_HAVE_PRCTL_DUMPABLE
	int	ret;

	ret = prctl(PR_SET_DUMPABLE, 1);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		PFC_ASSERT(ret == -1);
		ret = errno;
	}

	return ret;
#else	/* !PFC_HAVE_PRCTL_DUMPABLE */
	return 0;
#endif	/* PFC_HAVE_PRCTL_DUMPABLE */
}
