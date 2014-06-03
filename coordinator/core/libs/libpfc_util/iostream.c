/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * iostream.c - Buffered I/O stream using non-blocking file descriptor.
 */

#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pfc/synch.h>
#include <pfc/util.h>
#include <pfc/log.h>
#include <pfc/debug.h>
#include <pfc/atomic.h>
#include "iostream_impl.h"

#ifndef	PFC_HAVE_PPOLL
#error	Current implementation requires ppoll(2).
#endif	/* !PFC_HAVE_PPOLL */

/*
 * Internal I/O buffer.
 */
typedef struct {
	uint8_t		*io_buffer;		/* pointer to buffer */
	uint32_t	io_size;		/* buffer size */
	uint32_t	io_top;			/* offset of valid data */
	uint32_t	io_bottom;		/* offset of end of data */
	pfc_bool_t	io_enable;		/* enable flag */
} iobuf_t;

/*
 * True if I/O buffer is empty.
 */
#define	IOBUF_IS_EMPTY(bp)	((bp)->io_top == (bp)->io_bottom)

/*
 * Buffered input/output stream.
 */
struct __pfc_iostream {
	iobuf_t			s_input;	/* input buffer */
	iobuf_t			s_output;	/* output buffer */
	pfc_mutex_t		s_mutex;	/* mutex for stream */
	pfc_cond_t		s_cond;		/* condition variable */
	pfc_iointr_cb_t		s_intrfunc;	/* EINTR callback */
	pfc_ptr_t		s_intrarg;	/* argument for s_intrfunc */
	const sigset_t		*s_sigmask;	/* signal mask */
	int			s_fd;		/* file descriptor */
	int			s_cancel;	/* cancel event FD */
	uint32_t		s_flags;	/* flags */
	volatile pfc_bool_t	s_busy;		/* busy flag */
};

#define	IOSTREAM_LOCK(stream)		pfc_mutex_lock(&(stream)->s_mutex)
#define	IOSTREAM_UNLOCK(stream)		pfc_mutex_unlock(&(stream)->s_mutex)
#define	IOSTREAM_TIMEDLOCK_ABS(stream, abstime)			\
	pfc_mutex_timedlock_abs(&(stream)->s_mutex, (abstime))

#define	IOSTREAM_TIMEDWAIT_ABS(stream, abstime)				\
	pfc_cond_timedwait_abs(&(stream)->s_cond, &(stream)->s_mutex,	\
			       (abstime))
#define	IOSTREAM_SIGNAL(stream)		pfc_cond_signal(&(stream)->s_cond)
#define	IOSTREAM_BROADCAST(stream)	pfc_cond_broadcast(&(stream)->s_cond)

/*
 * Flags for s_flags.
 */
#define	IOSTRF_EOF	PFC_CONST_U(0x1)	/* EOF has been detected */
#define	IOSTRF_HUP	PFC_CONST_U(0x2)	/* hang up */
#define	IOSTRF_SHUT_RD	PFC_CONST_U(0x4)	/* read shut down */
#define	IOSTRF_SHUT_WR	PFC_CONST_U(0x8)	/* write shut down */
#define	IOSTRF_DESROYED	PFC_CONST_U(0x10)	/* destroyed */
#define	IOSTRF_SOCKET	PFC_CONST_U(0x20)	/* socket based stream */

#define	IOSTREAM_IS_RD_SHUTDOWN(stream)				\
	((stream)->s_flags & (IOSTRF_EOF | IOSTRF_SHUT_RD))
#define	IOSTREAM_IS_WR_SHUTDOWN(stream)				\
	((stream)->s_flags & (IOSTRF_HUP | IOSTRF_SHUT_WR))
#define	IOSTREAM_IS_SHUTDOWN(stream)				\
	((stream)->s_flags & (IOSTRF_EOF | IOSTRF_SHUT_RD |	\
			      IOSTRF_HUP | IOSTRF_SHUT_WR))
#define	IOSTREAM_IS_VALID(stream)			\
	(((stream)->s_flags & IOSTRF_DESROYED) == 0)
#define	IOSTREAM_IS_SOCKET(stream)	((stream)->s_flags & IOSTRF_SOCKET)

/*
 * Convert PFC_IOSRCV_* flags to recvmsg(2) flags.
 */
#ifdef	PFC_HAVE_CMSG_CLOEXEC
#define	IOSTREAM_RCVFLAG(flags)					\
	(((flags) & PFC_IOSRCV_CLOEXEC) ? MSG_CMSG_CLOEXEC : 0)
#else	/* !PFC_HAVE_CMSG_CLOEXEC */
#define	IOSTREAM_RCVFLAG(flags)			(0)
#endif	/* PFC_HAVE_CMSG_CLOEXEC */

/*
 * Convert timeout period to absolute system time.
 * This macro may return from the calling process on error.
 */
#define	IOSTREAM_ABSTIME(timeout, abstime, tsbuf)			\
	do {								\
		if ((timeout) != NULL) {				\
			int	__err;					\
									\
			(abstime) = (tsbuf);				\
			__err = pfc_clock_abstime((abstime), (timeout)); \
			if (PFC_EXPECT_FALSE(__err != 0)) {		\
				return __err;				\
			}						\
		}							\
		else {							\
			(abstime) = NULL;				\
		}							\
	} while (0)

/*
 * Same as IOSTREAM_ABSTIME(), but this macro zeros `*sizep' on error.
 */
#define	IOSTREAM_ABSTIME_SIZEP(timeout, abstime, tsbuf, sizep)		\
	do {								\
		if ((timeout) != NULL) {				\
			int	__err;					\
									\
			(abstime) = (tsbuf);				\
			__err = pfc_clock_abstime((abstime), (timeout)); \
			if (PFC_EXPECT_FALSE(__err != 0)) {		\
				if ((sizep) != NULL) {			\
					*(sizep) = 0;			\
				}					\
									\
				return __err;				\
			}						\
		}							\
		else {							\
			(abstime) = NULL;				\
		}							\
	} while (0)

/*
 * Operations to send or receive control message via UNIX domain socket.
 */

struct cmsg_ctx;
typedef struct cmsg_ctx		cmsg_ctx_t;

