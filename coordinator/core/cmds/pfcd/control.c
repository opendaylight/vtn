/*
 * Copyright (c) 2010-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * control.c - Daemon controller via UNIX domain socket.
 *
 * The main thread of the PFC daemon process becomes the control thread.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pfc/exstatus.h>
#include <pfc/socket.h>
#include <pfc/atomic.h>
#include <pfc/thread.h>
#include <module_impl.h>
#include <modevent_impl.h>
#include <iostream_impl.h>
#include "pfcd.h"

#ifndef	PFC_HAVE_PPOLL
#error	Current implementation requires ppoll(2).
#endif	/* !PFC_HAVE_PPOLL */

/*
 * Backlog for control port.
 */
#define	CTRL_BACKLOG		4

/*
 * Default access permission bits for control port.
 */
#define	CTRL_PERM		(S_IRWXU)

/*
 * Maximum size of control socket path.
 */
#define	CTRL_PATH_MAXLEN	(sizeof(((struct sockaddr_un *)0)->sun_path))

/*
 * File descriptor of control socket.
 */
static int	ctrl_socket;

/*
 * Path to control socket.
 */
static char	ctrl_socket_path[CTRL_PATH_MAXLEN + 1];

/*
 * Ticket to synchronize control socket cleanup.
 */
static uint8_t	ctrl_socket_bound;

/*
 * Buffer size for control protocol session.
 */
#define	CTRL_BUFSIZE_IN		64U
#define	CTRL_BUFSIZE_OUT	256U

/*
 * Record error log, but ignore EINTR error.
 */
#define	CTRL_LOG_ERROR(err, logargs)			\
	do {						\
		if ((err) != EINTR) {			\
			pfc_log_error logargs;		\
		}					\
	} while (0)

/*
 * Module specific event.
 */
struct ctrl_event;
typedef struct ctrl_event	ctrl_event_t;

struct ctrl_event {
	const char	*ce_name;		/* module name */
	uint32_t	ce_type;		/* event type */
	ctrl_event_t	*ce_next;
};

/*
 * Prototype of module dependency iterator function.
 */
typedef int	(*moddep_iter_t)(pmodule_t *mod, pmod_iter_t iter,
				 pfc_ptr_t arg);

/*
 * Internal prototypes.
 */
static void	ctrl_setup_directory(const char *path, mode_t mode);

static pfc_bool_t	ctrl_intr_callback(pfc_ptr_t arg);

static int	ctrl_session_init(cproto_sess_t *sess, int fd);
static void	ctrl_session_error(int err, const char *fmt, va_list ap);

static int	ctrl_execute(cproto_sess_t *sess);

static int	ctrl_terminate_response(cproto_sess_t *sess,
					pfc_bool_t need_null);
