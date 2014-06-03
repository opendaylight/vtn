/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * libpfc.c - libpfc initialization and cleanup.
 */

#include <string.h>
#include <unistd.h>
#include <pfc/log.h>
#include <pfc/conf.h>
#include <pfc/clock.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <pfc/util.h>
#include <ipcsrv_impl.h>
#include <ipcclnt_impl.h>
#include <refptr_impl.h>
#include "thread_impl.h"
#include "event_impl.h"
#include "taskq_impl.h"
#include "module_impl.h"
#include "libpfc_impl.h"
#include "modevent_impl.h"
#include "property_impl.h"

/*
 * Program name.
 */
static const char	libpfc_progname_default[] = "pfcd";
static pfc_refptr_t	*libpfc_rprogname;
const char		*libpfc_progname PFC_ATTR_HIDDEN =
	libpfc_progname_default;

/*
 * Configuration block name for global options.
 */
const char	libpfc_conf_options[] PFC_ATTR_HIDDEN = "options";

const char	libpfc_conf_reap_interval[] PFC_ATTR_HIDDEN = "reap_interval";

/*
 * Periodic timer for libpfc.
 */
static pfc_ptimer_t	libpfc_ptimer = PFC_PTIMER_INVALID;

/*
 * Default interval, in seconds, of reaper thread.
 */
#define	LIBPFC_REAP_INTERVAL		(5 * 60)	/* 5 minutes */

/*
 * Read side of shutdown pipe.
 */
static int		shutdown_rdpipe = -1;

/*
 * Write side of shutdown pipe.
 */
static int		shutdown_wrpipe = -1;

/*
 * Shutdown flag.
 */
static volatile pfc_bool_t	shutdown_started = PFC_FALSE;

/*
 * Internal prototypes.
 */
static void	libpfc_ptimer_init(void);
static void	libpfc_ptimer_fini(void);
static void	libpfc_resource_reaper(void *unused);

#ifdef	PFC_USE_GLIBC

#include <malloc.h>

/*
 * Counter used to heap trimming.
 */
typedef struct {
	uint32_t	ht_count;	/* counter */
	uint32_t	ht_freq;	/* trim frequency */
	uint32_t	ht_retain;	/* size to be retained */
} heap_trim_t;

static heap_trim_t	reaper_heap_trim;

static const char	conf_heap_trim_freq[] = "heap_trim_freq";
static const char	conf_heap_trim_retain[] = "heap_trim_retain";

#define	LIBPFC_TRIM_HEAP_FREQ		PFC_CONST_U(12)
#define	LIBPFC_TRIM_HEAP_RETAIN		PFC_CONST_U(0x10000)

/*
 * static void
 * heap_trim_init(pfc_cfblk_t options)
 *	Initialize heap trimming counter.
 */
static void
heap_trim_init(pfc_cfblk_t options)
{
	heap_trim_t	*htp = &reaper_heap_trim;

	htp->ht_count = 0;
	htp->ht_freq = pfc_conf_get_uint32(options, conf_heap_trim_freq,
					   LIBPFC_TRIM_HEAP_FREQ);

	if (htp->ht_freq == 0) {
		pfc_log_info("Heap trimming is disabled.");

		return;
	}

	htp->ht_retain = pfc_conf_get_uint32(options, conf_heap_trim_retain,
					     LIBPFC_TRIM_HEAP_RETAIN);
	pfc_log_info("Heap trimming configuration: freq=%u, retain=0x%x",
		     htp->ht_freq, htp->ht_retain);
}

/*
 * static void
 * heap_trim_execute(void)
 *	Try to trim the size of process heap.
 */
static void
heap_trim_execute(void)
{
	heap_trim_t	*htp = &reaper_heap_trim;
	uint32_t	freq = htp->ht_freq;

	if (freq == 0) {
		return;
	}

	htp->ht_count++;
	if (htp->ht_count >= freq) {
		int	ret = malloc_trim(htp->ht_retain);

		htp->ht_count = 0;
		pfc_log_debug("Result of heap trimming: %s",
			      (ret == 0) ? "Failed" : "Succeeded");
	}
}

#else	/* !PFC_USE_GLIBC */

#define	heap_trim_init(options)		((void)0)
#define	heap_trim_execute()		((void)0)

#endif	/* PFC_USE_GLIBC */

/*
 * static void PFC_FATTR_INIT
 * libpfc_libinit(void)
 *	Constructor of libpfc library.
 */
static void PFC_FATTR_INIT
libpfc_libinit(void)
{
	/* Do early initialization of thread pool subsystem. */
	pfc_tpool_libinit();
}

