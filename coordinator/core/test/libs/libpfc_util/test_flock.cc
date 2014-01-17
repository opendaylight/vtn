/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_flock.cc - Test for functions defined in libpfc_util/flock.c
 */

#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pfc/util.h>
#include <pfc/clock.h>
#include "test.h"
#include "misc.hh"
#include "tmpfile.hh"
#include "signal_subr.hh"
#include "flock_subr.hh"

/*
 * Base name of temporary file.
 */
#define	FLK_TMPFILE		OBJDIR "/tmp_test_flock"
#define	FLK_TMPDIR		OBJDIR "/tmp_test_flock_dir"

/*
 * Name of internal directory lock file.
 */
#define	FLK_DIRLOCK_NAME	".dirlock"

/*
 * Permission bits for temporary file.
 */
#define	FLK_PERM	(S_IRUSR | S_IWUSR)
#define	FLK_PERM2	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/*
 * How long, in milliseconds, the sub_flock command should hold the lock.
 */
#define	FLK_HOLDTIME		200		/* 200 milliseconds */

/*
 * Timeout, in seconds, to wait notification from sub_flock.
 */
#define	FLK_NOTIFY_TIMEOUT	10		/* 10 seconds */

/*
 * Allowable error, in nanoseconds, of time for which pfc_flock_rdlock() or
 * pfc_flock_wrlock() blocks. (5 seconds)
 */
#define	FLK_BLOCKTIME_ERROR_SEC		5
#define	FLK_BLOCKTIME_ERROR_NSEC	0

/*
 * Abstract class which represents test context using sub_flock command.
 */
class LockTest
{
    typedef int	(*flkfunc_t)(pfc_flock_t lk, pid_t *ownerp);

public:
    explicit LockTest(bool writer);
    virtual ~LockTest() {}

    inline int
    lock(pfc_flock_t lk, pid_t *ownerp)
    {
        return (*_lockFunc)(lk, ownerp);
    }

    void	check(FlockCmd &cmd) throw(std::runtime_error);

    virtual void	acquire(FlockCmd &cmd) = 0;

    inline bool
    isWriter(void) const
    {
        return (_openMode == O_WRONLY) ? true : false;
    }

protected:
    pfc_flock_t		_lk;
    SignalHandler	_sigusr1;

private:
    int		_openMode;
    flkfunc_t	_lockFunc;
};

/*
 * LockTest::LockTest(bool writer)
 *	Initialize flock test context.
 */
LockTest::LockTest(bool writer)
    : _lk(-1), _sigusr1(SIGUSR1)
{
    if (writer) {
        _openMode = O_WRONLY;
        _lockFunc = pfc_flock_wrlock;
    }
    else {
        _openMode = O_RDONLY;
        _lockFunc = pfc_flock_rdlock;
    }
}

/*
 * void
 * LockTest::check(FlockCmd &cmd) throw(std::runtime_error)
 *	Run flock test using sub_flock command.
 */
