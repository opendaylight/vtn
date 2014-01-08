/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_cloexec.cc - Test for functions defined in libpfc_util/cloexec.c
 */

#include <gtest/gtest.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <poll.h>
#include <pfc/util.h>
#include <pfc/debug.h>
#include "tmpfile.hh"
#include "sub_cloexec/sub_cloexec.h"
#include "signal_subr.hh"
#include "misc.hh"

#define	CLOEXEC_NPIPES		2
#define	CLOEXEC_PIPE_RD		0
#define	CLOEXEC_PIPE_WR		1

#define	CLOEXEC_TOGGLE_LOOP	16

/*
 * Base name of temporary file that keeps stderr output of sub_cloexec.
 */
#define	ERROR_FILE	OBJDIR "/err_sub_cloexec"

/*
 * Pipe channel to communicate with sub_cloexec command.
 */
class PipeChannel
{
public:
    PipeChannel();
    ~PipeChannel();

    void	setup(void) throw(std::runtime_error);
    void	readData(void) throw(std::runtime_error);
    void	setCloExec(pfc_bool_t set) throw(std::runtime_error);
    void	verify(pid_t pid) throw(std::runtime_error);

    inline pfc_bool_t
    getCloExec(void) const
    {
        return _cloexec;
    }

    inline int
    forRead(void) const
    {
        return (_active) ? _pipe[CLOEXEC_PIPE_RD] : -1;
    }

    inline int
    forWrite(void) const
    {
        return (_active) ? _pipe[CLOEXEC_PIPE_WR] : -1;
    }

    inline void
    closeForRead(void)
    {
        closePipe(CLOEXEC_PIPE_RD);
    }

    inline void
    closeForWrite(void)
    {
        closePipe(CLOEXEC_PIPE_WR);
    }

    inline const char *
    getData(void) const
    {
        return _data;
    }

    inline size_t
    getSize(void) const
    {
        return _size;
    }

    inline void
    setExpectedError(std::ostringstream &stream)
    {
        if (getCloExec()) {
            stream << "fcntl(" << _savedPipe[CLOEXEC_PIPE_WR]
                   << ", F_GETFL) failed: errno=" << EBADF << std::endl;
        }
    }

private:
    void	closePipe(int type);

    bool	_active;
    pfc_bool_t	_cloexec;
    size_t	_size;
    int		_pipe[2];
    int		_savedPipe[2];
    char	_data[SUB_CLOEXEC_MAX_OUTSIZE];
};

/*
 * PipeChannel::PipeChannel()
 *	Constructor of pipe channel.
 */
PipeChannel::PipeChannel()
    : _active(false), _cloexec(PFC_FALSE), _size(0)
{
    _pipe[0] = -1;
    _pipe[1] = -1;
    _savedPipe[0] = -1;
    _savedPipe[0] = -1;
}

/*
 * PipeChannel::~PipeChannel()
 *	Destructor of pipe channel.
 */
PipeChannel::~PipeChannel()
{
    for (int *pp(_pipe); pp < PFC_ARRAY_LIMIT(_pipe); pp++) {
        int	fd(*pp);

        if (fd != -1) {
            (void)close(fd);
        }
    }
}

/*
 * void
 * PipeChannel::setup(void) throw(std::runtime_error)
 *	Set up the pipe channel.
 */
void
PipeChannel::setup(void) throw(std::runtime_error)
{
    if (pipe(_pipe) == -1) {
        THROW_ERRNO("pipe() failed", errno);
    }

    for (int i(0); i < static_cast<int>(PFC_ARRAY_CAPACITY(_pipe)); i++) {
        // Set non-blocking flag.
        _savedPipe[i] = _pipe[i];
        EXASSERT_EQ(0, pfc_set_nonblock(_pipe[i], PFC_TRUE));
    }

    _active = true;
}

/*
 * void
 * PipeChannel::readData(void) throw(std::runtime_error)
 *	Read data from the pipe channel.
 */
void
PipeChannel::readData(void) throw(std::runtime_error)
{
    int	fd(forRead());
    size_t	size(SUB_CLOEXEC_MAX_OUTSIZE - _size);
    char	*cursor(_data + _size);

    if (size == 0) {
        THROW_MESSAGE("Too large output from sub_cloexec.");
    }

    do {
        ssize_t	nbytes(read(fd, cursor, size));

        if (nbytes == -1) {
            int	err(errno);

            if (err == EINTR) {
                continue;
            }
            if (err == EAGAIN) {
                break;
            }

            THROW_ERRNO("read() failed", err);
        }

        if (nbytes == 0) {
            _active = false;
            break;
        }

        size -= nbytes;
        _size += nbytes;
        cursor += nbytes;
    } while (size > 0);
}

