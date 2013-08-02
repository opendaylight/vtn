/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.c - Miscellaneous utilities.
 */

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pfc/atomic.h>
#include <pfc/ctype.h>
#include <pfc/util.h>
#include <iostream_impl.h>
#include "ipc_impl.h"
#include "ipc_struct_impl.h"

/*
 * IPC name specification.
 */
typedef struct {
	const char	*ns_name;		/* name of IPC name type */
	const char	*ns_validchar;		/* valid characters */
	uint32_t	ns_maxlen;		/* maximum length */
} ipc_nmspec_t;

typedef const ipc_nmspec_t	ipc_cnmspec_t;

#define	IPC_NMSPEC_DECL(name, valid, maxlen)	\
	{					\
		.ns_name	= name,		\
		.ns_validchar	= valid,	\
		.ns_maxlen	= maxlen,	\
	}

static ipc_nmspec_t	ipc_name_spec[] = {
	/* IPC channel name */
	IPC_NMSPEC_DECL("channel", "_", IPC_CHANNEL_NAMELEN_MAX),

	/* IPC service name */
	IPC_NMSPEC_DECL("service", "_.", IPC_SERVICE_NAMELEN_MAX),

	/* IPC host set name */
	IPC_NMSPEC_DECL("hostset", "_", IPC_HOSTSET_NAMELEN_MAX),
};

/*
 * Flag which indicates whether internal log should be recorded by PFC logging
 * architecture.
 * This flag is updated by atomic operation.
 */
volatile uint32_t	ipc_log_enabled PFC_ATTR_HIDDEN = 0;

/*
 * fork(2) handler for libpfc_ipcclnt.
 */
static const ipc_fork_t	*ipc_client_fork;

/*
 * Internal prototypes.
 */
static void	ipc_fork_prepare(void);
static void	ipc_fork_parent(void);
static void	ipc_fork_child(void);
static void	ipc_sock_setbufsize(int sock, int option, uint32_t size);

/*
 * static void PFC_FATTR_INIT
 * ipc_libinit(void)
 *	Constructor of libpfc_ipc library.
 */
static void PFC_FATTR_INIT
ipc_libinit(void)
{
	int	err;

	/* Register fork(2) handlers. */
	err = pthread_atfork(ipc_fork_prepare, ipc_fork_parent,
			     ipc_fork_child);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fprintf(stderr, "libpfc_ipc: Failed to register prefork "
			"handler: %s\n", strerror(err));
		abort();
		/* NOTREACHED */
	}
}

/*
 * static void PFC_FATTR_FINI
 * ipc_libfini(void)
 *	Destructor of libpfc_ipc library.
 */
static void PFC_FATTR_FINI
ipc_libfini(void)
{
	pfc_ipc_struct_fini();
}

/*
 * void
 * pfc_ipc_client_init(const ipc_fork_t *handler)
 *	Initialization routine for IPC client library.
 *
 *	This function must be called only from libpfc_ipcclnt's constructor.
 */
void
pfc_ipc_client_init(const ipc_fork_t *handler)
{
	if (PFC_EXPECT_FALSE(ipc_client_fork != NULL)) {
		fprintf(stderr, "libpfc_ipc: IPC client fork handler is "
			"already registered.");
		abort();
		/* NOTREACHED */
	}

	ipc_client_fork = handler;
}

/*
 * void
 * pfc_ipc_enable_log(pfc_bool_t enable)
 *	Enable or disable internal logging.
 *
 *	If PFC_TRUE is specified to `enable', internal logging by PFC log
 *	architecture is enabled. It is up to the caller for initialization of
 *	PFC log.
 *
 *	Internal logging is disabled by default.
 *
 * Remarks:
 *	Note that logging flags is implemented by counter.
 *	This function increments the counter if `enable' is PFC_TRUE, and
 *	decrements if `enable' is PFC_FALSE. The logging is enabled if the
 *	counter is not zero.
 */