/*
 * static void PFC_FATTR_FINI
 * libpfc_libfini(void)
 *	Destructor of libpfc library.
 */
static void PFC_FATTR_FINI
libpfc_libfini(void)
{
	pfc_prop_fini();
}

/*
 * void
 * libpfc_init(void)
 *	Bootstrap code of libpfc.
 *	This function must be called once at the bootstrap of PFC daemon.
 */
void
libpfc_init(void)
{
	int	fds[2];

	/* Create pipe which is used to notify shutdown. */
	if (PFC_EXPECT_FALSE(pfc_pipe_open(fds, PFC_PIPE_CLOEXEC_NB) != 0)) {
		pfc_log_fatal("Failed to create shutdown pipe: %s",
			      strerror(errno));
		/* NOTREACHED */
	}
	shutdown_rdpipe = fds[0];
	shutdown_wrpipe = fds[1];

	/* At first, we must call bootstrap code of thread pool layer. */
	pfc_tpool_bootstrap();

	/* Initialize event subsystem. */
	pfc_event_init();

	/* Initialize PFC module subsystem. */
	pfc_module_bootstrap();

	/* Initialize task queue */
	pfc_taskq_init();

	/* Initialize timer */
	pfc_timer_init();

	/* Initialize periodic timer. */
	libpfc_ptimer_init();
}

/*
 * void
 * libpfc_setprogname(pfc_refptr_t *rname)
 *	Set program name by a refptr string.
 *	This function does nothing if `rname' is NULL.
 */
void
libpfc_setprogname(pfc_refptr_t *rname)
{
	if (PFC_EXPECT_TRUE(rname != NULL)) {
		pfc_refptr_get(rname);
		libpfc_rprogname = rname;
		libpfc_progname = pfc_refptr_string_value(rname);
	}
}

/*
 * void
 * libpfc_fini(void)
 *	Destructor of libpfc.
 *	All features will be unusable after this function is called.
 */
void
libpfc_fini(void)
{
	pfc_refptr_t	*rname;
	int		fd;

	/* Call libpfc_shutdown_start() just for safe. */
	libpfc_shutdown_start();

	/* Destroy libpfc periodic timer. */
	libpfc_ptimer_fini();

	/* Shutdown timer. */
	pfc_timer_fini();

	/* Shutdown taskq. */
	pfc_taskq_fini();

	/* Shutdown the PFC module subsystem. */
	pfc_module_fini();

	/* Shutdown event system. */
	pfc_event_shutdown();

	/* Shutdown thread pools. */
	pfc_tpool_fini();

	/* Close read side of shutdown pipe. */
	fd = shutdown_rdpipe;
	if (fd != -1) {
		if (PFC_EXPECT_FALSE(close(fd) != 0)) {
			pfc_log_warn("Failed to close read side of shutdown "
				     "pipe: %s", strerror(errno));
		}
	}

	/* Reset program name. */
	libpfc_progname = libpfc_progname_default;
	rname = libpfc_rprogname;
	libpfc_rprogname = NULL;
	if (rname != NULL) {
		pfc_refptr_put(rname);
	}
}

/*
 * void
 * libpfc_shutdown_start(void)
 *	Notifies start of libpfc shutdown sequence.
 *	After the call of this function, libpfc_is_shutdown() will always
 *	return PFC_TRUE.
 */
void
libpfc_shutdown_start(void)
{
	int	fd;

	/* Turn the shutdown flag on. */
	shutdown_started = PFC_TRUE;

	fd = (int)pfc_atomic_swap_uint32((uint32_t *)&shutdown_wrpipe,
					 (uint32_t)-1);
	if (PFC_EXPECT_TRUE(fd != -1)) {
		/* Close write side of shutdown pipe. */
		if (PFC_EXPECT_FALSE(close(fd) != 0)) {
			pfc_log_fatal("Failed to close write side of "
				      "shutdown pipe: %s",
				      strerror(errno));
		}
	}
}

/*
 * int
 * libpfc_shutdown_getfd(void)
 *	Return file descriptor to receive shutdown notification.
 *	Once libpfc_fini() is called, read event is raised on returned
 *	file descriptor.
 *
 * Remarks:
 *	The caller must NOT close returned file descriptor, and must NOT read
 *	any data.
 */
int
libpfc_shutdown_getfd(void)
{
	return shutdown_rdpipe;
}

/*
 * pfc_bool_t
 * libpfc_is_shutdown(void)
 *	Determine whether libpfc shutdown has been started or not.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if libpfc shutdown sequence has been started.
 *	Otherwise PFC_FALSE is returned.
 */
pfc_bool_t
libpfc_is_shutdown(void)
{
	return shutdown_started;
}