/*
 * void
 * PipeChannel::setCloExec(pfc_bool_t set) throw(std::runtime_error)
 *	Set close-on-exec flag to writer side of the pipe channel.
 */
void
PipeChannel::setCloExec(pfc_bool_t set) throw(std::runtime_error)
{
    int	fd(forWrite());

    EXASSERT_EQ(0, pfc_set_cloexec(fd, set));
    _cloexec = set;
}

/*
 * void
 * PipeChannel::verify(pid_t pid) throw(std::runtime_error)
 *	Verify output from the sub_cloexec command.
 */
void
PipeChannel::verify(pid_t pid) throw(std::runtime_error)
{
    EXASSERT_FALSE(_active);

    if (_cloexec) {
        EXASSERT_EQ(0, static_cast<int>(_size));
    }
    else {
        char	buf[SUB_CLOEXEC_MAX_OUTSIZE];

        EXASSERT_NE(0, static_cast<int>(_size));
        EXASSERT_NE(SUB_CLOEXEC_MAX_OUTSIZE, static_cast<int>(_size));

        snprintf(buf, sizeof(buf), "%s:%d:%d",
                 SUB_CLOEXEC_NAME, pid, _savedPipe[CLOEXEC_PIPE_WR]);
        _data[_size] = '\0';
        if (strcmp(_data, buf) != 0) {
            std::ostringstream	stream;
            stream << __FILE__ << ":" << __LINE__
                   << ": Unexpected output[" << _data << "], required["
                   << buf << "]";
            throw std::runtime_error(stream.str());
        }
    }
}

/*
 * void
 * PipeChannel::closePipe(int type)
 *	Close the file descriptor specified by the type.
 */
void
PipeChannel::closePipe(int type)
{
    close(_pipe[type]);
    _pipe[type] = -1;
}

/*
 * Test context.
 */
class CloExecContext
{
public:
    CloExecContext();
    ~CloExecContext();

    void	setup(void) throw(std::runtime_error);
    void	setCloExec(int index, pfc_bool_t set) throw(std::runtime_error);
    void	spawn(void) throw(std::runtime_error);
    void	readPipe(void) throw(std::runtime_error);

private:
    pid_t		_pid;
    TmpFile		_tmpfile;
    SignalHandler	_sigpipe;
    PipeChannel		_pipes[CLOEXEC_NPIPES];
};

/*
 * CloExecContext::CloExecContext()
 *	Constructor of cloexec test context.
 */
CloExecContext::CloExecContext()
    : _pid(-1), _tmpfile(ERROR_FILE), _sigpipe(SIGPIPE)
{
}

/*
 * CloExecContext::~CloExecContext()
 *	Destructor of cloexec test context.
 */
CloExecContext::~CloExecContext()
{
    if (_pid != -1) {
        if (kill(_pid, SIGKILL) != -1) {
            (void)waitpid(_pid, NULL, 0);
        }
    }
}

/*
 * void
 * CloExecContext::setup(void) throw(std::runtime_error)
 *	Set up cloexec test environment.
 */
void
CloExecContext::setup(void) throw(std::runtime_error)
{
    for (int index(0); index < CLOEXEC_NPIPES; index++) {
        _pipes[index].setup();
    }

    // Ignore SIGPIPE.
    if (!_sigpipe.ignore()) {
        THROW_MESSAGE(_sigpipe.getError());
    }
}

/*
 * void
 * CloExecContext::setCloExec(int index, pfc_bool_t set)
 *     throw(std::runtime_error)
 *	Set close-on-exec flag to pipe channel at the specified index.
 */
void
CloExecContext::setCloExec(int index, pfc_bool_t set) throw(std::runtime_error)
{
    if (index >= CLOEXEC_NPIPES) {
        THROW_MESSAGE("Internal error: invalid pipe index.");
    }

    _pipes[index].setCloExec(set);
}

/*
 * void
 * CloExecContext::spawn(void) throw(std::runtime_error)
 *	Spawn a child process and execute sub_cloexec command.
 */
