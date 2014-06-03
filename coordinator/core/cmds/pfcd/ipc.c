/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * ipc.c - IPC service management.
 */

#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pfc/thread.h>
#include <pfc/conf.h>
#include <pfc/strtoint.h>
#include <pfc/ipc_client.h>
#include <event_impl.h>
#include <property_impl.h>
#include "pfcd.h"
#include "ipc.h"

/*
 * Stack size of the listener thread.
 */
#define	IPC_LISTENER_STKSIZE		PFC_CONST_U(0x8000)	/* 32K */

/*
 * Interval to be inserted after error on the listener thread.
 */
#define	IPC_LISTENER_ERR_INTERVAL	PFC_CONST_U(1)		/* 1 sec */

/*
 * How long, in seconds, we should wait for completion of IPC event delivery.
 */
#define	IPC_EVWAIT_TIMEOUT		PFC_CONST_U(10)		/* 10 sec */

/*
 * Load options for IPC event subsystem.
 * Note that zero means default value.
 */
#define	IPC_EVENT_LOAD_OPTION(cfblk, opts, name)			\
	do {								\
		(opts)->evopt_##name =					\
			pfc_conf_get_uint32((cfblk), #name, 0); \
	} while (0)

/*
 * Separator in PFC_PROP_IPC_NOTIFY value.
 */
#define	IPC_PROP_NOTIFY_SEP		','

/*
 * Hook function to post an IPC event.
 */
typedef const struct {
	pfc_ipcevtype_t		pe_type;	/* event type */

	/*
	 * int
	 * pe_prepare(pfc_ipcsrv_t *srv)
	 *	Prepare an IPC event.
	 *	This method is used to set additional data to the given event.
	 *
	 * Calling/Exit State:
	 *	Upon successful completion, zero is returned.
	 *	Otherwise error number which indicates the cause of error is
	 *	returned.
	 */
	int	(*pe_prepare)(pfc_ipcsrv_t *srv);
} pfcd_ipcevopt_t;

/*
 * Name of IPC channel.
 */
static const char	*ipc_channel_name;

/*
 * IPC service listener thread.
 */
static pthread_t	ipc_listener_thread = PFC_PTHREAD_INVALID_ID;

/*
 * Internal prototypes.
 */
static void	ipc_server_init(int shutfd);
static void	ipc_client_init(void);
static void	ipc_client_fini(void);
static void	*ipc_listener(void *arg);
static int	ipc_thread_create(void *(*func)(void *), void *arg);

static int	ipc_event_prepare_stop(pfc_ipcsrv_t *srv);

/*
 * IPC server operations.
 */
static const pfc_ipcsrvops_t	ipc_pfcd_ops = {
	.isvops_thread_create	= ipc_thread_create,
};

/*
 * Operations to construct IPC event.
 */
#define	PFCD_IPCEVENT_PREPARE_SYS_STOP	ipc_event_prepare_stop

#define	PFCD_IPCEVOPT_DECL(type)				\
	{							\
		.pe_type	= PFCD_IPCEVENT_##type,		\
		.pe_prepare	= PFCD_IPCEVENT_PREPARE_##type,	\
	}

static pfcd_ipcevopt_t		ipc_event_opts[] = {
	PFCD_IPCEVOPT_DECL(SYS_STOP),
};

/*
 * Determine whether to enable IPC event subsystem.
 */
static pfc_bool_t	ipc_event_enabled;

/*
 * void
 * ipc_init(void)
 *	Initialize the IPC service of the PFC daemon.
 */
void
ipc_init(void)
{
	int	shutfd;

	shutfd = libpfc_shutdown_getfd();
	if (PFC_EXPECT_FALSE(shutfd == -1)) {
		/* Shutdown sequence is already running. */
		return;
	}

	/* Initialize IPC server. */
	ipc_server_init(shutfd);

	/* Initialize IPC client. */
	ipc_client_init();
}

/*
 * void
 * ipc_start(void)
 *	Start the IPC service on the "pfcd" channel.
 */
void
ipc_start(void)
{
	pthread_attr_t	attr;
	int		err;

	if (ipc_channel_name == NULL) {
		/* IPC server is disabled. */
		return;
	}

	/* Create the IPC service listener thread. */
	PFC_ASSERT_INT(pthread_attr_init(&attr), 0);
	PFC_ASSERT_INT(pthread_attr_setstacksize(&attr, IPC_LISTENER_STKSIZE),
		       0);
        PFC_ASSERT_INT(pthread_attr_setdetachstate
		       (&attr, PTHREAD_CREATE_JOINABLE), 0);

	err = pthread_create(&ipc_listener_thread, &attr, ipc_listener, NULL);
	PFC_ASSERT_INT(pthread_attr_destroy(&attr), 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to start IPC listener thread: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void
 * ipc_notify_ready(void)
 *	Issue IPC service request to notify that the service is ready.
 */
void
ipc_notify_ready(void)
{
	pfc_ipcconn_t	conn;
	pfc_ipcsess_t	*sess;
	pfc_ipcresp_t	resp;
	const char	*notify;
	char		*channel, *svname, *p;
	uint32_t	svid;
	int		err;

	if (ipc_channel_name == NULL) {
		/* IPC server is disabled. */
		return;
	}

	notify = pfc_prop_get(PFC_PROP_IPC_NOTIFY);
	if (notify == NULL) {
		/* Service-ready notification is not specified. */
		return;
	}

	channel = strdup(notify);
	if (PFC_EXPECT_FALSE(channel == NULL)) {
		pfc_log_fatal("Failed to copy service-ready notification "
			      "property.");

		return;
	}

	/* Parse notification argument. */
	p = strchr(channel, IPC_PROP_NOTIFY_SEP);
	if (PFC_EXPECT_FALSE(p == NULL)) {
		goto bad_format;
	}

	*p = '\0';
	svname = p + 1;

	p = strchr(svname, IPC_PROP_NOTIFY_SEP);
	if (PFC_EXPECT_FALSE(p == NULL)) {
		goto bad_format;
	}

	*p = '\0';
	p++;

	err = pfc_strtou32(p, &svid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto bad_format;
	}

	/* Create an IPC client session on alternative connection. */
	err = pfc_ipcclnt_altopen(channel, &conn);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create connection for notification: "
			      "addr=%s: %s", channel, strerror(err));
		goto error;
	}

	err = pfc_ipcclnt_sess_altcreate(&sess, conn, svname, svid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create notification session: "
			      "addr=%s, service=%s/%u: %s", channel, svname,
			      svid, strerror(err));
		goto error_conn;
	}

	err = pfc_ipcclnt_output_string(sess, ipc_channel_name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to add IPC channel name to notification "
			      "message: %s", strerror(err));
		goto error_sess;
	}

	err = pfc_ipcclnt_sess_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to send notification message: "
			      "addr=%s, service=%s/%u: %s", channel, svname,
			      svid, strerror(err));
		goto error_sess;
	}

	if (PFC_EXPECT_FALSE(resp != 0)) {
		pfc_log_fatal("Notification service returned non-zero value: "
			      "addr=%s, service=%s/%u, resp=%d",
			      channel, svname, svid, resp);
		goto error_sess;
	}

	PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);
	PFC_ASSERT_INT(pfc_ipcclnt_altclose(conn), 0);
	free(channel);

	return;

