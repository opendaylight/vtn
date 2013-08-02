/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_pidfile.cc - Test for functions defined in libpfc_cmd/pidfile.c
 */

#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pfc/util.h>
#include <cmdutil.h>
#include "test.h"
#include "flock_subr.hh"
#include "signal_subr.hh"
#include "misc.hh"
#include "tmpfile.hh"
#include "random.hh"

/*
 * Base name of temporary file.
 */
#define	PF_TMPFILE		OBJDIR "/tmp_test_pidfile"
#define	PF_TMPDIR		OBJDIR "/tmp_test_pidfile_dir"

/*
 * Required permission bits for PID file.
 */
#define	PF_REQPERMS		(0644U)

/*
 * Internal buffer size used to read file contents.
 */
#define	PF_BUFSIZE		32

/*
 * How long, in milliseconds, sub_flock command should hold the reader lock of
 * the PID file. This is used by PidInstallLockTest.
 */
#define	PF_RDLCK_HOLDTIME	200		/* 200 milliseconds */

/*
 * How long, in seconds, pidfile_install() will wait for the PID file lock
 * if the file is reader-locked. This is defined in libpfc_cmd/pidfile.c
 * as PIDFILE_LOCK_TIMEOUT.
 */
#define	PF_RDLCK_TIMEOUT	3000		/* 3 seconds */

/*
 * How long, in seconds, sub_flock command should hold the reader or writer
 * lock of the PID file. This is used by PidInstallFailLockTest.
 */
#define	PF_LOCK_HOLDTIME	10000		/* 10 seconds */

/*
 * Timeout, in seconds, to wait notification from sub_flock.
 */
#define	PF_NOTIFY_TIMEOUT	10		/* 10 seconds */

/*
 * Allowable error, in nanoseconds, of time for which pidfile_install() blocks.
 */
#define	PF_BLOCKTIME_ERROR	500000000	/* 500 milliseconds */

/*
 * Class which represents a temporary PID file.
 * Created PID file will be removed by destructor.
 */
class TmpPidFile
{
public:
    explicit TmpPidFile(const char *path);
    ~TmpPidFile();

    static int	createPath(std::string &path);

    void	readContents(std::string &contents) throw(std::runtime_error);
    pid_t	readPID(void) throw(std::runtime_error);

    inline int
    forWrite(void)
    {
        int	err(pidfile_open(&_pf, _path.c_str()));
        if (err == 0) {
            _created = true;
        }

        return err;
    }

    inline int
    forRead(void)
    {
        return pidfile_open_rdonly(&_pf, _path.c_str());
    }

    inline void
    closeHandle(void)
    {
        if (_pf != -1) {
            pidfile_close(_pf);
            _pf = -1;
        }
    }

    inline pfc_pidf_t
    operator*() const
    {
        return _pf;
    }

    inline const char *
    getPath(void) const
    {
        return _path.c_str();
    }

private:
    pfc_pidf_t	_pf;
    bool	_created;
    std::string	_path;
};

/*
 * int
 * TmpPidFile::createPath(std::string &path)
 *	Create nonexistent file path.
 *
 * Calling/Exit State:
 *	Upon successful completion, file path is stored to \path' and
 *	zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpPidFile::createPath(std::string &path)
{
    TmpFile	tmpfile(PF_TMPFILE);

    int	err(tmpfile.createFile());
    if (err == 0) {
        path = tmpfile.getPath();
    }

    return err;
}

/*
 * TmpPidFile::TmpPidFile(const char *path)
 *	Constructor of temporary PID file instance.
 */
TmpPidFile::TmpPidFile(const char *path)
    : _pf(-1), _created(false), _path(path)
{
}

/*
 * TmpPidFile::TmpPidFile()
 *	Destructor of temporary PID file instance.
 */
TmpPidFile::~TmpPidFile()
{
    closeHandle();
    (void)unlink(_path.c_str());
}

/*
 * void
 * TmpPidFile::readContents(std::string &contents) throw(std::runtime_error)
 *	Read contents of the PID file, and put it to the specified string.
 */
void
TmpPidFile::readContents(std::string &contents) throw(std::runtime_error)
{
    int	fd(static_cast<int>(_pf));

    if (fd == -1) {
        throw std::runtime_error("PID file is not opened.");
    }

    // Rewind file pointer.
    if (lseek(fd, 0, SEEK_SET) == -1) {
        THROW_ERRNO("lseek() failed", errno);
    }

    for (;;) {
        char	buf[PF_BUFSIZE];
        ssize_t	sz(read(fd, buf, sizeof(buf)));

        if (sz == -1) {
            int	err(errno);
            if (err == EINTR) {
                continue;
            }
            THROW_ERRNO("read() failed", errno);
        }
        if (sz == 0) {
            break;
        }
        contents.append(buf, sz);
    }
}

