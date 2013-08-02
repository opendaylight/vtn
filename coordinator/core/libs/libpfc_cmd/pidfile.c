/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * pidfile.c - PID file management.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <pfc/path.h>
#include <pfc/debug.h>
#include <pfc/conf.h>
#include <pfc/util.h>
#include <pfc/clock.h>
#include "cmdutil.h"
#include "cmd_impl.h"

/* Permission of PID file. */
#define	PIDFILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * String length enough to store string representation of process ID.
 */
#define	PIDFILE_BUFSIZE		16

/*
 * How long, in seconds, pidfile_install() should wait for the PID file lock.
 */
#define	PIDFILE_LOCK_TIMEOUT	3		/* 3 seconds */

/*
 * Delay time, in nanoseconds, in pidfile_wait_lock().
 */
#define	PIDFILE_LOCK_DELAY	1000000		/* 1 millisecond */

/*
 * Internal prototypes.
 */
static int	pidfile_do_open(pfc_pidf_t *PFC_RESTRICT pfp,
				const char *PFC_RESTRICT path, int omode);
static int	pidfile_wait_lock(pfc_pidf_t pf, pid_t *ownerp);

/*
 * int
 * pidfile_open(pfc_pidf_t *PFC_RESTRICT pfp, const char *PFC_RESTRICT path)
 *	Open PID file for writing.
 *
 *	This function also checks whether the specified PID file path is safe.
 *	If NULL is specified to `path', it is derived from the PFC system
 *	configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, PID file handle is set to `*pfp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pidfile_open(pfc_pidf_t *PFC_RESTRICT pfp, const char *PFC_RESTRICT path)
{
	return pidfile_do_open(pfp, path, O_RDWR | O_CREAT);
}

/*
 * int
 * pidfile_open_rdonly(pfc_pidf_t *PFC_RESTRICT pfp,
 *		       const char *PFC_RESTRICT path)
 *	Open PID file for reading.
 *
 *	Unlike pidfile_open(), safe path check for PID file path is omitted.
 *	If NULL is specified to `path', it is derived from the PFC system
 *	configuration file.
 *
 * Calling/Exit State:
 *	Upon successful completion, PID file handle is set to `*pfp', and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pidfile_open_rdonly(pfc_pidf_t *PFC_RESTRICT pfp, const char *PFC_RESTRICT path)
{
	return pidfile_do_open(pfp, path, O_RDONLY);
}

/*
 * static int
 * pidfile_do_open(pfc_pidf_t *PFC_RESTRICT pfp, const char *PFC_RESTRICT path,
 *		   int omode)
 *	Open PID file with specifying open mode.
 *
 *	Open mode specifies the mode in the same way as open(2). If O_RDWR
 *	is specified, PID file is opened in read-write mode. If you want to
 *	create a new PID file, O_CREAT flag needs to be set to open mode.
 *	If O_RDONLY is specified, PID file is opened in read only mode.
 */
static int
pidfile_do_open(pfc_pidf_t *PFC_RESTRICT pfp, const char *PFC_RESTRICT path,
		int omode)
{
	pfc_flock_t	lk;
	pfc_bool_t	writable;
	int	err;

	if (path == NULL) {
		path = pidfile_default_path();
	}

	if ((omode & O_ACCMODE) != O_RDONLY) {
		char	*cpath, *sep;

		/*
		 * Safe PID file path is required for writable mode.
		 * Note that we can't pass the specified path to
		 * pfc_is_safepath() because it may not exist.
		 */
		writable = PFC_TRUE;
		cpath = strdup(path);
		if (PFC_EXPECT_FALSE(cpath == NULL)) {
			return ENOMEM;
		}

		/* Create path to parent directory of PID file. */
		sep = strrchr(cpath, '/');
		if (PFC_EXPECT_FALSE(sep == NULL)) {
			/* Relative path is not allowed. */
			free(cpath);

			return EINVAL;
		}
		*sep = '\0';

		/* Ensure parent directory is safe. */
		err = pfc_is_safepath(cpath);
		free(cpath);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		writable = PFC_FALSE;
	}

	/*
	 * Open PID file.
	 *
	 * Remarks:
	 *	Current implementation of pfc_flock_open() sets a file
	 *	descriptor to `lk'.
	 */
	err = pfc_flock_open(&lk, path, omode, PIDFILE_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Ensure the permission is safe. */
	if (writable) {
		if (PFC_EXPECT_FALSE(fchmod((int)lk, PIDFILE_PERM))) {
			err = errno;
			pfc_flock_close(lk);

			return err;
		}
	}

	*pfp = lk;

	return err;
}

/*
 * void
 * pidfile_close(pfc_pidf_t pf)
 *	Close PID file handle.
 */
void
pidfile_close(pfc_pidf_t pf)
{
	pfc_flock_close((pfc_flock_t)pf);
}

/*
 * int
 * pidfile_install(pfc_pidf_t pf, pid_t *ownerp)
 *	Install current process ID to the PID file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If another process is holding the PID file lock, process ID of the
 *	owner process is set to `*ownerp' and EAGAIN is returned.
 *
 *	If one or more other processes are holding the reader lock,
 *	pidfile_install() waits for the writer lock.
 *	ETIMEDOUT is returned if the current process can't acquire the
 *	writer lock within PIDFILE_LOCK_TIMEOUT seconds.
 *
 *	Otherwise zero is set to `*ownerp' and error number which indicates
 *	the cause of error is returned.
 */
int
pidfile_install(pfc_pidf_t pf, pid_t *ownerp)
{
	int	err;
	pid_t	pid;
	char	buf[PIDFILE_BUFSIZE], *p;
	size_t	size;
	off_t	off;

	/* Acquire exclusive file record lock. */
	err = pfc_flock_wrlock((pfc_flock_t)pf, ownerp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != EAGAIN ||
		    (err = pidfile_wait_lock(pf, ownerp)) != 0) {
			return err;
		}
	}

	/* Truncate PID file. */
	if (PFC_EXPECT_FALSE(ftruncate(pf, 0) == -1)) {
		err = errno;
		(void)pfc_flock_unlock((pfc_flock_t)pf);

		return err;
	}

	/* Write current PID. */
	pid = getpid();
	size = (size_t)snprintf(buf, sizeof(buf), "%d\n", pid);
	PFC_ASSERT(size < sizeof(buf));

	p = buf;
	off = 0;
	do {
		ssize_t	sz = pwrite(pf, p, size, off);

		if (sz == -1) {
			err = errno;
			if (err == EINTR) {
				err = 0;
				continue;
			}
			(void)pfc_flock_unlock((pfc_flock_t)pf);
			break;
		}
		size -= sz;
		p += sz;
		off += sz;
	} while (size > 0);

	return err;
}

