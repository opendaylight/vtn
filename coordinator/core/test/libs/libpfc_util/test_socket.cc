/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_socket.cc - Test for socket APIs.
 */

#include <gtest/gtest.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <pfc/socket.h>
#include "misc.hh"
#include "random.hh"

/*
 * Port number used for test.
 */
#define	TEST_PORT_STRING	"6001"

/*
 * Localhost.
 */
#define	TEST_HOST_LOCALHOST	"127.0.0.1"

/*
 * Unreachable IP address.
 */
#define	TEST_HOST_UNREACHABLE	"192.168.100.123"

/*
 * Class which represents a socket created by pfc_sock_open().
 */
class Socket
{
public:
    Socket(int domain, int type, int protocol, int flags)
    {
        _socket = pfc_sock_open(domain, type, protocol, flags);
        _error = (_socket == -1) ? errno : 0;
    }

    ~Socket()
    {
        if (_socket != -1) {
            (void)close(_socket);
        }
    }

    int
    getSocket(void) const
    {
        return _socket;
    }

    int
    getError(void) const
    {
        return _error;
    }

private:
    int		_socket;
    int		_error;
};

/*
 * Class which represents a pair of sockets created by pfc_sock_openpair().
 */
class SocketPair
{
public:
    SocketPair(int domain, int type, int protocol, int flags)
    {
        if (pfc_sock_openpair(domain, type, protocol, flags, _socket) == 0) {
            _error = 0;
        }
        else {
            _socket[0] = _socket[1] = -1;
            _error = errno;
        }
    }

    ~SocketPair()
    {
        if (_socket[0] != -1) {
            (void)close(_socket[0]);
            (void)close(_socket[1]);
        }
    }

    int
    getSocket0(void) const
    {
        return _socket[0];
    }

    int
    getSocket1(void) const
    {
        return _socket[1];
    }

    int
    getError(void) const
    {
        return _error;
    }

private:
    int		_socket[2];
    int		_error;
};

/*
 * Class for pfc_sock_accept() test.
 */
class AcceptTest
{
public:
    AcceptTest(int flags, bool testaddr)
        : _flags(flags), _error(0), _sock(-1), _testaddr(testaddr),
          _thread(PFC_PTHREAD_INVALID_ID)
    {
        std::ostringstream	stream;
        stream << "socket_test_port." << getpid();
        _path = stream.str();
        _saddr.sun_family = AF_UNIX;
        snprintf(_saddr.sun_path, sizeof(_saddr.sun_path), "%s",
                 _path.c_str());
    }

    ~AcceptTest()
    {
        (void)unlink(_path.c_str());
        if (_thread != PFC_PTHREAD_INVALID_ID) {
            (void)pthread_join(_thread, NULL);
        }
    }

    void	run(void);

    int
    getError(void) const
    {
        return _error;
    }

private:
    static void	*conn_thread(void *arg);

    int		_flags;
    int		_error;
    int		_sock;
    bool	_testaddr;
    pthread_t	_thread;
    std::string	_errmsg;
    std::string	_path;
    struct sockaddr_un	_saddr;
};

void
AcceptTest::run(void)
{
    Socket	server(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC);
    int	serv(server.getSocket());
    ASSERT_NE(-1, serv);

    ASSERT_EQ(0, bind(serv, (const struct sockaddr *)&_saddr, sizeof(_saddr)));
    ASSERT_EQ(0, listen(serv, 1));

    ASSERT_EQ(0, pthread_create(&_thread, NULL, conn_thread, this));

    int	sock(-1);
    FdRef	fdref(&sock);
    if (_testaddr) {
        struct sockaddr_un	caddr;
        socklen_t	slen(sizeof(caddr));

        sock = pfc_sock_accept(serv, (struct sockaddr *)&caddr, &slen, _flags);
        if (sock == -1) {
            _error = errno;

            return;
        }

        ASSERT_EQ(static_cast<socklen_t>(sizeof(caddr.sun_family)), slen);
        ASSERT_EQ(AF_UNIX, caddr.sun_family);
    }
    else {
        sock = pfc_sock_accept(serv, NULL, NULL, _flags);
        if (sock == -1) {
            _error = errno;

            return;
        }
    }

    ASSERT_EQ(0, pthread_join(_thread, NULL));
    _thread = PFC_PTHREAD_INVALID_ID;
    if (_errmsg.length() != 0) {
        FAIL() << _errmsg;
    }

    int	reqfd((_flags & PFC_SOCK_CLOEXEC) ? FD_CLOEXEC : 0);
    int	reqfl((_flags & PFC_SOCK_NONBLOCK) ? O_NONBLOCK : 0);

    // Test close-on-exec flag.
    int	f(fcntl(sock, F_GETFD));
    ASSERT_NE(-1, f);
    ASSERT_EQ(reqfd, f & FD_CLOEXEC);

    // Test non-blocking flag.
    f = fcntl(sock, F_GETFL);
    ASSERT_NE(-1, f);
    ASSERT_EQ(reqfl, f & O_NONBLOCK);

    ASSERT_EQ(0, close(sock)) << "close() failed: " << strerror(errno);
    sock = -1;
}