void
LockTest::check(FlockCmd &cmd) throw(std::runtime_error)
{
    // Create lock file.
    TmpFile	tmpfile(FLK_TMPFILE);
    EXASSERT_EQ(0, tmpfile.createFile());
    const char	*path(tmpfile.getPath());

    // Open lock file.
    FdRef	fdref(reinterpret_cast<int *>(&_lk));
    EXASSERT_EQ(0, pfc_flock_open(&_lk, path, _openMode, FLK_PERM));
    EXASSERT_NE(-1, _lk);

    // Ensure that no one holds the lock of this file.
    pid_t	owner(-1);
    EXASSERT_EQ(0, pfc_flock_getowner(_lk, &owner, PFC_TRUE));
    EXASSERT_EQ(0, owner);
    owner = -1;
    EXASSERT_EQ(0, pfc_flock_getowner(_lk, &owner, PFC_FALSE));
    EXASSERT_EQ(0, owner);

    // Set up arguments for sub_flock.
    cmd.setup(FLK_HOLDTIME);

    // Install signal handler to catch notification from sub_flock.
    _sigusr1.bind(pthread_self());
    if (!_sigusr1.install()) {
        THROW_MESSAGE(_sigusr1.getError());
    }

    // Invoke sub_flock command.
    cmd.invoke(path);

    // Wait for notification from sub_flock.
    pfc_timespec_t	start, end;

    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    start.tv_sec += FLK_NOTIFY_TIMEOUT;

    sig_atomic_t	received;
    while ((received = _sigusr1.getReceived()) == 0) {
        struct timespec	ts;

        EXASSERT_EQ(0, pfc_clock_gettime(&end));
        EXASSERT_TRUE(pfc_clock_compare(&start, &end) >= 0);

        ts.tv_sec = 0;
        ts.tv_nsec = 100;
        nanosleep(&ts, NULL);
    }

    // Verify the result of pfc_flock_getowner().
    pid_t	cpid(cmd.getPID());
    owner = -1;
    EXASSERT_EQ(0, pfc_flock_getowner(_lk, &owner, PFC_TRUE));
    EXASSERT_EQ(owner, cpid);

    owner = -1;
    uint32_t	flags(cmd.getFlags());
    if (flags & FlockCmd::C_WRITER) {
        EXASSERT_EQ(0, pfc_flock_getowner(_lk, &owner, PFC_FALSE));
        EXASSERT_EQ(cpid, owner);
    }
    else {
        EXASSERT_EQ(0, pfc_flock_getowner(_lk, &owner, PFC_FALSE));
        EXASSERT_EQ(0, owner);
    }

    // Try to acquire the lock.
    acquire(cmd);

    if ((flags & cmd.C_PAUSE) == 0) {
        // sub_flock must exit with status zero.
        cmd.reap();
    }
    else {
        cmd.terminate();
    }

    if (flags & cmd.C_SIGNAL) {
        // SIGUSR1 must be caught during test.
        EXASSERT_TRUE(received < _sigusr1.getReceived());
    }
    else {
        EXASSERT_EQ(received, _sigusr1.getReceived());
    }

    pfc_flock_t	lk(_lk);
    _lk = -1;
    EXASSERT_EQ(0, pfc_flock_close(lk));
}

/*
 * Class used for flock test with blocking mode.
 */
class BlockingLockTest
    : public LockTest
{
public:
    explicit BlockingLockTest(bool writer) : LockTest(writer) {}

    void	acquire(FlockCmd &cmd) throw(std::runtime_error);
};

/*
 * void
 * BlockingLockTest::acquire(FlockCmd &cmd) throw(std::runtime_error)
 *	Try to acquire flock with blocking mode, and verify the results.
 */
void
BlockingLockTest::acquire(FlockCmd &cmd) throw(std::runtime_error)
{
    uint32_t	flags(cmd.getFlags());
    pfc_timespec_t	start, end;

    if (flags & cmd.C_SIGNAL) {
        uint32_t	count(0);

        // Lock function will get EINTR error.
        EXASSERT_EQ(0, pfc_clock_gettime(&start));
        for (;;) {
            int	err(lock(_lk, NULL));
            if (err == 0) {
                break;
            }
            EXASSERT_EQ(EINTR, err);
            count++;
        }
        EXASSERT_EQ(0, pfc_clock_gettime(&end));
        EXASSERT_NE(0U, count);
    }
    else {
        EXASSERT_EQ(0, pfc_clock_gettime(&start));
        EXASSERT_EQ(0, lock(_lk, NULL));
        EXASSERT_EQ(0, pfc_clock_gettime(&end));
    }

    pfc_timespec_sub(&end, &start);

    // The lock function must block unless sub_flock holds reader lock.
    pfc_timespec_t  reqtime;
    if ((flags & cmd.C_WRITER) || isWriter()) {
        pfc_clock_msec2time(&reqtime, FLK_HOLDTIME);
    }
    else {
        reqtime.tv_sec = 0;
        reqtime.tv_nsec = 0;
    }

    pfc_timespec_t  errtime = {
        FLK_BLOCKTIME_ERROR_SEC,
        FLK_BLOCKTIME_ERROR_NSEC,
    };
    if (pfc_clock_compare(&reqtime, &end) > 0) {
        pfc_timespec_t  ts = reqtime;
        pfc_timespec_sub(&ts, &end);
        if (pfc_clock_compare(&ts, &errtime) > 0) {
            std::ostringstream	stream;
            stream << "Too short blocking time: " << end.tv_sec << "." 
                   << end.tv_nsec;
            THROW_MESSAGE(stream.str());
        }
    }

    pfc_timespec_add(&reqtime, &errtime);
    if (pfc_clock_compare(&reqtime, &end) < 0) {
        std::ostringstream	stream;
        stream << "Too long blocking time: " << end.tv_sec << "." 
               << end.tv_nsec;
        THROW_MESSAGE(stream.str());
    }
}

