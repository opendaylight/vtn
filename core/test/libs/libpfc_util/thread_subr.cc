/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * thread_subr.cc - Thread utilities for testing.
 */

#include <sstream>
#include <cstring>
#include "thread_subr.hh"

/*
 * TempThread::TempThread()
 *	Constructor of temporary thread instance.
 */
TempThread::TempThread()
    : _state(THR_INIT)
{
    PFC_MUTEX_INIT(&_mutex);
    pfc_cond_init(&_cond);
}

/*
 * TempThread::~TempThread()
 *	Destructor of temporary thread.
 */
TempThread::~TempThread()
{
    lock();

    tstate_t	st(_state);
    if (st == THR_RUNNING) {
        _state = THR_STOP;
        condSignal();
    }

    unlock();

    if (st == THR_RUNNING) {
        pthread_t	t(_thread);

        if (t != PFC_PTHREAD_INVALID_ID) {
            (void)pthread_join(_thread, NULL);
        }
    }

    pfc_cond_destroy(&_cond);
    pfc_mutex_destroy(&_mutex);
}

/*
 * int
 * TempThread::start(void)
 *	Create and start a temporary thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TempThread::start(void)
{
    lock();

    if (getState() != THR_INIT) {
        unlock();

        return EBUSY;
    }

    void *arg(reinterpret_cast<void *>(this));
    int	err(pthread_create(&_thread, NULL, TempThread::entry, arg));
    if (err == 0) {
        _state = THR_RUNNING;
    }

    unlock();

    return err;
}

/*
 * int
 * TempThread::join(void)
 *	Wait for termination of a temporary thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TempThread::join(void)
{
    lock();

    if (getState() != THR_RUNNING) {
        unlock();

        return ESRCH;
    }

    _state = THR_STOP;
    condSignal();
    unlock();

    int	err(pthread_join(_thread, NULL));
    _thread = PFC_PTHREAD_INVALID_ID;

    return err;
}

/*
 * void
 * TempThread::lock(void)
 *	Acquire lock for the thread's mutex.
 */
void
TempThread::lock(void)
{
    pfc_mutex_lock(&_mutex);
}

/*
 * void
 * TempThread::unlock(void)
 *	Release lock for the thread's mutex.
 */
void
TempThread::unlock(void)
{
    pfc_mutex_unlock(&_mutex);
}

/*
 * void
 * TempThread::condSignal(void)
 *	Wake up one thread which is waiting on the condition variable.
 */
void
TempThread::condSignal(void)
{
    pfc_cond_signal(&_cond);
}

/*
 * int
 * TempThread::condTimedWait(const pfc_timespec_t *timeout)
 *	Wait on the condition variable until the specified time period is
 *	passed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The caller must acquire thread's lock on calling.
 */
int
TempThread::condTimedWait(const pfc_timespec_t *timeout)
{
    return pfc_cond_timedwait(&_cond, &_mutex, timeout);
}

/*
 * void
 * TempThread::testStop(void)
 *	Test whether the thread must be stopped or not.
 *	If the thread state is THR_STOP, the calling thread with exit with
 *	specifying TEMPTHR_STATE_STOPPED.
 *	Note that the calling thread will exit if the thread state is THR_STOP.
 *
 * Remarks:
 *	The caller must acquire thread's lock on calling.
 */
void
TempThread::testStop(void)
{
    if (_state == THR_STOP) {
        unlock();
        pthread_exit(TEMPTHR_STATE_STOPPED);
    }
}

/*
 * void
 * TempThread::setError(const char *msg, int err)
 *	Set error message.
 */
void
TempThread::setError(const char *msg, int err)
{
    std::ostringstream stream;

    stream << msg;
    if (err != 0) {
        stream << ": " << strerror(err);
    }

    _message = stream.str();
}

/*
 * void *
 * TempThread::entry(void *arg)
 *	Entry function of a temporary thread.
 */
void *
TempThread::entry(void *arg)
{
    TempThread	*thr(reinterpret_cast<TempThread *>(arg));

    // Disable cancellation.
    (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    thr->run();

    return NULL;
}