static int	ctrl_error(cproto_sess_t *PFC_RESTRICT sess, int retval,
			   const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(3, 4);
static int	ctrl_verror(cproto_sess_t *PFC_RESTRICT sess, int retval,
			    const char *PFC_RESTRICT fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(3, 0);

static int	ctrl_command_loglevel(cproto_sess_t *sess);
static int	ctrl_command_modlist(cproto_sess_t *sess);
static int	ctrl_command_event(cproto_sess_t *sess);

static int	loglevel_set(cproto_sess_t *PFC_RESTRICT sess,
			     const char *PFC_RESTRICT modname,
			     pfc_log_level_t level);
static int	loglevel_modlist(cproto_sess_t *sess);

static int	modlist_iterator(pmodule_t *PFC_RESTRICT mod,
				 pfc_ptr_t PFC_RESTRICT arg);

static void	event_list_free(ctrl_event_t *events);

/*
 * Control command operations.
 */
typedef struct {
	/*
	 * Execute command.
	 * Zero is returned on success.
	 * Error number is returned on failure.
	 */
	int	(*execute)(cproto_sess_t *sess);
} ctrl_cmdops_t;

#define	CTRL_CMDOPS_IDX(cmd)	((cmd) - 2)

#define	CTRL_CMDOPS_DECL(name)	{ ctrl_command_##name }

static const ctrl_cmdops_t	command_ops[] = {
	CTRL_CMDOPS_DECL(loglevel),		/* LOGLEVEL */
	CTRL_CMDOPS_DECL(modlist),		/* MODLIST */
	CTRL_CMDOPS_DECL(event),		/* EVENT */
};

/*
 * void
 * ctrl_init(void)
 *	Initialize PFC daemon control port.
 */
void
ctrl_init(void)
{
	size_t		len;
	int		sock;
	mode_t		cperm;
	struct sockaddr_un	un;

	/* Determine access permission of control port. */
	cperm = (mode_t)pfc_conf_get_uint32(pfcd_options, "ctrl_perm",
					    CTRL_PERM);
	pfc_log_verbose("ctrl_perm = 0%o", cperm);

	len = (size_t)snprintf(ctrl_socket_path, sizeof(ctrl_socket_path),
			       "%s/%s", pfcd_work_dir, PFC_PFCD_CHANNEL_NAME);
	if (PFC_EXPECT_FALSE(len >= sizeof(ctrl_socket_path))) {
		pfc_log_fatal("Too long channel directory path: %s/%s",
			      pfcd_work_dir, PFC_PFCD_CHANNEL_NAME);
		/* NOTREACHED */
	}

	/* Set up channel directory. */
	ctrl_setup_directory(ctrl_socket_path, cperm);

	len = (size_t)snprintf(ctrl_socket_path, sizeof(ctrl_socket_path),
			       "%s/%s/%s", pfcd_work_dir,
			       PFC_PFCD_CHANNEL_NAME, PFC_PFCD_CTRL_NAME);
	if (PFC_EXPECT_FALSE(len >= sizeof(ctrl_socket_path))) {
		pfc_log_fatal("Too long ctrl port path: %s/%s/%s",
			      pfcd_work_dir, PFC_PFCD_CHANNEL_NAME,
			      PFC_PFCD_CTRL_NAME);
		/* NOTREACHED */
	}

	PFC_ASSERT(strlen(ctrl_socket_path) == len);

	/*
	 * Ensure that the control port doesn't exist.
	 * We can safely remove the port because no other PFC daemon can't be
	 * executed because of PID file serialization.
	 */
	(void)unlink(ctrl_socket_path);

	/* Create a socket with setting close-on-exec and non-blocking flag. */
	sock = pfc_sock_open(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC_NB);
	if (PFC_EXPECT_FALSE(sock == -1)) {
		pfc_log_fatal("Failed to create socket for control port: %s",
			      strerror(errno));
		/* NOTREACHED */
	}

	/* Make this socket visible to the filesystem. */
	un.sun_family = AF_UNIX;
	memcpy(un.sun_path, ctrl_socket_path, len);
	if (PFC_EXPECT_TRUE(len < sizeof(un.sun_path))) {
		un.sun_path[len] = '\0';
	}

	if (PFC_EXPECT_FALSE(bind(sock, (const struct sockaddr *)&un,
				  PFC_SOCK_UNADDR_SIZE(len)) == -1)) {
		pfc_log_fatal("bind(%s) failed: %s", ctrl_socket_path,
			      strerror(errno));
		/* NOTREACHED */
	}

	ctrl_socket_bound = 1;

	/* Make this socket as a listener. */
	if (PFC_EXPECT_FALSE(listen(sock, CTRL_BACKLOG) == -1)) {
		pfc_log_fatal("listen(%s) failed: %s", ctrl_socket_path,
			      strerror(errno));
		/* NOTREACHED */
	}

	/* Change access permission. */
	if (chmod(ctrl_socket_path, cperm) == -1) {
		pfc_log_fatal("chmod(%s) failed: %s", ctrl_socket_path,
			      strerror(errno));
		/* NOTREACHED */
	}

	ctrl_socket = sock;
}

/*
 * void
 * ctrl_main(void)
 *	Main routine for daemon control thread.
 */
void
ctrl_main(void)
{
	sigset_t	mask;
	pfc_bool_t	loop = PFC_TRUE;

	/* Initialize signal mask for polling. */
	sigemptyset(&mask);

	do {
		cproto_sess_t	session;
		struct pollfd	pfd;
		int	nfds, fd, err;

		/* Wait for requests to control port. */
		pfd.fd = ctrl_socket;
		pfd.events = POLLIN;
		pfd.revents = 0;

		nfds = ppoll(&pfd, 1, NULL, (const sigset_t *)&mask);
		if (nfds < 0) {
			err = errno;
			if (PFC_EXPECT_TRUE(err == EINTR)) {
				/* Dispatch signals. */
				if (signal_dispatch()) {
					break;
				}
			}
			else {
				pfc_log_warn("ppoll() failed on control "
					     "thread: %s", strerror(err));
			}

			continue;
		}

		if (PFC_EXPECT_FALSE(nfds == 0)) {
			continue;
		}

		/* Accept connection request from client. */
		fd = pfc_sock_accept(ctrl_socket, NULL, NULL,
				     PFC_SOCK_CLOEXEC_NB);
		if (PFC_EXPECT_FALSE(fd == -1)) {
			err = errno;
			if (!PFC_IS_EWOULDBLOCK(err)) {
				pfc_log_error("accept() failed on control "
					      "thread: %s", strerror(err));
			}
			continue;
		}

		/* Establish control protocol session. */
		err = ctrl_session_init(&session, fd);
		if (PFC_EXPECT_FALSE(err != 0)) {
			continue;
		}

		pfc_log_verbose("New control session has been accepted.");

		/* Execute control command. */
		for (;;) {
			sigset_t	old;

			err = ctrl_execute(&session);
			if (err < 0) {
				/*
				 * Command loop must be terminated without
				 * destroying the session.
				 */
				goto next;
			}
			if (PFC_EXPECT_FALSE(err != 0)) {
				break;
			}

			/* Unmask signals. */
			PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, &mask,
						       &old), 0);
			PFC_ASSERT_INT(pthread_sigmask(SIG_SETMASK, &old,
						       NULL), 0);
			if (signal_dispatch()) {
				loop = PFC_FALSE;
				break;
			}
		}

		PFC_ASSERT_INT(pfc_iostream_destroy(session.cps_stream), 0);
		pfc_log_verbose("Control session has been disconnected.");
	next:;
	} while (loop);

	/* Close control port. */
	close(ctrl_socket);
}

