/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_UTIL_H
#define	_PFC_UTIL_H

/*
 * Miscellaneous utilities.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pfc/base.h>
#include <pfc/clock.h>
#include <pfc/listmodel.h>
#include <pfc/debug.h>

PFC_C_BEGIN_DECL

/*
 * Buffer size enough to store system date in string.
 */
#define	PFC_TIME_STRING_LENGTH		PFC_CONST_U(32)

/*
 * Buffer size enough to store nanosec order system date in string.
 */
#define	PFC_TIME_CLOCK_STRING_LENGTH		\
	(PFC_TIME_STRING_LENGTH + 12)

/*
 * Flags for pfc_pipe_open().
 */
#define	PFC_PIPE_CLOEXEC	0x1
#define	PFC_PIPE_NONBLOCK	0x2
#define	PFC_PIPE_CLOEXEC_NB	(PFC_PIPE_CLOEXEC | PFC_PIPE_NONBLOCK)

/*
 * Ensures that pipe flags, PFC_PIPE_CLOEXEC and PFC_PIPE_NONBLOCK, are
 * applied to the file descriptors.
 */
#ifdef	PFC_VERBOSE_DEBUG

#include <pfc/debug.h>

#define	PFC_PIPE_FLAGS_ASSERT(pipefd, flags)				\
	do {								\
		int	__i, __reqfd, __reqfl;				\
									\
		__reqfd = ((flags) & PFC_PIPE_CLOEXEC)			\
			? FD_CLOEXEC : 0;				\
		__reqfl = ((flags) & PFC_PIPE_NONBLOCK)			\
			? O_NONBLOCK : 0;				\
		for (__i = 0; __i < 2; __i++) {				\
			int	__f;					\
			int	__fd = (pipefd)[__i];			\
									\
			/* Test close-on-exec flag. */			\
			__f = fcntl(__fd, F_GETFD);			\
			PFC_ASSERT(__f != -1);				\
			PFC_ASSERT((__f & FD_CLOEXEC) == __reqfd);	\
									\
			/* Test non-blocking flag. */			\
			__f = fcntl(__fd, F_GETFL);			\
			PFC_ASSERT(__f != -1);				\
			PFC_ASSERT((__f & O_NONBLOCK) == __reqfl);	\
		}							\
	} while (0)

#else	/* !PFC_VERBOSE_DEBUG */
#define	PFC_PIPE_FLAGS_ASSERT(pipefd, flags)	((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * File record lock handle.
 */
typedef int	pfc_flock_t;

/*
 * Process status.
 */
typedef struct {
	pfc_ulong_t	pp_utime;	/* process user time in ticks */
	pfc_ulong_t	pp_stime;	/* process system time in ticks */
	uint64_t	pp_starttime;	/* realtime clock the process started */
	uint64_t	pp_running;	/* process running time in ticks */
	pfc_ulong_t	pp_nthreads;	/* number of kernel threads */
	pfc_ulong_t	pp_vsize;	/* size of virtual memory in bytes */
	pfc_ulong_t	pp_rss;		/* resident set size in pages */
	pfc_ulong_t	pp_heapbase;	/* base address of heap segment */
	pfc_ulong_t	pp_heapsize;	/* size of heap segment */

	/* Amount of private anonymous pages in bytes, excluding heap. */
	pfc_ulong_t	pp_anonsize;
} pfc_procstat_t;

#ifdef	PFC_OSTYPE_LINUX

/*
 * Memory region in the virtual address space.
 * This is Linux specific.
 */
typedef struct {
	pfc_ulong_t	pmr_start;	/* start address */
	pfc_ulong_t	pmr_end;	/* end boundary address */
	uint16_t	pmr_attr;	/* mapping attribute bits */
	pfc_ulong_t	pmr_offset;	/* file offset */
	dev_t		pmr_dev;	/* device ID */
	ino_t		pmr_inode;	/* inode number */
	const char	*pmr_path;	/* file path */
} pfc_procmreg_t;

/*
 * Attribute bits for pmr_attr.
 */
