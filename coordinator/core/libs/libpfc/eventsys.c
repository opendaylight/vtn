/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * eventsys.c - PFC event delivery system.
 */

/*
 * Use "event" as log identifier.
 */
static const char	log_ident[] = "event";
#undef	PFC_LOG_IDENT
#define	PFC_LOG_IDENT	log_ident

#include <string.h>
#include <pthread.h>
#include <pfc/log.h>
#include <pfc/listmodel.h>
#include <pfc/thread.h>
#include <pfc/conf.h>
#include "event_impl.h"
#include "thread_impl.h"

/*
 * This section describes about resource serialization in the event system.
 * All locks in the event system are listed below. Note that they are
 * described in lock order.
 *
 * = Global lock
 *   The event system has one rwlock to guard whole resources in the event
 *   system. The following resources must be serialized by this lock:
 *
 *   - Event source tree
 *     The Red-Black Tree which keeps pairs of event source name and event
 *     source instance (evsource_t) associated with the name.
 *
 *   - Event handler tree
 *     The Red-Black Tree which keeps pairs of event handler ID and event
 *     handler instance (evhandler_t) associated with the ID.
 *
 *   - Orphan handler tree
 *     The Red-Black Tree which keeps pairs of event source name and event
 *     handler list which are not yet bound to the event source.
 *
 * = Event source lock
 *   Each event source instance (evsource_t) has a rwlock. All contents of
 *   evsource_t must be serialized by this lock.
 *
 * = Event queue mutex
 *   Event queue instance (eventq_t) has a mutex. All contents of eventq_t
 *   must be serialized by this mutex. This mutex is also used to control
 *   behavior of event dispatch threads associated with the event queue.
 *
 * = Event handler mutex
 *   Event handler instance (evhandler_t) is created with one mutex when a
 *   event handler is registered to the event source. All contents of
 *   evhandler_t must be serialized by this mutex. evhandler_t has also
 *   condition variable, which is used to wait for the completion of event
 *   handler execution.
 */

/*
 * The global lock for the event system.
 */
static pfc_rwlock_t	event_system_lock = PFC_RWLOCK_INITIALIZER;

#define	EVENT_SYSTEM_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&event_system_lock), 0)
#define	EVENT_SYSTEM_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&event_system_lock), 0)
#define	EVENT_SYSTEM_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&event_system_lock), 0)

/*
 * Timeout, in seconds, of finalization of the PFC event delivery system.
 */
#define	EVENT_SHUTDOWN_TIMEOUT		PFC_CONST_U(5)

/*
 * Context to shutdown event delivery system.
 */
typedef struct {
	uint32_t	esd_nerrors;		/* number of errors */
	pfc_timespec_t	esd_abstime;		/* deadline of shutdown */
} event_sdctx_t;

/*
 * Determine whether the event delivery system is already finalized or not.
 */
static pfc_bool_t	event_disabled = PFC_FALSE;

/*
 * The global event dispatch queue.
 */
static eventq_t		*event_globalq;

/*
 * The asynchronous event dispatch queue.
 */
static eventq_t		*event_asyncq;

/*
 * Maximum number of threads in the global/async queue.
 */
#define	EVENT_MAXTHREADS_GLOBAL		1U
#define	EVENT_MAXTHREADS_ASYNC		8U

/*
 * Number of event dispatch threads for the debugging event.
 * The number of debugging event dispatch threads is fixed to 1.
 */
#define	EVENT_MAXTHREADS_DEBUG		1U

/*
 * Default event mask.
 */
static const pfc_evmask_t	event_mask_fill = (pfc_evmask_t)-1;

/*
 * Handler ID for the next allocation.
 */
static pfc_evhandler_t		evhandler_id_next = EVHANDLER_ID_INVALID + 1;

/*
 * How long, in seconds, the event dispatch thread should wait for a new event.
 * If a new event is not posted within this seconds, the thread quits.
 *
 * If timeout value is defined in the PFC system configuration file,
 * it will be used.
 */
#define	EVENT_THREAD_DEFAULT_TIMEOUT	10U	/* 10 seconds */

/*
 * Default number of threads in the local queue.
 */
#define	EVENT_THREAD_DEFAULT_NUM	1U

/*
 * Callback to watch removal of event handler.
 */
static evcall_hdr_t		evhandler_rm_callback;

/*
 * Keys for configuration file.
 */
static const char	event_conf[] = "event";
static const char	event_conf_async_nthreads[] = "async_nthreads";
static const char	event_conf_async_timeout[] = "async_timeout";
static const char	event_conf_log_force[] = "log_force";

static const char	event_map[] = "event_source";
static const char	event_map_nthreads[] = "nthreads";
static const char	event_map_timeout[] = "timeout";

/*
 * System event source name.
 */

/* Sequential events */
static const char	evsrc_global[] = "core";

/* Asynchronous events */
static const char	evsrc_async[] = "core_async";

/* Debug events */
static const char	evsrc_debug[] = "core_debug";

/*
 * Determine whether the given event source is system event source or not.
 */
#define	EVENT_IS_SYSTEM_SOURCE(src)					\
	((src) == evsrc_global || (src) == evsrc_async || (src) == evsrc_debug)

/*
 * Private data for event dispatch thread.
 * This is used to implement pfc_event_flush() and event handler removal check.
 *
 * evthread_t is assigned to all event dispatch threads, and it has a linked
 * list, et_sync, to keep sync event. et_sync is cleared and the reference
 * counter in the sync event is decremented every time the event dispatch
 * thread finishes delivering an event. If all references to the sync event
 * is removed, private data destructor, sync_event_dtor() will be called and
 * the waiter thread will be woken up.
 */
typedef struct {
	eventq_t	*et_queue;		/* event queue */
	pfc_listm_t	et_sync;		/* list of sync events */
	pfc_list_t	et_list;		/* link for thread list */
	evhandler_t	*et_handler;		/* current handler */
} evthread_t;

/*
 * TSD key for event dispatch threads.
 */
static pfc_tsd_key_t	evthread_key;

/*
 * Private data for synchronization event.
 * This is used to implement pfc_event_flush().
 *
 * The sync event instance, sync_event_t, is set as private data of sync event.
 * It has a condition variable, se_cond, and flag bits, se_flags.
 * pfc_event_flush() will wait for the flag to be set SEV_DONE bit.
 * If all references to the sync event is removed, sync_event_dtor() will be
 * called. sync_event_dtor() will set SEV_DONE bit and wake up waiter thread.
 *
 * See comments on sync_event_dtor() for more details.
 */
typedef struct {
	pfc_mutex_t		se_mutex;	/* mutex */
	pfc_cond_t		se_cond;	/* condition variable */
	volatile uint32_t	se_flags;	/* flags */
} sync_event_t;

/*
 * Flags for se_flags.
 */
#define	SEV_WAITING		0x1		/* waiting on this event */
#define	SEV_DONE		0x2		/* end of synchronization */

/*
 * Lock/Unlock macros for sync event.
 */
#define	SYNC_EVENT_LOCK(sep)	pfc_mutex_lock(&(sep)->se_mutex)
#define	SYNC_EVENT_UNLOCK(sep)	pfc_mutex_unlock(&(sep)->se_mutex)

/*
 * Condition variable macros for sync event.
 */
#define	SYNC_EVENT_TIMEDWAIT_ABS(sep, abstime)				\
	pfc_cond_timedwait_abs(&(sep)->se_cond, &(sep)->se_mutex, (abstime))
#define	SYNC_EVENT_SIGNAL(sep)	pfc_cond_signal(&(sep)->se_cond)

/*
 * Head of orphan event handler list.
 */
typedef struct {
	const char	*ol_name;		/* event source name */
	pfc_list_t	ol_list;		/* list of orphan handlers */
	pfc_rbnode_t	ol_node;		/* Red-Black Tree node */
} orphan_list_t;

#define	ORPHAN_NODE2PTR(node)					\
	PFC_CAST_CONTAINER((node), orphan_list_t, ol_node)

/*
 * static inline void
 * orphan_list_free(orphan_list_t *olp)
 *	Free the specified head of orphan handler list.
 *
 * Remarks:
 *	The caller must guarantee that no event handler is linked to the
 *	specified list.
 */
static inline void
orphan_list_free(orphan_list_t *olp)
{
	PFC_ASSERT(pfc_list_is_empty(&olp->ol_list));
	free((void *)olp->ol_name);
	free(olp);
}

#ifdef	PFC_VERBOSE_DEBUG

/*
 * Assertion to detect unexpected pthread_exit() in event handler.
 */

static void
eventq_cleanup(void *arg)
{
	evhandler_t	*ehp = (evhandler_t *)arg;
	evsource_t	*esrc = ehp->eh_source;

	pfc_log_error("%s: unexpected exit in event handler: %u, %p",
		      esrc->es_name, ehp->eh_id, ehp->eh_handler);
	abort();
}

#define	EVENTQ_DISPATCH_ASSERT_PUSH(ehp)		\
	pthread_cleanup_push(eventq_cleanup, (ehp))
#define	EVENTQ_DISPATCH_ASSERT_POP()		\
	pthread_cleanup_pop(0)

#else	/* !PFC_VERBOSE_DEBUG */
#define	EVENTQ_DISPATCH_ASSERT_PUSH(ehp)
#define	EVENTQ_DISPATCH_ASSERT_POP()
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * Boolean switch which determines whether event delivery logging is enabled
 * on all events or not.
 */
pfc_bool_t	event_log_force PFC_ATTR_HIDDEN;

#define	EVENT_LOG_FORCE_DEFAULT		PFC_FALSE

/*
 * Determine whether event delivery log for the given event should be
 * recorded or not.
 */
#define	EVENT_NEED_LOG(ev)		((ev)->e_flags & EVF_LOG)

/*
 * Pseudo name of anonymous event handler.
 */
#define	EVHANDLER_ANON_NAME		"<anonymous>"

/*
 * Evaluate name of event handler.
 */
#define	EVHANDLER_REFPTR_NAME(rname)			\
	(PFC_EXPECT_FALSE((rname) == NULL)		\
	 ? EVHANDLER_ANON_NAME				\
	 : pfc_refptr_string_value(rname))

#define	EVHANDLER_NAME(ehp)		EVHANDLER_REFPTR_NAME((ehp)->eh_name)

/*
 * Internal prototypes.
 */
static int	evsource_register(const char *PFC_RESTRICT name,
				  eventq_t *PFC_RESTRICT evq);
static int	evsource_teardown(evsource_t *PFC_RESTRICT esrc,
				  pfc_bool_t syslocked,
				  const pfc_timespec_t *PFC_RESTRICT abstime);
static void	evsource_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static void	evsource_add_handler(evsource_t *PFC_RESTRICT esrc,
				     evhandler_t *PFC_RESTRICT ehp);
static void	evsource_bind_orphan(evsource_t *PFC_RESTRICT esrc,
				     evhandler_t *PFC_RESTRICT ehp);

static pfc_cptr_t	evsource_getkey(pfc_rbnode_t *node);

static int	evhandler_add(pfc_evhandler_t *PFC_RESTRICT idp,
			      const char *PFC_RESTRICT name,
			      pfc_refptr_t *domain, pfc_evfunc_t handler,
			      pfc_ptr_t PFC_RESTRICT arg,
			      const pfc_evmask_t *PFC_RESTRICT evmask,
			      uint32_t priority,
			      pfc_refptr_t *rhname);
static int	evhandler_add_orphan(const char *PFC_RESTRICT name,
				     evhandler_t *PFC_RESTRICT ehp);
