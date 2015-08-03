/*
 * Copyright (c) 2010-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * client.c - PFC daemon protocol client.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pfc/path.h>
#include <pfc/debug.h>
#include <pfc/socket.h>
#include <pfc/util.h>
#include <ctrl_proto.h>
#include "ctrl_client.h"

/*
 * Size of UNIX domain socket path.
 */
#define	UNIX_SOCKET_PATH_SIZE	(sizeof(((struct sockaddr_un *)0)->sun_path))

/*
 * Buffer size for control protocol session.
 */
#define	CLIENT_BUFSIZE_IN	256U
#define	CLIENT_BUFSIZE_OUT	64U

/*
 * Control socket path.
 */
static char	ctrl_socket_path[UNIX_SOCKET_PATH_SIZE + 1];

/*
 * Length of control socket path.
 */
static uint32_t	ctrl_socket_pathlen;

/*
 * Error handler.
 */
static pfc_ctrl_err_t	ctrl_errfunc;

/*
 * Debug handler.
 */
static pfc_ctrl_dbg_t	ctrl_dbgfunc;

/*
 * Internal prototypes.
 */
static int	client_open(int *PFC_RESTRICT sockp,
			    const pfc_timespec_t *PFC_RESTRICT timeout,
			    const pfc_iowait_t *PFC_RESTRICT iowait,
			    int canceller);
static void	client_error(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2);
static void	client_error_default(const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(1, 0);
static void	client_error_with_errno(const char *message, int err);
static void	client_error_handler(int err, const char *fmt, va_list ap);
static int	client_error_response(cproto_sess_t *sess);

/*
 * static inline void
 * client_error_v(const char *fmt, va_list ap)
 *	Print error message.
 */
static inline void  PFC_FATTR_PRINTFLIKE(1, 0)
client_error_v(const char *fmt, va_list ap)
{
	(*ctrl_errfunc)(fmt, ap);
}

/*
 * static inline void PFC_FATTR_PRINTFLIKE(2, 0)
 * client_debug_v(int level, const char *fmt, va_list ap)
 *	Call debug message handler with specifying message in vprintf(3)
 *	format.
 */
static inline void PFC_FATTR_PRINTFLIKE(2, 0)
client_debug_v(int level, const char *fmt, va_list ap)
{
	pfc_ctrl_dbg_t	dbgfunc = ctrl_dbgfunc;

	if (dbgfunc != NULL) {
		(*dbgfunc)(level, fmt, ap);
	}
}

/*
 * static inline void
 * client_debug(int level, const char *fmt, ...)
 *	Call debug message handler.
 */
static inline void  PFC_FATTR_PRINTFLIKE(2, 3)
client_debug(int level, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	client_debug_v(level, fmt, ap);
	va_end(ap);
}

/*
 * int
 * pfc_ctrl_client_init(const char *PFC_RESTRICT workdir,
 *			pfc_ctrl_err_t errfunc)
 *	Initialize control protocol client library.
 *
 *	`workdir' must be the working directory path for the PFC daemon.
 *
 *	`errfunc' is an error message handler which will be called on error
 *	with specifying error message in vprintf(3) format. If NULL is
 *	specified, any error message will be discarded. Note that EINTR error
 *	is never logged even if a non-NULL function pointer is specified to
 *	`errfunc'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ctrl_client_init(const char *PFC_RESTRICT workdir, pfc_ctrl_err_t errfunc)
{
	int	len;

	/* At first, initialize error message handler. */
	ctrl_errfunc = (errfunc) ? errfunc : client_error_default;

	/* Initialize UNIX domain socket path. */
	len = snprintf(ctrl_socket_path, sizeof(ctrl_socket_path), "%s/%s/%s",
		       workdir, PFC_PFCD_CHANNEL_NAME, PFC_PFCD_CTRL_NAME);
	if (PFC_EXPECT_FALSE((size_t)len >= sizeof(ctrl_socket_path))) {
		client_error("Too long control socket path: %s/%s/%s",
			     workdir, PFC_PFCD_CHANNEL_NAME,
			     PFC_PFCD_CTRL_NAME);

		return ENAMETOOLONG;
	}

	ctrl_socket_pathlen = (uint32_t)len;

	return 0;
}

/*
 * void
 * pfc_ctrl_debug(pfc_ctrl_dbg_t dbgfunc)
 *	Install debug handler.
 *	If NULL is specified, any debug message will be discarded.
 *
 * Remarks:
 *	This function is not thread safe.
 */
void
pfc_ctrl_debug(pfc_ctrl_dbg_t dbgfunc)
{
	ctrl_dbgfunc = dbgfunc;
}

