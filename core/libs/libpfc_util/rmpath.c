/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * rmpath.c - Remove file or directory and its sub components.
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pfc/util.h>

#ifndef	PFC_HAVE_ATFILE_SYSCALL
#error	Current implementation requires ATFILE system calls.
#endif	/* !PFC_HAVE_ATFILE_SYSCALL */

/*
 * open(2) flags to open directory entry.
 */
#define	RMPATH_OPEN_FLAGS						\
	(O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NOFOLLOW)

/*
 * Internal prototypes.
 */
static int	dir_cleanup(int dirfd, struct dirent *buf);

/*
 * static inline int
 * unlink_file(int dirfd, const char *path)
 *	Unlink file specified by directory file descriptor `dirfd' and file
 *	path `path'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if non-empty directory is specified.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int
unlink_file(int dirfd, const char *path)
{
	int		uerr;
	struct stat	sbuf;

	if (unlinkat(dirfd, path, 0) == 0) {
		return 0;
	}

	/* Preserve errno set by the call of unlinkat(2). */
	if ((uerr = errno) == ENOENT || uerr == EFAULT) {
		return uerr;
	}

	/* Determine file type. */
	if (PFC_EXPECT_FALSE(fstatat(dirfd, path, &sbuf,
				     AT_SYMLINK_NOFOLLOW) != 0)) {
		return errno;
	}

	if (S_ISDIR(sbuf.st_mode)) {
		return -1;
	}

	return uerr;
}

/*
 * int
 * pfc_rmpath(const char *path)
 *	Remove up file or directory specified by `path'.
 *	Any file, including sub-directory, contained by `path' is also removed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function uses recursive call. The caller must take care of
 *	  procedure stack usage.
 *
 *	- Specifying path which has "." or ".." as last path component causes
 *	  undefined behavior.
 */
int
pfc_rmpath(const char *path)
{
	return pfc_rmpathat(PFC_AT_FDCWD, path);
}

/*
 * int
 * pfc_rmpathat(int dirfd, const char *path)
 *	Remove up file or directory specified by `dirfd' and `path'.
 *
 *	`dirfd' must be a file descriptor associated with directory.
 *	If the given file path specified by `path' is relative, it is
 *	interpreted relative to the directory associated with `dirfd'.
 *
 *	If PFC_AT_FDCWD is specified to `dirfd', `path' is interpreted
 *	relative to the current working directory, just like pfc_rmpath().
 *
 *	If the given file path specified by `path' is absolute, `dirfd' is
 *	simply ignored.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function uses recursive call. The caller must take care of
 *	  procedure stack usage.
 *
 *	- Specifying path which has "." or ".." as last path component causes
 *	  undefined behavior.
 */
int
pfc_rmpathat(int dirfd, const char *path)
{
	int	err;

	/* At first, try to unlink the given path. */
	err = unlink_file(dirfd, path);
	if (err >= 0) {
		/* Succeeded, or fatal error. */
		return err;
	}

	/* Clean up the target directory. */
	err = pfc_cleandirat(dirfd, path);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Remove directory. */
		if (PFC_EXPECT_FALSE(unlinkat(dirfd, path, AT_REMOVEDIR)
				     != 0)) {
			err = errno;
		}
	}

	return err;
}

/*
 * int
 * pfc_cleandir(const char *path)
 *	Make the given directory empty.
 *
 *	`path' must be a path to existing directory. Any file, including
 *	sub-directory, under the given directory is removed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function uses recursive call. The caller must take care of
 *	  procedure stack usage.
 *
 *	- Specifying path which has "." or ".." as last path component causes
 *	  undefined behavior.
 */
int
pfc_cleandir(const char *path)
{
	return pfc_cleandirat(PFC_AT_FDCWD, path);
}

