/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_SOCKET_H
#define	_PFC_SOCKET_H

/*
 * Socket utilities.
 */

#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pfc/base.h>
#include <pfc/iostream.h>

PFC_C_BEGIN_DECL

/*
 * Return actual size of struct sockaddr_un.
 * `plen' must be the length of socket file path.
 */
#define	PFC_SOCK_UNADDR_SIZE(plen)				\
	(offsetof(struct sockaddr_un, sun_path) + (plen))

/*
 * Flags for pfc_sock_open() and pfc_sock_accept().
 */
#define	PFC_SOCK_CLOEXEC	0x1
#define	PFC_SOCK_NONBLOCK	0x2
#define	PFC_SOCK_CLOEXEC_NB	(PFC_SOCK_CLOEXEC | PFC_SOCK_NONBLOCK)

#ifndef	PFC_HAVE_SOCK_CLOEXEC_NONBLOCK

/* Disable use of accept4(2). */
#undef	PFC_HAVE_ACCEPT4

#endif	/* !PFC_HAVE_SOCK_CLOEXEC_NONBLOCK */

/*
 * Ensures that socket flags, PFC_SOCK_CLOEXEC and PFC_SOCK_NONBLOCK, are
 * applied to the socket descriptor.
 */
#ifdef	PFC_VERBOSE_DEBUG

#include <fcntl.h>
#include <pfc/debug.h>

#define	PFC_SOCK_FLAGS_ASSERT(sock, flags)			\
	do {							\
		int	__f, __required;			\
								\
		/* Test close-on-exec flag. */			\
		__f = fcntl(sock, F_GETFD);			\
		PFC_ASSERT(__f != -1);				\
		__required = ((flags) & PFC_SOCK_CLOEXEC)	\
			? FD_CLOEXEC : 0;			\
		PFC_ASSERT((__f & FD_CLOEXEC) == __required);	\
								\
		/* Test non-blocking flag. */			\
		__f = fcntl(sock, F_GETFL);			\
		PFC_ASSERT(__f != -1);				\
		__required = ((flags) & PFC_SOCK_NONBLOCK)	\
			? O_NONBLOCK : 0;			\
		PFC_ASSERT((__f & O_NONBLOCK) == __required);	\
	} while (0)

#else	/* !PFC_VERBOSE_DEBUG */
#define	PFC_SOCK_FLAGS_ASSERT(sock, flags)	((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * User credentials.
 */
typedef struct ucred		pfc_ucred_t;
typedef const pfc_ucred_t	pfc_cucred_t;

/*
 * Prototypes.
 */
extern int	pfc_sock_connect(int sock,
				 const struct sockaddr *PFC_RESTRICT addr,
				 socklen_t addrlen,
				 const pfc_timespec_t *PFC_RESTRICT timeout,
				 const pfc_iowait_t *PFC_RESTRICT iowait);
extern int	pfc_sock_connect_c(int sock,
				   const struct sockaddr *PFC_RESTRICT addr,
				   socklen_t addrlen, int canceller,
				   const pfc_timespec_t *PFC_RESTRICT timeout,
				   const pfc_iowait_t *PFC_RESTRICT iowait);
extern int	pfc_sock_connect_abs(int sock,
				     const struct sockaddr *PFC_RESTRICT addr,
				     socklen_t addrlen,
				     const pfc_timespec_t *PFC_RESTRICT abstime,
				     const pfc_iowait_t *PFC_RESTRICT iowait);
extern int	pfc_sock_connect_abs_c(int sock,
				       const struct sockaddr *PFC_RESTRICT addr,
				       socklen_t addrlen, int canceller,
				       const pfc_timespec_t *PFC_RESTRICT
				       abstime,
				       const pfc_iowait_t *PFC_RESTRICT
				       iowait);

#if	defined(PFC_HAVE_SOCK_CLOEXEC_NONBLOCK) &&	\
	!defined(__PFC_UTIL_DONT_DEFINE_SOCK_OPEN)

