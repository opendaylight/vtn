/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for flock test utilities.
 */

#ifndef	_TEST_FLOCK_SUBR_HH
#define	_TEST_FLOCK_SUBR_HH

#include <cstdlib>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <sys/types.h>
#include "misc.hh"
#include "sub_flock/sub_flock.h"

/*
 * Class which represents a sub_flock command running on a child process.
 */
class FlockCmd
{
public:
    /*
     * Flags for constructor.
     */

    /* Acquire writer lock. */
    static const uint32_t	C_WRITER	= 0x1U;

    /* Raise SIGUSR1 while locking. */
    static const uint32_t	C_SIGNAL	= 0x2U;

    /* Pause after unlocking. */
    static const uint32_t	C_PAUSE		= 0x4U;

    /* Send many SIGUSR1 signals after unlocking. */
    static const uint32_t	C_PAUSE_SIGNAL	= 0x8U;

    /* Ensure that the file contents is never changed in the locked section. */
    static const uint32_t	C_CHECK		= 0x10U;

    explicit FlockCmd(uint32_t flags);
    ~FlockCmd();

    void	setup(int holdtime) throw(std::runtime_error);
    void	invoke(const char *path) throw(std::runtime_error);
    void	reap(void) throw(std::runtime_error);
    void	terminate(void) throw(std::runtime_error);

    inline uint32_t
    getFlags(void) const
    {
        return _flags;
    }

    template <class T>
    void
    push(T arg) throw(std::runtime_error)
    {
        EXASSERT_TRUE(_argv.push(arg));
    }

    inline pid_t
    getPID(void) const
    {
        return _pid;
    }

private:
    pid_t	_pid;
    uint32_t	_flags;
    ArgVector	_argv;
};

#endif	/* !_TEST_FLOCK_SUBR_HH */