void
pfc_ipc_enable_log(pfc_bool_t enable)
{
	uint32_t	*ptr = (uint32_t *)&ipc_log_enabled;
	uint32_t	value, result;

	if (enable) {
		pfc_atomic_inc_uint32(ptr);

		/* Enable I/O stream internal logging. */
		__pfc_iostream_log_enable();

		return;
	}

	do {
		uint32_t	newvalue;

		value = *ptr;
		if (PFC_EXPECT_FALSE(value == 0)) {
			/* The logging is already disabled. */
			break;
		}
		else {
			newvalue = value - 1;
		}

		result = pfc_atomic_cas_uint32(ptr, newvalue, value);
	} while (result != value);
}

/*
 * int
 * pfc_ipc_sess_init(ipc_sess_t *PFC_RESTRICT sess,
 *		     const pfc_hostaddr_t *PFC_RESTRICT haddr)
 *	Initialize IPC session.
 *
 *	`haddr' must be a pointer to pfc_hostaddr_t which represents peer
 *	address.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_sess_init(ipc_sess_t *PFC_RESTRICT sess,
		  const pfc_hostaddr_t *PFC_RESTRICT haddr)
{
	int	family = pfc_hostaddr_gettype(haddr);

	if (PFC_EXPECT_FALSE(family != AF_UNIX)) {
		return EAFNOSUPPORT;
	}

	sess->iss_stream = NULL;

	/* Copy host address. */
	sess->iss_addr = *haddr;

	/* Initialize session flags. */
	sess->iss_flags = 0;
	sess->iss_version = 0;

	return 0;
}

/*
 * int
 * pfc_ipc_iostream_create(ipc_sess_t *PFC_RESTRICT sess, int sock,
 *			   int canceller, ipc_coption_t *PFC_RESTRICT opts)
 *	Create an I/O stream for IPC session.
 *	An I/O stream which treats additional data must be created by this
 *	function.
 *
 *	`sock' is a socket descriptor associated with the IPC session.
 *	`canceller' is an I/O cancellation FD.
 *
 *	If `opts' is not NULL, it must point to IPC global option which
 *	contains socket options to be applied. If `opts' is NULL, any socket
 *	option is not changed.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to I/O stream is
 *	stored to sess->iss_stream, and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- `sock' is always closed on error return.
 *
 *	- `sess' must be initialized by pfc_ipc_sess_init() in advance.
 */
int
pfc_ipc_iostream_create(ipc_sess_t *PFC_RESTRICT sess, int sock, int canceller,
			ipc_coption_t *PFC_RESTRICT opts)
{
	pfc_iostream_t	stream;
	int		err;

	PFC_ASSERT(sess != NULL);

	if (opts != NULL) {
		ipc_sock_setbufsize(sock, SO_SNDBUF, opts->iopt_sndbuf_size);
		ipc_sock_setbufsize(sock, SO_RCVBUF, opts->iopt_rcvbuf_size);
	}

	/* Create a new I/O stream. */
	err = pfc_iostream_create(&stream, sock, IPC_SESS_BUFSIZE_IN,
				  IPC_SESS_BUFSIZE_OUT, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Unable to create session stream: sock=%d: %s",
			      sock, strerror(err));
		if (err != EBADF) {
			PFC_IPC_CLOSE(sock);
		}

		return err;
	}

	/* Set cancellation FD to the stream. */
	err = pfc_iostream_setcanceller(stream, canceller);
	if (PFC_EXPECT_FALSE(err != 0)) {
		int	e;

		IPC_LOG_ERROR("Unable to set canceller to the stream: "
			      "stream=%p, sock=%d, fd=%d: %s",
			      stream, sock, canceller, strerror(err));

		e = pfc_iostream_destroy(stream);
		if (PFC_EXPECT_FALSE(e != 0)) {
			IPC_LOG_ERROR("Failed to destroy stream: stream=%p, "
				      "sock=%d: %s", stream, sock,
				      strerror(e));
		}

		return err;
	}

	sess->iss_stream = stream;

	return 0;
}

