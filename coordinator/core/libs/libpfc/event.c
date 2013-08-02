/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - PFC event object.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pfc/atomic.h>
#include <pfc/clock.h>
#include <pfc/log.h>
#include <pfc/config.h>
#include <pfc/util.h>
#include "event_impl.h"
#include "libpfc_impl.h"

/*
 * Prime number to generate hash value.
 */
#define	PFC_EVENT_LARGE_PRIME		62497459U

/*
 * Internal prototypes.
 */
static void		pfc_event_dtor(pfc_ptr_t object);
static int		pfc_event_compare(pfc_cptr_t o1, pfc_cptr_t o2);
static pfc_bool_t	pfc_event_equals(pfc_cptr_t o1, pfc_cptr_t o2);
static uint32_t		pfc_event_hashfunc(pfc_cptr_t object);

/*
 * Reference pointer operations for event object.
 */
const pfc_refptr_ops_t	event_refptr_ops PFC_ATTR_HIDDEN = {
	.dtor		= pfc_event_dtor,
	.compare	= pfc_event_compare,
	.equals		= pfc_event_equals,
	.hashfunc	= pfc_event_hashfunc,
};

/*
 * Invalid serial ID.
 */
#define	PFC_EVENT_SERIAL_INVALID	PFC_CONST_U(0)

/*
 * Event serial number for the next allocation.
 */
static uint32_t		event_serial_next = PFC_EVENT_SERIAL_INVALID;

/*
 * static inline pfc_evid_t
 * pfc_event_serial_alloc(void)
 *	Allocate a new event serial ID.
 *
 * Remarks:
 *	We assume that an event object is destroyed in very short term.
 *	So we don't consider serial ID overflow.
 */
static inline pfc_evid_t
pfc_event_serial_alloc(void)
{
	pfc_evid_t	id;

	id = pfc_atomic_inc_uint32_old(&event_serial_next) + 1;
	if (PFC_EXPECT_FALSE(id == PFC_EVENT_SERIAL_INVALID)) {
		id = pfc_atomic_inc_uint32_old(&event_serial_next) + 1;
	}

	return id;
}

/*
 * int
 * pfc_event_create(pfc_event_t *eventp, pfc_evtype_t type, pfc_ptr_t data,
 *		    pfc_evdtor_t dtor)
 *	Create a new event object which has the specified event type.
 *
 *	`data' is a private data which is delivered to event listeners.
 *	`dtor' is a destructor of private data. If the event object is
 *	destroyed, `dtor' is called with specifying private data.
 *	NULL can be passed to `data' or `dtor' if you don't need to set
 *	private data or destructor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to new event object is set to
 *	`*eventp', and zero is returned. Otherwise error number which indicates
 *	the cause of error is returned.
 */
