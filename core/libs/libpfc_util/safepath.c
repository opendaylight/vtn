/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * safepath.c - Determine whether the file or directory is safe or not.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pfc/util.h>
#include <pfc/debug.h>

/*
 * int
 * pfc_is_safepath(char *path)
 *	Determine whether specified file path is safe or not.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Error number which indicates the cause of error is returned on failure.
 *
 * Remarks:
 *	This function always breaks the specified string.
 *	On failure, it is updated to unsafe file path.
 */
int
pfc_is_safepath(char *path)
{
	struct stat	sbuf;
	char	*sep;
	mode_t	mode;

	if (PFC_EXPECT_FALSE(*path != '/')) {
		/* Must be an absolute path. */
		return EINVAL;
	}

	if (PFC_EXPECT_FALSE(stat(path, &sbuf) == -1)) {
		return errno;
	}

	mode = sbuf.st_mode;
	if (mode & (S_IWGRP | S_IWOTH)) {
		/* Group or world writable. */
		return EPERM;
	}

	/* Ensure that the parent directory is safe. */
	sep = strrchr(path, '/');
	PFC_ASSERT(sep != NULL);
	if (sep != path) {
		PFC_ASSERT(path < sep);
		*sep = '\0';

		while (1) {
			mode_t	pmode;

			if (PFC_EXPECT_FALSE(stat(path, &sbuf) == -1)) {
				return errno;
			}

			pmode = sbuf.st_mode;
			if (PFC_EXPECT_FALSE(pmode & (S_IWGRP | S_IWOTH))) {
				/*
				 * Group or world writable file is unsafe.
				 */
				return EPERM;
			}

			sep--;
			if (sep < path) {
				break;
			}

			/*
			 * We don't need to check underrun because the path
			 * is an absolute path.
			 */
			for (; *sep != '/'; sep--);
			PFC_ASSERT(sep >= path);
			if (sep == path) {
				break;
			}
			*sep = '\0';
		}
	}

	/* Ensure the root directory is safe. */
	PFC_ASSERT(*path == '/');
	*(path + 1) = '\0';
	if (PFC_EXPECT_FALSE(stat(path, &sbuf) == -1)) {
		return errno;
	}

	mode = sbuf.st_mode;
	if (mode & (S_IWGRP | S_IWOTH)) {
		/* Group or world writable. */
		return EPERM;
	}

	return 0;
}