/*
 * pfc_ptimer_t
 * libpfc_getptimer(void)
 *	Return global periodic timer handle for libpfc internal use.
 *
 * Calling/Exit State:
 *	Upon successful completion, global periodic timer handle is returned.
 *	PFC_PTIMER_INVALID is returned if it is not yet created.
 */
pfc_ptimer_t
libpfc_getptimer(void)
{
	return libpfc_ptimer;
}

/*
 * int
 * libpfc_post_sysevent(pfc_evtype_t type, const pfc_timespec_t *timeout)
 *	Post a global system event.
 *
 *	This function always wait for completion of event delivery.
 *	`timeout' specifies the maximum amount of time of the wait.
 *	Specifying NULL to `timeout' means infinite timeout.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
libpfc_post_sysevent(pfc_evtype_t type, const pfc_timespec_t *timeout)
{
	pfc_event_t	event;
	pfc_bool_t	emergency;
	pfc_ptr_t	data = NULL;
	const char	*evsrc = pfc_event_global_source();
	int		err;

	if (type == PFC_EVTYPE_SYS_STOP) {
		/* Post an emergency event if fatal error is logged. */
		emergency = pfc_log_isfatal();
		data = (pfc_ptr_t)(uintptr_t)emergency;
	}
	else {
		emergency = PFC_FALSE;
	}

	err = pfc_event_create(&event, type, data, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to generate global event: %u: %s",
			      type, strerror(err));

		return err;
	}

	/* Enable event delivery logging. */
	pfc_event_enable_log(event, PFC_TRUE);

	if (emergency) {
		err = pfc_event_post_emergency(evsrc, event);
	}
	else {
		err = pfc_event_post(evsrc, event);
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to deliver global event: %u: %s",
			      type, strerror(err));

		return err;
	}

	pfc_log_trace("System event has been posted: type=%u", type);

	/* We should wait for completion of system event delivery. */
	err = pfc_event_flush(evsrc, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ETIMEDOUT) {
			pfc_log_error("System event delivery seems to stuck: "
				      "type=%u", type);
		}
		else {
			pfc_log_error("System event sync failed: %s",
				      strerror(err));
		}

		/* FALLTHROUGH */
	}

	return err;
}

/*
 * static void
 * libpfc_ptimer_init(void)
 *	Initialize libpfc periodic timer.
 */
static void
libpfc_ptimer_init(void)
{
	pfc_cfblk_t	options = pfc_sysconf_get_block(libpfc_conf_options);
	pfc_ptimer_t	timer;
	pfc_ptmattr_t	attr;
	pfc_ptimeout_t	id;
	uint32_t	interval;
	int		err;

	/* Create libpfc periodic timer. */
	err = pfc_ptimer_create(&timer);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to create libpfc periodic timer: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	libpfc_ptimer = timer;

	/* Initialize heap trimming counter. */
	heap_trim_init(options);

	/* Register resource reaper as periodic timeout. */
	interval = pfc_conf_get_uint32(options, libpfc_conf_reap_interval,
				       LIBPFC_REAP_INTERVAL);
	attr.ptma_interval = interval * PFC_CLOCK_MILLISEC;
	attr.ptma_enabled = PFC_TRUE;
	err = pfc_ptimer_timeout(timer, libpfc_resource_reaper, NULL,
				 &attr, &id);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("Failed to register resource reaper: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	pfc_log_debug("Resource reaper: id=%u, interval=%u", id, interval);
}

/*
 * static void
 * libpfc_ptimer_fini(void)
 *	Destroy libpfc periodic timer.
 */
static void
libpfc_ptimer_fini(void)
{
	pfc_ptimer_t	timer;
	int		err;

	timer = (pfc_ptimer_t)pfc_atomic_swap_ptr((pfc_ptr_t *)&libpfc_ptimer,
						  PFC_PTIMER_INVALID);
	if (PFC_EXPECT_FALSE(timer == PFC_PTIMER_INVALID)) {
		return;
	}

	err = pfc_ptimer_destroy(timer);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("Failed to destroy libpfc periodic timer: %s",
			      strerror(err));
	}
}

/*
 * static void
 * libpfc_resource_reaper(void *unused PFC_ATTR_UNUSED)
 *	Reap unused resources.
 *	This function is invoked as periodic timer.
 */
static void
libpfc_resource_reaper(void *unused PFC_ATTR_UNUSED)
{
	pfc_log_verbose("Resource reaper is activated.");

	/* Reap free threads in thread pools. */
	pfc_tpool_reap();

	/* Reap unused IPC client connections in the connection pool. */
	pfc_ipcclnt_cpool_reap(PFC_FALSE);

	/* Try to trim process heap. */
	heap_trim_execute();
}