int
pfc_event_create(pfc_event_t *eventp, pfc_evtype_t type, pfc_ptr_t data,
		 pfc_evdtor_t dtor)
{
	return pfc_event_create_flags(eventp, type, data, dtor, 0);
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_event_create_flags(pfc_event_t *eventp, pfc_evtype_t type,
 *			  pfc_ptr_t data, pfc_evdtor_t dtor, uint16_t flags)
 *	Create a new event object which has the given type and flags.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to new event object is set to
 *	`*eventp', and zero is returned. Otherwise error number which indicates
 *	the cause of error is returned.
 */
int PFC_ATTR_HIDDEN
pfc_event_create_flags(pfc_event_t *eventp, pfc_evtype_t type, pfc_ptr_t data,
		       pfc_evdtor_t dtor, uint16_t flags)
{
	event_t		*ev;
	pfc_refptr_t	*ref;

	PFC_ASSERT(eventp != NULL);

	/* Ensure that event type is valid. */
	if (PFC_EXPECT_FALSE(!PFC_EVTYPE_IS_VALID(type))) {
		return EINVAL;
	}

	/* Allocate event object. */
	ev = (event_t *)malloc(sizeof(*ev));
	if (PFC_EXPECT_FALSE(ev == NULL)) {
		return ENOMEM;
	}

	if (event_log_force) {
		/* Force to set event delivery logging flag. */
		flags |= EVF_LOG;
	}

	ev->e_type = type;
	ev->e_flags = flags;
	ev->e_data = data;
	ev->e_dtor = dtor;
	ev->e_object = NULL;
	ev->e_objdtor = NULL;

	/* Create reference pointer. */
	ref = pfc_refptr_create(&event_refptr_ops, (pfc_ptr_t)ev);
	if (PFC_EXPECT_FALSE(ref == NULL)) {
		free(ev);

		return ENOMEM;
	}

	ev->e_serial = pfc_event_serial_alloc();
	ev->e_refptr = ref;
	*eventp = ev;

	return 0;
}

/*
 * pfc_refptr_t *
 * pfc_event_refptr(pfc_event_t event)
 *	Return reference pointer object which manages the specified event
 *	object.
 */
pfc_refptr_t *
pfc_event_refptr(pfc_event_t event)
{
	return PFC_EVENT_PTR(event)->e_refptr;
}

/*
 * pfc_evid_t
 * pfc_event_serial(pfc_event_t event)
 *	Return serial event ID assigned to the specified event object.
 */
pfc_evid_t
pfc_event_serial(pfc_event_t event)
{
	return PFC_EVENT_PTR(event)->e_serial;
}

/*
 * pfc_evtype_t
 * pfc_event_type(pfc_event_t event)
 *	Return event type associated with the specified event object.
 */
pfc_evtype_t
pfc_event_type(pfc_event_t event)
{
	return PFC_EVENT_PTR(event)->e_type;
}

/*
 * pfc_ptr_t
 * pfc_event_data(pfc_event_t event)
 *	Return private data set by event generator.
 */
pfc_ptr_t
pfc_event_data(pfc_event_t event)
{
	return PFC_EVENT_PTR(event)->e_data;
}

/*
 * pfc_event_t
 * pfc_event_get_event(pfc_refptr_t *ref)
 *	Return an event object associated with the given refptr object.
 *
 * Calling/Exit State:
 *	If the given refptr object contains an event object, an event object
 *	is returned. PFC_EVENT_INVALID is returned if not.
 */
pfc_event_t
pfc_event_get_event(pfc_refptr_t *ref)
{
	if (PFC_EXPECT_FALSE(ref == NULL)) {
		/* Invalid refptr object. */
		return PFC_EVENT_INVALID;
	}

	if (PFC_EXPECT_FALSE(pfc_refptr_operation(ref) != &event_refptr_ops)) {
		/* Not an event object. */
		return PFC_EVENT_INVALID;
	}

	return PFC_REFPTR_VALUE(ref, pfc_event_t);
}

/*
 * const pfc_refptr_ops_t *
 * pfc_event_refops(void)
 *	Returns a pointer to refptr operation for event object.
 */
const pfc_refptr_ops_t *
pfc_event_refops(void)
{
	return &event_refptr_ops;
}

/*
 * void
 * pfc_event_enable_log(pfc_event_t event, pfc_bool_t enable)
 *	Enable or disable delivery logging on the given event.
 *
 *	If PFC_TRUE is passed to `enable', a log message is recorded into
 *	the PFC log when the given event is delivered to event handlers.
 *
 *	If PFC_FALSE is passed to `enable', event delivery logging on the
 *	given event is disabled.
 */
void
pfc_event_enable_log(pfc_event_t event, pfc_bool_t enable)
{
	event_t	*ev;

	if (event_log_force) {
		/* Don't change logging flag. */
		return;
	}

	ev = PFC_EVENT_PTR(event);
	if (enable) {
		/* Enable event delivery logging. */
		pfc_atomic_or_uint16(&ev->e_flags, EVF_LOG);
	}
	else {
		pfc_atomic_and_uint16(&ev->e_flags, (uint16_t)~EVF_LOG);
	}
}

/*
 * static void
 * pfc_event_dtor(pfc_ptr_t object)
 *	Destructor of the event object.
 */
static void
pfc_event_dtor(pfc_ptr_t object)
{
	event_t	*ev = PFC_EVENT_PTR(object);

	if (ev->e_dtor != NULL && ev->e_data != NULL) {
		/* Call user-specified destructor. */
		ev->e_dtor(ev->e_data);
	}

	if (ev->e_objdtor != NULL) {
		/* Call object destructor. */
		PFC_ASSERT(ev->e_object != NULL);
		ev->e_objdtor(ev->e_object);
	}

	free(ev);
}

/*
 * static int
 * pfc_event_compare(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Compare event object.
 *
 *	Comparison is done by event serial ID.
 */
static int
pfc_event_compare(pfc_cptr_t o1, pfc_cptr_t o2)
{
	event_t		*ev1 = PFC_EVENT_PTR(o1);
	event_t		*ev2 = PFC_EVENT_PTR(o2);
	pfc_evid_t	s1 = ev1->e_serial;
	pfc_evid_t	s2 = ev2->e_serial;

	if (s1 == s2) {
		return 0;
	}

	return (s1 < s2) ? -1 : 1;
}

/*
 * static pfc_bool_t
 * pfc_event_equals(pfc_cptr_t o1, pfc_cptr_t o2)
 *	Determine whether the given two event objects are identical or not.
 *
 *	Return true if they have the same serial ID.
 */
static pfc_bool_t
pfc_event_equals(pfc_cptr_t o1, pfc_cptr_t o2)
{
	event_t		*ev1 = PFC_EVENT_PTR(o1);
	event_t		*ev2 = PFC_EVENT_PTR(o2);
	pfc_evid_t	s1 = ev1->e_serial;
	pfc_evid_t	s2 = ev2->e_serial;

	return (s1 == s2) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static uint32_t
 * pfc_event_hashfunc(pfc_cptr_t object)
 *	Generate hash value of the event object.
 */
static uint32_t
pfc_event_hashfunc(pfc_cptr_t object)
{
	event_t	*ev = PFC_EVENT_PTR(object);

	return ev->e_serial * PFC_EVENT_LARGE_PRIME;
}
