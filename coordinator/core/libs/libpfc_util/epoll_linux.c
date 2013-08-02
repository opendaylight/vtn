/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Low level utilities for I/O event notification facility. (Linux specific)
 */

/*
 * Suppress declaration of pfc_epoll_create().
 */
#define	__PFC_UTIL_DONT_DEFINE_EPOLL_CREATE	1

#include <pfc/epoll.h>
#include <pfc/util.h>

/*
 * int
 * pfc_epoll_create(void)
 *	Create an event poll instance.
 *
 *	The close-on-exec bit is always set to the file descriptor associated
 *	with the new event poll instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, a valid file descriptor associated with
 *	an event poll instance is returned.
 *	Otherwise, error number which indicates the cause of error is set
 *	to errno, and then -1 is returned.
 */
int
pfc_epoll_create(void)
{
	int	epfd;

#ifdef	PFC_HAVE_EPOLL_CLOEXEC
	epfd = epoll_create1(EPOLL_CLOEXEC);
#else	/* !PFC_HAVE_EPOLL_CLOEXEC */
	epfd = epoll_create(1);
	if (PFC_EXPECT_TRUE(epfd != -1)) {
		int	err = pfc_set_cloexec(epfd, PFC_TRUE);

		if (PFC_EXPECT_FALSE(err != 0)) {
			(void)close(epfd);
			errno = err;
			epfd = -1;
		}
	}
#endif	/* PFC_HAVE_EPOLL_CLOEXEC */

	return epfd;
}