/*
 * pid_t
 * TmpPidFile::readPID(void) throw(std::runtime_error)
 *	Read process ID in the PID file.
 */
pid_t
TmpPidFile::readPID(void) throw(std::runtime_error)
{
    std::string	contents;
    readContents(contents);
    const char	*line(contents.c_str());
    char	*p;

    errno = 0;
    long	value(strtol(line, &p, 10));
    int	err(errno);
    if (err != 0) {
        THROW_ERRNO("strtol() failed", err);
    }
    if (*p != '\n' || *(p + 1) != '\0') {
        std::ostringstream	stream;
        stream << "Unexpected contents of PID file: " << contents;
        throw std::runtime_error(stream.str());
    }
    int64_t	v64(static_cast<int64_t>(value));
    if (v64 <= 0 || v64 > INT_MAX) {
        std::ostringstream	stream;
        stream << "Unexpected PID in PID file: " << contents;
        throw std::runtime_error(stream.str());
    }

    return static_cast<pid_t>(value);
}

/*
 * Abstract class for test which verifies PID file lock feature.
 */
class PidLockTest
{
public:
    PidLockTest(int holdtime) : _rdonly(false), _holdtime(holdtime)
                              , _sigusr1(SIGUSR1) {}
    virtual ~PidLockTest() {}

    void	check(FlockCmd &cmd) throw(std::runtime_error);

    virtual void	verify(TmpPidFile &pfile, FlockCmd &cmd) = 0;
    virtual void	verifyAfter(TmpPidFile &pfile) = 0;

    inline void
    setReadOnly(void)
    {
        _rdonly = true;
    }

protected:
    bool		_rdonly;
    int			_holdtime;
    std::string		_garbage;
    SignalHandler	_sigusr1;
};

/*
 * void
 * PidLockTest::check(FlockCmd &cmd) throw(std::runtime_error)
 *	Invoke sub_flock command, and then run tests.
 */
void
PidLockTest::check(FlockCmd &cmd) throw(std::runtime_error)
{
    // Create temporary PID file.
    std::string	path;
    EXASSERT_EQ(0, TmpPidFile::createPath(path));
    TmpPidFile	pfile(path.c_str());
    EXASSERT_EQ(0, pfile.forWrite());
    pfc_pidf_t	pf(*pfile);

    struct stat	sbuf;
    EXASSERT_EQ(0, fstat(static_cast<int>(pf), &sbuf));
    EXASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);

    if (_rdonly) {
        // Open as read only.
        pfile.closeHandle();
        EXASSERT_EQ(0, pfile.forRead());
        pf = *pfile;

        int	flags(fcntl(static_cast<int>(pf), F_GETFL));
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
    }
    else {
        // Write garbage to the PID file.
        RandomGenerator	rand;
        uint64_t	u64;

        RANDOM_INTEGER(rand, u64);
        FILE	*fp(fopen(path.c_str(), "w"));
        EXASSERT_TRUE(fp != NULL);
        std::ostringstream	stream;
        stream << "pidfile.c test: " << getpid() << ": " << u64 << std::endl;
        _garbage = stream.str();
        fprintf(fp, "%s", _garbage.c_str());
        fclose(fp);

        EXASSERT_EQ(0, fstat(static_cast<int>(pf), &sbuf));
        EXASSERT_NE(static_cast<off_t>(0), sbuf.st_size);
    }

    // Install signal handler to catch notification from sub_flock.
    _sigusr1.bind(pthread_self());
    if (!_sigusr1.install()) {
        THROW_MESSAGE(_sigusr1.getError());
    }

    // Invoke sub_flock command.
    cmd.setup(_holdtime);
    cmd.invoke(path.c_str());

    // Wait for notification from sub_flock.
    pfc_timespec_t	start, end;

    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    start.tv_sec += PF_NOTIFY_TIMEOUT;

    sig_atomic_t	received;
    while ((received = _sigusr1.getReceived()) == 0) {
        struct timespec	ts;

        EXASSERT_EQ(0, pfc_clock_gettime(&end));
        EXASSERT_TRUE(pfc_clock_compare(&start, &end) >= 0);

        ts.tv_sec = 0;
        ts.tv_nsec = 100;
        nanosleep(&ts, NULL);
    }

    // Run test.
    verify(pfile, cmd);

    // Terminate sub_flock command.
    cmd.terminate();

    // Run test which should be invoked after sub_flock command is killed.
    verifyAfter(pfile);
}