void
CloExecContext::spawn(void) throw(std::runtime_error)
{
    EXASSERT_EQ(0, _tmpfile.createFile());

    // Prepare arguments for sub_cloexec.
    ArgVector	argv;
    EXASSERT_TRUE(argv.push(SUB_CLOEXEC_NAME));

    for (int index(0); index < CLOEXEC_NPIPES; index++) {
        int	fd(_pipes[index].forWrite());

        EXASSERT_TRUE(argv.push(fd));

        // Close reader pipe on child process.
        fd = _pipes[index].forRead();
        EXASSERT_EQ(0, pfc_set_cloexec(fd, PFC_TRUE));
    }

    // Create a child process.
    _pid = fork();
    EXASSERT_NE(-1, _pid);

    if (_pid == 0) {
        if (dup2(_tmpfile.getDescriptor(), 2) == -1) {
            fprintf(stderr, "dup2() failed: %s\n", strerror(errno));

            exit(EXIT_FAILURE);
        }

        // Execute sub_cloexec command.
        execv(SUB_CLOEXEC_PATH, *argv);

        fprintf(stderr, "execv() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int index(0); index < CLOEXEC_NPIPES; index++) {
        _pipes[index].closeForWrite();
    }
}

/*
 * void
 * CloExecContext::readPipe(void) throw(std::runtime_error)
 *	Read output from sub_cloexec command.
 */
void
CloExecContext::readPipe(void) throw(std::runtime_error)
{
    sigset_t		empty;
    struct timespec	to;

    sigemptyset(&empty);
    to.tv_sec = SUB_CLOEXEC_IO_TIMEOUT;
    to.tv_nsec = 0;

    for (;;) {
        struct pollfd	array[CLOEXEC_NPIPES];
        int	index(0), idxmap[CLOEXEC_NPIPES];

        for (int i(0); i < CLOEXEC_NPIPES; i++) {
            int	fd(_pipes[i].forRead());

            if (fd != -1) {
                struct pollfd	*pfd(&array[index]);

                pfd->fd = fd;
                pfd->events = POLLIN;
                pfd->revents = 0;
                idxmap[index] = i;
                index++;
            }
        }

        nfds_t	nfds(static_cast<nfds_t>(index));
        if (nfds == 0) {
            break;
        }

	for (;;) {
            int	ret(ppoll(array, nfds, &to, &empty));

            if (ret == -1) {
                int	err(errno);

                if (err == EINTR) {
                    continue;
                }

                THROW_ERRNO("ppoll() failed", err);
            }
            if (ret == 0) {
                THROW_MESSAGE("I/O timeout.");
            }

            break;
	};

        for (nfds_t i(0); i < nfds; i++) {
            int	pidx(idxmap[i]);
            _pipes[pidx].readData();
        }
    }

    int		status;
    pid_t	cpid;
    for (;;) {
        cpid = waitpid(_pid, &status, 0);
        if (cpid != -1) {
            break;
        }

        int	err(errno);
        if (err == EINTR) {
            continue;
        }
        THROW_ERRNO("waitpid() failed", err);
    }

    EXASSERT_EQ(cpid, _pid);
    _pid = -1;
    EXASSERT_EQ(0, status);

    // Construct expected stderr output.
    std::ostringstream	stream;
    for (int i(0); i < CLOEXEC_NPIPES; i++) {
        PipeChannel	*pp(&_pipes[i]);

        pp->setExpectedError(stream);

        // Verify channel output.
        pp->verify(cpid);
    }

    std::string	exerr(stream.str());

    // Obtain stderr output.
    size_t	errsize;
    EXASSERT_EQ(0, _tmpfile.getSize(errsize));
    if (errsize != 0) {
        std::string	errout;
        ASSERT_EQ(0, _tmpfile.readAsString(errout));

        if (exerr.compare(errout) != 0) {
            std::ostringstream	ostr;
            ostr << "=== Unexpected error on sub_cloexec:" << std::endl
                 << errout
                 << "=== Expected:" << std::endl
                 << exerr
                 << "=== End of output";
            throw std::runtime_error(ostr.str());
        }
    }
    else if (exerr.size() != 0) {
        THROW_MESSAGE("No error output.");
    }
}

/*
 * Class which represents file descriptor returned by pfc_open_cloexec().
 */
class OpenCloExec
{
public:
    OpenCloExec(const char *path, int flags)
        : _fd(pfc_open_cloexec(path, flags)), _stream(NULL)
    {
        fixUp();
    }

    OpenCloExec(const char *path, int flags, mode_t mode)
        : _fd(pfc_open_cloexec(path, flags, mode)), _stream(NULL)
    {
        fixUp();
    }

    OpenCloExec(FILE *fp) : _fd(-1), _stream(fp)
    {
        if (_stream == NULL) {
            _error = errno;
        }
        else {
            _error = 0;
        }
    }

    virtual ~OpenCloExec()
    {
        if (_fd != -1) {
            (void)close(_fd);
        }
        else if (_stream != NULL) {
            (void)fclose(_stream);
        }
    }

    int
    getDescriptor(void) const
    {
        return (_stream != NULL) ? fileno(_stream) : _fd;
    }

    FILE *
    getStream(void) const
    {
        return _stream;
    }

    int
    getError(void) const
    {
        return _error;
    }

protected:
    OpenCloExec(int fd) : _fd(fd), _stream(NULL)
    {
        fixUp();
    }

private:
    void
    fixUp(void)
    {
        if (_fd == -1) {
            _error = errno;
        }
        else {
            _error = 0;
        }
    }

    int		_fd;
    FILE	*_stream;
    int		_error;
};

/*
 * Class which represents file descriptor returned by pfc_openat_cloexec().
 */
class OpenAtCloExec
    : public OpenCloExec
{
public:
    OpenAtCloExec(int dirfd, const char *path, int flags)
        : OpenCloExec(pfc_openat_cloexec(dirfd, path, flags)) {}

    OpenAtCloExec(int dirfd, const char *path, int flags, mode_t mode)
        : OpenCloExec(pfc_openat_cloexec(dirfd, path, flags, mode)) {}
};

/*
 * Class which represents a pipe created by pfc_pipe_open().
 */
class PfcPipe
{
public:
    PfcPipe(int flags)
    {
        _pipe[0] = _pipe[1] = -1;
        _ret = pfc_pipe_open(_pipe, flags);
        _error = (_ret == 0) ? 0 : errno;
    }

    ~PfcPipe()
    {
        for (int i = 0; i < 2; i++) {
            if (_pipe[i] != -1) {
                (void)close(_pipe[i]);
            }
        }
    }

    int
    getResult(void)
    {
        return _ret;
    }

    int
    getError(void)
    {
        return _error;
    }

    int
    getDescriptor(int index)
    {
        return _pipe[index];
    }

private:
    int		_pipe[2];
    int		_ret;
    int		_error;
};

/*
 * Close the given file descriptor, and restore on destructor.
 */
class CloseFd
{
public:
    CloseFd(int fd) : _fd(fd)
    {
        _keep = dup(fd);
        PFC_VERIFY(_keep != -1);
        PFC_VERIFY_INT(close(fd), 0);
    }

    ~CloseFd()
    {
        PFC_VERIFY_INT(dup2(_keep, _fd), _fd);
        PFC_VERIFY_INT(close(_keep), 0);
    }

private:
    int		_fd;
    int		_keep;
};

/*
 * Ensure the given two file descriptor is identical.
 */
static void
fd_equals(int fd1, int fd2)
{
    struct stat	sbuf1, sbuf2;

    ASSERT_EQ(0, fstat(fd1, &sbuf1));
    ASSERT_EQ(0, fstat(fd2, &sbuf2));

    ASSERT_EQ(sbuf2.st_dev, sbuf1.st_dev);
    ASSERT_EQ(sbuf2.st_ino, sbuf1.st_ino);
}

/*
 * Return close-on-exec flag of the given file descriptor.
 */
static int
get_cloexec(int fd)
{
    int	flag(fcntl(fd, F_GETFD));
    if (PFC_EXPECT_TRUE(flag != -1)) {
        flag &= FD_CLOEXEC;
    }

    return flag;
}

/*
 * Below are test cases.
 */

/*
 * Test case that specifies invalid file descriptor.
 */
TEST(cloexec, invalid_fd)
{
    int	fd(socket(AF_UNIX, SOCK_STREAM, 0));
    ASSERT_NE(-1, fd);

    ASSERT_NE(-1, close(fd));
    ASSERT_EQ(EBADF, pfc_set_cloexec(fd, PFC_TRUE));
    ASSERT_EQ(EBADF, pfc_set_cloexec(fd, PFC_FALSE));
}

/*
 * Execute sub_cloexec command, and ensure that we can communicate with it
 * via pipe channel.
 *	- Use default close-on-exec flag.
 */
TEST(cloexec, exec_1)
{
    CloExecContext	context;

    try {
        context.setup();
        context.spawn();
        context.readPipe();
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Execute sub_cloexec command, and ensure that we can communicate with it
 * via pipe channel.
 *	- Change close-on-exec flag again and again, and clear it.
 */
TEST(cloexec, exec_2)
{
    CloExecContext	context;

    try {
        context.setup();
        for (int i(0); i < CLOEXEC_TOGGLE_LOOP; i++) {
            context.setCloExec(1, PFC_TRUE);
            context.setCloExec(1, PFC_TRUE);
            context.setCloExec(1, PFC_FALSE);
            context.setCloExec(1, PFC_FALSE);
            context.setCloExec(1, PFC_TRUE);
        }
        context.setCloExec(1, PFC_FALSE);
        context.spawn();
        context.readPipe();
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Execute sub_cloexec command, and ensure that we can communicate with it
 * via pipe channel.
 *	- Set close-on-exec flag.
 */
TEST(cloexec, exec_close_1)
{
    CloExecContext	context;

    try {
        context.setup();
        context.setCloExec(1, PFC_TRUE);
        context.spawn();
        context.readPipe();
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Execute sub_cloexec command, and ensure that we can communicate with it
 * via pipe channel.
 *	- Change close-on-exec flag again and again, and set it.
 */
TEST(cloexec, exec_close_2)
{
    CloExecContext	context;

    try {
        context.setup();
        for (int i(0); i < CLOEXEC_TOGGLE_LOOP; i++) {
            context.setCloExec(1, PFC_TRUE);
            context.setCloExec(1, PFC_TRUE);
            context.setCloExec(1, PFC_FALSE);
            context.setCloExec(1, PFC_FALSE);
            context.setCloExec(1, PFC_TRUE);
        }
        context.setCloExec(1, PFC_TRUE);
        context.spawn();
        context.readPipe();
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }
}

/*
 * Test case for pfc_open_cloexec().
 */
TEST(cloexec, open)
{
    Umask	umask;

    // ENOENT error.
    {
        OpenCloExec	fd("File_Not_Exist", O_RDONLY);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOENT, fd.getError());
    }

    TmpFile	tmpf("test_open_cloexec");
    ASSERT_EQ(0, tmpf.createFile());
    const char	*path(tmpf.getPath());

    // EEXIST error.
    {
        const mode_t	mode(0644);
        OpenCloExec	fd(path, O_RDWR | O_CREAT | O_EXCL, mode);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(EEXIST, fd.getError());
    }

    // Read-only mode.
    {
        OpenCloExec	fd(path, O_RDONLY);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
    }

    // Create a new file.
    ASSERT_EQ(0, unlink(path));
    {
        const mode_t	mode(0622);
        OpenCloExec	fd(path, O_RDWR | O_CREAT, mode);
        
        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);

        const off_t	fsize(0x10);
        ASSERT_EQ(0, ftruncate(desc, fsize));

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);
        ASSERT_EQ(mode, sbuf.st_mode & 07777);
    }

    // Truncate flag test.
    {
        OpenCloExec	fd(path, O_WRONLY | O_TRUNC);
        
        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(0U, sbuf.st_size);
    }
}

/*
 * Test case for pfc_openat_cloexec().
 */
TEST(cloexec, openat)
{
    Umask	umask;

    TmpDir	tmpd("openat_cloexec");
    ASSERT_EQ(0, tmpd.createDirectory());
    const char	*dirpath(tmpd.getPath());
    int	dirfd(open(dirpath, O_RDONLY));
    ASSERT_NE(-1, dirfd);
    FdRef dfd(dirfd);

    // ENOENT error.
    {
        OpenAtCloExec	fd(dirfd, "File_Not_Exist", O_RDONLY);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOENT, fd.getError());
    }

    // EBADF error.
    {
        OpenAtCloExec	fd(-1, "File_Not_Exist", O_RDONLY);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(EBADF, fd.getError());
    }

    std::string	s(dirpath);
    const char	*fname("test_openat_cloexec");
    s += "/";
    s += fname;
    int	fd(open(s.c_str(), O_CREAT | O_RDWR, 0644));
    ASSERT_NE(-1, fd);
    (void)close(fd);

    // ENOTDIR error.
    {
        int file_fd = open(s.c_str(), O_RDONLY);
        ASSERT_NE(-1, file_fd);
        OpenAtCloExec	fd(file_fd, "File_Not_Exist", O_RDONLY);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOTDIR, fd.getError());
        (void)close(file_fd);
    }

    // EEXIST error.
    {
        OpenAtCloExec	fd(dirfd, fname, O_RDWR | O_CREAT | O_EXCL, 0644);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(EEXIST, fd.getError());
    }

    // Read-only mode.
    {
        OpenAtCloExec	fd(dirfd, fname, O_RDONLY);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
    }

    // Create a new file.
    ASSERT_EQ(0, unlink(s.c_str()));
    {
        const mode_t	mode(0622);
        OpenAtCloExec	fd(dirfd, fname, O_RDWR | O_CREAT, mode);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);

        const off_t	fsize(0x10);
        ASSERT_EQ(0, ftruncate(desc, fsize));

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);
        ASSERT_EQ(mode, sbuf.st_mode & 07777);
    }

    // Truncate flag test.
    {
        OpenAtCloExec	fd(dirfd, fname, O_WRONLY | O_TRUNC);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(0U, sbuf.st_size);
    }
}