void *
AcceptTest::conn_thread(void *arg)
{
    AcceptTest	*test(reinterpret_cast<AcceptTest *>(arg));
    int	sock(-1);

    try {
        sock = pfc_sock_open(AF_UNIX, SOCK_STREAM, 0, PFC_SOCK_CLOEXEC);
        if (sock == -1) {
            THROW_ERRNO("pfc_sock_open() failed", errno);
        }

        if (connect(sock, (const struct sockaddr *)&test->_saddr,
                    sizeof(test->_saddr)) != 0) {
            THROW_ERRNO("connect() failed", errno);
        }
    }
    catch (const std::runtime_error &e) {
        test->_errmsg = e.what();
    }

    if (sock != -1) {
        (void)close(sock);
    }

    return NULL;
}

/*
 * Destructor of struct addrinfo.
 */
class AddrInfo
{
public:
    AddrInfo(struct addrinfo *aip=NULL) : _addrinfo(aip) {}
    ~AddrInfo()
    {
        if (_addrinfo != NULL) {
            freeaddrinfo(_addrinfo);
        }
    }

    inline AddrInfo &
    operator=(struct addrinfo *aip)
    {
        _addrinfo = aip;

        return *this;
    }

    inline struct addrinfo *
    operator*() const
    {
        return _addrinfo;
    }

private:
    struct addrinfo	*_addrinfo;
};

/*
 * pipe() destructor.
 */
class Pipe
{
public:
    Pipe()
    {
        for (int i = 0; i < 2; i++) {
            _pipe[i] = -1;
        }
    }


    ~Pipe()
    {
        for (int i = 0; i < 2; i++) {
            int	fd = _pipe[i];

            if (fd != -1) {
                (void)close(fd);
            }
        }
    }

    inline Pipe &
    operator=(int fds[2])
    {
        for (int i = 0; i < 2; i++) {
            _pipe[i] = fds[i];
        }

        return *this;
    }

    inline int
    forRead(void) const
    {
        return _pipe[0];
    }

    inline void
    shutdown(void)
    {
        int	fd(_pipe[1]);

        _pipe[1] = -1;
        (void)close(fd);
    }

private:
    int	_pipe[2];
};

/*
 * Server thread used by pfc_sock_connect() test.
 */
class ServerThread
{
public:
    ServerThread(const char *port, int canceller, uint32_t timeout)
        : _error(0), _serv(-1), _canceller(canceller), _port(port),
          _timeout(timeout), _thread(PFC_PTHREAD_INVALID_ID),
          _addrinfo(NULL) {}

    ~ServerThread()
    {
        if (_serv != -1) {
            (void)close(_serv);
        }
        if (_thread != PFC_PTHREAD_INVALID_ID) {
            (void)pthread_join(_thread, NULL);
        }
    }

    void	run();

private:
    static void	*server_thread(void *arg);

    int			_error;
    int			_serv;
    int			_canceller;
    const char		*_port;
    uint32_t		_timeout;
    pthread_t		_thread;
    AddrInfo		_addrinfo;

    std::string	_errmsg;
};

