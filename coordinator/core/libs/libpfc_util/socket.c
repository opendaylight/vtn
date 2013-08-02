/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * socket.c - Socket utilities.
 */

/*
 * Suppress declaration of inlined pfc_sock_open(), pfc_sock_openpair(), and
 * pfc_sock_accept() in pfc/socket.h in order to define them as function.
 */
#define	__PFC_UTIL_DONT_DEFINE_SOCK_OPEN	1
#define	__PFC_UTIL_DONT_DEFINE_SOCK_OPENPAIR	1
#define	__PFC_UTIL_DONT_DEFINE_SOCK_ACCEPT	1

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <pfc/socket.h>
#include <pfc/util.h>

#ifndef	PFC_HAVE_PPOLL
#error	Current implementation requires ppoll(2).
#endif	/* !PFC_HAVE_PPOLL */

/*
 * int
 * pfc_sock_connect(int sock, const struct sockaddr *PFC_RESTRICT addr,
 *		    socklen_t addrlen,
 *		    const pfc_timespec_t *PFC_RESTRICT timeout,
 *		    const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Establish a connection to the address on the specified socket.
 *
 *	pfc_sock_connect() connects the specified socket using non-blocking
 *	mode. So O_NONBLOCK flag is set during the call of this function.
 *
 *	If the connection can not be completed immediately, this function
 *	blocks the calling thread. `timeout' specifies an upper limit on
 *	the amount of time which the calling thread is blocked. If timeout
 *	exceeds, ETIMEDOUT is returned. NULL means an infinite timeout.
 *
 *	`iowait' determines behavior of I/O wait.
 *	- iw_intrfunc
 *	  Function which determines behavior of EINTR error on I/O wait.
 *	  It will be called if pfc_sock_connect() gets EINTR error.
 *	  If `iw_intrfunc' is NULL or it returns PFC_FALSE, EINTR error is
 *	  simply ignored. If it returns PFC_TRUE, pfc_sock_connect() terminates
 *	  I/O wait and it returns EINTR.
 *	- iw_intrarg
 *	  This value is passed to argument for `iw_intrfunc'.
 *	- iw_sigmask
 *	  If not NULL, the specified signal mask is applied during I/O wait.
 *	  If NULL, signal mask is not affected.
 *
 *	If `iowait' is NULL, it is treated as if NULL is specified to all
 *	members.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ETIMEDOUT is returned if a connection wasn't established within
 *	the specified `timeout' period.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The specified socket is always closed on error return.
 */
int
pfc_sock_connect(int sock, const struct sockaddr *PFC_RESTRICT addr,
		 socklen_t addrlen, const pfc_timespec_t *PFC_RESTRICT timeout,
		 const pfc_iowait_t *PFC_RESTRICT iowait)
{
	return pfc_sock_connect_c(sock, addr, addrlen, -1, timeout, iowait);
}

/*
 * int
 * pfc_sock_connect_c(int sock, const struct sockaddr *PFC_RESTRICT addr,
 *		      socklen_t addrlen, int canceller,
 *		      const pfc_timespec_t *PFC_RESTRICT timeout,
 *		      const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Establish a connection to the address on the specified socket.
 *
 *	This function is the same as pfc_sock_connect(), but it takes
 *	canceller file descriptor. The canceller file descriptor, specified
 *	by `canceller', is readable file descriptor. Any read event on
 *	the canceller file descriptor interrupts I/O wait, and then ECANCELED
 *	is returned. Cancellation is disabled if -1 is specified to
 *	`canceller'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ETIMEDOUT is returned if a connection wasn't established within
 *	the specified `timeout' period.
 *
 *	ECANCELED is returned if an I/O wait is interrupted due to read event
 *	on `canceller'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The specified socket is always closed on error return.
 */
int
pfc_sock_connect_c(int sock, const struct sockaddr *PFC_RESTRICT addr,
		   socklen_t addrlen, int canceller,
		   const pfc_timespec_t *PFC_RESTRICT timeout,
		   const pfc_iowait_t *PFC_RESTRICT iowait)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		int	err;

		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	return pfc_sock_connect_abs_c(sock, addr, addrlen, canceller, abstime,
				      iowait);
}