/*
 * Class which ensures that pidfile_install() succeeds after the reader lock
 * is released.
 */
class PidInstallLockTest
    : public PidLockTest
{
public:
    PidInstallLockTest() : PidLockTest(PF_RDLCK_HOLDTIME) {}

    void	verify(TmpPidFile &pfile, FlockCmd &cmd)
        throw(std::runtime_error);
    void	verifyAfter(TmpPidFile &pfile) throw(std::runtime_error);
};

/*
 * void
 * PidInstallLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
 *     throw(std::runtime_error)
 *	Install PID into the PID file, and ensures that it succeeds.
 */
void
PidInstallLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
    throw(std::runtime_error)
{
    // Install PID.
    pfc_timespec_t	start, end;
    pfc_pidf_t	pf(*pfile);
    pid_t	owner(-1);

    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    EXASSERT_EQ(0, pidfile_install(pf, &owner));
    EXASSERT_EQ(0, pfc_clock_gettime(&end));
    EXASSERT_EQ(0, owner);

    // pidfile_install() must block.
    pfc_timespec_sub(&end, &start);

    pfc_timespec_t  reqtime;
    pfc_clock_msec2time(&reqtime, _holdtime);
    
    pfc_timespec_t  errtime = {0, PF_BLOCKTIME_ERROR};
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
 * void
 * PidInstallLockTest::verifyAfter(TmpPidFile &pfile) throw(std::runtime_error)
 *	Ensure that PID is installed into the PID file.
 */
void
PidInstallLockTest::verifyAfter(TmpPidFile &pfile) throw(std::runtime_error)
{
    pid_t	pid(pfile.readPID());
    EXASSERT_EQ(getpid(), pid);
}

/*
 * Class which ensures that pidfile_install() fails because sub_flock command
 * holds the lock.
 */
class PidInstallFailLockTest
    : public PidLockTest
{
public:
    PidInstallFailLockTest() : PidLockTest(PF_LOCK_HOLDTIME) {}

    void	verify(TmpPidFile &pfile, FlockCmd &cmd)
        throw(std::runtime_error);
    void	verifyAfter(TmpPidFile &pfile) throw(std::runtime_error);
};

/*
 * void
 * PidInstallFailLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
 *     throw(std::runtime_error)
 *	Install PID into the PID file, and ensures that it fails.
 */
void
PidInstallFailLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
    throw(std::runtime_error)
{
    // Install PID.
    pfc_timespec_t	start, end;
    pfc_pidf_t	pf(*pfile);
    pid_t	owner(-1);

    int		reqerr;
    pid_t	reqowner;
    pfc_timespec_t  reqtime;

    if (cmd.getFlags() & FlockCmd::C_WRITER) {
        // If sub_flock command is holding the writer lock, pidfile_install()
        // will return EAGAIN immediately.
        reqerr = EAGAIN;
        reqowner = cmd.getPID();
        reqtime.tv_sec = 0;
        reqtime.tv_nsec = 0;
    }
    else {
        // pidfile_install() tries to acquire the lock within 3 seconds.
        reqerr = ETIMEDOUT;
        reqowner = 0;
        pfc_clock_msec2time(&reqtime, PF_RDLCK_TIMEOUT);
    }

    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    EXASSERT_EQ(reqerr, pidfile_install(pf, &owner));
    EXASSERT_EQ(0, pfc_clock_gettime(&end));
    EXASSERT_EQ(reqowner, owner);

    // pidfile_install() must not block.
    pfc_timespec_sub(&end, &start);

    pfc_timespec_t  errtime = {0, PF_BLOCKTIME_ERROR};
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
 * void
 * PidInstallFailLockTest::verifyAfter(TmpPidFile &pfile)
 *     throw(std::runtime_error)
 *	Ensure that PID file is not changed.
 */
void
PidInstallFailLockTest::verifyAfter(TmpPidFile &pfile)
    throw(std::runtime_error)
{
    std::string	contents;
    pfile.readContents(contents);

    if (contents != _garbage) {
        std::ostringstream	stream;
        stream << "=== Unexpected contents of the PID file:" << std::endl
               << contents
               << "=== Expected:" << std::endl
               << _garbage
               << "=== End of PID file";
        THROW_MESSAGE(stream.str());
    }
}

/*
 * Class which ensures that pidfile_getowner() works correctly.
 */
class PidOwnerLockTest
    : public PidLockTest
{
public:
    PidOwnerLockTest() : PidLockTest(PF_LOCK_HOLDTIME) {}

    void	verify(TmpPidFile &pfile, FlockCmd &cmd)
        throw(std::runtime_error);
    void	verifyAfter(TmpPidFile &pfile) throw(std::runtime_error);
};

/*
 * void
 * PidOwnerLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
 *     throw(std::runtime_error)
 *	Ensure that pidfile_getowner() returns the owner PID if it is holding
 *	the writer lock.
 */
void
PidOwnerLockTest::verify(TmpPidFile &pfile, FlockCmd &cmd)
    throw(std::runtime_error)
{
    pfc_pidf_t	pf(*pfile);

    pid_t	reqowner;
    if (cmd.getFlags() & FlockCmd::C_WRITER) {
        reqowner = cmd.getPID();
    }
    else {
        reqowner = 0;
    }

    pfc_timespec_t	start, end;
    pid_t	owner(-1);	
    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    EXASSERT_EQ(0, pidfile_getowner(pf, &owner));
    EXASSERT_EQ(0, pfc_clock_gettime(&end));
    EXASSERT_EQ(reqowner, owner);

    // pidfile_getowner() must not block.
    pfc_timespec_sub(&end, &start);
    if (!TIMESPEC_EQUALS(&end, 0, PF_BLOCKTIME_ERROR)) {
        std::ostringstream	stream;
        stream << "Unexpected blocking time: " << end.tv_sec << "." 
               << end.tv_nsec;
        THROW_MESSAGE(stream.str());
    }
}

/*
 * void
 * PidOwnerLockTest::verifyAfter(TmpPidFile &pfile)
 *     throw(std::runtime_error)
 *	Ensure that no one holds the PID file lock.
 */
void
PidOwnerLockTest::verifyAfter(TmpPidFile &pfile)
    throw(std::runtime_error)
{
    pfc_pidf_t	pf(*pfile);
    pfc_timespec_t	start, end;
    pid_t	owner(-1);	
    EXASSERT_EQ(0, pfc_clock_gettime(&start));
    EXASSERT_EQ(0, pidfile_getowner(pf, &owner));
    EXASSERT_EQ(0, pfc_clock_gettime(&end));
    EXASSERT_EQ(0, owner);

    // pidfile_getowner() must not block.
    pfc_timespec_sub(&end, &start);
    if (!TIMESPEC_EQUALS(&end, 0, PF_BLOCKTIME_ERROR)) {
        std::ostringstream	stream;
        stream << "Unexpected blocking time: " << end.tv_sec << "." 
               << end.tv_nsec;
        THROW_MESSAGE(stream.str());
    }
}

/*
 * Below are test cases.
 */

/*
 * Test case for pidfile_open().
 */
TEST(pidfile, open)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    pfc_pidf_t	pf;

    // Relative path is not allowed.
    ASSERT_EQ(EINVAL, pidfile_open(&pf, ""));
    ASSERT_EQ(EINVAL, pidfile_open(&pf, "test.pid"));

    std::string	pidpath;
    {
        TmpDir	tmpdir(PF_TMPDIR);

        ASSERT_EQ(0, tmpdir.createDirectory());
        const char	*dirpath(tmpdir.getPath());

        pidpath = dirpath;
        pidpath += "/test.pid";
        const char	*path(pidpath.c_str());

        // PID file can't create under unsafe directory.
        ASSERT_EQ(0, chmod(dirpath, 0777));
        ASSERT_EQ(EPERM, pidfile_open(&pf, path));

        ASSERT_EQ(0, chmod(dirpath, 0770));
        ASSERT_EQ(EPERM, pidfile_open(&pf, path));

        ASSERT_EQ(0, chmod(dirpath, 0755));

        TmpPidFile	pfile(path);
        ASSERT_EQ(0, pfile.forWrite());

        int	fd(static_cast<int>(*pfile));
        ASSERT_NE(-1, fd);

        // Permission must be 0644.
        struct stat	sbuf;
        ASSERT_EQ(0, fstat(fd, &sbuf));
        ASSERT_EQ(PF_REQPERMS, MODE_GETPERMS(sbuf.st_mode));

        // Access mode must be O_RDWR.
        int	flags(fcntl(fd, F_GETFL));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
    }

    // Try to create PID file under invalid directory path.
    ASSERT_EQ(ENOENT,pidfile_open(&pf, pidpath.c_str()));
}

/*
 * Test case for pidfile_open_rdonly().
 */
TEST(pidfile, open_rdonly)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    pfc_pidf_t	pf;
    std::string	pidpath;
    {
        TmpDir	tmpdir(PF_TMPDIR);

        ASSERT_EQ(0, tmpdir.createDirectory());
        const char	*dirpath(tmpdir.getPath());

        pidpath = dirpath;
        pidpath += "/test.pid";
        const char	*path(pidpath.c_str());
        {
            // Try to open PID file which does not exist.
            TmpPidFile	pfile(path);
            ASSERT_EQ(ENOENT, pfile.forRead());
        }

        // Create PID file.
        ASSERT_EQ(0, pidfile_open(&pf, path));
        pidfile_close(pf);

        TmpPidFile	pfile(path);
        ASSERT_EQ(0, pfile.forRead());

        int	fd(static_cast<int>(*pfile));
        ASSERT_NE(-1, fd);

        // Access mode must be O_RDONLY.
        int	flags(fcntl(fd, F_GETFL));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
    }

    // Try to create PID file under invalid directory path.
    ASSERT_EQ(ENOENT,pidfile_open(&pf, pidpath.c_str()));
}

