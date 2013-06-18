/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_TIMER_HH
#define _PFCXX_TIMER_HH

/*
 * Definition for C++ timer that's a wrapper
 * for PFC core library for C lang
 */

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <string>
#include <pfc/timer.h>

namespace pfc {
namespace core {

/*
 * Type of Timer function
 */
typedef boost::function<void (void)> timer_func_t;

/*
 * Class Timer
 */

class Timer
        : boost::noncopyable
{
public:
    /*
     * factory method that creates timer instance.
     * timer handler runs from default thread pool.
     */
    static Timer* create(pfc_taskq_t tqid, const pfc_timespec_t *resolution=NULL);

    /*
     * factory method that creates timer instance.
     * timer handler runs from specified thread pool.
     */
    static Timer* create(pfc_taskq_t tqid, const std::string &pool_name,
                         const pfc_timespec_t *resolution=NULL);

    /*
     * destructor that destroy a timer
     */
    ~Timer();

    /*
     * post timer handler.
     */
    int post(const pfc_timespec_t *timeout, const timer_func_t &func,
             pfc_timeout_t *toidp);

    /*
     * cancel timer handler.
     */
    int cancel(const pfc_timeout_t toid);

    /*
     * getter method of timer id.
     */
    inline pfc_timer_t getId() {
        return _timer_id;
    }

private:
    /*
     * constructor of timer
     */
    explicit Timer(pfc_timer_t id);

    /*
     * factory method that creates timer instance.
     * timer handler runs from specified thread pool.
     */
    static Timer* create(pfc_taskq_t tqid, const char *pool_name,
                         const pfc_timespec_t *resolution = NULL);

    /*
     * stub function for post() to call pfc_timer_post()a
     */
    static void stub(void *arg);

    /*
     * argument destructor
     */
    static void argDtor(void *arg);

    pfc_timer_t _timer_id;
};

}
}

#endif  /* !_PFCXX_TIMER_HH */