/*
 * int
 * pfc_cleandirat(int dirfd, const char *path)
 *	Make the given directory empty.
 *
 *	The target directory is specified `dirfd' and `path'.
 *	`dirfd' must be a file descriptor associated with directory.
 *	If the given directory path specified by `path' is relative, it is
 *	interpreted relative to the directory associated with `dirfd'.
 *
 *	If PFC_AT_FDCWD is specified to `dirfd', `path' is interpreted
 *	relative to the current working directory, just like pfc_cleandir().
 *
 *	If the given file path specified by `path' is absolute, `dirfd' is
 *	simply ignored.
 *
 *	Similar to pfc_cleandir(), the directory specified by `dirfd' and
 *	`path' must exist.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function uses recursive call. The caller must take care of
 *	  procedure stack usage.
 *
 *	- Specifying path which has "." or ".." as last path component causes
 *	  undefined behavior.
 */
int
pfc_cleandirat(int dirfd, const char *path)
{
	struct dirent	*buf;
	int		fd, err;

	/* Open target directory. */
	fd = pfc_openat_cloexec(dirfd, path, RMPATH_OPEN_FLAGS);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return errno;
	}

	/*
	 * On GNU libc, 256 bytes are preserved for dirent.d_name.
	 * So we should not put it on the stack.
	 */
	buf = (struct dirent *)malloc(sizeof(*buf));
	if (PFC_EXPECT_FALSE(buf == NULL)) {
		(void)close(fd);

		return ENOMEM;
	}

	/* Clean up directory. */
	err = dir_cleanup(fd, buf);
	free(buf);

	return err;
}

/*
 * static int
 * dir_cleanup(int dirfd, struct dirent *buf)
 *	Remove all directory entries contained by directory associated with
 *	file descriptor `dirfd'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function always closes file descriptor specified by `dirfd'.
 */
static int
dir_cleanup(int dirfd, struct dirent *buf)
{
	struct dirent	*dp;
	DIR	*dirp;
	int	err;

	dirp = fdopendir(dirfd);
	if (PFC_EXPECT_FALSE(dirp == NULL)) {
		err = errno;
		(void)close(dirfd);

		return err;
	}

	while ((err = readdir_r(dirp, buf, &dp)) == 0) {
		const char	*name;
		int	fd, result;
		char	*dname = NULL;

		if (dp == NULL) {
			/* End of directory stream. */
			break;
		}

		name = (const char *)dp->d_name;
		if (*name == '.' &&
		    (*(name + 1) == '\0' ||
		     (*(name + 1) == '.' && *(name + 2) == '\0'))) {
			/* Skip "." and "..". */
			continue;
		}

		/* Try to unlink this directory entry. */
		err = unlink_file(dirfd, name);
		if (err == 0 || err == ENOENT) {
			continue;
		}
		if (PFC_EXPECT_FALSE(err > 0)) {
			break;
		}

		/* Open directory entry. */
		fd = pfc_openat_cloexec(dirfd, name, RMPATH_OPEN_FLAGS);
		if (PFC_EXPECT_FALSE(fd == -1)) {
			if ((err = errno) == ENOENT) {
				/*
				 * Directory entry may be removed by another
				 * context.
				 */
				continue;
			}
			break;
		}

		/*
		 * Directory name needs to be preserved because dir_cleanup()
		 * will update contents of `buf'.
		 */
		dname = strdup(name);
		if (PFC_EXPECT_FALSE(dname == NULL)) {
			err = ENOMEM;
			break;
		}

		/*
		 * Clean up sub directory.
		 * `fd' will be closed by this call().
		 */
		err = dir_cleanup(fd, buf);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		/* Unlink directory entry. */
		result = unlinkat(dirfd, dname, AT_REMOVEDIR);
		free(dname);
		if (PFC_EXPECT_FALSE(result != 0)) {
			if ((err = errno) == ENOENT) {
				/*
				 * Directory entry may be removed by another
				 * context.
				 */
				continue;
			}
			break;
		}
	}

	/* `dirfd' must be closed by this call. */
	closedir(dirp);

	return err;
}