/*
 * Test case for pfc_openat_cloexec() with specifying PFC_AT_FDCWD.
 */
TEST(cloexec, openat_cwd)
{
    Umask	umask;

    // ENOENT error.
    {
        OpenAtCloExec	fd(PFC_AT_FDCWD, "File_Not_Exist", O_RDONLY);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOENT, fd.getError());
    }

    TmpFile	tmpf("test_openat_cloexec");
    ASSERT_EQ(0, tmpf.createFile());
    const char	*path(tmpf.getPath());
    const char	*fname(strrchr(path, '/'));
    if (fname != NULL) {
        fname++;
    }
    else {
        fname = path;
    }

    // EEXIST error.
    {
        const mode_t	mode(0644);
        OpenAtCloExec	fd(PFC_AT_FDCWD, fname, O_RDWR | O_CREAT | O_EXCL,
                           mode);

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(EEXIST, fd.getError());
    }

    // Read-only mode.
    {
        OpenAtCloExec	fd(PFC_AT_FDCWD, fname, O_RDONLY);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
    }

    // Create a new file.
    ASSERT_EQ(0, unlink(path));
    {
        const mode_t	mode(0622);
        OpenAtCloExec	fd(PFC_AT_FDCWD, fname, O_RDWR | O_CREAT, mode);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);

        const off_t	fsize(0x10);
        ASSERT_EQ(0, ftruncate(desc, fsize));

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);
        ASSERT_EQ(mode, sbuf.st_mode & 07777);
    }

    // Truncate flag test.
    {
        OpenAtCloExec	fd(PFC_AT_FDCWD, fname, O_WRONLY | O_TRUNC);

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(0U, sbuf.st_size);
    }
}

