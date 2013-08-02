/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_TASK_QUEUE_HH
#define	_PFCXX_TASK_QUEUE_HH

/*
 * Definitions for C++ task queue management that's a wrapper
 * for PFC core library for C lang
 */

#include <string>
#include <exception>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <pfc/taskq.h>
#include <pfc/log.h>

namespace pfc {
namespace core {

/*
 * Type of Task Queue function
 */
typedef boost::function<void (void)> taskq_func_t;

class Task;

/*
 * Class for a task queue instance.
 *
 */
class TaskQueue
    : boost::noncopyable
{

public:
    /*
     * Factory method of Task Queue class.
     * Return a new task queue instance created from default thread pool.
     */
    static inline TaskQueue *create(uint32_t concurrency,
                                    const std::string &owner = "")
    {
        return create(NULL, concurrency,
                      ((owner == "") ? PFC_LOG_IDENT : owner.c_str()));
    }

    /*
     * Factory method of Task Queue class.
     * Return a new task queue instance created from specified thread pool.
     */
    static inline TaskQueue *create(const std::string &pool_name,
                                    uint32_t concurrency,
                                    const std::string &owner = "")
    {
        return create(pool_name.c_str(), concurrency,
                      ((owner == "") ? PFC_LOG_IDENT : owner.c_str()));
    }

    /*
     * a destructor that destroy a task queue
     */
    ~TaskQueue(void);

    /*
     * dispatch task
     */
    int dispatch(const taskq_func_t &func);

    /*
     * Enqueue a task and create a Task object.
     */
    Task *createTask(const taskq_func_t &func, uint32_t flags = 0);

    /*
     * set maxfree to task thread
     */
    int setFree(uint32_t maxfree=0);

    /*
     * flush the task queue
     */
    int flush(const pfc_timespec_t *ts = NULL);

    /*
     * clear the task queue
     */
    int clear(const pfc_timespec_t *ts = NULL);

    /*
     * get task queue ID
     */
    inline pfc_taskq_t getId() {
        return _tq_id;
    }

private:
    /*
     * constructor
     */
    explicit TaskQueue(pfc_taskq_t id);

    /*
     * a constructor that creates a task queue from specified thread pool
     */
    static TaskQueue *create(const char *pool_name, uint32_t concurrency,
                             const char *owner);

    /*
     * argument destructor
     */
    static void argDtor(void *arg);

    /* TaskQueue ID */
    pfc_taskq_t _tq_id;
};

/*
 * Class for a task instance.
 *
 */
class Task
    : boost::noncopyable
{
    /*
     * TaskQueue class calls private member functions
     */
    friend class TaskQueue;

public:
    /*
     * destructor
     */
    ~Task(void);

    /*
     * cancel the task
     */
    int cancel(void);

    /*
     * join the task
     */
    int join(void);

    /*
     * cancel the the task within the time
     */
    int timedJoin(const pfc_timespec_t *ts);

private:
    /*
     * constructor
     */
    explicit Task(pfc_taskq_t tqid);

    /*
     * set task id
     */
    void setTid(pfc_task_t tid);

    /*
     * a stub function for dispatch() to call pfc_taskq_dispatch()
     */
    static void stub(void *arg);

    /* Task Queue ID */
    pfc_taskq_t _tq_id;

    /* Task ID */
    pfc_task_t _t_id;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_TASK_QUEUE_HH */
