/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Sub command which is used by libpfc_util/cloexec.c tests.
 * This command takes file descriptors as argument, and tries to write
 * "sub_cloexec:<process ID>:<file descriptor>" to each descriptor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include "sub_cloexec.h"

/*
 * Pipe management.
 */
struct test_pipe;
typedef struct test_pipe	test_pipe_t;

struct test_pipe {
	int		t_fd;
	char		t_output[SUB_CLOEXEC_MAX_OUTSIZE];
	char		*t_cursor;
	size_t		t_outsize;
	uint32_t	t_flags;
};

static test_pipe_t	*Pipes;
static size_t		NumPipes;
static struct pollfd	*PollFds;
static pid_t		Pid;

#define	TPF_INVALID	0x1U
#define	TPF_DONE	0x2U

/*
 * Internal prototypes.
 */
static void	setup(int argc, char **argv);
static void	pipe_init(test_pipe_t *tp, char *arg);
static void	run(void);
static nfds_t	collect_pipe(void);
static void	do_output(nfds_t nfds);
static void	process_pipe(test_pipe_t *tp, struct pollfd *pfd);

int
main(int argc, char **argv)
{
	argv++;
	argc--;
	setup(argc, argv);
	run();

	return 0;
}

static void
setup(int argc, char **argv)
{
	struct sigaction	act;
	test_pipe_t	*tp;

	if (argc <= 0) {
		fprintf(stderr, "No argument is specified.\n");

		exit(1);
	}

	Pid = getpid();

	/* Ignore SIGPIPE. */
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGPIPE, &act, NULL) == -1) {
		fprintf(stderr, "sigaction() failed: %s\n", strerror(errno));

		exit(1);
	}

	/* Allocate pipe array. */
	Pipes = (test_pipe_t *)malloc(sizeof(test_pipe_t) * argc);
	if (Pipes == NULL) {
		fprintf(stderr, "Failed to allocate pipe array.\n");

		exit(1);
	}

	/* Allocate pollfd array. */
	PollFds = (struct pollfd *)malloc(sizeof(struct pollfd) * argc);
	if (PollFds == NULL) {
		fprintf(stderr, "Failed to allocate pollfd array.\n");

		exit(1);
	}

	NumPipes = argc;

	for (tp = Pipes; argc > 0; argc--, argv++, tp++) {
		pipe_init(tp, *argv);
	}
}

static void
pipe_init(test_pipe_t *tp, char *arg)
{
	char	*p;
	int	fd, len, fflags;
	unsigned long	v;

	errno = 0;
	v = strtoul(arg, &p, 10);
	if (*p != '\0') {
		fprintf(stderr, "Invalid character in argument: %s\n", arg);

		exit(1);
	}
	if (errno != 0) {
		fprintf(stderr, "Invalid file descriptor: %s\n", arg);

		exit(1);
	}

	fd = tp->t_fd = (int)v;
	len = snprintf(tp->t_output, SUB_CLOEXEC_MAX_OUTSIZE, "%s:%d:%d",
		       SUB_CLOEXEC_NAME, Pid, fd);
	if (len >= SUB_CLOEXEC_MAX_OUTSIZE) {
		fprintf(stderr, "Internal buffer overflow: %s:%d:%d\n",
			SUB_CLOEXEC_NAME, Pid, fd);

		exit(1);
	}
	tp->t_cursor = tp->t_output;
	tp->t_outsize = len;
	tp->t_flags = 0;

	/* Make this pipe non-blocking. */
	fflags = fcntl(fd, F_GETFL);
	if (fflags == -1) {
		/*
		 * Don't change this error message because it is referred by
		 * unit test code.
		 */
		fprintf(stderr, "fcntl(%d, F_GETFL) failed: errno=%d\n",
			fd, errno);
		tp->t_flags = TPF_INVALID;

		return;
	}

	if ((fflags & O_NONBLOCK) == 0) {
		if (fcntl(fd, F_SETFL, (long)(fflags | O_NONBLOCK)) == -1) {
			fprintf(stderr, "fcntl(F_SETFL) failed: %s\n",
				strerror(errno));

			exit(1);
		}
	}
}

static void
run(void)
{
	nfds_t	nfds;

	while ((nfds = collect_pipe()) != 0) {
		do_output(nfds);
	}
}

static nfds_t
collect_pipe(void)
{
	test_pipe_t	*tp;
	struct pollfd	*pfd = PollFds;

	for (tp = Pipes; tp < Pipes + NumPipes; tp++) {
		if ((tp->t_flags & (TPF_INVALID | TPF_DONE)) == 0) {
			pfd->fd = tp->t_fd;
			pfd->events = POLLOUT;
			pfd->revents = 0;
			pfd++;
		}
	}

	return (nfds_t)(pfd - PollFds);
}

static void
do_output(nfds_t nfds)
{
	struct pollfd	*pfd;
	sigset_t	empty;
	struct timespec	to;

	sigemptyset(&empty);
	to.tv_sec = SUB_CLOEXEC_IO_TIMEOUT;
	to.tv_nsec = 0;

	for (;;) {
		int	ret = ppoll(PollFds, nfds, &to, &empty);

		if (ret == -1) {
			int	err = errno;

			if (err == EINTR) {
				continue;
			}

			fprintf(stderr, "ppoll() failed: %s\n", strerror(err));

			exit(1);
		}
		if (ret == 0) {
			fprintf(stderr, "Timed out.\n");

			exit(1);
		}

		break;
	};

	for (pfd = PollFds; pfd < PollFds + nfds; pfd++) {
		test_pipe_t	*tp;

		for (tp = Pipes; tp < Pipes + NumPipes; tp++) {
			if (tp->t_fd == pfd->fd) {
				process_pipe(tp, pfd);
			}
		}
	}
}

static void
process_pipe(test_pipe_t *tp, struct pollfd *pfd)
{
	size_t	size;

	if (pfd->revents == 0) {
		return;
	}

	if (pfd->revents & POLLERR) {
		fprintf(stderr, "Error on pipe: %d\n", tp->t_fd);
		tp->t_flags |= TPF_INVALID;

		return;
	}
	if (pfd->revents & POLLHUP) {
		fprintf(stderr, "Hang up: %d", tp->t_fd);
		tp->t_flags |= TPF_INVALID;

		return;
	}

	size = tp->t_outsize;
	while (size > 0) {
		ssize_t	nbytes = write(tp->t_fd, tp->t_cursor, size);

		if (nbytes == -1) {
			int	err = errno;

			if (err == EINTR) {
				continue;
			}
			if (err != EAGAIN) {
				fprintf(stderr, "write() failed: %s\n",
					strerror(err));
				tp->t_flags |= TPF_INVALID;
			}

			return;
		}

		size -= nbytes;
		tp->t_cursor += nbytes;
	}

	tp->t_outsize = size;
	if (size == 0) {
		tp->t_flags |= TPF_DONE;
	}
}
