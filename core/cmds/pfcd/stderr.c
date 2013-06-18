/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * stderr.c - stderr logging.
 */

#include <pfc/util.h>
#include "pfcd.h"

/*
 * Minimum length of stderr log directory path.
 */
#define	PFCD_STDERR_LOGDIRLEN_MIN	PFC_CONST_U(2)

/*
 * Name of stderr log file.
 */
#define	PFCD_STDERR_LOG_NAME		"pfcd_err.log"

/*
 * Permission of stderr log file.
 */
#define	STDERR_LOG_PERM		(S_IRUSR | S_IWUSR)

/*
 * Default value of stderr_rotate.
 */
#define	PFCD_STDERR_ROTATE_DEFAULT	10U

/*
 * void
 * stderr_init(int nullfd)
 *	Initialize the standard error output for the daemon process.
 *
 *	`nullfd' must be a file descriptor associated with /dev/null.
 *	Note that it is closed on return unless it is 2.
 */
void
stderr_init(int nullfd)
{
	int		dfd, fd, err;
	size_t		len;
	const char	*logdir = pfcd_stderr_logdir;
	uint32_t	rotate;

	len = strlen(logdir);
	if (PFC_EXPECT_FALSE(len < PFCD_STDERR_LOGDIRLEN_MIN)) {
		pfc_log_info("Disable stderr logging.");

		/* Redirect stderr to /dev/null. */
		if (nullfd != 2) {
			if (PFC_EXPECT_FALSE(dup2(nullfd, 2) == -1)) {
				pfc_log_fatal("dup2(stderr) failed: %s",
					      strerror(errno));
				/* NOTREACHED */
			}

			(void)close(nullfd);
		}

		return;
	}

	(void)close(nullfd);

	/* Create logging directory if it does not exist. */
	err = pfc_mkdir(logdir, PFCD_DIR_PERM);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Unable to create stderr logging directory"
			      ": %s: %s", logdir, strerror(err));
		/* NOTREACHED */
	}

	/* Ensure that the logging directory is safe. */
	path_verify(logdir);

	rotate = pfc_conf_get_uint32(pfcd_options, "stderr_rotate",
				     PFCD_STDERR_ROTATE_DEFAULT);
	dfd = pfc_open_cloexec(logdir, O_RDONLY);
	if (PFC_EXPECT_FALSE(dfd == -1)) {
		pfc_log_fatal("%s: Failed to open stderr log directory: %s",
			      logdir, strerror(errno));
		/* NOTREACHED */
	}

	/* Open stderr log file after rotating existing files. */
	fd = pfc_frotate_openat(dfd, PFCD_STDERR_LOG_NAME, STDERR_LOG_PERM,
				0, rotate);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		pfc_log_fatal("%s/%s: Failed to open stderr log: %s",
			      logdir, PFCD_STDERR_LOG_NAME, strerror(errno));
		/* NOTREACHED */
	}
	(void)close(dfd);

	if (fd != 2) {
		if (PFC_EXPECT_FALSE(dup2(fd, 2) == -1)) {
			pfc_log_fatal("dup2(stderr) failed: %s",
				      strerror(errno));
			/* NOTREACHED */
		}
		(void)close(fd);
	}
}
