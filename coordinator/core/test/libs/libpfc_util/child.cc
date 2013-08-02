/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * child.cc - Utility to run test on a child process.
 */

#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <gtest/gtest.h>
#include <pfc/base.h>
#include "test.h"
#include "child.hh"

#ifdef  PFC_OSTYPE_LINUX
#include <dirent.h>
#endif  /* PFC_OSTYPE_LINUX */

/*
 * ChildContext::ChildContext()
 *      Initialize test context on a child process.
 */
ChildContext::ChildContext()
    : _pid(-1), _running(-1), _uid(-1), _gid(-1), _killed(0), _status(0),
      _errlist(NULL), _errfp(NULL)
{
}

/*
 * ChildContext::ChildContext(uid_t uid, gid_t gid)
 *      Initialize test context on a child process with specifying user and
 *      group ID for child process.
 */
ChildContext::ChildContext(uid_t uid, gid_t gid)
    : _pid(-1), _running(-1), _uid(uid), _gid(gid), _killed(0), _status(0),
      _errlist(NULL), _errfp(NULL)
{
}

/*
 * ChildContext::~ChildContext()
 *      Destroy test context on a child process.
 */
ChildContext::~ChildContext()
{
    pid_t  pid(_running);

    if (pid != -1 && pid != 0) {
        (void)kill(pid, SIGKILL);
        (void)waitpid(pid, NULL, 0);
    }

    if (_errfp != NULL) {
        fclose(_errfp);
    }
}

/*
 * void
 * ChildContext::run(void (*func)(ChildContext *child))
 *      Run the specified function on a child process.
 */
void
ChildContext::run(void (*func)(ChildContext *child))
{
    setUp();
    RETURN_ON_ERROR();

    if (_running == 0) {
        if (!::testing::Test::HasFailure()) {
            // Call the specified function.
            (*func)(this);
        }

        tearDown();
        // NOTREACHED
    }
}

/*
 * void
 * ChildContext::run(child_func_t &func)
 *      Run the function specified by boost::function on a child process.
 */
void
ChildContext::run(child_func_t &func)
{
    setUp();
    RETURN_ON_ERROR();

    if (_running == 0) {
        if (!::testing::Test::HasFailure()) {
            // Call the specified function.
            (func)(this);
        }

        tearDown();
        // NOTREACHED
    }
}

/*
 * bool
 * ChildContext::checkUnclosedFd(uint32_t mode)
 *      Determine whether unclosed file descriptor exists or not.
 *
 *      If CHECKFD_MODE_COLLECT is set to `mode', this method registeres
 *      opened file descriptors to _fdmap.
 *
 *      If not set, this method prints opened file descriptors which does
 *      not exist in _fdmap to the standard error output, unless
 *      CHECKFD_MODE_NOPRINT is set to `mode'.
 */
bool
ChildContext::checkUnclosedFd(uint32_t mode)
{
    bool  ret(true);

#ifdef  PFC_OSTYPE_LINUX
    DIR	  *dirp;
    int	  dfd;
    struct dirent   buf, *dp;

    dirp = opendir("/proc/self/fd");
    if (dirp == NULL) {
        return ret;
    }

    dfd = dirfd(dirp);
    while (readdir_r(dirp, &buf, &dp) == 0 && dp != NULL) {
        ssize_t	sz;
        char	first = dp->d_name[0];
        char	name[256], path[256], *p;
        long	fd;

        if (first < '0' || first > '9') {
            continue;
        }

        fd = strtol(dp->d_name, &p, 10);
        if (*p != '\0' || fd == dfd) {
            continue;
        }

        snprintf(path, sizeof(path), "/proc/self/fd/%s", dp->d_name);
        if ((sz = readlink(path, name, sizeof(name))) != -1) {
            if (sz == 0) {
                snprintf(name, sizeof(name), "<unknown>");
            }
            else {
                if ((size_t)sz >= sizeof(name)) {
                    sz = sizeof(name) - 1;
                }
                name[sz] = '\0';
            }
        }
        else {
            snprintf(name, sizeof(name), "<unknown>");
        }

        if ((mode & CHECKFD_MODE_COLLECT) == 0) {
            intstrmap_t::iterator it(_fdmap.find(fd));

            if (it == _fdmap.end()) {
                if ((mode & CHECKFD_MODE_NOPRINT) == 0) {
                    fprintf(stderr, "*** Unclosed file: pid=%u: %3s: %s\n",
                            getpid(), dp->d_name, name);
                }
                ret = false;
            }
        }
        else {
            _fdmap[fd] = name;
        }
    }

    closedir(dirp);
#endif  /* PFC_OSTYPE_LINUX */

    return ret;
}