void
ServerThread::run(void)
{
    // Create listener socket.
    struct addrinfo	hints, *result;

    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    int	gerr(getaddrinfo(TEST_HOST_LOCALHOST, TEST_PORT_STRING, &hints,
                         &result));
    ASSERT_EQ(0, gerr) << "getaddrinfo() error: " << gai_strerror(gerr);
    _addrinfo = result;

    int	sock(pfc_sock_open(result->ai_family, result->ai_socktype,
                           result->ai_protocol, PFC_SOCK_CLOEXEC_NB));
    ASSERT_NE(-1, sock) << "socket() failed: " << strerror(errno);
    _serv = sock;

    int	sflag(1);
    ASSERT_EQ(0, setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sflag,
                            sizeof(sflag)))
        << "setsockopt() failed: " << strerror(errno);
    ASSERT_EQ(0, bind(sock, result->ai_addr, result->ai_addrlen))
        << "bind() failed: " << strerror(errno);
    ASSERT_EQ(0, listen(sock, 1)) << "listen() failed: " << strerror(errno);

    ASSERT_EQ(0, pthread_create(&_thread, NULL, server_thread, this));
}

void *
ServerThread::server_thread(void *arg)
{
    ServerThread	*self(reinterpret_cast<ServerThread *>(arg));

    struct pollfd	pfd[2];
    int	serv(self->_serv);

    pfd[0].fd = serv;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    pfd[1].fd = self->_canceller;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    int	sock(-1);

    try {
        int	n(poll(pfd, 2, self->_timeout * 1000));

        if (n < 0) {
            THROW_ERRNO("poll() failed", errno);
        }
        if (n == 0) {
            THROW_MESSAGE("No connection was detected.");
        }
        if (pfd[1].revents != 0) {
            // Cancelled.
            return NULL;
        }

	sock = accept(serv, NULL, NULL);
        if (sock == -1) {
            THROW_ERRNO("accept() failed", errno);
        }

        uint8_t	data;
        if (read(sock, &data, sizeof(data)) != (ssize_t)sizeof(data)) {
            THROW_ERRNO("read() failed", errno);
        }
        if (write(sock, &data, sizeof(data)) != (ssize_t)sizeof(data)) {
            THROW_ERRNO("write() failed", errno);
        }
    }
    catch (const std::runtime_error &e) {
        self->_errmsg = e.what();
    }

    if (sock != -1) {
        (void)close(sock);
    }

    return NULL;
}

/*
 * Signal received flag.
 */
static volatile sig_atomic_t	signal_received;

/*
 * Connect test context.
 */
class ConnectTest
{
public:
    typedef boost::function<void (void)>	delayed_job_t;

    ConnectTest(const char *server, const char *port)
        : _sock(-1), _server(server), _port(port), _signum(0), _hasmask(false),
          _interrupt(PFC_FALSE), _thread(PFC_PTHREAD_INVALID_ID),
          _caller(pthread_self())
    {
        signal_received = 0;
    }

    ~ConnectTest()
    {
        if (_sock != -1) {
            (void)close(_sock);
        }
        if (_signum != 0) {
            (void)sigaction(_signum, &_oldsig, NULL);
        }
        if (_hasmask) {
            (void)pthread_sigmask(SIG_SETMASK, &_oldmask, NULL);
        }
        if (_thread != PFC_PTHREAD_INVALID_ID) {
            (void)pthread_join(_thread, NULL);
        }
    }

    void	connect(int reqerror,
                        const pfc_timespec_t *PFC_RESTRICT timeout,
                        const pfc_iowait_t *PFC_RESTRICT iowait);
    void	connect_c(int reqerror,
                          const pfc_timespec_t *PFC_RESTRICT timeout,
                          const pfc_iowait_t *PFC_RESTRICT iowait);
    void	connect_abs(int reqerror,
                            const pfc_timespec_t *PFC_RESTRICT abstime,
                            const pfc_iowait_t *PFC_RESTRICT iowait);
    void	connect_abs_c(int reqerror,
                              const pfc_timespec_t *PFC_RESTRICT abstime,
                              const pfc_iowait_t *PFC_RESTRICT iowait);

    void	cancel(void);
    void	interrupt(void);
    void	postDelayed(delayed_job_t &job);
    void	setSignalHandler(int sig);
    int		setAbsTimeout(pfc_timespec_t &abstime, uint32_t timeout);

    inline int
    getCanceller(void) const
    {
        return _canceller.forRead();
    }