/*
 * int
 * pfc_ipc_read(pfc_iostream_t PFC_RESTRICT stream, pfc_ptr_t PFC_RESTRICT buf,
 *		uint32_t size, ctimespec_t *PFC_RESTRICT abstime)
 *	Read data from the given I/O stream, and store data to the buffer
 *	pointed by `buf'.
 *	If `abstime' is not NULL, ETIMEDOUT is returned if the absolute time
 *	specified by `abstime' passes before completion of sending data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_read(pfc_iostream_t PFC_RESTRICT stream, pfc_ptr_t PFC_RESTRICT buf,
	     uint32_t size, ctimespec_t *PFC_RESTRICT abstime)
{
	size_t	sz = size;
	int	err;

	err = pfc_iostream_read_abs(stream, buf, &sz, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ECANCELED) {
			IPC_LOG_ERROR("Read error on IPC stream: %s",
				      strerror(err));
		}

		return err;
	}

	if (PFC_EXPECT_FALSE(sz != size)) {
		/* EOF has been detected. */
		IPC_LOG_ERROR("Connection reset by peer.");

		return ECONNRESET;
	}

	return 0;
}

/*
 * int
 * pfc_ipc_write(pfc_iostream_t PFC_RESTRICT stream,
 *		 pfc_cptr_t PFC_RESTRICT buf, uint32_t size,
 *		 pfc_bool_t do_flush, ctimespec_t *PFC_RESTRICT abstime)
 *	Write an arbitrary data, specified by `buf' and `size', to the peer
 *	connected to the specified I/O stream.
 *
 *	Output buffer of I/O stream is flushed only if PFC_TRUE is specified
 *	to `do_flush'.
 *
 *	If `abstime' is not NULL, ETIMEDOUT is returned if the absolute time
 *	specified by `abstime' passes before completion of sending data.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_write(pfc_iostream_t PFC_RESTRICT stream, pfc_cptr_t PFC_RESTRICT buf,
	      uint32_t size, pfc_bool_t do_flush,
	      ctimespec_t *PFC_RESTRICT abstime)
{
	size_t	sz = size;
	int	err;

	err = pfc_iostream_write_abs(stream, buf, &sz, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err != ECANCELED) {
			IPC_LOG_ERROR("Write error on IPC stream: %s",
				      strerror(err));
		}

		return err;
	}

	if (PFC_EXPECT_FALSE(sz != size)) {
		/* This should never happen. */
		IPC_LOG_ERROR("Unexpected size of written data: %"
			      PFC_PFMT_SIZE_T ", %u", sz, size);

		return EIO;
	}

	if (do_flush) {
		/* Flush output buffer. */
		err = pfc_iostream_flush_abs(stream, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			if (err != ECANCELED) {
				IPC_LOG_ERROR("Failed to flush IPC stream: %s",
					      strerror(err));
			}

			return err;
		}
	}

	return 0;
}

/*
 * int
 * pfc_ipc_checkname(ipc_nmtype_t type, const char *name)
 *	Verify name of IPC instance.
 *
 *	`type' determines type of IPC name.
 *	  - IPC_NAME_CHANNEL must be specified if `name' is an IPC channel
 *	    name.
 *	  - IPC_NAME_SERVICE must be specified if `name' is an IPC service
 *	    name.
 *	  - IPC_NAME_HOSTSET must be specified if `name' is an IPC host address
 *	    set name.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	NULL must not be specified to `name'.
 */
int
pfc_ipc_checkname(ipc_nmtype_t type, const char *name)
{
	ipc_cnmspec_t	*spec = &ipc_name_spec[(uint32_t)type];
	const char	*p, *validchar;
	uint8_t		c;
	uint32_t	maxlen, len;

	PFC_ASSERT(spec < PFC_ARRAY_LIMIT(ipc_name_spec));
	PFC_ASSERT(name != NULL);

	c = *name;
	if (PFC_EXPECT_FALSE(c == '\0')) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(!pfc_isalpha_u(c))) {
		return EINVAL;
	}

	maxlen = spec->ns_maxlen;
	validchar = spec->ns_validchar;
	for (p = name + 1, len = 1; (c = *p) != '\0' && len <= maxlen;
	     p++, len++) {
		if (PFC_EXPECT_FALSE(!pfc_isalnum_u(c) &&
				     strchr(validchar, c) == NULL)) {
			return EINVAL;
		}
	}

	if (PFC_EXPECT_FALSE(len > maxlen)) {
		return EINVAL;
	}

	return 0;
}

