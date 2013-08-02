/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cloexec.c - Change close-on-exec flag for the specified file descriptor.
 */

/*
 * Suppress declaration of inlined pfc_open_cloexec(), pfc_openat_cloexec(),
 * pfc_pipe_open(), pfc_dupfd_cloexec(), and pfc_dup2fd_cloexec() in
 * pfc/util.h in order to define them as function.
 */
#define	__PFC_UTIL_DONT_DEFINE_OPEN_CLOEXEC	1
#define	__PFC_UTIL_DONT_DEFINE_PIPE_OPEN	1
#define	__PFC_UTIL_DONT_DEFINE_DUPFD		1
#define	__PFC_UTIL_DONT_DEFINE_DUP2FD		1

#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pfc/util.h>

#ifndef	PFC_HAVE_ATFILE_SYSCALL
#error	Current implementation requires ATFILE system calls.
#endif	/* !PFC_HAVE_ATFILE_SYSCALL */

#ifdef	PFC_HAVE_O_CLOEXEC

/* O_CLOEXEC flag is supported. */
#define	PFC_SET_CLOEXEC(fd, set)	(0)
#define	PFC_OPENFLAGS(flags)		((flags) | O_CLOEXEC)

#else	/* PFC_HAVE_O_CLOEXEC */

#define	PFC_SET_CLOEXEC(fd, set)	pfc_set_cloexec((fd), (set))
#define	PFC_OPENFLAGS(flags)		(flags)

#endif	/* PFC_HAVE_O_CLOEXEC */

/*
 * Maximum length of mode string supported by pfc_fopen_cloexec().
 */
#define	FOPEN_MODE_MAXLEN		5U

/*
 * int
 * pfc_set_cloexec(int fd, pfc_bool_t set)
 *	Change close-on-exec flag for the specified file descriptor.
 *	If `set' is PFC_TRUE, this function set close-on-exec flag,
 *	otherwise clears.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_set_cloexec(int fd, pfc_bool_t set)
{
	int	flag;

	flag = fcntl(fd, F_GETFD);
	if (PFC_EXPECT_FALSE(flag == -1)) {
		return errno;
	}

	if (set) {
		if (flag & FD_CLOEXEC) {
			return 0;
		}
		flag |= FD_CLOEXEC;
	}
	else {
		if (!(flag &  FD_CLOEXEC)) {
			return 0;
		}
		flag &= ~FD_CLOEXEC;
	}

	if (PFC_EXPECT_FALSE(fcntl(fd, F_SETFD, (long)flag) == -1)) {
		return errno;
	}

	return 0;
}

/*
 * int
 * pfc_open_cloexec(const char *path, int flags, ...)
 *	Open a file specified by `path', and set close-on-exec flag.
 *	The same arguments as open(2) must be specified.
 *
 * Calling/Exit State:
 *	Upon successful completion, a file descriptor associated with the
 *	specified file is returned.
 *	On failure, -1 is returned. An appropriate error number is set to
 *	errno.
 */
int
pfc_open_cloexec(const char *path, int flags, ...)
{
	va_list	ap;
	mode_t	mode;
	int	fd, err;

	va_start(ap, flags);
	mode = va_arg(ap, mode_t);
	va_end(ap);

	/* Open file. */
	fd = open(path, PFC_OPENFLAGS(flags), mode);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return fd;
	}

	/* Set close-on-exec flag if O_CLOEXEC is not supported. */
	err = PFC_SET_CLOEXEC(fd, PFC_TRUE);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return fd;
	}

	(void)close(fd);

	/* close() may update errno. */
	errno = err;

	return -1;
}

/*
 * int
 * pfc_openat_cloexec(int dirfd, const char *path, int flags, ...)
 *	Open a file specified by `dirfd' and `path', and set close-on-exec
 *	flag. The same arguments as openat(2) must be specified.
 *
 *	If PFC_AT_FDCWD is specified to `dirfd', `path' is interpreted
 *	relative to the current working directory, just like
 *	pfc_open_cloexec().
 *
 * Calling/Exit State:
 *	Upon successful completion, a file descriptor associated with the
 *	specified file is returned.
 *	On failure, -1 is returned. An appropriate error number is set to
 *	errno.
 */