typedef struct {
	/*
	 * size_t
	 * cmops_size(cmsg_ctx_t *ctx)
	 *	Return the size of control message.
	 *	`data' is an arbitrary data used to send or receive control
	 *	message.
	 */
	size_t	(*cmops_size)(cmsg_ctx_t *ctx);

	/*
	 * void
	 * cmops_setup(cmsg_ctx_t *ctx)
	 *	Set up control message to be sent.
	 */
	void	(*cmops_setup)(cmsg_ctx_t *ctx);

	/*
	 * void
	 * cmops_fetch(cmsg_ctx_t *ctx)
	 *	Fetch control message read via UNIX domain socket.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	void	(*cmops_fetch)(cmsg_ctx_t *ctx);

	/*
	 * int
	 * cmops_fetch_prepare(cmsg_ctx_t *ctx)
	 *	Prepare to fetch control message.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*cmops_fetch_prepare)(cmsg_ctx_t *ctx);

	/*
	 * int
	 * cmops_fetch_cleanup(cmsg_ctx_t *ctx)
	 *	Clean up changes made by cmops_fetch_prepare().
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*cmops_fetch_cleanup)(cmsg_ctx_t *ctx);
} cmsg_ops_t;

typedef const cmsg_ops_t	cmsg_cops_t;

/*
 * Context to send or receive control message.
 */
struct cmsg_ctx {
	pfc_iostream_t		cm_stream;	/* socket stream */
	void			*cm_buffer;	/* pointer to data */
	size_t			*cm_sizep;	/* pointer to data size */
	const pfc_timespec_t	*cm_abstime;	/* timeout */
	cmsg_cops_t		*cm_ops;	/* operations */
	int			cm_mflags;	/* message flags */
	struct msghdr		cm_header;	/* message header */
};

#define	CMSG_CTX_INIT(ctx, stream, buf, sizep, abstime, ops, mflags)	\
	do {								\
		(ctx)->cm_stream = (stream);				\
		(ctx)->cm_buffer = (void *)(buf);			\
		(ctx)->cm_sizep = (sizep);				\
		(ctx)->cm_abstime = (abstime);				\
		(ctx)->cm_ops = (ops);					\
		(ctx)->cm_mflags = (mflags);				\
	} while (0)

/*
 * Context to send or receive credentials of process.
 */
typedef struct {
	cmsg_ctx_t	cmc_context;		/* control message context */
	pfc_ucred_t	*cmc_cred;		/* pointer to ucred */
	int		cmc_passcred;		/* preserved SO_PASSCRED */
} cmsg_credctx_t;

#define	CMSG_CREDCTX_INIT(crctx, stream, buf, sizep, credp, abstime)	\
	do {								\
		CMSG_CTX_INIT(&(crctx)->cmc_context, stream, buf,	\
			      sizep, abstime, &iostream_cmcred_ops, 0);	\
		(crctx)->cmc_cred = (pfc_ucred_t *)(credp);		\
	} while (0)

#define	CMSG_CTX_CREDCTX(ctx)					\
	PFC_CAST_CONTAINER((ctx), cmsg_credctx_t, cmc_context)

/*
 * Determine whether the internal logging is enabled or not.
 */
static pfc_bool_t	iostream_log_enabled = PFC_FALSE;

#ifdef	__PFC_LOG_GNUC

#define	IOSTREAM_LOG_ERROR(format, ...)				\
	if (iostream_log_enabled) {				\
		pfc_log_error((format), ##__VA_ARGS__);		\
	}

#else	/* !__PFC_LOG_GNUC */

static inline void PFC_FATTR_PRINTFLIKE(1, 2)
IOSTREAM_LOG_ERROR(const char *format, ...)
{
	if (iostream_log_enabled) {
		va_list	ap;

		va_start(ap, format);
		pfc_log_error_v(format, ap);
		va_end(ap);
	}
}

#endif	/* __PFC_LOG_GNUC */

/*
 * Internal prototypes.
 */
static int	iostream_buffer_init(iobuf_t *bp, uint32_t size);
static int	iostream_buffer_resize(iobuf_t *bp, uint32_t size);
static int	iostream_direct_read(pfc_iostream_t PFC_RESTRICT stream,
				     void *PFC_RESTRICT buf,
				     size_t *PFC_RESTRICT sizep,
				     const pfc_timespec_t *PFC_RESTRICT
				     abstime) PFC_FATTR_NOINLINE;
static int	iostream_direct_write(pfc_iostream_t PFC_RESTRICT stream,
				      const void *PFC_RESTRICT buf,
				      size_t *PFC_RESTRICT sizep,
				      const pfc_timespec_t *PFC_RESTRICT
				      abstime) PFC_FATTR_NOINLINE;
static ssize_t	iostream_read(pfc_iostream_t PFC_RESTRICT stream,
			      uint8_t *PFC_RESTRICT buf, size_t size,
			      const pfc_timespec_t *PFC_RESTRICT abstime);
static ssize_t	iostream_write(pfc_iostream_t PFC_RESTRICT stream,
			       const uint8_t *PFC_RESTRICT buf, size_t size,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static int	iostream_sendmsg(cmsg_ctx_t *ctx);
static ssize_t	iostream_do_sendmsg(pfc_iostream_t PFC_RESTRICT stream,
				    const struct msghdr *PFC_RESTRICT msg,
				    int mflags,
				    const pfc_timespec_t *PFC_RESTRICT abstime);
static int	iostream_recvmsg(cmsg_ctx_t *ctx);
static ssize_t	iostream_do_recvmsg(pfc_iostream_t PFC_RESTRICT stream,
				    struct msghdr *PFC_RESTRICT msg, int mflags,
				    const pfc_timespec_t *PFC_RESTRICT abstime);
static int	iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static int	iostream_lock(pfc_iostream_t PFC_RESTRICT stream,
			      const pfc_timespec_t *PFC_RESTRICT abstime);
static void	iostream_unlock(pfc_iostream_t stream);

static size_t	iostream_cmcred_size(cmsg_ctx_t *ctx);
static void	iostream_cmcred_setup(cmsg_ctx_t *ctx);
static void	iostream_cmcred_fetch(cmsg_ctx_t *ctx);
static int	iostream_cmcred_fetch_prepare(cmsg_ctx_t *ctx);
static int	iostream_cmcred_fetch_cleanup(cmsg_ctx_t *ctx);
static int	iostream_getpollerr(pfc_iostream_t stream, int fd,
				    short events, short revents);
static int	iostream_getsockerr(int sock);

/*
 * Operations to send or receive process credentials via control message.
 */
static cmsg_cops_t		iostream_cmcred_ops = {
	.cmops_size		= iostream_cmcred_size,
	.cmops_setup		= iostream_cmcred_setup,
	.cmops_fetch		= iostream_cmcred_fetch,
	.cmops_fetch_prepare	= iostream_cmcred_fetch_prepare,
	.cmops_fetch_cleanup	= iostream_cmcred_fetch_cleanup,
};

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * iostream_close_fd(int fd, const char *label)
 *	Close the given file descriptor.
 *
 *	`label' is an arbitrary string to embed to an error log message.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
iostream_close_fd(int fd, const char *label)
{
	if (PFC_EXPECT_FALSE(close(fd) != 0)) {
		IOSTREAM_LOG_ERROR("%s: Failed to close FD(%d): %s",
				   label, fd, strerror(errno));
	}
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * iostream_close(pfc_iostream_t stream, const char *label)
 *	Close the file descriptor associated with the given stream.
 *
 *	`label' is an arbitrary string to embed to an error log message.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
iostream_close(pfc_iostream_t stream, const char *label)
{
	int	fd = stream->s_fd;

	if (fd != -1 && PFC_EXPECT_FALSE(close(fd) != 0)) {
		int	err = errno;

		IOSTREAM_LOG_ERROR("%s: Failed to close FD(%d): "
				   "stream=%p: %s", label, fd, stream,
				   strerror(err));
		if (PFC_EXPECT_FALSE(err == 0)) {
			err = EIO;
		}

		return err;
	}

	return 0;
}

/*
 * static inline void
 * iostream_buffer_free(iobuf_t *bp)
 *	Release I/O buffer.
 */
static inline void
iostream_buffer_free(iobuf_t *bp)
{
	free((void *)bp->io_buffer);
}

/*
 * static inline pfc_bool_t
 * iostream_intr_callback(pfc_iostream_t stream)
 *	Call EINTR callback.
 *
 * Calling/Exit State:
 *	The value returned by the callback is returned.
 *	PFC_FALSE is returned if no callback is registered.
 */
static inline pfc_bool_t
iostream_intr_callback(pfc_iostream_t stream)
{
	if (stream->s_intrfunc == NULL) {
		return PFC_FALSE;
	}

	return stream->s_intrfunc(stream->s_intrarg);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * iostream_fetch_ucred(struct msghdr *PFC_RESTRICT msg,
 *			pfc_ucred_t *PFC_RESTRICT credp)
 *	Fetch the credentials in SCM_CREDENTIALS control message.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
iostream_fetch_ucred(struct msghdr *PFC_RESTRICT msg,
		     pfc_ucred_t *PFC_RESTRICT credp)
{
	struct cmsghdr	*cmsg;
	pfc_bool_t	received = PFC_FALSE;

	for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
	     cmsg = CMSG_NXTHDR(msg, cmsg)) {
		uint32_t	n;
		int		*src, *limit, type;

		if (PFC_EXPECT_FALSE(cmsg->cmsg_level != SOL_SOCKET)) {
			continue;
		}

		type = cmsg->cmsg_type;
		if (PFC_EXPECT_TRUE(type == SCM_CREDENTIALS)) {
			if (PFC_EXPECT_TRUE(!received)) {
				pfc_ucred_t	*cdp;

				/* Copy credentials. */
				cdp = (pfc_ucred_t *)CMSG_DATA(cmsg);
				PFC_ASSERT(PFC_IS_POW2_ALIGNED(&cdp->uid,
							       sizeof(uid_t)));
				*credp = *cdp;
				received = PFC_TRUE;
			}
			continue;
		}

		if (type != SCM_RIGHTS) {
			continue;
		}

		/* Close unwanted file descriptors. */
		src = (int *)CMSG_DATA(cmsg);
		PFC_ASSERT(PFC_IS_POW2_ALIGNED(src, sizeof(int)));
		n = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
		limit = src + n;
		for (; src < limit; src++) {
			iostream_close_fd(*src, "ucred");
		}
	}
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * iostream_getpollerr(pfc_iostream_t stream, int fd, short event,
 *		       short revents)
 *	Check error reported by ppoll(2) and determine error number.
 *
 *	`event' must be either POLLIN or POLLOUT.
 *
 * Calling/Exit State:
 *	Zero is returned if no error is reported by ppoll(2).
 *	Otherwise an error number which indicates the cause of error is
 *	returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
iostream_getpollerr(pfc_iostream_t stream, int fd, short event, short revents)
{
	if (event == POLLIN) {
		/*
		 * Return zero even on error state if POLLIN event is reported.
		 * Rest of data in file descriptor should be read by succeeding
		 * read(2) call.
		 */
		if (PFC_EXPECT_TRUE(revents & POLLIN)) {
			return 0;
		}
	}
	else {
		/* Check hang up if POLLOUT event is tested. */
		PFC_ASSERT(event == POLLOUT);
		if (PFC_EXPECT_FALSE(revents & POLLHUP)) {
			stream->s_flags |= IOSTRF_HUP;
			return EPIPE;
		}
	}

	if (PFC_EXPECT_FALSE(revents & POLLERR)) {
		if (IOSTREAM_IS_SOCKET(stream)) {
			return iostream_getsockerr(fd);
		}

		return EIO;
	}
	if (PFC_EXPECT_FALSE(revents & POLLNVAL)) {
		return EBADF;
	}

	return 0;
}

/*
 * int
 * pfc_iostream_create(pfc_iostream_t *PFC_RESTRICT streamp, int fd,
 *		       uint32_t insize, uint32_t outsize,
 *		       const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Create buffered I/O stream associated with the specified file
 *	descriptor.
 *
 *	Actual I/O on the stream is performed by non-blocking I/O, so
 *	O_NONBLOCK flag is set to the specified file descriptor.
 *
 *	`insize' and `outsize' are size of buffer for input and output.
 *	Zero means I/O should be done without buffering.
 *
 *	`iowait' determines behavior of I/O wait.
 *	- iw_intrfunc
 *	  Function which determines behavior of EINTR error on read or write.
 *	  It will be called if I/O function, such as pfc_iostream_read(),
 *	  gets EINTR error. If `iw_intrfunc' is NULL or it returns PFC_FALSE,
 *	  EINTR error is simply ignored. If it returns PFC_TRUE, I/O function
 *	  will terminate I/O immediately.
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
 *	Upon successful completion, buffered I/O stream instance is set to
 *	`*streamp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- EINTR callback will be called with holding the stream lock.
 *	  So the callback must not touch the stream.
 *
 *	- If `fd' is associated with a socket, it must be a STREAM socket.
 */
int
pfc_iostream_create(pfc_iostream_t *PFC_RESTRICT streamp, int fd,
		    uint32_t insize, uint32_t outsize,
		    const pfc_iowait_t *PFC_RESTRICT iowait)
{
	struct __pfc_iostream	*iop;
	socklen_t	slen;
	int		err, sflag;

	/* Allocate stream instance. */
	iop = (struct __pfc_iostream *)malloc(sizeof(*iop));
	if (PFC_EXPECT_FALSE(iop == NULL)) {
		return ENOMEM;
	}

	/* Allocate input/output buffer. */
	err = iostream_buffer_init(&iop->s_input, insize);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}
	err = iostream_buffer_init(&iop->s_output, outsize);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_input;
	}

	iop->s_input.io_enable = PFC_TRUE;
	iop->s_output.io_enable = PFC_TRUE;

	err = PFC_MUTEX_INIT(&iop->s_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_output;
	}
	err = pfc_cond_init(&iop->s_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_output;
	}

	/* Set O_NONBLOCK flag. */
	err = pfc_set_nonblock(fd, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_output;
	}

	/*
	 * Determine whether the file descriptor is associated with
	 * a socket or not.
	 */
	slen = sizeof(sflag);
	if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &sflag, &slen) == 0) {
		if (PFC_EXPECT_FALSE(sflag != SOCK_STREAM)) {
			/* Only STREAM socket is supported. */
			err = ESOCKTNOSUPPORT;
			goto error_output;
		}
		iop->s_flags = IOSTRF_SOCKET;
	}
	else {
		err = errno;
		if (PFC_EXPECT_FALSE(err != ENOTSOCK)) {
			goto error_output;
		}
		iop->s_flags = 0;
	}

	if (iowait != NULL) {
		if (iowait->iw_sigmask == NULL) {
			iop->s_sigmask = NULL;
		}
		else {
			sigset_t	*mask;

			/* Copy signal mask. */
			mask = (sigset_t *)malloc(sizeof(*mask));
			if (PFC_EXPECT_FALSE(mask == NULL)) {
				err = ENOMEM;
				goto error_output;
			}
			*mask = *(iowait->iw_sigmask);
			iop->s_sigmask = (const sigset_t *)mask;
		}
		iop->s_intrfunc = iowait->iw_intrfunc;
		iop->s_intrarg = iowait->iw_intrarg;
	}
	else {
		iop->s_sigmask = NULL;
		iop->s_intrfunc = NULL;
		iop->s_intrarg = NULL;
	}

	iop->s_fd = fd;
	iop->s_cancel = -1;
	iop->s_busy = PFC_FALSE;
	*streamp = iop;

	return 0;

error_output:
	iostream_buffer_free(&iop->s_output);

error_input:
	iostream_buffer_free(&iop->s_input);

error:
	free(iop);

	return err;
}