/*
 * Test case for pidfile_close().
 */
TEST(pidfile, close)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    std::string	path;

    // Create temporary file.
    ASSERT_EQ(0, TmpPidFile::createPath(path));
    TmpPidFile	pfile(path.c_str());
    ASSERT_EQ(0, pfile.forWrite());

    int	fd(static_cast<int>(*pfile));
    ASSERT_NE(-1, fcntl(fd, F_GETFL));

    pfile.closeHandle();
    ASSERT_EQ(-1, fcntl(fd, F_GETFL));
}

/*
 * Test case for pidfile_install().
 *
 * - No one blocks installation.
 * - Install PID to empty file.
 */
TEST(pidfile, install_1)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    // Create temporary PID file.
    std::string	path;
    ASSERT_EQ(0, TmpPidFile::createPath(path));
    TmpPidFile	pfile(path.c_str());
    ASSERT_EQ(0, pfile.forWrite());
    pfc_pidf_t	pf(*pfile);

    struct stat	sbuf;
    ASSERT_EQ(0, fstat(static_cast<int>(pf), &sbuf));
    ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);

    // Install PID.
    pid_t	owner(-1);
    ASSERT_EQ(0, pidfile_install(pf, &owner));
    ASSERT_EQ(0, owner);

    // Verify contents of the PID file.
    try {
        pid_t	pid(pfile.readPID());
        ASSERT_EQ(getpid(), pid);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_install().
 *
 * - No one blocks installation.
 * - Install PID to a file which contains garbage.
 */
TEST(pidfile, install_2)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    // Create temporary PID file.
    std::string	path;
    ASSERT_EQ(0, TmpPidFile::createPath(path));
    TmpPidFile	pfile(path.c_str());
    ASSERT_EQ(0, pfile.forWrite());
    pfc_pidf_t	pf(*pfile);

    struct stat	sbuf;
    ASSERT_EQ(0, fstat(static_cast<int>(pf), &sbuf));
    ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);

    // Write garbage to the PID file.
    FILE	*fp(fopen(path.c_str(), "w"));
    ASSERT_NE(reinterpret_cast<FILE *>(NULL), fp);
    fprintf(fp, "pidfile.c test: %d\n", getpid());
    fclose(fp);

    ASSERT_EQ(0, fstat(static_cast<int>(pf), &sbuf));
    ASSERT_NE(static_cast<off_t>(0), sbuf.st_size);

    // Install PID.
    pid_t	owner(-1);
    ASSERT_EQ(0, pidfile_install(pf, &owner));
    ASSERT_EQ(0, owner);

    // Verify contents of the PID file.
    try {
        pid_t	pid(pfile.readPID());
        ASSERT_EQ(getpid(), pid);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_install().
 *
 * - pidfile_install() blocks because another thread holds reader lock of
 *   the PID file, but it will succeed after the lock is released.
 * - Write dummy data to the PID file before the call of pidfile_install().
 *   sub_flock ensures that the contents of the PID file is never changed
 *   while it holds the reader lock.
 */
TEST(pidfile, install_3)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidInstallLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK);
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case which verifies error of pidfile_install().
 */