/*
 * Class used for flock test with non-blocking mode.
 */
class NonBlockingLockTest
    : public LockTest
{
public:
    explicit NonBlockingLockTest(bool writer) : LockTest(writer) {}

    void	acquire(FlockCmd &cmd) throw(std::runtime_error);
};

/*
 * void
 * NonBlockingLockTest::acquire(FlockCmd &cmd) throw(std::runtime_error)
 *	Try to acquire flock with non-blocking mode, and verify the results.
 */
void
NonBlockingLockTest::acquire(FlockCmd &cmd) throw(std::runtime_error)
{
    uint32_t	flags(cmd.getFlags());
    pfc_timespec_t	start, end;

    pid_t	cpid(cmd.getPID());
    EXASSERT_NE(-1, cpid);

    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    for (;;) {
        pfc_timespec_t	s, e;
        pid_t	owner(-1);

        EXASSERT_EQ(0, pfc_clock_gettime(&s));
        int	err(lock(_lk, &owner));
        EXASSERT_EQ(0, pfc_clock_gettime(&e));

        // The lock function must not block, and must not get EINTR error.
        pfc_timespec_sub(&e, &s);
        EXASSERT_EQ(static_cast<uint64_t>(0), static_cast<uint64_t>(e.tv_sec));

        if (err == 0) {
            EXASSERT_EQ(static_cast<pid_t>(0), owner);
            break;
        }
        EXASSERT_EQ(EAGAIN, err);
        EXASSERT_EQ(cpid, owner);
    }
    EXASSERT_EQ(0, pfc_clock_gettime(&end));
    pfc_timespec_sub(&end, &start);

    // The lock function must block unless sub_flock holds reader lock.
    pfc_timespec_t  reqtime;
    if ((flags & cmd.C_WRITER) || isWriter()) {
        pfc_clock_msec2time(&reqtime, FLK_HOLDTIME);
    }
    else {
        reqtime.tv_sec = 0;
        reqtime.tv_nsec = 0;
    }

    pfc_timespec_t  errtime = {
        FLK_BLOCKTIME_ERROR_SEC,
        FLK_BLOCKTIME_ERROR_NSEC,
    };
    if (pfc_clock_compare(&reqtime, &end) > 0) {
        pfc_timespec_t  ts = reqtime;
        pfc_timespec_sub(&ts, &end);
        if (pfc_clock_compare(&ts, &errtime) > 0) {
            std::ostringstream	stream;
            stream << "Too short blocking time: " << end.tv_sec << "." 
                   << end.tv_nsec;
            THROW_MESSAGE(stream.str());
        }
    }

    pfc_timespec_add(&reqtime, &errtime);
    if (pfc_clock_compare(&reqtime, &end) < 0) {
        std::ostringstream	stream;
        stream << "Too long blocking time: " << end.tv_sec << "." 
               << end.tv_nsec;
        THROW_MESSAGE(stream.str());
    }
}

/*
 * Below are test cases.
 */

/*
 * Test case for pfc_flock_open().
 */