/*
 * int
 * pfc_sock_connect_abs(int sock, const struct sockaddr *PFC_RESTRICT addr,
 *			socklen_t addrlen,
 *			const pfc_timespec_t *PFC_RESTRICT abstime,
 *			const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Establish a connection to the address on the specified socket.
 *
 *	This function is the same as pfc_sock_connect(), but timeout must be
 *	specified by the absolute system time. If `abstime' is not NULL,
 *	ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ETIMEDOUT is returned if the absolute time specified by `abstime' has
 *	passed.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The specified socket is always closed on error return.
 *
 *	`abstime' must be the absolute time of the PFC system clock.
 *	On Linux, it must be the system monotonic clock.
 */
int
pfc_sock_connect_abs(int sock, const struct sockaddr *PFC_RESTRICT addr,
		     socklen_t addrlen,
		     const pfc_timespec_t *PFC_RESTRICT abstime,
		     const pfc_iowait_t *PFC_RESTRICT iowait)
{
	return pfc_sock_connect_abs_c(sock, addr, addrlen, -1, abstime, iowait);
}

/*
 * int
 * pfc_sock_connect_abs_c(int sock, const struct sockaddr *PFC_RESTRICT addr,
 *			  socklen_t addrlen, int canceller,
 *			  const pfc_timespec_t *PFC_RESTRICT abstime,
 *			  const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Establish a connection to the address on the specified socket.
 *
 *	This function is the same as pfc_sock_connect_abs(), but it takes
 *	canceller file descriptor. The canceller file descriptor, specified
 *	by `canceller', is readable file descriptor. Any read event on
 *	the canceller file descriptor interrupts I/O wait, and then ECANCELED
 *	is returned. Cancellation is disabled if -1 is specified to
 *	`canceller'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	ETIMEDOUT is returned if the absolute time specified by `abstime' has
 *	passed.
 *
 *	ECANCELED is returned if an I/O wait is interrupted due to read event
 *	on `canceller'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The specified socket is always closed on error return.
 *
 *	`abstime' must be the absolute time of the PFC system clock.
 *	On Linux, it must be the system monotonic clock.
 */