static void	evhandler_remove_orphan(evhandler_t *ehp);
static int	evhandler_wait(evhandler_t *PFC_RESTRICT ehp,
			       evsource_t *PFC_RESTRICT esrc,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static void	evhandler_set_rmfailed(evhandler_t *PFC_RESTRICT ehp,
				       evsource_t *PFC_RESTRICT esrc);
static int	evhandler_fini(pfc_list_t *PFC_RESTRICT handlers,
			       evsource_t *PFC_RESTRICT esrc,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static int	evhandler_free(pfc_list_t *PFC_RESTRICT handlers,
			       evsource_t *PFC_RESTRICT esrc,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static void	evhandler_destroy(evhandler_t *ehp);

static pfc_cptr_t	evhandler_getkey(pfc_rbnode_t *node);

static int	eventq_create(eventq_t **evqp, evqtype_t qtype,
			      uint32_t maxthreads, uint16_t timeout);
static int	eventq_destroy(eventq_t *PFC_RESTRICT evq,
			       const pfc_timespec_t *PFC_RESTRICT abstime);
static void	eventq_remove_source(eventq_t *PFC_RESTRICT evq,
				     evsource_t *PFC_RESTRICT esrc);
static int	eventq_post(const char *PFC_RESTRICT name,
			    const char *PFC_RESTRICT domain,
			    event_t *PFC_RESTRICT ev, pfc_bool_t emergency);
static int	eventq_enqueue(evsource_t *PFC_RESTRICT esrc,
			       eventq_t *PFC_RESTRICT evq,
			       const char *PFC_RESTRICT domain,
			       event_t *PFC_RESTRICT ev, pfc_bool_t emergency);
static int	eventq_enqueue_data(evsource_t *PFC_RESTRICT esrc,
				    eventq_t *PFC_RESTRICT evq,
				    event_t *PFC_RESTRICT ev,
				    evhandler_t *PFC_RESTRICT ehp,
				    pfc_bool_t emergency);
static evqdata_t	*eventq_filter(evthread_t *PFC_RESTRICT etp,
				       evqdata_t *PFC_RESTRICT qdp);
static evqdata_t	*eventq_dequeue(evthread_t *PFC_RESTRICT etp,
					const pfc_timespec_t *PFC_RESTRICT
					timeout);
static void	*eventq_main(void *arg);
static void	eventq_dispatch(evsource_t *PFC_RESTRICT esrc,
				evhandler_t *PFC_RESTRICT ehp,
				event_t *PFC_RESTRICT ev,
				evthread_t *PFC_RESTRICT etp);
static void	eventq_start(eventq_t *PFC_RESTRICT evq,
			     evqdata_t *PFC_RESTRICT qdp);
static void	eventq_log(evsource_t *PFC_RESTRICT esrc,
			   evhandler_t *PFC_RESTRICT ehp,
			   event_t *PFC_RESTRICT ev,
			   const char *PFC_RESTRICT message);

static int	sync_event_create(pfc_event_t *eventp);
static void	sync_event_dtor(pfc_ptr_t data);
static void	sync_event_free(sync_event_t *sep);

static pfc_cptr_t	orphan_getkey(pfc_rbnode_t *node);

/*
 * Red-Black Tree which keeps pairs of event source name and event source
 * instance.
 */
static pfc_rbtree_t	event_sources =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, evsource_getkey);

/*
 * Red-Black Tree which keeps pairs of event handler ID and event handler
 * instance.
 */
static pfc_rbtree_t	event_handlers =
	PFC_RBTREE_INITIALIZER(pfc_rbtree_uint32_compare, evhandler_getkey);

/*
 * Red-Black Tree which keeps event handlers which are not yet bound to the
 * event source.
 */
static pfc_rbtree_t	event_orphans =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, orphan_getkey);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * evsource_free(evsource_t *esrc)
 *	Free the specified event source instance.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
evsource_free(evsource_t *esrc)
{
	if (!EVENT_IS_SYSTEM_SOURCE(esrc->es_name)) {
		free((void *)esrc->es_name);
	}
	free(esrc);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * evsource_hold(evsource_t *esrc)
 *	Hold the event source to protect against deletion.
 *
 * Remarks:
 *	This function must be called with holding event source lock in
 *	writer mode.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
evsource_hold(evsource_t *esrc)
{
	uint32_t	hold = esrc->es_hold;

	PFC_ASSERT(hold != 0);
	esrc->es_hold = hold + 1;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * evsource_release(evsource_t *esrc)
 *	Release the event source held by the call of evsource_hold().
 *
 * Remarks:
 *	This function must be called with holding event source lock in
 *	writer mode. Note that the event source lock is released on return.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
evsource_release(evsource_t *esrc)
{
	uint32_t	hold = esrc->es_hold;

	if (hold == 1) {
		/* Destroy this event source. */
		EVSOURCE_UNLOCK(esrc);
		PFC_ASSERT_INT(pfc_rwlock_destroy(&esrc->es_lock), 0);
		evsource_free(esrc);
	}
	else {
		PFC_ASSERT(hold != 0);
		esrc->es_hold = hold - 1;
		EVSOURCE_UNLOCK(esrc);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_event_init(void)
 *	Initialize the PFC event delivery system.
 */
void PFC_ATTR_HIDDEN
pfc_event_init(void)
{
	int		err;
	pfc_cfblk_t	cf;
	uint32_t	nthreads, timeout;

	/* Create global queue. */
	err = eventq_create(&event_globalq, EVQTYPE_GLOBAL,
			    EVENT_MAXTHREADS_GLOBAL, 0);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to create event global queue: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	/*
	 * Create async queue.
	 * Number of async thread and timeout value for dispatch threads are
	 * configurable.
	 */
	cf = pfc_sysconf_get_block(event_conf);
	nthreads = pfc_conf_get_uint32(cf, event_conf_async_nthreads,
				       EVENT_MAXTHREADS_ASYNC);
	timeout = pfc_conf_get_uint32(cf, event_conf_async_timeout,
				      EVENT_THREAD_DEFAULT_TIMEOUT);
	PFC_ASSERT(timeout <= UINT16_MAX);
	err = eventq_create(&event_asyncq, EVQTYPE_ASYNC, nthreads, timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to create event async queue: %s",
			      strerror(err));
		/* NOTREACHED */
	}

	pfc_log_verbose("async: nthreads=%u, timeout=%u", nthreads,
			timeout);

	/* Determine event delivery logging configuration. */
	event_log_force = pfc_conf_get_bool(cf, event_conf_log_force,
					    EVENT_LOG_FORCE_DEFAULT);
	if (event_log_force) {
		pfc_log_notice("Delivery logging is enabled by force.");
	}

	/* Register PFC system event sources. */
	err = pfc_event_register(evsrc_global);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to register system event source: "
			      "%s", strerror(err));
		/* NOTREACHED */
	}

	err = pfc_event_register_async(evsrc_async);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to register async system event "
			      "source: %s", strerror(err));
		/* NOTREACHED */
	}

	err = pfc_event_register_local(evsrc_debug, EVENT_MAXTHREADS_DEBUG);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to register debug system event "
			      "source: %s", strerror(err));
		/* NOTREACHED */
	}

	/* Create TSD key for event dispatch threads. */
	err = pfc_thread_key_create(&evthread_key, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_fatal("failed to create evthread TSD key: %s",
			      strerror(err));
		/* NOTREACHED */
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_event_shutdown(void)
 *	Shut down the PFC event system.
 *	After this call, any incoming event is discarded.
 */
void PFC_ATTR_HIDDEN
pfc_event_shutdown(void)
{
	pfc_rbtree_t	tree;
	pfc_rbnode_t	*node;
	event_sdctx_t	ctx;
	int		err;

	EVENT_SYSTEM_WRLOCK();

	event_disabled = PFC_TRUE;

	/* Shutdown event queues for system event sources. */
	node = NULL;
	while ((node = pfc_rbtree_next(&event_sources, node)) != NULL) {
		evsource_t	*esrc = EVSOURCE_NODE2PTR(node);
		eventq_t	*evq;

		EVSOURCE_RDLOCK(esrc);

		evq = esrc->es_queue;
		if (PFC_EXPECT_TRUE(evq != NULL)) {
			if (evq->eq_type == EVQTYPE_LOCAL) {
				EVENTQ_LOCK(evq);
				EVENTQ_SHUTDOWN(evq);
				EVENTQ_UNLOCK(evq);
			}
			else {
				PFC_ASSERT(evq->eq_type == EVQTYPE_GLOBAL ||
					   evq->eq_type == EVQTYPE_ASYNC);
			}
		}

		EVSOURCE_UNLOCK(esrc);
	}

	EVENTQ_LOCK(event_globalq);
	EVENTQ_SHUTDOWN(event_globalq);
	EVENTQ_UNLOCK(event_globalq);

	EVENTQ_LOCK(event_asyncq);
	EVENTQ_SHUTDOWN(event_asyncq);
	EVENTQ_UNLOCK(event_asyncq);

	/* Copy event sources, and clear global event source tree. */
	tree = event_sources;
	pfc_rbtree_init(&event_sources, (pfc_rbcomp_t)strcmp, evsource_getkey);

	EVENT_SYSTEM_UNLOCK();

	/* Try to destroy all event sources. */
	ctx.esd_nerrors = 0;
	PFC_ASSERT_INT(pfc_clock_gettime(&ctx.esd_abstime), 0);
	ctx.esd_abstime.tv_sec += EVENT_SHUTDOWN_TIMEOUT;
	pfc_rbtree_clear(&tree, evsource_dtor, &ctx);

	if (PFC_EXPECT_FALSE(ctx.esd_nerrors != 0)) {
		return;
	}

	/* Try to destroy system event queues. */
	err = eventq_destroy(event_asyncq, &ctx.esd_abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("failed to destroy event async queue: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	err = eventq_destroy(event_globalq, &ctx.esd_abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("failed to destroy event global queue: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}

	err = pfc_thread_key_delete(evthread_key);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("failed to delete evthread TSD key: %s",
			      strerror(err));
		/* FALLTHROUGH */
	}
}

/*
 * int
 * pfc_event_register(const char *name)
 *	Register event source with system global event queue.
 *	`name' is a string used as identifier of event source.
 *
 *	This function assigns the system global queue for the event source.
 *	So all events posted to the event source registered by this function
 *	are delivered by one system global thread.
 *
 *	If some event handlers are already registered to the given event source
 *	name, they will be bound to the event source registered by this
 *	function.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_register(const char *name)
{
	return evsource_register(name, event_globalq);
}

/*
 * int
 * pfc_event_register_local(const char *name, uint32_t maxthreads)
 *	Register event source with event source local queue.
 *	`name' is a string used as identifier of event source.
 *
 *	This function creates a new event queue for a new event source.
 *	`maxthreads' is the maximum number of event dispatch threads.
 *	Note that it is impossible to control order of calling event handler
 *	if you specify more than 1 to `maxthreads'.
 *
 *	If `maxthreads' is zero, the number of threads in the queue is
 *	determined by the system.
 *
 *	If some event handlers are already registered to the given event source
 *	name, they will be bound to the event source registered by this
 *	function.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_register_local(const char *name, uint32_t maxthreads)
{
	eventq_t	*evq;
	pfc_cfblk_t	conf;
	uint32_t	timeout;
	int	err;

	conf = pfc_sysconf_get_map(event_map, name);

	if (maxthreads == 0) {
		/* Use default value. */
		maxthreads = pfc_conf_get_uint32(conf, event_map_nthreads,
						 EVENT_THREAD_DEFAULT_NUM);
	}

	timeout = pfc_conf_get_uint32(conf, event_map_timeout,
				      EVENT_THREAD_DEFAULT_TIMEOUT);

	pfc_log_verbose("local: %s: nthreads=%u, timeout=%u",
			name, maxthreads, timeout);

	/* Create a new event queue. */
	err = eventq_create(&evq, EVQTYPE_LOCAL, maxthreads, timeout);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = evsource_register(name, evq);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT_INT(eventq_destroy(evq, NULL), 0);
		}
	}

	return err;
}

