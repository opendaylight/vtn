/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_EVENT_H
#define	_PFC_EVENT_H

/*
 * Definitions for PFC event delivery system.
 */

#include <stdio.h>
#include <errno.h>
#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/clock.h>

PFC_C_BEGIN_DECL

/*
 * Event type.
 * Each event has an event type, integer in range of [0, 31].
 * Event generator can choose one event type for each event.
 */
typedef uint8_t		pfc_evtype_t;

#define	PFC_EVTYPE_MIN		PFC_CONST_U(0)
#define	PFC_EVTYPE_MAX		PFC_CONST_U(31)

#define	PFC_EVTYPE_IS_VALID(type)	((type) <= PFC_EVTYPE_MAX)

/*
 * Event types for the PFC system event.
 * You can obtain event source name by calling pfc_event_global_source().
 */
#define	PFC_EVTYPE_SYS_START	PFC_CONST_U(0)	/* PFC service start event */
#define	PFC_EVTYPE_SYS_STOP	PFC_CONST_U(1)	/* PFC shutdown start event */

/*
 * Event types for the PFC asynchronous system event.
 * You can obtain event source name by calling pfc_event_async_source().
 */
#define	PFC_EVTYPE_ASYNC_SIGNAL	PFC_CONST_U(0)	/* External signal event */

/*
 * Event types for the PFC debugging event.
 * You can obtain event source name by calling pfc_event_debug_source().
 */
#define	PFC_EVTYPE_DBG_STDUMP	PFC_CONST_U(0)	/* statistics dump */

/*
 * Serial ID assigned to each event object.
 */
typedef uint32_t	pfc_evid_t;

/*
 * Mask value for event type.
 * One bit in event mask corresponds to one event type.
 */
typedef uint32_t	pfc_evmask_t;

#define	PFC_EVENT_MASK_EMPTY		((pfc_evmask_t)0)
#define	PFC_EVENT_MASK_FILL		((pfc_evmask_t)-1)
#define	PFC_EVENT_MASK_BIT(type)	(PFC_CONST_U(1) << (type))

