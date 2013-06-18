/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_EVENT_HH
#define	_PFCXX_EVENT_HH

/*
 * Definitions for event delivery system in the C++ world.
 */

#include <new>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <pfc/refptr.h>
#include <pfc/event.h>
#include <pfc/log.h>

namespace pfc {
namespace core {

/*
 * Type of event handler.
 */
class Event;
typedef boost::function<void (Event *)>	event_handler_t;

class Module;

/*
 * Event mask which determines event type to listen.
 */
class EventMask
{
    friend class Event;
    friend class Module;

public:
    /* Constructors */
    EventMask(void) : _mask(PFC_EVENT_MASK_FILL) {}
    EventMask(pfc_evtype_t type) : _mask(PFC_EVENT_MASK_BIT(type)) {}
    EventMask(const EventMask &mask) : _mask(mask._mask) {}

    /*
     * Fill all bits in the specified event mask, which means all supported
     * event types are selected.
     */
    inline void
    fill(void)
    {
        pfc_event_mask_fill(&_mask);
    }

    /*
     * Clear all bits in the specified event mask, which means no event
     * type is selected.
     */
    inline void
    empty(void)
    {
        pfc_event_mask_empty(&_mask);
    }

    /*
     * Add the specified event type to the event mask.
     * Note that this function returns error number if the specified event
     * type is invalid.
     */
    inline int
    add(pfc_evtype_t type)
    {
        return pfc_event_mask_add(&_mask, type);
    }

    /*
     * Remove the specified event type from the event mask.
     * Note that this function returns error number if the specified event
     * type is invalid.
     */
    inline int
    remove(pfc_evtype_t type)
    {
        return pfc_event_mask_delete(&_mask, type);
    }

    /*
     * Return PFC_TRUE if the event mask contains the specified event type.
     */
    inline pfc_bool_t
    test(pfc_evtype_t type)
    {
        return pfc_event_mask_test(&_mask, type);
    }

    /*
     * Return current mask value.
     */
    inline pfc_evmask_t &
    getValue(void)
    {
        return _mask;
    }

private:
    inline const pfc_evmask_t *
    getMask(void) const
    {
        return &_mask;
    }