int
pfc_sock_connect_abs_c(int sock, const struct sockaddr *PFC_RESTRICT addr,
		       socklen_t addrlen, int canceller,
		       const pfc_timespec_t *PFC_RESTRICT abstime,
		       const pfc_iowait_t *PFC_RESTRICT iowait)
{
	struct pollfd	pfd[2];
	nfds_t		nfds;
	int		fflag, err, soerr;
	sigset_t	*sigmask;
	pfc_iointr_cb_t	intrfunc;
	pfc_ptr_t	intrarg;
	pfc_timespec_t	*tmout, remains;
	socklen_t	optlen;

	/* Preserve current file descriptor flag. */
	fflag = fcntl(sock, F_GETFL);
	if (PFC_EXPECT_FALSE(fflag == -1)) {
		err = errno;
		goto error;
	}

	/* Make this socket as non-blocking if needed. */
	if ((fflag & O_NONBLOCK) == 0) {
		int	ret = fcntl(sock, F_SETFL, fflag | O_NONBLOCK);

		if (PFC_EXPECT_FALSE(ret == -1)) {
			err = errno;
			goto error;
		}
	}

	/* Call connect(2). */
	if (connect(sock, addr, addrlen) == 0) {
		/* Succeeded. */
		goto out;
	}

	err = errno;
	if (PFC_EXPECT_FALSE(err != EINPROGRESS)) {
		goto error;
	}

	/*
	 * The connection can't be completed immediately.
	 * We must wait for the completion.
	 */
	if (iowait == NULL) {
		intrfunc = NULL;
		intrarg = NULL;
		sigmask = NULL;
	}
	else {
		intrfunc = iowait->iw_intrfunc;
		intrarg = iowait->iw_intrarg;
		sigmask = iowait->iw_sigmask;
	}

	if (abstime != NULL) {
		err = pfc_clock_isexpired(&remains, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
		tmout = &remains;
	}
	else {
		tmout = NULL;
	}

	pfd[0].fd = sock;
	pfd[0].events = POLLOUT;
	pfd[0].revents = 0;

	if (canceller != -1) {
		/*
		 * I/O wait must be interrupted when read event is detected
		 * on the cancellation file descriptor.
		 */
		pfd[1].fd = canceller;
		pfd[1].events = POLLIN;
		pfd[1].revents = 0;
		nfds = 2;
	}
	else {
		nfds = 1;
	}

	while (1) {
		int	n;

		n = ppoll(pfd, nfds, tmout, sigmask);
		if (PFC_EXPECT_TRUE(n > 0)) {
			/* Received output. */
			break;
		}
		if (n == 0) {
			/* Timed out. */
			err = ETIMEDOUT;
			goto error;
		}

		/*
		 * Ignore interrupt unless iowait->iw_intrfunc() returns true
		 * on EINTR error.
		 */
		if ((err = errno) != EINTR ||
		    (intrfunc != NULL && (*intrfunc)(intrarg))) {
			goto error;
		}

		if (tmout != NULL) {
			err = pfc_clock_isexpired(tmout, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}
	}

	if (PFC_EXPECT_FALSE(nfds == 2 && pfd[1].revents != 0)) {
		/* I/O wait has been canceled. */
		err = ECANCELED;
		goto error;
	}

	/* Verify the result of connect(). */
	optlen = sizeof(soerr);
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &soerr, &optlen) != 0) {
		err = errno;
		goto error;
	}

	if (soerr != 0) {
		/* connect() has been failed. */
		err = soerr;
		goto error;
	}

out:
	/* Restore file descriptor's flag. */
	if ((fflag & O_NONBLOCK) == 0) {
		int	ret = fcntl(sock, F_SETFL, fflag);

		if (PFC_EXPECT_FALSE(ret == -1)) {
			err = errno;
			goto error;
		}
	}

	return 0;

error:
	(void)close(sock);

	return err;
}

/*
 * int
 * pfc_sock_open(int domain, int type, int protocol, int flags)
 *	Create a new socket and return socket descriptor.
 *	`domain', `type', and `protocol' are arguments to be passed to
 *	socket(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on a new socket.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on a new socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, valid socket descriptor that is not -1 is
 *	returned.
 *	On error, an appropriate error number is set to errno, and -1 is
 *	returned.
 */
int
pfc_sock_open(int domain, int type, int protocol, int flags)
{
	int	sock;

	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

#ifdef	PFC_HAVE_SOCK_CLOEXEC_NONBLOCK
	/* We can use SOCK_CLOEXEC and SOCK_NONBLOCK. */
	if (flags & PFC_SOCK_CLOEXEC) {
		type |= SOCK_CLOEXEC;
	}
	if (flags & PFC_SOCK_NONBLOCK) {
		type |= SOCK_NONBLOCK;
	}

	sock = socket(domain, type, protocol);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		return -1;
	}
#else	/* !PFC_HAVE_SOCK_CLOEXEC_NONBLOCK */
	sock = socket(domain, type, protocol);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		return -1;
	}

	if (flags & PFC_SOCK_CLOEXEC) {
		int	err;

		/* Set close-on-exec flag. */
		err = pfc_set_cloexec(sock, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(sock);
			errno = err;

			return -1;
		}
	}

	if (flags & PFC_SOCK_NONBLOCK) {
		int	err;

		/* Set O_NONBLOCK flag. */
		err = pfc_set_nonblock(sock, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(sock);
			errno = err;

			return -1;
		}
	}
#endif	/* PFC_HAVE_SOCK_CLOEXEC_NONBLOCK */

	PFC_SOCK_FLAGS_ASSERT(sock, flags);

	return sock;
}

/*
 * int
 * pfc_sock_openpair(int domain, int type, int protocol, int flags, int sock[2])
 *	Create an unnamed pair of connected sockets.
 *	`domain', `type', and `protocol', and `sock' are arguments to be passed
 *	to the call of socketpair(2).
 *
 *	Currently, only AF_UNIX can be specified to `domain'.
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on new sockets.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on new sockets.
 *
 * Calling/Exit State:
 *	Upon successful completion, valid socket descriptors are set to buffer
 *	specified by `sock', and zero is returned.
 *	On error, an appropriate error number is set to errno, and -1 is
 *	returned.
 */