error_sess:
	PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);

error_conn:
	PFC_ASSERT_INT(pfc_ipcclnt_altclose(conn), 0);

error:
	free(channel);

	return;

bad_format:
	free(channel);
	pfc_log_fatal("Unexpected format of notification property: %s",
		      notify);
}

/*
 * int
 * ipc_post_event(pfc_ipcevtype_t type)
 *	Post an IPC event of the specified type.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if the IPC server is disabled.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
ipc_post_event(pfc_ipcevtype_t type)
{
	pfc_ipcsrv_t	*srv;
	pfc_ipcevdesc_t	desc;
	pfc_timespec_t	timeout;
	pfcd_ipcevopt_t	*opt;
	int		err;

	if (ipc_channel_name == NULL) {
		/* IPC server is disabled. */
		return -1;
	}

	/* Create an IPC event. */
	err = pfc_ipcsrv_event_create(&srv, PFCD_IPC_SERVICE, type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create IPC event(%u): %s",
			      type, strerror(err));

		return err;
	}

	/* Set up additional data. */
	PFC_ASSERT(type < PFC_ARRAY_CAPACITY(ipc_event_opts));
	opt = &ipc_event_opts[type];
	err = opt->pe_prepare(srv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT_INT(pfc_ipcsrv_event_destroy(srv), 0);

		return err;
	}

	err = pfc_ipcsrv_evdesc_create(&desc, srv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_warn("Failed to create IPC event(%u) delivery "
			     "descriptor: %s", type, strerror(err));
		PFC_ASSERT(desc == PFC_IPCEVDESC_INVALID);
		/* FALLTHROUGH */
	}

	/* Post an IPC event. */
	err = pfc_ipcsrv_event_post(srv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to post IPC event(%u): %s",
			      type, strerror(err));

		return err;
	}

	if (PFC_EXPECT_TRUE(desc != PFC_IPCEVDESC_INVALID)) {
		/* We need to wait for completion of IPC event delivery. */
		timeout.tv_sec = IPC_EVWAIT_TIMEOUT;
		timeout.tv_nsec = 0;
		err = pfc_ipcsrv_evdesc_wait(desc, &timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Could not wait for completion of "
				      "IPC event(%u) delivery: %s",
				      type, strerror(err));
			/* FALLTHROUGH */
		}
	}

	return err;
}

