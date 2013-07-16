/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * signal_subr.cc - Signal utilities for testing.
 */

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <pfc/atomic.h>
#include "signal_subr.hh"

/*
 * Internal flags.
 */
#define	SIGH_SETMASK		0x1U	/* _mask keeps signal mask. */
#define	SIGH_BOUND		0x2U	/* signal is bound to the thread. */

/*
 * Installed signal handler instance.
 */
static SignalHandler	*sig_handler = NULL;
static void		(*sig_handler_func)(int) = NULL;

/*
 * SignalHandler::SignalHandler(int sig, const sigset_t *mask)
 *	Create temporary signal handler for the specified signal.
 *	If `mask' is not NULL, the specified mask is installed as signal mask.
 *	If NULL, only the target signal is removed from the current signal
 *	mask.
 */
SignalHandler::SignalHandler(int sig, const sigset_t *mask)
    : _target(sig), _received(0)
{
    if (mask == NULL) {
        // Unmask only the target signal.
        _flags = 0;
    }
    else {
        // Use the specified signal mask.
        _mask = *mask;
        _flags = SIGH_SETMASK;
    }
}

/*
 * SignalHandler::~SignalHandler()
 *	Destructor of temporary signal handler.
 */
SignalHandler::~SignalHandler()
{
    pfc_ptr_t	*addr(reinterpret_cast<pfc_ptr_t *>(&sig_handler));
    pfc_ptr_t	me(pfc_atomic_swap_ptr(addr, NULL));

    if (reinterpret_cast<SignalHandler *>(me) == this) {
        // Restore signal handler.
        (void)sigaction(_target, &_savedAction, NULL);

        // Restore signal mask.
        (void)pthread_sigmask(SIG_SETMASK, &_savedMask, NULL);
    }
}

/*
 * bool
 * SignalHandler::install(void)
 *	Install signal configuration.
 *
 * Calling/Exit State:
 *	Upon successful completion, true is returned.
 *	Otherwise false.
 *
 * Remarks:
 *	This method does nothing if signal configuration has been successfully
 *	installed.
 */
bool
SignalHandler::install(void)
{
    return install(SignalHandler::default_handler);
}

/*
 * bool
 * SignalHandler::ignore(void)
 *	Ignore signal.
 *
 * Calling/Exit State:
 *	Upon successful completion, true is returned.
 *	Otherwise false.
 *
 * Remarks:
 *	This method does nothing if signal configuration has been successfully
 *	installed.
 */
bool
SignalHandler::ignore(void)
{
    return install(SIG_IGN);
}

/*
 * void
 * SignalHandler::bind(pthread_t thread)
 *	Bind signal to the specified thread.
 */
void
SignalHandler::bind(pthread_t thread)
{
    _thread = thread;
    _flags |= SIGH_BOUND;
}

/*
 * void
 * SignalHandler::default_handler(int sig)
 *	Default signal handler.
 */
void
SignalHandler::default_handler(int sig)
{
    SignalHandler	*hdr = sig_handler;

    if (hdr != NULL) {
        if ((hdr->_flags & SIGH_BOUND) == 0 ||
            pthread_equal(hdr->_thread, pthread_self())) {
            hdr->_received++;
        }
        else {
            // Redirect this signal to the target thread.
            pthread_kill(hdr->_thread, sig);
        }
    }
}

/*
 * bool
 * SignalHandler::install(void (*handler)(int))
 *	Install signal handler.
 *
 * Calling/Exit State:
 *	Upon successful completion, true is returned.
 *	Otherwise false.
 *
 * Remarks:
 *	This method does nothing if signal configuration has been successfully
 *	installed.
 */
bool
SignalHandler::install(void (*handler)(int))
{
    // Install handler instance.
    pfc_ptr_t	*addr(reinterpret_cast<pfc_ptr_t *>(&sig_handler));
    pfc_ptr_t	me(reinterpret_cast<pfc_ptr_t>(this));
    pfc_ptr_t	ptr(pfc_atomic_cas_ptr(addr, me, NULL));

    if (ptr == me && sig_handler_func == handler) {
        // Already installed.
        return true;
    }
    if (ptr != NULL) {
        setError("Another signal handler is installed.");

        return false;
    }

    // Install signal handler.
    struct sigaction	act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);

    if (sigaction(_target, &act, &_savedAction) == -1) {
        setError("sigaction() failed", errno);
        (void)pfc_atomic_swap_ptr(addr, NULL);

        return false;
    }

    // Install signal mask.
    int	err;
    if (_flags & SIGH_SETMASK) {
        err = pthread_sigmask(SIG_SETMASK, &_mask, &_savedMask);
    }
    else {
        sigemptyset(&_mask);
        sigaddset(&_mask, _target);
        err = pthread_sigmask(SIG_UNBLOCK, &_mask, &_savedMask);
    }

    if (err != 0) {
        setError("pthread_sigmask() failed", err);
        (void)sigaction(_target, &_savedAction, NULL);
        (void)pfc_atomic_swap_ptr(addr, NULL);

        return false;
    }

    sig_handler_func = handler;

    return true;
}

/*
 * void
 * SignalHandler::setError(const char *msg, int err)
 *	Set error message.
 */
void
SignalHandler::setError(const char *msg, int err)
{
    std::ostringstream stream;

    stream << msg;
    if (err != 0) {
        stream << ": " << strerror(err);
    }

    _message = stream.str();
}
