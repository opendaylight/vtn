/*
 * Copyright (c) 2010-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Sub command which is used by libpfc_util/flock.c tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <cmdopt.h>
#include <pfc/util.h>
#include "sub_flock.h"

#define	NANOSEC		1000000000
#define	MILLISEC	1000
#define	MILLI2NANO	(NANOSEC / MILLISEC)

#define	HELP_MESSAGE	"Sub command for flock test."

static const pfc_cmdopt_def_t	option_spec[] = {
	{'w', "wrlock", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Acquire writer lock.", NULL},
	{'n', "notify", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Send SIGUSR1 to parent just after locking.", NULL},
	{'s', "signal", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Send SIGUSR1 to parent while holding the lock.", NULL},
	{'p', "pause", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Pause after unlocking.", NULL},
	{'P', "pause-signal", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Pause after unlocking with sending SIGUSR1.", NULL},
	{'c', "check", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Ensure that the file contents is not changed.", NULL},
	{'T', "time", PFC_CMDOPT_TYPE_UINT32, PFC_CMDOPT_DEF_ONCE,
	 "How long, in seconds, we should hold file lock.\n"
	 "(default: 10 seconds)", "SECONDS"},
	
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

#define	FLK_WRITER		0x1U
#define	FLK_NOTIFY		0x2U
#define	FLK_SIGNAL		0x4U
#define	FLK_PAUSE		0x8U
#define	FLK_PAUSE_SIGNAL	0x10U
#define	FLK_CHECK_CONTENTS	0x20U

/*
 * Internal prototypes.
 */
static void	setup();
static void	quit(int sig);
static void	do_lock(const char *path, uint32_t flags, uint32_t holdtime);
static void	fatal(const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(1, 2) PFC_FATTR_NORETURN;
static char	*get_contents(int fd, size_t *sizep);

int
main(int argc, char **argv)
{
	pfc_cmdopt_t	*parser;
	uint32_t	holdtime = 100, flags = 0;
	int	argidx;
	char	c;

	setup();

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(SUB_FLOCK_NAME, argc, argv, option_spec,
				 "file", 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		fatal("Failed to create option parser.");
		/* NOTREACHED */
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'w':
			flags |= FLK_WRITER;
			break;

		case 'n':
			flags |= FLK_NOTIFY;
			break;

		case 's':
			flags |= FLK_SIGNAL;
			break;

		case 'p':
			flags |= FLK_PAUSE;
			break;

		case 'P':
			flags |= (FLK_PAUSE | FLK_PAUSE_SIGNAL);
			break;

		case 'c':
			flags |= FLK_CHECK_CONTENTS;
			break;

		case 'T':
			holdtime = pfc_cmdopt_arg_uint32(parser);
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stderr);
			exit(1);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help(parser, stderr, HELP_MESSAGE);
			exit(1);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			exit(1);
			/* NOTREACHED */

		default:
			fatal("Failed to parse command line options.");
			/* NOTREACHED */
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		fatal("Invalid command line options.");
		/* NOTREACHED */
	}

	argc -= argidx;
	argv += argidx;
	if (argc == 0) {
		fatal("No lock file is specified.");
		/* NOTREACHED */
	}

	do_lock(*argv, flags, holdtime);

	return 0;
}

static void
setup()
{
	struct sigaction	act;
	sigset_t		empty;

	act.sa_handler = quit;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTERM, &act, NULL) != 0) {
		fatal("sigaction() failed: %s\n", strerror(errno));
		/* NOTREACHED */
	}

	sigemptyset(&empty);
	if (sigprocmask(SIG_SETMASK, &empty, NULL) != 0) {
		fatal("sigprocmask() failed: %s\n", strerror(errno));
		/* NOTREACHED */
	}
}

static void
quit(int sig)
{
	/* Remarks: exit() is not async-signal-safe. */
	_exit(0);
}

static void
do_lock(const char *path, uint32_t flags, uint32_t holdtime)
{
	pfc_flock_t	lk;
	int		err;
	pid_t		pid = getppid();
	char		*contents = NULL;
	size_t		contents_size = 0;

	err = pfc_flock_open(&lk, path, O_RDWR | O_CREAT, 0644);
	if (err != 0) {
		fatal("pfc_flock_open() failed: %s", strerror(err));
		/* NOTREACHED */
	}

	if (flags & FLK_WRITER) {
		err = pfc_flock_wrlock(lk, NULL);
	}
	else {
		err = pfc_flock_rdlock(lk, NULL);
	}

	if (err != 0) {
		fatal("Failed to acquire lock: %s", strerror(err));
		/* NOTREACHED */
	}

	if (flags & FLK_NOTIFY) {
		(void)kill(pid, SIGUSR1);
	}

	if (flags & FLK_CHECK_CONTENTS) {
		contents = get_contents((int)lk, &contents_size);
	}

	if (holdtime != 0) {
		struct timespec	ts;
		if (flags & FLK_SIGNAL) {
			uint32_t	i;

			ts.tv_sec = 0;
			ts.tv_nsec = MILLI2NANO;

			for (i = 0; i < holdtime; i++) {
				nanosleep(&ts, NULL);
				(void)kill(pid, SIGUSR1);
			}
		}
		else {
			pfc_ulong_t	nsec =
				(pfc_ulong_t)holdtime * MILLI2NANO;
			pfc_ulong_t	div = nsec / NANOSEC;

			ts.tv_sec = div;
			ts.tv_nsec = nsec - (div * NANOSEC);
			nanosleep(&ts, NULL);
		}
	}

	if (flags & FLK_CHECK_CONTENTS) {
		char	*c;
		size_t	size;

		c = get_contents((int)lk, &size);
		if (size != contents_size) {
			fatal("Size of file was changed: %" PFC_PFMT_SIZE_T
			      " => %" PFC_PFMT_SIZE_T "\n",
			      contents_size, size);
			/* NOTREACHED */
		}
		if (size != 0 && memcmp(contents, c, size) != 0) {
			fatal("File contents was changed.\n");
			/* NOTREACHED */
		}
		free(c);
		free(contents);
	}

	if (flags & FLK_PAUSE) {
		err = pfc_flock_unlock(lk);
		if (err != 0) {
			fatal("pfc_flock_unlock() failed: %s", strerror(err));
			/* NOTREACHED */
		}

		if (flags & FLK_PAUSE_SIGNAL) {
			struct timespec	ts;

			ts.tv_sec = 0;
			ts.tv_nsec = 1000;		/* 1 microsecond */

			for (;;) {
				nanosleep(&ts, NULL);
				(void)kill(pid, SIGUSR1);
			}
		}
		else {
			pause();
		}
	}
}

static void
fatal(const char *fmt, ...)
{
	va_list	ap;

	fputs("*** ERROR: ", stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);

	exit(1);
	/* NOTREACHED */
}

static char *
get_contents(int fd, size_t *sizep)
{
	char	*contents = NULL;
	size_t	size = 0;

	for (;;) {
		char	buf[64];
		ssize_t	sz = pread(fd, buf, sizeof(buf), (off_t)size);

		if (sz == -1) {
			fatal("read() failed: %s\n", strerror(errno));
			/* NOTREACHED */
		}
		if (sz == 0) {
			break;
		}

		contents = (char *)realloc(contents, size + sz);
		if (contents == NULL) {
			fatal("realloc() failed.\n");
			/* NOTREACHED */
		}
		memcpy(contents + size, buf, sz);
		size += sz;
	}

	*sizep = size;

	return contents;
}