#define	PFC_PMREG_ATTR_READ	PFC_CONST_U(0x0001)	/* readable */
#define	PFC_PMREG_ATTR_WRITE	PFC_CONST_U(0x0002)	/* writable */
#define	PFC_PMREG_ATTR_EXEC	PFC_CONST_U(0x0004)	/* executable */
#define	PFC_PMREG_ATTR_PRIVATE	PFC_CONST_U(0x0008)	/* private */
#define	PFC_PMREG_ATTR_HEAP	PFC_CONST_U(0x0010)	/* heap segment */

/*
 * Iterator function for memory region.
 */
typedef int	(*pfc_mregiter_t)(pfc_procmreg_t *mp, pfc_ptr_t arg);

#endif	/* PFC_OSTYPE_LINUX */

/*
 * Prototypes.
 */
#ifdef	PFC_HAVE_CLOSEFROM
#define	pfc_closefrom(lowfd)		closefrom(lowfd)
#else	/* !PFC_HAVE_CLOSEFROM */
extern void	pfc_closefrom(int lowfd);
#endif	/* PFC_HAVE_CLOSEFROM */

extern int	pfc_is_safepath(char *path);

extern size_t		pfc_get_pagesize(void);
extern uint32_t		pfc_get_online_cpus(void);
extern uint32_t		pfc_get_ngroups_max(void);

extern int	pfc_set_cloexec(int fd, pfc_bool_t set);
extern int	pfc_set_nonblock(int fd, pfc_bool_t set);

extern int	pfc_time_ctime(char *buf, size_t bufsize);
extern int	pfc_time_asctime(const struct tm *PFC_RESTRICT tp,
				 char *PFC_RESTRICT buf, size_t bufsize);
extern int	pfc_time_clock_ctime(char *PFC_RESTRICT buf, size_t bufsize);
extern int	pfc_time_clock_asctime(const pfc_timespec_t *PFC_RESTRICT clock,
				       char *PFC_RESTRICT buf, size_t bufsize);
extern int	pfc_time_mclock_ctime(char *PFC_RESTRICT buf, size_t bufsize);
extern int	pfc_time_mclock_asctime(const pfc_timespec_t *PFC_RESTRICT
					clock, char *PFC_RESTRICT buf,
					size_t bufsize);
extern int	pfc_time_gettime(struct tm *tp);

extern int	pfc_flock_open(pfc_flock_t *PFC_RESTRICT lkp,
			       const char *PFC_RESTRICT path, int mode,
			       mode_t perm);
extern int	pfc_flock_close(pfc_flock_t lk);
extern int	pfc_flock_opendir(pfc_flock_t *PFC_RESTRICT lkp,
				  const char *PFC_RESTRICT path, int mode,
				  mode_t perm);
extern int	pfc_flock_closedir(pfc_flock_t lk);
extern int	pfc_flock_wrlock(pfc_flock_t lk, pid_t *ownerp);
extern int	pfc_flock_rdlock(pfc_flock_t lk, pid_t *ownerp);
extern int	pfc_flock_unlock(pfc_flock_t lk);
extern int	pfc_flock_getowner(pfc_flock_t lk, pid_t *ownerp,
				   pfc_bool_t writer);

extern int	pfc_proc_getcmdline(pid_t pid, pfc_listm_t *listp);

extern int	pfc_mkdir(const char *path, mode_t perm);
extern int	pfc_rmpath(const char *path);
extern int	pfc_rmpathat(int dirfd, const char *path);
extern int	pfc_cleandir(const char *path);
extern int	pfc_cleandirat(int dirfd, const char *path);

extern int	pfc_pwd_strtouid(const char *name, uid_t *uidp);
extern int	pfc_pwd_strtogid(const char *name, gid_t *gidp);
extern int	pfc_pwd_uidtoname(uid_t uid, char *PFC_RESTRICT name,
				  size_t *PFC_RESTRICT namelenp);
extern int	pfc_pwd_gidtoname(gid_t gid, char *PFC_RESTRICT name,
				  size_t *PFC_RESTRICT namelenp);