/*
 * void
 * ctrl_cleanup(void)
 *	Clean up control socket.
 */
void
ctrl_cleanup(void)
{
	uint8_t	bound = pfc_atomic_swap_uint8(&ctrl_socket_bound, 0);

	if (bound) {
		pfc_log_verbose("Unlink control socket: %s", ctrl_socket_path);
		(void)unlink(ctrl_socket_path);
	}
}

/*
 * static void
 * ctrl_setup_directory(const char *path, mode_t mode)
 *	Set up channel directory.
 *	The caller must specify path to channel directory.
 */
static void
ctrl_setup_directory(const char *path, mode_t mode)
{
	int	err;

	/* Clean up channel directory. */
	err = pfc_rmpath(path);
	if (PFC_EXPECT_FALSE(err != 0 && err != ENOENT)) {
		pfc_log_fatal("Failed to remove channel directory: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	/* Append executable bits if needed. */
	if ((mode & S_IRWXU) != 0) {
		mode |= S_IXUSR;
	}
	if ((mode & S_IRWXG) != 0) {
		mode |= S_IXGRP;
	}
	if ((mode & S_IRWXO) != 0) {
		mode |= S_IXOTH;
	}

	/* Socket directory should never be group or world writable. */
	mode &= ~(S_IWGRP | S_IWOTH);

	/* Create channel directory. */
	if (mkdir(path, mode) != 0) {
		pfc_log_fatal("Failed to create channel directory: %s",
			      strerror(errno));
		/* NOTREACHED */
	}
}

/*
 * static pfc_bool_t
 * ctrl_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	EINTR callback for control session.
 */
static pfc_bool_t
ctrl_intr_callback(pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	/*
	 * Dispatch signals.
	 * signal_dispatch() returns PFC_TRUE if shutdown signal has been
	 * received. So I/O on control session will be terminated in that case.
	 */
	return signal_dispatch();
}

/*
 * static int
 * ctrl_session_init(cproto_sess_t *sess, int fd)
 *	Initialize control protocol session.
 */
static int
ctrl_session_init(cproto_sess_t *sess, int fd)
{
	pfc_iostream_t	stream;
	pfc_iowait_t	iowait;
	sigset_t	mask;
	int		err;

	/*
	 * Determine I/O wait behavior.
	 * We must set EINTR callback in order to dispatch signals.
	 */
	iowait.iw_intrfunc = ctrl_intr_callback;
	iowait.iw_intrarg = NULL;
	sigemptyset(&mask);
	iowait.iw_sigmask = &mask;

	/* Create control protocol stream. */
	err = pfc_iostream_create(&stream, fd, CTRL_BUFSIZE_IN,
				  CTRL_BUFSIZE_OUT, &iowait);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("control: Failed to create I/O stream: %s ",
			      strerror(err));
		goto error;
	}

	/* Initialize session context. */
	cproto_sess_init(sess, stream, ctrl_session_error, NULL);

	/* Read a handshake message from the client. */
	err = cproto_handshake_read(sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Send a response of handshake. */
	err = cproto_handshake_write(sess);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	return 0;

error:
	PFC_ASSERT_INT(pfc_iostream_destroy(stream), 0);

	return err;
}

/*
 * static void
 * ctrl_session_error(int err, const char *fmt, va_list ap)
 *	Record error logs on the control session.
 */
static void
ctrl_session_error(int err, const char *fmt, va_list ap)
{
	if (err == EINTR) {
		/* This EINTR should be caused by shutdown request. */
		return;
	}

	pfc_log_error_v(fmt, ap);
}

/*
 * static int
 * ctrl_execute(cproto_sess_t *sess)
 *	Read a command from the specified session stream, and execute it.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	A negative value is returned if the command loop should be terminated.
 *	In that case, the caller must not dispose the session.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ctrl_execute(cproto_sess_t *sess)
{
	const ctrl_cmdops_t	*ops;
	ctrl_cmdtype_t	cmd;
	uint32_t	index;
	int		err;

	/* Read command type. */
	err = cproto_cmd_read(sess, &cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	if (PFC_EXPECT_FALSE(cmd == CTRL_CMDTYPE_NOP)) {
		/* Send response. */
		err = cproto_resp_write(sess, CTRL_RESP_OK);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		err = ctrl_terminate_response(sess, PFC_FALSE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		return 0;
	}

	if (cmd == CTRL_CMDTYPE_QUIT || cmd == CTRL_CMDTYPE_EOF) {
		/* Quit control session. */
		return ECANCELED;
	}

	index = CTRL_CMDOPS_IDX(cmd);
	if (PFC_EXPECT_FALSE(index >= PFC_ARRAY_CAPACITY(command_ops))) {
		pfc_log_error("control: Unknown command: 0x%x", cmd);

		return EPROTO;
	}

	/* Execute command. */
	ops = &command_ops[index];

	return ops->execute(sess);
}

/*
 * static int
 * ctrl_terminate_response(cproto_sess_t *sess, pfc_bool_t need_null)
 *	Terminate the command response message.
 *	If `need_null' is true, NULL is sent to the session stream.
 */
static int
ctrl_terminate_response(cproto_sess_t *sess, pfc_bool_t need_null)
{
	int	err;

	if (need_null) {
		/* Send NULL data. */
		err = cproto_data_write_null(sess);
		if (PFC_EXPECT_FALSE(err != 0)) {
			CTRL_LOG_ERROR(err,
				       ("control: Failed to terminate "
					"response: %s", strerror(err)));

			return err;
		}
	}

	/* Flush the stream to send whole response message. */
	err = pfc_iostream_flush(sess->cps_stream, &sess->cps_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		CTRL_LOG_ERROR(err,
			       ("control: Failed to flush control session: %s",
				strerror(err)));
	}

	return err;
}

/*
 * static int
 * ctrl_error(cproto_sess_t *PFC_RESTRICT sess, int retval,
 *	      const char *PFC_RESTRICT fmt, ...)
 *	Send error response with error message.
 *	Output stream of the session is flushed.
 *
 * Calling/Exit State:
 *	Upon successful completion, value specified to `retval' is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ctrl_error(cproto_sess_t *PFC_RESTRICT sess, int retval,
	   const char *PFC_RESTRICT fmt, ...)
{
	va_list	ap;
	int	err;

	va_start(ap, fmt);
	err = ctrl_verror(sess, retval, fmt, ap);
	va_end(ap);

	return err;
}

/*
 * static int
 * ctrl_verror(cproto_sess_t *PFC_RESTRICT sess, int retval,
 *	       const char *PFC_RESTRICT fmt, va_list ap)
 *	Send error response with error message specified by printf(3) style
 *	format and va_list.
 *	Output stream of the session is flushed.
 *
 * Calling/Exit State:
 *	Upon successful completion, value specified to `retval' is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ctrl_verror(cproto_sess_t *PFC_RESTRICT sess, int retval,
	    const char *PFC_RESTRICT fmt, va_list ap)
{
	int	err;
	char	buf[CTRL_RESP_ERRMSG_MAX + 1];

	vsnprintf(buf, sizeof(buf), fmt, ap);
	buf[CTRL_RESP_ERRMSG_MAX] = '\0';

	err = cproto_resp_write(sess, CTRL_RESP_FAIL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = cproto_data_write_text(sess, buf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		CTRL_LOG_ERROR(err,
			       ("control: Failed to send error message: %s",
				strerror(err)));
		CTRL_LOG_ERROR(err,
			       ("control: Error: %s", buf));
		return err;
	}

	err = pfc_iostream_flush(sess->cps_stream, &sess->cps_timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		CTRL_LOG_ERROR(err,
			       ("control: Failed to flush control session: %s",
				strerror(err)));

		return err;
	}

	return retval;
}

/*
 * static int
 * ctrl_command_loglevel(cproto_sess_t *sess)
 *	Execute LOGLEVEL command.
 */
static int
ctrl_command_loglevel(cproto_sess_t *sess)
{
	cproto_data_t	data1, data2;
	ctrl_pdtype_t	type;
	int		err;
	pfc_log_level_t	level;
	const char	*modname;

	pfc_log_verbose("control: LOGLEVEL");

	/* Read target module name. */
	err = cproto_data_read(sess, &data1);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	type = data1.cpd_type;
	if (type == CTRL_PDTYPE_NULL) {
		modname = NULL;
	}
	else if (PFC_EXPECT_TRUE(type == CTRL_PDTYPE_TEXT)) {
		modname = cproto_data_text(&data1);
	}
	else {
		cproto_data_free(&data1);

		return ctrl_error(sess, EPROTO,
				  "Unexpected data type for target module: %u",
				  type);
	}

	/*
	 * Check whether control command is request to set log level,
	 * or get log level.
	 */
	err = cproto_data_read(sess, &data2);
	if (PFC_EXPECT_FALSE(err != 0)) {
		cproto_data_free(&data1);

		return err;
	}

	/*
	 * Remarks: cproto_data_free() doesn't need to be called if data type
	 * is NULL.
	 */
	if ((type = data2.cpd_type) == CTRL_PDTYPE_INT32) {
		/* This value is a new log level. */
		level = (pfc_log_level_t)cproto_data_int32(&data2);
		cproto_data_free(&data2);

		if (level == CTRL_LOGLVL_MODLIST) {
			/* List per-module logging level. */
			return loglevel_modlist(sess);
		}

		err = loglevel_set(sess, modname, level);
		if (PFC_EXPECT_FALSE(err != 0)) {
			cproto_data_free(&data1);

			return err;
		}
	}
	else if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_NULL)) {
		cproto_data_free(&data1);
		cproto_data_free(&data2);

		return ctrl_error(sess, EPROTO,
				  "Unexpected data type for logging level: %u",
				  type);
	}

	/* Return current log level. */
	level = (modname == NULL)
		? *__pfc_log_current_level
		: __pfc_log_current_modlevel(modname);

	cproto_data_free(&data1);

	/* Send response. */
	err = cproto_resp_write(sess, CTRL_RESP_OK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = cproto_data_write_int32(sess, (int32_t)level);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	return ctrl_terminate_response(sess, PFC_FALSE);
}

/*
 * static int
 * ctrl_command_modlist(cproto_sess_t *sess)
 *	Execute MODLIST command.
 */
static int
ctrl_command_modlist(cproto_sess_t *sess)
{
	int	err;

	pfc_log_verbose("control: MODLIST");

	/* Send response. */
	err = cproto_resp_write(sess, CTRL_RESP_OK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* MODLIST requires no argument. */
	err = pfc_module_iterate(modlist_iterator, (pfc_ptr_t)sess);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = ctrl_terminate_response(sess, PFC_TRUE);
	}

	return err;
}

/*
 * static int
 * ctrl_command_event(cproto_sess_t *sess)
 *	Execute EVENT command.
 */
static int
ctrl_command_event(cproto_sess_t *sess)
{
	cproto_data_t	data;
	ctrl_pdtype_t	type;
	uint32_t	nevents;
	ctrl_event_t	*events, **nextp;
	int		err;

	pfc_log_verbose("control: EVENT");

	/* Read the number of events to be sent. */
	err = cproto_data_read(sess, &data);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	type = data.cpd_type;
	if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_INT32)) {
		cproto_data_free(&data);

		return ctrl_error(sess, EPROTO,
				  "EVENT: Unexpected data type for the number "
				  "of events: %u", type);
	}

	nevents = cproto_data_uint32(&data);
	cproto_data_free(&data);

	if (PFC_EXPECT_FALSE(nevents == 0)) {
		return ctrl_error(sess, EPROTO,
				  "EVENT: The number of events must not be "
				  "zero.");
	}

	events = NULL;
	nextp = &events;
	for (; nevents > 0; nevents--) {
		ctrl_event_t	*cep;

		cep = (ctrl_event_t *)malloc(sizeof(*cep));
		if (PFC_EXPECT_FALSE(cep == NULL)) {
			err = ctrl_error(sess, ENOMEM,
					 "EVENT: No memory for event data.");
			goto error;
		}

		cep->ce_name = NULL;
		cep->ce_next = NULL;
		*nextp = cep;
		nextp = &cep->ce_next;

		/* Read target module name. */
		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		/*
		 * NULL PDU is sent for broadcast event.
		 * Otherwise TEXT PDU which contains the target module name
		 * must be sent.
		 */
		type = data.cpd_type;
		if (type == CTRL_PDTYPE_TEXT) {
			/*
			 * Don't call cproto_data_free() here because it
			 * releases this string. The string read here will be
			 * released by the call of event_list_free().
			 */
			cep->ce_name = cproto_data_text(&data);
		}
		else if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_NULL)) {
			cproto_data_free(&data);
			err = ctrl_error(sess, EPROTO,
					 "EVENT: Unexpected data type for "
					 "the target module: %u", type);
			goto error;
		}

		/* Read event type. */
		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		type = data.cpd_type;
		if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_INT32)) {
			cproto_data_free(&data);
			err = ctrl_error(sess, EPROTO,
					 "EVENT: Unexpected data type for "
					 "event type: %u", type);
			goto error;
		}

		cep->ce_type = cproto_data_uint32(&data);
		cproto_data_free(&data);
	}

	/* Send response. */
	err = cproto_resp_write(sess, CTRL_RESP_OK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	while (events != NULL) {
		ctrl_event_t	*cep = events;
		const char	*name = cep->ce_name;
		uint32_t	type = cep->ce_type;

		events = cep->ce_next;

		pfc_log_info("Post a module-specific event: name=%s, type=%u",
			     (name == NULL) ? "<all>" : name, type);
		err = pfc_modevent_post_ex(name, type);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to send module-specific event: "
				      "target=%s, type=%u, err=%s",
				      (name == NULL) ? "<all>" : name, type,
				      strerror(err));
		}

		free((void *)name);
		free(cep);

		/* Send result. */
		err = cproto_data_write_int32(sess, err);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}

	/* Flush output stream. */
	return ctrl_terminate_response(sess, PFC_FALSE);