/*
 * int
 * pfc_iostream_destroy(pfc_iostream_t stream)
 *	Destroy the specified I/O stream.
 *
 *	The file descriptor associated with the stream is closed.
 *	Unlike fclose(), pfc_iostream_destroy() discards unwritten data
 *	in the output buffer. If you want to flush them before destroying,
 *	you must call pfc_iostream_flush() in advance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- Note that this function returns EBUSY if the stream is used by
 *	  another thread.
 *
 *	- Any further access after the call of pfc_iostream_destroy() causes
 *	  undefined behavior.
 */
int
pfc_iostream_destroy(pfc_iostream_t stream)
{
	int	err;

	IOSTREAM_LOCK(stream);

	if (PFC_EXPECT_FALSE(!IOSTREAM_IS_VALID(stream))) {
		IOSTREAM_LOG_ERROR("destroy: Invalid stream: %p", stream);
		err = EBADF;
		goto error;
	}
	if (PFC_EXPECT_FALSE(stream->s_busy)) {
		IOSTREAM_LOG_ERROR("destroy: Stream is busy: %p", stream);
		err = EBUSY;
		goto error;
	}

	/* Close file descriptor. */
	err = iostream_close(stream, "destroy");
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	stream->s_flags |= IOSTRF_DESROYED;
	IOSTREAM_BROADCAST(stream);
	IOSTREAM_UNLOCK(stream);

	iostream_buffer_free(&stream->s_input);
	iostream_buffer_free(&stream->s_output);
	free((void *)stream->s_sigmask);
	free(stream);

	return 0;

error:
	IOSTREAM_UNLOCK(stream);

	return err;
}

/*
 * void
 * __pfc_iostream_log_enable(void)
 *	Enable the internal logging.
 */
void
__pfc_iostream_log_enable(void)
{
	(void)pfc_atomic_swap_uint8((uint8_t *)&iostream_log_enabled,
				    PFC_TRUE);
}

/*
 * void
 * __pfc_iostream_dispose(pfc_iostream_t stream)
 *	Dispose the specified I/O stream without any sanity check.
 *
 * Remarks:
 *	This is not public interface.
 */
void
__pfc_iostream_dispose(pfc_iostream_t stream)
{
	(void)iostream_close(stream, "dispose");
	iostream_buffer_free(&stream->s_input);
	iostream_buffer_free(&stream->s_output);
	free((void *)stream->s_sigmask);
	free(stream);
}

/*
 * int
 * pfc_iostream_read(pfc_iostream_t PFC_RESTRICT stream, void *PFC_RESTRICT buf,
 *		     size_t *PFC_RESTRICT sizep,
 *		     const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Read data from the specified input stream.
 *
 *	pfc_iostream_read() reads up to `*sizep' bytes from the specified input
 *	stream into the buffer pointed by `buf'. `sizep' must be initialized
 *	to indicate the number of bytes to be read. On return, it contains the
 *	number of bytes actually read. If zero is specified as data size,
 *	pfc_iostream_read() returns zero immediately.
 *
 *	`timeout' specifies an upper limit on the amount of time which
 *	the calling thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually read. If the end of file is detected, zero is stored to
 *	`*sizep', and zero is returned.
 *
 * Remarks:
 *	The number of bytes to be read must be less than PFC_IOSTREAM_MAXSIZE.
 */
int
pfc_iostream_read(pfc_iostream_t PFC_RESTRICT stream, void *PFC_RESTRICT buf,
		  size_t *PFC_RESTRICT sizep,
		  const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	IOSTREAM_ABSTIME_SIZEP(timeout, abstime, &tspec, sizep);

	return pfc_iostream_read_abs(stream, buf, sizep, abstime);
}

/*
 * int
 * pfc_iostream_read_abs(pfc_iostream_t PFC_RESTRICT stream,
 *			 void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
 *			 const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Read data from the specified input stream.
 *
 *	This function is the same as pfc_iostream_read(), but timeout must be
 *	specified by the absolute system time. If `abstime' is not NULL,
 *	ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually read. If the end of file is detected, zero is stored to
 *	`*sizep', and zero is returned.
 *
 * Remarks:
 *	The number of bytes to be read must be less than PFC_IOSTREAM_MAXSIZE.
 */
int
pfc_iostream_read_abs(pfc_iostream_t PFC_RESTRICT stream,
		      void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
		      const pfc_timespec_t *PFC_RESTRICT abstime)
{
	iobuf_t		*bp = &stream->s_input;
	uint8_t		*dst = (uint8_t *)buf, *iobuf;
	size_t		count = *sizep, vsize, iosize;
	int		err;

	/* Filter out zero byte read. */
	if (PFC_EXPECT_FALSE(count == 0)) {
		return 0;
	}
	if (PFC_EXPECT_FALSE(count > PFC_IOSTREAM_MAXSIZE)) {
		err = E2BIG;
		goto out_result;
	}

	/* Acquire stream lock. */
	err = iostream_lock(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_result;
	}

	if (PFC_EXPECT_FALSE(bp->io_size == 0 ||
			     (!bp->io_enable && IOBUF_IS_EMPTY(bp)))) {
		/* Read without the input buffer. */
		err = iostream_direct_read(stream, buf, sizep, abstime);
		iostream_unlock(stream);

		return err;
	}

	/* Check to see whether valid data exists in the input buffer. */
	PFC_ASSERT(bp->io_top <= bp->io_bottom);
	if ((vsize = bp->io_bottom - bp->io_top) != 0) {
		uint8_t	*src = bp->io_buffer + bp->io_top;

		if (vsize >= count) {
			/* Whole data is in the buffer. */
			memcpy(dst, src, count);
			bp->io_top += count;
			count = 0;
			goto out;
		}
		memcpy(dst, src, vsize);
		bp->io_top += vsize;
		count -= vsize;
		dst += vsize;
	}

	if (!bp->io_enable) {
		/* The input buffer is disabled. */
		*sizep -= count;
		err = iostream_direct_read(stream, dst, sizep, abstime);
		iostream_unlock(stream);

		return err;
	}

	if (count < bp->io_size) {
		/* Use the input buffer and read ahead. */
		iobuf = bp->io_buffer;
		iosize = bp->io_size;
	}
	else {
		/* Read data into the specified buffer directly. */
		iobuf = dst;
		iosize = count;
	}

	if (PFC_EXPECT_FALSE(IOSTREAM_IS_RD_SHUTDOWN(stream))) {
		/* EOF has already been detected. */
		goto out;
	}

	PFC_ASSERT(IOBUF_IS_EMPTY(bp));
	while (1) {
		ssize_t	nbytes;

		/* Read data from the stream. */
		nbytes = iostream_read(stream, iobuf, iosize, abstime);
		if (nbytes == 0) {
			break;
		}
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		if ((size_t)nbytes >= count) {
			/* Sufficient data has been received. */
			if (iobuf != dst) {
				/*
				 * Copy data from the input buffer, and
				 * update buffer state.
				 */
				PFC_ASSERT(iobuf == bp->io_buffer);
				memcpy(dst, iobuf, count);
				bp->io_top = count;
				bp->io_bottom = nbytes;
			}
			count = 0;
			break;
		}

		/* More I/O is needed. */
		if (iobuf != dst) {
			/* Copy data from the input buffer. */
			PFC_ASSERT(iobuf == bp->io_buffer);
			memcpy(dst, iobuf, nbytes);
		}
		PFC_ASSERT(IOBUF_IS_EMPTY(bp));

		count -= nbytes;
		dst += nbytes;
		if (count < bp->io_size) {
			/* Switch to read ahead mode using the input buffer. */
			iobuf = bp->io_buffer;
			iosize = bp->io_size;
		}
		else {
			iobuf = dst;
			iosize = count;
		}
	}

out:
	iostream_unlock(stream);

out_result:
	*sizep -= count;

	return err;
}