extern int	pfc_pwd_ismember(uid_t uid, gid_t gid, pfc_bool_t supp);

extern int	pfc_cred_setup(void);
extern int	pfc_cred_set(uid_t uid, gid_t gid, pfc_bool_t initgrp);
extern int	pfc_cred_setbyname(const char *user, const char *group,
				   pfc_bool_t initgrp);

extern int	pfc_frotate_openat(int dfd, const char *name, mode_t mode,
				   size_t maxsize, uint32_t rotate);

extern size_t	pfc_strlcpy(char *PFC_RESTRICT dst,
			    const char *PFC_RESTRICT src, size_t dstsize);

#ifdef	PFC_VERBOSE_DEBUG

/*
 * static inline size_t PFC_FATTR_ALWAYS_INLINE
 * pfc_strlcpy_assert(char *PFC_RESTRICT dst, const char *PFC_RESTRICT src,
 *		      size_t dstsize)
 *	Call pfc_strlcpy() with assertion.
 *
 * Calling/Exit State:
 *	This function returns the result of strlen(src).
 */
static inline size_t PFC_FATTR_ALWAYS_INLINE
pfc_strlcpy_assert(char *PFC_RESTRICT dst, const char *PFC_RESTRICT src,
		   size_t dstsize)
{
	size_t	ret = pfc_strlcpy(dst, src, dstsize);

	PFC_ASSERT_PRINTF(ret < dstsize,
			  "pfc_strlcpy() returned %" PFC_PFMT_SIZE_T
			  ", but dstsize is %" PFC_PFMT_SIZE_T ".",
			  ret, dstsize);

	return ret;
}

#else	/* !PFC_VERBOSE_DEBUG */

#define	pfc_strlcpy_assert(dst, src, dstsize)	pfc_strlcpy(dst, src, dstsize)

#endif	/* PFC_VERBOSE_DEBUG */

extern FILE	*pfc_fopen_cloexec(const char *PFC_RESTRICT path,
				   const char *PFC_RESTRICT mode);

#if	defined(PFC_HAVE_O_CLOEXEC) && defined(O_CLOEXEC) &&		\
	defined(__GNUC__) && !defined(__PFC_UTIL_DONT_DEFINE_OPEN_CLOEXEC)
#define	pfc_open_cloexec(path, flags, ...)				\
	open((path), (flags) | O_CLOEXEC, ##__VA_ARGS__)
#ifdef	PFC_HAVE_ATFILE_SYSCALL
#define	__PFC_OPENAT_CLOEXEC_INLINE
#endif	/* PFC_HAVE_ATFILE_SYSCALL */
#else	/* !(PFC_HAVE_O_CLOEXEC && O_CLOEXEC && __GNUC__ &&
	     !__PFC_UTIL_DONT_DEFINE_OPEN_CLOEXEC) */
extern int	pfc_open_cloexec(const char *path, int flags,
				 ... /* mode_t mode */);
#endif	/* PFC_HAVE_O_CLOEXEC && O_CLOEXEC && __GNUC__ &&
	   !__PFC_UTIL_DONT_DEFINE_OPEN_CLOEXEC */