/*
 * int
 * pfc_event_register_async(const char *name)
 *	Register event source with async event queue.
 *	`name' is a string used as identifier of event source.
 *
 *	This function assigns the async global queue for the event source.
 *	All events posted to the event source registered by this function
 *	are delivered asynchronously. It is impossible to control order of
 *	calling event handler.
 *
 *	If some event handlers are already registered to the given event source
 *	name, they will be bound to the event source registered by this
 *	function.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_register_async(const char *name)
{
	return evsource_register(name, event_asyncq);
}

/*
 * int
 * pfc_event_unregister(const char *PFC_RESTRICT name,
 *			const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Unregister event source.
 *	All events in the event queue are discarded, and all handlers
 *	registered to this source are also unregistered.
 *
 *	If the event source is busy, this function will block the calling
 *	thread. If `timeout' is NULL, the current thread waits forever.
 *	If non-NULL, it waits within the specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event source may not be able to reuse even if this function
 *	returns error.
 */
int
pfc_event_unregister(const char *PFC_RESTRICT name,
		     const pfc_timespec_t *PFC_RESTRICT timeout)
{
	evsource_t	*esrc;
	pfc_rbnode_t	*node;
	pfc_timespec_t	tspec, *abstime;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	EVENT_SYSTEM_WRLOCK();

	/* Obtain event source instance associated with the given name. */
	node = pfc_rbtree_get(&event_sources, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		EVENT_SYSTEM_UNLOCK();

		return ENOENT;
	}
	esrc = EVSOURCE_NODE2PTR(node);

	/*
	 * Tear down the event source.
	 * The system global lock is released by evsource_teardown().
	 */
	err = evsource_teardown(esrc, PFC_TRUE, abstime);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Remove the event source from the event source tree. */
		EVENT_SYSTEM_WRLOCK();
		pfc_rbtree_remove_node(&event_sources, &esrc->es_node);
		EVENT_SYSTEM_UNLOCK();

		/* Release the event source. */
		EVSOURCE_WRLOCK(esrc);
		evsource_release(esrc);
	}

	return err;
}

/*
 * int
 * pfc_event_add_handler_named(pfc_evhandler_t *PFC_RESTRICT idp,
 *			       const char *PFC_RESTRICT name,
 *			       pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
 *			       const pfc_evmask_t *PFC_RESTRICT evmask,
 *			       uint32_t priority,
 *			       const char *PFC_RESTRICT hname)
 *	Add an event handler which receives events posted to the event source
 *	specified by the name.
 *
 *	`hname' is a user-defined name of event handler.
 *	Currently, it is used only for event delivery logging.
 *	NULL means an anonymous handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function succeeds even if the specified event source is not yet
 *	registered. In that case, the handler will be bound to the event
 *	source when it is registered.
 */
int
pfc_event_add_handler_named(pfc_evhandler_t *PFC_RESTRICT idp,
			    const char *PFC_RESTRICT name,
			    pfc_evfunc_t handler, pfc_ptr_t PFC_RESTRICT arg,
			    const pfc_evmask_t *PFC_RESTRICT evmask,
			    uint32_t priority, const char *PFC_RESTRICT hname)
{
	pfc_refptr_t	*rhname;
	int		err;

	/* Copy user-defined handler's name. */
	if (hname == NULL || *hname == '\0') {
		rhname = NULL;
	}
	else {
		rhname = pfc_refptr_string_create(hname);
		if (PFC_EXPECT_FALSE(rhname == NULL)) {
			return ENOMEM;
		}
	}

	err = evhandler_add(idp, name, NULL, handler, arg, evmask, priority,
			    rhname);
	if (rhname != NULL) {
		pfc_refptr_put(rhname);
	}

	return err;
}

/*
 * int
 * pfc_event_remove_handler(pfc_evhandler_t id, const pfc_timespec_t *timeout)
 *	Remove the event handler associated with the specified handler ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event handler may not be able to reuse even if this function
 *	returns error, except for EINVAL.
 */
int
pfc_event_remove_handler(pfc_evhandler_t id, const pfc_timespec_t *timeout)
{
	evhandler_t	*ehp;
	evsource_t	*esrc;
	pfc_rbnode_t	*node;
	pfc_list_t	handlers;
	pfc_timespec_t	tspec, *abstime;
	uint8_t		state;
	int		err;

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	EVENT_SYSTEM_WRLOCK();

	/* At first, invalidate event handler ID. */
	node = pfc_rbtree_remove(&event_handlers, EVHANDLER_KEY(id));
	if (PFC_EXPECT_FALSE(node == NULL)) {
		EVENT_SYSTEM_UNLOCK();

		return ENOENT;
	}
	ehp = EVHANDLER_NODE2PTR(node);
	PFC_ASSERT(id == ehp->eh_id);

	/* Acquire event handler's lock before unlocking global lock. */
	EVHANDLER_LOCK(ehp);

	state = ehp->eh_state;
	if (PFC_EXPECT_FALSE(EVH_STATE_IS_REMOVED(state))) {
		/* Another thread is trying to remove this handler. */
		EVHANDLER_UNLOCK(ehp);
		EVENT_SYSTEM_UNLOCK();

		return EBUSY;
	}

	if (EVH_STATE_IS_ORPHAN(state)) {
		/* This handler is not yet bound to the event source. */
		evhandler_remove_orphan(ehp);

		return 0;
	}

	/* Acquire event source lock. */
	esrc = ehp->eh_source;
	EVHANDLER_UNLOCK(ehp);
	EVSOURCE_WRLOCK(esrc);
	EVHANDLER_LOCK(ehp);

	/* Check REMOVED flag again.  */
	state = ehp->eh_state;
	if (PFC_EXPECT_FALSE(EVH_STATE_IS_REMOVED(state))) {
		EVHANDLER_UNLOCK(ehp);
		EVSOURCE_UNLOCK(esrc);
		EVENT_SYSTEM_UNLOCK();

		return EBUSY;
	}
	PFC_ASSERT(!EVH_STATE_IS_ORPHAN(state));
	PFC_ASSERT(esrc->es_queue != NULL);

	/* Set REMOVED flag to this handler. */
	ehp->eh_state = (state | EVH_STATE_REMOVED);
	EVHANDLER_UNLOCK(ehp);
	EVENT_SYSTEM_UNLOCK();

	/*
	 * Remove this handler from the event source.
	 * From here, this handler is never posted to the event queue.
	 */
	pfc_list_remove(&ehp->eh_list);

	/* Finalize this event handler instance. */
	pfc_list_init(&handlers);
	pfc_list_push(&handlers, &ehp->eh_list);

	return evhandler_fini(&handlers, esrc, abstime);
}

/*
 * int
 * pfc_event_post(const char *name, pfc_event_t event)
 *	Post an event to the event source associated with the specified name.
 *	The lifetime of the specified event is managed by the event system.
 *
 *	The reference to the specified event is always decremented,
 *	irrespective of the results of this function. You must not touch the
 *	specified event object after this call.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_post(const char *name, pfc_event_t event)
{
	return eventq_post(name, NULL, PFC_EVENT_PTR(event), PFC_FALSE);
}

/*
 * int
 * pfc_event_post_emergency(const char *name, pfc_event_t event)
 *	Post an emergency event to the event source associated with the
 *	specified name.
 *
 *	Unlike pfc_event_post(), this function posts an event to the head
 *	of event queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_post_emergency(const char *name, pfc_event_t event)
{
	return eventq_post(name, NULL, PFC_EVENT_PTR(event), PFC_TRUE);
}

/*
 * int
 * pfc_event_flush(const char *PFC_RESTRICT name,
 *		   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Flush event queue for associated with the specified event source.
 *
 *	If this function returns zero, it is guaranteed that all events posted
 *	to the specified event source before the call of pfc_event_flush()
 *	have been delivered to event handlers.
 *
 *	If `timeout' is NULL, pfc_event_flush() waits until all events are
 *	flushed. If not NULL, pfc_event_flush() only waits the specified
 *	timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_event_flush(const char *PFC_RESTRICT name,
		const pfc_timespec_t *PFC_RESTRICT timeout)
{
	pfc_event_t	event;
	sync_event_t	*sep;
	event_t		*ev;
	pfc_timespec_t	tspec, *abstime;
	pfc_bool_t	tout = PFC_FALSE;
	int	err;

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	/* Create a dummy event object for synchronization. */
	err = sync_event_create(&event);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	ev = PFC_EVENT_PTR(event);
	sep = (sync_event_t *)ev->e_data;

	/* Post a sync event. */
	err = eventq_post(name, NULL, ev, PFC_FALSE);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/*
		 * Destructor of sync event, sync_event_dtor() uses a dirty
		 * trick. If pfc_event_post() gets error, event object will be
		 * released. But sync event instance, a private data of sync
		 * event, will not. This behavior is required to block the
		 * calling thread using sync event instance after the sync
		 * event is released.
		 *
		 * That is why sync event instance still exists though sync
		 * event has been destroyed. We must free it here.
		 */
		SYNC_EVENT_LOCK(sep);

		/* sync_event_dtor() must have been called. */
		PFC_ASSERT(sep->se_flags == (SEV_WAITING | SEV_DONE));
		sync_event_free(sep);

		return err;
	}

	/* Wait for this sync event has been delivered. */
	SYNC_EVENT_LOCK(sep);
	while (1) {
		if (sep->se_flags & SEV_DONE) {
			/*
			 * The sync event instance has been successfully
			 * released. This thread is responsible for destroying
			 * sync event instance.
			 */
			err = 0;
			sync_event_free(sep);
			break;
		}

		if (tout) {
			/*
			 * At least one event dispatch thread holds the sync
			 * event. So this sync event instance must not be
			 * destroyed here. Event dispatch thread will destroy
			 * later.
			 */
			PFC_ASSERT(err != 0);
			sep->se_flags = 0;
			SYNC_EVENT_UNLOCK(sep);
			break;
		}

		err = SYNC_EVENT_TIMEDWAIT_ABS(sep, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* One more check should be done. */
			tout = PFC_TRUE;
		}
	}

	return err;
}

/*
 * const char *
 * pfc_event_global_source(void)
 *	Return source name of PFC system global events.
 *
 *	Events posted to this event source are delivered by the global queue.
 *	So all events are delivered by the global single thread.
 */
const char *
pfc_event_global_source(void)
{
	return evsrc_global;
}

/*
 * const char *
 * pfc_event_async_source(void)
 *	Return source name of PFC asynchronous system events.
 *
 *	Events posted to this event source are delivered by the async queue.
 *	So the order of calling event handlers is undefined.
 */
const char *
pfc_event_async_source(void)
{
	return evsrc_async;
}

/*
 * const char *
 * pfc_event_debug_source(void)
 *	Return source name of PFC debugging events.
 *
 *	Events posted to this event source are delivered by the local queue
 *	which contains one thread. So event handlers are called in priority
 *	order.
 */
const char *
pfc_event_debug_source(void)
{
	return evsrc_debug;
}

/*
 * void
 * pfc_event_register_evcall(evcall_hdr_t callback)
 *	Register callback function to watch removal of event handler.
 *	This function is for system internal use.
 */