/*
 * int
 * pfc_ipc_checkshutfd(int fd)
 *	Determine whether the given file descriptor can be used as shutdown
 *	notification file descriptor or not.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	On successful return, both O_NONBLOCK flag and close-on-exec flag
 *	are set to the given file descriptor.
 */
int
pfc_ipc_checkshutfd(int fd)
{
	int	fl, err;

	/* Ensure that the given shutdown FD is valid. */
	fl = fcntl(fd, F_GETFL);
	if (PFC_EXPECT_FALSE(fl == -1)) {
		err = errno;
		IPC_LOG_ERROR("Invalid shutdown FD(%d): %s",
			      fd, strerror(err));

		return EINVAL;
	}

	if (PFC_EXPECT_FALSE((fl & O_ACCMODE) == O_WRONLY)) {
		IPC_LOG_ERROR("Shutdown FD(%d) is not readable.", fd);

		return EACCES;
	}

	if ((fl & O_NONBLOCK) == 0) {
		/* Set non-blocking and flag to the given file descriptor. */
		if (PFC_EXPECT_FALSE(fcntl(fd, F_SETFL, fl | O_NONBLOCK)
				     == -1)) {
			err = errno;
			IPC_LOG_ERROR("Unable to set O_NONBLOCK flag: %s",
				      strerror(err));
			goto error;
		}
	}

	err = pfc_set_cloexec(fd, PFC_TRUE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("Unable to set close-on-exec flag: %s",
			      strerror(err));
		goto error;
	}

	return 0;

error:
	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EIO;
	}

	return err;
}

/*
 * int
 * pfc_ipc_thread_create(void *(*func)(void *), void *arg)
 *	Create a new temporary thread.
 *	This function creates a detached POSIX thread with default attributes.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_thread_create(void *(*func)(void *), void *arg)
{
	pthread_attr_t	attr;
	pthread_t	t;
	int		err;

	PFC_ASSERT_INT(pthread_attr_init(&attr), 0);
	PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&attr, PTHREAD_CREATE_DETACHED), 0);

	err = pthread_create(&t, &attr, func, arg);
	PFC_ASSERT_INT(pthread_attr_destroy(&attr), 0);

	return err;
}

/*
 * static void
 * ipc_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
static void
ipc_fork_prepare(void)
{
	const ipc_fork_t	*handler = ipc_client_fork;

	/*
	 * IPC client handler must be called before libpfc_ipc's fork(2)
	 * prepare handler.
	 */
	if (handler != NULL) {
		handler->if_prepare();
	}

	pfc_ipc_struct_fork_prepare();
}

/*
 * static void
 * ipc_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
static void
ipc_fork_parent(void)
{
	const ipc_fork_t	*handler = ipc_client_fork;

	/*
	 * libpfc_ipc's fork(2) parent handler must be called before IPC
	 * client handler.
	 */
	pfc_ipc_struct_fork_parent();

	if (handler != NULL) {
		handler->if_parent();
	}
}

/*
 * static void
 * ipc_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
static void
ipc_fork_child(void)
{
	const ipc_fork_t	*handler = ipc_client_fork;

	/*
	 * libpfc_ipc's fork(2) child handler must be called before IPC
	 * client handler.
	 */
	pfc_ipc_struct_fork_child();

	if (handler != NULL) {
		handler->if_child();
	}
}

/*
 * static void
 * ipc_sock_setbufsize(int sock, int option, uint32_t size)
 *	Set socket buffer size.
 *
 *	`option' must be SO_SNDBUF for send buffer, or SO_RCVBUF for receive
 *	buffer.
 *
 *	`size' is a new buffer size of the socket buffer. Zero means that
 *	system default value should be used.
 */
static void
ipc_sock_setbufsize(int sock, int option, uint32_t size)
{
	int	ret;

	if (size == 0) {
		return;
	}

	ret = setsockopt(sock, SOL_SOCKET, option, &size, sizeof(size));
	if (PFC_EXPECT_FALSE(ret != 0)) {
		const char	*type = (option == SO_SNDBUF)
			? "send" : "receive";
		IPC_LOG_WARN("Unable to change socket %s buffer size to %u: "
			     "%s", type, size, strerror(errno));
		/* FALLTHROUGH */
	}
}