#ifdef	__PFC_OPENAT_CLOEXEC_INLINE
#define	pfc_openat_cloexec(dirfd, path, flags, ...)			\
	openat((dirfd), (path), (flags) | O_CLOEXEC, ##__VA_ARGS__)
#else	/* !__PFC_OPENAT_CLOEXEC_INLINE */
extern int	pfc_openat_cloexec(int dirfd, const char *path, int flags,
				   ... /* mode_t mode */);
#endif	/* __PFC_OPENAT_CLOEXEC_INLINE */

#ifdef	AT_FDCWD
#define	PFC_AT_FDCWD		AT_FDCWD
#else	/* !AT_FDCWD */
#define	PFC_AT_FDCWD		(-100)
#endif	/* AT_FDCWD */

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
#if	defined(PFC_HAVE_F_DUPFD_CLOEXEC) &&	\
	!defined(__PFC_UTIL_DONT_DEFINE_DUPFD)
#define	pfc_dupfd_cloexec(fd)		fcntl(fd, F_DUPFD_CLOEXEC, 0)
#else	/* !(PFC_HAVE_F_DUPFD_CLOEXEC && !__PFC_UTIL_DONT_DEFINE_DUPFD) */
extern int	pfc_dupfd_cloexec(int fd);
#endif	/* PFC_HAVE_F_DUPFD_CLOEXEC && !__PFC_UTIL_DONT_DEFINE_DUPFD */

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
#if	defined(PFC_HAVE_DUP3) && !defined(__PFC_UTIL_DONT_DEFINE_DUP2FD)
#define	pfc_dup2fd_cloexec(fd, newfd)	dup3(fd, newfd, O_CLOEXEC)
#else	/* !PFC_HAVE_DUP3 && !__PFC_UTIL_DONT_DEFINE_DUP2FD) */
extern int	pfc_dup2fd_cloexec(int fd, int newfd);
#endif	/* PFC_HAVE_DUP3 && !__PFC_UTIL_DONT_DEFINE_DUP2FD */

#if	defined(PFC_HAVE_PIPE2) && !defined(__PFC_UTIL_DONT_DEFINE_PIPE_OPEN)
/*
 * pfc_pipe_open() is a simple wrapper for pipe2(2).
 *
 * `flags' takes any combination of the following bits:
 *
 *	PFC_PIPE_CLOEXEC	Set close-on-exec flag on new descriptors.
 *	PFC_PIPE_NONBLOCK	Set O_NONBLOCK flag on new descriptors.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_pipe_open(int pipefd[2], int flags)
{
	int	ret, pflags;

	if (PFC_EXPECT_FALSE(flags & ~PFC_PIPE_CLOEXEC_NB)) {
		/* Invalid flag is specified. */
		errno = EINVAL;

		return -1;
	}

	pflags = 0;
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
#else	/* !(PFC_HAVE_PIPE2 && !__PFC_UTIL_DONT_DEFINE_PIPE_OPEN) */
extern int	pfc_pipe_open(int pipefd[2], int flags);
#endif	/* PFC_HAVE_PIPE2 && !__PFC_UTIL_DONT_DEFINE_PIPE_OPEN */

#ifdef	__GNUC__
#define	__PFC_FOPEN_MODE_ASSERT(mode)				\
	PFC_ASSERT((mode) != NULL &&					\
		   (__builtin_constant_p(mode)				\
		    ? (sizeof(mode) - 1 < 6) : strlen(mode) < 6))
#else	/* !__GNUC__ */
#define	__PFC_FOPEN_MODE_ASSERT(mode)			\
	PFC_ASSERT((mode) != NULL && strlen(mode) < 6)
#endif	/* __GNUC__ */

/*
 * FILE *
 * PFC_FOPEN_CLOEXEC(const char *PFC_RESTRICT path,
 *	 	     const char *PFC_RESTRICT mode)
 *	Open the specified file and associate a stream with it.
 *	Close-on-exec flag is always set to file descriptor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to stream associated
 *	with the file is returned.
 *	On error, an appropriate error number is set to `errno' and NULL is
 *	returned.
 *
 * Remarks:
 *	`mode' must be a pointer to constant string literal whose length is
 *	less than 6 characters, otherwise PFC_FOPEN_CLOEXEC() will cause
 *	undefined behavior.
 */
#if	defined(PFC_FOPEN_SUPPORTS_E)
#define	PFC_FOPEN_CLOEXEC(path, mode)				\
	(__PFC_FOPEN_MODE_ASSERT(mode), fopen((path), mode "e"))
#else	/* !(PFC_FOPEN_SUPPORTS_E && __GNUC__) */
#define	PFC_FOPEN_CLOEXEC(path, mode)	pfc_fopen_cloexec((path), (mode))
#endif	/* PFC_FOPEN_SUPPORTS_E && __GNUC__ */

PFC_C_END_DECL

#endif	/* !_PFC_UTIL_H */
