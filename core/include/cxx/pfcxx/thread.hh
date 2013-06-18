/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_THREAD_HH
#define _PFCXX_THREAD_HH

/*
 * Definitions for C++ thread management that's a wrapper
 * for PFC core library for C lang
 */

#include <new>
#include <string>
#include <exception>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <pfc/thread.h>

namespace pfc {
namespace core {

/*
 * Class for threads.
 * C lang thread functions (pfc_thread_*) are called internally.
 *
 * Remarks:
 *   The life cycles of instances and threads are independent.
 *   It means:
 *     - After releasing an instance, a threads keeps running
 *     - After a thread exits, an instance remains
 *   Instances should be correctly released by the user.
 */
class Thread
        : boost::noncopyable
{
  public:
    typedef boost::function<void *(void)> thr_func_t;

    /*
     * a constructor that creates a thread from the default thread pool
     */
    Thread(const boost::function<void *(void)> &func,
           uint32_t flags = 0);

    /*
     * a constructor that creates a thread from the specified thread pool
     */
    Thread(const std::string &pool_name,
           const boost::function<void *(void)> &func,
           uint32_t flags = 0);

    /*
     * a constructor that binds the specified thread
     */
    explicit Thread(pfc_thread_t id);

    /*
     * a destructor (that doesn't stop the thread)
     */
    ~Thread(void);

    /*
     * return the status of creating a new thread by constructor
     */
    int getCreateStatus(void);

    /*
     * a factory method to creates a thread from the default thread pool
     */
    static Thread *create(const boost::function<void *(void)> &func,
                          uint32_t flags = 0);

    /*
     * a factory method to creates a thread from the specified thread pool
     */
    static Thread *create(const std::string &pool_name,
                          const boost::function<void *(void)> &func,
                          uint32_t flags = 0);

    /*
     * a factory method to bind the specified thread
     */
    static Thread *create(pfc_thread_t id);

    /*
     * block the calling thread until the thread of the instance quits
     */
    int join(void **statusp);

    /*
     * block the calling thread until the thread of the instance quits
     * within the specified timeout
     */
    int timedJoin(void **statusp, const pfc_timespec_t *to);

    /*
     * detach the thread
     */
    int detach(void);

    /*
     * exit the calling thread
     */
    static void exit(void *status);

    /*
     * return the instance of the calling thread
     */
    static Thread *self(void);

    /*
     * return the thread ID that's compatible with C library
     */
    pfc_thread_t getId();

    /*
     * call function specified to `init' only once
     */
    static int once(pfc_once_t *control,
                    pfc_once_init_t init_routine);

  private:
    /*
     * create a new thread
     */
    void create_thread(const char *pool_name,
                       const boost::function<void *(void)> &func,
                       int flags);

    /*
     * a stub function for thread creation
     * create_thread() passes this function to pfc_thread_create()
     */
    static void *thr_stub(void *arg);

    /*
     * release a function object copied on heap
     */
    static void delete_func(void *arg);

    /* status of creating thread */
    int _create_status;

    /* thread ID */
    pfc_thread_t _thr_id;
};

/*
 * Class for Thread Specific Data (TSD)
 */
class ThreadDataKey
        : boost::noncopyable
{
  public:
    typedef boost::function<void (pfc_ptr_t value)> tsd_dtor_func_t;

    ~ThreadDataKey(void);

    static ThreadDataKey *create(pfc_tsd_dtor_t dtor = NULL);
    static ThreadDataKey *create(const tsd_dtor_func_t &dtor);

    inline int
    set(pfc_cptr_t value)
    {
        return pfc_thread_setspecific(_key, value);
    }

    inline pfc_ptr_t
    get(void)
    {
        return pfc_thread_getspecific(_key);
    }

  private:
    explicit ThreadDataKey(pfc_tsd_key_t key)
            : _key(key) {}

    explicit ThreadDataKey(const tsd_dtor_func_t &dtor)
            : _key(PFC_TSD_KEY_INVALID), _dtor(dtor) {}

    void call_dtor(pfc_ptr_t value);

    static void stub(pfc_ptr_t value, pfc_ptr_t arg);

    pfc_tsd_key_t _key;
    tsd_dtor_func_t _dtor;
};

}       // core
}       // pfc

#endif  /* !_PFCXX_THREAD_HH */