TEST(pidfile, install_error_1)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    std::string	path;
    pfc_pidf_t	pf;

    {
        // Create temporary PID file, and then close and unlink.
        ASSERT_EQ(0, TmpPidFile::createPath(path));
        TmpPidFile	pfile(path.c_str());
        ASSERT_EQ(0, pfile.forWrite());
        pf = *pfile;
    }

    // Pass closed handle.
    pid_t	owner(-1);
    ASSERT_EQ(EBADF, pidfile_install(pf, &owner));
    ASSERT_EQ(0, owner);

    {
        // Create PID file again.
        TmpPidFile	pfile(path.c_str());
        ASSERT_EQ(0, pfile.forWrite());
        pfile.closeHandle();

        // Open PID file as read only.
        ASSERT_EQ(0, pfile.forRead());
        pf = *pfile;

        owner = -1;
        ASSERT_EQ(EBADF, pidfile_install(pf, &owner));
        ASSERT_EQ(0, owner);
    }
}

/*
 * Test case which verifies error of pidfile_install().
 *
 * - pidfile_install() fails because sub_flock command holds the reader lock
 *   for more than 3 seconds.
 */
TEST(pidfile, install_error_2)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidInstallFailLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK);
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case which verifies error of pidfile_install().
 *
 * - pidfile_install() fails because sub_flock command holds the writer lock
 *   for more than 3 seconds.
 */
