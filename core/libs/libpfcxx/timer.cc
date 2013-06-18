/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * timer.cc - PFC timer in the C++ world.
 */

#include <pfc/log.h>
#include <pfc/debug.h>
#include <boost/bind.hpp>
#include <pfcxx/timer.hh>

namespace pfc {
namespace core {

/*
 * Timer(pfc_timer_t id)
 *     Constructor.
 *     Set timer id.
 */
Timer::Timer(pfc_timer_t id)
{
    _timer_id = id;
}

/*
 * ~Timer()
 *     Destructor.
 *     Destroy timer.
 */
Timer::~Timer()
{
    int ret_val = pfc_timer_destroy(_timer_id);

    if (ret_val != 0) {
        pfc_log_error("Failed to destroy the timer that has id %u"
                      " (errno = %d)", _timer_id, ret_val);
        PFC_ASSERT(PFC_FALSE);
    }
}

/*
 * Timer *
 * Timer::create(pfc_taskq_t tqid, const pfc_timespec_t *resolution)
 *     Factory method of Timer.
 *     Return a new Timer instance.
 *     Timer handler run from default thread pool.
 */
Timer *
Timer::create(pfc_taskq_t tqid, const pfc_timespec_t *resolution)
{
    return Timer::create(tqid, NULL, resolution);
}

/*
 * Timer *
 * Timer::create(pfc_taskq_t tqid, const std::string &pool_name,
 *               const pfc_timespec_t *resolution)
 *     Factory method of Timer.
 *     Return a new Timer instance.
 *     Timer handler run from specified thread pool.
 */
Timer *
Timer::create(pfc_taskq_t tqid, const std::string &pool_name,
              const pfc_timespec_t *resolution)
{
    return Timer::create(tqid, pool_name.c_str(), resolution);
}

/*
 * Timer *
 * Timer::create(pfc_taskq_t tqid, const char *pool_name,
 *               const pfc_timespec_t *resolution)
 *     Factory method of Timer.
 *     Return a new Timer instance.
 *     Timer handler run from specified thread pool.
 */
Timer *
Timer::create(pfc_taskq_t tqid, const char *pool_name,
              const pfc_timespec_t *resolution)
{
    pfc_timer_t id;
    int ret_val = pfc_timer_create(&id, pool_name, tqid, resolution);

    if (ret_val != 0) {
        if (pool_name == NULL) {
            pfc_log_error("Failed to create a timer at default pool"
                          " (errno = %d)", ret_val);
        } else {
            pfc_log_error("Failed to create a timer at pool \"%s\""
                          " (errno = %d)", pool_name, ret_val);
        }
        return NULL;
    }

    try {
        return new Timer(id);
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to allocate a timer instance with id %u"
                      ": %s", id, ex.what());
        PFC_ASSERT_INT(pfc_timer_destroy(id), 0);
        return NULL;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to allocate a timer instance with id %u",
                      id);
        PFC_ASSERT_INT(pfc_timer_destroy(id), 0);
        return NULL;
    }
}

/*
 * int
 * Timer::post(const pfc_timespec_t *timeout, const timer_func_t &func,
 *             pfc_timeout_t *toidp)
 *     Post a timeout request to timer.
 *     Return result of pfc_timer_post().
 */
int
Timer::post(const pfc_timespec_t *timeout, const timer_func_t &func,
            pfc_timeout_t *toidp)
{
    timer_func_t *func_p;
    try {
        func_p = new timer_func_t(func);
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to copy a timer function object: %s",
                      ex.what());
        return ENOMEM;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to copy a timer function object");
        return EINVAL;
    }

    int ret_val = pfc_timer_post_dtor(_timer_id, timeout, Timer::stub,
                                      static_cast<void *>(func_p),
                                      Timer::argDtor, toidp);

    if (ret_val != 0) {
        /* Failed to post */
        pfc_log_error("Failed to post a handler into the timer that"
                      " has id %u (errno = %d)", _timer_id, ret_val);
        delete func_p;
    }

    return ret_val;
}

/*
 * int
 * Timer::cancel(const pfc_timeout_t toid)
 *     Cancel timeout function.
 *     Return result of pfc_timer_cancel().
 */
int
Timer::cancel(const pfc_timeout_t toid)
{
    return pfc_timer_cancel(_timer_id, toid);
}

/*
 * void *
 * Timer::stub(void *arg)
 *      Stub function for post() to call pfc_poller_post().
 *
 *      It's declared as private and static in header.
 */
void
Timer::stub(void *arg)
{
    PFCXX_TRY_ON_RELEASE() {
        timer_func_t *func = static_cast<timer_func_t*>(arg);
        (*func)();
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        pfc_log_error("Unexpected exception on timer handler: %s", ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        pfc_log_error("Unexpected exception on timer handler.");
    }
}

/*
 * void
 * Timer::argDtor(void *arg)
 *     For delete timer handler.
 */
void
Timer::argDtor(void *arg)
{
    delete static_cast<timer_func_t*>(arg);
}

}	// core
}	// pfc
