/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * closefrom_linux.c - closefrom() implementation for Linux.
 */

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pfc/util.h>
#include <pfc/strtoint.h>
#include "util_impl.h"

/* Directory path that keeps active file descriptors of the current process. */
#define	PROC_SELF_FD	"/proc/self/fd"

/*
 * void
 * pfc_closefrom(int lowfd)
 *	Close all file descriptors greater than or equal to the specified
 *	number.
 */
void
pfc_closefrom(int lowfd)
{
	pfc_bool_t	closed;
	struct dirent	*buf;

	/*
	 * On GNU libc, 256 bytes are preserved for dirent.d_name.
	 * So we should not put it on the stack.
	 */
	buf = (struct dirent *)malloc(sizeof(*buf));
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		return;
	}

	do {
		DIR		*dirp;
		struct dirent	*dp;
		int	dirfd;

		closed = PFC_FALSE;

		/*
		 * Open /proc filesystem.
		 * Don't use opendir() here, or the below code may close file
		 * descriptor associated with the directory handle.
		 */
		dirfd = pfc_open_cloexec(PROC_SELF_FD, O_RDONLY);
		if (PFC_EXPECT_FALSE(dirfd == -1)) {
			break;
		}

		dirp = fdopendir(dirfd);
		if (PFC_EXPECT_FALSE(dirp == NULL)) {
			break;
		}

		while (readdir_r(dirp, buf, &dp) == 0) {
			int		err;
			uint32_t	fd;

			if (dp == NULL) {
				break;
			}
			if (dp->d_name[0] == '.') {
				continue;
			}

			err = pfc_strtou32(dp->d_name, &fd);
			if (PFC_EXPECT_FALSE(err != 0)) {
				continue;
			}

			if ((int)fd != dirfd && (int)fd >= lowfd) {
				(void)close(fd);

				/*
				 * Closing file descriptor may affect
				 * to the /proc/self/fd directory
				 * entry. So one more check is needed.
				 */
				closed = PFC_TRUE;
			}
		}
		closedir(dirp);

		/*
		 * If we are running on the valgrind, we should not loop here
		 * because the valgrind uses some file descriptors internally,
		 * and they are protected against close(2).
		 */
		if (pfc_valgrind_mode) {
			break;
		}
	} while (closed);

	free(buf);
}