int
pfc_openat_cloexec(int dirfd, const char *path, int flags, ...)
{
	va_list	ap;
	mode_t	mode;
	int	fd, err;

	va_start(ap, flags);
	mode = va_arg(ap, mode_t);
	va_end(ap);

#if	PFC_AT_FDCWD != AT_FDCWD
	if (dirfd == PFC_AT_FDCWD) {
		dirfd = AT_FDCWD;
	}
#endif	/* PFC_AT_FDCWD != AT_FDCWD */

	/* Open file at the specified directory file descriptor. */
	fd = openat(dirfd, path, PFC_OPENFLAGS(flags), mode);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return fd;
	}

	/* Set close-on-exec flag if O_CLOEXEC is not supported. */
	err = PFC_SET_CLOEXEC(fd, PFC_TRUE);
	if (PFC_EXPECT_TRUE(err == 0)) {
		return fd;
	}

	(void)close(fd);

	/* close() may update errno. */
	errno = err;

	return -1;
}

/*
 * int
 * pfc_pipe_open(int pipefd[2], int flags)
 *	Create a pipe.
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_PIPE_CLOEXEC	Set close-on-exec flag on new descriptors.
 *	PFC_PIPE_NONBLOCK	Set O_NONBLOCK flag on new descriptors.
 *
 * Calling/Exit State:
 *	Upon successful completion, two file descriptors associated with a
 *	new pipe is set to `pipefd', and zero is returned.
 *	On error, an appropriate error number is set to errno and -1 is
 *	returned.
 */
