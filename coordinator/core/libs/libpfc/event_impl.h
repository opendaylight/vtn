/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_EVENT_IMPL_H
#define	_PFC_LIBPFC_EVENT_IMPL_H

/*
 * Internal definitions for event delivery system.
 */

#include <stdio.h>
#include <pfc/event.h>
#include <pfc/thread.h>
#include <pfc/synch.h>
#include <pfc/list.h>
#include <pfc/rbtree.h>
#include <pfc/debug.h>
#include <pfc/atomic.h>

PFC_C_BEGIN_DECL

/*
 * Internal event type for special use.
 */
#define	PFC_IEVTYPE_SYNC	PFC_CONST_U(0xff)	/* sync event */

/*
 * Common event data.
 *
 * Remarks:
 *	- e_flags must be changed by atomic operation.
 */
typedef struct __pfc_event {
	pfc_refptr_t		*e_refptr;	/* refptr for this object */
	pfc_evid_t		e_serial;	/* serial ID for this event */
	pfc_evtype_t		e_type;		/* type of this event */
	uint16_t		e_flags;	/* flags */
	pfc_ptr_t		e_data;		/* private data */
	pfc_evdtor_t		e_dtor;		/* destructor for e_data */

	/* The following members are for event system internal use. */
	pfc_ptr_t		e_object;	/* event object */
	pfc_evdtor_t		e_objdtor;	/* object destructor */
} event_t;

/*
 * Flags for e_flags.
 */
#define	EVF_MODEVENT		PFC_CONST_U(0x0001)	/* module specific */
#define	EVF_LOG			PFC_CONST_U(0x0002)	/* enable logging */

/*
 * Internal version of pfc_event_hold() / pfc_event_release().
 */
#define	EVENT_HOLD(ev)		pfc_refptr_get((ev)->e_refptr);
#define	EVENT_RELEASE(ev)	pfc_refptr_put((ev)->e_refptr);

struct evsource;
typedef struct evsource		evsource_t;

/*
 * Type of event source, which determines behavior of event dispatching.
 */
typedef enum {
	/*
	 * All events are delivered by one system global thread.
	 */
	EVQTYPE_GLOBAL,

	/*
	 * Each event source which has this type of queue has own one event
	 * dispatch queue and one or more threads.
	 */
	EVQTYPE_LOCAL,

	/*
	 * All events posted to the event source which has this type of queue
	 * are delivered asynchronously. Priority of event handlers are
	 * ignored, and the order of event delivery are undefined.
	 */
	EVQTYPE_ASYNC,
} evqtype_t;

/*
 * Event handler instance.
 */
typedef struct {
	pfc_list_t		eh_list;	/* event source link */
	evsource_t		*eh_source;	/* event source */
	pfc_refptr_t		*eh_domain;	/* event delivery domain */
	pfc_refptr_t		*eh_name;	/* name of this handler */
	pfc_evfunc_t		eh_handler;	/* event handler */
	pfc_ptr_t		eh_arg;		/* argument for handler */
	pfc_mutex_t		eh_mutex;	/* mutex for event handler */
	pfc_cond_t		eh_cond;	/* condvar for event handler */
	pfc_rbnode_t		eh_node;	/* Red-Black Tree node */
	pfc_evmask_t		eh_mask;	/* event mask */
	uint32_t		eh_priority;	/* priority */
	volatile uint32_t	eh_nwaiters;	/* number of waiter threads */
	uint32_t		eh_nactives;	/* active counter */
	pfc_evhandler_t		eh_id;		/* handler ID */
	volatile uint8_t	eh_state;	/* state */
} evhandler_t;

/*
 * Convert list element pointer to event handler pointer.
 */
#define	EVHANDLER_CAST(elem)	PFC_CAST_CONTAINER((elem), evhandler_t, eh_list)

/*
 * Convert Red-Black Tree node pointer to event handler pointer.
 */
#define	EVHANDLER_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), evhandler_t, eh_node)

/*
 * Cast event handler ID to Red-Black Tree key.
 */
#define	EVHANDLER_KEY(id)	((pfc_cptr_t)(uintptr_t)(id))

/*
 * Lock/Unlock event handler instance.
 */
#define	EVHANDLER_LOCK(ehp)	pfc_mutex_lock(&(ehp)->eh_mutex)
#define	EVHANDLER_UNLOCK(ehp)	pfc_mutex_unlock(&(ehp)->eh_mutex)

