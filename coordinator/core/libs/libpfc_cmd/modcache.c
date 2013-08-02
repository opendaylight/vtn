/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * modcache.c - Module cache file management.
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pfc/path.h>
#include "cmdutil.h"
#include "modcache.h"

#ifndef	PFC_HAVE_MMAP
#error	mmap(2) is required.
#endif	/* !PFC_HAVE_MMAP */

#ifndef	PFC_HAVE_ATFILE_SYSCALL
#error	Current implementation requires ATFILE system calls.
#endif	/* !PFC_HAVE_ATFILE_SYSCALL */

/*
 * int
 * pfc_modcache_attach(const char *PFC_RESTRICT dir,
 *		       modch_map_t *PFC_RESTRICT map)
 *	Attach the contents of the module cache file.
 *
 *	`dir' a path to the module cache directory, not module cache file.
 *
 * Calling/Exit State:
 *	Upon successful completion, information about mapping associated with
 *	the module cache file is set to the buffer pointed by `map', and then
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with holding module directory lock.
 */
int
pfc_modcache_attach(const char *PFC_RESTRICT dir,
		    modch_map_t *PFC_RESTRICT map)
{
	struct stat	sbuf, fsbuf;
	modch_chead_t	*headp;
	mode_t		mode;
	int		fd, dfd, err;
	void		*addr;
	size_t		size;

	/* Open the module cache directory. */
	dfd = open(dir, O_RDONLY);
	if (PFC_EXPECT_FALSE(dfd == -1)) {
		err = errno;
		goto error;
	}

	/* Open the module cache file. */
	fd = openat(dfd, PFC_MODCACHE_NAME, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		goto error_dfd;
	}

	/* Get attributes of the module cache file. */
	if (PFC_EXPECT_FALSE(fstatat(dfd, PFC_MODCACHE_NAME, &sbuf,
				     AT_SYMLINK_NOFOLLOW) != 0)) {
		err = errno;
		goto error_fd;
	}

	/*
	 * Ensure that the file descriptor is associated with the module cache
	 * file in order to prevent symbolic link attack.
	 */
	if (PFC_EXPECT_FALSE(fstat(fd, &fsbuf) != 0)) {
		err = errno;
		goto error_fd;
	}

	if (PFC_EXPECT_FALSE(sbuf.st_dev != fsbuf.st_dev ||
			     sbuf.st_ino != fsbuf.st_ino)) {
		err = EINVAL;
		goto error_fd;
	}

	mode = sbuf.st_mode;
	if (PFC_EXPECT_FALSE(!S_ISREG(mode))) {
		/* Unexpected file type. */
		err = EINVAL;
		goto error_fd;
	}

	if (PFC_EXPECT_FALSE(mode & (S_IWGRP | S_IWOTH))) {
		/* Group or world writable. */
		err = EPERM;
		goto error_fd;
	}

	size = sbuf.st_size;
#ifdef	PFC_LP64
	if (size > UINT32_MAX) {
		/* File size is too large. */
		err = EFBIG;
		goto error_fd;
	}
#endif	/* PFC_LP64 */

	/* Map the contents of module cache file into address space. */
	addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (PFC_EXPECT_FALSE(addr == MAP_FAILED)) {
		err = errno;
		goto error_fd;
	}

	/* Ensure the module cache file contains valid data. */
	headp = (modch_chead_t *)addr;
	if (PFC_EXPECT_FALSE(!modch_head_is_valid(headp, size))) {
		munmap(addr, size);
		err = EINVAL;
		goto error_fd;
	}

	map->mm_addr = (const uint8_t *)addr;
	map->mm_size = (uint32_t)size;

	PFC_ASSERT_INT(close(fd), 0);
	PFC_ASSERT_INT(close(dfd), 0);

	return 0;

error_fd:
	PFC_ASSERT_INT(close(fd), 0);

error_dfd:
	PFC_ASSERT_INT(close(dfd), 0);

error:
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * int
 * pfc_modcache_detach(modch_map_t *map)
 *	Detach the contents of module cache file, attached by the call of
 *	pfc_modcache_attach().
 */
int
pfc_modcache_detach(modch_map_t *map)
{
	void	*addr = (void *)map->mm_addr;
	size_t	size = (size_t)map->mm_size;
	int	err;

	if (PFC_EXPECT_TRUE(munmap(addr, size) == 0)) {
		err = 0;
	}
	else {
		err = errno;
		if (PFC_EXPECT_FALSE(err == 0)) {
			/* This should never happen. */
			err = EIO;
		}
	}

	return err;
}