    inline void
    setInterrupt(pfc_bool_t intr)
    {
        _interrupt = intr;
    }

    static pfc_bool_t	interrupt_handler(pfc_ptr_t arg);

private:
    void	setup(void);
    void	verify(void);
    static void	*delayed_worker(void *arg);
    static void	signal_handler(int sig);

    int			_sock;
    const char		*_server;
    const char		*_port;
    int			_signum;
    bool		_hasmask;
    pfc_bool_t		_interrupt;
    pthread_t		_thread;
    pthread_t		_caller;
    struct sigaction	_oldsig;
    sigset_t		_oldmask;
    Pipe		_canceller;
    AddrInfo		_addrinfo;
    RandomGenerator	_rand;
    delayed_job_t	_job;
};

void
ConnectTest::setup(void)
{
    int	fds[2];
    ASSERT_EQ(0, pipe(fds));
    _canceller = fds;

    struct addrinfo	hints, *result;

    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    int	gerr(getaddrinfo(_server, _port, &hints, &result));
    ASSERT_EQ(0, gerr) << "getaddrinfo() error: " << gai_strerror(gerr);
    _addrinfo = result;

    int	sock(pfc_sock_open(result->ai_family, result->ai_socktype,
                           result->ai_protocol, PFC_SOCK_CLOEXEC));
    ASSERT_NE(-1, sock) << "pfc_sock_open() failed: " << strerror(errno);
    _sock = sock;
}

void
ConnectTest::verify(void)
{
    uint8_t	data, received;
    RANDOM_INTEGER(_rand, data);

    int	sock(_sock);
    ASSERT_EQ((ssize_t)sizeof(data), write(sock, &data, sizeof(data)));
    ASSERT_EQ((ssize_t)sizeof(data), read(sock, &received, sizeof(received)));
    ASSERT_EQ(data, received);
}

void
ConnectTest::connect(int reqerror, const pfc_timespec_t *PFC_RESTRICT timeout,
                     const pfc_iowait_t *PFC_RESTRICT iowait)
{
    setup();
    if (::testing::Test::HasFatalFailure()) {
        return;
    }

    struct addrinfo	*aip(*_addrinfo);
    int	err(pfc_sock_connect(_sock, aip->ai_addr, aip->ai_addrlen,
                             timeout, iowait));
    if (err != 0) {
        _sock = -1;
    }
    ASSERT_EQ(reqerror, err);
    if (err != 0) {
        return;
    }

    verify();
}

void
ConnectTest::connect_c(int reqerror,
                       const pfc_timespec_t *PFC_RESTRICT timeout,
                       const pfc_iowait_t *PFC_RESTRICT iowait)
{
    setup();
    if (::testing::Test::HasFatalFailure()) {
        return;
    }

    struct addrinfo	*aip(*_addrinfo);
    int	err(pfc_sock_connect_c(_sock, aip->ai_addr, aip->ai_addrlen,
                               _canceller.forRead(), timeout, iowait));
    if (err != 0) {
        _sock = -1;
    }
    ASSERT_EQ(reqerror, err);
    if (err != 0) {
        return;
    }

    verify();
}

void
ConnectTest::connect_abs(int reqerror,
                         const pfc_timespec_t *PFC_RESTRICT abstime,
                         const pfc_iowait_t *PFC_RESTRICT iowait)
{
    setup();
    if (::testing::Test::HasFatalFailure()) {
        return;
    }

    struct addrinfo	*aip(*_addrinfo);
    int	err(pfc_sock_connect_abs(_sock, aip->ai_addr, aip->ai_addrlen,
                                 abstime, iowait));
    if (err != 0) {
        _sock = -1;
    }
    ASSERT_EQ(reqerror, err);
    if (err != 0) {
        return;
    }

    verify();
}

void
ConnectTest::connect_abs_c(int reqerror,
                           const pfc_timespec_t *PFC_RESTRICT abstime,
                           const pfc_iowait_t *PFC_RESTRICT iowait)
{
    setup();
    if (::testing::Test::HasFatalFailure()) {
        return;
    }

    struct addrinfo	*aip(*_addrinfo);
    int	err(pfc_sock_connect_abs_c(_sock, aip->ai_addr, aip->ai_addrlen,
                                   _canceller.forRead(), abstime, iowait));
    if (err != 0) {
        _sock = -1;
    }
    ASSERT_EQ(reqerror, err);
    if (err != 0) {
        return;
    }

    verify();
}

