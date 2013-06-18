/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * launcher.c - launcher module definitions.
 */

/* Use launcher.conf definition in libunc_launcher.so. */
#include <unc/liblauncher.h>

#define	MODULE_CFDEF_FUNC()	liblnc_getmodcfdef()

#include <sys/stat.h>
#include <pfc/property.h>
#include <unc/lnc_ipc.h>
#include "launcher_impl.h"

/*
 * Permission bits for launcher directories.
 */
#define	LNC_PERM_DIR							\
	(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)	/* 0755 */

/*
 * Default name of working directory.
 */
#define	LNC_WORKDIR_DEFAULT		"launcher"

/*
 * Priority of module unload hook.
 */
#define	LNC_HOOK_PRI			PFC_CONST_U(0)	/* highest priority */

/*
 * Global launcher context.
 */
lnc_ctx_t	launcher_ctx PFC_ATTR_HIDDEN;

/*
 * Derive the number of milliseconds (UINT32) from the given configuration
 * block handle, and convert it into pfc_timespec_t value.
 */
#define	LNC_GETCONF_TIMESPEC(options, name, tsp)		\
	do {							\
		uint32_t	__u32;				\
								\
		__u32 = LNC_CONF_GET(options, uint32, name);	\
		pfc_log_verbose("%s=%u", #name, __u32);		\
		pfc_clock_msec2time((tsp), __u32);		\
	} while (0)

/*
 * Internal prototypes.
 */
static void		lnc_log_fatal_v(const char *fmt, va_list ap);
static pfc_bool_t	lnc_finalize_l(lnc_ctx_t *ctx);
static void		lnc_unload_hook(pfc_bool_t isstop);

/*
 * int
 * lncapi_isinitialized(void)
 *	Determine whether the launcher module is initialized or not.
 *
 * Calling/Exit State:
 *	Zero is returned if all daemons are already started.
 *	This means that LNC_EVTYPE_INIT event is already posted.
 *
 *	EBUSY is returned if the launcher module is now starting daemons.
 *	ECANCELED is returned if the launcher module is already finalized.
 */
int
lncapi_isinitialized(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	int		err;

	LNC_LOCK(ctx);

	if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
		err = ECANCELED;
	}
	else if (LNC_IS_INIT(ctx)) {
		err = 0;
	}
	else {
		err = EBUSY;
	}

	LNC_UNLOCK(ctx);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_log_fatal(const char *fmt, ...)
 *	Record fatal log.
 *	The call of this function will start shutdown sequence of the daemon.
 *
 * Remarks:
 *	This function must be called without holding the launcher lock.
 */
void PFC_ATTR_HIDDEN
lnc_log_fatal(const char *fmt, ...)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	va_list		ap;

	va_start(ap, fmt);
	LNC_LOCK(ctx);
	lnc_log_fatal_v(fmt, ap);
	LNC_UNLOCK(ctx);
	va_end(ap);
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_log_fatal_l(const char *fmt, ...)
 *	Record fatal log.
 *	The call of this function will start shutdown sequence of the daemon.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
void PFC_ATTR_HIDDEN
lnc_log_fatal_l(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	lnc_log_fatal_v(fmt, ap);
	va_end(ap);
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_wait(lnc_ctx_t *UNC_RESTRICT ctx,
 *	    const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Wait for all daemons to be launched.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ECANCELED is returned if the launcher module is finalized.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
int PFC_ATTR_HIDDEN
lnc_wait(lnc_ctx_t *UNC_RESTRICT ctx,
	 const pfc_timespec_t *UNC_RESTRICT abstime)
{
	return lnc_wait_flags(ctx, LNC_CF_INIT, abstime);
}

/*
 * int PFC_ATTR_HIDDEN
 * lnc_wait_flags(lnc_ctx_t *UNC_RESTRICT ctx, const uint32_t flags,
 *		  const pfc_timespec_t *UNC_RESTRICT abstime)
 *	Wait for the given launcher state flags to be set.
 *
 *	`flags' is a bitwise OR-ed launcher flags (LNC_CF_XXX).
 *	- At least one flag bit must be set.
 *	- LNC_CF_FINI must not be set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ECANCELED is returned if the launcher module is finalized.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
int PFC_ATTR_HIDDEN
lnc_wait_flags(lnc_ctx_t *UNC_RESTRICT ctx, const uint32_t flags,
	       const pfc_timespec_t *UNC_RESTRICT abstime)
{
	int	err = 0;

	PFC_ASSERT(flags != 0 && (flags & LNC_CF_FINI) == 0);

	for (;;) {
		if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
			err = ECANCELED;
			break;
		}
		if ((ctx->lc_flags & flags) == flags) {
			return 0;
		}
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		err = LNC_TIMEDWAIT_ABS(ctx, abstime);
	}

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * lnc_finalize(lnc_ctx_t *ctx)
 *	Finalize the launcher module.
 */
void PFC_ATTR_HIDDEN
lnc_finalize(lnc_ctx_t *ctx)
{
	LNC_LOCK(ctx);
	(void)lnc_finalize_l(ctx);
	LNC_UNLOCK(ctx);
}

/*
 * static void
 * lnc_log_fatal_v(const char *fmt, va_list ap)
 *	Record fatal log.
 *	The call of this function will start shutdown sequence of the daemon.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static void
lnc_log_fatal_v(const char *fmt, va_list ap)
{
	/*
	 * Don't call pfc_log_fatal_v() if the launcher module is already
	 * finalized.
	 */
	if (lnc_finalize_l(&launcher_ctx)) {
		pfc_log_fatal_v(fmt, ap);
	}
	else {
		pfc_log_error_v(fmt, ap);
	}
}

/*
 * static pfc_bool_t
 * lnc_finalize_l(lnc_ctx_t *ctx)
 *	Finalize the launcher module.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the launcher module has been finalized.
 *	PFC_FALSE is returned if the launcher module is already finalized.
 *
 * Remarks:
 *	This function must be called with holding the launcher lock.
 */
static pfc_bool_t
lnc_finalize_l(lnc_ctx_t *ctx)
{
	if (PFC_EXPECT_FALSE(LNC_IS_FINI(ctx))) {
		return PFC_FALSE;
	}

	/* Inhibit further daemon start. */
	LNC_SETFLAG_L(ctx, LNC_CF_FINI);

	/* Cancel all start-up notification wait. */
	lnc_daemon_fini();

	return PFC_TRUE;
}

/*
 * static void
 * lnc_unload_hook(pfc_bool_t isstop)
 *	Module unload hook.
 */
static void
lnc_unload_hook(pfc_bool_t isstop)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_timespec_t	limit;
	int		err;

	if (PFC_EXPECT_FALSE(!isstop)) {
		/* There is nothing to do in demand unloading case. */
		return;
	}

	LNC_LOCK(ctx);

	if (!LNC_IS_STOP_TASK_POSTED(ctx)) {
		/* We should stop low priority daemons here. */
		LNC_UNLOCK(ctx);
		lnc_daemon_stop_low();

		return;
	}

	PFC_ASSERT_INT(pfc_clock_gettime(&limit), 0);
	limit.tv_sec += LNC_FINI_TIMEOUT;

	/*
	 * Low priority daemon stop task is posted to a task queue.
	 * We should wait for its completion.
	 */
	err = 0;
	while (!LNC_IS_STOP_TASK_DONE(ctx)) {
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			pfc_log_error("Low priority daemon stop task did not "
				      "complete within %u seconds.",
				      LNC_FINI_TIMEOUT);
			break;
		}

		err = LNC_TIMEDWAIT_ABS(ctx, &limit);
	}

	LNC_UNLOCK(ctx);
}