/*
 * void
 * ipc_shutdown(void)
 *	Start shutdown sequence of the IPC subsystem.
 */
void
ipc_shutdown(void)
{
	int	err;

	/* Shut down the IPC event subsystem. */
	err = pfc_ipcclnt_event_shutdown();
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to shut down the IPC event subsystem: "
			      "%s", strerror(err));
		/* FALLTHROUGH */
	}
}

/*
 * void
 * ipc_fini(void)
 *	Finalize the IPC service of the PFC daemon.
 */
void
ipc_fini(void)
{
	pthread_t	t;
	int		err;

	/* Finalize the IPC client. */
	ipc_client_fini();

	t = ipc_listener_thread;
	if (t != PFC_PTHREAD_INVALID_ID) {
		err = pthread_join(t, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to join the IPC listener thread"
				      ": %s", strerror(err));
			/* FALLTHROUGH */
		}

		ipc_listener_thread = PFC_PTHREAD_INVALID_ID;
	}

	pfc_ipcsrv_fini();
}

/*
 * static void
 * ipc_server_init(int shutfd)
 *	Initialize IPC server.
 */
static void
ipc_server_init(int shutfd)
{
	const char	*channel;
	pfc_cfblk_t	cfblk;
	int		err;

	/* Determine IPC channel name. */
	cfblk = pfc_sysconf_get_block("ipc_server");
	channel = pfc_conf_get_string(cfblk, "channel_name", pfcd_progname);
	if (PFC_EXPECT_FALSE(*channel == '\0')) {
		pfc_log_info("IPC server is disabled.");

		return;
	}

	ipc_channel_name = channel;

	/* Enable logging in IPC server library. */
	pfc_ipcsrv_enable_log(PFC_TRUE);

	/* Set IPC channel name to the system property. */
	err = pfc_prop_set(PFC_PROP_IPC_CHANNEL, channel);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to set IPC channel name to the system "
			      "property: %s", strerror(err));
		/* NOTREACHED */
	}

	/* Initialize IPC service. */
	err = pfc_ipcsrv_sysinit(channel, shutfd, &ipc_pfcd_ops);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* ECANCELED must not return because signal is still masked. */
		pfc_log_fatal("Failed to initialize IPC service: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * ipc_client_init()
 *	Initialize IPC client.
 */
static void
ipc_client_init()
{
	pfc_cfblk_t	cfblk;
	pfc_ipcevopts_t	opts;
	pfc_bool_t	auto_cancel;
	int		err;

	/* Determine options for IPC event subsystem. */
	cfblk = pfc_sysconf_get_block("ipc_event");

	/*
	 * Determine whether IPC event subsystem should be initialized
	 * or not.
	 */
	ipc_event_enabled = pfc_conf_get_bool(cfblk, "enable", PFC_TRUE);
	if (PFC_EXPECT_FALSE(!ipc_event_enabled)) {
		pfc_log_info("IPC event subsystem is disabled.");

		return;
	}

	/* Enable logging in IPC client library. */
	pfc_ipcclnt_enable_log(PFC_TRUE);

	/* Enable auto-cancellation of IPC client session if requested. */
	auto_cancel = pfc_conf_get_bool(cfblk, "auto_cancel", PFC_FALSE);
	pfc_ipcclnt_event_setautocancel(auto_cancel);

	/* Zero means default value. */
	IPC_EVENT_LOAD_OPTION(cfblk, &opts, idle_timeout);
	IPC_EVENT_LOAD_OPTION(cfblk, &opts, maxthreads);
	IPC_EVENT_LOAD_OPTION(cfblk, &opts, conn_interval);
	IPC_EVENT_LOAD_OPTION(cfblk, &opts, timeout);

	/* Create event listener task threads from thread pool. */
	opts.evopt_thread_create = ipc_thread_create;

	/* Initialize IPC event subsystem. */
	err = pfc_ipcclnt_event_init(&opts);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to initialize IPC event subsystem: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * static void
 * ipc_client_fini(void)
 *	Finalize the IPC client.
 */
static void
ipc_client_fini(void)
{
	if (PFC_EXPECT_TRUE(ipc_event_enabled)) {
		int	err;

		/* Finalize the IPC event subsystem. */
		err = pfc_ipcclnt_event_fini();
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("Failed to finalize the IPC event "
				      "subsystem: %s", strerror(err));
			/* FALLTHROUGH */
		}
	}

	/* Finalize the IPC client library. */
	pfc_ipcclnt_cancel(PFC_TRUE);
}

/*
 * static void *
 * ipc_listener(void *arg)
 *	Start routine of the IPC listener thread.
 */
static void *
ipc_listener(void *arg)
{
	struct timespec	ts = {IPC_LISTENER_ERR_INTERVAL, 0};
	int		err;

	pfc_log_info("Start IPC listener thread.");

		/* Call IPC server's main loop. */
	while ((err = pfc_ipcsrv_main()) != ECANCELED) {
		PFC_ASSERT(err != ENODEV);
		(void)nanosleep(&ts, NULL);
	}

	pfc_log_info("IPC listener thread has been terminated.");

	return NULL;
}

/*
 * static int
 * ipc_thread_create(void *(*func)(void *), void *arg)
 *	Create a new thread for an IPC server session.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_thread_create(void *(*func)(void *), void *arg)
{
	pfc_thread_t	t;

	return pfc_thread_create(&t, func, arg, PFC_THREAD_DETACHED);
}

/*
 * static int
 * ipc_event_prepare_stop(pfc_ipcsrv_t *srv)
 *	Set additional data to PFCD_IPCEVENT_SYS_STOP event.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_event_prepare_stop(pfc_ipcsrv_t *srv)
{
	uint8_t	u8;
	int	err, status;

	/* Set exit status as UINT8 data. */
	status = pfcd_getexstatus();
	u8 = (status == 0) ? 1 : 0;

	err = pfc_ipcsrv_output_uint8(srv, u8);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to set exit status to SYS_STOP "
			      "IPC event: value=%u: %s", u8, strerror(err));
		/* FALLTHROUGH */
	}
	else {
		pfc_log_verbose("SYS_STOP: Set exit status: %u", u8);
	}

	return err;
}