void
ConnectTest::cancel(void)
{
    _canceller.shutdown();
}

void
ConnectTest::interrupt(void)
{
    (void)pthread_kill(_caller, _signum);
}

void
ConnectTest::postDelayed(delayed_job_t &job)
{
    ASSERT_EQ(0, pthread_create(&_thread, NULL, delayed_worker, &job));
}

void
ConnectTest::setSignalHandler(int sig)
{
    sigset_t	mask;
    sigfillset(&mask);
    ASSERT_EQ(0, pthread_sigmask(SIG_BLOCK, &mask, &_oldmask));
    _hasmask = true;

    struct sigaction	sact;

    sact.sa_handler = signal_handler;
    sact.sa_flags = 0;
    sigfillset(&sact.sa_mask);

    ASSERT_EQ(0, sigaction(sig, &sact, &_oldsig))
        << "sigaction() failed: " << strerror(errno);
    _signum = sig;
}

int
ConnectTest::setAbsTimeout(pfc_timespec_t &abstime, uint32_t msec)
{
    pfc_timespec_t  to;
    pfc_clock_msec2time(&to, msec);

    return pfc_clock_abstime(&abstime, &to);
}

pfc_bool_t
ConnectTest::interrupt_handler(pfc_ptr_t arg)
{
    ConnectTest	*test(reinterpret_cast<ConnectTest *>(arg));

    return test->_interrupt;
}

void *
ConnectTest::delayed_worker(void *arg)
{
    delayed_job_t	*job(reinterpret_cast<delayed_job_t *>(arg));

    struct timespec  ts = {
        0,
        10 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
    };
    nanosleep(&ts, NULL);
    (*job)();

    return NULL;
}

void
ConnectTest::signal_handler(int sig)
{
    signal_received = 1;
}

/*
 * Below are test cases.
 */

/*
 * Test case for pfc_sock_open().
 */
TEST(socket, open)
{
    // Invalid address family.
    {
        Socket	sock(12345678, SOCK_STREAM, 0, 0);

        ASSERT_EQ(-1, sock.getSocket());
        ASSERT_EQ(EAFNOSUPPORT, sock.getError());
    }

    // Invalid socket type.
    {
        Socket	sock(AF_INET, 12345678, 0, 0);

        ASSERT_EQ(-1, sock.getSocket());
        ASSERT_EQ(EINVAL, sock.getError());
    }

    // Invalid protocol.
    {
        Socket	sock(AF_UNIX, SOCK_DGRAM, 12345678, 0);

        ASSERT_EQ(-1, sock.getSocket());
        int err(sock.getError());
        if (err != EINVAL) {
            ASSERT_EQ(EPROTONOSUPPORT, err);
        }
    }
    {
        Socket	sock(AF_INET, SOCK_DGRAM, 0x100000, 0);

        ASSERT_EQ(-1, sock.getSocket());
        int err(sock.getError());
        if (err != EINVAL) {
            ASSERT_EQ(EPROTONOSUPPORT, err);
        }
    }

    // Invalid flag.
    for (int flags = 1; flags != 0; flags <<= 1) {
        if (flags & PFC_SOCK_CLOEXEC_NB) {
            continue;
        }

        Socket	sock(AF_UNIX, SOCK_DGRAM, 0, flags);
        ASSERT_EQ(-1, sock.getSocket());
        ASSERT_EQ(EINVAL, sock.getError());
    }

    // Close-on-exec and non-blocking flag test.
    int	domains[] = {AF_INET, AF_UNIX};
    int types[] = {SOCK_STREAM, SOCK_DGRAM};
    int	test_flags[] = {
        0, PFC_SOCK_CLOEXEC, PFC_SOCK_NONBLOCK, PFC_SOCK_CLOEXEC_NB,
    };

    for (int *dmp(domains); dmp < PFC_ARRAY_LIMIT(domains); dmp++) {
        for (int *tp(types); tp < PFC_ARRAY_LIMIT(types); tp++) {
            for (int *fp(test_flags); fp < PFC_ARRAY_LIMIT(test_flags); fp++) {
                int	flags(*fp);
                Socket	sock(*dmp, *tp, 0, flags);
                int	fd(sock.getSocket());

                ASSERT_NE(-1, fd);
                ASSERT_EQ(0, sock.getError());

                int	reqfd((flags & PFC_SOCK_CLOEXEC) ? FD_CLOEXEC : 0);
                int	reqfl((flags & PFC_SOCK_NONBLOCK) ? O_NONBLOCK : 0);

                // Test close-on-exec flag.
                int	f(fcntl(fd, F_GETFD));
                ASSERT_NE(-1, f);
                ASSERT_EQ(reqfd, f & FD_CLOEXEC);

                // Test non-blocking flag.
                f = fcntl(fd, F_GETFL);
                ASSERT_NE(-1, f);
                ASSERT_EQ(reqfl, f & O_NONBLOCK);
            }
        }
    }
}