int
pfc_sock_openpair(int domain, int type, int protocol, int flags, int sock[2])
{
	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

#ifdef	PFC_HAVE_SOCK_CLOEXEC_NONBLOCK
	/* We can use SOCK_CLOEXEC and SOCK_NONBLOCK. */
	if (flags & PFC_SOCK_CLOEXEC) {
		type |= SOCK_CLOEXEC;
	}
	if (flags & PFC_SOCK_NONBLOCK) {
		type |= SOCK_NONBLOCK;
	}

	if (PFC_EXPECT_FALSE(socketpair(domain, type, protocol, sock) == -1)) {
		return -1;
	}
#else	/* !PFC_HAVE_SOCK_CLOEXEC_NONBLOCK */
	if (PFC_EXPECT_FALSE(socketpair(domain, type, protocol, sock) == -1)) {
		return -1;
	}

	if (flags & PFC_SOCK_CLOEXEC) {
		int	err, i;

		/* Set close-on-exec flag. */
		for (i = 0; i < 2; i++) {
			err = pfc_set_cloexec(sock[i], PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				(void)close(sock[0]);
				(void)close(sock[1]);
				errno = err;

				return -1;
			}
		}
	}

	if (flags & PFC_SOCK_NONBLOCK) {
		int	err, i;

		/* Set O_NONBLOCK flag. */
		for (i = 0; i < 2; i++) {
			err = pfc_set_nonblock(sock[i], PFC_TRUE);
			if (PFC_EXPECT_FALSE(err != 0)) {
				(void)close(sock[0]);
				(void)close(sock[1]);
				errno = err;

				return -1;
			}
		}
	}
#endif	/* PFC_HAVE_SOCK_CLOEXEC_NONBLOCK */

	PFC_SOCK_FLAGS_ASSERT(sock[0], flags);
	PFC_SOCK_FLAGS_ASSERT(sock[1], flags);

	return 0;
}

/*
 * int
 * pfc_sock_accept(int sock, struct sockaddr *PFC_RESTRICT addr,
 *		   socklen_t *PFC_RESTRICT addrlen, int flags)
 *	Accept a connection on a socket.
 *	`sock', `addr', and `addrlen' are arguments to be passed to accept(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on a new socket.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on a new socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, valid socket descriptor that is not -1 is
 *	returned.
 *	On error, an appropriate error number is set to errno, and -1 is
 *	returned.
 */
int
pfc_sock_accept(int sock, struct sockaddr *PFC_RESTRICT addr,
		socklen_t *PFC_RESTRICT addrlen, int flags)
{
	int	fd;

	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

#ifdef	PFC_HAVE_ACCEPT4
	/* We can use accept4(). */
	{
		int	acflags = 0;

		if (flags & PFC_SOCK_CLOEXEC) {
			acflags |= SOCK_CLOEXEC;
		}
		if (flags & PFC_SOCK_NONBLOCK) {
			acflags |= SOCK_NONBLOCK;
		}

		fd = accept4(sock, addr, addrlen, acflags);
		if (PFC_EXPECT_FALSE(fd == -1)) {
			return fd;
		}
	}
#else	/* !PFC_HAVE_ACCEPT4 */
	fd = accept(sock, addr, addrlen);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		return -1;
	}

	if (flags & PFC_SOCK_CLOEXEC) {
		int	err;

		/* Set close-on-exec flag. */
		err = pfc_set_cloexec(fd, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(fd);
			errno = err;

			return -1;
		}
	}

	if (flags & PFC_SOCK_NONBLOCK) {
		int	err;

		/* Set O_NONBLOCK flag. */
		err = pfc_set_nonblock(fd, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(fd);
			errno = err;

			return -1;
		}
	}
#endif	/* PFC_HAVE_ACCEPT4 */

	PFC_SOCK_FLAGS_ASSERT(fd, flags);

	return fd;
}