/*
 * Condition variable macros for event handler.
 */
#define	EVHANDLER_SIGNAL(ehp)		pfc_cond_signal(&(ehp)->eh_cond)
#define	EVHANDLER_BROADCAST(ehp)	pfc_cond_broadcast(&(ehp)->eh_cond)
#define	EVHANDLER_TIMEDWAIT_ABS(ehp, abstime)				\
	pfc_cond_timedwait_abs(&(ehp)->eh_cond, &(ehp)->eh_mutex, (abstime))

/*
 * Increment, decrement, and test number of active threads which dispatch
 * events to this handler. These macros require event handler lock.
 */
#define	EVHANDLER_INC_ACTIVE(ehp)	((ehp)->eh_nactives++)
#define	EVHANDLER_DEC_ACTIVE(ehp)	((ehp)->eh_nactives--)
#define	EVHANDLER_IS_ACTIVE(ehp)				\
	(*((volatile uint32_t *)&(ehp)->eh_nactives) != 0)

/*
 * State for event handler.
 */
#define	EVH_STATE_REMOVED	0x01U		/* handler is removed */
#define	EVH_STATE_ORPHAN	0x02U		/* in orphan tree */
#define	EVH_STATE_RMFAILED	0x04U		/* failed to remove */

#define	EVH_STATE_IS_REMOVED(state)	((state) & EVH_STATE_REMOVED)
#define	EVH_STATE_IS_ORPHAN(state)	((state) & EVH_STATE_ORPHAN)
#define	EVH_STATE_IS_RMFAILED(state)	((state) & EVH_STATE_RMFAILED)
#define	EVHANDLER_IS_REMOVED(ehp)	EVH_STATE_IS_REMOVED((ehp)->eh_state)
#define	EVHANDLER_IS_ORPHAN(ehp)	EVH_STATE_IS_ORPHAN((ehp)->eh_state)
#define	EVHANDLER_IS_RMFAILED(ehp)	EVH_STATE_IS_RMFAILED((ehp)->eh_state)

/*
 * Event queue which contains threads to dispatch events.
 */
typedef struct {
	pfc_list_t		eq_head;	/* queue head */
	pfc_list_t		eq_tlist;	/* thread data list */
	pfc_mutex_t		eq_mutex;	/* mutex for the queue */
	pfc_cond_t		eq_cond;	/* condvar for the queue */
	uint32_t		eq_maxthreads;	/* max number of threads */
	uint32_t		eq_nthreads;	/* current number of threads */
	volatile uint32_t	eq_nwaiting;	/* number of waiting threads */
	evqtype_t		eq_type;	/* event queue type */
	volatile uint8_t	eq_state;	/* event queue state */
	uint16_t		eq_timeout;	/* thread timeout in seconds */
	uint32_t		eq_count;	/* number of queued data */
} eventq_t;

/*
 * Lock/Unlock event queue.
 */
#define	EVENTQ_LOCK(evq)	pfc_mutex_lock(&(evq)->eq_mutex)
#define	EVENTQ_UNLOCK(evq)	pfc_mutex_unlock(&(evq)->eq_mutex)

/*
 * Initialize event queue list.
 */
#define	EVENTQ_HEAD_INIT(evq)				\
	do {						\
		pfc_list_init(&(evq)->eq_head);		\
		(evq)->eq_count = 0;			\
	} while (0)

/*
 * Adjust event queue counter.
 */
#define	EVENTQ_COUNT_INC(evq)			\
	do {					\
		(evq)->eq_count++;		\
	} while (0)

#define	EVENTQ_COUNT_DEC(evq)				\
	do {						\
		PFC_ASSERT((evq)->eq_count > 0);	\
		(evq)->eq_count--;			\
	} while (0)

/*
 * Condition variable macros for event queue.
 */
#define	EVENTQ_SIGNAL(evq)	pfc_cond_signal(&(evq)->eq_cond)
#define	EVENTQ_BROADCAST(evq)	pfc_cond_broadcast(&(evq)->eq_cond)
#define	EVENTQ_WAIT(evq)					\
	pfc_cond_wait(&(evq)->eq_cond, &(evq)->eq_mutex)
#define	EVENTQ_TIMEDWAIT(evq, timeout)					\
	pfc_cond_timedwait(&(evq)->eq_cond, &(evq)->eq_mutex, (timeout))
#define	EVENTQ_TIMEDWAIT_ABS(evq, abstime)				\
	pfc_cond_timedwait_abs(&(evq)->eq_cond, &(evq)->eq_mutex, (abstime))

