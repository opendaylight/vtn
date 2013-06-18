/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_OSDEP_H
#define	_PFC_OSDEP_H

/*
 * This header file contains Linux-specific configuration.
 *
 * Remarks:
 *	This file is designed to be included from pfc/base.h.
 */

#ifndef	_PFC_BASE_H
#error	Do NOT include osdep.h directly.
#endif	/* !_PFC_BASE_H */

#include <pthread.h>

/*
 * Define OS type.
 */
#define	PFC_OSTYPE_LINUX		1

/*
 * Use GNU libc.
 */
#define	PFC_USE_GLIBC			1

/*
 * We have struct timespec in time.h.
 */
#define	PFC_HAVE_TIMESPEC		1

/*
 * Use monotonic clock.
 */
#define	PFC_USE_CLOCK_MONOTONIC		1

/*
 * We have POSIX thread.
 */
#define	PFC_HAVE_POSIX_THREAD		1

/*
 * POSIX thread ID which means invalid thread.
 */
#define	PFC_PTHREAD_INVALID_ID		((pthread_t)-1)

/*
 * We have vsyslog(3).
 */
#define	PFC_HAVE_VSYSLOG		1

/*
 * We have file advisory lock by fcntl(2).
 */
#define	PFC_HAVE_FCNTL_FLOCK		1

/*
 * We have ppoll(2).
 */
#define	PFC_HAVE_PPOLL			1

/*
 * We have mmap(2).
 */
#define	PFC_HAVE_MMAP			1

/*
 * We don't have ntohll(3).
 */
#undef	PFC_HAVE_NTOHLL

/*
 * We don't have htonll(3).
 */
#undef	PFC_HAVE_HTONLL

/*
 * We don't have closefrom(3).
 */
#undef	PFC_HAVE_CLOSEFROM

/*
 * We have timegm(3).
 */
#define	PFC_HAVE_TIMEGM			1

/*
 * Path to secure random device, which may block the calling thread on read.
 */
#define	PFC_RANDOM_DEVICE_PATH		"/dev/random"

/*
 * Path to unblocked random device.
 */
#define	PFC_URANDOM_DEVICE_PATH		"/dev/urandom"

#endif	/* !_PFC_OSDEP_H */