TEST(pidfile, install_error_3)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidInstallFailLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK | FlockCmd::C_WRITER);
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_getowner().
 *
 * - Open PID file as read-write mode.
 * - pidfile_getowner() returns no owner if sub_flock command is holding the
 *   reader lock.
 */
TEST(pidfile, getowner_1)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidOwnerLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK);
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_getowner().
 *
 * - Open PID file as read-write mode.
 * - pidfile_getowner() returns process ID of sub_flock command if it is
 *    holding the reader lock.
 */
TEST(pidfile, getowner_2)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidOwnerLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK | FlockCmd::C_WRITER);
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_getowner().
 *
 * - Open PID file as read-only mode.
 * - pidfile_getowner() returns no owner if sub_flock command is holding the
 *   reader lock.
 */
TEST(pidfile, getowner_3)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidOwnerLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK);
        test.setReadOnly();
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pidfile_getowner().
 *
 * - Open PID file as read-only mode.
 * - pidfile_getowner() returns process ID of sub_flock command if it is
 *    holding the reader lock.
 */
TEST(pidfile, getowner_4)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    try {
        PidOwnerLockTest	test;
        FlockCmd	cmd(FlockCmd::C_SIGNAL | FlockCmd::C_PAUSE_SIGNAL |
                            FlockCmd::C_CHECK | FlockCmd::C_WRITER);
        test.setReadOnly();
        test.check(cmd);
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case which verifies error of pidfile_getowner().
 */
TEST(pidfile, getowner_error)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    std::string	path;
    pfc_pidf_t	pf;

    {
        // Create temporary PID file, and then close and unlink.
        ASSERT_EQ(0, TmpPidFile::createPath(path));
        TmpPidFile	pfile(path.c_str());
        ASSERT_EQ(0, pfile.forWrite());
        pf = *pfile;

        pid_t	owner(-1);
        ASSERT_EQ(0, pidfile_getowner(pf, &owner));
        ASSERT_EQ(0, owner);
    }

    // Pass closed handle.
    pid_t	owner(-1);
    ASSERT_EQ(EBADF, pidfile_getowner(pf, &owner));
    ASSERT_EQ(0, owner);
}

/*
 * Test case for pidfile_unlink().
 */
TEST(pidfile, unlink)
{
    bool safe;
    is_testdir_safe(safe);
    RETURN_ON_ERROR();

    if (!safe) {
        // This test requires the current directory to be safe.
        return;
    }

    std::string	path;

    // Create temporary PID file.
    ASSERT_EQ(0, TmpPidFile::createPath(path));
    const char	*p(path.c_str());
    TmpPidFile	pfile(p);
    ASSERT_EQ(0, pfile.forWrite());

    // Ensure that the PID file exists.
    struct stat	sbuf;
    ASSERT_EQ(0, stat(p, &sbuf));

    // Unlink the file.
    pidfile_unlink(p);
    int	ret(stat(p, &sbuf));
    int	err(errno);
    ASSERT_EQ(-1, ret);
    ASSERT_EQ(ENOENT, err);
}