/*
 * static pfc_bool_t
 * launcher_init(void)
 *	Initialize launcher module.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on successful return.
 *	Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
launcher_init(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_task_t	task;
	const char	*workdir = LNC_WORKDIR(ctx);
	int		err;

	if (PFC_EXPECT_FALSE(ctx->lc_ndaemons == 0)) {
		pfc_log_notice("No daemon is configured.");
		/* FALLTHROUGH */
	}

	/* Register module unload hook. */
	err = pfc_module_addhook(lnc_unload_hook, LNC_HOOK_PRI);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to register unload hook: %s",
			      strerror(err));

		return PFC_FALSE;
	}

	ctx->lc_channel = pfc_prop_get(PFC_PROP_IPC_CHANNEL);
	if (PFC_EXPECT_FALSE(ctx->lc_channel == NULL)) {
		/* The launcher module requires IPC service. */
		pfc_log_error("IPC server is disabled.");

		return PFC_FALSE;
	}

	/* Prepare working directory. */
	err = pfc_mkdir(workdir, LNC_PERM_DIR);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create working directory: %s: %s",
			      workdir, strerror(err));

		return PFC_FALSE;
	}

	/* Create a task queue for asynchronous jobs. */
	err = pfc_taskq_create(&ctx->lc_taskq, NULL, LNC_TASKQ_NTHREADS);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to create task queue: %s",
			      strerror(err));

		return PFC_FALSE;
	}

	/* Initialize IPC services. */
	err = lnc_ipc_init(ctx);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize launcher event. */
	err = lnc_event_init(ctx);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Initialize daemon command environment. */
	err = lnc_daemon_init();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_event;
	}

	/* Start all daemons. */
	err = pfc_taskq_dispatch(ctx->lc_taskq, lnc_daemon_start, ctx, 0,
				 &task);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to dispatch start-up task: %s",
			      strerror(err));
		goto error_event;
	}

	return PFC_TRUE;