/*
 * Test case for pfc_sock_openpair().
 */
TEST(socket, openpair)
{
    // Invalid address family.
    {
        SocketPair	sock(12345678, SOCK_STREAM, 0, 0);

        ASSERT_EQ(-1, sock.getSocket0());
        ASSERT_EQ(-1, sock.getSocket1());
        ASSERT_EQ(EAFNOSUPPORT, sock.getError());
    }

    // Invalid socket type.
    {
        SocketPair	sock(AF_UNIX, 12345678, 0, 0);

        ASSERT_EQ(-1, sock.getSocket0());
        ASSERT_EQ(-1, sock.getSocket1());
        ASSERT_EQ(EINVAL, sock.getError());
    }

    // Invalid protocol.
    {
        SocketPair	sock(AF_UNIX, SOCK_DGRAM, 12345678, 0);

        ASSERT_EQ(-1, sock.getSocket0());
        ASSERT_EQ(-1, sock.getSocket1());
        int err(sock.getError());
        if (err != EINVAL) {
            ASSERT_EQ(EPROTONOSUPPORT, err);
        }
    }
    {
        SocketPair	sock(AF_UNIX, SOCK_DGRAM, 0x100000, 0);

        ASSERT_EQ(-1, sock.getSocket0());
        ASSERT_EQ(-1, sock.getSocket1());
        int err(sock.getError());
        if (err != EINVAL) {
            ASSERT_EQ(EPROTONOSUPPORT, err);
        }
    }

    // Invalid flag.
    for (int flags = 1; flags != 0; flags <<= 1) {
        if (flags & PFC_SOCK_CLOEXEC_NB) {
            continue;
        }

        SocketPair	sock(AF_UNIX, SOCK_DGRAM, 0, flags);
        ASSERT_EQ(-1, sock.getSocket0());
        ASSERT_EQ(-1, sock.getSocket1());
        ASSERT_EQ(EINVAL, sock.getError());
    }

    // Close-on-exec and non-blocking flag test.
    int types[] = {SOCK_STREAM, SOCK_DGRAM};
    int	test_flags[] = {
        0, PFC_SOCK_CLOEXEC, PFC_SOCK_NONBLOCK, PFC_SOCK_CLOEXEC_NB,
    };

    uint8_t	dummy = 0;
    for (int *tp(types); tp < PFC_ARRAY_LIMIT(types); tp++) {
        for (int *fp(test_flags); fp < PFC_ARRAY_LIMIT(test_flags); fp++) {
            int		flags(*fp);
            SocketPair	sock(AF_UNIX, *tp, 0, flags);
            int		fd0(sock.getSocket0());
            int		fd1(sock.getSocket1());

            ASSERT_NE(-1, fd0);
            ASSERT_NE(-1, fd1);
            ASSERT_EQ(0, sock.getError());

            int	reqfd((flags & PFC_SOCK_CLOEXEC) ? FD_CLOEXEC : 0);
            int	reqfl((flags & PFC_SOCK_NONBLOCK) ? O_NONBLOCK : 0);

            // Test close-on-exec flag.
            int	f(fcntl(fd0, F_GETFD));
            ASSERT_NE(-1, f);
            ASSERT_EQ(reqfd, f & FD_CLOEXEC);

            f = fcntl(fd1, F_GETFD);
            ASSERT_NE(-1, f);
            ASSERT_EQ(reqfd, f & FD_CLOEXEC);

            // Test non-blocking flag.
            f = fcntl(fd0, F_GETFL);
            ASSERT_NE(-1, f);
            ASSERT_EQ(reqfl, f & O_NONBLOCK);

            f = fcntl(fd1, F_GETFL);
            ASSERT_NE(-1, f);
            ASSERT_EQ(reqfl, f & O_NONBLOCK);

            // Ensure that sockets are connected.
            uint8_t	c;
            dummy++;
            ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                      write(fd0, &dummy, sizeof(dummy)));
            ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                      read(fd1, &c, sizeof(c)));
            ASSERT_EQ(dummy, c);

            dummy++;
            ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                      write(fd1, &dummy, sizeof(dummy)));
            ASSERT_EQ(static_cast<ssize_t>(sizeof(uint8_t)),
                      read(fd0, &c, sizeof(c)));
            ASSERT_EQ(dummy, c);
        }
    }
}