/*
 * int
 * pfc_iostream_write(pfc_iostream_t PFC_RESTRICT stream, const void *buf,
 *		      size_t *PFC_RESTRICT sizep,
 *		      const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Write data to the specified output stream.
 *
 *	pfc_iostream_write() writes up to `*sizep' bytes from the buffer
 *	pointed by `buf' to the specified output stream. `sizep' must be
 *	initialized to indicate the number of bytes to be written. On return,
 *	it contains the number of bytes actually written. If zero is specified
 *	as data size, pfc_iostream_write() returns zero immediately.
 *
 *	`timeout' specifies an upper limit on the amount of time which
 *	the calling thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually written.
 *
 * Remarks:
 *	The number of bytes to be written must be less than
 *	PFC_IOSTREAM_MAXSIZE.
 */
int
pfc_iostream_write(pfc_iostream_t PFC_RESTRICT stream, const void *buf,
		   size_t *PFC_RESTRICT sizep,
		   const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	IOSTREAM_ABSTIME_SIZEP(timeout, abstime, &tspec, sizep);

	return pfc_iostream_write_abs(stream, buf, sizep, abstime);
}

/*
 * int
 * pfc_iostream_write_abs(pfc_iostream_t PFC_RESTRICT stream,
 *			  const void *PFC_RESTRICT buf,
 *			  size_t *PFC_RESTRICT sizep,
 *		          const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Write data to the specified output stream.
 *
 *	This function is the same as pfc_iostream_write(), but timeout must be
 *	specified by the absolute system time. If `abstime' is not NULL,
 *	ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually written.
 *
 * Remarks:
 *	The number of bytes to be written must be less than
 *	PFC_IOSTREAM_MAXSIZE.
 */
int
pfc_iostream_write_abs(pfc_iostream_t PFC_RESTRICT stream,
		       const void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
		       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	iobuf_t		*bp = &stream->s_output;
	const uint8_t	*src = (const uint8_t *)buf, *iobuf;
	size_t		count = *sizep, vsize, iosize;
	int		err;
	pfc_bool_t	flush = PFC_FALSE;

	/* Filter out zero byte write. */
	if (PFC_EXPECT_FALSE(count == 0)) {
		return 0;
	}
	if (PFC_EXPECT_FALSE(count > PFC_IOSTREAM_MAXSIZE)) {
		err = E2BIG;
		goto out_result;
	}

	/* Acquire stream lock. */
	err = iostream_lock(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_result;
	}

	if (PFC_EXPECT_FALSE(IOSTREAM_IS_WR_SHUTDOWN(stream))) {
		/* Hang up has already been detected. */
		err = EPIPE;
		goto out;
	}

	if (PFC_EXPECT_FALSE(bp->io_size == 0 || !bp->io_enable)) {
		/* Write without the output buffer. */
		PFC_ASSERT(IOBUF_IS_EMPTY(bp));
		err = iostream_direct_write(stream, buf, sizep, abstime);
		iostream_unlock(stream);

		return err;
	}

	/* Check to see whether the output buffer is available. */
	PFC_ASSERT(bp->io_bottom <= bp->io_size);
	if ((vsize = bp->io_size - bp->io_bottom) != 0) {
		uint8_t	*dst = bp->io_buffer + bp->io_bottom;

		if (vsize >= count) {
			/* Whole data can be written to the buffer. */
			memcpy(dst, src, count);
			bp->io_bottom += count;
			count = 0;
			goto out;
		}
		memcpy(dst, src, vsize);
		bp->io_bottom += vsize;
		count -= vsize;
		src += vsize;
	}

	if (bp->io_bottom != 0) {
		/* Need to flush unwritten data. */
		iobuf = (const uint8_t *)bp->io_buffer + bp->io_top;
		iosize = bp->io_bottom - bp->io_top;
		flush = PFC_TRUE;
	}
	else {
		/* Write data to the stream directly. */
		iobuf = src;
		iosize = count;
	}

	while (1) {
		ssize_t	nbytes;

		/* Write data to the stream. */
		nbytes = iostream_write(stream, iobuf, iosize, abstime);
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		if (flush) {
			if ((size_t)nbytes < iosize) {
				/* More unwritten data remains. */
				iobuf += nbytes;
				iosize -= nbytes;
				continue;
			}

			/* All unwritten data have been flushed. */
			PFC_ASSERT((size_t)nbytes == iosize);
			bp->io_top = 0;
			if (count <= bp->io_size) {
				/* Whole data can be written to the buffer. */
				memcpy(bp->io_buffer, src, count);
				bp->io_bottom = count;
				count = 0;
				break;
			}

			/* Write data to the stream directly. */
			flush = PFC_FALSE;
			bp->io_bottom = 0;
			iobuf = src;
			iosize = count;
			continue;
		}

		/* The output buffer must be empty. */
		PFC_ASSERT(bp->io_top == 0);
		PFC_ASSERT(IOBUF_IS_EMPTY(bp));
		PFC_ASSERT(count == iosize);

		count -= nbytes;
		if (count == 0) {
			/* Completed. */
			break;
		}

		src += nbytes;
		if (count <= bp->io_size) {
			/*
			 * The rest of data can be written to the output
			 * buffer.
			 */
			memcpy(bp->io_buffer, src, count);
			bp->io_bottom = count;
			count = 0;
			break;
		}

		iobuf = src;
		iosize = count;
	}

out:
	iostream_unlock(stream);

out_result:
	*sizep -= count;

	return err;
}

/*
 * int
 * pfc_iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
 *		      const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Flush all unwritten data in the output buffer.
 *
 *	`timeout' specifies an upper limit on the amount of time which
 *	the calling thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
		   const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	IOSTREAM_ABSTIME(timeout, abstime, &tspec);

	return pfc_iostream_flush_abs(stream, abstime);
}

/*
 * int
 * pfc_iostream_flush_abs(pfc_iostream_t PFC_RESTRICT stream,
 *			  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Flush all unwritten data in the output buffer.
 *
 *	This function is the same as pfc_iostream_flush(), but timeout must be
 *	specified by the absolute system time. If `abstime' is not NULL,
 *	ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_iostream_flush_abs(pfc_iostream_t PFC_RESTRICT stream,
		       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	int	err;

	/* Acquire stream lock. */
	err = iostream_lock(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Flush output buffer. */
	err = iostream_flush(stream, abstime);
	iostream_unlock(stream);

	return err;
}

/*
 * int
 * pfc_iostream_shutdown(pfc_iostream_t stream, uint32_t how)
 *	Shut down part of a full-duplex I/O stream.
 *
 *	If `how' is PFC_IOSTREAM_SHUT_RD, further data reading from the file
 *	descriptor is disabled. If `how' is PFC_IOSTREAM_SHUT_WR, further data
 *	writing to the file descriptor is disabled. Both are disabled if
 *	`how' is PFC_IOSTREAM_SHUT_RDWR.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- The file descriptor associated with the stream must be a socket.
 *
 *	- If `how' is PFC_IOSTREAM_SHUT_WR or PFC_IOSTREAM_SHUT_RDWR,
 *	  the output buffer must be empty. EBUSY is returned if not empty.
 *
 *	- The caller must guarantees that no other thread is performing I/O
 *	  on the specified stream. EBUSY is returned if not.
 */
int
pfc_iostream_shutdown(pfc_iostream_t stream, uint32_t how)
{
	uint32_t	flags;
	int		err = 0;

	if (PFC_EXPECT_FALSE(how & ~PFC_IOSTREAM_SHUT_RDWR)) {
		return EINVAL;
	}

	IOSTREAM_LOCK(stream);

	if (PFC_EXPECT_FALSE(stream->s_busy)) {
		err = EBUSY;
		goto out;
	}

	/* Clear flags which have already been processed. */
	flags = stream->s_flags;
	if (flags & IOSTRF_SHUT_RD) {
		how &= ~PFC_IOSTREAM_SHUT_RD;
	}
	if (flags & IOSTRF_SHUT_WR) {
		how &= ~PFC_IOSTREAM_SHUT_WR;
	}
	if (how == 0) {
		goto out;
	}

	if (how & PFC_IOSTREAM_SHUT_WR) {
		iobuf_t	*bp = &stream->s_output;

		/* The output buffer must be empty. */
		if (PFC_EXPECT_FALSE(bp->io_size != 0 &&
				     !IOBUF_IS_EMPTY(bp))) {
			err = EBUSY;
			goto out;
		}
	}

	if (IOSTREAM_IS_SOCKET(stream)) {
		int	shut;

		/* Shut down part of connection on the socket. */
		if (how == PFC_IOSTREAM_SHUT_RDWR) {
			shut = SHUT_RDWR;
		}
		else if (how == PFC_IOSTREAM_SHUT_RD) {
			shut = SHUT_RD;
		}
		else {
			PFC_ASSERT(how == PFC_IOSTREAM_SHUT_WR);
			shut = SHUT_WR;
		}

		/* Shut down the connection. */
		if (PFC_EXPECT_FALSE(shutdown(stream->s_fd, shut) != 0)) {
			err = errno;
			goto out;
		}
	}

	/* Free I/O buffer if we can. */
	if (how & PFC_IOSTREAM_SHUT_RD) {
		iobuf_t	*bp = &stream->s_input;

		if (bp->io_size != 0 && IOBUF_IS_EMPTY(bp)) {
			iostream_buffer_free(bp);
			PFC_ASSERT_INT(iostream_buffer_init(bp, 0), 0);
		}
		stream->s_flags = (flags | IOSTRF_SHUT_RD);
	}
	if (how & PFC_IOSTREAM_SHUT_WR) {
		iobuf_t	*bp = &stream->s_output;

		/* We have already ensured that the output buffer is empty. */
		iostream_buffer_free(bp);
		PFC_ASSERT_INT(iostream_buffer_init(bp, 0), 0);
		stream->s_flags = (flags | IOSTRF_SHUT_WR);
	}

out:
	IOSTREAM_UNLOCK(stream);

	return err;
}