void
pfc_event_register_evcall(evcall_hdr_t callback)
{
	PFC_ASSERT(evhandler_rm_callback == NULL);

	evhandler_rm_callback = callback;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_event_domain_add_handler(pfc_evhandler_t *PFC_RESTRICT idp,
 *				const char *PFC_RESTRICT name,
 *				pfc_refptr_t *domain, pfc_evfunc_t handler,
 *				pfc_ptr_t PFC_RESTRICT arg,
 *				const pfc_evmask_t *PFC_RESTRICT evmask,
 *				uint32_t priority, pfc_refptr_t *rhname)
 *	Add an event handler with specifying event delivery domain.
 *
 *	`domain' must be a non-NULL pointer to refptr string which keeps
 *	the name of delivery domain.
 *
 *	`rhname' is a string refptr which represents user-defined name of
 *	event handler. Currently, it is used only for event delivery logging.
 *	If NULL is specified, domain name `name' is used as handler's name.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `*idp',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function is not public API.
 *
 *	- This function succeeds even if the specified event source is not yet
 *	  registered. In that case, the handler will be bound to the event
 *	  source when it is registered.
 */
int PFC_ATTR_HIDDEN
pfc_event_domain_add_handler(pfc_evhandler_t *PFC_RESTRICT idp,
			     const char *PFC_RESTRICT name,
			     pfc_refptr_t *domain, pfc_evfunc_t handler,
			     pfc_ptr_t PFC_RESTRICT arg,
			     const pfc_evmask_t *PFC_RESTRICT evmask,
			     uint32_t priority, pfc_refptr_t *rhname)
{
	int	err;

	if (PFC_EXPECT_FALSE(domain == NULL)) {
		return EINVAL;
	}

	pfc_refptr_get(domain);
	err = evhandler_add(idp, name, domain, handler, arg, evmask, priority,
			    rhname);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_refptr_put(domain);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_event_domain_remove_handler(const char *name, const char *domain,
 *				   const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Remove all event handlers associated with the specified domain in the
 *	event source specified by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The event handler may not be able to reuse even if this function
 *	returns error, except for EINVAL.
 */
int PFC_ATTR_HIDDEN
pfc_event_domain_remove_handler(const char *name, const char *domain,
				const pfc_timespec_t *PFC_RESTRICT timeout)
{
	evsource_t	*esrc;
	pfc_list_t	handlers, *elem, *next;
	pfc_rbtree_t	*tree = &event_handlers;
	pfc_rbnode_t	*node;
	pfc_timespec_t	tspec, *abstime;

	if (PFC_EXPECT_FALSE(name == NULL || domain == NULL)) {
		return EINVAL;
	}

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		int	err;

		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	/* Obtain event source instance associated with the given name. */
	EVENT_SYSTEM_WRLOCK();
	node = pfc_rbtree_get(&event_sources, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		EVENT_SYSTEM_UNLOCK();

		return ENOENT;
	}

	esrc = EVSOURCE_NODE2PTR(node);
	EVSOURCE_WRLOCK(esrc);

	if (PFC_EXPECT_FALSE(esrc->es_queue == NULL)) {
		/* Another thread is trying to remove this event source. */
		EVSOURCE_UNLOCK(esrc);
		EVENT_SYSTEM_UNLOCK();

		return EBUSY;
	}

	/*
	 * Unlink handlers associated with the given domain.
	 * From here, any event is never delivered to handlers associated
	 * with the given domain.
	 */
	pfc_list_init(&handlers);
	PFC_LIST_FOREACH_SAFE(&esrc->es_handler, elem, next) {
		evhandler_t	*ehp = EVHANDLER_CAST(elem);
		pfc_refptr_t	*rdomain = ehp->eh_domain;

		if (rdomain != NULL &&
		    strcmp(pfc_refptr_string_value(rdomain), domain) == 0) {
			uint8_t	state;

			/* Unlink this handler unless it is already removed. */
			EVHANDLER_LOCK(ehp);
			state = ehp->eh_state;
			if (!EVH_STATE_IS_REMOVED(state)) {
				ehp->eh_state = (state | EVH_STATE_REMOVED);
				pfc_list_remove(elem);
				pfc_list_push_tail(&handlers, elem);

				/* Invalidate event handler ID. */
				pfc_log_debug("Removing event domain handler: "
					      "domain=%s, id=%u", domain,
					      ehp->eh_id);
				pfc_rbtree_remove_node(tree, &ehp->eh_node);
			}
			EVHANDLER_UNLOCK(ehp);
		}
	}
	EVENT_SYSTEM_UNLOCK();

	if (pfc_list_is_empty(&handlers)) {
		/* No handler is associated with the given domain. */
		EVSOURCE_UNLOCK(esrc);

		return ENOENT;
	}

	/* Finalize all event handlers associated with the given domain. */
	return evhandler_fini(&handlers, esrc, abstime);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_event_domain_post(const char *name, const char *domain,
 *			 pfc_event_t event)
 *	Post an event to the event source associated with `name' with
 *	specifying event delivery domain.
 *
 *	If a non-NULL value is specified to `domain', the event specified by
 *	`event' will be posted to only handlers associated with the domain
 *	specified by `domain'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The reference to the specified event is always decremented.
 */
int PFC_ATTR_HIDDEN
pfc_event_domain_post(const char *name, const char *domain, pfc_event_t event)
{
	return eventq_post(name, domain, PFC_EVENT_PTR(event), PFC_FALSE);
}

/*
 * static int
 * evsource_register(const char *PFC_RESTRICT name, eventq_t *PFC_RESTRICT evq)
 *	Register event source which has the specified name and event queue.
 */
static int
evsource_register(const char *PFC_RESTRICT name, eventq_t *PFC_RESTRICT evq)
{
	evsource_t	*esrc;
	pfc_rbnode_t	*node;
	int	err;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		return EINVAL;
	}

	/* Allocate event source instance. */
	esrc = (evsource_t *)malloc(sizeof(*esrc));
	if (PFC_EXPECT_FALSE(esrc == NULL)) {
		return ENOMEM;
	}

	err = pfc_rwlock_init(&esrc->es_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(esrc);

		return err;
	}

	/* Create copy of event source name. */
	if (EVENT_IS_SYSTEM_SOURCE(name)) {
		esrc->es_name = name;
	}
	else {
		esrc->es_name = strdup(name);
		if (PFC_EXPECT_FALSE(esrc->es_name == NULL)) {
			free(esrc);

			return ENOMEM;
		}
	}

	pfc_list_init(&esrc->es_handler);
	esrc->es_queue = evq;
	esrc->es_hold = 1;

	EVENT_SYSTEM_WRLOCK();

	if (PFC_EXPECT_FALSE(event_disabled)) {
		EVENT_SYSTEM_UNLOCK();
		evsource_free(esrc);

		pfc_log_error("%s: event delivery system is already "
			      "finalized.", name);

		return ESHUTDOWN;
	}

	/* Register event source. */
	err = pfc_rbtree_put(&event_sources, &esrc->es_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		EVENT_SYSTEM_UNLOCK();
		evsource_free(esrc);

		pfc_log_error("%s: failed to register event source: %s",
			      name, strerror(err));

		return err;
	}

	/*
	 * Check to see whether there are event handlers which should be bound
	 * to this source.
	 */
	node = pfc_rbtree_remove(&event_orphans, name);
	if (PFC_EXPECT_FALSE(node != NULL)) {
		orphan_list_t	*olp = ORPHAN_NODE2PTR(node);
		pfc_list_t	*elem;

		/* Acquire event source lock. */
		EVSOURCE_WRLOCK(esrc);

		/* Bind handlers to this source. */
		while ((elem = pfc_list_pop(&olp->ol_list)) != NULL) {
			evhandler_t	*ehp = EVHANDLER_CAST(elem);

			evsource_bind_orphan(esrc, ehp);
		}
		EVSOURCE_UNLOCK(esrc);

		orphan_list_free(olp);
	}

	pfc_log_debug("%s: new event source: qtype=%d",
		      name, evq->eq_type);

	EVENT_SYSTEM_UNLOCK();

	return 0;
}

/*
 * static int
 * evsource_teardown(evsource_t *PFC_RESTRICT esrc, pfc_bool_t syslocked,
 *		     const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Free up all resources held by the given event source.
 *
 *	The caller must pass PFC_TRUE to `syslocked' if it calls this function
 *	with holding the system global lock. Note that this function always
 *	releases the lock on return.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
evsource_teardown(evsource_t *PFC_RESTRICT esrc, pfc_bool_t syslocked,
		  const pfc_timespec_t *PFC_RESTRICT abstime)
{
	eventq_t	*evq;
	pfc_list_t	handlers, *elem;
	int		err;

	/* Acquire event source lock. */
	EVSOURCE_WRLOCK(esrc);

	evq = esrc->es_queue;
	if (PFC_EXPECT_FALSE(evq == NULL)) {
		/* Another thread is trying to remove this event source. */
		EVSOURCE_UNLOCK(esrc);
		if (syslocked) {
			EVENT_SYSTEM_UNLOCK();
		}

		return EBUSY;
	}

	/*
	 * Disable event queue.
	 * From here, we can't rollback on error.
	 */
	esrc->es_queue = NULL;

	/* Unlink all event handlers in this source. */
	pfc_list_move_all(&esrc->es_handler, &handlers);
	pfc_list_init(&esrc->es_handler);

	/*
	 * Set REMOVED flag to all event handlers, and make them invisible.
	 * All event handler IDs associated with this event source must be
	 * removed here or pfc_event_remove_handler() may access removed event
	 * handler instance.
	 */
	PFC_LIST_FOREACH(&handlers, elem) {
		evhandler_t	*ehp = EVHANDLER_CAST(elem);
		uint8_t		state;

		pfc_rbtree_remove_node(&event_handlers, &ehp->eh_node);

		EVHANDLER_LOCK(ehp);
		state = ehp->eh_state;
		PFC_ASSERT(!EVH_STATE_IS_REMOVED(state));
		ehp->eh_state = (state | EVH_STATE_REMOVED);
		EVHANDLER_UNLOCK(ehp);
	}

	EVSOURCE_UNLOCK(esrc);
	if (syslocked) {
		EVENT_SYSTEM_UNLOCK();
	}

	if (evq->eq_type == EVQTYPE_LOCAL) {
		/* Destroy local event queue. */
		PFC_ASSERT(evq != event_globalq && evq != event_asyncq);
		err = eventq_destroy(evq, abstime);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		/* Remove all queue data which contains this event source. */
		eventq_remove_source(evq, esrc);
	}

	/* Destroy all event handlers in this source. */
	return evhandler_free(&handlers, esrc, abstime);
}

/*
 * static void
 * evsource_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
 *	Destructor of event source node.
 */
static void
evsource_dtor(pfc_rbnode_t *node, pfc_ptr_t arg)
{
	event_sdctx_t	*ctx = (event_sdctx_t *)arg;
	evsource_t	*esrc;
	int		err;

	esrc = EVSOURCE_NODE2PTR(node);

	/* Tear down the event source. */
	err = evsource_teardown(esrc, PFC_FALSE, &ctx->esd_abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("%s: failed to tear down event source: %s",
			      esrc->es_name, strerror(err));
		ctx->esd_nerrors++;
	}
	else {
		const char	*name = esrc->es_name;

		/* Release the event source. */
		if (!EVENT_IS_SYSTEM_SOURCE(name)) {
			pfc_log_warn("%s: destroy event source by force.",
				     name);
		}

		EVSOURCE_WRLOCK(esrc);
		evsource_release(esrc);
	}
}

/*
 * static void
 * evsource_add_handler(evsource_t *PFC_RESTRICT esrc,
 *			evhandler_t *PFC_RESTRICT ehp)
 *	Bind the specified handler to the event source.
 *
 * Remarks:
 *	This function must be called with holding event source lock in
 *	writer mode.
 */
static void
evsource_add_handler(evsource_t *PFC_RESTRICT esrc,
		     evhandler_t *PFC_RESTRICT ehp)
{
	pfc_list_t	*elem, *head = &esrc->es_handler;
	uint32_t	pri = ehp->eh_priority;
	evhandler_t	*h;

	ehp->eh_source = esrc;
	ehp->eh_state = 0;

	if (pfc_list_is_empty(head)) {
		pfc_list_push(head, &ehp->eh_list);

		return;
	}

	/* Sort event handlers in their priority order. */
	h = EVHANDLER_CAST(head->pl_prev);
	if (h->eh_priority <= pri) {
		/* A new handler has the largest priority value. */
		pfc_list_push_tail(head, &ehp->eh_list);

		return;
	}

#ifdef	PFC_VERBOSE_DEBUG
	pfc_list_init(&ehp->eh_list);
#endif	/* PFC_VERBOSE_DEBUG */

	PFC_LIST_FOREACH(head, elem) {
		h = EVHANDLER_CAST(elem);
		if (h->eh_priority >= pri) {
			/* Link the handler just before this element. */
			pfc_list_push_tail(elem, &ehp->eh_list);
			break;
		}
	}

	/* The handler must be linked by above loop. */
	PFC_ASSERT(!pfc_list_is_empty(&ehp->eh_list));
}

/*
 * static void
 * evsource_bind_orphan(evsource_t *PFC_RESTRICT esrc,
 *			evhandler_t *PFC_RESTRICT ehp)
 *	Bind the orphan event handler to the specified event source.
 *
 * Remarks:
 *	This function must be called with holding system global lock in
 *	writer mode, and event source lock in writer mode.
 */
static void
evsource_bind_orphan(evsource_t *PFC_RESTRICT esrc,
		     evhandler_t *PFC_RESTRICT ehp)
{
	EVHANDLER_LOCK(ehp);

	if (PFC_EXPECT_TRUE(!EVHANDLER_IS_REMOVED(ehp))) {
		pfc_log_debug("%s: bind orphan handler: id=%u",
			      esrc->es_name, ehp->eh_id);
		evsource_add_handler(esrc, ehp);
	}

	EVHANDLER_UNLOCK(ehp);
}

/*
 * static pfc_cptr_t
 * evsource_getkey(pfc_rbnode_t *node)
 *	Return the key of event source specified by `node'.
 *	`node' must be a pointer to es_node in evsource_t.
 */
static pfc_cptr_t
evsource_getkey(pfc_rbnode_t *node)
{
	evsource_t	*esrc = EVSOURCE_NODE2PTR(node);

	return (pfc_cptr_t)esrc->es_name;
}

/*
 * static int
 * evhandler_add(pfc_evhandler_t *PFC_RESTRICT idp,
 *		 const char *PFC_RESTRICT name,
 *		 pfc_refptr_t *domain, pfc_evfunc_t handler,
 *		 pfc_ptr_t PFC_RESTRICT arg,
 *		 const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority,
 *		 pfc_refptr_t *rhname)
 *	Add an event handler which receives events posted to the event source
 *	specified by the name.
 *
 *	`name' is a name of the event source.
 *	`domain' is a refptr string which represents event delivery domain.
 *	NULL can be specified to `domain' if no event domain needs to be
 *	defined.
 *
 *	`rhname' is a string refptr which represents user-defined name of
 *	event handler. Currently, it is used only for event delivery logging.
 */
static int
evhandler_add(pfc_evhandler_t *PFC_RESTRICT idp, const char *PFC_RESTRICT name,
	      pfc_refptr_t *domain, pfc_evfunc_t handler,
	      pfc_ptr_t PFC_RESTRICT arg,
	      const pfc_evmask_t *PFC_RESTRICT evmask, uint32_t priority,
	      pfc_refptr_t *rhname)
{
	evhandler_t	*ehp;
	pfc_evhandler_t	newid, start;
	pfc_rbnode_t	*node;
	int		err;

	if (PFC_EXPECT_FALSE(idp == NULL || name == NULL || handler == NULL)) {
		return EINVAL;
	}

	if (evmask == NULL) {
		evmask = &event_mask_fill;
	}
	else if (PFC_EXPECT_FALSE(*evmask == PFC_EVENT_MASK_EMPTY)) {
		return EINVAL;
	}

	/* Allocate a new event handler instance. */
	ehp = (evhandler_t *)malloc(sizeof(*ehp));
	if (PFC_EXPECT_FALSE(ehp == NULL)) {
		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&ehp->eh_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_alloc;
	}

	err = pfc_cond_init(&ehp->eh_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_alloc;
	}

	if (rhname == NULL) {
		/* Use domain name as handler name. */
		rhname = domain;
	}
	if (rhname != NULL) {
		pfc_refptr_get(rhname);
	}

	ehp->eh_source = NULL;
	ehp->eh_domain = domain;
	ehp->eh_name = rhname;
	ehp->eh_handler = handler;
	ehp->eh_arg = arg;
	ehp->eh_mask = *evmask;
	ehp->eh_priority = priority;
	ehp->eh_nwaiters = 0;
	ehp->eh_state = 0;
	ehp->eh_nactives = 0;

	EVENT_SYSTEM_WRLOCK();

	/*
	 * New event handler ID must be copied before it is added to the
	 * event source. Once event handler becomes visible to event source,
	 * the event handler instance specified by `ehp' may be destroyed
	 * by the call of pfc_event_unregister() on another thread.
	 */
	newid = evhandler_id_next;
	PFC_ASSERT(newid != EVHANDLER_ID_INVALID);

	/* Register event handler to the handler tree. */
	start = newid;
	for (;;) {
		pfc_evhandler_t	id;

		ehp->eh_id = newid;
		err = pfc_rbtree_put(&event_handlers, &ehp->eh_node);

		/* Prepare ID for next allocation. */
		id = newid + 1;
		if (PFC_EXPECT_FALSE(id == EVHANDLER_ID_INVALID)) {
			id++;
		}
		if (PFC_EXPECT_TRUE(err == 0)) {
			evhandler_id_next = id;
			break;
		}

		/* Try next ID. */
		newid = id;
		if (PFC_EXPECT_FALSE(newid == start)) {
			/* No ID is available. */
			err = ENFILE;
			goto error;
		}
	}

	/* Obtain event source instance associated with the given name. */
	node = pfc_rbtree_get(&event_sources, name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		evsource_t	*esrc = EVSOURCE_NODE2PTR(node);

		/* Bind the event handler to this event source. */
		EVSOURCE_WRLOCK(esrc);

		if (PFC_EXPECT_FALSE(esrc->es_queue == NULL)) {
			EVSOURCE_UNLOCK(esrc);
			err = ENOENT;
			goto error_rmnode;
		}

		EVENT_SYSTEM_UNLOCK();

		evsource_add_handler(esrc, ehp);
		EVSOURCE_UNLOCK(esrc);
	}
	else {
		/*
		 * Register this handler to the orphan handler tree.
		 * This handler will be activated when the target event source
		 * is registered.
		 */
		err = evhandler_add_orphan(name, ehp);

		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("failed to add handler to orphan "
				      "tree: %s", strerror(err));
			goto error_rmnode;
		}

		EVENT_SYSTEM_UNLOCK();
		pfc_log_debug("%s: added to orphan tree: id=%u",
			      name, ehp->eh_id);
	}

	*idp = newid;

	return 0;

error_rmnode:
	pfc_rbtree_remove_node(&event_handlers, &ehp->eh_node);

error:
	EVENT_SYSTEM_UNLOCK();
	if (rhname != NULL) {
		pfc_refptr_put(rhname);
	}

error_alloc:
	free(ehp);

	return err;
}

/*
 * static int
 * evhandler_add_orphan(const char *PFC_RESTRICT name,
 *			evhandler_t *PFC_RESTRICT ehp)
 *	Add the given handler to the orphan handler tree.
 *
 *	This function is called if the target event source for the event
 *	handler is not yet registered. Those handlers are kept in the orphan
 *	handler tree. They will be bound to the event source when the target
 *	event source is registered.
 *
 * Remarks:
 *	This function must be called with holding the event system global lock
 *	in writer mode.
 */
static int
evhandler_add_orphan(const char *PFC_RESTRICT name,
		     evhandler_t *PFC_RESTRICT ehp)
{
	orphan_list_t	*olp;
	pfc_rbtree_t	*tree = &event_orphans;
	pfc_rbnode_t	*node;

	/*
	 * Check to see whether orphan handler for the same event source
	 * already exists.
	 */
	node = pfc_rbtree_get(tree, name);
	if (node != NULL) {
		/* Orphan list for this event source is already registered. */
		olp = ORPHAN_NODE2PTR(node);
	}
	else {
		int	err;

		/* Allocate a new orphan list head. */
		olp = (orphan_list_t *)malloc(sizeof(*olp));
		if (PFC_EXPECT_FALSE(olp == NULL)) {
			return ENOMEM;
		}

		olp->ol_name = strdup(name);
		if (PFC_EXPECT_FALSE(olp->ol_name == NULL)) {
			free(olp);

			return ENOMEM;
		}
		pfc_list_init(&olp->ol_list);

		/* Register orphan list to the orphan list tree. */
		err = pfc_rbtree_put(tree, &olp->ol_node);
		if (PFC_EXPECT_FALSE(err != 0)) {
			orphan_list_free(olp);

			return err;
		}
	}

	/* Register this handler to the orphan list. */
	pfc_list_push(&olp->ol_list, &ehp->eh_list);

	/* Set orphan list pointer to eh_source. */
	ehp->eh_source = (evsource_t *)olp;
	ehp->eh_state |= EVH_STATE_ORPHAN;

	return 0;
}

/*
 * static void
 * evhandler_remove_orphan(evhandler_t *ehp)
 *	Remove the event handler which is not bound to any event source.
 *	The specified handler must exist in the orphan tree.
 *
 * Remarks:
 *	This function must be called with holding the event system global lock
 *	in writer mode, and the event handler lock. In addition, the caller
 *	must ensure that ORPHAN bit is set in the handler instance.
 *
 *	Both locks are released on return.
 */
static void
evhandler_remove_orphan(evhandler_t *ehp)
{
	orphan_list_t	*olp;

	/* Remove this entry from the orphan list. */
	pfc_list_remove(&ehp->eh_list);

	/* A pointer to the head of orphan list must be kept in eh_source. */
	olp = (orphan_list_t *)ehp->eh_source;
	if (pfc_list_is_empty(&olp->ol_list)) {
		/* Remove the head of orphan list. */
		pfc_rbtree_remove_node(&event_orphans, &olp->ol_node);
		orphan_list_free(olp);
	}

	ehp->eh_source = NULL;
	ehp->eh_state &= ~EVH_STATE_ORPHAN;

	EVHANDLER_UNLOCK(ehp);
	EVENT_SYSTEM_UNLOCK();

	/* Destroy handler instance. */
	evhandler_destroy(ehp);
}

/*
 * static int
 * evhandler_wait(evhandler_t *PFC_RESTRICT ehp, evsource_t *PFC_RESTRICT esrc,
 *		  const pfc_timespec_t *abstime)
 *	Block the calling thread until the event handler specified by `ehp'
 *	is inactivated.
 *
 *	`esrc' must be a pointer to event source instance which contains the
 *	event handler specified by `ehp'.
 *
 *	`current' must be the identifier of the calling thread.
 *
 *	If the system absolute time specified by `abstime' passes before the
 *	event handler is inactivated, ETIMEDOUT is returned. It means an
 *	infinite timeout to specify NULL to `abstime'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EBUSY is returned if this function is called by the given event
 *	handler.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the event handler lock.
 *	  The handler lock is always released on return.
 *
 *	- The event handler instance specified by `ehp' must be unlinked
 *	  from the event source before calling this function.
 */
static int
evhandler_wait(evhandler_t *PFC_RESTRICT ehp, evsource_t *PFC_RESTRICT esrc,
	       const pfc_timespec_t *abstime)
{
	evthread_t	*etp;

	PFC_ASSERT(!EVHANDLER_IS_ORPHAN(ehp));
	PFC_ASSERT(EVHANDLER_IS_REMOVED(ehp));

	if (PFC_EXPECT_TRUE(!EVHANDLER_IS_ACTIVE(ehp))) {
		/* The given handler is not being called. */
		goto out;
	}

	etp = (evthread_t *)pfc_thread_getspecific(evthread_key);
	if (etp != NULL && etp->et_handler == ehp) {
		pfc_log_info("handler is trying to remove itself: ehp=%p, "
			     "id=%u, name=%s", ehp, ehp->eh_id,
			     EVHANDLER_NAME(ehp));

		/*
		 * This handler will be removed when the handler is
		 * returned.
		 */
		evhandler_set_rmfailed(ehp, esrc);

		return EBUSY;
	}

	for (;;) {
		int	err;

		ehp->eh_nwaiters++;
		err = EVHANDLER_TIMEDWAIT_ABS(ehp, abstime);
		ehp->eh_nwaiters--;

		if (PFC_EXPECT_TRUE(!EVHANDLER_IS_ACTIVE(ehp))) {
			break;
		}
		if (PFC_EXPECT_FALSE(err != 0)) {
			/*
			 * Set RMFAILED flag to indicate that this handler
			 * instance must be released by event dispatch thread.
			 */
			pfc_log_error("handler seems stuck: ehp=%p, id=%u, "
				      "name=%s", ehp, ehp->eh_id,
				      EVHANDLER_NAME(ehp));
			evhandler_set_rmfailed(ehp, esrc);

			return err;
		}
	}

out:
	EVHANDLER_UNLOCK(ehp);

	return 0;
}

/*
 * static void
 * evhandler_set_rmfailed(evhandler_t *PFC_RESTRICT ehp,
 *			  evsource_t *PFC_RESTRICT esrc)
 *	Set RMFAILED flag to the given event handler, which indicates that
 *	the handler is removed although it is being called.
 *
 * Remarks:
 *	This function must be called with holding the event handler lock.
 *	The handler lock is always released on return.
 */
static void
evhandler_set_rmfailed(evhandler_t *PFC_RESTRICT ehp,
		       evsource_t *PFC_RESTRICT esrc)
{
	ehp->eh_state |= EVH_STATE_RMFAILED;
	EVHANDLER_UNLOCK(ehp);

	/* Hold the event source. */
	EVSOURCE_WRLOCK(esrc);
	evsource_hold(esrc);
	EVSOURCE_UNLOCK(esrc);
}

/*
 * static int
 * evhandler_fini(pfc_list_t *PFC_RESTRICT handlers,
 *		  evsource_t *PFC_RESTRICT esrc,
 *		  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Finalize event handler instances.
 *
 *	`handlers' must be the head of event handler's list which contains
 *	event handler instances to be removed. They must be contained by the
 *	event source specified by `esrc'.
 *
 *	If the system absolute time specified by `abstime' passes before the
 *	event handler is inactivated, ETIMEDOUT is returned. It means an
 *	infinite timeout to specify NULL to `abstime'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the event source lock
 *	  in writer mode. The lock is always released on return.
 *
 *	- `handlers' is destructive. all event handlers in the `handlers' list
 *	  will be removed on successful return.
 */
static int
evhandler_fini(pfc_list_t *PFC_RESTRICT handlers,
	       evsource_t *PFC_RESTRICT esrc,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	eventq_t	*evq = esrc->es_queue;
	pfc_list_t	*elem, *next, removed = PFC_LIST_INITIALIZER(removed);
	int		err;

	/* Remove queue data which contains handlers to be removed. */
	PFC_ASSERT(evq != NULL);
	EVENTQ_LOCK(evq);
	PFC_LIST_FOREACH_SAFE(&evq->eq_head, elem, next) {
		evqdata_t	*qdp = EVQDATA_CAST(elem);
		pfc_list_t	*e;

		PFC_LIST_FOREACH(handlers, e) {
			if (qdp->eqd_handler == EVHANDLER_CAST(e)) {
				EVENTQ_COUNT_DEC(evq);
				pfc_list_remove(elem);
				pfc_list_push(&removed, elem);
				break;
			}
		}
	}
	EVENTQ_UNLOCK(evq);

	/* Hold the event source. */
	evsource_hold(esrc);
	EVSOURCE_UNLOCK(esrc);

	/* Destroy all event handlers in the given list. */
	err = evhandler_free(handlers, esrc, abstime);

	/* Release the event source. */
	EVSOURCE_WRLOCK(esrc);
	evsource_release(esrc);

	/* Free up event queue data to be removed. */
	while ((elem = pfc_list_pop(&removed)) != NULL) {
		evqdata_t	*qdp = EVQDATA_CAST(elem);

		EVENT_RELEASE(qdp->eqd_event);
		free(qdp);
	}

	return err;
}

/*
 * static int
 * evhandler_free(pfc_list_t *PFC_RESTRICT handlers,
 *		  evsource_t *PFC_RESTRICT esrc,
 *		  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Free up resources held by event handlers in the list specified by
 *	`handlers'.
 *
 *	`handlers' must be the head of event handler's list which contains
 *	event handler instances to be removed. They must be contained by the
 *	event source specified by `esrc'.
 *
 *	If the system absolute time specified by `abstime' passes before the
 *	event handler is inactivated, ETIMEDOUT is returned. It means an
 *	infinite timeout to specify NULL to `abstime'.
 *
 * Calling/Exit State:
 *	Zero is returned if all handlers are freed successfully.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must not be called with holding the event source lock
 *	  because this function may acquire it.
 *
 *	- `handlers' is destructive. all event handlers in the `handlers' list
 *	  will be removed on successful return.
 */
static int
evhandler_free(pfc_list_t *PFC_RESTRICT handlers,
	       evsource_t *PFC_RESTRICT esrc,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_list_t	*elem;
	int		ret = 0;

	/* Ensure that all handlers are inactivated. */
	while ((elem = pfc_list_pop(handlers)) != NULL) {
		evhandler_t	*ehp = EVHANDLER_CAST(elem);
		int		err;

		/* Handler's lock is released by evhandler_wait(). */
		EVHANDLER_LOCK(ehp);
		err = evhandler_wait(ehp, esrc, abstime);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/* Destroy this handler instance. */
			evhandler_destroy(ehp);
		}
		else if (err != EBUSY && ret == 0) {
			ret = err;
		}
	}

	return ret;
}