/*
 * Test case for pfc_pipe_open().
 */
TEST(cloexec, pipe)
{
    // Test invalid flag. */
    for (int flags = 1; flags != 0; flags <<= 1) {
        if (flags & PFC_PIPE_CLOEXEC_NB) {
            continue;
        }

        PfcPipe	pp(flags);
        ASSERT_EQ(-1, pp.getResult());
        ASSERT_EQ(EINVAL, pp.getError());
    }

    int	test_flags[] = {
        0, PFC_PIPE_CLOEXEC, PFC_PIPE_NONBLOCK, PFC_PIPE_CLOEXEC_NB, -1,
    };
    int	*flagp(test_flags);
    int	flags(*flagp);
    do {
        PfcPipe	pp(flags);

        ASSERT_EQ(0, pp.getResult());
        ASSERT_EQ(0, pp.getError());
        ASSERT_NE(-1, pp.getDescriptor(0));
        ASSERT_NE(-1, pp.getDescriptor(1));

        int	reqfd((flags & PFC_PIPE_CLOEXEC) ? FD_CLOEXEC : 0);
        int	reqfl((flags & PFC_PIPE_NONBLOCK) ? O_NONBLOCK : 0);
        for (int i = 0; i < 2; i++) {
            // Test close-on-exec flag.
            int	fd(fcntl(pp.getDescriptor(i), F_GETFD));
            ASSERT_NE(-1, fd);
            ASSERT_EQ(reqfd, fd & FD_CLOEXEC);

            // Test non-blocking flag.
            int	fl(fcntl(pp.getDescriptor(i), F_GETFL));
            ASSERT_NE(-1, fl);
            ASSERT_EQ(reqfl, fl & O_NONBLOCK);
        }

        flagp++;
    } while ((flags = *flagp) != -1);
}