/*
 * int
 * pfc_iostream_setcanceller(pfc_iostream_t stream, int fd)
 *	Set file descriptor for cancellation to the specified stream.
 *
 *	If valid file descriptor is specified to `fd', read input event
 *	on the specified file descriptor is also watched on I/O event wait.
 *	If any incoming read event is detected on the file descriptor,
 *	the following function returns ECANCELED.
 *
 *	- pfc_iostream_read()
 *	- pfc_iostream_read_abs()
 *	- pfc_iostream_write()
 *	- pfc_iostream_write_abs()
 *	- pfc_iostream_flush()
 *	- pfc_iostream_flush_abs()
 *	- __pfc_iostream_sendcred()
 *	- __pfc_iostream_sendcred_abs()
 *	- __pfc_iostream_recvcred()
 *	- __pfc_iostream_recvcred_abs()
 *
 *	Cancellation is disabled if -1 is specified to `fd'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EACCES is returned if the file descriptor specified by `fd' is
 *	not readable.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The specified file descriptor for cancellation is never closed by
 *	iostream API, even pfc_iostream_destroy().
 */
int
pfc_iostream_setcanceller(pfc_iostream_t stream, int fd)
{
	int	err = 0;

	/* Ensure that the given file descriptor is valid. */
	if (fd != -1) {
		int	fl = fcntl(fd, F_GETFL);

		if (PFC_EXPECT_FALSE(fl == -1)) {
			return errno;
		}

		if (PFC_EXPECT_FALSE((fl & O_ACCMODE) == O_WRONLY)) {
			return EACCES;
		}
	}

	IOSTREAM_LOCK(stream);

	if (PFC_EXPECT_FALSE(stream->s_busy)) {
		err = EBUSY;
		goto out;
	}

	if (PFC_EXPECT_FALSE(!IOSTREAM_IS_VALID(stream))) {
		err = EBADF;
		goto out;
	}
	if (PFC_EXPECT_FALSE(IOSTREAM_IS_SHUTDOWN(stream))) {
		/* This stream is no longer active. */
		err = ESHUTDOWN;
		goto out;
	}

	stream->s_cancel = fd;

out:
	IOSTREAM_UNLOCK(stream);

	return err;
}

/*
 * int
 * pfc_iostream_getfd(pfc_iostream_t stream)
 *	Return file descriptor set in the specified stream.
 */
int
pfc_iostream_getfd(pfc_iostream_t stream)
{
	return stream->s_fd;
}

/*
 * int
 * __pfc_iostream_setfd(pfc_iostream_t stream, int newfd)
 *	Change file descriptor associated with the stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This is not public interface.
 */
int
__pfc_iostream_setfd(pfc_iostream_t stream, int newfd)
{
	int	err;

	IOSTREAM_LOCK(stream);

	if (PFC_EXPECT_TRUE(!stream->s_busy)) {
		stream->s_fd = newfd;
		err = 0;
	}
	else {
		IOSTREAM_LOG_ERROR("Unable to set FD: stream=%p, fd=%d, "
				   "newfd=%d", stream, stream->s_fd, newfd);
		err = EBUSY;
	}

	IOSTREAM_UNLOCK(stream);

	return err;
}

/*
 * int
 * __pfc_iostream_resize(pfc_iostream_t stream, uint32_t insize,
 *			 uint32_t outsize)
 *	Resize I/O buffer.
 *
 *	`insize' is a new size of input buffer, and `outsize' is a new size
 *	of output buffer. If PFC_IOSSZ_RETAIN is specified as buffer size,
 *	the size of associated buffer is unchanged.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Note that this function returns EBUSY if the stream is used by
 *	another thread.
 */
int
__pfc_iostream_resize(pfc_iostream_t stream, uint32_t insize, uint32_t outsize)
{
	uint8_t		*oldin, *oldout;
	uint32_t	oldinsz, oldoutsz;
	iobuf_t		*in = &stream->s_input;
	iobuf_t		*out = &stream->s_output;
	int		err;

	IOSTREAM_LOCK(stream);

	if (PFC_EXPECT_FALSE(stream->s_busy)) {
		err = EBUSY;
		goto error_unlock;
	}

	/* Preserve current buffers for rollback. */
	oldin = in->io_buffer;
	oldinsz = in->io_size;
	oldout = out->io_buffer;
	oldoutsz = out->io_size;

	/* Check whether buffer sizes are actually changed. */
	if (insize == oldinsz) {
		insize = PFC_IOSSZ_RETAIN;
	}
	if (outsize == oldoutsz) {
		outsize = PFC_IOSSZ_RETAIN;
	}

	/* Update I/O buffers. */
	if (insize != PFC_IOSSZ_RETAIN) {
		err = iostream_buffer_resize(in, insize);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto rollback;
		}
	}
	if (outsize != PFC_IOSSZ_RETAIN) {
		err = iostream_buffer_resize(out, outsize);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto rollback;
		}
	}

	/* Free up old buffers. */
	if (insize != PFC_IOSSZ_RETAIN) {
		free(oldin);
	}
	if (outsize != PFC_IOSSZ_RETAIN) {
		free(oldout);
	}

	IOSTREAM_UNLOCK(stream);

	return 0;

rollback:
	if (in->io_buffer != oldin) {
		free(in->io_buffer);
		in->io_buffer = oldin;
		in->io_size = oldinsz;
	}
	if (out->io_buffer != oldout) {
		free(out->io_buffer);
		out->io_buffer = oldout;
		out->io_size = oldoutsz;
	}

error_unlock:
	IOSTREAM_UNLOCK(stream);

	return err;
}

/*
 * int
 * __pfc_iostream_sendcred(pfc_iostream_t PFC_RESTRICT stream,
 *			   const void *PFC_RESTRICT buf,
 *			   size_t *PFC_RESTRICT sizep,
 *			   pfc_cucred_t *PFC_RESTRICT credp,
 *			   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Write arbitrary data to the specified output stream, and send the
 *	credentials of the calling process.
 *
 *	A file descriptor associated with `stream' must be an UNIX domain
 *	stream socket, which is already connected. This function writes up to
 *	`*sizep' bytes from the buffer pointed by `buf' to the specified
 *	output stream. At the same time, the credentials of the calling
 *	process specified by `credp' is sent to a process which owns other
 *	side of the stream socket.
 *
 *	If NULL is passed to `credp', the credentials is initialized by this
 *	function with the current process ID, real user ID, and real group ID.
 *	If a non NULL pointer is passed to `credp', the credentials must be
 *	initialized by the caller in advance.
 *
 *	`sizep' must be initialized to indicate the number of bytes to be
 *	written. If zero is specified as data size, this function returns
 *	zero immediately.
 *
 *	`timeout' specifies an upper limit on the amount of time which
 *	the calling thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually written.
 *
 * Remarks:
 *	- The number of bytes to be written must be less than
 *	  PFC_IOSTREAM_MAXSIZE.
 *
 *	- This function may flush any data in output buffer before sending
 *	  data and file descriptors.
 */
int
__pfc_iostream_sendcred(pfc_iostream_t PFC_RESTRICT stream,
			const void *PFC_RESTRICT buf,
			size_t *PFC_RESTRICT sizep,
			pfc_cucred_t *PFC_RESTRICT credp,
			const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	IOSTREAM_ABSTIME_SIZEP(timeout, abstime, &tspec, sizep);

	return __pfc_iostream_sendcred_abs(stream, buf, sizep, credp, abstime);
}

/*
 * int
 * __pfc_iostream_sendcred_abs(pfc_iostream_t PFC_RESTRICT stream,
 *			       const void *PFC_RESTRICT buf,
 *			       size_t *PFC_RESTRICT sizep,
 *			       pfc_cucred_t *PFC_RESTRICT credp,
 *			       const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Write arbitrary data to the specified output stream, and send the
 *	credentials of the calling process.
 *
 *	This function is similar to __pfc_iostream_sendcred(), but timeout
 *	must be specified by the absolute system time. If `abstime' is not
 *	NULL, ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually written.
 *
 * Remarks:
 *	- The number of bytes to be written must be less than
 *	  PFC_IOSTREAM_MAXSIZE.
 *
 *	- This function may flush any data in output buffer before sending
 *	  data and file descriptors.
 */
int
__pfc_iostream_sendcred_abs(pfc_iostream_t PFC_RESTRICT stream,
			    const void *PFC_RESTRICT buf,
			    size_t *PFC_RESTRICT sizep,
			    pfc_cucred_t *PFC_RESTRICT credp,
			    const pfc_timespec_t *PFC_RESTRICT abstime)
{
	cmsg_credctx_t	crctx;
	pfc_ucred_t	credbuf;

	if (credp == NULL) {
		/* Initialize the credentials with real UID and GID. */
		credbuf.pid = getpid();
		credbuf.uid = getuid();
		credbuf.gid = getgid();
		credp = &credbuf;
	}

	CMSG_CREDCTX_INIT(&crctx, stream, buf, sizep, credp, abstime);

	return iostream_sendmsg(&crctx.cmc_context);
}

/*
 * int
 * __pfc_iostream_recvcred(pfc_iostream_t PFC_RESTRICT stream,
 *			   void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
 *			   pfc_ucred_t *PFC_RESTRICT credp,
 *			   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Read data from the specified stream, and receive the credentials
 *	of the process.
 *
 *	A file descriptor associated with `stream' must be an UNIX domain
 *	stream socket, which is already connected. This function reads up to
 *	`*sizep' bytes from the specified input stream into the buffer pointed
 *	by `buf'. At the same time, the credentials of the process sent by
 *	the call of __pfc_iostream_sendcred() or __pfc_iostream_sendcred_abs()
 *	is copied to the buffer pointed by `credp'. Note that `credp' must not
 *	be NULL.
 *
 *	`sizep' must be initialized to indicate the number of bytes to be read.
 *	On return, it contains the number of bytes actually read. If zero is
 *	specified as data size, this function returns zero immediately.
 *
 *	`timeout' specifies an upper limit on the amount of time which
 *	the calling thread is blocked. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually read. If the end of file is detected, zero is stored to
 *	`*sizep', and zero is returned.
 *
 * Remarks:
 *	- The number of bytes to be read must be less than
 *	  PFC_IOSTREAM_MAXSIZE.
 *
 *	- If the number of bytes specified by `*sizep' already exists in the
 *	  input buffer, this function copies it to the buffer pointed by
 *	  `buf' without receiving the credentials. In this case, -1 is stored
 *	  to all fields in `credp'.
 */
