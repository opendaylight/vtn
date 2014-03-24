/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * proc_linux.c - Utilities to obtain process status via /proc. (Linux specific)
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <pfc/util.h>
#include <pfc/refptr.h>
#include <pfc/clock.h>
#include "util_impl.h"

/*
 * String length enough to store /proc file path.
 */
#define	PROC_PATH_MAX		32U

/* printf(3) format which creates /proc directory path for a process. */
#define	PROC_DIR_FMT		"/proc/%d/"

/*
 * Variable sized buffer.
 * This is used to read whole contents of /proc file.
 */
typedef struct {
	char		*pb_buffer;		/* buffer address */
	size_t		pb_cursor;		/* current cursor */
	size_t		pb_size;		/* current buffer size */

	/*
	 * The first cursor position where the left parenthesis is detected.
	 * This is used to parse stat file.
	 */
	size_t		pb_firstlparen;

	/*
	 * The last cursor position where right parenthesis is detected.
	 * This is used to parse stat file.
	 */
	size_t		pb_lastrparen;
} proc_buf_t;

static const char	maps_fname_heap[] = "[heap]";

#define	MAPS_FNAME_HEAP_LEN	(sizeof(maps_fname_heap) - 1)

/*
 * Determine whether the map is anonymous mapping.
 */
#define	PMREG_ATTR_PRIVATE	(PFC_PMREG_ATTR_WRITE | PFC_PMREG_ATTR_PRIVATE)
#define	PROC_PMREG_IS_ANON(mp)						\
	(((mp)->pmr_attr & PMREG_ATTR_PRIVATE) == PMREG_ATTR_PRIVATE && \
	 (mp)->pmr_inode == 0)

/*
 * Block size of buffer in proc_buf_t.
 */
#define	PROC_BUF_BLKSIZE	128U

/*
 * Parse a token in stat file, and put its value into the specified field.
 * On error, this macro returns to the caller directly.
 */
#define	PROCSTAT_PARSE(ptr, limit, statp, field, type)			\
	do {								\
		uint64_t	__v;					\
		int	__err;						\
									\
		__err = proc_parse_u64(&(ptr), (limit), &__v);	\
		if (PFC_EXPECT_FALSE(__err != 0)) {			\
			return __err;					\
		}							\
		(statp)->field = (type)__v;				\
	} while (0)

/*
 * Parse permission bits in maps, and forward pointer.
 * On error, this macro returns to the caller directly.
 */
#define	PROC_PMREG_ATTR_PARSE(p, limit, bitchar, prot, mp)	\
	do {							\
		if ((p) >= (limit)) {				\
			return EINVAL;				\
		}						\
		if (*(p) == (bitchar)) {			\
			(mp)->pmr_attr |= (prot);		\
		}						\
		(p)++;						\
	} while (0)

/*
 * Internal buffer size for file I/O.
 */
#define	PROC_IO_BUFSIZE		PFC_CONST_U(512)

/*
 * Internal prototypes.
 */
static void	proc_buf_init(proc_buf_t *pbufp);
static void	proc_buf_free(proc_buf_t *pbufp);
static int	proc_buf_append(proc_buf_t *pbufp, char c);
static int	proc_buf_readline(proc_buf_t *pbufp, FILE *fp);

/*
 * int
 * pfc_proc_getcmdline(pid_t pid, pfc_listm_t *listp)
 *	Obtain complete command line which invokes the process specified
 *	by the process ID.
 *
 *	Command lines are parsed as string refptr, and pushed into the
 *	specified list. Reference counter of each argument are configured to
 *	1 so that they are released automatically when the list is destroyed.
 *
 * Calling/Exit State:
 *	Upon successful completion, a list model instance which contains
 *	arguments is set to `*listp', and zero is returned.
 *	ESRCH is returned if the specified process does not exist.
 *	ENOMEM is returned if no memory is available.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller is responsible for destroying the list model created by
 *	this function.
 */