/*
 * State for event queue.
 */
#define	EVQ_STATE_SHUTDOWN	0x01U		/* shutdown */

#define	EVQ_STATE_IS_SHUTDOWN(state)	((state) & EVQ_STATE_SHUTDOWN)

#define	EVENTQ_IS_SHUTDOWN(evq)		EVQ_STATE_IS_SHUTDOWN((evq)->eq_state)

/*
 * Shut down the queue.
 * You must use this macro with holding event queue lock.
 */
#define	EVENTQ_SHUTDOWN(evq)				\
	do {						\
		(evq)->eq_state |= EVQ_STATE_SHUTDOWN;	\
		EVENTQ_BROADCAST(evq);			\
	} while (0)

/*
 * Event source.
 */
struct evsource {
	/*
	 * Event handler list.
	 * Each elements in the list are always sorted in its priority order.
	 */
	pfc_list_t		es_handler;

	pfc_rwlock_t		es_lock;	/* lock for event source */
	const char		*es_name;	/* name of event source */
	eventq_t		*es_queue;	/* event queue to post event */
	pfc_rbnode_t		es_node;	/* Red-Black tree node */
	volatile uint32_t	es_hold;	/* hold counter */
};

/*
 * Convert Red-Black Tree node pointer to event source pointer.
 */
#define	EVSOURCE_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), evsource_t, es_node)

/*
 * Lock/Unlock event source.
 */
#define	EVSOURCE_RDLOCK(esrc)					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&(esrc)->es_lock), 0)
#define	EVSOURCE_WRLOCK(esrc)					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&(esrc)->es_lock), 0)
#define	EVSOURCE_UNLOCK(esrc)					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&(esrc)->es_lock), 0)

/*
 * Event queue data.
 */
typedef struct {
	evsource_t		*eqd_source;	/* event source */
	event_t			*eqd_event;	/* event object to be posted */
	evhandler_t		*eqd_handler;	/* event handler */
	pfc_list_t		eqd_list;	/* link for event queue */
} evqdata_t;

/*
 * Convert list element pointer to event queue data pointer.
 */
#define	EVQDATA_CAST(elem)	PFC_CAST_CONTAINER((elem), evqdata_t, eqd_list)

/*
 * Cast to event_t pointer.
 */
#define	PFC_EVENT_PTR(event)	((event_t *)(event))

/*
 * Internal version of event mask APIs.
 */
#define	EVMASK_BIT(type)		((pfc_evmask_t)(1U << (type)))
#define	EVMASK_TEST(mask, bits)		((mask) & (bits))
#define	EVMASK_TEST_TYPE(mask, type)	EVMASK_TEST((mask), EVMASK_BIT(type))

/*
 * Prototypes for system internal callback functions.
 */
typedef void	(*evcall_hdr_t)(evhandler_t *ehp);

#ifdef	_PFC_LIBPFC_BUILD

/*
 * Reference pointer operations for event object.
 */
extern const pfc_refptr_ops_t	event_refptr_ops;

/*
 * Boolean switch which determines whether event delivery logging is enabled
 * on all events or not.
 */
extern pfc_bool_t	event_log_force;

/*
 * Internal prototypes.
 */
extern int	pfc_event_create_flags(pfc_event_t *eventp, pfc_evtype_t type,
				       pfc_ptr_t data, pfc_evdtor_t dtor,
				       uint16_t flags);

extern void	pfc_event_init(void);
extern void	pfc_event_shutdown(void);

extern int	pfc_event_domain_add_handler(pfc_evhandler_t *PFC_RESTRICT idp,
					     const char *PFC_RESTRICT name,
					     pfc_refptr_t *domain,
					     pfc_evfunc_t handler,
					     pfc_ptr_t PFC_RESTRICT arg,
					     const pfc_evmask_t *PFC_RESTRICT
					     evmask, uint32_t priority,
					     pfc_refptr_t *rhname);
extern int	pfc_event_domain_remove_handler(const char *name,
						const char *domain,
						const pfc_timespec_t
						*PFC_RESTRICT timeout);
extern int	pfc_event_domain_post(const char *name, const char *domain,
				      pfc_event_t event);
#endif	/* _PFC_LIBPFC_BUILD */

/*
 * Prototypes.
 */
extern void	pfc_event_register_evcall(evcall_hdr_t callback);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_EVENT_IMPL_H */