int
__pfc_iostream_recvcred(pfc_iostream_t PFC_RESTRICT stream,
			void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
			pfc_ucred_t *PFC_RESTRICT credp,
			const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_timespec_t	tspec, *abstime;

	/* Convert timeout period into system absolute time. */
	IOSTREAM_ABSTIME_SIZEP(timeout, abstime, &tspec, sizep);

	return __pfc_iostream_recvcred_abs(stream, buf, sizep, credp, abstime);
}

/*
 * int
 * __pfc_iostream_recvcred_abs(pfc_iostream_t PFC_RESTRICT stream,
 *			       void *PFC_RESTRICT buf,
 *			       size_t *PFC_RESTRICT sizep,
 *			       pfc_ucred_t *PFC_RESTRICT credp,
 *			       const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Read data from the specified stream, and receive the credentials
 *	of the process.
 *
 *	This function is similar to __pfc_iostream_recvcred(), but timeout must
 *	be specified by the absolute system time. If `abstime' is not NULL,
 *	ETIMEDOUT is returned if the absolute time specified by `abstime'
 *	passes. NULL means an infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually read. If the end of file is detected, zero is stored to
 *	`*sizep', and zero is returned.
 *
 * Remarks:
 *	- The number of bytes to be read must be less than
 *	  PFC_IOSTREAM_MAXSIZE.
 *
 *	- If the number of bytes specified by `*sizep' already exists in the
 *	  input buffer, this function copies it to the buffer pointed by
 *	  `buf' without receiving the credentials. In this case, -1 is stored
 *	  to all fields in `credp'.
 */
int
__pfc_iostream_recvcred_abs(pfc_iostream_t PFC_RESTRICT stream,
			    void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
			    pfc_ucred_t *PFC_RESTRICT credp,
			    const pfc_timespec_t *PFC_RESTRICT abstime)
{
	cmsg_credctx_t	crctx;

	CMSG_CREDCTX_INIT(&crctx, stream, buf, sizep, credp, abstime);

	/* Set undefined value to all fields in the credentials. */
	credp->pid = -1;
	credp->uid = -1;
	credp->gid = -1;

	return iostream_recvmsg(&crctx.cmc_context);
}

/*
 * static int
 * iostream_buffer_init(iobuf_t *bp, uint32_t size)
 *	Initialize I/O buffer.
 */
static int
iostream_buffer_init(iobuf_t *bp, uint32_t size)
{
	bp->io_top = 0;
	bp->io_bottom = 0;
	bp->io_size = size;

	if (size == 0) {
		bp->io_buffer = NULL;
	}
	else {
		bp->io_buffer = (uint8_t *)malloc(size);
		if (PFC_EXPECT_FALSE(bp->io_buffer == NULL)) {
			return ENOMEM;
		}
	}

	return 0;
}

/*
 * static int
 * iostream_buffer_resize(iobuf_t *bp, uint32_t size)
 *	Resize I/O buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function never frees old buffer.
 */
static int
iostream_buffer_resize(iobuf_t *bp, uint32_t size)
{
	PFC_ASSERT(size != PFC_IOSSZ_RETAIN);

	if (PFC_EXPECT_FALSE(IOBUF_IS_EMPTY(bp))) {
		bp->io_top = bp->io_bottom = 0;
	}

	if (PFC_EXPECT_FALSE(size < bp->io_size && !IOBUF_IS_EMPTY(bp) &&
			     size < bp->io_bottom)) {
		/* Valid data still exists in shrinked area in this buffer. */
		return ENOSPC;
	}

	if (size == 0) {
		bp->io_buffer = NULL;
	}
	else {
		uint8_t		*newbuf;
		uint32_t	bottom;

		newbuf = (uint8_t *)malloc(size);
		if (PFC_EXPECT_FALSE(newbuf == NULL)) {
			return ENOMEM;
		}

		if ((bottom = bp->io_bottom) != 0) {
			PFC_ASSERT(size >= bp->io_bottom);
			memcpy(newbuf, bp->io_buffer, bottom);
		}

		bp->io_buffer = newbuf;
	}

	bp->io_size = size;

	return 0;
}

/*
 * static int
 * iostream_direct_read(pfc_iostream_t PFC_RESTRICT stream,
 *			void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
 *			const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Read data from the specified input stream without buffering.
 *	`abstime' is an expiry time of read.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually read. If the end of file is detected, zero is stored to
 *	`*sizep', and zero is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static int
iostream_direct_read(pfc_iostream_t PFC_RESTRICT stream, void *PFC_RESTRICT buf,
		     size_t *PFC_RESTRICT sizep,
		     const pfc_timespec_t *PFC_RESTRICT abstime)
{
	uint8_t	*dst = (uint8_t *)buf;
	size_t	count = *sizep;
	int	err = 0;

	if (PFC_EXPECT_FALSE(IOSTREAM_IS_RD_SHUTDOWN(stream))) {
		/* EOF has already been detected. */
		goto out;
	}

	while (1) {
		ssize_t	nbytes;

		/* Read data from the stream. */
		nbytes = iostream_read(stream, dst, count, abstime);
		if (nbytes == 0) {
			break;
		}
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		count -= nbytes;
		if (count == 0) {
			break;
		}

		dst += nbytes;
	}

out:
	*sizep -= count;

	return err;
}

/*
 * static int
 * iostream_direct_write(pfc_iostream_t PFC_RESTRICT stream,
 *			 const void *PFC_RESTRICT buf,
 *			 size_t *PFC_RESTRICT sizep,
 *			 const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Write data to the specified input stream without buffering.
 *
 *	`abstime' is an expiry time of write.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 *	Irrespective of the result, `*sizep' contains the number of bytes
 *	actually written.
 */
static int
iostream_direct_write(pfc_iostream_t PFC_RESTRICT stream,
		      const void *PFC_RESTRICT buf, size_t *PFC_RESTRICT sizep,
		      const pfc_timespec_t *PFC_RESTRICT abstime)
{
	const uint8_t	*src = (const uint8_t *)buf;
	size_t		count = *sizep;
	int		err = 0;

	/*
	 * Remarks:
	 *	Shutdown flag must have already been checked by the caller.
	 */
	while (1) {
		ssize_t	nbytes;

		/* Write data to the stream. */
		nbytes = iostream_write(stream, src, count, abstime);
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		count -= nbytes;
		if (count == 0) {
			break;
		}

		src += nbytes;
	}

	*sizep -= count;

	return err;
}

