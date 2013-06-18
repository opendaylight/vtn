/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * thread.cc - thread management for C++
 */

#include <pfc/log.h>
#include <pfc/hash.h>
#include <pfc/synch.h>
#include <pfcxx/thread.hh>
#include <thread_impl.h>
#include <pthread.h>

#ifdef  PFC_VERBOSE_DEBUG
#define THREAD_LOG_ERROR(FMT, ...)                                  \
    pfc_log_error("%s(%d) " FMT, __FILE__, __LINE__, ##__VA_ARGS__)
#else   /* PFC_VERBOSE_DEBUG */
#define THREAD_LOG_ERROR(FMT, ...)              \
    pfc_log_error(FMT, ##__VA_ARGS__)
#endif  /* PFC_VERBOSE_DEBUG */

namespace pfc {
namespace core {

/*
 * Thread::Thread(const boost::function<void *(void)> &func,
 *                uint32_t flags)
 *      Constructor of Thread class.
 *      It creates a new thread from the default thread pool.
 *      Use getCreateStatus() to refer the result of creating it.
 */
Thread::Thread(const boost::function<void *(void)> &func,
               uint32_t flags)
{
    create_thread(NULL, func, flags);
}

/*
 * Thread::Thread(const std::string &pool_name,
 *                const boost::function<void *(void)> &func,
 *                uint32_t flags)
 *      Constructor of Thread class.
 *      It creates a new thread from the specified thread pool.
 *      Use getCreateStatus() to refer the result of creating it.
 */
Thread::Thread(const std::string &pool_name,
               const boost::function<void *(void)> &func,
               uint32_t flags)
{
    create_thread(pool_name.c_str(), func, flags);
}

/*
 * Thread::Thread(pfc_thread_t id)
 *      Constructor of Thread class.
 *      It binds the instance with the specified thread instead of
 *      creating a new one.
 *
 * Remarks:
 *      getCreateStatus() function of the instance always returns
 *      zero regardless an actual status of a thread.
 */
Thread::Thread(pfc_thread_t id)
{
    _thr_id = id;
    _create_status = 0;
}

/*
 * Thread::~Thread(void)
 *      Destructor of Thread class.
 */
Thread::~Thread(void)
{
}

/*
 * int
 * Thread::getCreateStatus(void)
 *      Return the result of creating a new thread in constructor.
 *      Zero shows success and others shows failure.
 */
int
Thread::getCreateStatus(void)
{
    return _create_status;
}

/*
 * Thread *
 * Thread::create(const boost::function<void *(void)> &func,
 *                uint32_t flags = 0)
 *      A factory method to creates a thread from the default thread
 *      pool.
 *
 *      If an error is occurred, NULL is returned.
 *
 *      It's declared as static in the header.
 */
/* static */
Thread *
Thread::create(const boost::function<void *(void)> &func,
               uint32_t flags)
{
    Thread *thr;
    try {
        thr = new Thread(func, flags);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a thread instance"
                         " (default pool): %s", ex.what());
        thr = NULL;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a thread instance"
                         " (default pool)");
        thr = NULL;
    }

    if (thr != NULL && thr->getCreateStatus() != 0) {
        delete thr;
        thr = NULL;
    }

    return thr;
}

/*
 * Thread *
 * Thread::create(const std::string &pool_name,
 *                const boost::function<void *(void)> &func,
 *                uint32_t flags)
 *      A factory method to creates a thread from the specified thread
 *      pool.
 *
 *      If an error is occurred, NULL is returned.
 *
 *      It's declared as static in the header.
 */
/* static */
Thread *
Thread::create(const std::string &pool_name,
               const boost::function<void *(void)> &func,
               uint32_t flags)
{
    Thread *thr;
    try {
        thr = new Thread(pool_name, func, flags);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a thread instance"
                         " (pool = \"%s\"): %s", pool_name.c_str(),
                         ex.what());
        thr = NULL;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a thread instance"
                         " (pool = \"%s\")", pool_name.c_str());
        thr = NULL;
    }

    if (thr != NULL && thr->getCreateStatus() != 0) {
        delete thr;
        thr = NULL;
    }

    return thr;
}

/*
 * Thread *
 * Thread::create(pfc_thread_t id)
 *      A factory method to bind the specified thread.
 *
 *      It's declared as static in the header.
 *
 * Remarks:
 *      This function always returns an instance regardless of actual
 *      status of a thread.
 *      This behavior originates Thread::Thread(pfc_thread_t id).
 */
/* static */
Thread *
Thread::create(pfc_thread_t id)
{
    Thread *obj;
    try {
        obj = new Thread(id);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a thread instance (id = %u)"
                         ": %s", id, ex.what());
        obj = NULL;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a thread instance (id = %u)",
                         id);
        obj = NULL;
    }

    return obj;
}