/*
 * Test case for pfc_fopen_cloexec().
 */
TEST(cloexec, fopen)
{
    Umask	umask;

    // ENOENT error.
    {
        OpenCloExec	fd(pfc_fopen_cloexec("File_Not_Exist", "r"));

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOENT, fd.getError());
    }

    TmpFile	tmpf("test_fopen_cloexec");
    ASSERT_EQ(0, tmpf.createFile());
    const char	*path(tmpf.getPath());
    const off_t	fsize(0x1000);
    {
        int	fd(open(path, O_RDWR));
        ASSERT_NE(-1, fd) << "errno = " << strerror(errno);
        FdRef	fref(fd);
        off_t	off(fsize - 1);
        ASSERT_EQ(off, lseek(fd, off, SEEK_SET));

        uint8_t	data(0);
        ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                  write(fd, &data, sizeof(data)));
    }

    // Read-only mode.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path, "r"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Read-write mode.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path, "r+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Append mode.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path, "a"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        off_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(fsize, off);
    }

    // Read and append mode.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path, "a+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        // Initial file offset must be zero.
        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Write only mode with file truncation.
    {
        struct stat	sbuf;
        ASSERT_EQ(0, stat(path, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);

        OpenCloExec	fd(pfc_fopen_cloexec(path, "w"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        // Target file must be truncated.
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);
    }

    // Read and write mode with file truncation.
    {
        struct stat	sbuf;
        ASSERT_EQ(0, truncate(path, fsize));
        ASSERT_EQ(0, stat(path, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);

        OpenCloExec	fd(pfc_fopen_cloexec(path, "w+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        // Target file must be truncated.
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);
    }

    // Create a new file.
    ASSERT_EQ(0, unlink(path));
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path, "w+"));
        
        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        const off_t	fsize(0x10);
        ASSERT_EQ(0, ftruncate(desc, fsize));

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);
        ASSERT_EQ(static_cast<mode_t>(0666), sbuf.st_mode & 07777);
    }

    // Specify dynamically generated mode string.
    {
        std::string	smode("r+");
        OpenCloExec	fd(pfc_fopen_cloexec(path, smode.c_str()));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Specify too long mode string without 'e'.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path,
                                             "a+b++++++++++++++++++++++++"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        // Initial file offset must be zero.
        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Specify too long mode string with 'e'.
    {
        OpenCloExec	fd(pfc_fopen_cloexec(path,
                                             "a+be+++++++++++++++++++++++"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        // Initial file offset must be zero.
        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }
}

/*
 * Test case for PFC_FOPEN_CLOEXEC().
 */
TEST(cloexec, fopen_macro)
{
    Umask	umask;

    // ENOENT error.
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC("File_Not_Exist", "r"));

        ASSERT_EQ(-1, fd.getDescriptor());
        ASSERT_EQ(ENOENT, fd.getError());
    }

    TmpFile	tmpf("test_fopen_cloexec");
    ASSERT_EQ(0, tmpf.createFile());
    const char	*path(tmpf.getPath());
    const off_t	fsize(0x1000);
    {
        int	fd(open(path, O_RDWR));
        ASSERT_NE(-1, fd) << "errno = " << strerror(errno);
        FdRef	fref(fd);
        off_t	off(fsize - 1);
        ASSERT_EQ(off, lseek(fd, off, SEEK_SET));

        uint8_t	data(0);
        ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                  write(fd, &data, sizeof(data)));
    }

    // Read-only mode.
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "r"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDONLY, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Read-write mode.
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "r+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        size_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<size_t>(0), off);
    }

    // Append mode.
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "a"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        off_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(fsize, off);
    }

    // Read and append mode.
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "a+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(O_APPEND, flags & O_APPEND);

        // Initial file offset must be zero.
        off_t	off(lseek(desc, 0, SEEK_CUR));
        ASSERT_EQ(static_cast<off_t>(0), off);
    }

    // Write only mode with file truncation.
    {
        struct stat	sbuf;
        ASSERT_EQ(0, stat(path, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);

        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "w"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_WRONLY, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        // Target file must be truncated.
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);
    }

    // Read and write mode with file truncation.
    {
        struct stat	sbuf;
        ASSERT_EQ(0, truncate(path, fsize));
        ASSERT_EQ(0, stat(path, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);

        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "w+"));

        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        // Target file must be truncated.
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(static_cast<off_t>(0), sbuf.st_size);
    }

    // Create a new file.
    ASSERT_EQ(0, unlink(path));
    {
        OpenCloExec	fd(PFC_FOPEN_CLOEXEC(path, "w+"));
        
        int	desc(fd.getDescriptor());
        ASSERT_NE(-1, desc);
        ASSERT_EQ(0, fd.getError());

        int	flags(fcntl(desc, F_GETFD));
        ASSERT_NE(-1, flags);
        ASSERT_EQ(FD_CLOEXEC, (flags & FD_CLOEXEC));
        flags = fcntl(desc, F_GETFL);
        ASSERT_EQ(O_RDWR, flags & O_ACCMODE);
        ASSERT_EQ(0, flags & O_APPEND);

        const off_t	fsize(0x10);
        ASSERT_EQ(0, ftruncate(desc, fsize));

        struct stat	sbuf;
        ASSERT_EQ(0, fstat(desc, &sbuf));
        ASSERT_EQ(fsize, sbuf.st_size);
        ASSERT_EQ(static_cast<mode_t>(0666), sbuf.st_mode & 07777);
    }
}

