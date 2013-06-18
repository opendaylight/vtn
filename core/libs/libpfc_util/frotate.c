/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * frotate.c - Rotate files under the given directory.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pfc/util.h>

#ifndef	PFC_HAVE_ATFILE_SYSCALL
#error	Current implementation requires ATFILE system calls.
#endif	/* !PFC_HAVE_ATFILE_SYSCALL */

/*
 * Maximum length of filename.
 */
#define	FROTATE_FNAME_MAXLEN	PFC_CONST_U(64)

/*
 * Size of buffer to keep filename.
 */
#define	FROTATE_FNAME_BUFSIZE	(FROTATE_FNAME_MAXLEN + 16)

/*
 * Internal prototypes.
 */
static int	rotate_files(int dfd, const char *name, uint32_t rotate);

/*
 * int
 * pfc_frotate_openat(int dfd, const char *name, mode_t mode, size_t maxsize,
 *		      uint32_t rotate)
 *	Open the file specified by `dfd' and `name' with preserving old files.
 *
 *	`dfd' must be a file descriptor associated with parent directory of
 *	the target file, and `name' is the name of the target file.
 *	If PFC_AT_FDCWD is specified to `dfd', the target file is interpreted
 *	relative to the current working directory. The target file is always
 *	opened with (O_RDWR | O_APPEND) and close-on-exec flag.
 *
 *	`mode' is a file permission bits passed to open(2).
 *
 *	`maxsize' is the limit of file size. If the size of the target file is
 *	greater than or equal to `maxsize', the target file is preserved by
 *	rotating files. If `maxsize' is zero, the rotation is always performed.
 *
 *	`rotate' is the maximum number of rotated files. Specifying zero to
 *	`rotate' disables the rotation. In that case, the target file is
 *	removed before creating the target file if its size is greater than
 *	or equal to `maxsize'.
 *
 *	If `rotate' is 4, each files under the directory is renamed as follows:
 *
 *	  + name.3  -> name.4
 *	  + name.2  -> name.3
 *	  + name.1  -> name.2
 *	  + name    -> name.1
 *
 * Calling/Exit State:
 *	Upon successful completion, a valid file descriptor associated with
 *	the target file is returned.
 *	On failure, -1 is returned. An appropriate error number is set to
 *	errno.
 *
 * Remarks:
 *	- The target file must be a regular file. Specifying non-regular file
 *	  to the target file or rotated files results in undefined behavior.
 *
 *	- Unlike ATFILE system calls, `name' can not contain directory
 *	  separator.
 *
 *	- The length of file name specified by `name' must be less than
 *	  or equal to 64 bytes.
 *
 *	- It is up to the caller to serialize multiple access to the same
 *	  directory.
 */
int
pfc_frotate_openat(int dfd, const char *PFC_RESTRICT name, mode_t mode,
		   size_t maxsize, uint32_t rotate)
{
	const char	*p;
	size_t		len;
	pfc_bool_t	do_rotate;
	int		err = 0, fd;

	if (PFC_EXPECT_FALSE(name == NULL || *name == '\0')) {
		goto error;
	}

	/* Verify file name. */
	for (p = name, len = 0; *p != '\0'; p++) {
		len++;
		if (PFC_EXPECT_FALSE(len > FROTATE_FNAME_MAXLEN)) {
			err = ENAMETOOLONG;
			goto error;
		}

		if (PFC_EXPECT_FALSE(*p == '/')) {
			goto error;
		}
	}

	/* Determine whether file rotation should be performed or not. */
	if (maxsize == 0) {
		/* File rotation must be always performed. */
		do_rotate = PFC_TRUE;
	}
	else {
		struct stat	sbuf;
		size_t		size;

		/* Determine the size of the target file. */
		if (fstatat(dfd, name, &sbuf, AT_SYMLINK_NOFOLLOW) == 0) {
			if (PFC_EXPECT_FALSE(!S_ISREG(sbuf.st_mode))) {
				err = EPERM;
				goto error;
			}
			size = (size_t)sbuf.st_size;
		}
		else {
			int	e = errno;

			if (PFC_EXPECT_FALSE(e != ENOENT)) {
				err = e;
				goto error;
			}
			size = 0;
		}

		do_rotate = (size >= maxsize) ? PFC_TRUE : PFC_FALSE;
	}

	if (do_rotate) {
		/* Perform file rotation. */
		err = rotate_files(dfd, name, rotate);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}

	/* Open the target file. */
	fd = pfc_openat_cloexec(dfd, name, O_RDWR | O_CREAT | O_APPEND, mode);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		goto error;
	}

	return fd;

error:
	errno = (err == 0) ? EINVAL : err;

	return -1;
}

/*
 * static int
 * rotate_files(int dfd, const char *name, uint32_t rotate)
 *	Preserve old files by rotating file name.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
rotate_files(int dfd, const char *name, uint32_t rotate)
{
	uint32_t	i;
	char		src[FROTATE_FNAME_BUFSIZE], dst[FROTATE_FNAME_BUFSIZE];
	int		ret;

	if (rotate == 0) {
		int	err = 0;

		/* Remove the target file. */
		ret = unlinkat(dfd, name, 0);
		if (PFC_EXPECT_FALSE(ret != 0)) {
			/* Ignore ENOENT error. */
			if ((err = errno) == ENOENT) {
				err = 0;
			}
		}

		return err;
	}

	/* Rotate old files. */
	for (i = rotate; i > 1; i--) {
		snprintf(src, sizeof(src), "%s.%u", name, i - 1);
		snprintf(dst, sizeof(dst), "%s.%u", name, i);
		ret = renameat(dfd, src, dfd, dst);
		if (PFC_EXPECT_FALSE(ret != 0)) {
			int	err = errno;

			/* Ignore ENOENT error. */
			if (err != ENOENT) {
				return err;
			}
		}
	}

	snprintf(dst, sizeof(dst), "%s.1", name);
	ret = renameat(dfd, name, dfd, dst);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		int	err = errno;

		/* Ignore ENOENT error. */
		if (err != ENOENT) {
			return err;
		}
	}

	return 0;
}
