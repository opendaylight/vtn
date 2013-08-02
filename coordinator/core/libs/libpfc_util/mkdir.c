/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * mkdir.c - Create directory and its parent directories.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pfc/util.h>
#include <pfc/debug.h>

#define	PERM_MASK	07777

/*
 * Internal prototypes.
 */
static int	is_directory(const char *path, mode_t perm, pfc_bool_t ckperm);

/*
 * int
 * pfc_mkdir(const char *path, mode_t perm)
 *	Create directory.
 *	Its parent directories are created as needed just like "mkdir -p".
 *	`perm' is a directory permission to be applied. It is also used to
 *	create parent directories.
 *
 * Calling/Exit State:
 *	Zero is returned if a directory is successfully created, or it already
 *	exists.
 *	EEXIST is returned if a non-directory file exists at the specified
 *	path.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- pfc_mkdir() guarantees that the specified directory has a permission
 *	  specified by `perm', but not for existing parent directories.
 *
 *	- `path' must not contain "." and ".." as path component.
 *
 *	- Symbolic link in path component is not allowed.
 */
int
pfc_mkdir(const char *path, mode_t perm)
{
	char	*buf, *p;
	size_t	len;
	int	err;

	if (PFC_EXPECT_FALSE(path == NULL || *path == '\0')) {
		return EINVAL;
	}

	/* At first, try to create directory. */
	if (mkdir(path, perm) == 0) {
		return 0;
	}

	err = errno;
	if (err == EEXIST) {
		/* Ensure that a directory exists at the specified path. */
		return is_directory(path, perm, PFC_TRUE);
	}
	if (PFC_EXPECT_FALSE(err != ENOENT)) {
		/* Fatal error. */
		return err;
	}

	/* Try to create parent directory. */
	len = strlen(path);
	buf = (char *)malloc(len + 1);
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		return ENOMEM;
	}
	memcpy(buf, path, len + 1);
	p = buf + len - 1;

	for (;;) {
		for (; p > buf && *p != '/'; p--);
		if (p == buf) {
			/* This should not happen. */
			err = ENOENT;
			goto out;
		}

		*p = '\0';
		if (mkdir(buf, perm) == 0) {
			break;
		}

		if ((err = errno) == EEXIST) {
			err = is_directory(path, perm, PFC_FALSE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto out;
			}
			break;
		}

		if (err != ENOENT) {
			goto out;
		}
	}

	err = 0;
	for (; p < buf + len; p++) {
		if (*p != '\0') {
			continue;
		}

		*p = '/';
		if (PFC_EXPECT_TRUE(mkdir(buf, perm) == 0)) {
			err = 0;
		}
		else {
			err = errno;
			if (PFC_EXPECT_FALSE(err != EEXIST)) {
				goto out;
			}

			/* Check file type and its permission. */
			err = is_directory(path, perm, PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto out;
			}
		}
	}

out:
	free(buf);

	return err;
}

/*
 * static int
 * is_directory(const char *path, mode_t perm, pfc_bool_t ckperm)
 *	Determine whether a file specified by `path' is a directory or not.
 *	If `ckperm' is PFC_TRUE, a directory specified by `path' is
 *	guaranteed to have a permission specified by `perm'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if a file specified by `path' is not a directory.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
is_directory(const char *path, mode_t perm, pfc_bool_t ckperm)
{
	struct stat	sbuf;
	mode_t		mode;

	if (PFC_EXPECT_FALSE(lstat(path, &sbuf) != 0)) {
		return errno;
	}

	mode = sbuf.st_mode;
	if (PFC_EXPECT_FALSE(!S_ISDIR(mode))) {
		return EEXIST;
	}

	if (ckperm && (mode & PERM_MASK) != (perm & PERM_MASK)) {
		/* Change access permission. */
		if (PFC_EXPECT_FALSE(chmod(path, perm) != 0)) {
			return errno;
		}
	}

	return 0;
}
