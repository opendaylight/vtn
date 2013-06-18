/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * flock.c - File record lock.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pfc/util.h>
#include <pfc/debug.h>

#ifndef	PFC_HAVE_FCNTL_FLOCK
#error	File record lock is required.
#endif	/* !PFC_HAVE_FCNTL_FLOCK */

/*
 * Lock file name for the directory lock.
 */
#define	FLOCK_DIRLOCK_NAME	".dirlock"
#define	FLOCK_DIRLOCK_NAMELEN	8

/*
 * Initialize struct flock to acquire or release the lock for the whole file
 * contents.
 */
#define	FLOCK_INIT(lkp, type)			\
	do {					\
		(lkp)->l_type = (type);		\
		(lkp)->l_whence = SEEK_SET;	\
		(lkp)->l_start = 0;		\
		(lkp)->l_len = 0;		\
		(lkp)->l_pid = -1;		\
	} while (0)

/*
 * Internal prototypes.
 */
static int	flock_do_lock(int fd, short type, pid_t *ownerp);
static int	flock_get_owner(int fd, short type, pid_t *ownerp);

/*
 * int
 * pfc_flock_open(pfc_flock_t *PFC_RESTRICT lkp, const char *PFC_RESTRICT path,
 *		  int mode, mode_t perm)
 *	Open file lock handle to serialize access to the specified path.
 *
 *	`mode' is open mode bits, and `perm' is a permission of file.
 *	They will be passed to open(2) to open the specified path.
 *
 * Calling/Exit State:
 *	Upon successful completion, file lock handle is set to `*lkp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_open(pfc_flock_t *PFC_RESTRICT lkp, const char *PFC_RESTRICT path,
	       int mode, mode_t perm)
{
	int	fd;

	/*
	 * Open a file, and set close-on-exec flag.
	 * File record lock is never inherited to child process.
	 */
	fd = pfc_open_cloexec(path, mode, perm);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return errno;
	}

	*lkp = (pfc_flock_t)fd;

	return 0;
}

/*
 * int
 * pfc_flock_close(pfc_flock_t lk)
 *	Close the file lock handle.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_close(pfc_flock_t lk)
{
	if (PFC_EXPECT_TRUE(close((int)lk) == 0)) {
		return 0;
	}

	return errno;
}

/*
 * int
 * pfc_flock_opendir(pfc_flock_t *PFC_RESTRICT lkp,
 *		     const char *PFC_RESTRICT path, int mode, mode_t perm)
 *	Open lock handle to lock the specified directory.
 *
 *	pfc_flock_opendir() creates a lock file under the specified directory.
 *	Returned lock handle is a file descriptor associated with the lock file,
 *	not directory.
 *
 * Calling/Exit State:
 *	Upon successful completion, file lock handle is set to `*lkp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_opendir(pfc_flock_t *PFC_RESTRICT lkp, const char *PFC_RESTRICT path,
		  int mode, mode_t perm)
{
	char	*fpath;
	size_t	len = strlen(path) + FLOCK_DIRLOCK_NAMELEN + 2;
	int	err;

	fpath = (char *)malloc(len);
	if (PFC_EXPECT_FALSE(fpath == NULL)) {
		return ENOMEM;
	}

	PFC_ASSERT_INT(snprintf(fpath, len, "%s/" FLOCK_DIRLOCK_NAME, path),
		       len - 1);

	err = pfc_flock_open(lkp, fpath, mode, perm);
	free(fpath);

	return err;
}

/*
 * int
 * pfc_flock_closedir(pfc_flock_t lk)
 *	Close file lock handle created by pfc_flock_opendir().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_closedir(pfc_flock_t lk)
{
	if (PFC_EXPECT_TRUE(close((int)lk) == 0)) {
		return 0;
	}

	return errno;
}

/*
 * int
 * pfc_flock_wrlock(pfc_flock_t lk, pid_t *ownerp)
 *	Acquire exclusive file lock.
 *
 *	If `ownerp' is not NULL, pfc_flock_wrlock() tries to acquire writer
 *	lock without blocking the calling thread. If can't, process ID of
 *	the current lock owner is set to `*ownerp'. In this case, EINTR error
 *	is simply ignored.
 *
 *	If `ownerp' is NULL, pfc_flock_wrlock() waits for the lock to be
 *	released. So pfc_flock_wrlock() never returns EAGAIN. If the wait is
 *	interrupted by a signal, EINTR is returned.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If another process owns the lock and `ownerp' is not NULL,
 *	process ID of the current owner is set to `*ownerp' and EAGAIN is
 *	returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_wrlock(pfc_flock_t lk, pid_t *ownerp)
{
	return flock_do_lock((int)lk, F_WRLCK, ownerp);
}

/*
 * int
 * pfc_flock_rdlock(pfc_flock_t lk, pid_t *ownerp)
 *	Acquire shared file lock.
 *
 *	If `ownerp' is not NULL, pfc_flock_rdlock() tries to acquire reader
 *	lock without blocking the calling thread. If can't, process ID of
 *	the current lock owner is set to `*ownerp'. In this case, EINTR error
 *	is simply ignored.
 *
 *	If `ownerp' is NULL, pfc_flock_rdlock() waits for the lock to be
 *	released. So pfc_flock_rdlock() never returns EAGAIN. If the wait is
 *	interrupted by a signal, EINTR is returned.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If another process owns the lock and `ownerp' is not NULL,
 *	process ID of the current owner is set to `*ownerp' and EAGAIN is
 *	returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_flock_rdlock(pfc_flock_t lk, pid_t *ownerp)
{
	return flock_do_lock((int)lk, F_RDLCK, ownerp);
}

/*
 * int
 * pfc_flock_unlock(pfc_flock_t lk)
 *	Release the file record lock.
 */