/*
 * Test case for pfc_sock_accept().
 */
TEST(socket, accept)
{
    // Invalid socket.
    ASSERT_EQ(-1, pfc_sock_accept(-1, NULL, NULL, 0));
    ASSERT_EQ(EBADF, errno);

    // Invalid flag.
    for (int flags = 1; flags != 0; flags <<= 1) {
        if (flags & PFC_SOCK_CLOEXEC_NB) {
            continue;
        }

        AcceptTest	test(flags, false);
        test.run();
        if (HasFatalFailure()) {
            return;
        }
        ASSERT_EQ(EINVAL, test.getError());
    }    

    bool	booleans[] = {true, false};
    int	test_flags[] = {
        0, PFC_SOCK_CLOEXEC, PFC_SOCK_NONBLOCK, PFC_SOCK_CLOEXEC_NB,
    };

    // Test for close-on-exec, non-blocking flags.
    for (bool *bp(booleans); bp < PFC_ARRAY_LIMIT(booleans); bp++) {
        for (int *fp(test_flags); fp < PFC_ARRAY_LIMIT(test_flags); fp++) {
            int	flags(*fp);

            AcceptTest	test(flags, *bp);
            test.run();
            if (HasFatalFailure()) {
                return;
            }
            ASSERT_EQ(0, test.getError());
        }
    }
}

/*
 * Test case for pfc_sock_connect().
 */
TEST(socket, connect)
{
    ConnectTest	test(TEST_HOST_LOCALHOST, TEST_PORT_STRING);
    ServerThread	server(TEST_PORT_STRING, test.getCanceller(), 10);

    server.run();
    if (HasFatalFailure()) {
        return;
    }

    test.connect(0, NULL, NULL);
}

/*
 * Test case for pfc_sock_connect().
 * - ETIMEDOUT test.
 */
TEST(socket, DISABLED_connect_timeout)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    test.connect(ETIMEDOUT, &timeout, NULL);
}

/*
 * Test case for pfc_sock_connect().
 * - Ingore EINTR because no intrfunc is specified.
 */
TEST(socket, DISABLED_connect_intr1)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = NULL;
    iowait.iw_intrarg = NULL;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect().
 * - Ingore EINTR because intrfunc returned PFC_FALSE.
 */
TEST(socket, DISABLED_connect_intr2)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect().
 * - Return EINTR because intrfunc returned PFC_TRUE.
 */
TEST(socket, DISABLED_connect_intr3)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;
    test.setInterrupt(PFC_TRUE);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect(EINTR, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_c().
 */
TEST(socket, connect_c)
{
    ConnectTest	test(TEST_HOST_LOCALHOST, TEST_PORT_STRING);
    ServerThread	server(TEST_PORT_STRING, test.getCanceller(), 10);

    server.run();
    if (HasFatalFailure()) {
        return;
    }

    test.connect_c(0, NULL, NULL);
}

/*
 * Test case for pfc_sock_connect_c().
 * - ETIMEDOUT test.
 */
TEST(socket, DISABLED_connect_c_timeout)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    test.connect_c(ETIMEDOUT, &timeout, NULL);
}

/*
 * Test case for pfc_sock_connect_c().
 * - Ingore EINTR because no intrfunc is specified.
 */