int
pfc_proc_getcmdline(pid_t pid, pfc_listm_t *listp)
{
	proc_buf_t	pbuf;
	pfc_listm_t	list;
	char	path[PROC_PATH_MAX];
	FILE	*fp;
	char	*start, *p, *limit;
	int	err;

	snprintf(path, sizeof(path), PROC_DIR_FMT "cmdline", pid);
	proc_buf_init(&pbuf);

	/* Open cmdline file. */
	fp = PFC_FOPEN_CLOEXEC(path, "r");
	if (PFC_EXPECT_FALSE(fp == NULL)) {
		if ((err = errno) == ENOENT) {
			err = ESRCH;
		}
		else if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}

		return err;
	}

	/* cmdline file consists of only one line. */
	err = proc_buf_readline(&pbuf, fp);
	fclose(fp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err < 0) {
			err = ENODATA;
		}
		goto out;
	}

	/* Create list model instance. */
	err = PFC_VECTOR_CREATE_REF(&list, pfc_refptr_string_ops());
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/*
	 * Parse cmdline file contents.
	 * Each argument is separated by '\0'.
	 */
	start = pbuf.pb_buffer;
	limit = start + pbuf.pb_cursor;
	for (p = start; p < limit; p++) {
		if (*p == '\0') {
			pfc_refptr_t	*rstr;

			/*
			 * Create refptr string which represents this
			 * argument.
			 */
			rstr = pfc_refptr_string_create(start);
			if (PFC_EXPECT_FALSE(rstr == NULL)) {
				pfc_listm_destroy(list);
				err = ENOMEM;
				goto out;
			}

			/* Push this argument to the tail of the list. */
			err = pfc_listm_push_tail(list, (pfc_cptr_t)rstr);
			if (PFC_EXPECT_FALSE(err != 0)) {
				pfc_listm_destroy(list);
				goto out;
			}
			pfc_refptr_put(rstr);
			start = p + 1;
		}
	}

	*listp = list;

out:
	proc_buf_free(&pbuf);

	return err;
}

/*
 * static void
 * proc_buf_init(proc_buf_t *pbufp)
 *	Initialize proc_buf_t.
 */
static void
proc_buf_init(proc_buf_t *pbufp)
{
	pbufp->pb_cursor = 0;
	pbufp->pb_size = 0;
	pbufp->pb_firstlparen = (size_t)-1;
	pbufp->pb_lastrparen = (size_t)-1;
	pbufp->pb_buffer = NULL;
}

/*
 * static void
 * proc_buf_free(proc_buf_t *pbufp)
 *	Release all resources grabbed by the specified proc_buf_t.
 */
static void
proc_buf_free(proc_buf_t *pbufp)
{
	free(pbufp->pb_buffer);
}

/*
 * static int
 * proc_buf_append(proc_buf_t *pbufp, char c)
 *	Append the specified character to the proc_buf_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 */
static int
proc_buf_append(proc_buf_t *pbufp, char c)
{
	size_t	cursor = pbufp->pb_cursor;

	PFC_ASSERT(cursor <= pbufp->pb_size);

	if (cursor == pbufp->pb_size) {
		char	*newbuf;
		size_t	newsize = pbufp->pb_size + PROC_BUF_BLKSIZE;

		/* Buffer must be expanded. */
		newbuf = (char *)realloc(pbufp->pb_buffer, newsize);
		if (PFC_EXPECT_FALSE(newbuf == NULL)) {
			return ENOMEM;
		}

		pbufp->pb_size = newsize;
		pbufp->pb_buffer = newbuf;
	}

	if (c == '(') {
		if (pbufp->pb_firstlparen == (size_t)-1) {
			pbufp->pb_firstlparen = cursor;
		}
	}
	else if (c == ')') {
		pbufp->pb_lastrparen = cursor;
	}

	*(pbufp->pb_buffer + cursor) = c;
	pbufp->pb_cursor = cursor + 1;

	return 0;
}

/*
 * static int
 * proc_buf_readline(proc_buf_t *pbufp, FILE *fp)
 *	Read one line from the file specified by the FILE pointer, and stores
 *	into proc_buf_t. Line feed character ('\n') is considered as the end
 *	of line, and it is also transferred to proc_buf_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the end of file is detected.
 *	Otherwise, positive error number which indicates the cause of error
 *	is returned.
 *
 * Remarks:
 *	The specified proc_buf_t is always reset before processing.
 */
static int
proc_buf_readline(proc_buf_t *pbufp, FILE *fp)
{
	int	c;

	/* Reset buffer. */
	pbufp->pb_cursor = 0;

	while ((c = getc(fp)) != EOF) {
		int	err;

		err = proc_buf_append(pbufp, c);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		if (c == '\n') {
			break;
		}
	}

	if (PFC_EXPECT_FALSE(ferror(fp))) {
		return EIO;
	}

	if (pbufp->pb_cursor == 0 && c == EOF) {
		return -1;
	}

	return 0;
}