error_event:
	lnc_event_fini(ctx, NULL);
	lnc_daemon_destroy();

error:
	PFC_ASSERT_INT(pfc_taskq_destroy(ctx->lc_taskq), 0);

	return PFC_FALSE;
}

/*
 * static pfc_bool_t
 * launcher_fini(void)
 *	Finalize launcher module.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on successful return.
 *	Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
launcher_fini(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_refptr_t	*workdir;
	pfc_timespec_t	limit;
	pfc_taskq_t	taskq;
	int		err;

	PFC_ASSERT_INT(pfc_clock_gettime(&limit), 0);
	limit.tv_sec += LNC_FINI_TIMEOUT;

	lnc_finalize(ctx);

	/* Destroy all IPC client sessions. */
	err = clstat_ipclist_fini(&ctx->lc_sessions, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to disable IPC client session list: %s",
			      strerror(err));

		return PFC_FALSE;
	}

	pfc_log_verbose("All IPC client sessions have been destroyed.");

	LNC_LOCK(ctx);
	taskq = ctx->lc_taskq;
	ctx->lc_taskq = PFC_TASKQ_INVALID_ID;
	LNC_UNLOCK(ctx);

	/* Destroy task queue. */
	PFC_ASSERT(taskq != PFC_TASKQ_INVALID_ID);
	err = pfc_taskq_destroy(taskq);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to destroy task queue: %s",
			      strerror(err));

		return PFC_FALSE;
	}

	/* Stop all daemons. */
	err = lnc_daemon_stop(UINT32_MAX, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Kill all children by force. */
		lnc_daemon_kill();

		if (PFC_EXPECT_FALSE(lnc_daemon_haschild())) {
			return PFC_FALSE;
		}
	}

	/* Finalize IPC services. */
	err = lnc_ipc_fini(ctx, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	/* Finalize launcher event. */
	err = lnc_event_fini(ctx, &limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	/* Destroy all daemon configurations. */
	lnc_daemon_destroy();

	LNC_LOCK(ctx);
	workdir = ctx->lc_workdir;
	ctx->lc_workdir = NULL;
	LNC_UNLOCK(ctx);

	pfc_refptr_put(workdir);

	return PFC_TRUE;
}

/*
 * static int
 * launcher_boot(void)
 *	Bootstrap function of the launcher module.
 *	This function will be called just after the launcher module is loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
launcher_boot(void)
{
	lnc_ctx_t	*ctx = &launcher_ctx;
	pfc_cfblk_t	options;
	pfc_evhandler_t	*hdlrp;
	const char	*workdir, *prop;
	int		err;

	prop = pfc_prop_get(LNC_PROP_PID);
	if (PFC_EXPECT_FALSE(prop != NULL)) {
		/*
		 * This process is spawned by the launcher module.
		 * Cascading process launch is not supported because it may
		 * cause catastrophic error.
		 */
		pfc_conf_error("launcher: This process is spawned by the "
			       "launcher module: pid=%s", prop);

		return EPERM;
	}

	PFC_ASSERT_INT(PFC_MUTEX_INIT(&ctx->lc_mutex), 0);
	PFC_ASSERT_INT(pfc_cond_init(&ctx->lc_cond), 0);
	clstat_ipclist_init(&ctx->lc_sessions);
	ctx->lc_ndaemons = 0;
	ctx->lc_flags = 0;
	ctx->lc_ncallbacks = 0;
	ctx->lc_taskq = PFC_TASKQ_INVALID_ID;
	for (hdlrp = ctx->lc_event_handler;
	     hdlrp < PFC_ARRAY_LIMIT(ctx->lc_event_handler); hdlrp++) {
		*hdlrp = EVHANDLER_ID_INVALID;
	}

	ctx->lc_clevent_start.tv_sec = 0;
	ctx->lc_clevent_start.tv_nsec = 0;
	ctx->lc_clevent_end.tv_sec = 0;
	ctx->lc_clevent_end.tv_nsec = 0;

	/* Fetch module configurations. */
	options = pfc_module_conf_getblock("options");
	ctx->lc_confdir = LNC_CONF_GET(options, string, conf_dir);
	workdir = LNC_CONF_GET(options, string, work_dir);
	LNC_GETCONF_TIMESPEC(options, event_timeout, &ctx->lc_event_timeout);
	LNC_GETCONF_TIMESPEC(options, clact_timeout, &ctx->lc_clact_timeout);
	LNC_GETCONF_TIMESPEC(options, startup_timeout,
			     &ctx->lc_startup_timeout);

	if (workdir == NULL) {
		const char	*workdir = pfc_prop_get(PFC_PROP_WORKDIR);

		/* Construct path to default logging directory. */
		ctx->lc_workdir = pfc_refptr_sprintf("%s/%s", workdir,
						     LNC_WORKDIR_DEFAULT);
	}
	else {
		ctx->lc_workdir = pfc_refptr_string_create(workdir);
	}

	if (PFC_EXPECT_FALSE(ctx->lc_workdir == NULL)) {
		pfc_conf_error("launcher: Failed to construct working "
			       "directory path.");

		return ENOMEM;
	}

	pfc_log_verbose("conf_dir=%s, work_dir=%s",
			ctx->lc_confdir, LNC_WORKDIR(ctx));

	/* Initialize cluster configuration. */
	err = lnc_cluster_boot(ctx, options);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Load daemon configurations. */
	err = lnc_daemon_boot();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	return 0;

error:
	pfc_refptr_put(ctx->lc_workdir);
	ctx->lc_workdir = NULL;
	PFC_ASSERT(err != 0);

	return err;
}

/* Declare C module. */
PFC_MODULE_IPC_DECL(launcher_init, launcher_fini, lnc_ipc_service,
		    LNC_IPC_NSERVICES);

/* Register bootstrap function. */
PFC_MODULE_BOOTSTRAP(launcher_boot);