/*
 * Test case for pfc_dupfd_cloexec().
 */
TEST(cloexec, dupfd)
{
    // Invalid FD.
    ASSERT_EQ(-1, pfc_dupfd_cloexec(-1));
    ASSERT_EQ(errno, EBADF);

    int	fd(open(".", O_RDONLY));
    ASSERT_NE(-1, fd);
    ASSERT_EQ(0, close(fd));
    ASSERT_EQ(-1, pfc_dupfd_cloexec(fd));
    ASSERT_EQ(errno, EBADF);

    // Duplicate FD.
    fd = open(".", O_RDONLY);
    ASSERT_NE(-1, fd);
    ASSERT_EQ(0, get_cloexec(fd));
    FdRef	fref(fd);

    {
        int	newfd(pfc_dupfd_cloexec(fd));
        FdRef	nfref(newfd);
        ASSERT_NE(-1, newfd);

        ASSERT_EQ(FD_CLOEXEC, get_cloexec(newfd));
        fd_equals(fd, newfd);
        if (HasFatalFailure()) {
            return;
        }
    }

    // Ensure that the lowest numbered FD is used.
    for (int oldfd(0); oldfd <= 2; oldfd++) {
        CloseFd	cfd(oldfd);
        int	newfd(pfc_dupfd_cloexec(fd));
        ASSERT_EQ(oldfd, newfd);
        ASSERT_EQ(FD_CLOEXEC, get_cloexec(newfd));
        fd_equals(fd, newfd);
        if (HasFatalFailure()) {
            return;
        }
    }
}