/*
 * static int
 * pidfile_wait_lock(pfc_pidf_t pf, pid_t *ownerp)
 *	Wait until the current process gets the writer lock of the PID file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If another process is holding the writer lock of the PID file, process
 *	ID of the owner process is set to `*ownerp' and EAGAIN is returned.
 *
 *	If one or more other processes are holding the reader lock,
 *	pidfile_install() waits for the writer lock.
 *	ETIMEDOUT is returned if the current process can't acquire the
 *	writer lock within PIDFILE_LOCK_TIMEOUT seconds.
 *
 *	Otherwise zero is set to `*ownerp' and error number which indicates
 *	the cause of error is returned.
 *
 * Remarks:
 *	This function is designed to be called only if the call of
 *	pfc_flock_wrlock() in pidfile_install() returns EAGAIN.
 */
static int
pidfile_wait_lock(pfc_pidf_t pf, pid_t *ownerp)
{
	pfc_flock_t	lk = (pfc_flock_t)pf;
	pfc_timespec_t	expire;
	struct timespec	delay;
	int		err;

	PFC_ASSERT(ownerp != NULL);

	/* Reset lock owner. */
	*ownerp = 0;

	/* Set expiry time. */
	err = pfc_clock_gettime(&expire);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	expire.tv_sec += PIDFILE_LOCK_TIMEOUT;

	delay.tv_sec = 0;
	delay.tv_nsec = PIDFILE_LOCK_DELAY;

	for (;;) {
		pfc_timespec_t	cur;
		pid_t		pid;

		/*
		 * Check to see whether the lock owner holds the writer
		 * lock.
		 */
		err = pfc_flock_getowner(lk, &pid, PFC_FALSE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		if (pid != 0) {
			/* The owner holds the writer lock. */
			err = EAGAIN;
			*ownerp = pid;
			break;
		}

		/* Try to acquire the writer lock. */
		err = pfc_flock_wrlock(lk, &pid);
		if (err != EAGAIN) {
			/*
			 * Succeeded, or fatal error.
			 * We don't need to copy pid to *ownerp because this
			 * is not EAGAIN error.
			 */
			break;
		}

		/* Obtain current time. */
		err = pfc_clock_gettime(&cur);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
		if (pfc_clock_compare(&cur, &expire) > 0) {
			/* Timed out. */
			err = ETIMEDOUT;
			break;
		}

		nanosleep(&delay, NULL);
	}

	return err;
}

/*
 * int
 * pidfile_getowner(pfc_pidf_t pf, pid_t *ownerp)
 *	Determine the lock owner process of the PID file.
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
pidfile_getowner(pfc_pidf_t pf, pid_t *ownerp)
{
	/*
	 * The owner of PID file holds the writer lock. So we must determine
	 * the owner by testing whether we can acquire the reader lock.
	 */
	return pfc_flock_getowner((pfc_flock_t)pf, ownerp, PFC_FALSE);
}

/*
 * void
 * pidfile_unlink(const char *path)
 *	Unlink PID file.
 *	If NULL is specified to `path', it is derived from the PFC system
 *	configuration file.
 */
void
pidfile_unlink(const char *path)
{
	if (path == NULL) {
		path = pidfile_default_path();
	}

	(void)unlink(path);
}

/*
 * const char *
 * pidfile_default_path(void)
 *	Return default PID file path.
 */
const char *
pidfile_default_path(void)
{
	pfc_cfblk_t	options;

	options = pfccmd_conf_options();

	return pfc_conf_get_string(options, conf_pid_file, PFC_PFCD_PID_PATH);
}