error:
	event_list_free(events);

	return err;
}

/*
 * static int
 * loglevel_set(cproto_sess_t *PFC_RESTRICT sess,
 *		const char *PFC_RESTRICT modname, pfc_log_level_t level)
 *	Change current logging level to `level'.
 *	If `level' is negative value, this function reset the logging level
 *	to initial level.
 *
 *	`modname' must be a pointer to string which represents module name.
 *	If NULL is specified to `modname', this function changes global logging
 *	level.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
loglevel_set(cproto_sess_t *PFC_RESTRICT sess,
	     const char *PFC_RESTRICT modname, pfc_log_level_t level)
{
	int	ret;

	if ((int32_t)level < 0) {
		/* Reset to initial logging level. */
		if (modname != NULL || level == CTRL_LOGLVL_MODRESET) {
			/* Remove per-module logging level. */
			pfc_log_modlevel_reset(modname);

			return 0;
		}
		if (PFC_EXPECT_FALSE(level != CTRL_LOGLVL_RESET)) {
			/* This should never happen. */
			return ctrl_error(sess, EPROTO,
					  "Unexpected pseudo logging level: %d",
					  level);
		}

		level = PFC_LOGLVL_NONE;
	}

	if (modname != NULL) {
		PFC_ASSERT((int32_t)level >= 0);
		ret = pfc_log_modlevel_set(modname, level);
	}
	else {
		ret = pfc_log_set_level(level);
	}

	if (PFC_EXPECT_FALSE(ret < 0)) {
		int	err = -ret;

		if (err == EINVAL) {
			return ctrl_error(sess, 0, "Invalid logging level: %u",
					  level);
		}

		return ctrl_error(sess, err, "Failed to set logging level: %s",
				  strerror(err));
	}

	return 0;
}

