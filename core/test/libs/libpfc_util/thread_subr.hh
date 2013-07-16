/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for thread utilities.
 */

#ifndef	_TEST_THREAD_SUBR_HH
#define	_TEST_THREAD_SUBR_HH

#include <cstdlib>
#include <string>
#include <stdint.h>
#include <pthread.h>
#include <pfc/synch.h>
#include <pfc/clock.h>

/*
 * Thread's exit state that means the thread is stopped by force.
 */
#define	TEMPTHR_STATE_STOPPED		(reinterpret_cast<void *>(-1))

/*
 * Class for temporary thread.
 * An temporary thread is created on start(), and removed on destructor.
 * You may want to inherit this class and override run(void) method to run
 * your code on a temporary thread.
 */
class TempThread
{
    typedef enum {
        THR_INIT,		/* Initial state. */
        THR_RUNNING,		/* Thread is running. */
        THR_STOP,		/* Thread is stopped. */
    } tstate_t;

public:
    TempThread();
    virtual ~TempThread();

    /* Default body of thread function. */
    virtual void
    run(void)
    {
        // Do nothing.
    }

    int		start(void);
    int		join(void);
    void	lock(void);
    void	unlock(void);
    void	condSignal(void);
    int		condTimedWait(const pfc_timespec_t *timeout);

    inline tstate_t
    getState(void) const
    {
        return _state;
    }

    inline bool
    hasError() const
    {
        return (_message.size() != 0);
    }

    inline const std::string &
    getError() const
    {
        return _message;
    }

protected:
    void	testStop(void);
    void	setError(const char *msg, int err = 0);

private:
    static void	*entry(void *arg);

    /* Thread state. */
    volatile tstate_t	_state;

    /* Thread ID */
    pthread_t		_thread;

    /* Mutex lock. */
    pfc_mutex_t		_mutex;

    /* Condition variable. */
    pfc_cond_t		_cond;

    /* Error message. */
    std::string		_message;
};

#endif	/* !_TEST_THREAD_SUBR_HH */