/*
 * static ssize_t
 * iostream_read(pfc_iostream_t PFC_RESTRICT stream, uint8_t *PFC_RESTRICT buf,
 *		 size_t size, const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Read data from the stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of bytes actually read is
 *	returned. If the end of file is detected, zero is returned.
 *	Otherwise negative error number which indicates the cause of error
 *	is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static ssize_t
iostream_read(pfc_iostream_t PFC_RESTRICT stream, uint8_t *PFC_RESTRICT buf,
	      size_t size, const pfc_timespec_t *PFC_RESTRICT abstime)
{
	struct pollfd	pfd[2];
	nfds_t		nfds;
	ssize_t		nbytes;
	int		fd = stream->s_fd;
	pfc_timespec_t	*timeout, tspec;

	pfd[0].fd = fd;
	pfd[0].events = POLLIN;

	if ((pfd[1].fd = stream->s_cancel) == -1) {
		nfds = 1;
	}
	else {
		/* Watch cancellation event. */
		pfd[1].events = POLLIN;
		nfds = 2;
	}

	while (1) {
		int	n, err;

		pfd[0].revents = 0;
		pfd[1].revents = 0;

		if (abstime != NULL) {
			/* Convert expiry data into timeout period. */
			timeout = &tspec;
			err = pfc_clock_isexpired(timeout, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return (ssize_t)-err;
			}
		}
		else {
			timeout = NULL;
		}

		/* Wait until the stream can receive data without blocking. */
		n = ppoll(pfd, nfds, timeout, stream->s_sigmask);
		if (PFC_EXPECT_FALSE(n == -1)) {
			if ((err = errno) == EINTR &&
			    !iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (PFC_EXPECT_FALSE(n == 0)) {
			return -ETIMEDOUT;
		}

		if (pfd[1].fd != -1 && pfd[1].revents != 0) {
			/* Cancellation event has been received. */
			return -ECANCELED;
		}

		err = iostream_getpollerr(stream, fd, POLLIN, pfd[0].revents);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return (ssize_t)-err;
		}

		/* Read data into the specified buffer. */
		nbytes = read(fd, buf, size);
		if (nbytes == 0) {
			/* EOF has been detected. */
			stream->s_flags |= IOSTRF_EOF;

			break;
		}
		if (PFC_EXPECT_TRUE(nbytes > 0)) {
			break;
		}

		if ((err = errno) == EINTR) {
			if (!iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		else if (!PFC_IS_EWOULDBLOCK(err)) {
			return (ssize_t)-err;
		}
	}

	return nbytes;
}

/*
 * static ssize_t
 * iostream_write(pfc_iostream_t PFC_RESTRICT stream, uint8_t *PFC_RESTRICT buf,
 *		  size_t size, const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Write data to the stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of bytes actually written is
 *	returned.
 *	Otherwise negative error number which indicates the cause of error
 *	is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static ssize_t
iostream_write(pfc_iostream_t PFC_RESTRICT stream,
	       const uint8_t *PFC_RESTRICT buf, size_t size,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	struct pollfd	pfd[2];
	nfds_t		nfds;
	ssize_t		nbytes;
	int		fd = stream->s_fd;

	pfd[0].fd = fd;
	pfd[0].events = POLLOUT;

	if ((pfd[1].fd = stream->s_cancel) == -1) {
		nfds = 1;
	}
	else {
		/* Watch cancellation event. */
		pfd[1].events = POLLIN;
		nfds = 2;
	}

	while (1) {
		int	n, err;
		pfc_timespec_t	*timeout, tspec;

		pfd[0].revents = 0;
		pfd[1].revents = 0;

		if (abstime != NULL) {
			/* Convert expiry data into timeout period. */
			timeout = &tspec;
			err = pfc_clock_isexpired(timeout, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return (ssize_t)-err;
			}
		}
		else {
			timeout = NULL;
		}

		/* Wait until the stream can accept write request. */
		n = ppoll(pfd, nfds, timeout, stream->s_sigmask);
		if (PFC_EXPECT_FALSE(n == -1)) {
			if ((err = errno) == EINTR &&
			    !iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (PFC_EXPECT_FALSE(n == 0)) {
			return -ETIMEDOUT;
		}

		if (pfd[1].fd != -1 && pfd[1].revents != 0) {
			/* Cancellation event has been received. */
			return -ECANCELED;
		}

		err = iostream_getpollerr(stream, fd, POLLOUT, pfd[0].revents);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return (ssize_t)-err;
		}

		/* Write data. */
		nbytes = write(fd, buf, size);
		if (PFC_EXPECT_TRUE(nbytes >= 0)) {
			break;
		}

		if ((err = errno) == EINTR) {
			if (!iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (err == EPIPE) {
			stream->s_flags |= IOSTRF_HUP;

			return -EPIPE;
		}
		if (!PFC_IS_EWOULDBLOCK(err)) {
			return (ssize_t)-err;
		}
	}

	return nbytes;
}

/*
 * static int
 * iostream_sendmsg(cmsg_ctx_t *ctx)
 *	Send control message defined by `ctx'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned. `*ctx->cm_sizep' is
 *	updated to the data size actually sent.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
iostream_sendmsg(cmsg_ctx_t *ctx)
{
	size_t		count, cbufsize;
	char		*cbuf = NULL;
	pfc_iostream_t	stream = ctx->cm_stream;
	cmsg_cops_t	*ops = ctx->cm_ops;
	const uint8_t	*src;
	struct msghdr	*msg;
	struct iovec	iov;
	int		err, mflags;
	const pfc_timespec_t	*abstime = ctx->cm_abstime;

	/* Filter out zero byte write and too large size. */
	count = *(ctx->cm_sizep);
	if (PFC_EXPECT_FALSE(count == 0)) {
		return 0;
	}
	if (PFC_EXPECT_FALSE(count > PFC_IOSTREAM_MAXSIZE)) {
		err = E2BIG;
		goto out_result;
	}

	/* Allocate buffer for control message. */
	cbufsize = CMSG_SPACE(ops->cmops_size(ctx));
	cbuf = (char *)malloc(cbufsize);
	if (PFC_EXPECT_FALSE(cbuf == NULL)) {
		err = ENOMEM;
		goto out_result;
	}

	/* Acquire stream lock. */
	err = iostream_lock(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_result;
	}

	/* Flush any unwritten data in output buffer. */
	err = iostream_flush(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Set up message header. */
	msg = &ctx->cm_header;
	msg->msg_iov = &iov;
	msg->msg_iovlen = 1;
	msg->msg_flags = 0;
	msg->msg_name = NULL;
	msg->msg_namelen = 0;

	/* Set up control message to send set of file descriptors. */
	msg->msg_control = cbuf;
	msg->msg_controllen = cbufsize;
	ops->cmops_setup(ctx);

	src = (const uint8_t *)ctx->cm_buffer;
	mflags = ctx->cm_mflags;
	for (;;) {
		ssize_t	nbytes;

		iov.iov_base = (void *)src;
		iov.iov_len = count;

		/* Send a message to the stream. */
		nbytes = iostream_do_sendmsg(stream, msg, mflags, abstime);
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		count -= nbytes;
		if (count == 0) {
			/* Completed. */
			break;
		}

		/* Clear control message. */
		msg->msg_control = NULL;
		msg->msg_controllen = 0;
		src += nbytes;
	}

out:
	iostream_unlock(stream);

out_result:
	*(ctx->cm_sizep) -= count;
	free(cbuf);

	return err;
}

/*
 * static ssize_t
 * iostream_do_sendmsg(pfc_iostream_t PFC_RESTRICT stream,
 *		       const struct msghdr *PFC_RESTRICT msg, int mflags,
 *		       const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Send the message specified by `msg' to the stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of bytes actually written is
 *	returned.
 *	Otherwise negative error number which indicates the cause of error
 *	is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static ssize_t
iostream_do_sendmsg(pfc_iostream_t PFC_RESTRICT stream,
		    const struct msghdr *PFC_RESTRICT msg, int mflags,
		    const pfc_timespec_t *PFC_RESTRICT abstime)
{
	struct pollfd	pfd[2];
	nfds_t		nfds;
	ssize_t		nbytes;
	int		fd = stream->s_fd;

	pfd[0].fd = fd;
	pfd[0].events = POLLOUT;

	if ((pfd[1].fd = stream->s_cancel) == -1) {
		nfds = 1;
	}
	else {
		/* Watch cancellation event. */
		pfd[1].events = POLLIN;
		nfds = 2;
	}

	while (1) {
		int	n, err;
		pfc_timespec_t	*timeout, tspec;

		pfd[0].revents = 0;
		pfd[1].revents = 0;

		if (abstime != NULL) {
			/* Convert expiry data into timeout period. */
			timeout = &tspec;
			err = pfc_clock_isexpired(timeout, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return (ssize_t)-err;
			}
		}
		else {
			timeout = NULL;
		}

		/* Wait until the stream can accept write request. */
		n = ppoll(pfd, nfds, timeout, stream->s_sigmask);
		if (PFC_EXPECT_FALSE(n == -1)) {
			if ((err = errno) == EINTR &&
			    !iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (PFC_EXPECT_FALSE(n == 0)) {
			return -ETIMEDOUT;
		}

		if (pfd[1].fd != -1 && pfd[1].revents != 0) {
			/* Cancellation event has been received. */
			return -ECANCELED;
		}

		err = iostream_getpollerr(stream, fd, POLLOUT, pfd[0].revents);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return (ssize_t)-err;
		}

		/* Send a message. */
		nbytes = sendmsg(fd, msg, mflags);
		if (PFC_EXPECT_TRUE(nbytes >= 0)) {
			break;
		}

		if ((err = errno) == EINTR) {
			if (!iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (err == EPIPE) {
			stream->s_flags |= IOSTRF_HUP;

			return -EPIPE;
		}
		if (!PFC_IS_EWOULDBLOCK(err)) {
			return (ssize_t)-err;
		}
	}

	return nbytes;
}

/*
 * static int
 * iostream_recvmsg(cmsg_ctx_t *ctx)
 *	Read control message defined by `ctx'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned. `*ctx->cm_sizep' is
 *	updated to the data size actually received.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
iostream_recvmsg(cmsg_ctx_t *ctx)
{
	size_t		count, cbufsize;
	char		*cbuf = NULL;
	pfc_iostream_t	stream = ctx->cm_stream;
	cmsg_cops_t	*ops = ctx->cm_ops;
	iobuf_t		*bp;
	uint8_t		*dst;
	struct msghdr	*msg;
	struct iovec	iov;
	int		err, mflags;
	const pfc_timespec_t	*abstime = ctx->cm_abstime;

	/* Filter out zero byte read. */
	count = *(ctx->cm_sizep);
	if (PFC_EXPECT_FALSE(count == 0)) {
		err = 0;
		goto out_result;
	}
	if (PFC_EXPECT_FALSE(count > PFC_IOSTREAM_MAXSIZE)) {
		err = E2BIG;
		goto out_result;
	}

	/* Allocate buffer for control message. */
	cbufsize = CMSG_SPACE(ops->cmops_size(ctx));
	cbuf = (char *)malloc(cbufsize);
	if (PFC_EXPECT_FALSE(cbuf == NULL)) {
		err = ENOMEM;
		goto out_result;
	}

	/* Acquire stream lock. */
	err = iostream_lock(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_result;
	}

	if (PFC_EXPECT_FALSE(IOSTREAM_IS_RD_SHUTDOWN(stream))) {
		/* EOF has already been detected. */
		goto out;
	}

	bp = &stream->s_input;
	dst = (uint8_t *)ctx->cm_buffer;
	if (bp->io_size != 0) {
		uint8_t	*src;
		size_t	vsize = bp->io_bottom - bp->io_top;

		/* Copy data in the input buffer. */
		PFC_ASSERT(bp->io_top <= bp->io_bottom);
		if (vsize >= count) {
			/* Whole data is in the input buffer. */
			PFC_ASSERT(bp->io_enable);
			src = bp->io_buffer + bp->io_top;
			memcpy(dst, src, count);
			bp->io_top += count;
			count = 0;
			goto out;
		}
		else if (vsize != 0) {
			PFC_ASSERT(bp->io_enable);
			src = bp->io_buffer + bp->io_top;
			memcpy(dst, src, vsize);
			bp->io_top += vsize;
			count -= vsize;
			dst += vsize;
		}
		PFC_ASSERT(IOBUF_IS_EMPTY(bp));
	}

	if (PFC_EXPECT_FALSE(IOSTREAM_IS_RD_SHUTDOWN(stream))) {
		/* EOF has already been detected. */
		goto out;
	}

	/* Set up message header. */
	msg = &ctx->cm_header;
	msg->msg_iov = &iov;
	msg->msg_iovlen = 1;
	msg->msg_flags = 0;
	msg->msg_name = NULL;
	msg->msg_namelen = 0;
	msg->msg_control = cbuf;
	msg->msg_controllen = cbufsize;

	if (ops->cmops_fetch_prepare != NULL) {
		err = ops->cmops_fetch_prepare(ctx);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}
	}

	mflags = ctx->cm_mflags;
	for (;;) {
		ssize_t	nbytes;

		iov.iov_base = (void *)dst;
		iov.iov_len = count;

		/* Receive a message from the stream. */
		nbytes = iostream_do_recvmsg(stream, msg, mflags, abstime);
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		/* Fetch control message. */
		ops->cmops_fetch(ctx);

		if (nbytes == 0) {
			break;
		}

		PFC_ASSERT((size_t)nbytes <= count);
		count -= nbytes;
		if (count == 0) {
			break;
		}
		dst += nbytes;
	}

	if (ops->cmops_fetch_cleanup != NULL) {
		int	e;

		e = ops->cmops_fetch_cleanup(ctx);
		if (PFC_EXPECT_FALSE(e != 0 && err == 0)) {
			err = e;
		}
	}

out:
	iostream_unlock(stream);

out_result:
	*(ctx->cm_sizep) -= count;
	free(cbuf);

	return err;
}

/*
 * static ssize_t
 * iostream_do_recvmsg(pfc_iostream_t PFC_RESTRICT stream,
 *		       struct msghdr *PFC_RESTRICT msg, int mflags,
 *		       const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Read a message from the stream.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of bytes actually read is
 *	returned. If the end of file is detected, zero is returned.
 *	Otherwise negative error number which indicates the cause of error
 *	is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static ssize_t
iostream_do_recvmsg(pfc_iostream_t PFC_RESTRICT stream,
		    struct msghdr *PFC_RESTRICT msg, int mflags,
		    const pfc_timespec_t *PFC_RESTRICT abstime)
{
	struct pollfd	pfd[2];
	nfds_t		nfds;
	ssize_t		nbytes;
	int		fd = stream->s_fd;
	pfc_timespec_t	*timeout, tspec;

	pfd[0].fd = fd;
	pfd[0].events = POLLIN;

	if ((pfd[1].fd = stream->s_cancel) == -1) {
		nfds = 1;
	}
	else {
		/* Watch cancellation event. */
		pfd[1].events = POLLIN;
		nfds = 2;
	}

	while (1) {
		int	n, err;

		pfd[0].revents = 0;
		pfd[1].revents = 0;

		if (abstime != NULL) {
			/* Convert expiry data into timeout period. */
			timeout = &tspec;
			err = pfc_clock_isexpired(timeout, abstime);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return (ssize_t)-err;
			}
		}
		else {
			timeout = NULL;
		}

		/* Wait until the stream can receive data without blocking. */
		n = ppoll(pfd, nfds, timeout, stream->s_sigmask);
		if (PFC_EXPECT_FALSE(n == -1)) {
			if ((err = errno) == EINTR &&
			    !iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		if (PFC_EXPECT_FALSE(n == 0)) {
			return -ETIMEDOUT;
		}

		if (pfd[1].fd != -1 && pfd[1].revents != 0) {
			/* Cancellation event has been received. */
			return -ECANCELED;
		}

		err = iostream_getpollerr(stream, fd, POLLIN, pfd[0].revents);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return (ssize_t)-err;
		}

		/* Receive a message into the specified message header. */
		nbytes = recvmsg(fd, msg, mflags);
		if (nbytes == 0) {
			/* EOF has been detected. */
			stream->s_flags |= IOSTRF_EOF;

			break;
		}
		if (PFC_EXPECT_TRUE(nbytes > 0)) {
			break;
		}

		if ((err = errno) == EINTR) {
			if (!iostream_intr_callback(stream)) {
				continue;
			}

			return (ssize_t)-err;
		}
		else if (!PFC_IS_EWOULDBLOCK(err)) {
			return (ssize_t)-err;
		}
	}

	return nbytes;
}