/*
 * static void
 * evhandler_destroy(evhandler_t *ehp)
 *	Destroy handler instance.
 */
static void
evhandler_destroy(evhandler_t *ehp)
{
	pfc_refptr_t	*domain = ehp->eh_domain;
	pfc_refptr_t	*rhname = ehp->eh_name;

	/* Call handler removal callback. */
	if (evhandler_rm_callback != NULL) {
		(*evhandler_rm_callback)(ehp);
	}

	/* Free up resources. */
	PFC_ASSERT_INT(pfc_cond_destroy(&ehp->eh_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&ehp->eh_mutex), 0);

	if (domain != NULL) {
		pfc_refptr_put(domain);
	}
	if (rhname != NULL) {
		pfc_refptr_put(rhname);
	}

	free(ehp);
}

/*
 * static pfc_cptr_t
 * evhandler_getkey(pfc_rbnode_t *node)
 *	Return the key of event handler specified by `node'.
 *	`node' must be a pointer to eh_node in evhandler_t.
 */
static pfc_cptr_t
evhandler_getkey(pfc_rbnode_t *node)
{
	evhandler_t	*ehp = EVHANDLER_NODE2PTR(node);

	return EVHANDLER_KEY(ehp->eh_id);
}

/*
 * static int
 * eventq_create(eventq_t **evqp, evqtype_t qtype, uint32_t maxthreads,
 *		 uint16_t timeout)
 *	Create an event queue.
 *	`maxthreads' is the maximum number of threads in the specified queue.
 *	If `maxthreads' is 1, all event delivery in the queue are serialized.
 *
 *	`timeout' is a timeout value, in seconds, for event dispatch threads.
 *	Event dispatch thread will exit if no new event is received in the
 *	specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to event queue is set to `*evqp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	This function returns zero if at least one thread is created.
 */