TEST(flock, open)
{
    Umask       um;
    pfc_flock_t	lk(-1);
    FdRef	fdref(reinterpret_cast<int *>(&lk));
    std::string	tpath;

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    {
        // EACCES test.
        TmpFile	tmpfile(FLK_TMPFILE);
        ASSERT_EQ(0, tmpfile.createFile());
        const char	*path(tmpfile.getPath());
        tpath = path;
        ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

        ASSERT_EQ(0, chmod(path, S_IRUSR));
        int err(pfc_flock_open(&lk, path, O_RDWR, FLK_PERM));
        if (privileged) {
            ASSERT_EQ(0, err);
            ASSERT_NE(-1, lk);
            ASSERT_EQ(0, pfc_flock_close(lk));
            lk = -1;
        }
        else {
            ASSERT_EQ(EACCES, err);
            ASSERT_EQ(-1, lk);
        }

        ASSERT_EQ(0, chmod(path, 0));
        err = pfc_flock_open(&lk, path, O_RDONLY, FLK_PERM);
        if (privileged) {
            ASSERT_EQ(0, err);
            ASSERT_NE(-1, lk);
            ASSERT_EQ(0, pfc_flock_close(lk));
            lk = -1;
        }
        else {
            ASSERT_EQ(EACCES, err);
            ASSERT_EQ(-1, lk);
        }

        ASSERT_EQ(0, chmod(path, FLK_PERM));
    }

    // ENOENT test.
    const char	*path(tpath.c_str());
    ASSERT_EQ(ENOENT, pfc_flock_open(&lk, path, O_RDWR, FLK_PERM));
    ASSERT_EQ(-1, lk);

    mode_t oldmask(um.getOldMask());
    umask(oldmask);
    ASSERT_EQ(0, pfc_flock_open(&lk, path, O_RDWR | O_CREAT, FLK_PERM));
    ASSERT_NE(-1, lk);

    struct stat	sbuf;
    if (stat(path, &sbuf) == -1) {
        (void)unlink(path);
        FAIL() << "stat() failed: " << strerror(errno);
    }
    ASSERT_EQ(0, unlink(path));
    ASSERT_EQ(FLK_PERM & (~oldmask), MODE_GETPERMS(sbuf.st_mode));

    // Close-on-exec flag must be set.
    int	flag(fcntl(static_cast<int>(lk), F_GETFD));
    ASSERT_NE(-1, flag);
    ASSERT_TRUE(flag & FD_CLOEXEC);
}

/*
 * Test case for pfc_flock_close().
 */
TEST(flock, close)
{
    Umask       um;
    pfc_flock_t	lk(-1);
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    TmpFile	tmpfile(FLK_TMPFILE);
    ASSERT_EQ(0, tmpfile.createFile());
    const char	*path(tmpfile.getPath());
    ASSERT_NE(reinterpret_cast<const char *>(NULL), path);
    ASSERT_EQ(0, unlink(path));

    mode_t  oldmask(um.getOldMask());
    umask(oldmask);
    ASSERT_EQ(0, pfc_flock_open(&lk, path, O_RDWR | O_CREAT, FLK_PERM2));
    ASSERT_NE(-1, lk);

    struct stat	sbuf;
    if (stat(path, &sbuf) == -1) {
        (void)unlink(path);
        FAIL() << "stat() failed: " << strerror(errno);
    }
    ASSERT_EQ(FLK_PERM2 & (~oldmask), MODE_GETPERMS(sbuf.st_mode));

    int	flag(fcntl(static_cast<int>(lk), F_GETFD));
    ASSERT_NE(-1, flag);
    ASSERT_TRUE(flag & FD_CLOEXEC);

    ASSERT_EQ(0, pfc_flock_close(lk));
    ASSERT_EQ(EBADF, pfc_flock_close(lk));
    lk = -1;
}

/*
 * Test case for pfc_flock_opendir().
 */