/*
 * static int
 * iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
 *		  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Flush all unwritten data in the output buffer.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must call this function with holding the stream lock.
 */
static int
iostream_flush(pfc_iostream_t PFC_RESTRICT stream,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	iobuf_t		*bp = &stream->s_output;
	const uint8_t	*src;
	size_t		count;
	int		err;

	if (PFC_EXPECT_FALSE(bp->io_size == 0)) {
		/* Nothing to do. */
		return 0;
	}

	if (PFC_EXPECT_FALSE(stream->s_flags & IOSTRF_HUP)) {
		/* Hang up has already been detected. */
		return EPIPE;
	}

	PFC_ASSERT(bp->io_top <= bp->io_bottom);
	count = bp->io_bottom - bp->io_top;
	if (count == 0) {
		/* No unwritten data. */
		bp->io_top = bp->io_bottom = 0;

		return 0;
	}

	src = bp->io_buffer + bp->io_top;
	while (1) {
		ssize_t	nbytes;

		/* Write data to the stream. */
		nbytes = iostream_write(stream, src, count, abstime);
		if (PFC_EXPECT_FALSE(nbytes < 0)) {
			err = -nbytes;
			break;
		}

		count -= nbytes;
		bp->io_top += nbytes;
		if (count == 0) {
			/* Completed. */
			bp->io_top = bp->io_bottom = 0;
			err = 0;
			break;
		}
		src += nbytes;
	}

	return err;
}

/*
 * static int
 * iostream_lock(pfc_iostream_t PFC_RESTRICT stream,
 *	         const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Acquire lock for the specified stream.
 */
static int
iostream_lock(pfc_iostream_t PFC_RESTRICT stream,
	      const pfc_timespec_t *PFC_RESTRICT abstime)
{
	int	err;

	err = IOSTREAM_TIMEDLOCK_ABS(stream, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	while (stream->s_busy) {
		err = IOSTREAM_TIMEDWAIT_ABS(stream, abstime);

		if (PFC_EXPECT_FALSE(err != 0)) {
			if (err != EINTR || iostream_intr_callback(stream)) {
				IOSTREAM_UNLOCK(stream);

				return err;
			}
		}
		if (PFC_EXPECT_FALSE(!IOSTREAM_IS_VALID(stream))) {
			IOSTREAM_UNLOCK(stream);

			return EBADF;
		}

		PFC_MEMORY_RELOAD();
	}
	stream->s_busy = PFC_TRUE;

	IOSTREAM_UNLOCK(stream);

	return 0;
}

/*
 * static void
 * iostream_unlock(pfc_iostream_t stream)
 *	Release lock for the specified stream.
 */
static void
iostream_unlock(pfc_iostream_t stream)
{
	IOSTREAM_LOCK(stream);
	stream->s_busy = PFC_FALSE;
	IOSTREAM_SIGNAL(stream);
	IOSTREAM_UNLOCK(stream);
}

/*
 * static size_t
 * iostream_cmcred_size(cmsg_ctx_t *ctx)
 *	Return the size of control message which sends or receives credentials
 *	of process.
 */
static size_t
iostream_cmcred_size(cmsg_ctx_t *ctx)
{
	return sizeof(pfc_ucred_t);
}

/*
 * static void
 * iostream_cmcred_setup(cmsg_ctx_t *ctx)
 *	Set up control message to send process credentials.
 */
static void
iostream_cmcred_setup(cmsg_ctx_t *ctx)
{
	cmsg_credctx_t	*crctx = CMSG_CTX_CREDCTX(ctx);
	struct msghdr	*msg = &ctx->cm_header;
	struct cmsghdr	*cmsg;
	pfc_ucred_t	*cdp;

	cmsg = CMSG_FIRSTHDR(msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_CREDENTIALS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(pfc_ucred_t));
	msg->msg_controllen = cmsg->cmsg_len;

	/* Copy the credentials. */
	cdp = (pfc_ucred_t *)CMSG_DATA(cmsg);
	*cdp = *(crctx->cmc_cred);
}

/*
 * static void
 * iostream_cmcred_fetch(cmsg_ctx_t *ctx)
 *	Fetch the credentials in SCM_CREDENTIALS control message.
 */
static void
iostream_cmcred_fetch(cmsg_ctx_t *ctx)
{
	struct msghdr	*msg = &ctx->cm_header;

	if (msg->msg_controllen != 0) {
		cmsg_credctx_t	*crctx = CMSG_CTX_CREDCTX(ctx);

		/* Fetch received credentials. */
		iostream_fetch_ucred(msg, crctx->cmc_cred);

		/* No more control message needs to be received. */
		msg->msg_control = NULL;
		msg->msg_controllen = 0;
	}
}

/*
 * static int
 * iostream_cmcred_fetch_prepare(cmsg_ctx_t *ctx)
 *	Prepare to fetch SCM_CREDENTIALS control message.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
iostream_cmcred_fetch_prepare(cmsg_ctx_t *ctx)
{
	cmsg_credctx_t	*crctx = CMSG_CTX_CREDCTX(ctx);
	pfc_iostream_t	stream = ctx->cm_stream;
	int		sock = stream->s_fd, ret;
	socklen_t	len = sizeof(crctx->cmc_passcred);

	/* Obtain current value of SO_PASSCRED option. */
	ret = getsockopt(sock, SOL_SOCKET, SO_PASSCRED, &crctx->cmc_passcred,
			 &len);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		return errno;
	}

	if (!crctx->cmc_passcred) {
		int	value = 1;

		/* Set SO_PASSCRED option. */
		ret = setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &value,
				 sizeof(value));
		if (PFC_EXPECT_FALSE(ret != 0)) {
			return errno;
		}
	}

	return 0;
}

/*
 * static int
 * iostream_cmcred_fetch_cleanup(cmsg_ctx_t *ctx)
 *	Clean up changes made by iostream_cmcred_fetch_prepare().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
iostream_cmcred_fetch_cleanup(cmsg_ctx_t *ctx)
{
	cmsg_credctx_t	*crctx = CMSG_CTX_CREDCTX(ctx);

	if (!crctx->cmc_passcred) {
		pfc_iostream_t	stream = ctx->cm_stream;
		int		sock = stream->s_fd, value = 0, ret;

		/* Turn the SO_PASSCRED option off. */
		ret = setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &value,
				 sizeof(value));
		if (PFC_EXPECT_FALSE(ret != 0)) {
			return errno;
		}
	}

	return 0;
}

/*
 * static int
 * iostream_getsockerr(int sock)
 *	Get and clear the socket error pending on the specified socket.
 *
 * Calling/Exit State:
 *	Upon successful completion, an error number which indicates the pending
 *	socket error is returned.
 *	Otherwise EIO is returned.
 */
static int
iostream_getsockerr(int sock)
{
	int		soerr = 0, ret;
	socklen_t	optlen = sizeof(soerr);

	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, &soerr, &optlen);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		soerr = errno;
	}

	if (soerr == 0) {
		soerr = EIO;
	}

	return soerr;
}