/*
 * static int
 * loglevel_modlist(cproto_sess_t *sess)
 *	Send per-module logging level configuration to the client.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
loglevel_modlist(cproto_sess_t *sess)
{
	pfc_log_modconf_t	*levels, *plmp;
	int	count, err;

	/* Copy current configuration of per-module logging level. */
	count = pfc_log_modlevel_copy(&levels);
	if (PFC_EXPECT_FALSE(count < 0)) {
		return ctrl_error(sess, ENOMEM, "Failed to copy per-module "
				  "logging configuration.");
	}

	/* Send response. */
	err = cproto_resp_write(sess, CTRL_RESP_OK);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Send number of pfc_log_modconf_t array elements. */
	err = cproto_data_write_int32(sess, count);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	for (plmp = levels; plmp < levels + count; plmp++) {
		/* Send module name. */
		err = cproto_data_write_text(sess, plmp->plm_name);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		/* Send logging level for this module. */
		err = cproto_data_write_int32(sess, (int32_t)plmp->plm_level);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}
	}

	/* Flush output stream. */
	err = ctrl_terminate_response(sess, PFC_FALSE);

out:
	pfc_log_modlevel_free(levels, count);

	return err;
}

/*
 * static int
 * modlist_iterator(pmodule_t *PFC_RESTRICT mod, pfc_ptr_t PFC_RESTRICT arg)
 *	Module iterator to send module information to the client.
 */
static int
modlist_iterator(pmodule_t *PFC_RESTRICT mod, pfc_ptr_t PFC_RESTRICT arg)
{
	cproto_sess_t	*sess = (cproto_sess_t *)arg;
	int		err;

	/* Send module name. */
	err = cproto_data_write_text(sess, PMODULE_NAME(mod));
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Send module state. */
	err = cproto_data_write_int32(sess, mod->pm_state);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Send module flags. */
	return cproto_data_write_int32(sess, mod->pm_flags);
}

/*
 * static void
 * event_list_free(ctrl_event_t *events)
 *	Free up resources held by the given ctrl_event_t list.
 */
static void
event_list_free(ctrl_event_t *events)
{
	ctrl_event_t	*next;

	for (; events != NULL; events = next) {
		next = events->ce_next;
		free((void *)events->ce_name);
		free(events);
	}
}
