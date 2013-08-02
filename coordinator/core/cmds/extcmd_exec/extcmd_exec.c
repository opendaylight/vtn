/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * extcmd_exec.c - Simple execv(3) wrapper for pfc_extcmd_t.
 *
 * This command closes unused file descriptors, and then execute command
 * specified by command line argument.
 *
 * Remarks:
 *	This program should not link libpfc_util for performance reason.
 *	libpfc_util has library constructor which is unnecessary for
 *	extcmd_exec.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <extcmd_impl.h>

/*
 * Program name.
 */
#define	PROGNAME		"extcmd_exec"

/* Directory path that keeps active file descriptors of the current process. */
#define	PROC_SELF_FD	"/proc/self/fd"

/*
 * Substring of valgrind library path.
 */
static const char	valgrind_keyword[] = "vgpreload";

static int	ctrl_fd = -1;

/*
 * Internal prototypes.
 */
static void	fatal(int err, const char *fmt, ...)
	PFC_FATTR_NORETURN PFC_FATTR_PRINTFLIKE(2, 3);
static void	ctrl_fd_init(void);
static void	close_files(int lowfd);
static int	convert_fd(const char *str);

int
main(int argc, char **argv)
{
	const char	*path = NULL;
	char		**newargv, **pp;
	int		c;

	while ((c = getopt(argc, argv, "c")) != EOF) {
		if (c == 'c') {
			/* Use control fd. */
			ctrl_fd_init();
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * The first argument must be a path to external command.
	 * In addition, argv[0] for external command must be specified.
	 */
	if (PFC_EXPECT_FALSE(argc < 2)) {
		fatal(EINVAL, "No command path is specified.");
		/* NOTREACHED */
	}

	path = *argv;
	argc--;
	argv++;

	/*
	 * We must copy argument vector because execv(3) requires NULL pointer
	 * at the end of argument vector.
	 */
	newargv = (char **)malloc(sizeof(char *) * (argc + 1));
	if (PFC_EXPECT_FALSE(newargv == NULL)) {
		fatal(ENOMEM, "Failed to copy arguments.");
		/* NOTREACHED */
	}
	for (pp = newargv; argc > 0; pp++, argc--, argv++) {
		*pp = *argv;
	}
	*pp = NULL;

	/* Close all file descriptors greater than or equal to 3. */
	close_files(3);

	/* Execute external command. */
	execv(path, (char *const *)newargv);

	fatal(errno, "execv(%s) failed.", path);
	/* NOTREACHED */
}

/*
 * static void
 * fatal(int err, const char *fmt, ...)
 *	Print error message and die.
 */
static void
fatal(int err, const char *fmt, ...)
{
	struct sigaction	sact;
	va_list		ap;

	/* Ignore SIGPIPE. */
	sact.sa_handler = SIG_IGN;
	sact.sa_flags = 0;
	sigemptyset(&sact.sa_mask);
	(void)sigaction(SIGPIPE, &sact, NULL);

	va_start(ap, fmt);

	if (ctrl_fd != -1) {
		char	buf[EXTCMD_ERRMSG_SIZE];
		int	len;

		/* Send error number. */
		(void)write(ctrl_fd, &err, sizeof(err));

		/* Send error message to control fd. */
		len = vsnprintf(buf, sizeof(buf), fmt, ap);
		if (PFC_EXPECT_TRUE(len > 0)) {
			if (PFC_EXPECT_FALSE((size_t)len >= sizeof(buf))) {
				len = sizeof(buf) - 1;
			}
			(void)write(ctrl_fd, buf, len);
			(void)close(ctrl_fd);
		}
	}
	else {
		/* Send error message to the standard error output. */
		fputs("*** ERROR: ", stderr);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": %s\n", strerror(err));
	}

	va_end(ap);

	exit(EXTCMD_ERRST_EXEC);
	/* NOTREACHED  */
}

/*
 * static void
 * ctrl_fd_init(void)
 *	Initialize control file descriptor.
 */
static void
ctrl_fd_init(void)
{
	int	flag;

	/* Set close-on-exec flag. */
	flag = fcntl(EXTCMD_CTRL_FD, F_GETFD);
	if (PFC_EXPECT_FALSE(flag == -1)) {
		return;
	}
	if ((flag & FD_CLOEXEC) == 0) {
		flag |= FD_CLOEXEC;
		if (PFC_EXPECT_FALSE(fcntl(EXTCMD_CTRL_FD, F_SETFD,
					   (long)flag) != 0)) {
			return;
		}
	}

	ctrl_fd = EXTCMD_CTRL_FD;
}

/*
 * static void
 * close_files(int lowfd)
 *	Close all file descriptors greater than or equal to the specified
 *	number.
 */
static void
close_files(int lowfd)
{
	DIR		*dirp;
	struct dirent	*dp;
	int		dfd;
	char		*preload;
	pfc_bool_t	valgrind_mode;

	/*
	 * Determine whether the program is launched by valgrind or not.
	 * If valgrind library is pre-loaded, we assume that we're on the
	 * valgrind world.
	 */
	preload = getenv("LD_PRELOAD");
	if (preload != NULL && strstr(preload, valgrind_keyword) != NULL) {
		valgrind_mode = PFC_TRUE;
	}

	dirp = opendir(PROC_SELF_FD);
	if (PFC_EXPECT_FALSE(dirp == NULL)) {
		return;
	}

	dfd = dirfd(dirp);
	if (PFC_EXPECT_FALSE(dfd == -1)) {
		goto out;
	}

	for (;;) {
		pfc_bool_t	closed = PFC_FALSE;

		while ((dp = readdir(dirp)) != NULL) {
			int	fd;

			if (dp->d_name[0] == '.') {
				continue;
			}

			fd = convert_fd(dp->d_name);
			if (PFC_EXPECT_FALSE(fd == -1)) {
				continue;
			}

			/*
			 * Don't close /proc file descriptor and control
			 * file descriptor.
			 */
			if (fd != dfd && fd != ctrl_fd && fd >= lowfd) {
				(void)close(fd);

				/*
				 * Closing file descriptor may affect
				 * to the /proc/self/fd directory entry.
				 * So one more check is needed.
				 */
				closed = PFC_TRUE;
			}
		}

		/*
		 * If we are running on the valgrind, we should not loop here
		 * because the valgrind uses some file descriptors internally,
		 * and they are protected against close(2).
		 */
		if (!closed || valgrind_mode) {
			break;
		}

		rewinddir(dirp);
	}

out:
	closedir(dirp);
}

/*
 * static int
 * convert_fd(const char *str)
 *	Convert string representation of file descriptor into integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, valid file descriptor is returned.
 *	-1 is returned on error.
 */
static int
convert_fd(const char *str)
{
	const uint8_t	*p;
	int		fd = 0;

	for (p = (const uint8_t *)str; *p != '\0'; p++) {
		char	c = *p;
		int	digit, next;

		digit = c - '0';
		if (PFC_EXPECT_FALSE(digit < 0 || digit > 9)) {
			return -1;
		}

		next = fd * 10 + digit;
		if (PFC_EXPECT_FALSE(next < fd)) {
			return -1;
		}

		fd = next;
	}

	return fd;
}