/*
 * pfc_sock_open() is a simple wrapper for socket(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on a new socket.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on a new socket.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_sock_open(int domain, int type, int protocol, int flags)
{
	int	sock;

	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

	if (flags & PFC_SOCK_CLOEXEC) {
		type |= SOCK_CLOEXEC;
	}
	if (flags & PFC_SOCK_NONBLOCK) {
		type |= SOCK_NONBLOCK;
	}

	sock = socket(domain, type, protocol);
	if (PFC_EXPECT_TRUE(sock != -1)) {
		PFC_SOCK_FLAGS_ASSERT(sock, flags);
	}

	return sock;
}

#else	/* !(PFC_HAVE_SOCK_CLOEXEC_NONBLOCK &&
	     !__PFC_UTIL_DONT_DEFINE_SOCK_OPEN) */
extern int	pfc_sock_open(int domain, int type, int protocol, int flags);
#endif	/* PFC_HAVE_SOCK_CLOEXEC_NONBLOCK &&
	   !__PFC_UTIL_DONT_DEFINE_SOCK_OPEN */

#if	defined(PFC_HAVE_SOCK_CLOEXEC_NONBLOCK) &&	\
	!defined(__PFC_UTIL_DONT_DEFINE_SOCK_OPENPAIR)

/*
 * pfc_sock_openpair is a simple wrapper for socketpair(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on new sockets.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on new sockets.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_sock_openpair(int domain, int type, int protocol, int flags, int sock[2])
{
	int	ret;

	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

	if (flags & PFC_SOCK_CLOEXEC) {
		type |= SOCK_CLOEXEC;
	}
	if (flags & PFC_SOCK_NONBLOCK) {
		type |= SOCK_NONBLOCK;
	}

	ret = socketpair(domain, type, protocol, sock);
	if (PFC_EXPECT_TRUE(ret != -1)) {
		PFC_SOCK_FLAGS_ASSERT(sock[0], flags);
		PFC_SOCK_FLAGS_ASSERT(sock[1], flags);
	}

	return ret;
}

#else	/* defined(PFC_HAVE_SOCK_CLOEXEC_NONBLOCK) &&
	   !defined(__PFC_UTIL_DONT_DEFINE_SOCK_OPENPAIR) */
extern	int	pfc_sock_openpair(int domain, int type, int protocol,
				  int flags, int sock[2]);
#endif	/* defined(PFC_HAVE_SOCK_CLOEXEC_NONBLOCK) &&
	   !defined(__PFC_UTIL_DONT_DEFINE_SOCK_OPENPAIR) */

#if	defined(PFC_HAVE_ACCEPT4) &&			\
	!defined(__PFC_UTIL_DONT_DEFINE_SOCK_ACCEPT)

/*
 * pfc_sock_accept() is a simple wrapper for accept4(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_SOCK_CLOEXEC	Set close-on-exec flag on a new socket.
 *	PFC_SOCK_NONBLOCK	Set O_NONBLOCK flag on a new socket.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_sock_accept(int sock, struct sockaddr *PFC_RESTRICT addr,
		socklen_t *PFC_RESTRICT addrlen, int flags)
{
	int	fd, acflags;

	if (PFC_EXPECT_FALSE(flags & ~PFC_SOCK_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

	acflags = 0;
	if (flags & PFC_SOCK_CLOEXEC) {
		acflags |= SOCK_CLOEXEC;
	}
	if (flags & PFC_SOCK_NONBLOCK) {
		acflags |= SOCK_NONBLOCK;
	}

	fd = accept4(sock, addr, addrlen, acflags);
	if (PFC_EXPECT_TRUE(fd != -1)) {
		PFC_SOCK_FLAGS_ASSERT(fd, flags);
	}

	return fd;
}

#else	/* !(PFC_HAVE_ACCEPT4 && !__PFC_UTIL_DONT_DEFINE_SOCK_ACCEPT) */
extern int	pfc_sock_accept(int sock, struct sockaddr *PFC_RESTRICT addr,
				socklen_t *PFC_RESTRICT addrlen, int flags);
#endif	/* PFC_HAVE_ACCEPT4 && !__PFC_UTIL_DONT_DEFINE_SOCK_ACCEPT */

PFC_C_END_DECL

#endif	/* !_PFC_SOCKET_H */
