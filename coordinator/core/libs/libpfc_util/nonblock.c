/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * nonblock.c - Change nonblock flag for the specified file descriptor.
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pfc/util.h>

/*
 * int
 * pfc_set_nonblock(int fd, pfc_bool_t set)
 *	Change nonblock flag for the specified file descriptor.
 *	If `set' is PFC_TRUE, this function set nonblock flag,
 *	otherwise clears.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_set_nonblock(int fd, pfc_bool_t set)
{
	int	fflags;

	fflags = fcntl(fd, F_GETFL);
	if (PFC_EXPECT_FALSE(fflags == -1)) {
		return errno;
	}

	if (set) {
		if (fflags & O_NONBLOCK) {
			return 0;
		}
		fflags |= O_NONBLOCK;
	} else {
		if (!(fflags & O_NONBLOCK)) {
			return 0;
		}
		fflags &= ~O_NONBLOCK;
	}

	if (PFC_EXPECT_FALSE(fcntl(fd, F_SETFL, (long)fflags) == -1)) {
		return errno;
	}

	return 0;
}