/*
 * int
 * Thread::join(void **statusp)
 *      Wait for a thread termination.
 *      The calling thread is blocked until the thread of the instance
 *      quits.
 *
 *      If the thread is terminated by uncaught exception, statusp is
 *      set PFC_THREAD_CANCELED.
 */
int
Thread::join(void **statusp)
{
    return timedJoin(statusp, NULL);
}

/*
 * int
 * Thread::timedJoin(void **statusp, const pfc_timespec_t *to)
 *      Wait for a thread termination.
 *      The calling thread is blocked until the thread of the instance
 *      quits within the specified timeout.
 *      If to is a null pointer, timeout is never occurred.
 */
int
Thread::timedJoin(void **statusp, const pfc_timespec_t *to)
{
    return pfc_thread_timedjoin(_thr_id, statusp, to);
}

/*
 * int
 * Thread::detach(void)
 *      Detach the thread.
 */
int
Thread::detach(void)
{
    return pfc_thread_detach(_thr_id);
}

/*
 * void
 * Thread::exit(void *status)
 *      Terminate the calling thread.
 *
 *      It's declared as static in the header.
 *
 * Remarks:
 *      Use of this function is strongly discouraged by the same
 *      reason as pfc_thread_exit().
 */
/* static */
void
Thread::exit(void *status)
{
    pfc_thread_exit(status);
    // At termination of pthread, clean up function is implicitly
    // called by pthread's cancellation handler facility.
    // See also Thread::thr_stub().
}

/*
 * Thread *
 * Thread::self(void)
 *      Return the instance of the calling thread.
 *      This function always creates a dynamic object.
 *      Caller should release it properly.
 *
 *      It's declared as static in the header.
 */
/* static */
Thread *
Thread::self(void)
{
    Thread *obj;
    try {
        obj = new Thread(pfc_thread_self());
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a thread instance (self)"
                         ": %s", ex.what());
        obj = NULL;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a thread instance (self)");
        obj = NULL;
    }

    return obj;
}

/*
 * pfc_thread_t
 * Thread::getId()
 *      Return the thread ID that's compatible with C library.
 */
pfc_thread_t
Thread::getId()
{
    return _thr_id;
}

/*
 * int
 * Thread::once(pfc_once_t *control,
 *              pfc_once_init_t init_routine)
 *      Call function specified to `init' only once.
 */
/* static */
int
Thread::once(pfc_once_t *control,
             pfc_once_init_t init_routine)
{
    return pfc_thread_once(control, init_routine);
}

/*
 * void
 * Thread::create_thread(const char *pool_name,
 *                       const boost::function<void *(void)> &func,
 *                       int flags)
 *      Create a new thread.
 *      It's declared as private and called only from constructors.
 */
void
Thread::create_thread(const char *pool_name,
                      const boost::function<void *(void)> &func,
                      int flags)
{
    /*
     * `func_p' is passed to `thr_stub()' as void pointer at starting
     * a new thread.  `thr_stub()' releases it at the end of the
     * thread.
     *
     * Otherwise, when Thread::exit() is called, `func_p' is released
     * by pthread's cancellation handler facility.
     */
    thr_func_t *func_p;

    try {
        func_p = new thr_func_t(func);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to copy a thread function object: %s",
                         ex.what());
        _thr_id = PFC_THREAD_INVALID;
        _create_status = ENOMEM;
        goto end;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to copy a thread function object");
        _thr_id = PFC_THREAD_INVALID;
        _create_status = EINVAL;
        goto end;
    }

    _create_status = pfc_thread_createat(&_thr_id,
                                         pool_name,
                                         Thread::thr_stub,
                                         static_cast<void *>(func_p),
                                         flags);
    if (_create_status != 0) {
        if (pool_name == NULL) {
            THREAD_LOG_ERROR("Failed to start a new thread at default pool"
                             " (errno = %d)", _create_status);
        } else {
            THREAD_LOG_ERROR("Failed to start a new thread at pool \"%s\""
                             " (errno = %d)",
                             pool_name, _create_status);
        }
        /* release immediately when thr_stub() is not called */
        delete func_p;
        _thr_id = PFC_THREAD_INVALID;
    }

  end:
    return;
}

/*
 * void *
 * Thread::thr_stub(void *arg)
 *      Stub function for create_thread() to call pfc_thread_create().
 *
 *      It's declared as private and static in the header.
 */
