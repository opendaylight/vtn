/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFCXX_SYNCH_HH
#define	_PFCXX_SYNCH_HH

/*
 * Definitions for thread synchronization utilities in the C++ world.
 */

#include <boost/noncopyable.hpp>
#include <pfc/synch.h>
#include <pfc/debug.h>

namespace pfc {
namespace core {

/*
 * Mutex
 */
class Mutex
    : boost::noncopyable
{
public:
    /*
     * Create a new mutex object.
     */
    Mutex(void)
    {
        PFC_ASSERT_INT(PFC_MUTEX_INIT(&_mutex), 0);
    }

    /*
     * Destructor of a mutex object.
     */
    ~Mutex(void)
    {
        PFC_ASSERT_INT(pfc_mutex_destroy(&_mutex), 0);
    }

    /*
     * Acquire the mutex lock.
     */
    inline void
    lock(void)
    {
        PFC_ASSERT_INT(pfc_mutex_lock(&_mutex), 0);
    }

    /*
     * Try to acquire the mutex lock.
     *
     * Zero is returned on success.
     * EBUSY is returned, if the mutex was already locked.
     */
    inline int
    trylock(void)
    {
        int  ret(pfc_mutex_trylock(&_mutex));

        PFC_ASSERT_PRINTF(ret == 0 || ret == EBUSY, "ret = %d", ret);

        return ret;
    }

    /*
     * Try to acquire mutex within the specified timeout period.
     * Unlike POSIX thread interface, timeout is specified by time period,
     * not absolute time.
     *
     * Zero is returned on success.
     * ETIMEDOUT is returned, if the specified timeout expired.
     */
    inline int
    timedlock(const pfc_timespec_t &timeout)
    {
        int  ret(pfc_mutex_timedlock(&_mutex, &timeout));

        PFC_ASSERT_PRINTF(ret == 0 || ret == ETIMEDOUT, "ret = %d", ret);

        return ret;
    }

    /*
     * Release the mutex lock
     */
    inline void
    unlock(void)
    {
        PFC_ASSERT_INT(pfc_mutex_unlock(&_mutex), 0);
    }

protected:
    /*
     * Create a new mutex object.
     * If 'recursive' is true, this object is initialized as RECURSIVE type.
     */
    explicit Mutex(bool recursive)
    {
        pfc_mutextype_t  mtype((recursive)
                               ? PFC_MUTEX_TYPE_RECURSIVE
                               : PFC_MUTEX_TYPE_NORMAL);

        PFC_ASSERT_INT(pfc_mutex_init(&_mutex, mtype), 0);
    }

private:
    friend class Condition;

    /* PFC mutex object. */
    pfc_mutex_t	_mutex;
};

/*
 * Recursive mutex
 */
class RecursiveMutex
    : public Mutex
{
public:
    /*
     * Create a new recursive mutex object.
     */
    RecursiveMutex(void) : Mutex(true) {}
};

/*
 * Scoped mutex
 */
class ScopedMutex
    : boost::noncopyable
{
public:
    /*
     * Create a new scoped mutex object.
     * This constructor acquires the given mutex lock.
     */
    explicit ScopedMutex(Mutex &m)
    {
        _mutexp = &m;
        _mutexp->lock();
    }

    /*
     * Destructor of a scoped mutex object.
     * This destructor releases the mutex lock.
     */
    ~ScopedMutex()
    {
        unlock();
    }

    /*
     * Release the mutex lock
     */
    inline void
    unlock(void)
    {
        Mutex  *m(_mutexp);

        if (m != NULL) {
            _mutexp = NULL;
            m->unlock();
        }
    }

private:
    /* Mutex object. */
    Mutex	*_mutexp;
};

/*
 * Read-write lock
 */
class ReadWriteLock
    : boost::noncopyable
{
public:
    /*
     * Create a new read-write lock object.
     */
    ReadWriteLock(void)
    {
        PFC_ASSERT_INT(pfc_rwlock_init(&_rwlock), 0);
    }

    /*
     * Destructor of a read-write lock object.
     */
    ~ReadWriteLock(void)
    {
        PFC_ASSERT_INT(pfc_rwlock_destroy(&_rwlock), 0);
    }

    /*
     * Acquire the read lock.
     */
    inline void
    rdlock(void)
    {
        PFC_ASSERT_INT(pfc_rwlock_rdlock(&_rwlock), 0);
    }

    /*
     * Try to acquire the read lock.
     *
     * Zero is returned on success.
     * EBUSY is returned, if a writer holds the lock or a writer
     * was blocked on this lock.
     */
    inline int
    tryrdlock(void)
    {
        int  ret(pfc_rwlock_tryrdlock(&_rwlock));

        PFC_ASSERT_PRINTF(ret == 0 || ret == EBUSY, "ret = %d", ret);

        return ret;
    }

    /*
     * Try to acquire read lock within the specified timeout period.
     * Unlike POSIX thread interface, timeout is specified by time period,
     * not absolute time.
     *
     * Zero is returned on success.
     * ETIMEDOUT is returned, if the specified timeout expired.
     */
    inline int
    timedrdlock(const pfc_timespec_t &timeout)
    {
        int  ret(pfc_rwlock_timedrdlock(&_rwlock, &timeout));

        PFC_ASSERT_PRINTF(ret == 0 || ret == ETIMEDOUT, "ret = %d", ret);

        return ret;
    }

    /*
     * Acquire the write lock.
     */
    inline void
    wrlock(void)
    {
        PFC_ASSERT_INT(pfc_rwlock_wrlock(&_rwlock), 0);
    }

    /*
     * Try to acquire the write lock.
     *
     * Zero is returned on success.
     * EBUSY is returned, if the read-write lock was already locked.
     */
    inline int
    trywrlock(void)
    {
        int  ret(pfc_rwlock_trywrlock(&_rwlock));

        PFC_ASSERT_PRINTF(ret == 0 || ret == EBUSY, "ret = %d", ret);

        return ret;
    }

    /*
     * Try to acquire write lock within the specified timeout period.
     * Unlike POSIX thread interface, timeout is specified by time period,
     * not absolute time.
     *
     * Zero is returned on success.
     * ETIMEDOUT is returned, if the specified timeout expired.
     */
    inline int
    timedwrlock(const pfc_timespec_t &timeout)
    {
        int  ret(pfc_rwlock_timedwrlock(&_rwlock, &timeout));

        PFC_ASSERT_PRINTF(ret == 0 || ret == ETIMEDOUT, "ret = %d", ret);

        return ret;
    }

    /*
     * Release the read-write lock
     */
    inline void
    unlock(void)
    {
        PFC_ASSERT_INT(pfc_rwlock_unlock(&_rwlock), 0);
    }

private:
    /* PFC rwlock object. */
    pfc_rwlock_t	_rwlock;
};

/*
 * Condition variable
 */
class Condition
    : boost::noncopyable
{
public:
    /*
     * Create a new condition variable.
     */
    Condition(void)
    {
        PFC_ASSERT_INT(pfc_cond_init(&_cond), 0);
    }

    /*
     * Destructor of a condition variable.
     */
    ~Condition(void)
    {
        PFC_ASSERT_INT(pfc_cond_destroy(&_cond), 0);
    }

    /*
     * Signal a condition.
     */
    inline void
    signal(void)
    {
        PFC_ASSERT_INT(pfc_cond_signal(&_cond), 0);
    }

    /*
     * Broadcast a condition.
     */
    inline void
    broadcast(void)
    {
        PFC_ASSERT_INT(pfc_cond_broadcast(&_cond), 0);
    }

    /*
     * Block on the condition variable.
     */
    inline void
    wait(Mutex &m)
    {
        PFC_ASSERT_INT(pfc_cond_wait(&_cond, &m._mutex), 0);
    }

    /*
     * Block on the condition variable, within the specified timeout period.
     * Unlike POSIX thread interface, timeout is specified by time period,
     * not absolute time.
     *
     * Zero is returned on success.
     * ETIMEDOUT is returned, if the specified timeout expired.
     */
    inline int
    timedwait(Mutex &m, const pfc_timespec_t &timeout)
    {
        int  ret(pfc_cond_timedwait(&_cond, &m._mutex, &timeout));

        PFC_ASSERT_PRINTF(ret == 0 || ret == ETIMEDOUT, "ret = %d", ret);

        return ret;
    }

private:
    /* PFC cond object. */
    pfc_cond_t	_cond;
};

/*
 * Assertions for PFC semaphore APIs.
 */
#ifdef	PFC_VERBOSE_DEBUG
#define	__PFCXX_SEM_ASSERT(ex)                                            \
	do {								\
		int	__r(ex);					\
									\
		PFC_ASSERT_PRINTF(__r == 0, "errno = %d", errno);	\
	} while (0)
#else	/* !PFC_VERBOSE_DEBUG */
#define	__PFCXX_SEM_ASSERT(ex)		((void)(ex))
#endif	/* PFC_VERBOSE_DEBUG */

#define	__PFCXX_SEM_ERRNO_ASSERT(ex, ret, accept_errno)			\
	do {								\
		int	__r(ex);					\
									\
		if (PFC_EXPECT_TRUE(__r == 0)) {			\
			(ret) = 0;					\
		}							\
		else {							\
			(ret) = errno;					\
			PFC_ASSERT_PRINTF((ret) == (accept_errno),	\
                                          "errno = %d", (ret));         \
                }                                                       \
	} while (0)

/*
 * Semaphore
 */
class Semaphore
    : boost::noncopyable
{
public:
    /*
     * Create a new semaphore object.
     */
    explicit Semaphore(unsigned int value)
    {
        __PFCXX_SEM_ASSERT(pfc_sem_init(&_sem, value));
    }

    /*
     * Destructor of a semaphore object.
     */
    ~Semaphore(void)
    {
        __PFCXX_SEM_ASSERT(pfc_sem_destroy(&_sem));
    }

    /*
     * Increment (unlock) the semaphore.
     */
    inline void
    post(void)
    {
        __PFCXX_SEM_ASSERT(pfc_sem_post(&_sem));
    }

    /*
     * Decrement (lock) the semaphore.
     */
    inline void
    wait(void)
    {
        __PFCXX_SEM_ASSERT(pfc_sem_wait(&_sem));
    }

    /*
     * Try to decrement (lock) the semaphore.
     *
     * Zero is returned on success.
     * EAGAIN is returned, if the semaphore currently has the value zero.
     */
    inline int
    trywait(void)
    {
        int  ret;

        __PFCXX_SEM_ERRNO_ASSERT(pfc_sem_trywait(&_sem), ret, EAGAIN);

        return ret;
    }

    /*
     * Try to decrement the semaphore, within the specified timeout period.
     * Unlike POSIX thread interface, timeout is specified by time period,
     * not absolute time.
     *
     * Zero is returned on success.
     * ETIMEDOUT is returned, if the specified timeout expired.
     */
    inline int
    timedwait(const pfc_timespec_t &timeout)
    {
        int  ret;

        __PFCXX_SEM_ERRNO_ASSERT(pfc_sem_timedwait(&_sem, &timeout), ret,
                                 ETIMEDOUT);

        return ret;
    }

    /*
     * Get the value of a semaphore.
     *
     * The current value of the semaphore is returned.
     */
    inline int
    getvalue(void)
    {
        int  sval(0);

        __PFCXX_SEM_ASSERT(pfc_sem_getvalue(&_sem, &sval));

        return sval;
    }

private:
    /* PFC sem object. */
    pfc_sem_t	_sem;
};

}	// core
}	// pfc

#endif	/* !_PFCXX_SYNCH_HH */