/*
 * Test case for pfc_dup2fd_cloexec().
 */
TEST(cloexec, dup2fd)
{
    // Invalid FD.
    ASSERT_EQ(-1, pfc_dup2fd_cloexec(-1, 10));
    ASSERT_EQ(errno, EBADF);

    {
        int	fd(open(".", O_RDONLY));
        ASSERT_NE(-1, fd);
        int	fd1(fd);
        FdRef	fref(&fd1);

        ASSERT_EQ(-1, pfc_dup2fd_cloexec(fd, -1));
        ASSERT_EQ(errno, EBADF);

        ASSERT_EQ(0, close(fd));
        fd1 = -1;
        ASSERT_EQ(-1, pfc_dup2fd_cloexec(fd, 10));
        ASSERT_EQ(errno, EBADF);
    }

    // Reject identical FD.
    int	fd(open(".", O_RDONLY));
    ASSERT_NE(-1, fd);
    ASSERT_EQ(0, get_cloexec(fd));
    FdRef	fref(fd);
    ASSERT_EQ(-1, pfc_dup2fd_cloexec(fd, fd));
    ASSERT_EQ(EINVAL, errno);

    // Duplicate FD.
    for (int newfd(30); newfd <= 40; newfd++) {
        int	ret(pfc_dup2fd_cloexec(fd, newfd));
        FdRef	nfref(ret);
        ASSERT_EQ(newfd, ret);

        ASSERT_EQ(FD_CLOEXEC, get_cloexec(ret));
        fd_equals(fd, ret);
        if (HasFatalFailure()) {
            return;
        }
    }

    // Ensure that newfd is closed.
    int	newfd(open("/dev/null", O_RDONLY));
    FdRef	nfref(newfd);
    ASSERT_EQ(0, get_cloexec(newfd));
    int	ret(pfc_dup2fd_cloexec(fd, newfd));
    ASSERT_EQ(newfd, ret);
    ASSERT_EQ(FD_CLOEXEC, get_cloexec(ret));
    fd_equals(fd, ret);
}