/*
 * int
 * pfc_ctrl_client_create_c(cproto_sess_t *PFC_RESTRICT sess, int canceller,
 *			    const pfc_timespec_t *PFC_RESTRICT timeout,
 *			    const pfc_iowait_t *PFC_RESTRICT iowait)
 *	Create control protocol client session.
 *
 *	If `canceller' is not -1, it is set as canceller FD to the session
 *	stream. Control protocol will return ECANCELED error if a read event
 *	is detected on the canceller FD.
 *
 *	If `timeout' is not NULL, it is used as session timeout.
 *	If NULL, the default value is used, which is specified by
 *	options.ctrl_timeout in pfcd.conf.
 *
 *	`iowait' determines behavior of I/O wait. The specified value is passed
 *	to pfc_iostream_create().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ctrl_client_create_c(cproto_sess_t *PFC_RESTRICT sess, int canceller,
			 const pfc_timespec_t *PFC_RESTRICT timeout,
			 const pfc_iowait_t *PFC_RESTRICT iowait)
{
	pfc_iostream_t	stream;
	int		sock, err;

	/* Establish connection to the daemon. */
	err = client_open(&sock, timeout, iowait, canceller);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Create I/O stream on the socket. */
	err = pfc_iostream_create(&stream, sock, CLIENT_BUFSIZE_IN,
				  CLIENT_BUFSIZE_OUT, iowait);
	if (PFC_EXPECT_FALSE(err != 0)) {
		close(sock);
		client_error("Failed to create session stream: %s",
			     strerror(err));

		return err;
	}

	if (canceller != -1) {
		/* Set canceller. */
		err = pfc_iostream_setcanceller(stream, canceller);
		if (PFC_EXPECT_FALSE(err != 0)) {
			client_error("Failed to set canceller FD: %s",
				     strerror(err));
			goto error;
		}
	}

	/* Initialize session context. */
	cproto_sess_init(sess, stream, client_error_handler, timeout);

	/* Send a handshake message to the daemon. */
	client_debug(2, "Sending handshake.");
	err = cproto_handshake_write(sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Read a response from the daemon. */
	client_debug(2, "Receiving handshake.");
	err = cproto_handshake_read(sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	client_debug(2, "Start control protocol session.");

	return 0;

error:
	PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);

	return err;
}

/*
 * void
 * pfc_ctrl_client_destroy(cproto_sess_t *sess)
 *	Destroy control protocol session.
 */
void
pfc_ctrl_client_destroy(cproto_sess_t *sess)
{
	if (sess->cps_stream != NULL) {
		PFC_ASSERT_INT(pfc_iostream_destroy(sess->cps_stream), 0);
		sess->cps_stream = NULL;
	}
}

/*
 * int
 * pfc_ctrl_client_execute(cproto_sess_t *PFC_RESTRICT sess, ctrl_cmdtype_t cmd,
 *			   const pfc_ctrl_ops_t *PFC_RESTRICT ops,
 *			   pfc_ptr_t arg)
 *	Execute subcommand on the specified control protocol session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	If an error response is received, 1 is returned.
 *	Otherwise negative error number which indicates the cause of error is
 *	returned.
 */
int
pfc_ctrl_client_execute(cproto_sess_t *PFC_RESTRICT sess, ctrl_cmdtype_t cmd,
			const pfc_ctrl_ops_t *PFC_RESTRICT ops, pfc_ptr_t arg)
{
	ctrl_cmdresp_t	resp;
	int		err;

	/*
	 * Send control protocol command associated with the specified
	 * subcommand.
	 */
	client_debug(2, "Sending command: %u", cmd);
	err = cproto_cmd_write(sess, cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return -err;
	}

	/* Send optional arguments if needed. */
	if (ops != NULL && ops->send != NULL) {
		err = ops->send(sess, arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return -err;
		}
	}

	/* Flush the session stream. */
	err = pfc_iostream_flush(sess->cps_stream, &sess->cps_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		client_error_with_errno("Failed to flush the session stream",
					err);

		return -err;
	}

	/* Receive response code from the daemon. */
	err = cproto_resp_read(sess, &resp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return -err;
	}

	client_debug(2, "Command response: %u", resp);
	if (PFC_EXPECT_FALSE(resp == CTRL_RESP_FAIL)) {
		/* An error message must follow. */
		return client_error_response(sess);
	}
	else if (PFC_EXPECT_FALSE(resp != CTRL_RESP_OK)) {
		client_error("Unexpected response code: %u", resp);

		return 1;
	}

	if (ops != NULL && ops->receive != NULL) {
		/* Read additional response. */
		err = ops->receive(sess, arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return -err;
		}
	}

	return 0;
}

/*
 * int
 * pfc_ctrl_check_permission(void)
 *	Determine whether the calling process can access to the PFC daemon
 *	control port.
 *
 * Calling/Exit State:
 *	Zero is returned if the calling process can acess to the control port.
 *
 *	ENOENT is returned if the daemon is not running.
 *	EACCES is returned if the calling process can not access to the
 *	control port.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ctrl_check_permission(void)
{
	struct stat	sbuf;
	int	err;

	PFC_ASSERT(ctrl_socket_pathlen != 0);

	/*
	 * Use stat(2) in place of access(2) to check access permission because
	 * access(2) checks access permission using real uid/gid, not effective
	 * uid/gid. We can use stat(2) instead because the same permission
	 * bits are configured to the control socket file and its parent
	 * directory.
	 */
	if (PFC_EXPECT_TRUE(stat(ctrl_socket_path, &sbuf) == 0)) {
		return 0;
	}

	err = errno;
	if (PFC_EXPECT_FALSE(err == 0)) {
		/* This should never happen. */
		err = EIO;
	}

	return err;
}