static int
eventq_create(eventq_t **evqp, evqtype_t qtype, uint32_t maxthreads,
	      uint16_t timeout)
{
	eventq_t	*evq;
	int	err;

	PFC_ASSERT(maxthreads != 0);

	/* Allocate event queue. */
	evq = (eventq_t *)malloc(sizeof(*evq));
	if (PFC_EXPECT_FALSE(evq == NULL)) {
		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&evq->eq_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	err = pfc_cond_init(&evq->eq_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	EVENTQ_HEAD_INIT(evq);
	evq->eq_maxthreads = maxthreads;
	evq->eq_nthreads = 0;
	evq->eq_nwaiting = 0;
	evq->eq_type = qtype;
	evq->eq_state = 0;
	evq->eq_timeout = timeout;
	pfc_list_init(&evq->eq_tlist);

	*evqp = evq;

	return 0;

error:
	free(evq);

	return err;
}

/*
 * static int
 * eventq_destroy(eventq_t *PFC_RESTRICT evq,
 *		  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Destroy the specified event queue.
 *	All queue data in the queue are discarded.
 */
static int
eventq_destroy(eventq_t *PFC_RESTRICT evq,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	pfc_list_t	removed, *elem;

	EVENTQ_LOCK(evq);

	/* Unlink all queue data. */
	pfc_list_move_all(&evq->eq_head, &removed);
	EVENTQ_HEAD_INIT(evq);

	/* Shut down the queue. */
	EVENTQ_SHUTDOWN(evq);

	EVENTQ_UNLOCK(evq);

	/* Free up event queue data to be removed. */
	while ((elem = pfc_list_pop(&removed)) != NULL) {
		evqdata_t	*qdp = EVQDATA_CAST(elem);
		event_t		*ev = qdp->eqd_event;

		EVENT_RELEASE(ev);
		free(qdp);
	}

	/* Wait for all threads to quit. */
	EVENTQ_LOCK(evq);
	while (evq->eq_nthreads != 0) {
		int	err = EVENTQ_TIMEDWAIT_ABS(evq, abstime);

		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_log_error("thread seems stuck.");
			EVENTQ_UNLOCK(evq);

			return err;
		}
		PFC_MEMORY_RELOAD();
	}
	EVENTQ_UNLOCK(evq);

	PFC_ASSERT_INT(pfc_cond_destroy(&evq->eq_cond), 0);
	PFC_ASSERT_INT(pfc_mutex_destroy(&evq->eq_mutex), 0);
	free(evq);

	return 0;
}

/*
 * static void
 * eventq_remove_source(eventq_t *PFC_RESTRICT evq,
 *			evsource_t *PFC_RESTRICT esrc)
 *	Remove all event queue data which contains the specified event source.
 */
static void
eventq_remove_source(eventq_t *PFC_RESTRICT evq, evsource_t *PFC_RESTRICT esrc)
{
	pfc_list_t	removed = PFC_LIST_INITIALIZER(removed), *elem;

	EVENTQ_LOCK(evq);

	elem = evq->eq_head.pl_next;
	while (elem != &evq->eq_head) {
		pfc_list_t	*next = elem->pl_next;
		evqdata_t	*qdp = EVQDATA_CAST(elem);

		if (qdp->eqd_source == esrc) {
			EVENTQ_COUNT_DEC(evq);
			pfc_list_remove(elem);
			pfc_list_push(&removed, elem);
		}
		elem = next;
	}

	EVENTQ_UNLOCK(evq);

	/* Free up event queue data to be removed. */
	while ((elem = pfc_list_pop(&removed)) != NULL) {
		evqdata_t	*qdp = EVQDATA_CAST(elem);

		EVENT_RELEASE(qdp->eqd_event);
		free(qdp);
	}
}

/*
 * static int
 * eventq_post(const char *PFC_RESTRICT name, const char *PFC_RESTRICT domain,
 *	       event_t *PFC_RESTRICT ev, pfc_bool_t emergency)
 *	Post an event to the event queue associated with the given event
 *	source.
 *
 *	If a non-NULL pointer is specified to `domain', the specified event
 *	is delivered to only handlers associated with the specified domain.
 */
static int
eventq_post(const char *PFC_RESTRICT name, const char *PFC_RESTRICT domain,
	    event_t *PFC_RESTRICT ev, pfc_bool_t emergency)
{
	pfc_rbnode_t	*node;
	evsource_t	*esrc;
	eventq_t	*evq;
	int	err;

	if (PFC_EXPECT_FALSE(name == NULL)) {
		err = EINVAL;
		goto out;
	}

	EVENT_SYSTEM_RDLOCK();

	/* Obtain event source instance associated with the given name. */
	node = pfc_rbtree_get(&event_sources, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		EVENT_SYSTEM_UNLOCK();
		err = ENOENT;
		goto out;
	}

	/* Acquire event source lock before unlocking global lock. */
	esrc = EVSOURCE_NODE2PTR(node);
	EVSOURCE_RDLOCK(esrc);
	EVENT_SYSTEM_UNLOCK();

	evq = esrc->es_queue;
	if (PFC_EXPECT_FALSE(evq == NULL)) {
		/* This event source has been removed. */
		err = ESHUTDOWN;
	}
	else {
		err = eventq_enqueue(esrc, evq, domain, ev, emergency);
	}

	EVSOURCE_UNLOCK(esrc);

out:
	EVENT_RELEASE(ev);

	return err;
}

/*
 * static int
 * eventq_enqueue(evsource_t *PFC_RESTRICT esrc, eventq_t *PFC_RESTRICT evq,
 *		  const char *PFC_RESTRICT domain, event_t *PFC_RESTRICT ev,
 *		  pfc_bool_t emergency)
 *	Enqueue the given event to the event source.
 *
 *	If a non-NULL pointer is specified to `domain', the specified event
 *	is delivered to only handlers associated with the specified domain.
 *
 *	If `emergency' is true, the specified event is pushed to the head of
 *	the queue. If false, it is pushed to the tail.
 *
 * Remarks:
 *	This function must be called with holding the event source lock in
 *	reader or writer mode.
 */
static int
eventq_enqueue(evsource_t *PFC_RESTRICT esrc, eventq_t *PFC_RESTRICT evq,
	       const char *PFC_RESTRICT domain, event_t *PFC_RESTRICT ev,
	       pfc_bool_t emergency)
{
	pfc_list_t	*elem;
	pfc_evmask_t	mask = EVMASK_BIT(ev->e_type);

	PFC_ASSERT(evq != NULL);

	if (ev->e_type == PFC_IEVTYPE_SYNC) {
		/* Enqueue sync event to flush event queue. */
		return eventq_enqueue_data(esrc, evq, ev, NULL, emergency);
	}

	PFC_LIST_FOREACH(&esrc->es_handler, elem) {
		evhandler_t	*ehp = EVHANDLER_CAST(elem);
		int		err;

		if (!EVMASK_TEST(ehp->eh_mask, mask)) {
			/* This handler doesn't want this type of event. */
			continue;
		}

		if (domain != NULL) {
			pfc_refptr_t	*rdomain = ehp->eh_domain;
			const char	*d = pfc_refptr_string_value(rdomain);

			/*
			 * Deliver this event only handlers that match the
			 * given delivery domain.
			 */
			if (strcmp(d, domain) != 0) {
				continue;
			}
		}

		/* Enqueue a pair of event and handler. */
		err = eventq_enqueue_data(esrc, evq, ev, ehp, emergency);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * static int
 * eventq_enqueue_data(evsource_t *PFC_RESTRICT esrc, event_t *PFC_RESTRICT ev,
 *		       evhandler_t *PFC_RESTRICT ehp, pfc_bool_t emergency)
 *	Enqueue the given event, which will be delivered to the specified
 *	handler, to the event source.
 *	If `emergency' is true, the specified event is pushed to the head of
 *	the queue. If false, it is pushed to the tail.
 *
 * Remarks:
 *	This function must be called with holding the event source lock in
 *	reader or writer mode.
 */
static int
eventq_enqueue_data(evsource_t *PFC_RESTRICT esrc, eventq_t *PFC_RESTRICT evq,
		    event_t *PFC_RESTRICT ev, evhandler_t *PFC_RESTRICT ehp,
		    pfc_bool_t emergency)
{
	evqdata_t	*qdp;

	/* Allocate event queue data. */
	qdp = (evqdata_t *)malloc(sizeof(*qdp));
	if (PFC_EXPECT_FALSE(qdp == NULL)) {
		pfc_log_error("failed to allocate memory for sending event.");

		return ENOMEM;
	}

	qdp->eqd_source = esrc;
	qdp->eqd_event = ev;
	qdp->eqd_handler = ehp;
	EVENT_HOLD(ev);

	/* Ensure that the event queue is active. */
	EVENTQ_LOCK(evq);
	if (PFC_EXPECT_FALSE(EVENTQ_IS_SHUTDOWN(evq))) {
		EVENTQ_UNLOCK(evq);
		EVENT_RELEASE(ev);
		free(qdp);

		return ESHUTDOWN;
	}

	if (ev->e_type == PFC_IEVTYPE_SYNC && evq->eq_nthreads == 0 &&
	    pfc_list_is_empty(&evq->eq_head)) {
		/* No event is queued. */
		PFC_ASSERT(evq->eq_count == 0);
		EVENTQ_UNLOCK(evq);
		EVENT_RELEASE(ev);
		free(qdp);

		return 0;
	}

	/* Post event to the queue. */
	if (emergency) {
		pfc_list_push(&evq->eq_head, &qdp->eqd_list);
	}
	else {
		pfc_list_push_tail(&evq->eq_head, &qdp->eqd_list);
	}
	EVENTQ_COUNT_INC(evq);

	eventq_start(evq, qdp);
	EVENTQ_UNLOCK(evq);

	return 0;
}

/*
 * static evqdata_t *
 * eventq_filter(evthread_t *PFC_RESTRICT etp, evqdata_t *PFC_RESTRICT qdp)
 *	Filter out system internal event.
 *
 *	If the specified queue data contains a system internal event,
 *	process it and returns next queue data. NULL is returned if no more
 *	queue data exists.
 *
 * Remarks:
 *	This function must be called with holding lock of event queue
 *	associated with the event dispatch thread.
 */
static evqdata_t *
eventq_filter(evthread_t *PFC_RESTRICT etp, evqdata_t *PFC_RESTRICT qdp)
{
	pfc_list_t	*elem;
	event_t		*ev;
	eventq_t	*evq;

	while (qdp->eqd_handler == NULL) {
		/*
		 * This must be a synchronization event.
		 * To ensure flush all events, we must ensure that all existing
		 * event dispatch threads have finished the current work.
		 *
		 * So this event is pushed to sync list for other threads.
		 * It will be cleared when the thread finishes the current
		 * work. So the reference counter of the event will also be
		 * decremented at the time.
		 */
		ev = qdp->eqd_event;
		free(qdp);
		PFC_ASSERT(ev->e_type == PFC_IEVTYPE_SYNC);

		evq = etp->et_queue;
		PFC_LIST_FOREACH(&evq->eq_tlist, elem) {
			int	err;
			evthread_t	*t =
				PFC_CAST_CONTAINER(elem, evthread_t, et_list);

			if (t == etp) {
				continue;
			}

			err = pfc_listm_push(t->et_sync,
					     (pfc_cptr_t)ev->e_refptr);
			if (PFC_EXPECT_FALSE(err != 0)) {
				pfc_log_error("failed to push sync event: %s",
					      strerror(err));
			}
		}

		/* Wake up all threads. */
		EVENTQ_BROADCAST(evq);

		/* Release sync event. */
		EVENT_RELEASE(ev);

		/* Return next queue data. */
		elem = pfc_list_pop(&evq->eq_head);
		if (elem == NULL) {
			qdp = NULL;
			break;
		}

		EVENTQ_COUNT_DEC(evq);
		qdp = EVQDATA_CAST(elem);
	}

	return qdp;
}

/*
 * static evqdata_t *
 * eventq_dequeue(evthread_t *PFC_RESTRICT etp,
 *		 const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Dequeue one event data at the head of event queue.
 *	The current thread will be blocked if no event exists in the queue.
 *
 * Calling/Exit State:
 *	An event queue data pointer is returned on success.
 *	The calling thread must quit if this function returns NULL.
 *
 * Remarks:
 *	- The caller must call this function with holding event queue lock.
 *
 *	- If this function returns non-NULL value, the event handler instance
 *	  in the returned evqdata_t are held by EVHANDLER_INC_ACTIVE().
 *	  The caller must release it by EVHANDLER_DEC_ACTIVE().
 *
 *	- NULL is set to qdp->eqd_handler if the event handler is already
 *	  removed. The caller must ensure that qdp->eqd_handler is not NULL.
 */
static evqdata_t *
eventq_dequeue(evthread_t *PFC_RESTRICT etp,
	       const pfc_timespec_t *PFC_RESTRICT timeout)
{
	eventq_t	*evq = etp->et_queue;
	evqdata_t	*qdp = NULL;
	evhandler_t	*ehp;
	pfc_listm_t	slist = etp->et_sync;
	pfc_bool_t	tout = PFC_FALSE;

	while (1) {
		pfc_list_t	*elem;
		int		err;
		uint8_t		state = evq->eq_state;

		if (PFC_EXPECT_FALSE(EVQ_STATE_IS_SHUTDOWN(state))) {
			/* Queue has been shut down. */
			return NULL;
		}

		/*
		 * Clear sync event list here.
		 * Currently we have no event to deliver, so we can say
		 * all events posted before sync events are delivered.
		 */
		pfc_listm_clear(slist);

		elem = pfc_list_pop(&evq->eq_head);
		if (elem != NULL) {
			/* A new event has come. */
			EVENTQ_COUNT_DEC(evq);
			qdp = eventq_filter(etp, EVQDATA_CAST(elem));
			if (qdp != NULL) {
				break;
			}
		}

		if (tout) {
			return NULL;
		}

		evq->eq_nwaiting++;
		err = EVENTQ_TIMEDWAIT(evq, timeout);
		evq->eq_nwaiting--;
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* One more check should be done. */
			tout = PFC_TRUE;
		}
	}

	PFC_ASSERT(qdp == NULL || qdp->eqd_handler != NULL);

	ehp = qdp->eqd_handler;
	EVHANDLER_LOCK(ehp);

	if (PFC_EXPECT_FALSE(EVHANDLER_IS_REMOVED(ehp))) {
		/* This handler is already removed. */
		qdp->eqd_handler = NULL;
	}
	else {
		/* Increment active counter. */
		EVHANDLER_INC_ACTIVE(ehp);
	}
	EVHANDLER_UNLOCK(ehp);

	return qdp;
}

/*
 * static void *
 * eventq_main(void *arg)
 *	Start routine of event dispatch queue thread.
 */
static void *
eventq_main(void *arg)
{
	evthread_t	*etp = (evthread_t *)arg;
	eventq_t	*evq = etp->et_queue;
	pfc_timespec_t	tspec, *timeout;
	int		err;

	if (evq->eq_type == EVQTYPE_GLOBAL) {
		/* Global queue thread never quits by timeout. */
		timeout = NULL;
	}
	else {
		timeout = &tspec;
		timeout->tv_sec = evq->eq_timeout;
		timeout->tv_nsec = 0;
	}

	EVENTQ_LOCK(evq);

	/* Link thread information. */
	pfc_list_push(&evq->eq_tlist, &etp->et_list);

	/* Set thread specific data. */
	err = pfc_thread_setspecific(evthread_key, etp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_log_error("failed to set evthread specific data: %s",
			      strerror(err));
		goto out;
	}

	while (1) {
		evqdata_t	*qdp;
		evsource_t	*esrc;
		evhandler_t	*ehp;
		event_t		*ev;

		/*
		 * Dequeue an event.
		 * Note that eventq_dequeue() drops the queue lock.
		 */
		qdp = eventq_dequeue(etp, timeout);
		if (PFC_EXPECT_TRUE(qdp == NULL)) {
			break;
		}

		EVENTQ_UNLOCK(evq);
		ev = qdp->eqd_event;
		esrc = qdp->eqd_source;
		ehp = qdp->eqd_handler;
		free(qdp);

		if (PFC_EXPECT_TRUE(ehp != NULL)) {
			/* Dispatch this event. */
			eventq_dispatch(esrc, ehp, ev, etp);
		}

		/* Release this event. */
		EVENT_RELEASE(ev);

		EVENTQ_LOCK(evq);
	}

out:
	/* Decrement number of threads because this thread is about to quit. */
	PFC_ASSERT(evq->eq_nthreads != 0);
	evq->eq_nthreads--;
	if (evq->eq_nthreads == 0) {
		/* Some threads may wait for this queue to be down. */
		EVENTQ_BROADCAST(evq);
	}

	/* Unlink thread private data. */
	pfc_list_remove(&etp->et_list);

	EVENTQ_UNLOCK(evq);

	/* Clean up sync event list, and destroy data. */
	pfc_listm_destroy(etp->et_sync);
	free(etp);

	return NULL;
}

/*
 * static void
 * eventq_dispatch(evsource_t *PFC_RESTRICT esrc,
 *		   evhandler_t *PFC_RESTRICT ehp, event_t *PFC_RESTRICT ev,
 *		   evthread_t *PFC_RESTRICT etp)
 *	Dispatch an event to the specified event handler.
 */
static void
eventq_dispatch(evsource_t *PFC_RESTRICT esrc, evhandler_t *PFC_RESTRICT ehp,
		event_t *PFC_RESTRICT ev, evthread_t *PFC_RESTRICT etp)
{
	pfc_bool_t	removed;

	/* Ensure that the event source is still valid. */
	EVSOURCE_RDLOCK(esrc);
	if (PFC_EXPECT_FALSE(esrc->es_queue == NULL)) {
		EVSOURCE_UNLOCK(esrc);
		goto out;
	}

	/* Event mask test must be already done. */
	PFC_ASSERT(EVMASK_TEST_TYPE(ehp->eh_mask, ev->e_type));

	/* Ensure that this handler is still valid. */
	EVHANDLER_LOCK(ehp);
	PFC_ASSERT((evthread_t *)pfc_thread_getspecific(evthread_key) == etp);
	PFC_ASSERT(etp->et_handler == NULL);

	if (PFC_EXPECT_FALSE(EVHANDLER_IS_REMOVED(ehp))) {
		removed = PFC_TRUE;
	}
	else {
		removed = PFC_FALSE;

		/* Set current handler to the thread specific data. */
		etp->et_handler = ehp;
	}

	EVHANDLER_UNLOCK(ehp);
	EVSOURCE_UNLOCK(esrc);

	if (PFC_EXPECT_TRUE(!removed)) {
		if (EVENT_NEED_LOG(ev)) {
			eventq_log(esrc, ehp, ev, "Calling");
		}

		/* Call event handler. */
		EVENTQ_DISPATCH_ASSERT_PUSH(ehp);
		ehp->eh_handler(ev, ehp->eh_arg);
		EVENTQ_DISPATCH_ASSERT_POP();

		if (EVENT_NEED_LOG(ev)) {
			eventq_log(esrc, ehp, ev, "Returned");
		}
	}

out:
	EVHANDLER_LOCK(ehp);

	/* Revert changes to active counter made by eventq_dequeue(). */
	EVHANDLER_DEC_ACTIVE(ehp);

	/* Clear current event handler in the thread specific data. */
	etp->et_handler = NULL;

	if (!EVHANDLER_IS_ACTIVE(ehp)) {
		if (PFC_EXPECT_FALSE(ehp->eh_nwaiters != 0)) {
			/* Wake up all threads waiting on this handler. */
			EVHANDLER_BROADCAST(ehp);
		}

		if (PFC_EXPECT_FALSE(EVHANDLER_IS_RMFAILED(ehp))) {
			/* This handler instance must be destroyed here. */
			pfc_log_notice("reaping removed handler: ehp=%p, "
				       "id=%u, name=%s", ehp, ehp->eh_id,
				       EVHANDLER_NAME(ehp));
			EVHANDLER_UNLOCK(ehp);
			evhandler_destroy(ehp);

			EVSOURCE_WRLOCK(esrc);
			evsource_release(esrc);

			return;
		}
	}

	EVHANDLER_UNLOCK(ehp);
}

/*
 * static void
 * eventq_start(eventq_t *PFC_RESTRICT evq, evqdata_t *PFC_RESTRICT qdp)
 *	Start event queue processing.
 *
 * Remarks:
 *	This function must be called with holding the event queue lock.
 */
static void
eventq_start(eventq_t *PFC_RESTRICT evq, evqdata_t *PFC_RESTRICT qdp)
{
	/*
	 * If at least one thread is waiting for a new event, or no more thread
	 * is available, send a signal to one thread.
	 */
	if (evq->eq_nwaiting != 0 || evq->eq_nthreads >= evq->eq_maxthreads) {
		EVENTQ_SIGNAL(evq);
	}
	else {
		evthread_t	*etp;
		pfc_thread_t	t;
		int	err;

		/* Create a thread private data. */
		etp = (evthread_t *)malloc(sizeof(*etp));
		if (PFC_EXPECT_FALSE(etp == NULL)) {
			evsource_t	*esrc = qdp->eqd_source;

			pfc_log_error("%s: failed to allocate thread data",
				      esrc->es_name);

			return;
		}

		/* Create linked list to keep synchronization events. */
		err = pfc_llist_create_ref(&etp->et_sync, &event_refptr_ops);
		if (PFC_EXPECT_FALSE(err != 0)) {
			evsource_t	*esrc = qdp->eqd_source;

			pfc_log_error("%s: failed to create sync event list: "
				      "%s", esrc->es_name,
				      strerror(err));
			free(etp);

			return;
		}
		etp->et_queue = evq;
		etp->et_handler = NULL;
		pfc_list_init(&etp->et_list);

		/* Create a new thread. */
		err = pfc_thread_create(&t, eventq_main, (void *)etp,
					PFC_THREAD_DETACHED);
		if (PFC_EXPECT_FALSE(err != 0)) {
			evsource_t	*esrc = qdp->eqd_source;

			pfc_log_error("%s: failed to create thread: %s",
				      esrc->es_name, strerror(err));
			pfc_listm_destroy(etp->et_sync);
			free(etp);
		}
		else {
			evq->eq_nthreads++;
		}
	}
}

/*
 * static void
 * eventq_log(evsource_t *PFC_RESTRICT esrc, evhandler_t *PFC_RESTRICT ehp,
 *	      event_t *PFC_RESTRICT ev, const char *PFC_RESTRICT message)
 *	Record event delivery log.
 *
 * Remarks:
 *	It is safe to access the event source `esrc' and the event handler
 *	`ehp' without holding any lock because this function is called
 *	while the active counter in `ehp' is not zero.
 */
static void
eventq_log(evsource_t *PFC_RESTRICT esrc, evhandler_t *PFC_RESTRICT ehp,
	   event_t *PFC_RESTRICT ev, const char *PFC_RESTRICT message)
{
	pfc_log_info("%s: "
		     "handler[id=%u name=\"%s\" pri=%u] "
		     "event[id=%u src=\"%s\" type=%u]",
		     message,
		     ehp->eh_id, EVHANDLER_NAME(ehp), ehp->eh_priority,
		     ev->e_serial, esrc->es_name, ev->e_type);
}

/*
 * static int
 * sync_event_create(pfc_event_t *eventp)
 *	Create a synchronization event object, which is used to flush
 *	event queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to new event object is set to
 *	`*eventp', and zero is returned. Otherwise error number which indicates
 *	the cause of error is returned.
 */
static int
sync_event_create(pfc_event_t *eventp)
{
	sync_event_t	*sep;
	event_t	*ev;
	int	err;

	/* Create sync event instance. */
	sep = (sync_event_t *)malloc(sizeof(*sep));
	if (PFC_EXPECT_FALSE(sep == NULL)) {
		return ENOMEM;
	}

	err = PFC_MUTEX_INIT(&sep->se_mutex);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	err = pfc_cond_init(&sep->se_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	sep->se_flags = SEV_WAITING;

	err = pfc_event_create(eventp, 0, (pfc_ptr_t)sep, sync_event_dtor);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Update event type. */
	ev = PFC_EVENT_PTR(*eventp);
	ev->e_type = PFC_IEVTYPE_SYNC;

	return 0;

error:
	free(sep);

	return err;
}

/*
 * static void
 * sync_event_dtor(pfc_ptr_t data)
 *	Private data destructor of synchronization event.
 *	The specified data must be a pointer to sync_event_t instance.
 *
 *	sync_event_dtor() is used as private data destructor for sync event,
 *	which will be called when reference counter of sync event becomes zero.
 *	But if SEV_WAITING flag is set in the specified sync event instance,
 *	it must not be destroyed here because another thread still refers
 *	the sync event instance by the call of pfc_event_flush().
 *
 * Remarks:
 *	sync_event_dtor() may be called with holding event queue lock.
 */
static void
sync_event_dtor(pfc_ptr_t data)
{
	sync_event_t	*sep = (sync_event_t *)data;
	uint32_t	flags;

	SYNC_EVENT_LOCK(sep);

	flags = sep->se_flags;
	if (flags & SEV_WAITING) {
		/*
		 * Wake up waiter.
		 * This sync event instance will be destroyed by waiter.
		 */
		sep->se_flags = (flags | SEV_DONE);
		SYNC_EVENT_SIGNAL(sep);
		SYNC_EVENT_UNLOCK(sep);
	}
	else {
		/*
		 * No one waits on this event. This can occur if the waiter
		 * has given up to synchronize because of timeout.
		 * In this case, this sync event must be destroyed here.
		 */
		sync_event_free(sep);
	}
}

/*
 * static void
 * sync_event_free(sync_event_t *sep)
 *	Release sync event instance.
 *
 * Remarks:
 *	This function must be called with holding sync event lock.
 *	It is always released on return.
 */
static void
sync_event_free(sync_event_t *sep)
{
	PFC_ASSERT_INT(pfc_cond_destroy(&sep->se_cond), 0);

	SYNC_EVENT_UNLOCK(sep);
	PFC_ASSERT_INT(pfc_mutex_destroy(&sep->se_mutex), 0);

	free(sep);
}

/*
 * static pfc_cptr_t
 * orphan_getkey(pfc_rbnode_t *node)
 *	Return the key of orphan handler list specified by `node'.
 *	`node' must be a pointer to ol_node in orphan_list_t.
 */
static pfc_cptr_t
orphan_getkey(pfc_rbnode_t *node)
{
	orphan_list_t	*olp = ORPHAN_NODE2PTR(node);

	return (pfc_cptr_t)olp->ol_name;
}
