/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for signal utilities.
 */

#ifndef	_TEST_SIGNAL_SUBR_HH
#define	_TEST_SIGNAL_SUBR_HH

#include <cstdlib>
#include <string>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>

/*
 * Temporary signal handler.
 * This class will change signal handler and mask, and they will be restored
 * by destructor.
 *
 * Remaks:
 *	This class is not thread safe. Only one instance can be created at the
 *	time in the process.
 */
class SignalHandler
{
public:
    SignalHandler(int sig, const sigset_t *mask = NULL);
    ~SignalHandler();

    bool	install(void);
    bool	ignore(void);
    void	bind(pthread_t thread);

    inline sig_atomic_t
    getReceived(void) const
    {
        return _received;
    }

    inline const std::string&
    getError(void) const
    {
        return _message;
    }

private:
    static void	default_handler(int sig);
    bool	install(void (*handler)(int));
    void	setError(const char *msg, int err = 0);

    /* Target signal. */
    int		_target;

    /* Signal received counter. */
    volatile sig_atomic_t	_received;

    /* Signal mask during test. */
    sigset_t	_mask;

    /* Saved signal mask. */
    sigset_t	_savedMask;

    /* Saved signal handler. */
    struct sigaction	_savedAction;

    /* Error message. */
    std::string	_message;

    /* Target thread. */
    pthread_t	_thread;

    /* Internal flags */
    uint32_t	_flags;
};

#endif	/* !_TEST_SIGNAL_SUBR_HH */