int
pfc_pipe_open(int pipefd[2], int flags)
{
	int	ret;

	if (PFC_EXPECT_FALSE(flags & ~PFC_PIPE_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

#ifdef	PFC_HAVE_PIPE2
	/* We can use pipe2(). */
	{
		int	pflags = 0;

		/* Flag conversion is needed. */
		if (flags & PFC_PIPE_CLOEXEC) {
			pflags |= O_CLOEXEC;
		}
		if (flags & PFC_PIPE_NONBLOCK) {
			pflags |= O_NONBLOCK;
		}

		ret = pipe2(pipefd, pflags);
		if (PFC_EXPECT_TRUE(ret == 0)) {
			PFC_PIPE_FLAGS_ASSERT(pipefd, flags);
		}

		return ret;
	}
#else	/* !PFC_HAVE_PIPE2 */
	int	err;

	ret = pipe(pipefd);
	if (PFC_EXPECT_FALSE(ret == -1)) {
		return -1;
	}

	if (flags & PFC_PIPE_CLOEXEC) {
		int	i;

		/* Set close-on-exec flag. */
		for (i = 0; i < 2; i++) {
			err = pfc_set_cloexec(pipefd[i], PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}
	}

	if (flags & PFC_PIPE_NONBLOCK) {
		int	i;

		/* Set non-blocking flag. */
		for (i = 0; i < 2; i++) {
			err = pfc_set_nonblock(pipefd[i], PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}
	}

	PFC_PIPE_FLAGS_ASSERT(pipefd, flags);

	return 0;

error:
	(void)close(pipefd[0]);
	(void)close(pipefd[1]);
	errno = err;

	return -1;
#endif	/* PFC_HAVE_PIPE2 */
}

/*
 * FILE *
 * pfc_fopen_cloexec(const char *PFC_RESTRICT path,
 *		     const char *PFC_RESTRICT mode)
 *	Open the file specified by the file path `path', and associate a
 *	stream with it. Close-on exec flag is always set to the opened file
 *	descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to stream associated
 *	with the file is returned.
 *	On error, an appropriate error number is set to `errno' and NULL is
 *	returned.
 *
 * Remarks:
 *	`mode' must be a pointer to string whose length is less than 6
 *	characters, otherwise pfc_fopen_cloexec() will cause undefined
 *	behavior.
 */
FILE *
pfc_fopen_cloexec(const char *PFC_RESTRICT path, const char *PFC_RESTRICT mode)
{
#ifdef	PFC_FOPEN_SUPPORTS_E
	char		*mp, modebuf[FOPEN_MODE_MAXLEN + 2];
	pfc_bool_t	eflag = PFC_FALSE;

	/* Embed 'e' in mode string. */
	for (mp = modebuf; mp < PFC_ARRAY_LIMIT(modebuf) - 2; mode++, mp++) {
		char	c = *mode;

		if (c == '\0') {
			break;
		}
		if (c == 'e') {
			eflag = PFC_TRUE;
		}
		*mp = c;
	}

	if (!eflag) {
		*mp = 'e';
		mp++;
	}
	*mp = '\0';
	PFC_ASSERT(mp < PFC_ARRAY_LIMIT(modebuf));

	return fopen(path, modebuf);
#else	/* !PFC_FOPEN_SUPPORTS_E */
	FILE	*fp;

	/* Open the file. */
	fp = fopen(path, mode);
	if (PFC_EXPECT_TRUE(fp != NULL)) {
		int	err;

		/* Set close-on-exec flag. */
		err = pfc_set_cloexec(fileno(fp), PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)fclose(fp);
			fp = NULL;
			errno = err;
		}
	}

	return fp;
#endif	/* PFC_FOPEN_SUPPORTS_E */
}

/*
 * int
 * pfc_dupfd_cloexec(int fd)
 *	Duplicate the file descriptor specified by `fd'.
 *
 *	The lowest numbered unused descriptor is used for the new descriptor.
 *	Close-on-exec flag is set to the new descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new file descriptor which is a copy of
 *	the given file descriptor is returned.
 *	On failure, -1 is returned. An appropriate error number is set to
 *	errno.
 */
int
pfc_dupfd_cloexec(int fd)
{
#ifdef	PFC_HAVE_F_DUPFD_CLOEXEC
	return fcntl(fd, F_DUPFD_CLOEXEC, 0);
#else	/* !PFC_HAVE_F_DUPFD_CLOEXEC */
	int	newfd;

	newfd = dup(fd);
	if (PFC_EXPECT_TRUE(newfd != -1)) {
		int	err = pfc_set_cloexec(newfd, PFC_TRUE);

		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(newfd);
			errno = err;
			newfd = -1;
		}
	}

	return newfd;
#endif	/* PFC_HAVE_F_DUPFD_CLOEXEC */
}

/*
 * int
 * pfc_dup2fd_cloexec(int fd, int newfd)
 *	Duplicate the file descriptor specified by `fd'.
 *
 *	A file descriptor specified `newfd' is used for the new descriptor.
 *	If `newfd' is opened file descriptor, it is closed at first.
 *	Close-on-exec flag is set to the new descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a new file descriptor which is a copy of
 *	the given file descriptor is returned.
 *	On failure, -1 is returned. An appropriate error number is set to
 *	errno.
 *
 * Remarks:
 *	Unlike dup2(), pfc_dup2fd_cloexec() returns EINVAL error if `newfd'
 *	equals `fd'.
 */
int
pfc_dup2fd_cloexec(int fd, int newfd)
{
#ifdef	PFC_HAVE_DUP3
	return dup3(fd, newfd, O_CLOEXEC);
#else	/* !PFC_HAVE_DUP3 */
	int	ret;

	if (PFC_EXPECT_FALSE(fd == newfd)) {
		errno = EINVAL;

		return -1;
	}

	ret = dup2(fd, newfd);
	if (PFC_EXPECT_TRUE(ret != -1)) {
		int	err;

		PFC_ASSERT(ret == newfd);
		err = pfc_set_cloexec(ret, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(ret);
			errno = err;
			ret = -1;
		}
	}

	return ret;
#endif	/* PFC_HAVE_DUP3 */
}