TEST(flock, opendir)
{
    Umask       um;
    std::string	tpath;

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    {
        pfc_flock_t	lk(-1);
        FdRef	fdref(reinterpret_cast<int *>(&lk));
        TmpDir	tmpdir(FLK_TMPDIR);
        ASSERT_EQ(0, tmpdir.createDirectory());
        const char	*path(tmpdir.getPath());
        tpath = path;
        ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

        // ENOENT test. (.dirlock does not exist)
        ASSERT_EQ(ENOENT, pfc_flock_opendir(&lk, path, O_RDWR, FLK_PERM));
        ASSERT_EQ(-1, lk);

        // EACCES test. (directory permission)
        ASSERT_EQ(0, chmod(path, S_IRUSR | S_IXUSR));
        int err(pfc_flock_opendir(&lk, path, O_RDWR | O_CREAT, FLK_PERM));
        if (privileged) {
            ASSERT_EQ(0, err);
            ASSERT_NE(-1, lk);
            ASSERT_EQ(0, pfc_flock_closedir(lk));
            lk = -1;
        }
        else {
            ASSERT_EQ(EACCES, err);
            ASSERT_EQ(-1, lk);
        }

        mode_t  oldmask(um.getOldMask());
        umask(oldmask);
        ASSERT_EQ(0, chmod(path, S_IRUSR | S_IWUSR | S_IXUSR));
        ASSERT_EQ(0, pfc_flock_opendir(&lk, path, O_RDWR | O_CREAT, FLK_PERM));
        ASSERT_NE(-1, lk);

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(static_cast<int>(lk), &sbuf))
            << "stat() failed: " << strerror(errno);
        ASSERT_EQ(FLK_PERM & (~oldmask), MODE_GETPERMS(sbuf.st_mode));

        // Close-on-exec flag must be set.
        int	flag(fcntl(static_cast<int>(lk), F_GETFD));
        ASSERT_NE(-1, flag);
        ASSERT_TRUE(flag & FD_CLOEXEC);
        ASSERT_EQ(0, pfc_flock_closedir(lk));
        lk = -1;

        // EACCES test. (.dirlock permission)
        std::string	dirlock(path);
        dirlock.append("/");
        dirlock.append(FLK_DIRLOCK_NAME);
        const char	*dlk(dirlock.c_str());
        ASSERT_EQ(0, chmod(dlk, S_IRUSR));
        err = pfc_flock_opendir(&lk, path, O_RDWR | O_CREAT, FLK_PERM);
        if (privileged) {
            ASSERT_EQ(0, err);
            ASSERT_NE(-1, lk);
            ASSERT_EQ(0, pfc_flock_closedir(lk));
            lk = -1;
        }
        else {
            ASSERT_EQ(EACCES, err);
            ASSERT_EQ(-1, lk);
        }

        ASSERT_EQ(0, tmpdir.cleanup());
    }

    pfc_flock_t	lk(-1);
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    // ENOENT test. (directory does not exist)
    ASSERT_EQ(ENOENT,
              pfc_flock_opendir(&lk, tpath.c_str(), O_RDWR | O_CREAT,
                                FLK_PERM));
    ASSERT_EQ(-1, lk);
}

/*
 * Test case for pfc_flock_closedir().
 */
TEST(flock, closedir)
{
    Umask       um;
    pfc_flock_t	lk(-1);
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    TmpDir	tmpdir(FLK_TMPDIR);
    ASSERT_EQ(0, tmpdir.createDirectory());
    const char	*path(tmpdir.getPath());
    ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

    mode_t  oldmask(um.getOldMask());
    umask(oldmask);
    ASSERT_EQ(0, pfc_flock_opendir(&lk, path, O_RDWR | O_CREAT, FLK_PERM2));
    ASSERT_NE(-1, lk);

    struct stat	sbuf;
    ASSERT_EQ(0, fstat(static_cast<int>(lk), &sbuf))
        << "stat() failed: " << strerror(errno);
    ASSERT_EQ(FLK_PERM2 & (~oldmask), MODE_GETPERMS(sbuf.st_mode));

    int	flag(fcntl(static_cast<int>(lk), F_GETFD));
    ASSERT_NE(-1, flag);
    ASSERT_TRUE(flag & FD_CLOEXEC);

    ASSERT_EQ(0, pfc_flock_closedir(lk));
    ASSERT_EQ(EBADF, pfc_flock_closedir(lk));
    lk = -1;
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Blocking mode.
 * - Sub command holds reader lock.
 */
TEST(flock, rdlock_block_1)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_PAUSE);
        BlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, rdlock_block_2)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER);
        BlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, rdlock_block_3)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_PAUSE);
        BlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command sends SIGUSR1 while holding the lock, so pfc_flock_rdlock()
 *   gets EINTR error.
 * - Sub command exits with holding lock.
 */
TEST(flock, rdlock_block_4)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_SIGNAL);
        BlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Non-blocking mode.
 * - Sub command holds reader lock.
 */
TEST(flock, rdlock_nonblock_1)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_PAUSE);
        NonBlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}


/*
 * Test case of pfc_flock_rdlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, rdlock_nonblock_2)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER);
        NonBlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, rdlock_nonblock_3)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_PAUSE);
        NonBlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_rdlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command sends SIGUSR1 while holding the lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, rdlock_nonblock_4)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_SIGNAL);
        NonBlockingLockTest	lock(false);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case which verifies error of pfc_flock_rdlock().
 */