int
pfc_flock_unlock(pfc_flock_t lk)
{
	int	err;

	while (1) {
		struct flock	flock;

		FLOCK_INIT(&flock, F_UNLCK);
		if (PFC_EXPECT_TRUE(fcntl(lk, F_SETLK, &flock) != -1)) {
			err = 0;
			break;
		}

		err = errno;
		if (err != EINTR) {
			break;
		}
	}

	return err;
}

/*
 * int
 * pfc_flock_getowner(pfc_flock_t lk, pid_t *ownerp, pfc_bool_t writer)
 *	Determine the lock owner process of the specified lock handle.
 *
 *	A value specified by `writer' affects the result.
 *	If PFC_TRUE is passed to `writer', pfc_flock_getowner() determines the
 *	owner by testing whether the current process can hold the writer lock
 *	of the file. If one or more processes is holding the reader lock,
 *	one process ID of them is set to `ownerp'.
 *	Otherwise, pfc_flock_getowner() determines the owner by testing
 *	whether the current process can hold the reader lock of the file.
 *	If a process is holding the writer lock, its process ID is set to
 *	`ownerp'.
 *
 * Calling/Exit State:
 *	If no one holds the lock, zero is set to `*ownerp' and zero is
 *	returned. If another process is holding the lock, process ID of the
 *	owner process is set to `*ownerp' and zero is returned.
 *
 *	Otherwise zero is set to `*ownerp' and error number which indicates
 *	the cause of error is returned.
 */
int
pfc_flock_getowner(pfc_flock_t lk, pid_t *ownerp, pfc_bool_t writer)
{
	return flock_get_owner((int)lk, (writer) ? F_WRLCK : F_RDLCK, ownerp);
}

/*
 * static int
 * flock_do_lock(int fd, short type, pid_t *PFC_RESTRICT ownerp)
 *	Try to acquire the file record lock.
 */
static int
flock_do_lock(int fd, short type, pid_t *PFC_RESTRICT ownerp)
{
	int	err, cmd;

	if (ownerp == NULL) {
		/* Wait for the lock. */
		cmd = F_SETLKW;
	}
	else {
		/* Return EAGAIN if we lost the race. */
		*ownerp = 0;
		cmd = F_SETLK;
	}

	while (1) {
		struct flock	flock;

		/* Try to acquire the file lock. */
		FLOCK_INIT(&flock, type);
		if (PFC_EXPECT_TRUE(fcntl(fd, cmd, &flock) != -1)) {
			/* Succeeded. */
			err = 0;
			break;
		}

		err = errno;
		if (err == EINTR) {
			if (cmd == F_SETLK) {
				continue;
			}
			break;
		}
		if (PFC_EXPECT_FALSE(err != EACCES && err != EAGAIN)) {
			break;
		}

		if (ownerp != NULL) {
			/* Determine who holds the lock. */
			err = flock_get_owner(fd, type, ownerp);
			if (PFC_EXPECT_FALSE(err != 0)) {
				break;
			}

			if (*ownerp != 0) {
				err = EAGAIN;
				break;
			}
		}
	}

	return err;
}

/*
 * static int
 * flock_get_owner(int fd, short type, pid_t *ownerp)
 *	Determine the lock owner process of the specified file descriptor.
 *
 *	Lock owner is determined by fcntl(F_GETLK) with the lock type
 *	specified by `type'. So the owner PID may be changed by the lock type.
 *
 * Calling/Exit State:
 *	If no one holds the lock, zero is set to `*ownerp' and zero is
 *	returned. If another process is holding the lock, process ID of the
 *	owner process is set to `*ownerp' and zero is returned.
 *
 *	Otherwise zero is set to `*ownerp' and error number which indicates
 *	the cause of error is returned.
 */
static int
flock_get_owner(int fd, short type, pid_t *ownerp)
{
	int	err;

	*ownerp = 0;
	while (1) {
		struct flock	flock;

		/* Get the file lock status. */
		FLOCK_INIT(&flock, type);
		if (PFC_EXPECT_TRUE(fcntl(fd, F_GETLK, &flock) != -1)) {
			err = 0;
			if (flock.l_type != F_UNLCK) {
				*ownerp = flock.l_pid;
			}
			break;
		}

		err = errno;
		if (err != EINTR) {
			break;
		}
	}

	return err;
}
