/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * task_queue.cc - task queue management for C++
 */

#include <pfc/log.h>
#include <pfc/debug.h>
#include <pfcxx/task_queue.hh>

namespace pfc {
namespace core {

/*
 * TaskQueue*
 * TaskQueue::create(const char *pool_name, uint32_t concurrency,
 *                   const char *owner)
 *      Factory method of Task Queue class.
 *      Return a new task queue instance.
 */
TaskQueue *
TaskQueue::create(const char *pool_name, uint32_t concurrency,
                  const char *owner)
{
    pfc_taskq_t id;
    int ret_val = pfc_taskq_create_named(&id, pool_name, concurrency, owner);

    if (ret_val != 0) {
        if (pool_name == NULL) {
            pfc_log_error("Failed to create a task queue at default pool"
                          " (errno = %d)", ret_val);
        } else {
            pfc_log_error("Failed to create a task queue at pool \"%s\""
                          " (errno = %d)", pool_name, ret_val);
        }
        return NULL;
    }

    try {
        return new TaskQueue(id);
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to allocate a task queue instance"
                      " with id %u: %s", id, ex.what());
        PFC_ASSERT_INT(pfc_taskq_destroy(id), 0);
        return NULL;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to allocate a task queue instance"
                      " with id %u", id);
        PFC_ASSERT_INT(pfc_taskq_destroy(id), 0);
        return NULL;
    }
}

/*
 * TaskQueue::TaskQueue(void)
 *      Constructor of task queue class.
 *      Set task queue id.
 */
TaskQueue::TaskQueue(pfc_taskq_t id)
{
    _tq_id = id;
}

/*
 * TaskQueue::~TaskQueue(void)
 *      Destructor of task queue class.
 *      Destroy the task queue.
 */
TaskQueue::~TaskQueue(void)
{
    int ret_val = pfc_taskq_destroy(_tq_id);

    if (ret_val != 0) {
        pfc_log_error("Failed to destroy the task queue that has id %u"
                      " (errno = %d)", _tq_id, ret_val);
        PFC_ASSERT(PFC_FALSE);
    }
}

/*
 * int
 * TaskQueue::dispatch(const taskq_func_t &func)
 *      Return result of pfc_taskq_dispatch().
 *      It can't join() specified task.
 */
int
TaskQueue::dispatch(const taskq_func_t &func)
{
    taskq_func_t *func_p;
    try {
        func_p = new taskq_func_t(func);
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to copy a non-joinable task function object"
                      ": %s", ex.what());
        return ENOMEM;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to copy a non-joinable task function object");
        return EINVAL;
    }

    pfc_task_t tid;
    int ret_val = pfc_taskq_dispatch_dtor(_tq_id, Task::stub,
                                          static_cast<void *>(func_p),
                                          TaskQueue::argDtor, 0, &tid);
    if (ret_val != 0) {
        /* Failed to dispatch */
        pfc_log_error("Failed to dispatch a non-joinable task into"
                      " the task queue that has id %u (errno = %d)",
                      _tq_id, ret_val);
        delete func_p;
    }

    return ret_val;
}

/*
 * Task*
 * TaskQueue::createTask(const taskq_func_t &func, uint32_t flags = 0)
 *      Enqueue a task and create a Task object.
 *      Return result of pfc_taskq_dispatch().
 *      It can join specified task if flags = JOINABLE.
 */