/*
 * static int
 * client_open(int *PFC_RESTRICT sockp,
 *	       const pfc_timespec_t *PFC_RESTRICT timeout,
 *	       const pfc_iowait_t *PFC_RESTRICT iowait, int canceller)
 *	Create socket, and establish connection to the daemon.
 */
static int
client_open(int *PFC_RESTRICT sockp, const pfc_timespec_t *PFC_RESTRICT timeout,
	    const pfc_iowait_t *PFC_RESTRICT iowait, int canceller)
{
	struct sockaddr_un	un;
	pfc_timespec_t	to;
	int		sock, err;

	*sockp = -1;

	/* Create a socket. */
	sock = pfc_sock_open(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		err = errno;
		client_error("Failed to create socket: %s", strerror(err));

		return err;
	}

	un.sun_family = AF_UNIX;
	PFC_ASSERT(strlen(ctrl_socket_path) == ctrl_socket_pathlen);
	PFC_ASSERT(ctrl_socket_pathlen <= sizeof(un.sun_path));
	memcpy(un.sun_path, ctrl_socket_path, ctrl_socket_pathlen);
	if (PFC_EXPECT_TRUE(ctrl_socket_pathlen < sizeof(un.sun_path))) {
		un.sun_path[ctrl_socket_pathlen] = '\0';
	}

	if (timeout == NULL) {
		to.tv_sec = cproto_timeout_value();
		to.tv_nsec = 0;
		timeout = (const pfc_timespec_t *)&to;
	}

	/* Establish connection to the daemon. */
	client_debug(2, "Trying to connect to the daemon...");
	client_debug(2, "socket: %s", un.sun_path);
	err = pfc_sock_connect_c(sock, (const struct sockaddr *)&un,
				 PFC_SOCK_UNADDR_SIZE(ctrl_socket_pathlen),
				 canceller, timeout, iowait);
	if (PFC_EXPECT_FALSE(err != 0)) {
		client_error_with_errno("Failed to connect to the daemon", err);
		goto error;
	}
	client_debug(2, "Connected to the daemon.");
	*sockp = sock;

	return 0;

error:
	close(sock);

	return err;
}

/*
 * static void
 * client_error(const char *fmt, ...)
 *	Print error message specified in printf(3) format.
 */
static void
client_error(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	client_error_v(fmt, ap);
	va_end(ap);
}

/*
 * static void
 * client_error_default(const char *fmt PFC_ATTR_UNUSED,
 *			va_list ap PFC_ATTR_UNUSED)
 *	Default error message handler.
 *	This function is used if no error handler is specified to
 *	pfc_ctrl_client_init().
 */
static void
client_error_default(const char *fmt PFC_ATTR_UNUSED,
		     va_list ap PFC_ATTR_UNUSED)
{
	/* Discard any error message. */
}

/*
 * static void
 * client_error_with_errno(const char *message, int err)
 *	Log error message with specifying errno.
 */
static void
client_error_with_errno(const char *message, int err)
{
	if (err != EINTR) {
		client_error("%s: %s", message, strerror(err));
	}
	else {
		client_debug(2, "%s: Interrupted.", message);
	}
}

/*
 * static void
 * client_error_handler(int err, const char *fmt, va_list ap)
 *	Print message about error on the control session, except for
 *	EINTR error.
 */
static void
client_error_handler(int err, const char *fmt, va_list ap)
{
	if (err != EINTR) {
		client_error_v(fmt, ap);
	}
	else {
		client_debug_v(2, fmt, ap);
	}
}

/*
 * static int
 * client_error_response(cproto_sess_t *sess)
 *	Read error message from the daemon.
 *
 * Calling/Exit State:
 *	1 is returned on success.
 *	Otherwise negative error number which indicates the cause of error is
 *	returned.
 */
static int
client_error_response(cproto_sess_t *sess)
{
	cproto_data_t	data;
	int		err;

	err = cproto_data_read(sess, &data);
	if (PFC_EXPECT_FALSE(err != 0)) {
		client_error_with_errno("Failed to read error message", err);

		return -err;
	}

	if (PFC_EXPECT_TRUE(data.cpd_type == CTRL_PDTYPE_TEXT)) {
		client_error("Command failed: %s", cproto_data_text(&data));
	}
	else {
		client_error("Unexpected type of error message: %u",
			     data.cpd_type);
	}
	cproto_data_free(&data);

	return 1;
}