/*
 * Currently, event mask APIs are defined as inline function.
 */

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_event_mask_fill(pfc_evmask_t *mask)
 *	Fill all bits in the specified event mask, which means all supported
 *	event types are selected.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_event_mask_fill(pfc_evmask_t *mask)
{
	*mask = PFC_EVENT_MASK_FILL;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_event_mask_empty(pfc_evmask_t *mask)
 *	Clear all bits in the specified event mask, which means no event
 *	type is selected.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_event_mask_empty(pfc_evmask_t *mask)
{
	*mask = PFC_EVENT_MASK_EMPTY;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_event_mask_add(pfc_evmask_t *mask, pfc_evtype_t type)
 *	Add the specified event type to the event mask.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_event_mask_add(pfc_evmask_t *mask, pfc_evtype_t type)
{
	if (PFC_EXPECT_FALSE(!PFC_EVTYPE_IS_VALID(type))) {
		return EINVAL;
	}

	*mask |= PFC_EVENT_MASK_BIT(type);

	return 0;
}

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_event_mask_delete(pfc_evmask_t *mask, pfc_evtype_t type)
 *	Delete the specified event type from the event mask.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_event_mask_delete(pfc_evmask_t *mask, pfc_evtype_t type)
{
	if (PFC_EXPECT_FALSE(!PFC_EVTYPE_IS_VALID(type))) {
		return EINVAL;
	}

	*mask &= ~PFC_EVENT_MASK_BIT(type);

	return 0;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_event_mask_test(pfc_evmask_t *mask, pfc_evtype_t type)
 *	Return PFC_TRUE if the event mask contains the specified event type.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_event_mask_test(pfc_evmask_t *mask, pfc_evtype_t type)
{
	if (PFC_EXPECT_FALSE(!PFC_EVTYPE_IS_VALID(type))) {
		return PFC_FALSE;
	}

	return (*mask & PFC_EVENT_MASK_BIT(type)) ? PFC_TRUE : PFC_FALSE;
}

/*
 * Prototype of private data destructor in event object.
 */
typedef void	(*pfc_evdtor_t)(pfc_ptr_t data);

/*
 * Common event data.
 */
struct __pfc_event;
typedef struct __pfc_event	*pfc_event_t;

/*
 * Event data which means the event is invalid.
 */
#define	PFC_EVENT_INVALID	((pfc_event_t)NULL)

/*
 * Invalid event handler ID.
 */
#define	EVHANDLER_ID_INVALID	((pfc_evhandler_t)0U)

/*
 * Prototype of event handler function.
 */
typedef void		(*pfc_evfunc_t)(pfc_event_t event, pfc_ptr_t arg);

/*
 * Event handler ID.
 */
typedef uint32_t	pfc_evhandler_t;

/*
 * Prototypes.
 */
extern int		pfc_event_create(pfc_event_t *eventp, pfc_evtype_t type,
					 pfc_ptr_t data, pfc_evdtor_t dtor);
extern pfc_refptr_t	*pfc_event_refptr(pfc_event_t event);
extern pfc_evid_t	pfc_event_serial(pfc_event_t event);
extern pfc_evtype_t	pfc_event_type(pfc_event_t event);
extern pfc_ptr_t	pfc_event_data(pfc_event_t event);
extern pfc_event_t	pfc_event_get_event(pfc_refptr_t *ref);
extern const pfc_refptr_ops_t	*pfc_event_refops(void);

extern void	pfc_event_enable_log(pfc_event_t event, pfc_bool_t enable);

extern int	pfc_event_register(const char *name);
extern int	pfc_event_register_local(const char *name,
					 uint32_t maxthreads);
extern int	pfc_event_register_async(const char *name);
extern int	pfc_event_unregister(const char *PFC_RESTRICT name,
				     const pfc_timespec_t
				     *PFC_RESTRICT timeout);

extern int	pfc_event_add_handler_named(pfc_evhandler_t *PFC_RESTRICT idp,
					    const char *PFC_RESTRICT name,
					    pfc_evfunc_t handler,
					    pfc_ptr_t PFC_RESTRICT arg,
					    const pfc_evmask_t *PFC_RESTRICT
					    evmask, uint32_t priority,
					    const char *PFC_RESTRICT hname);
extern int	pfc_event_remove_handler(pfc_evhandler_t id,
					 const pfc_timespec_t *timeout);

/*
 * Although PFC_LOG_IDENT is preferred to be the default event handler's name,
 * pfc/log.h, which contains the definition of PFC_LOG_IDENT, can not be
 * included here because it causes recursive inclusion.
 */
#ifdef	__PFC_LOG_IDENT_DEFINED
/* pfc/log.h is already included. */
#define	__PFC_EVENT_HANDLER_NAME	PFC_LOG_IDENT
#else	/* !__PFC_LOG_IDENT_DEFINED */

#if	defined(PFC_MODULE_BUILD) && defined(MODULE_NAME)
/* Use module name on module source build. */
#define	__PFC_EVENT_HANDLER_NAME	MODULE_NAME
#elif	defined(PFC_LOG_IDENT)
/* Use user-defined log identifier. */
#define	__PFC_EVENT_HANDLER_NAME	PFC_LOG_IDENT
#else	/* !(PFC_MODULE_BUILD && MODULE_NAME) && !PFC_LOG_IDENT */
#define	__PFC_EVENT_HANDLER_NAME	NULL
#endif	/* defined(PFC_MODULE_BUILD) && defined(MODULE_NAME) */

#endif	/* __PFC_LOG_IDENT_DEFINED */

#define	pfc_event_add_handler(idp, name, handler, arg, evmask, priority) \
	pfc_event_add_handler_named(idp, name, handler, arg, evmask,	\
				    priority, __PFC_EVENT_HANDLER_NAME)

extern int	pfc_event_post(const char *name, pfc_event_t event);
extern int	pfc_event_post_emergency(const char *name, pfc_event_t event);
extern int	pfc_event_flush(const char *PFC_RESTRICT name,
				const pfc_timespec_t *PFC_RESTRICT timeout);

extern const char	*pfc_event_global_source(void);
extern const char	*pfc_event_async_source(void);
extern const char	*pfc_event_debug_source(void);

extern FILE	*pfc_event_statdump_file(pfc_event_t event);
extern uint32_t	pfc_event_statdump_verbose(pfc_event_t event);

/*
 * Create event object without private data.
 */
#define	PFC_EVENT_CREATE(eventp, type)				\
	(pfc_event_create((eventp), (type), NULL, NULL))

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_event_hold(pfc_event_t event)
 *	Increment references to the specified event object to keep event
 *	in memory. An application must decrement references by calling
 *	pfc_event_release() when it finishes using the event.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_event_hold(pfc_event_t event)
{
	pfc_refptr_t	*ref = pfc_event_refptr(event);

	pfc_refptr_get(ref);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_event_release(pfc_event_t event)
 *	Decrement references to the specified event.
 *	Note that the specified event may be destroyed after the call to
 *	this function.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_event_release(pfc_event_t event)
{
	pfc_refptr_t	*ref = pfc_event_refptr(event);

	pfc_refptr_put(ref);
}

/*
 * Obtain event private data with type cast.
 */
#define	PFC_EVENT_DATA(event, type)	((type)(uintptr_t)pfc_event_data(event))
#define	PFC_EVENT_DATA_INT(event)	PFC_EVENT_DATA(event, int)

PFC_C_END_DECL

#endif	/* !_PFC_EVENT_H */