Task *
TaskQueue::createTask(const taskq_func_t &func, uint32_t flags)
{
    /* copy taskq_func_t */
    taskq_func_t *func_p;
    try {
        func_p = new taskq_func_t(func);
    }
    catch (const std::exception &ex) {
        pfc_log_error("Failed to copy a task function object: %s",
                      ex.what());
        return NULL;
    }
    PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to copy a task function object");
        return NULL;
    }

    /* Create a new Task object before dispatching */
    Task *task;
    try {
        task = new Task(_tq_id);
        /* Don't forget to set task id later ! */
    } catch (const std::exception &ex) {
        pfc_log_error("Failed to allocate a task instance: %s", ex.what());
        delete func_p;
        return NULL;
    } PFCXX_CATCH_ALL() {
        pfc_log_error("Failed to allocate a task instance.");
        delete func_p;
        return NULL;
    }

    pfc_task_t tid;
    int ret_val = pfc_taskq_dispatch_dtor(_tq_id, Task::stub,
                                          static_cast<void *>(func_p),
                                          TaskQueue::argDtor, flags, &tid);

    if (ret_val != 0) {
        /* Failed to dispatch */
        pfc_log_error("Failed to dispatch a task into the task queue"
                      " that has id %u (errno = %d)", _tq_id, ret_val);
        delete func_p;
        delete task;
        return NULL;
    }

    task->setTid(tid);

    return task;
}

/*
 * int
 * TaskQueue::setFree(uint32_t maxfree=0)
 *      Set max free thread number.
 *      Return pfc_taskq_set_free().
 */
int
TaskQueue::setFree(uint32_t maxfree)
{
    return pfc_taskq_set_free(_tq_id, maxfree);
}

/*
 * int
 * TaskQueue::flush(const pfc_timespec_t *ts)
 *      Flush the specified task queue.
 */
int
TaskQueue::flush(const pfc_timespec_t *ts)
{
    return pfc_taskq_flush(_tq_id, ts);
}

/*
 * int
 * TaskQueue::clear(const pfc_timespec_t *ts)
 *      Clear the specified task queue.
 */
int
TaskQueue::clear(const pfc_timespec_t *ts)
{
    return pfc_taskq_clear(_tq_id, ts);
}

/*
 * void
 * TaskQueue::argDtor(void *arg)
 *      Destructor of taskq_func_t
 */
void
TaskQueue::argDtor(void *arg)
{
    delete static_cast<taskq_func_t*>(arg);
}

/*
 * Task::Task(pfc_taskq_t tqid)
 *     Constructor of task.
 *     Set task queue id.
 *     Task id is temporarily set as PFC_TASKQ_INVALID_TASKID and it must
 *     be updated later.
 */
Task::Task(pfc_taskq_t tqid)
{
    _tq_id = tqid;
    _t_id  = PFC_TASKQ_INVALID_TASKID;
}

/*
 * Task::~Task(void)
 *     Destructor of task.
 */
Task::~Task(void)
{
}

/*
 * int
 * Task::cancel()
 *     Cancel the task.
 *     Return result of pfc_taskq_cancel().
 */
int
Task::cancel()
{
    return pfc_taskq_cancel(_tq_id, _t_id);
}

/*
 * int
 * Task::join()
 *     Block the calling thread until the task associated with this
 *     instance quits.
 *     Return the result of pfc_taskq_join().
 */
int
Task::join()
{
    return pfc_taskq_join(_tq_id, _t_id);
}

/*
 * int
 * Task::timedJoin(const pfc_timespec_t *ts)
 *     Block the calling thread until the task associated with this instance
 *     quits within the specified timeout.
 *     Return the result of pfc_taskq_timedjoin().
 */
int
Task::timedJoin(const pfc_timespec_t *ts)
{
    return pfc_taskq_timedjoin(_tq_id, _t_id, ts);
}

/*
 * void
 * Task::setTid(pfc_task_t tid)
 *     Set task id.
 *     It must be called just once after creating instance by
 *     Task::Task(pfc_taskq_t).
 */
void
Task::setTid(pfc_task_t tid)
{
    PFC_ASSERT(_t_id == PFC_TASKQ_INVALID_TASKID);
    _t_id = tid;
}

/*
 * void
 * Task::stub(void *arg)
 *      Stub function for dispatch() to call pfc_taskq_dispatch().
 *
 *      It's declared as private and static in header.
 */
void
Task::stub(void *arg)
{
    PFCXX_TRY_ON_RELEASE() {
        taskq_func_t *func = static_cast<taskq_func_t*>(arg);
        (*func)();
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        pfc_log_error("Unexpected exception on taskq function: %s", ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        pfc_log_error("Unexpected exception on taskq function.");
    }
}

}// core
}// pfc
