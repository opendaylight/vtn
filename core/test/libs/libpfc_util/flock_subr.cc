/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * flock_subr.cc - flock test utilities.
 */

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "flock_subr.hh"

/*
 * FlockCmd(uint32_t flags)
 *	Constructor of sub_flock command context.
 */
FlockCmd::FlockCmd(uint32_t flags)
    : _pid(-1), _flags(flags)
{
}

/*
 * FlockCmd::~FlockCmd()
 *	Destructor of sub_flock command context.
 */
FlockCmd::~FlockCmd()
{
    if (_pid != -1) {
        if (kill(_pid, SIGKILL) != -1) {
            (void)waitpid(_pid, NULL, 0);
        }
    }
}

/*
 * void
 * FlockCmd::setup(void) throw(std::runtime_error)
 *	Set up sub_flock command context.
 */
void
FlockCmd::setup(int holdtime) throw(std::runtime_error)
{
    // Set command name.
    push(SUB_FLOCK_NAME);

    // Hold file lock for the specified milliseconds.
    push("-T");
    push(holdtime);

    // Raise SIGUSR1 just after locking.
    push("-n");

    if (_flags & C_WRITER) {
        // Acquire writer lock on sub_flock command.
        push("-w");
    }

    if (_flags & C_SIGNAL) {
        // Raise SIGUSR1 to parent while holding the lock.
        push("-s");
    }

    if (_flags & C_PAUSE) {
        // Pause just after unlocking.
        push("-p");
    }

    if (_flags & C_PAUSE_SIGNAL) {
        // Send many SIGUSR1 signals after unlocking.
        push("-P");
    }

    if (_flags & C_CHECK) {
        // Ensure that the file contents is never changed in the locked
        // section.
        push("-c");
    }
}

/*
 * void
 * FlockCmd::invoke(const char *path) throw(std::runtime_error)
 *	Create a child procss, and invoke sub_flock command.
 */
void
FlockCmd::invoke(const char *path) throw(std::runtime_error)
{
    // Add lock file path.
    push(path);

    // Create a child process.
    _pid = fork();
    EXASSERT_NE(-1, _pid);

    if (_pid == 0) {
        // Execute sub_flock command.
        execv(SUB_FLOCK_PATH, *_argv);

        fprintf(stderr, "execv() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/*
 * void
 * FlockCmd::reap(void) throw(std::runtime_error)
 *	Reap child process.
 */
void
FlockCmd::reap(void) throw(std::runtime_error)
{
    EXASSERT_NE(-1, _pid);

    int		status;
    for (;;) {
        pid_t	cpid(waitpid(_pid, &status, 0));
        if (cpid != -1) {
            EXASSERT_EQ(_pid, cpid);
            _pid = -1;
            break;
        }
        EXASSERT_EQ(EINTR, errno);
    }

    EXASSERT_TRUE(WIFEXITED(status));
    EXASSERT_EQ(0, WEXITSTATUS(status));
}

/*
 * void
 * FlockCmd::terminate(void) throw(std::runtime_error)
 *	Teaminate the process by sending SIGTERM.
 */
void
FlockCmd::terminate(void) throw(std::runtime_error)
{
    EXASSERT_NE(-1, _pid);
    EXASSERT_EQ(0, kill(_pid, SIGTERM));

    // sub_flock quits with status zero in SIGTERM handler.
    reap();
}