TEST(socket, DISABLED_connect_c_intr1)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = NULL;
    iowait.iw_intrarg = NULL;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_c(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_c().
 * - Ingore EINTR because intrfunc returned PFC_FALSE.
 */
TEST(socket, DISABLED_connect_c_intr2)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_c(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_c().
 * - Return EINTR because intrfunc returned PFC_TRUE.
 */
TEST(socket, DISABLED_connect_c_intr3)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;
    test.setInterrupt(PFC_TRUE);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_c(EINTR, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_c().
 * - ECANCELED test.
 */
TEST(socket, DISABLED_connect_c_cancel)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::cancel, &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_c(ECANCELED, &timeout, NULL);
}

/*
 * Test case for pfc_sock_connect_abs().
 */
TEST(socket, connect_abs)
{
    ConnectTest	test(TEST_HOST_LOCALHOST, TEST_PORT_STRING);
    ServerThread	server(TEST_PORT_STRING, test.getCanceller(), 10);

    server.run();
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs(0, NULL, NULL);
}

/*
 * Test case for pfc_sock_connect_abs().
 * - ETIMEDOUT test.
 */
TEST(socket, DISABLED_connect_abs_timeout)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    test.connect_abs(ETIMEDOUT, &timeout, NULL);
}

/*
 * Test case for pfc_sock_connect_abs().
 * - Ingore EINTR because no intrfunc is specified.
 */
TEST(socket, DISABLED_connect_abs_intr1)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = NULL;
    iowait.iw_intrarg = NULL;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abs().
 * - Ingore EINTR because intrfunc returned PFC_FALSE.
 */
TEST(socket, DISABLED_connect_abs_intr2)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abs().
 * - Return EINTR because intrfunc returned PFC_TRUE.
 */
TEST(socket, DISABLED_connect_abs_intr3)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;
    test.setInterrupt(PFC_TRUE);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect(EINTR, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abc_c().
 */
TEST(socket, connect_abs_c)
{
    ConnectTest	test(TEST_HOST_LOCALHOST, TEST_PORT_STRING);
    ServerThread	server(TEST_PORT_STRING, test.getCanceller(), 10);

    server.run();
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs_c(0, NULL, NULL);
}

/*
 * Test case for pfc_sock_connect_abs_c().
 * - ETIMEDOUT test.
 */
TEST(socket, DISABLED_connect_abs_c_timeout)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    test.connect_abs_c(ETIMEDOUT, &timeout, NULL);
}

/*
 * Test case for pfc_sock_connect_abs_c().
 * - Ingore EINTR because no intrfunc is specified.
 */
TEST(socket, DISABLED_connect_abs_c_intr1)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = NULL;
    iowait.iw_intrarg = NULL;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs_c(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abs_c().
 * - Ingore EINTR because intrfunc returned PFC_FALSE.
 */
TEST(socket, DISABLED_connect_abs_c_intr2)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs_c(ETIMEDOUT, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abs_c().
 * - Return EINTR because intrfunc returned PFC_TRUE.
 */
TEST(socket, DISABLED_connect_abs_c_intr3)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    test.setSignalHandler(SIGUSR2);
    if (HasFatalFailure()) {
        return;
    }

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    pfc_iowait_t	iowait;
    sigset_t	mask;
    sigemptyset(&mask);
    iowait.iw_intrfunc = &ConnectTest::interrupt_handler;
    iowait.iw_intrarg = &test;
    iowait.iw_sigmask = &mask;
    test.setInterrupt(PFC_TRUE);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::interrupt,
                                                &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs_c(EINTR, &timeout, &iowait);
    ASSERT_EQ(1, signal_received);
}

/*
 * Test case for pfc_sock_connect_abs_c().
 * - ECANCELED test.
 */
TEST(socket, DISABLED_connect_abs_c_cancel)
{
    ConnectTest	test(TEST_HOST_UNREACHABLE, TEST_PORT_STRING);

    pfc_timespec_t	timeout;
    test.setAbsTimeout(timeout, 300);

    ConnectTest::delayed_job_t	job(boost::bind(&ConnectTest::cancel, &test));
    test.postDelayed(job);
    if (HasFatalFailure()) {
        return;
    }

    test.connect_abs_c(ECANCELED, &timeout, NULL);
}