TEST(flock, rdlock_error)
{
    Umask       umask;
    pfc_flock_t	lk;
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    TmpFile	tmpfile(FLK_TMPFILE);
    ASSERT_EQ(0, tmpfile.createFile());
    const char	*path(tmpfile.getPath());
    ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

    ASSERT_EQ(0, pfc_flock_open(&lk, path, O_WRONLY, FLK_PERM));
    ASSERT_NE(-1, lk);
    pfc_flock_t	lk2(lk);

    ASSERT_EQ(EBADF, pfc_flock_rdlock(lk, NULL));
    ASSERT_EQ(0, pfc_flock_close(lk));
    lk = -1;

    ASSERT_EQ(EBADF, pfc_flock_rdlock(lk2, NULL));
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Blocking mode.
 * - Sub command holds reader lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_block_1)
{
    Umask  um;

    try {
        FlockCmd		cmd(0U);
        BlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Blocking mode.
 * - Sub command holds reader lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, wrlock_block_2)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_PAUSE);
        BlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_block_3)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER);
        BlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, wrlock_block_4)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_PAUSE);
        BlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Blocking mode.
 * - Sub command holds writer lock.
 * - Sub command sends SIGUSR1 while holding the lock, so pfc_flock_rdlock()
 *   gets EINTR error.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_block_5)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_SIGNAL);
        BlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Non-blocking mode.
 * - Sub command holds reader lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_nonblock_1)
{
    Umask  um;

    try {
        FlockCmd		cmd(0U);
        NonBlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Non-blocking mode.
 * - Sub command holds reader lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, wrlock_nonblock_2)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_PAUSE);
        NonBlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_nonblock_3)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER);
        NonBlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command pauses just after unlocking.
 */
TEST(flock, wrlock_nonblock_4)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_PAUSE);
        NonBlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case of pfc_flock_wrlock().
 *
 * - Non-blocking mode.
 * - Sub command holds writer lock.
 * - Sub command sends SIGUSR1 while holding the lock.
 * - Sub command exits with holding lock.
 */
TEST(flock, wrlock_nonblock_5)
{
    Umask  um;

    try {
        FlockCmd		cmd(FlockCmd::C_WRITER | FlockCmd::C_SIGNAL);
        NonBlockingLockTest	lock(true);

        lock.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case which verifies error of pfc_flock_wrlock().
 */
TEST(flock, wrlock_error)
{
    Umask       umask;
    pfc_flock_t	lk;
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    TmpFile	tmpfile(FLK_TMPFILE);
    ASSERT_EQ(0, tmpfile.createFile());
    const char	*path(tmpfile.getPath());
    ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

    ASSERT_EQ(0, pfc_flock_open(&lk, path, O_RDONLY, FLK_PERM));
    ASSERT_NE(-1, lk);
    pfc_flock_t	lk2(lk);

    ASSERT_EQ(EBADF, pfc_flock_wrlock(lk, NULL));
    ASSERT_EQ(0, pfc_flock_close(lk));
    lk = -1;

    ASSERT_EQ(EBADF, pfc_flock_wrlock(lk2, NULL));
}

/*
 * Test case which verifies error of pfc_flock_getowner().
 */
TEST(flock, getowner_error)
{
    pfc_flock_t	lk;
    FdRef	fdref(reinterpret_cast<int *>(&lk));

    TmpFile	tmpfile(FLK_TMPFILE);
    ASSERT_EQ(0, tmpfile.createFile());
    const char	*path(tmpfile.getPath());
    ASSERT_NE(reinterpret_cast<const char *>(NULL), path);

    ASSERT_EQ(0, pfc_flock_open(&lk, path, O_RDWR, FLK_PERM));
    ASSERT_NE(-1, lk);
    pfc_flock_t	lk2(lk);

    pid_t	owner(-1);
    ASSERT_EQ(0, pfc_flock_getowner(lk, &owner, PFC_TRUE));
    ASSERT_EQ(0, owner);
    owner = -1;
    ASSERT_EQ(0, pfc_flock_getowner(lk, &owner, PFC_FALSE));
    ASSERT_EQ(0, owner);

    ASSERT_EQ(0, pfc_flock_close(lk));
    lk = -1;

    owner = -1;
    ASSERT_EQ(EBADF, pfc_flock_getowner(lk2, &owner, PFC_TRUE));
    ASSERT_EQ(0, owner);
    owner = -1;
    ASSERT_EQ(EBADF, pfc_flock_getowner(lk2, &owner, PFC_FALSE));
    ASSERT_EQ(0, owner);
}