/* static */
void *
Thread::thr_stub(void *arg)
{
    /*
     * This function has a responsibility to release a function
     * object copied on heap.
     */

    thr_func_t *func_p = static_cast<thr_func_t *>(arg);
    void *ret(PFC_THREAD_CANCELED);

    /*
     * Register a cancellation handler for the case that
     * Thread::exit() is called.
     */
    pthread_cleanup_push(Thread::delete_func, static_cast<void *>(func_p));

    PFCXX_TRY_ON_RELEASE() {
        ret = (*func_p)();
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        THREAD_LOG_ERROR("Unexpected exception on thread start routine: %s",
                         ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        THREAD_LOG_ERROR("Unexpected exception on thread start routine.");
    }

    delete func_p;

    pthread_cleanup_pop(0);

    return ret;
}

/*
 * void
 * Thread::delete_func(void *arg)
 *      Release a function object copied on heap.
 *      It's provided to clean up when Thread::exit() is called.
 *
 *      It's declared as private and static in the header.
 */
/* static */
void
Thread::delete_func(void *arg)
{
    thr_func_t *func_p = static_cast<thr_func_t *>(arg);
    delete func_p;
}

/*
 * ThreadDataKey::~ThreadDataKey(void)
 *      Destructor of ThreadDataKey
 */
ThreadDataKey::~ThreadDataKey(void)
{
    if (_key != PFC_TSD_KEY_INVALID) {
        PFC_ASSERT_INT(pfc_thread_key_delete(_key), 0);
    }
}

/*
 * ThreadDataKey *
 * ThreadDataKey::create(pfc_tsd_dtor_t dtor)
 *      Create key with C style destructor.
 *
 *      It's declared as static in the header.
 */
/* static */
ThreadDataKey *
ThreadDataKey::create(pfc_tsd_dtor_t dtor)
{
    ThreadDataKey *tkey;

    pfc_tsd_key_t key;
    int err(pfc_thread_key_create(&key, dtor));
    if (PFC_EXPECT_FALSE(err != 0)) {
        THREAD_LOG_ERROR("Failed to create a key of TSD"
                         " with dtor func pointer %p (errno = %d)",
                         dtor, err);
        tkey = NULL;
        goto end;
    }

    try {
        tkey = new ThreadDataKey(key);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a TSD key instance"
                         " with key %u and dtor func pointer %p: %s",
                         key, dtor, ex.what());
        tkey = NULL;
        pfc_thread_key_delete(key);
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a TSD key instance"
                         " with key %u and dtor func pointer %p",
                         key, dtor);
        tkey = NULL;
        pfc_thread_key_delete(key);
    }

  end:
    return tkey;
}

/*
 * ThreadDataKey *
 * ThreadDataKey::create(const tsd_dtor_func_t &dtor)
 *      Create key with boost::function.
 *
 *      It's declared as static in the header.
 */
/* static */
ThreadDataKey *
ThreadDataKey::create(const tsd_dtor_func_t &dtor)
{
    ThreadDataKey *tkey;

    try {
        tkey = new ThreadDataKey(dtor);
    }
    catch (const std::exception &ex) {
        THREAD_LOG_ERROR("Failed to allocate a TSD key instance"
                         " with dtor func object: %s", ex.what());
        tkey = NULL;
        goto end;
    }
    PFCXX_CATCH_ALL() {
        THREAD_LOG_ERROR("Failed to allocate a TSD key instance"
                         " with dtor func object");
        tkey = NULL;
        goto end;
    }

    pfc_tsd_key_t key;

    // Declare at a different line from substitution.
    // It's a workaround for compile error.
    int err;
    err = __pfc_thread_key_create(&key, ThreadDataKey::stub,
                                  static_cast<pfc_ptr_t>(tkey));
    if (PFC_EXPECT_FALSE(err != 0)) {
        THREAD_LOG_ERROR("Failed to create a key of TSD"
                         " with dtor func object (errno = %d)",
                         err);
        delete tkey;
        tkey = NULL;
        goto end;
    }

    tkey->_key = key;

  end:
    return tkey;
}

/*
 * void
 * ThreadDataKey::call_dtor(pfc_ptr_t value)
 *      Call destructor for TSD value
 *
 *      It's declared as private in the header.
 */
void
ThreadDataKey::call_dtor(pfc_ptr_t value)
{
    PFCXX_TRY_ON_RELEASE() {
        _dtor(value);
    }
    PFCXX_CATCH_ON_RELEASE(ex) {
        THREAD_LOG_ERROR("Unexpected exception on TSD destructor: %s",
                         ex.what());
    }
    PFCXX_CATCH_ALL_ON_RELEASE() {
        THREAD_LOG_ERROR("Unexpected exception on TSD destructor.");
        throw;
    }
}

/*
 * void
 * ThreadDataKey::stub(pfc_ptr_t value, pfc_ptr_t arg)
 *      Stub function for TSD destructor.
 *      It will call boost::function object after casting from arg.
 *
 *      It's declared as private and static in the header.
 */
/* static */
void
ThreadDataKey::stub(pfc_ptr_t value, pfc_ptr_t arg)
{
    ThreadDataKey *tkey = static_cast<ThreadDataKey *>(arg);
    tkey->call_dtor(value);
}

}       // core
}       // pfc