/*
 * void
 * ChildContext::wait(int &status)
 *      Wait for completion of a child process.
 */
void
ChildContext::wait(int &status)
{
    pid_t  pid(_running);

    if (::testing::Test::HasFailure() || pid == 0 || pid == -1) {
        status = -1;
        return;
    }

    int    st;

    for (;;) {
        int  ret(waitpid(pid, &st, 0));

        if (ret == pid) {
            break;
        }

        if (ret == -1) {
            ASSERT_EQ(EINTR, errno);
            continue;
        }
    }

    _running = -1;

    ASSERT_TRUE(WIFEXITED(st)) << "status = " << st;
    status = WEXITSTATUS(st);
}

/*
 * void
 * ChildContext::verify(void)
 *      Verify results of the test.
 */
void
ChildContext::verify(void)
{
    pid_t  pid(_running);

    if (::testing::Test::HasFailure() || pid == 0 || pid == -1) {
        return;
    }

    if (_errlist != NULL) {
        // Read stderr outputs.
        FILE  *fp(_errfp);
        char  line[1024];

        while (fgets(line, sizeof(line), fp) != NULL) {
            _errlist->push_back(line);
        }
    }

    int    status;

    for (;;) {
        int  ret(waitpid(pid, &status, 0));

        if (ret == pid) {
            break;
        }

        if (ret == -1) {
            ASSERT_EQ(EINTR, errno);
            continue;
        }
    }

    _running = -1;

    if (_killed == 0) {
        ASSERT_TRUE(WIFEXITED(status)) << "status = " << status;
        int  st(WEXITSTATUS(status));
        ASSERT_EQ(0, st);
    }
    else {
        ASSERT_TRUE(WIFSIGNALED(status)) << "status = " << status;
        int  sig(WTERMSIG(status));
        ASSERT_EQ(SIGKILL, sig);
    }
}

/*
 * void
 * ChildContext::setUp(void)
 *      Set up test environment to spawn a child process.
 */
void
ChildContext::setUp(void)
{
    int errfds[2];

    if (_errlist != NULL) {
        // Create pipe to read stderr outputs.
        ASSERT_EQ(0, pipe(errfds)) << "*** ERROR: " << strerror(errno);
    }
    else {
        errfds[0] = errfds[1] = -1;
    }

    // Spawn a child process.
    _pid = _running = fork();
    ASSERT_NE(static_cast<pid_t>(-1), _pid);
    if (_pid == 0) {
        if (_errlist != NULL) {
            // Bind stderr pipe to the standard error output.
            ASSERT_EQ(0, close(errfds[0])) << "*** ERROR: " << strerror(errno);
            ASSERT_NE(-1, dup2(errfds[1], STDERR_FILENO))
                << "*** ERROR: " << strerror(errno);
        }

        if (_gid != static_cast<gid_t>(-1)) {
            // Change group.
            ASSERT_EQ(0, setgid(_gid)) << "*** ERROR: " << strerror(errno);
        }
        if (_uid != static_cast<uid_t>(-1)) {
            // Change user.
            ASSERT_EQ(0, setuid(_uid)) << "*** ERROR: " << strerror(errno);
        }

        // Preserve file descriptors.
        checkUnclosedFd(CHECKFD_MODE_COLLECT);
    }
    else if (errfds[1] != -1) {
        ASSERT_EQ(0, close(errfds[1])) << "*** ERROR: " << strerror(errno);
        _errfp = fdopen(errfds[0], "r");
        ASSERT_TRUE(_errfp != NULL);
    }
}

/*
 * void
 * ChildContext::tearDown(void)
 *      Tear down test environment on a child process.
 */
void
ChildContext::tearDown(void)
{
    int  status;

    if (PFC_EXPECT_FALSE(::testing::Test::HasFailure())) {
        status = 1;
    }
    else {
        status = getStatus();
        if (!checkUnclosedFd()) {
            status = 127;
        }
    }

    _exit(status);
}
