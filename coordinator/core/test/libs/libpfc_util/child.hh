/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Utility to run test on a child process.
 */

#ifndef _TEST_CHILD_HH
#define _TEST_CHILD_HH

#include <map>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <boost/function.hpp>
#include "misc.hh"

class ChildContext;
typedef boost::function<void (ChildContext *)>     child_func_t;
typedef std::map<int, std::string>       intstrmap_t;

/*
 * Test context on a child process.
 */
class ChildContext
{
public:
    ChildContext();
    ChildContext(uid_t uid, gid_t gid);
    virtual ~ChildContext();

    void   run(void (*func)(ChildContext *child));
    void   run(child_func_t &func);
    void   verify(void);
    void   wait(int &status);

    inline pid_t
    getId(void)
    {
        return _pid;
    }

    inline int
    getStatus(void)
    {
        return _status;
    }

    inline void
    setStatus(int status)
    {
        _status = status;
    }

    inline void
    setErrorList(strlist_t &list)
    {
        _errlist = &list;
    }

protected:
    static const uint32_t   CHECKFD_MODE_COLLECT = PFC_CONST_U(0x1);
    static const uint32_t   CHECKFD_MODE_NOPRINT = PFC_CONST_U(0x2);

    virtual void   tearDown(void);

    void   setUp(void);
    bool   checkUnclosedFd(uint32_t mode = 0);

    pid_t        _pid;                 // child process ID
    pid_t        _running;             // child process ID currently running
    uid_t        _uid;                 // user ID for child process
    gid_t        _gid;                 // group ID for child process
    int          _killed;              // expected signal that killed child
    int          _status;              // exit status
    strlist_t    *_errlist;            // string list to store stderr outputs
    FILE         *_errfp;              // input stream associated with stderr
    intstrmap_t  _fdmap;               // preserve opened file descriptors
};

#endif  /* !_TEST_CHILD_HH */
