/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.cc - PFC event delivery system in the C++ world.
 */

#include <new>
#include <pfcxx/event.hh>
#include <pfc/atomic.h>
#include <pfc/log.h>
#include <pfc/hash.h>
#include <pfc/debug.h>
#include <event_impl.h>
#include "event_impl.hh"

namespace pfc {
namespace core {

/*
 * Internal prototypes.
 */
static void	event_handler_dtor(evhandler_t *ehp);

#define	EVENT_CAST_PTR(ptr, type)	(reinterpret_cast<type>(ptr))

#define	EVENT_OBJECT(object)	EVENT_CAST_PTR((object), Event *)
#define	EVENT_HANDLER(object)	EVENT_CAST_PTR((object), event_handler_t *)

/*
 * int
 * Event::registerEvent(const char *name)
 *	Register event source which delivers events using the global queue.
 *	All events posted to the event source registered by this function
 *	are delivered by one system global thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::registerEvent(const char *name)
{
    return pfc_event_register(name);
}

/*
 * int
 * Event::registerLocalEvent(const char *name, uint32_t maxthreads);
 *	Register event source which delivers events using event source local
 *	queue. `maxthreads' is the maximum number of threads to dispatch events.
 *
 *	Note that it is impossible to control order of calling event handler
 *	if you specify more than 1 to `maxthreads'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::registerLocalEvent(const char *name, uint32_t maxthreads)
{
    return pfc_event_register_local(name, maxthreads);
}

/*
 * int
 * Event::registerAsyncEvent(const char *name)
 *	Register event source which delivers events using async event queue.
 *	All events posted to the event source registered by this function
 *	are delivered asynchronously. It is impossible to control order of
 *	calling event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::registerAsyncEvent(const char *name)
{
    return pfc_event_register_async(name);
}

/*
 * int
 * Event::unregisterEvent(const char *name, const pfc_timespec_t *timeout)
 *	Unregister event source.
 *	All events in the event queue are discarded, and all handlers
 *	registered to this source are also unregistered.
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
Event::unregisterEvent(const char *name, const pfc_timespec_t *timeout)
{
    return pfc_event_unregister(name, timeout);
}

/*
 * int
 * Event::addHandler(pfc_evhandler_t &id, const char *name,
 *		     event_handler_t &handler, const pfc_evmask_t *maskp,
 *		     uint32_t priority, const char *hname)
 *	Common method to register event handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, event handler ID is set to `id',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function succeeds even if the specified event source is not yet
 *	registered. In that case, the handler will be bound to the event
 *	source when it is registered.
 */
int
Event::addHandler(pfc_evhandler_t &id, const char *name,
                  event_handler_t &handler, const pfc_evmask_t *maskp,
                  uint32_t priority, const char *hname)
{
    /* Copy event handler object. */
    try {
        event_handler_t	*func(new event_handler_t(handler));
        pfc_evhandler_t	hid;

        int err(pfc_event_add_handler_named(&hid, name, event_cxx_handler, func,
                                            maskp, priority, hname));
        if (PFC_EXPECT_FALSE(err != 0)) {
            delete func;
        }
        else {
            id = hid;
        }

        return err;
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to copy event handler: %s", ex.what());

        return ENOMEM;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to copy event handler.");
    }

    return EINVAL;
}

/*
 * int
 * Event::removeHandler(pfc_evhandler_t id, const pfc_timespec_t *timeout)
 *	Remove the event handler associated with the specified handler ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::removeHandler(pfc_evhandler_t id, const pfc_timespec_t *timeout)
{
    return pfc_event_remove_handler(id, timeout);
}

/*
 * int
 * Event:: post(const char *name, Event *eobj)
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
Event:: post(const char *name, Event *eobj)
{
    PFC_ASSERT(eobj != NULL);

    return pfc_event_post(name, eobj->getEvent());
}

/*
 * int
 * Event::postEmergency(const char *name, Event *eobj)
 *	Post an emergency event to the event source associated with the
 *	specified name.
 *
 *	Unlike Event::post(), this function posts an event to the head
 *	of event queue.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::postEmergency(const char *name, Event *eobj)
{
    PFC_ASSERT(eobj != NULL);

    return pfc_event_post_emergency(name, eobj->getEvent());
}

/*
 * int
 * Event::flush(const char *name, const pfc_timespec_t *timeout)
 *	Flush event queue for associated with the specified event source.
 *
 *	If this function returns zero, it is guaranteed that all events posted
 *	to the specified event source before the call of Event::flush() have
 *	been delivered to event handlers.
 *
 *	If `timeout' is NULL, Event::flush() waits until all events are
 *	flushed. If not NULL, Event::flush() only waits the specified
 *	timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
Event::flush(const char *name, const pfc_timespec_t *timeout)
{
    return pfc_event_flush(name, timeout);
}

/*
 * Event::Event(pfc_evtype_t type, pfc_ptr_t data)
 *	Create a new event object which has the given type and private data.
 *
 * Remarks:
 *	The caller must ensure getEvent() method of this object doesn't return
 *	PFC_EVENT_INVALID. If PFC_EVENT_INVALID is returned, the event object
 *	must not be delivered.
 */
Event::Event(pfc_evtype_t type, pfc_ptr_t data)
    : _refptr((pfc_refptr_t *)NULL)
{
    /* Create a new event object. */
    int	err(pfc_event_create(&_event, type, data, NULL));

    if (PFC_EXPECT_FALSE(err != 0)) {
        /* Set invalid event object. */
        _event = PFC_EVENT_INVALID;
    }
    else {
        event_t	*ev(PFC_EVENT_PTR(_event));

        /* Install this object and destructor. */
        ev->e_object = this;
        ev->e_objdtor = Event::dtor;

        /* Cache reference pointer to the event. */
        _refptr = ev->e_refptr;
    }
}

/*
 * Event(void)
 *	Create a new empty event object.
 *	This constructor is used to create C++ wrapper object.
 */
Event::Event(void)
    : _event(PFC_EVENT_INVALID), _refptr(NULL)
{
}

/*
 * Event *
 * Event::getInstance(pfc_event_t event)
 *	Return a C++ wrapper object associated with the specified event.
 *
 *	If an event object is already associated, it is returned.
 *	If not, a new object is created, and it is associated with the
 *	specified event atomically.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to event object is returned.
 *	NULL is returned on failure.
 */
Event *
Event::getInstance(pfc_event_t event)
{
    PFC_ASSERT(event != PFC_EVENT_INVALID);

    /* Check to see whether an event object is already associated or not. */
    event_t	*ev(PFC_EVENT_PTR(event));
    Event	*eobj(EVENT_OBJECT(ev->e_object));

    if (PFC_EXPECT_FALSE(eobj == NULL)) {
        /* Create a new event object. */
        try {
            eobj = new Event();
        }
        catch (const std::exception &ex) {
            /* Failed to create a new event object. */
            pfc_log_error("Failed to allocate event(%u,%u) object: %s",
                          ev->e_serial, ev->e_type, ex.what());

            return NULL;
        }
        PFCXX_CATCH_ALL() {
            /* Failed to create a new event object. */
            pfc_log_error("Failed to allocate event(%u,%u) object.",
                          ev->e_serial, ev->e_type);

            return NULL;
        }

        /*
         * Install this event object into raw event object.
         * Once ev->e_object is updated to non-NULL value, it is never be
         * updated. So we can use atomic operation to install object pointer.
         */
        pfc_ptr_t	ptr(pfc_atomic_cas_ptr(&ev->e_object,
                                               EVENT_CAST_PTR(eobj, pfc_ptr_t),
                                               NULL));
        if (PFC_EXPECT_FALSE(ptr != NULL)) {
            /* Lost the race. */
            delete eobj;

            eobj = EVENT_OBJECT(ptr);
        }

        /*
         * Initialize private members.
         * Although this code may run on two or more threads, we don't need
         * to acquire any lock because values to be set into the same Event
         * instance are always the same.
         */

        /* Install event to event object. */
        eobj->_event = event;
        eobj->_refptr = ev->e_refptr;

        /* Install object destructor. */
        ev->e_objdtor = Event::dtor;
    }

    return eobj;
}

/*
 * Event::~Event(void)
 *	Destructor of event object.
 *	All resources held by this event object must be released.
 */
Event::~Event(void)
{
}

/*
 * pfc_evid_t
 * Event::getSerial(void) const
 *	Return serial ID of this event.
 */
pfc_evid_t
Event::getSerial(void) const
{
    event_t	*ev(PFC_EVENT_PTR(_event));

    if (PFC_EXPECT_FALSE(ev == NULL)) {
        return 0;
    }

    return ev->e_serial;
}

/*
 * pfc_evtype_t
 * Event::getType(void) const
 *	Return type of this event.
 */
pfc_evtype_t
Event::getType(void) const
{
    event_t	*ev(PFC_EVENT_PTR(_event));

    if (PFC_EXPECT_FALSE(ev == NULL)) {
        return 0;
    }

    return ev->e_type;
}

/*
 * pfc_ptr_t
 * Event::getData(void) const
 *	Return pointer to private data in this event.
 */
pfc_ptr_t
Event::getData(void) const
{
    event_t	*ev(PFC_EVENT_PTR(_event));

    if (PFC_EXPECT_FALSE(ev == NULL)) {
        return NULL;
    }

    return ev->e_data;
}

/*
 * void
 * Event::destroyData(pfc_ptr_t data)
 *	Destructor of private data in this event.
 *
 * Remarks:
 *	This method of this class does nothing.
 *	If you need a destructor of the private data, you must override this
 *	method to implement code to free up private data.
 */
void
Event::destroyData(pfc_ptr_t data)
{
}

/*
 * static void
 * Event::dtor(pfc_ptr_t object)
 *	Object destructor of the event.
 *	This method is called when all references to the event object is
 *	removed.
 */
void
Event::dtor(pfc_ptr_t object)
{
    Event	*eobj(static_cast<Event *>(object));

    PFC_ASSERT(eobj != NULL);

    /*
     * Reference pointer and event_t are destroyed by the caller.
     * So only thing to do here is to destroy private data and event object.
     */
    pfc_event_t	event(eobj->_event);
    if (PFC_EXPECT_TRUE(event != PFC_EVENT_INVALID)) {
        event_t	*ev(PFC_EVENT_PTR(event));

        if (ev->e_data != NULL) {
            eobj->destroyData(ev->e_data);
        }
    }

    delete eobj;
}

/*
 * void PFC_ATTR_HIDDEN
 * event_cxx_handler(pfc_event_t event, pfc_ptr_t arg)
 *	Wrapper interface for event handler.
 *	This function is registered as event handler to the PFC event system.
 *
 * Remarks:
 *	The specified event may be lost if the system is very low on memory.
 */
void PFC_ATTR_HIDDEN
event_cxx_handler(pfc_event_t event, pfc_ptr_t arg)
{
    Event	*eobj(Event::getInstance(event));

    if (PFC_EXPECT_TRUE(eobj != NULL)) {
        event_handler_t	*handler(EVENT_HANDLER(arg));

        /* Call actual event handler specifying Event instance. */
        PFC_ASSERT(handler != NULL);
        PFCXX_TRY_ON_RELEASE() {
            (*handler)(eobj);
        }
        PFCXX_CATCH_ON_RELEASE(ex) {
            pfc_log_error("Unexpected exception on event handler: %s",
                          ex.what());
        }
        PFCXX_CATCH_ALL_ON_RELEASE() {
            pfc_log_error("Unexpected exception on event handler.");
        }
    }
}

/*
 * static void
 * event_handler_dtor(evhandler_t *ehp)
 *	Destructor of event handler.
 *	This function is registered as event handler removal callback.
 */
static void
event_handler_dtor(evhandler_t *ehp)
{
    if (ehp->eh_handler == event_cxx_handler) {
        PFC_ASSERT(ehp->eh_arg != NULL);

        event_handler_t	*handler(EVENT_HANDLER(ehp->eh_arg));
        delete handler;
    }
}

/*
 * static void PFC_FATTR_INIT
 * event_cxx_init(void)
 *	Initialize PFC event system interfaces for the C++ world.
 */
static void PFC_FATTR_INIT
event_cxx_init(void)
{
    /*
     * Register event handler removal callback.
     * This is used to delete boost::function instance created by
     * Event::addHandler().
     */
    pfc_event_register_evcall(event_handler_dtor);
}

}	// core
}	// pfc