    /* Mask value */
    pfc_evmask_t	_mask;
};

/*
 * Common event object class.
 */
class Event
    : boost::noncopyable
{
public:
    /* Default priority of event handler. */
    static const uint32_t	DEFAULT_PRIORITY = 10U;

    /*
     * Register event source which delivers events using the global queue.
     * All events posted to the event source registered by this function
     * are delivered by one system global thread.
     */
    static int	registerEvent(const char *name);

    /*
     * Register event source which delivers events using event source local
     * queue. `maxthreads' is the maximum number of threads to dispatch events.
     * If `maxthreads' is zero, the number of threads in the queue is
     * determined by the system.
     *
     * Note that it is impossible to control order of calling event handler
     * if you specify more than or equal 1 to `maxthreads'.
     */
    static int	registerLocalEvent(const char *name, uint32_t maxthreads = 0);

    /*
     * Register event source which delivers events using async event queue.
     * All events posted to the event source registered by this function
     * are delivered asynchronously. It is impossible to control order of
     * calling event handler.
     */
    static int	registerAsyncEvent(const char *name);

    /*
     * Wrapper methods to register event source specifying its name as
     * std::string.
     */
    static inline int
    registerEvent(std::string &name)
    {
        return registerEvent(name.c_str());
    }

    static inline int
    registerLocalEvent(std::string &name, uint32_t maxthreads = 0)
    {
        return registerLocalEvent(name.c_str(), maxthreads);
    }

    static inline int
    registerAsyncEvent(std::string &name)
    {
        return registerAsyncEvent(name.c_str());
    }

    /*
     * Unregister event source.
     * All events in the event queue are discarded, and all handlers
     * registered to this source are also unregistered.
     */
    static int	unregisterEvent(const char *name,
                                const pfc_timespec_t *timeout = NULL);

    /*
     * Wrapper method to unregister event source specifying its name as
     * std::string.
     */
    static inline int
    unregisterEvent(std::string &name, const pfc_timespec_t *timeout = NULL)
    {
        return unregisterEvent(name.c_str(), timeout);
    }

    /*
     * Add an event handler which receives events posted to the event source
     * specified by the name.
     * The handler registered by this function will receive all events
     * posted to the specified event source.
     *
     * If `hname' is specified, it is used as event handler's name.
     */
    static inline int
    addHandler(pfc_evhandler_t &id, const char *name, event_handler_t &handler,
               uint32_t priority = DEFAULT_PRIORITY,
               const char *hname = PFC_LOG_IDENT)
    {
        return addHandler(id, name, handler, NULL, priority, hname);
    }

    /*
     * Add an event handler which receives events posted to the event source
     * specified by the name.
     * Only events which have event type specified by the event mask bits
     * are delivered to the registered handler.
     *
     * If `hname' is specified, it is used as event handler's name.
     */
    static inline int
    addHandler(pfc_evhandler_t &id, const char *name, event_handler_t &handler,
               EventMask &mask, uint32_t priority = DEFAULT_PRIORITY,
               const char *hname = PFC_LOG_IDENT)
    {
        return addHandler(id, name, handler, mask.getMask(), priority, hname);
    }

    /*
     * Wrapper methods to add event handler specifying event source name as
     * std::string.
     *
     * If `hname' is specified, it is used as event handler's name.
     */
    static inline int
    addHandler(pfc_evhandler_t &id, std::string &name, event_handler_t &handler,
               uint32_t priority = DEFAULT_PRIORITY,
               const char *hname = PFC_LOG_IDENT)
    {
        return addHandler(id, name.c_str(), handler, NULL, priority, hname);
    }

    static inline int
    addHandler(pfc_evhandler_t &id, std::string &name, event_handler_t &handler,
               EventMask &mask, uint32_t priority = DEFAULT_PRIORITY,
               const char *hname = PFC_LOG_IDENT)
    {
        return addHandler(id, name.c_str(), handler, mask, priority, hname);
    }

    /*
     * Remove the event handler associated with the specified handler ID.
     */
    static int	removeHandler(pfc_evhandler_t id,
                              const pfc_timespec_t *timeout = NULL);

    /*
     * Post an event to the event source associated with the specified name.
     * The lifetime of the specified event is managed by the event system.
     *
     * The reference to the specified event is always decremented,
     * irrespective of the results of this function. You must not touch the
     * specified event object after this call.
     */
    static int post(const char *name, Event *eobj);

    /*
     * Post an emergency event to the event source associated with the
     * specified name.
     *
     * Unlike post(), this function posts an event to the head of event queue.
     */
    static int postEmergency(const char *name, Event *eobj);

    /*
     * Wrapper methods to post an event specifying event source name as
     * std::string.
     */
    static inline int
    post(std::string &name, Event *eobj)
    {
        return post(name.c_str(), eobj);
    }

    static inline int
    postEmergency(std::string &name, Event *eobj)
    {
        return postEmergency(name.c_str(), eobj);
    }

    /*
     * Flush event queue associated with the specified event source.
     */
    static int	flush(const char *name, const pfc_timespec_t *timeout = NULL);

    static inline int
    flush(std::string &name, const pfc_timespec_t *timeout = NULL)
    {
        return flush(name.c_str(), timeout);
    }

    /*
     * Return a C++ wrapper object associated with the specified event.
     * Note that NULL is returned on failure.
     */
    static Event	*getInstance(pfc_event_t event);

    /*
     * Return a C++ wrapper object associated with the given refptr object.
     * Note that NULL is returned on failure.
     */
    static inline Event *
    getInstance(pfc_refptr_t *ref)
    {
        pfc_event_t	event(pfc_event_get_event(ref));

        if (PFC_EXPECT_FALSE(event == PFC_EVENT_INVALID)) {
            return NULL;
        }

        return Event::getInstance(event);
    }

    /*
     * Increment references to the specified event object to keep event object
     * in memory. An application must decrement references by calling release()
     * when it finishes using the event object.
     */
    static inline void
    hold(Event *evt)
    {
        pfc_refptr_t	*ref(evt->getRefPointer());

        if (PFC_EXPECT_TRUE(ref != NULL)) {
            pfc_refptr_get(ref);
        }
    }

    /*
     * Decrement references to the specified event object.
     * Note that the specified event object may be destroyed after the call
     * to this method.
     */
    static inline void
    release(Event *evt)
    {
        pfc_refptr_t	*ref(evt->getRefPointer());

        if (PFC_EXPECT_TRUE(ref != NULL)) {
            pfc_refptr_put(ref);
        }
    }

    /*
     * Returns a pointer to refptr operation for event object.
     */
    static inline const pfc_refptr_ops_t *
    getRefOperation(void)
    {
        return pfc_event_refops();
    }

    /*
     * Create a new event object which has the given type and private data.
     */
    Event(pfc_evtype_t type, pfc_ptr_t data = NULL);

    /*
     * Destructor of an event object.
     */
    virtual ~Event(void);

    /*
     * Return raw event object.
     * Note that this may return PFC_EVENT_INVALID if the creation of event
     * object fails.
     */
    inline pfc_event_t
    getEvent(void) const
    {
        return _event;
    }

    /*
     * Return reference pointer for this object.
     */
    inline pfc_refptr_t *
    getRefPointer(void) const
    {
        return _refptr;
    }

    /*
     * Enable or disable delivery logging.
     * Note that this method does nothing if this instance does not keep
     * valid event object.
     */
    inline void
    enableLog(pfc_bool_t enable)
    {
        if (PFC_EXPECT_TRUE(_event != PFC_EVENT_INVALID)) {
            pfc_event_enable_log(_event, enable);
        }
    }

    /*
     * Return serial ID of this event.
     */
    pfc_evid_t		getSerial(void) const;

    /*
     * Return type of this event.
     */
    pfc_evtype_t	getType(void) const;

    /*
     * Return private data in this event.
     */
    pfc_ptr_t		getData(void) const;

    /*
     * Destructor of private data.
     * Note that this method of this class does nothing.
     */
    virtual void	destroyData(pfc_ptr_t data);

private:
    /* Internal constructor to create wrapper event object. */
    Event(void);

    /* Wrapper interface to destroy event object. */
    static void	dtor(pfc_ptr_t object);

    /* Register event handler. */
    static int	addHandler(pfc_evhandler_t &id, const char *name,
                           event_handler_t &handler, const pfc_evmask_t *maskp,
                           uint32_t priority, const char *hname);

    /* Raw event object. */
    pfc_event_t	_event;

    /* Reference pointer object. */
    pfc_refptr_t *_refptr;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_EVENT_HH */
