/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Additional Test
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pfc/socket.h>
#include <sys/un.h>
#include <pfc/path.h>
#include <pfc/iostream.h>
#include <pfc/thread.h>
#include <pfc/strtoint.h>
#include <gtest/gtest.h>
#include "cmdutil.h"
#include "cmd_impl.h"
#include "ctrl_client.h"
#include "conf_impl.h"
#include "ctrl_proto.h"
#include "iostream_impl.h"
#include "tmpfile.hh"
#include "misc.hh"
#include "random.hh"
#include "signal_subr.hh"
#include "test.h"

typedef struct{
	const char *test_path;
	int        ex_val;
} safe_path_test_data;

#define	TOO_LONG_NAME	"/123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890122345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

/*
 * Size of UNIX domain socket path.
 */
#define	UNIX_SOCKET_PATH_SIZE	(sizeof(((struct sockaddr_un *)0)->sun_path))

/* 
 * UNIX_SOCKET_PATH_SIZE - (PFC_PFCD_CHANNEL_NAMELEN
 *  + PFC_PFCD_CTRL_NAMELEN + 3)
 *    = 108 - (7 + 4 + 3)
 *    = 94
 */
#define	WORK_DIR_95	"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345"
#define	WORK_DIR_96	"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"
#define	TEST_MISC_BUFSIZE_IN		64U
#define	TEST_MISC_BUFSIZE_OUT		256U
#define	TEST_MISC_CTRL_NAME		"ctrl1"
#define	TEST_MISC_CTRL_NAMELEN		5
#define	TEST_MISC_TIMEOUT_SEC		0
#define	TEST_MISC_TIMEOUT_NSEC				\
	(300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC))
#define	TEST_MISC_CMD_COUNT		7

#define	TEST_IOSTREAM_FILE_NLINES_MAX	256U
#define	TEST_IOSTREAM_FILE_MAXWIDTH	128U

extern "C" int	pfc_modcache_attach(const char *PFC_RESTRICT path,
				    uint8_t **PFC_RESTRICT addrp,
				    uint32_t *PFC_RESTRICT sizep);

/*
 * execution flag
 */
pfc_bool_t test_misc_callback_true_flag = PFC_FALSE;
pfc_bool_t test_misc_callback_false_flag = PFC_FALSE;
pfc_bool_t test_misc_sig_handler_flag = PFC_FALSE;

/*
 * Control socket path.
 */
static const struct timespec req = {2, 0};

pfc_bool_t err_flag = PFC_FALSE;
static pfc_mutex_t test_misc_dest_3_lock = PFC_MUTEX_INITIALIZER;
static pfc_mutex_t test_misc_read_6_lock = PFC_MUTEX_INITIALIZER;

/*
 * Base name of temporary file that keeps error on child.
 */
#define	CHILD_ERROR_FILE	OBJDIR "/tmp_test_misc"

/*
 * Assertions for child process.
 */
static void
childAssFail(TmpFile &tmp, const char *str)
{
	tmp.print("%s", str);
	tmp.flush();
	_exit(EXIT_FAILURE);
}

#define	CHILD_ASSERT_EQ(tmp, required, value)				\
	do {								\
		try {							\
			EXASSERT_EQ(required, value);			\
		}							\
		catch (const std::runtime_error &__e) {			\
			childAssFail(tmp, __e.what());			\
		}							\
	} while (0)

#define	CHILD_ASSERT_NE(tmp, required, value)				\
	do {								\
		try {							\
			EXASSERT_NE(required, value);			\
		}							\
		catch (const std::runtime_error &__e) {			\
			childAssFail(tmp, __e.what());			\
		}							\
	} while (0)

#define	CHILD_ASSERT_TRUE(tmp, value)					\
	do {								\
		try {							\
		     EXASSERT_TRUE(value);				\
		}							\
		catch (const std::runtime_error &__e) {			\
			childAssFail(tmp, __e.what());			\
		}							\
	} while (0)

#define	CHILD_ASSERT_FALSE(tmp, value)					\
	do {								\
		try {							\
		     EXASSERT_FALSE(value);				\
		}							\
		catch (const std::runtime_error &__e) {			\
			childAssFail(tmp, __e.what());			\
		}							\
	} while (0)

/*
 * Ensure that a child has quit without error.
 */
#define	VERIFY_CHILD(tmp)						\
	do {								\
		size_t	__sz;						\
									\
		ASSERT_EQ(0, (tmp).getSize(__sz));			\
		if (__sz != 0) {					\
			std::string	__err;				\
									\
			ASSERT_EQ(0, (tmp).readAsString(__err));	\
			FAIL() << "Error on child: " << __err;		\
		}							\
	} while (0)

/*
 * Temporary string.
 */
class TmpString
{
public:
	TmpString(const char *str) : _string(strdup(str)) {}
	~TmpString()
	{
		free(_string);
	}

	inline char *
	operator*() const
	{
		return _string;
	}

private:
	char	*_string;
};

#define	TMPSTR_ASSERT(tstr, ptr)				\
	do {							\
		(ptr) = *(tstr);				\
		ASSERT_TRUE((ptr) != NULL);			\
	} while (0)

/*
 * pfc_iostream_t instance.
 */
class IoStream
{
public:
    IoStream(pfc_iostream_t stream)
        : _stream(stream), _strptr(NULL) {}
    IoStream(pfc_iostream_t *streamp)
        : _stream(NULL), _strptr(streamp) {}

    ~IoStream()
    {
        if (_stream != NULL) {
            (void)pfc_iostream_destroy(_stream);
        }
        if (_strptr != NULL && *_strptr != NULL) {
            (void)pfc_iostream_destroy(*_strptr);
        }
    }

private:
    pfc_iostream_t	_stream;
    pfc_iostream_t	*_strptr;
};

/*
 * Temporary UNIX domain socket file.
 */
class SocketFile
    : public PathRef
{
public:
    SocketFile()
    {
        char  buf[64];

        snprintf(buf, sizeof(buf), "/tmp/test_misc.sock.%u", getpid());
        _path.assign(buf);

	(void)pfc_rmpath(buf);
    }
};

/*
 * Test Data
 *      pfc_is_safepath()
 */
static safe_path_test_data sptd[] = {
	{"invalid", EINVAL},
	{TOO_LONG_NAME, ENAMETOOLONG},
	{"/homehomehome", ENOENT},
	{"/etc/hosts/tmp", ENOTDIR}
};

static void
wait_child(SignalHandler &sig, pid_t pid)
{
	time_t	limit(time(NULL) + 10);
	struct timespec ts = {0, PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC};

	while (sig.getReceived() == 0) {
		nanosleep(&ts, NULL);
		ASSERT_EQ(0, waitpid(pid, NULL, WNOHANG));
		time_t cur(time(NULL));
		ASSERT_LE(cur, limit);
	}
}

static void
do_kill_and_wait(pid_t pid, int sig, int &status)
{
	time_t  limit(time(NULL) + 10);
	struct timespec ts = {
		0, 100 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
	};

	status = 0;
	for (;;) {
		nanosleep(&ts, NULL);
		ASSERT_EQ(0, kill(pid, sig));
		pid_t cpid(waitpid(pid, &status, WNOHANG));
		ASSERT_NE(-1, cpid) << "*** ERROR: " << strerror(errno);
		if (cpid != 0) {
			ASSERT_EQ(pid, cpid);
			break;
		}

		time_t cur(time(NULL));
		ASSERT_LE(cur, limit);
	}
}

static void
kill_and_wait(pid_t pid, int sig, int &status)
{
	do_kill_and_wait(pid, sig, status);
        if (::testing::Test::HasFailure()) {
		kill(pid, SIGKILL);
	}
}

/*
 * Test
 *      pfc_is_safepath()
 */
TEST(misc, pfc_is_safepath)
{
	int ret;
	char *cpath;

	for (uint32_t i = 0; i < PFC_ARRAY_CAPACITY(sptd); i++) {
		TmpString tmpstr(sptd[i].test_path);
		TMPSTR_ASSERT(tmpstr, cpath);
		ret = pfc_is_safepath(cpath);
		ASSERT_EQ(sptd[i].ex_val, ret);
	}
}

/*
 * Test
 *      pfc_flock_getowner()
 */
TEST(misc, pfc_flock_getowner)
{
	pfc_flock_t lk;
	const char *path;
	int ret, status;
	pid_t own = -1, fork_pid;
	FILE *fp;
	char buf[32];

	path = OBJDIR "/flock_getowner.tmp";
	ret = pfc_flock_open(&lk, path, (O_RDWR | O_CREAT),
			     (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
	ASSERT_EQ(0, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	ret = pfc_flock_wrlock(lk, &own);

	if (ret == EAGAIN) {
		/* pfcd running */
		ret = pfc_flock_getowner(lk, &own, PFC_TRUE);
		ASSERT_EQ(0, ret);

		fp = fopen(path, "r");
		ASSERT_NE((FILE *)NULL, fp);
		ASSERT_TRUE(NULL != fgets(buf, sizeof buf, fp));
		fclose(fp);
		ASSERT_EQ(atoi(buf), own);
		ret = pfc_flock_close(lk);
		ASSERT_EQ(0, ret);
	} else {
		fork_pid = fork();
		ASSERT_GE(fork_pid, 0);
		if (fork_pid == 0) {
			pid_t	parent_pid = getppid();

			/* Child */
			ret = pfc_flock_wrlock(lk, &own);
			CHILD_ASSERT_EQ(tmp, EAGAIN, ret);
			CHILD_ASSERT_EQ(tmp, parent_pid, own);

			ret = pfc_flock_getowner(lk, &own, PFC_TRUE);
			CHILD_ASSERT_EQ(tmp, 0, ret);
			CHILD_ASSERT_EQ(tmp, parent_pid, own);

			_exit(EXIT_SUCCESS);
		} else if (fork_pid > 0) {
			/* Parent */
			ret = pfc_flock_getowner(lk, &own, PFC_TRUE);
			ASSERT_EQ(0, ret);
			ASSERT_EQ(0, own);

			kill_and_wait(fork_pid, 0, status);
			RETURN_ON_ERROR();
			ASSERT_TRUE(WIFEXITED(status));
			ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
			VERIFY_CHILD(tmp);
			ret = pfc_flock_unlock(lk);
			ASSERT_EQ(0, ret);
			ret = pfc_flock_close(lk);
			ASSERT_EQ(0, ret);
			pidfile_unlink(path);
		}
	}
}


static void
test_errfunc(const char *fmt, va_list ap)
{
	ASSERT_FALSE(err_flag);
	err_flag = PFC_TRUE;
	ASSERT_TRUE(PFC_TRUE);
}

/*
 * Test
 *      pfc_ctrl_client_init()
 */
TEST(misc, pfc_ctrl_client_init)
{
	int ret;

	ret = pfc_ctrl_client_init(WORK_DIR_95, test_errfunc);
	ASSERT_EQ(0, ret);

	ret = pfc_ctrl_client_init(WORK_DIR_96, test_errfunc);
	ASSERT_EQ(ENAMETOOLONG, ret);
	ASSERT_TRUE(PFC_TRUE == err_flag);
}


/*
 * Callback (return PFC_TRUE)
 */
static pfc_bool_t
test_misc_callback_true(pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	test_misc_callback_true_flag = PFC_TRUE;
	return PFC_TRUE;
}

/*
 * Callback (return PFC_FALSE)
 */
static pfc_bool_t
test_misc_callback_false(pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	test_misc_callback_false_flag = PFC_TRUE;
	return PFC_FALSE;
}

/*
 * signal handler
 */
void
test_misc_sig_handler(int sig)
{
	test_misc_sig_handler_flag = PFC_TRUE;
}

/*
 * Test
 *      pfc_iostream_create()
 *      pfc_iostream_destroy()
 *      pfc_iostream_flush()
 *      pfc_iostream_read()
 *      pfc_iostream_write()
 */
TEST(misc, pfc_iostream_create_1)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pid_t fork_pid;
	int buf_write[] = {1,2,3};
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	int client_sockfd;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
				       sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_write);
		ret = pfc_iostream_write(stream, (void *)&buf_write, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		ASSERT_EQ(0, ret);

		si = sizeof(buf_read);
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		ret = pfc_iostream_read(stream, (void *)&buf_read,
					&si, &t_out);

		ASSERT_EQ(buf_write[0], buf_read[0]);
		ASSERT_EQ(buf_write[1], buf_read[1]);
		ASSERT_EQ(buf_write[2], buf_read[2]);

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);
	}
}

/*
 * Test
 *      pfc_iostream_create() 2
 */
TEST(misc, pfc_iostream_create_2)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pid_t fork_pid;
	int buf_write[] = {1,2,3};
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	int client_sockfd;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* input/output buffer size = 0 */
		ret = pfc_iostream_create(&stream, sock, 0, 0, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_write);
		ret = pfc_iostream_write(stream, (void *)&buf_write, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		/* input/output buffer size = 0 */
		ret = pfc_iostream_create(&stream, client_sockfd, 0, 0, NULL);
		ASSERT_EQ(0, ret);

		si = sizeof(buf_read);
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		ret = pfc_iostream_read(stream, (void *)&buf_read,
					&si, &t_out);

		ASSERT_EQ(buf_write[0], buf_read[0]);
		ASSERT_EQ(buf_write[1], buf_read[1]);
		ASSERT_EQ(buf_write[2], buf_read[2]);

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);
	}
}

/*
 * Test
 *      pfc_iostream_create() 3
 */
TEST(misc, pfc_iostream_create_3)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	int buf_read[] = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	SignalHandler  sigusr1(SIGUSR1);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		iowait.iw_sigmask = NULL;

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_read);
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		kill(getppid(), SIGUSR1);
		ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
					&t_out);

		/* call back check */
		CHILD_ASSERT_FALSE(tmp, test_misc_callback_true_flag);
		CHILD_ASSERT_EQ(tmp, ETIMEDOUT, ret);
		CHILD_ASSERT_TRUE(tmp, test_misc_sig_handler_flag);
		test_misc_sig_handler_flag = PFC_FALSE;

		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		kill_and_wait(fork_pid, SIGINT, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      pfc_iostream_create() 4
 */
TEST(misc, pfc_iostream_create_4)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	int buf_read[] = {0,0,0};
	size_t si;
	struct sockaddr_un un;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	SignalHandler  sigusr1(SIGUSR1);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = test_misc_callback_true;
		iowait.iw_intrarg = NULL;
		iowait.iw_sigmask = NULL;

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_read);

		kill(getppid(), SIGUSR1);
		ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
					NULL);

		/* call back check */
		CHILD_ASSERT_TRUE(tmp, test_misc_callback_true_flag);
		test_misc_callback_true_flag = PFC_FALSE;
		CHILD_ASSERT_TRUE(tmp, test_misc_sig_handler_flag);
		test_misc_sig_handler_flag = PFC_FALSE;
		CHILD_ASSERT_EQ(tmp, EINTR, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		kill_and_wait(fork_pid, SIGINT, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      pfc_iostream_create() 5
 */
TEST(misc, pfc_iostream_create_5)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	sigset_t mask;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	sigset_t  smask;
	sigemptyset(&smask);
	sigaddset(&smask, SIGINT);
	SignalHandler  sigusr1(SIGUSR1, &smask);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = test_misc_callback_true;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		iowait.iw_sigmask = &mask;

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_read);

		kill(getppid(), SIGUSR1);
		ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
					 NULL);

		/* NOTREACHED */
		CHILD_ASSERT_TRUE(tmp, PFC_FALSE);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		struct timespec ts = {
			0, 100 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
		};

		nanosleep(&ts, NULL);
		ASSERT_EQ(0, kill(fork_pid, SIGINT));

		kill_and_wait(fork_pid, SIGALRM, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFSIGNALED(status));
		ASSERT_EQ(SIGALRM, WTERMSIG(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test case that ensures pfc_iostream_create() can handle regular file.
 */
TEST(misc, pfc_iostream_create_file)
{
    uint32_t	nlines;
    RandomGenerator	rand;

    RANDOM_INTEGER_MAX(rand, nlines, TEST_IOSTREAM_FILE_NLINES_MAX);

    TmpFile	tmpf("iostream_file");
    ASSERT_EQ(0, tmpf.createFile());

    const char	*path(tmpf.getPath());
    int	fd(open(path, O_WRONLY));
    ASSERT_NE(-1, fd);
    FdRef	fref(&fd);

    pfc_iostream_t	stream;
    ASSERT_EQ(0, pfc_iostream_create(&stream, fd, 0, 0x400, NULL));
    fd = -1;
    IoStream	ios(&stream);

    std::string	required;
    for (uint32_t i(0); i < nlines; i++) {
        uint32_t	value;

        RANDOM_INTEGER(rand, value);

        char	buf[TEST_IOSTREAM_FILE_MAXWIDTH];
        int	len(snprintf(buf, sizeof(buf), "iostream test: 0x%08x\n",
                             value));
        ASSERT_TRUE(len > 0 && (size_t)len < sizeof(buf));
        required.append(buf, len);

        size_t	size(len);
        ASSERT_EQ(0, pfc_iostream_write(stream, (void *)buf, &size, NULL));
        ASSERT_EQ(static_cast<size_t>(len), size);
    }
    ASSERT_EQ(0, pfc_iostream_flush(stream, NULL));
    ASSERT_EQ(0, pfc_iostream_shutdown(stream, PFC_IOSTREAM_SHUT_WR));
    ASSERT_EQ(0, pfc_iostream_destroy(stream));
    stream = NULL;

    std::string	results;
    fd = open(path, O_RDONLY);
    ASSERT_NE(-1, fd);
    ASSERT_EQ(0, pfc_iostream_create(&stream, fd, 0x400, 0, NULL));
    fd = -1;
    for (;;) {
        char	buf[TEST_IOSTREAM_FILE_MAXWIDTH];
        size_t	size(sizeof(buf));
        ASSERT_EQ(0, pfc_iostream_read(stream, (void *)buf, &size, NULL));
        if (size == 0) {
            break;
        }
        results.append(buf, size);
    }
    ASSERT_EQ(0, pfc_iostream_shutdown(stream, PFC_IOSTREAM_SHUT_RD));
    ASSERT_EQ(0, pfc_iostream_destroy(stream));
    stream = NULL;

    ASSERT_STREQ(required.c_str(), results.c_str());
}

/*
 * Test case for pfc_iostream_create() error.
 */
TEST(misc, pfc_iostream_create_error)
{
    pfc_iostream_t stream;

    // Invalid file descriptor.
    ASSERT_EQ(EBADF, pfc_iostream_create(&stream, -1, 0, 0, NULL));

    int sock(socket(AF_UNIX, SOCK_STREAM, 0));
    FdRef  sref(&sock);
    ASSERT_NE(-1, sock);
    ASSERT_EQ(0, pfc_iostream_create(&stream, sock, 0, 0, NULL));
    ASSERT_EQ(0, pfc_iostream_destroy(stream));
    ASSERT_EQ(EBADF, pfc_iostream_create(&stream, sock, 0, 0, NULL));
    sock = -1;

    // Invalid socket type.
    static const int	bad_type[] = {SOCK_DGRAM, SOCK_RAW};
    for (const int *tp = bad_type; tp < PFC_ARRAY_LIMIT(bad_type); tp++) {
        int	type(*tp);

        sock = socket(AF_UNIX, type, 0);
        ASSERT_NE(-1, sock);

        ASSERT_EQ(ESOCKTNOSUPPORT,
                  pfc_iostream_create(&stream, sock, 0, 0, NULL));
        ASSERT_EQ(0, close(sock));
        sock = -1;
    }
}

/*
 * Test case for pfc_iostream_destroy().
 */
TEST(misc, pfc_iostream_destroy_1)
{
    int	sock(socket(AF_INET, SOCK_STREAM, 0));

    ASSERT_NE(-1, sock);
    FdRef	sref(&sock);

    pfc_iostream_t	stream;
    ASSERT_EQ(0, pfc_iostream_create(&stream, sock, 0, 0, NULL));

    // Ensure that pfc_iostream_destroy() closes the file descriptor.
    int	stype(0);
    socklen_t	slen(sizeof(stype));
    ASSERT_EQ(0, pfc_iostream_destroy(stream));
    ASSERT_EQ(-1, getsockopt(sock, SOL_SOCKET, SO_TYPE, &stype, &slen));
    ASSERT_EQ(EBADF, errno);

    // Ensure that pfc_iostream_destroy() never closes the file descriptor
    // if its value is -1.
    stype = 0;
    slen = sizeof(stype);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(-1, sock);
    ASSERT_EQ(0, pfc_iostream_create(&stream, sock, 0, 0, NULL));
    ASSERT_EQ(0, __pfc_iostream_setfd(stream, -1));
    ASSERT_EQ(0, pfc_iostream_destroy(stream));
    ASSERT_EQ(0, getsockopt(sock, SOL_SOCKET, SO_TYPE, &stype, &slen));
    ASSERT_EQ(SOCK_STREAM, stype);
    ASSERT_EQ(0, close(sock));

    // Ensure that __pfc_iostream_dispose() never closes the file descriptor
    // if its value is -1.
    stype = 0;
    slen = sizeof(stype);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_NE(-1, sock);
    ASSERT_EQ(0, pfc_iostream_create(&stream, sock, 0, 0, NULL));
    ASSERT_EQ(0, __pfc_iostream_setfd(stream, -1));
    __pfc_iostream_dispose(stream);
    ASSERT_EQ(0, getsockopt(sock, SOL_SOCKET, SO_TYPE, &stype, &slen));
    ASSERT_EQ(SOCK_STREAM, stype);
    ASSERT_EQ(0, close(sock));
    sock = -1;
}

/*
 * Test
 *      pfc_iostream_destroy() 2
 */
TEST(misc, pfc_iostream_destroy_2)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pid_t fork_pid;
	int buf_write[] = {1,2,3};
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	int client_sockfd;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_write);
		ret = pfc_iostream_write(stream, (void *)&buf_write, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		ASSERT_EQ(0, ret);

		si = sizeof(buf_read);
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		ret = pfc_iostream_read(stream, (void *)&buf_read,
					&si, &t_out);

		ASSERT_EQ(buf_write[0], buf_read[0]);
		ASSERT_EQ(buf_write[1], buf_read[1]);
		ASSERT_EQ(buf_write[2], buf_read[2]);

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

#if	0
		// This code causes undefined behavior.
		/* twice */
		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(EBADF, ret);
#endif	/* 0 */
	}
}

static void *
test_misc_destroy_3_func(void *arg)
{
	int ret = 0;
	pfc_iostream_t stream_thr = (pfc_iostream_t)arg;

	do {
		ret = pfc_mutex_trylock(&test_misc_dest_3_lock);
		if (ret == EBUSY) {
			break;
		} else if (ret == 0) {
			pfc_mutex_unlock(&test_misc_dest_3_lock);
		} else {
			EXPECT_TRUE(PFC_FALSE);
		}
	} while (1);

	struct timespec  ts = {
		TEST_MISC_TIMEOUT_SEC,
		TEST_MISC_TIMEOUT_NSEC,
	};
	nanosleep(&ts, NULL);
	ret = pfc_iostream_destroy(stream_thr);
	EXPECT_EQ(EBUSY, ret);

	return NULL;
}

/*
 * Test
 *      pfc_iostream_destroy() 3
 */
TEST(misc, pfc_iostream_destroy_3)
{
	int sock, ret;
	pfc_iostream_t stream;
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	pfc_timespec_t t_out;
	pfc_thread_t at_thr;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
				  TEST_MISC_BUFSIZE_OUT, NULL);
	ASSERT_EQ(0, ret);

	ret = pfc_thread_create(&at_thr, test_misc_destroy_3_func,
			        (void *)stream, 0);

	si = sizeof(buf_read);
	t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
	t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;
	pfc_timespec_t tmerr = {
		0,
		300 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
	};
	pfc_timespec_add(&t_out, &tmerr);

	ret = pfc_mutex_lock(&test_misc_dest_3_lock);
	ASSERT_EQ(0, ret);
	ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
				&t_out);

	/* time-out check */
	ASSERT_EQ(ETIMEDOUT, ret);

	ret = pfc_thread_join(at_thr, NULL);
	ASSERT_EQ(0, ret);

	ret = pfc_iostream_destroy(stream);
	ASSERT_EQ(0, ret);
}

/*
 * Test
 *      pfc_iostream_flush()
 */
TEST(misc, pfc_iostream_flush)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pid_t fork_pid;
	int buf_write[] = {1,2,3};
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	int client_sockfd;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(int);
		ret = pfc_iostream_write(stream, (void *)&buf_write[0], &si,
					 NULL);

		/* pfc_iostream_flush() is not used */

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		ASSERT_EQ(0, ret);

		si = sizeof(buf_read);
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		ret = pfc_iostream_read(stream, (void *)&buf_read,
					&si, &t_out);

		/* Child did not call pfc_iostream_flush() */
		ASSERT_NE(buf_write[0], buf_read[0]);
		ASSERT_NE(buf_write[1], buf_read[1]);
		ASSERT_NE(buf_write[2], buf_read[2]);

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);
	}
}

/*
 * Test
 *      pfc_iostream_read()
 */
TEST(misc, pfc_iostream_read_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	int buf_read[] = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	pfc_timespec_t t_out;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
				  TEST_MISC_BUFSIZE_OUT, NULL);
	ASSERT_EQ(0, ret);

	si = sizeof(buf_read);
	t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
	t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

	ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
				&t_out);

	/* time-out check */
	ASSERT_EQ(ETIMEDOUT, ret);

	ret = pfc_iostream_destroy(stream);
	ASSERT_EQ(0, ret);
}

/*
 * Test
 *      pfc_iostream_read() 2
 */
TEST(misc, pfc_iostream_read_2)
{
	int ret;
	size_t si;

	si = 0;

	ret = pfc_iostream_read(NULL, NULL, &si , NULL);
	ASSERT_EQ(0, ret);

}

/*
 * Test
 *      pfc_iostream_read() 3
 */
TEST(misc, pfc_iostream_read_3)
{
	int ret;
	size_t si;

	si = PFC_IOSTREAM_MAXSIZE + 1;

	ret = pfc_iostream_read(NULL, NULL, &si , NULL);
	ASSERT_EQ(E2BIG, ret);
}

/*
 * Test
 *      pfc_iostream_read() 4
 */
TEST(misc, pfc_iostream_read_4)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	int buf_read[] = {0,0,0};
	size_t si;
	struct sockaddr_un un;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	SignalHandler  sigusr1(SIGUSR1);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = test_misc_callback_false;
		iowait.iw_intrarg = NULL;
		iowait.iw_sigmask = NULL;

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		si = sizeof(buf_read);

		kill(getppid(), SIGUSR1);
		ret = pfc_iostream_read(stream, (void *)&buf_read, &si,
					 NULL);

		/* NOTREACHED */
		CHILD_ASSERT_TRUE(tmp, PFC_FALSE);
		_exit(EXIT_SUCCESS);
	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		struct timespec ts = {
			0, 100 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
		};
		nanosleep(&ts, NULL);
		ASSERT_EQ(0, kill(fork_pid, SIGINT));

		kill_and_wait(fork_pid, SIGALRM, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFSIGNALED(status));
		ASSERT_EQ(SIGALRM, WTERMSIG(status));
		VERIFY_CHILD(tmp);
	}
}

static void *
test_misc_read_5_func(void *arg)
{
	int ret = 0;
	int sock = *(int *)arg;

	do {
		ret = pfc_mutex_trylock(&test_misc_read_6_lock);
		if (ret == EBUSY) {
			break;
		} else if (ret == 0) {
			pfc_mutex_unlock(&test_misc_read_6_lock);
		} else {
			EXPECT_TRUE(PFC_FALSE);
		}
	} while (1);

	ret = close(sock);
	EXPECT_EQ(0, ret);

	return NULL;
}

/*
 * Test
 *      pfc_iostream_read() 5
 */
TEST(misc, pfc_iostream_read_5)
{
	int sock, ret;
	pfc_iostream_t stream;
	int buf_read[]  = {0,0,0};
	size_t si;
	struct sockaddr_un un;
	pfc_timespec_t t_out;
	pfc_thread_t at_thr;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	ret = pfc_iostream_create(&stream, sock, 0, 0, NULL);
	ASSERT_EQ(0, ret);

	ret = pfc_thread_create(&at_thr, test_misc_read_5_func,
			        (void *)&sock, 0);

	si = sizeof(buf_read);
	t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
	t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

	ret = pfc_mutex_lock(&test_misc_read_6_lock);
	ASSERT_EQ(0, ret);
	ret = pfc_iostream_read(stream, (void *)&buf_read, &si, &t_out);
	ASSERT_EQ(EBADF, ret);

	ret = pfc_thread_join(at_thr, NULL);
	ASSERT_EQ(0, ret);

	/* because it was improperly closed. */
	free(stream);
}

/*
 * Test
 *      pfc_iostream_read() 6
 */
TEST(misc, pfc_iostream_read_6)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	void *buf_write;
	size_t si;
	struct sockaddr_un un;
	sigset_t mask;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		int client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		sigaddset(&mask, SIGPIPE);
		iowait.iw_sigmask = &mask;

		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		TmpBuffer tbuf(INT32_MAX/2);
		buf_write = tbuf.getAsMuch();
		ASSERT_TRUE(buf_write != NULL);
		si = tbuf.getSize();

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_write(stream, buf_write, &si, NULL);
		ASSERT_EQ(EPIPE, ret);
		ret = pfc_iostream_read(stream, buf_write, &si, NULL);
		ASSERT_EQ(0, ret);
		ASSERT_EQ((size_t)0, si);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);
	}
}

/*
 * Test
 *      pfc_iostream_write() 
 */
TEST(misc, pfc_iostream_write_1)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pid_t fork_pid;
	void *buf_write;
	size_t si;
	struct sockaddr_un un;
	int client_sockfd;
	pfc_timespec_t t_out;


	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		TmpBuffer tbuf(INT32_MAX/2);
		buf_write = tbuf.getAsMuch();
		CHILD_ASSERT_TRUE(tmp, buf_write != NULL);
		si = tbuf.getSize();
		t_out.tv_sec = TEST_MISC_TIMEOUT_SEC;
		t_out.tv_nsec = TEST_MISC_TIMEOUT_NSEC;

		ret = pfc_iostream_write(stream, buf_write, &si, &t_out);

		/* time-out check */
		CHILD_ASSERT_EQ(tmp, ETIMEDOUT, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		ASSERT_EQ(0, ret);

		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);
	}
}

/*
 * Test
 *      pfc_iostream_write() 2
 */
TEST(misc, pfc_iostream_write_2)
{
	int ret;
	size_t si;

	si = 0;

	ret = pfc_iostream_write(NULL, NULL, &si , NULL);
	ASSERT_EQ(0, ret);

}

/*
 * Test
 *      pfc_iostream_write() 3
 */
TEST(misc, pfc_iostream_write_3)
{
	int ret;
	size_t si;

	si = PFC_IOSTREAM_MAXSIZE + 1;

	ret = pfc_iostream_write(NULL, NULL, &si , NULL);
	ASSERT_EQ(E2BIG, ret);
}


/*
 * Test
 *      pfc_iostream_write() 4
 */
TEST(misc, pfc_iostream_write_4)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	void *buf_write;
	size_t si;
	struct sockaddr_un un;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	SignalHandler  sigusr1(SIGUSR1);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = test_misc_callback_true;
		iowait.iw_intrarg = NULL;
		iowait.iw_sigmask = NULL;

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		TmpBuffer tbuf(INT32_MAX/2);
		buf_write = tbuf.getAsMuch();
		CHILD_ASSERT_TRUE(tmp, buf_write != NULL);
		si = tbuf.getSize();

		kill(getppid(), SIGUSR1);
		ret = pfc_iostream_write(stream, buf_write, &si, NULL);

		/* call back check */
		CHILD_ASSERT_TRUE(tmp, test_misc_callback_true_flag);
		test_misc_callback_true_flag = PFC_FALSE;
		CHILD_ASSERT_TRUE(tmp, test_misc_sig_handler_flag);
		test_misc_sig_handler_flag = PFC_FALSE;
		CHILD_ASSERT_EQ(tmp, EINTR, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		kill_and_wait(fork_pid, SIGINT, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      pfc_iostream_write() 5
 */
TEST(misc, pfc_iostream_write_5)
{
	int sock, ret, status;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	void *buf_write;
	size_t si;
	struct sockaddr_un un;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	SignalHandler  sigusr1(SIGUSR1);
	sigusr1.bind(pthread_self());
	ASSERT_TRUE(sigusr1.install());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		/* set signal handler */
		sigset(SIGINT, test_misc_sig_handler);
		iowait.iw_intrfunc = test_misc_callback_false;
		iowait.iw_intrarg = NULL;
		iowait.iw_sigmask = NULL;

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		TmpBuffer tbuf(INT32_MAX/2);
		buf_write = tbuf.getAsMuch();
		CHILD_ASSERT_TRUE(tmp, buf_write != NULL);
		si = tbuf.getSize();

		kill(getppid(), SIGUSR1);
		sleep(1);
		ret = pfc_iostream_write(stream, buf_write, &si, NULL);

		/* NOTREACHED */
		CHILD_ASSERT_TRUE(tmp, PFC_FALSE);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		wait_child(sigusr1, fork_pid);
		RETURN_ON_ERROR();

		struct timespec ts = {
			0, 100 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC),
		};
		nanosleep(&ts, NULL);
		ASSERT_EQ(0, kill(fork_pid, SIGINT));

		kill_and_wait(fork_pid, SIGALRM, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFSIGNALED(status));
		ASSERT_EQ(SIGALRM, WTERMSIG(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test case for pfc_iostream_setcanceller().
 */
TEST(misc, pfc_iostream_setcanceller)
{
    int	fds[2], sdpipe[2];

    int	nullfd(open("/dev/null", O_WRONLY));
    ASSERT_NE(-1, nullfd);
    FdRef	rnull(nullfd);

    ASSERT_EQ(0, pfc_pipe_open(fds, PFC_PIPE_NONBLOCK));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd0(&sdpipe[0]);

    {
        pfc_iostream_t	stream;
        ASSERT_EQ(0, pfc_iostream_create(&stream, fds[0], 0, 0, NULL));
        IoStream	ios(stream);

        // Invalid file descriptor.
        ASSERT_EQ(EBADF, pfc_iostream_setcanceller(stream, sdpipe[1]));

        // Write only file descriptor.
        ASSERT_EQ(EACCES, pfc_iostream_setcanceller(stream, nullfd));

        // Invalid stream.
        ASSERT_EQ(0, pfc_iostream_shutdown(stream, PFC_IOSTREAM_SHUT_RDWR));
        ASSERT_EQ(ESHUTDOWN, pfc_iostream_setcanceller(stream, sdpipe[0]));
    }
}

/*
 * Test case for cancellation of pfc_iostream_read().
 */
TEST(misc, pfc_iostream_read_cancel)
{
    int	fds[2], sdpipe[2];

    ASSERT_EQ(0, pfc_pipe_open(fds, PFC_PIPE_NONBLOCK));
    (void)close(fds[1]);
    FdRef	rfd0(&fds[0]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd0(&sdpipe[0]);

    pfc_iostream_t	stream;
    ASSERT_EQ(0, pfc_iostream_create(&stream, fds[0], 0, 0, NULL));
    IoStream	ios(stream);

    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, sdpipe[0]));

    // This read must be canceled.
    char	buf;
    size_t	size(sizeof(buf));
    ASSERT_EQ(ECANCELED, pfc_iostream_read(stream, &buf, &size, NULL));

    // Disable cancellation.
    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, -1));
    size = sizeof(buf);
    ASSERT_EQ(0, pfc_iostream_read(stream, &buf, &size, NULL));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for cancellation of pfc_iostream_write().
 */
TEST(misc, pfc_iostream_write_cancel)
{
    int	fds[2], sdpipe[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    (void)close(fds[0]);
    FdRef	rfd1(&fds[1]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd1(&sdpipe[0]);

    pfc_iostream_t	stream;
    ASSERT_EQ(0, pfc_iostream_create(&stream, fds[1], 0, 0, NULL));
    IoStream	ios(stream);

    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, sdpipe[0]));

    // This write must be canceled.
    char	buf(0);
    size_t	size(sizeof(buf));
    ASSERT_EQ(ECANCELED, pfc_iostream_write(stream, &buf, &size, NULL));

    // Disable cancellation.
    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, -1));
    size = sizeof(buf);
    ASSERT_EQ(EPIPE, pfc_iostream_write(stream, &buf, &size, NULL));
}

/*
 * Test case for cancellation of pfc_iostream_flush().
 */
TEST(misc, pfc_iostream_flush_cancel)
{
    int	fds[2], sdpipe[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd1(&sdpipe[0]);

    pfc_iostream_t	stream;
    ASSERT_EQ(0, pfc_iostream_create(&stream, fds[1], 0, 32, NULL));
    IoStream	ios(stream);

    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, sdpipe[0]));
    char	buf(0);
    size_t	size(sizeof(buf));
    ASSERT_EQ(0, pfc_iostream_write(stream, &buf, &size, NULL));
    ASSERT_EQ(sizeof(buf), size);
    (void)close(fds[0]);
    fds[0] = -1;

    // This flush request must be canceled.
    ASSERT_EQ(ECANCELED, pfc_iostream_flush(stream, NULL));

    // Disable cancellation.
    ASSERT_EQ(0, pfc_iostream_setcanceller(stream, -1));
    size = sizeof(buf);
    ASSERT_EQ(EPIPE, pfc_iostream_flush(stream, NULL));
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - Pass NULL as ucred pointer.
 */
TEST(misc, pfc_iostream_cred_1)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    ASSERT_EQ(0, __pfc_iostream_sendcred(wstream, &cur, &size, NULL, NULL));
    ASSERT_EQ(sizeof(cur), size);

    pfc_ucred_t	cred;
    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, __pfc_iostream_recvcred(rstream, &t, &size, &cred, NULL));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(cur, t);

    uid_t	uid(getuid());
    gid_t	gid(getgid());
    ASSERT_EQ(cred.pid, getpid());
    ASSERT_EQ(cred.uid, uid);

    // Linux 3.0.1 has a bug that uid is copied to cred.gid if the calling
    // process is setuid or setgid process.
    if (uid == geteuid() && gid == getegid()) {
        ASSERT_EQ(cred.gid, getgid());
    }
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - Set effective UID and GID to the credential.
 */
TEST(misc, pfc_iostream_cred_2)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));
    pfc_timespec_t	tout;
    tout.tv_sec = 3;
    tout.tv_nsec = 0;

    pfc_ucred_t	cred;
    cred.pid = getpid();
    cred.uid = geteuid();
    cred.gid = getegid();
    ASSERT_EQ(0, __pfc_iostream_sendcred(wstream, &cur, &size, &cred, &tout));
    ASSERT_EQ(sizeof(cur), size);

    pfc_ucred_t	cred1;
    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, __pfc_iostream_recvcred(rstream, &t, &size, &cred1, &tout));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(cur, t);
    ASSERT_EQ(cred.pid, cred1.pid);
    ASSERT_EQ(cred.uid, cred1.uid);
    ASSERT_EQ(cred.gid, cred1.gid);
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - All fields must be initialized with -1 if recvmsg() is not issued.
 */
TEST(misc, pfc_iostream_cred_4)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	buffer[2];
    buffer[0] = time(NULL);
    buffer[1] = buffer[0] * 2;
    size_t	size(sizeof(buffer));
    pfc_timespec_t	tout;
    tout.tv_sec = 3;
    tout.tv_nsec = 0;

    ASSERT_EQ(0, pfc_iostream_write(wstream, buffer, &size, &tout));
    ASSERT_EQ(0, pfc_iostream_flush(wstream, &tout));
    ASSERT_EQ(sizeof(buffer), size);

    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, pfc_iostream_read(rstream, &t, &size, &tout));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(buffer[0], t);

    pfc_ucred_t	cred;
    size = sizeof(t);
    ASSERT_EQ(0, __pfc_iostream_recvcred(rstream, &t, &size, &cred, &tout));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(buffer[1], t);
    ASSERT_EQ(static_cast<pid_t>(-1), cred.pid);
    ASSERT_EQ(static_cast<uid_t>(-1), cred.uid);
    ASSERT_EQ(static_cast<gid_t>(-1), cred.gid);
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - ENOTSOCK is returned if the stream is not socket.
 */
TEST(misc, pfc_iostream_cred_ENOTSOCK)
{
    int	fds[2];

    ASSERT_EQ(0, pfc_pipe_open(fds, PFC_PIPE_NONBLOCK));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    pfc_timespec_t	tout;
    tout.tv_sec = 5;
    tout.tv_nsec = 0;
    ASSERT_EQ(ENOTSOCK, __pfc_iostream_sendcred(wstream, &cur, &size, NULL,
                                                &tout));
    ASSERT_EQ(0U, size);

    pfc_ucred_t	cred;
    size = sizeof(cur);
    ASSERT_EQ(ENOTSOCK, __pfc_iostream_recvcred(rstream, &cur, &size,
                                                &cred, &tout));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - Set invalid value to the credentials.
 */
TEST(misc, pfc_iostream_cred_EPERM)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    pid_t	pid(getpid());
    uid_t	uid(getuid());
    gid_t	gid(getgid());

    if (geteuid() == 0) {
        return;
    }

    /* Set invalid PID. */
    pfc_ucred_t	cred;
    cred.pid = pid + 100;
    cred.uid = uid;
    cred.gid = gid;
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred(wstream, &cur, &size, &cred,
                                             NULL));
    ASSERT_EQ(0U, size);

    /* Set invalid UID. */
    cred.pid = pid;
    cred.uid = uid + 100;
    cred.gid = gid;
    size = sizeof(cur);
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred(wstream, &cur, &size, &cred,
                                             NULL));
    ASSERT_EQ(0U, size);

    /* Set invalid GID. */
    cred.pid = pid;
    cred.uid = uid;
    cred.gid = gid + 100;
    size = sizeof(cur);
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred(wstream, &cur, &size, &cred,
                                             NULL));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - ETIMEDOUT error test.
 */
TEST(misc, pfc_iostream_cred_ETIMEDOUT)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    uint32_t	outsize(1024);
    ASSERT_EQ(0, setsockopt(fds[1], SOL_SOCKET, SO_SNDBUF, &outsize,
                            sizeof(outsize)));
    socklen_t	len(sizeof(outsize));
    ASSERT_EQ(0, getsockopt(fds[1], SOL_SOCKET, SO_SNDBUF, &outsize, &len));
    outsize <<= 1;
    TmpBuffer	buffer(outsize);
    memset(*buffer, 0, outsize);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 0, 0, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    size_t	size(outsize);
    pfc_ucred_t	cred;
    pfc_timespec_t	tout, abs, cur;
    tout.tv_sec = 0;
    tout.tv_nsec = 150 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ETIMEDOUT, __pfc_iostream_recvcred(rstream, *buffer, &size,
                                                 &cred, &tout));
    ASSERT_EQ(0, pfc_clock_gettime(&cur));
    ASSERT_LE(0, pfc_clock_compare(&cur, &abs));
    ASSERT_EQ(0U, size);

    // Fill the output buffer.
    int	sock(pfc_iostream_getfd(wstream));
    uint32_t	sz(outsize);
    for (;;) {
        ssize_t	nbytes(write(sock, *buffer, sz));

        if (nbytes == (ssize_t)-1) {
            int	err(errno);

            ASSERT_TRUE(PFC_IS_EWOULDBLOCK(err));
            if (sz == 1) {
                break;
            }

            sz >>= 1;
            if (sz == 0) {
                sz = 1;
            }
        }
    }

    size = outsize;
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ETIMEDOUT, __pfc_iostream_sendcred(wstream, *buffer, &size,
                                                 NULL, &tout));
    ASSERT_EQ(0, pfc_clock_gettime(&cur));
    ASSERT_LE(0, pfc_clock_compare(&cur, &abs));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred() and __pfc_iostream_recvcred().
 *
 * - Cancellation test.
 */
TEST(misc, pfc_iostream_cred_ECANCELED)
{
    int	fds[2], sdpipe[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd1(&sdpipe[0]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 0, 0, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    ASSERT_EQ(0, pfc_iostream_setcanceller(rstream, sdpipe[0]));
    ASSERT_EQ(0, pfc_iostream_setcanceller(wstream, sdpipe[0]));

    pfc_timespec_t	tout;
    tout.tv_sec = 1;
    tout.tv_nsec = 0;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));
    pfc_ucred_t	cred;

    ASSERT_EQ(ECANCELED, __pfc_iostream_recvcred(rstream, &cur, &size,
                                                 &cred, &tout));
    ASSERT_EQ(0U, size);

    size = sizeof(cur);
    ASSERT_EQ(ECANCELED, __pfc_iostream_sendcred(wstream, &cur, &size,
                                                 NULL, &tout));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - Pass NULL as ucred pointer.
 */
TEST(misc, pfc_iostream_cred_abs_1)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    ASSERT_EQ(0, __pfc_iostream_sendcred_abs(wstream, &cur, &size, NULL, NULL));
    ASSERT_EQ(sizeof(cur), size);

    pfc_ucred_t	cred;
    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, __pfc_iostream_recvcred_abs(rstream, &t, &size, &cred, NULL));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(cur, t);

    uid_t	uid(getuid());
    gid_t	gid(getgid());
    ASSERT_EQ(cred.pid, getpid());
    ASSERT_EQ(cred.uid, getuid());

    // Linux 3.0.1 has a bug that uid is copied to cred.gid if the calling
    // process is setuid or setgid process.
    if (uid == geteuid() && gid == getegid()) {
        ASSERT_EQ(cred.gid, getgid());
    }
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - Set effective UID and GID to the credential.
 */
TEST(misc, pfc_iostream_cred_abs_2)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));
    pfc_timespec_t	tout, abs;
    tout.tv_sec = 3;
    tout.tv_nsec = 0;

    pfc_ucred_t	cred;
    cred.pid = getpid();
    cred.uid = geteuid();
    cred.gid = getegid();
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(0, __pfc_iostream_sendcred_abs(wstream, &cur, &size, &cred,
                                             &abs));
    ASSERT_EQ(sizeof(cur), size);

    pfc_ucred_t	cred1;
    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(0, __pfc_iostream_recvcred_abs(rstream, &t, &size, &cred1,
                                             &abs));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(cur, t);
    ASSERT_EQ(cred.pid, cred1.pid);
    ASSERT_EQ(cred.uid, cred1.uid);
    ASSERT_EQ(cred.gid, cred1.gid);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - All fields must be initialized with -1 if recvmsg() is not issued.
 */
TEST(misc, pfc_iostream_cred_abs_4)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	buffer[2];
    buffer[0] = time(NULL);
    buffer[1] = buffer[0] * 2;
    size_t	size(sizeof(buffer));
    pfc_timespec_t	tout, abs;
    tout.tv_sec = 3;
    tout.tv_nsec = 0;

    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(0, pfc_iostream_write_abs(wstream, buffer, &size, &abs));
    ASSERT_EQ(0, pfc_iostream_flush_abs(wstream, &abs));
    ASSERT_EQ(sizeof(buffer), size);

    time_t	t;
    size = sizeof(t);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(0, pfc_iostream_read_abs(rstream, &t, &size, &abs));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(buffer[0], t);

    pfc_ucred_t	cred;
    size = sizeof(t);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(0, __pfc_iostream_recvcred_abs(rstream, &t, &size, &cred, &abs));
    ASSERT_EQ(sizeof(t), size);
    ASSERT_EQ(buffer[1], t);
    ASSERT_EQ(static_cast<pid_t>(-1), cred.pid);
    ASSERT_EQ(static_cast<uid_t>(-1), cred.uid);
    ASSERT_EQ(static_cast<gid_t>(-1), cred.gid);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - ENOTSOCK is returned if the stream is not socket.
 */
TEST(misc, pfc_iostream_cred_abs_ENOTSOCK)
{
    int	fds[2];

    ASSERT_EQ(0, pfc_pipe_open(fds, PFC_PIPE_NONBLOCK));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    pfc_timespec_t	tout, abs;
    tout.tv_sec = 5;
    tout.tv_nsec = 0;

    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ENOTSOCK, __pfc_iostream_sendcred_abs(wstream, &cur, &size, NULL,
                                                    &abs));
    ASSERT_EQ(0U, size);

    pfc_ucred_t	cred;
    size = sizeof(cur);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ENOTSOCK, __pfc_iostream_recvcred_abs(rstream, &cur, &size,
                                                    &cred, &abs));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - Set invalid value to the credentials.
 */
TEST(misc, pfc_iostream_cred_abs_EPERM)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 64, 64, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 64, 64, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));

    pid_t	pid(getpid());
    uid_t	uid(getuid());
    gid_t	gid(getgid());

    if (geteuid() == 0) {
        return;
    }

    pfc_timespec_t	tout, abs;
    tout.tv_sec = 5;
    tout.tv_nsec = 0;

    /* Set invalid PID. */
    pfc_ucred_t	cred;
    cred.pid = pid + 1;
    cred.uid = uid;
    cred.gid = gid;
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred_abs(wstream, &cur, &size, &cred,
                                                 &abs));
    ASSERT_EQ(0U, size);

    /* Set invalid UID. */
    cred.pid = pid;
    cred.uid = uid + 1;
    cred.gid = gid;
    size = sizeof(cur);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred(wstream, &cur, &size, &cred,
                                             &abs));
    ASSERT_EQ(0U, size);

    /* Set invalid GID. */
    cred.pid = pid;
    cred.uid = uid;
    cred.gid = gid + 1;
    size = sizeof(cur);
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(EPERM, __pfc_iostream_sendcred(wstream, &cur, &size, &cred,
                                             &abs));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - ETIMEDOUT error test.
 */
TEST(misc, pfc_iostream_cred_abs_ETIMEDOUT)
{
    int	fds[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    uint32_t	outsize(1024);
    ASSERT_EQ(0, setsockopt(fds[1], SOL_SOCKET, SO_SNDBUF, &outsize,
                            sizeof(outsize)));
    socklen_t	len(sizeof(outsize));
    ASSERT_EQ(0, getsockopt(fds[1], SOL_SOCKET, SO_SNDBUF, &outsize, &len));
    outsize <<= 1;
    TmpBuffer	buffer(outsize);
    memset(*buffer, 0, outsize);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 0, 0, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    size_t	size(outsize);
    pfc_ucred_t	cred;
    pfc_timespec_t	tout, abs, cur;
    tout.tv_sec = 0;
    tout.tv_nsec = 150 * (PFC_CLOCK_NANOSEC / PFC_CLOCK_MILLISEC);

    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ETIMEDOUT, __pfc_iostream_recvcred_abs(rstream, *buffer, &size,
                                                     &cred, &abs));
    ASSERT_EQ(0, pfc_clock_gettime(&cur));
    ASSERT_LE(0, pfc_clock_compare(&cur, &abs));
    ASSERT_EQ(0U, size);

    // Fill the output buffer.
    int	sock(pfc_iostream_getfd(wstream));
    uint32_t	sz(outsize);
    for (;;) {
        ssize_t	nbytes(write(sock, *buffer, sz));

        if (nbytes == (ssize_t)-1) {
            int	err(errno);

            ASSERT_TRUE(PFC_IS_EWOULDBLOCK(err));
            if (sz == 1) {
                break;
            }

            sz >>= 1;
            if (sz == 0) {
                sz = 1;
            }
        }
    }

    size = outsize;
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));
    ASSERT_EQ(ETIMEDOUT, __pfc_iostream_sendcred_abs(wstream, *buffer, &size,
                                                     NULL, &abs));
    ASSERT_EQ(0, pfc_clock_gettime(&cur));
    ASSERT_LE(0, pfc_clock_compare(&cur, &abs));
    ASSERT_EQ(0U, size);
}

/*
 * Test case for __pfc_iostream_sendcred_abs() and
 * __pfc_iostream_recvcred_abs().
 *
 * - Cancellation test.
 */
TEST(misc, pfc_iostream_cred_abs_ECANCELED)
{
    int	fds[2], sdpipe[2];

    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    FdRef	rfd0(&fds[0]), rfd1(&fds[1]);

    ASSERT_EQ(0, pfc_pipe_open(sdpipe, PFC_PIPE_NONBLOCK));
    (void)close(sdpipe[1]);
    FdRef	rsd1(&sdpipe[0]);

    pfc_iostream_t	rstream;
    ASSERT_EQ(0, pfc_iostream_create(&rstream, fds[0], 0, 0, NULL));
    IoStream	rios(rstream);
    fds[0] = -1;

    pfc_iostream_t	wstream;
    ASSERT_EQ(0, pfc_iostream_create(&wstream, fds[1], 0, 0, NULL));
    IoStream	wios(wstream);
    fds[1] = -1;

    ASSERT_EQ(0, pfc_iostream_setcanceller(rstream, sdpipe[0]));
    ASSERT_EQ(0, pfc_iostream_setcanceller(wstream, sdpipe[0]));

    pfc_timespec_t	tout, abs;
    tout.tv_sec = 5;
    tout.tv_nsec = 0;
    ASSERT_EQ(0, pfc_clock_abstime(&abs, &tout));

    time_t	cur(time(NULL));
    size_t	size(sizeof(cur));
    pfc_ucred_t	cred;

    ASSERT_EQ(ECANCELED, __pfc_iostream_recvcred_abs(rstream, &cur, &size,
                                                     &cred, &abs));
    ASSERT_EQ(0U, size);

    size = sizeof(cur);
    ASSERT_EQ(ECANCELED, __pfc_iostream_sendcred_abs(wstream, &cur, &size,
                                                     NULL, &abs));
    ASSERT_EQ(0U, size);
}

/*
 * Test
 *      cproto_cmd_read()
 */
TEST(misc, cproto_cmd_read_1)
{
	int sock, ret, i;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	ctrl_cmdtype_t write_cmd[TEST_MISC_CMD_COUNT] = {
		CTRL_CMDTYPE_NOP,
		CTRL_CMDTYPE_QUIT,
		CTRL_CMDTYPE_LOGLEVEL,
		CTRL_CMDTYPE_MODLIST,
	};
	ctrl_cmdtype_t cmd = -1;
	sigset_t mask;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		for (i = 0; i < TEST_MISC_CMD_COUNT; i++) {
			ret = cproto_cmd_write(&session, write_cmd[i]);
			CHILD_ASSERT_EQ(tmp, 0, ret);
			ret = pfc_iostream_flush(stream, NULL);
			CHILD_ASSERT_EQ(tmp, 0, ret);
		}

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		for (i = 0; i < TEST_MISC_CMD_COUNT; i++) {
			ret = cproto_cmd_read(&session, &cmd);
			ASSERT_EQ(0, ret);
			ASSERT_EQ(write_cmd[i], cmd);
		}

		ret = cproto_cmd_read(&session, &cmd);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(CTRL_CMDTYPE_EOF, cmd);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_cmd_read() 2
 */
TEST(misc, cproto_cmd_read_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	ctrl_cmdtype_t cmd = -1;
	sigset_t mask;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* cproto_cmd_write() */

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_cmd_read(&session, &cmd);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(CTRL_CMDTYPE_EOF, cmd);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_cmd_read() 3
 */
TEST(misc, cproto_cmd_read_3)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	ctrl_cmdtype_t cmd = -1;
	sigset_t mask;
	void *write_cmd;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* write_cmd size is less than sizeof(ctrl_cmdtype_t) */
		si = 1;
		TmpBuffer tbuf(si);
		write_cmd = *tbuf;
		CHILD_ASSERT_TRUE(tmp, write_cmd != NULL);
		ret = pfc_iostream_write(session.cps_stream, write_cmd, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_cmd_read(&session, &cmd);
		ASSERT_EQ(EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_int32()
 */
TEST(misc, cproto_data_int32)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data;
	int32_t data_i32 = -1234;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_data_write_int32(&session, data_i32);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(data_i32, cproto_data_int32(&data));
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_text()
 */
TEST(misc, cproto_data_text_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data;
	const char *data_txt = "TEST_DATA";

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_data_write_text(&session, data_txt);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);

#if 0
		/*
		 * Assertion Failed
		 *       PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_INT32)
		 */
		cproto_data_int32(&data);
#else
		ASSERT_STREQ(data_txt, cproto_data_text(&data));
#endif
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_text() 2
 */
TEST(misc, cproto_data_text_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data;
	int32_t data_i32 = -1234;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_data_write_int32(&session, data_i32);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);

#if 0
		/*
		 * Assertion Failed
		 *       PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_INT32)
		 */
		cproto_data_text(&data);
#else
		ASSERT_EQ(data_i32, cproto_data_int32(&data));
#endif

		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_read() 1
 */
TEST(misc, cproto_data_read_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_data_write_null(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);
		ASSERT_EQ((uint32_t)0, data.cpd_size);
		ASSERT_TRUE(data.cpd_value == NULL);

		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_read() 2
 */
TEST(misc, cproto_data_read_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* INVALID type */
		data_snd.cpd_type = 0x0003U;
		si = sizeof(ctrl_pdtype_t);

		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(EPROTO, ret);
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_read() 3
 */
TEST(misc, cproto_data_read_3)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_type = CTRL_PDTYPE_INT32;
		si = sizeof(ctrl_pdtype_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_size = sizeof(uint16_t);
		/* invalid size */
		si = sizeof(uint16_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_size, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(EPROTO, ret);
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_read() 4
 */
TEST(misc, cproto_data_read_4)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_type = CTRL_PDTYPE_INT32;
		si = sizeof(ctrl_pdtype_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		/* invalid size */
		data_snd.cpd_size = 0;
		si = sizeof(ctrl_pdsize_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_size, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(EPROTO, ret);
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_uint32()
 */
TEST(misc, cproto_data_uint32_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	uint32_t data_ui32 = 1234;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_type = CTRL_PDTYPE_INT32;
		si = sizeof(ctrl_pdtype_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_size = sizeof(data_ui32);
		/* invalid size */
		si = sizeof(uint32_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_size, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_value = (void *)&data_ui32;
		ret = pfc_iostream_write(stream, data_snd.cpd_value, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(data_ui32, cproto_data_uint32(&data));
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_uint32() 2
 */
TEST(misc, cproto_data_uint32_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data;
	const char *data_txt = "TEST_DATA";

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_data_write_text(&session, data_txt);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);

#if 0
		/*
		 * Assertion Failed
		 *       PFC_ASSERT(datap->cpd_type == CTRL_PDTYPE_INT32)
		 */
		cproto_data_uint32(&data);
#else
		ASSERT_STREQ(data_txt, cproto_data_text(&data));
#endif
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_data_write_text()
 */
TEST(misc, cproto_data_write_text)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	char *data_txt;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		size_t	bufsz(0x10000U + 2);
		TmpBuffer tbuf(bufsz);
		data_txt = reinterpret_cast<char *>(*tbuf);
		CHILD_ASSERT_TRUE(tmp, data_txt != NULL);
		memset(data_txt, 'a', bufsz - 1);
		*(data_txt + bufsz - 1) = '\0';

		ret = cproto_data_write_text(&session, data_txt);
		CHILD_ASSERT_EQ(tmp, E2BIG, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_handshake_read()
 */
TEST(misc, cproto_handshake_read_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	int32_t data_i16 = -12;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * invalid handshake message
		 *       (size != sizeof(cproto_hshake_t)
		 */
		si = sizeof(data_i16);
		ret = pfc_iostream_write(stream, &data_i16, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_handshake_read() 2
 */
TEST(misc, cproto_handshake_read_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_hshake_t msg;
	size_t si;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * invalid handshake message
		 *       magic number error
		 */
		si = sizeof(msg);
		ret = pfc_iostream_write(stream, &msg, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_handshake_read() 3
 */
TEST(misc, cproto_handshake_read_3)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_hshake_t msg;
	size_t si;
	uint32_t i;
	const uint8_t cproto_magic[CTRL_PROTO_MAGIC_SIZE] = {0xf7,
							     'C',
							     't',
							     'L',
	};

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * invalid handshake message
		 *       protocol version error
		 */
		si = sizeof(msg);
		for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
			msg.cph_magic[i] = cproto_magic[i];
		}
		msg.cph_version = CTRL_PROTO_VERSION - 1;
		msg.cph_order = CTRL_ORDER_NATIVE;
		ret = pfc_iostream_write(stream, &msg, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_handshake_read() 4
 */
TEST(misc, cproto_handshake_read_4)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_hshake_t msg;
	size_t si;
	uint32_t i;
	const uint8_t cproto_magic[CTRL_PROTO_MAGIC_SIZE] = {0xf7,
							     'C',
							     't',
							     'L',
	};

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * handshake message
		 *       byte order = CTRL_ORDER_NATIVE
		 */
		si = sizeof(msg);
		for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
			msg.cph_magic[i] = cproto_magic[i];
		}
		msg.cph_version = CTRL_PROTO_VERSION;
		msg.cph_order = CTRL_ORDER_NATIVE;
		ret = pfc_iostream_write(stream, &msg, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* reset byte order */
		session.cps_bswap = PFC_TRUE;
		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);
		ASSERT_FALSE(session.cps_bswap);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_handshake_read() 5
 */
TEST(misc, cproto_handshake_read_5)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_hshake_t msg;
	size_t si;
	uint32_t i;
	const uint8_t cproto_magic[CTRL_PROTO_MAGIC_SIZE] = {0xf7,
							     'C',
							     't',
							     'L',
	};

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * invalid handshake message
		 *       byte order error
		 */
		si = sizeof(msg);
		for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
			msg.cph_magic[i] = cproto_magic[i];
		}
		msg.cph_version = CTRL_PROTO_VERSION;
		msg.cph_order = CTRL_ORDER_NATIVE + 2;
		ret = pfc_iostream_write(stream, &msg, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}


/*
 * Test
 *      cproto_handshake_read() 6
 */
TEST(misc, cproto_handshake_read_6)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_hshake_t msg;
	size_t si;
	uint32_t i;
	const uint8_t cproto_magic[CTRL_PROTO_MAGIC_SIZE] = {0xf7,
							     'C',
							     't',
							     'L',
	};

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		/*
		 * handshake message
		 *       byte order = CTRL_ORDER_NATIVE
		 */
		si = sizeof(msg);
		for (i = 0; i < CTRL_PROTO_MAGIC_SIZE; i++) {
			msg.cph_magic[i] = cproto_magic[i];
		}
		msg.cph_version = CTRL_PROTO_VERSION;

		if (CTRL_ORDER_NATIVE != CTRL_ORDER_LITTLE) {
			msg.cph_order = CTRL_ORDER_LITTLE;
		} else {
			msg.cph_order = CTRL_ORDER_BIG;
		}
		ret = pfc_iostream_write(stream, &msg, &si, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* reset byte order */
		session.cps_bswap = PFC_FALSE;
		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);
		ASSERT_TRUE(session.cps_bswap);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_resp_read()
 */
TEST(misc, cproto_resp_read_1)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	uint32_t data_ui32 = 1234;
	size_t si;
	ctrl_cmdresp_t	resp;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp,-1, ret);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
				       sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_type = CTRL_PDTYPE_INT32;
		si = sizeof(ctrl_pdtype_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_size = sizeof(data_ui32);
		/* invalid size */
		si = sizeof(uint32_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_size, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_value = (void *)&data_ui32;
		ret = pfc_iostream_write(stream, data_snd.cpd_value, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_resp_read(&session, &resp);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		CHILD_ASSERT_EQ(tmp, static_cast<ctrl_cmdresp_t>(CTRL_RESP_OK),
				resp);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(data_ui32, cproto_data_uint32(&data));
		if (ret == 0) {
			cproto_data_free(&data);
		}

		ret = cproto_resp_write(&session, CTRL_RESP_OK);
		ASSERT_EQ(0, ret);
		/* same as ctrl_terminate_response() */
		ret = pfc_iostream_flush(stream, NULL);
		ASSERT_EQ(0, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}

/*
 * Test
 *      cproto_resp_read() 2
 */
TEST(misc, cproto_resp_read_2)
{
	int sock, ret;
	pfc_iostream_t stream;
	pfc_iowait_t iowait;
	pid_t fork_pid;
	struct sockaddr_un un;
	int client_sockfd;
	cproto_sess_t session;
	sigset_t mask;
	cproto_data_t data, data_snd;
	uint32_t data_ui32 = 1234;
	size_t si;
	ctrl_cmdresp_t	resp;

	SocketFile sfile;
	const char *path(*sfile);
	un.sun_family = AF_UNIX;
	pfc_strlcpy_assert(un.sun_path, path, sizeof(un.sun_path));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_NE(-1, sock);
	FdRef servsock(sock);

	ret = bind(sock, (const struct sockaddr *)&un, sizeof(un));
	ASSERT_NE(-1, ret);

	ret = listen(sock, 1);
	ASSERT_NE(-1, ret);

	TmpFile	tmp(CHILD_ERROR_FILE);
	ASSERT_EQ(0, tmp.createFile());

	fork_pid = fork();
	ASSERT_GE(fork_pid, 0);

	if (fork_pid == 0) {
		/* Child */
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		CHILD_ASSERT_NE(tmp, -1, sock);

		ret = pfc_set_nonblock(sock, PFC_TRUE);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_sock_connect(sock, (const struct sockaddr *)&un,
					sizeof(un), NULL, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_create(&stream, sock, TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		cproto_sess_init(&session, stream, NULL, NULL);

		ret = cproto_handshake_write(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_handshake_read(&session);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_type = CTRL_PDTYPE_INT32;
		si = sizeof(ctrl_pdtype_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_type, &si,
					  NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_size = sizeof(data_ui32);
		/* invalid size */
		si = sizeof(uint32_t);
		ret = pfc_iostream_write(stream, &data_snd.cpd_size, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		data_snd.cpd_value = (void *)&data_ui32;
		ret = pfc_iostream_write(stream, data_snd.cpd_value, &si,
					 NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = pfc_iostream_flush(stream, NULL);
		CHILD_ASSERT_EQ(tmp, 0, ret);

		ret = cproto_resp_read(&session, &resp);
		CHILD_ASSERT_EQ(tmp, EPROTO, ret);

		ret = pfc_iostream_destroy(stream);
		CHILD_ASSERT_EQ(tmp, 0, ret);
		_exit(EXIT_SUCCESS);

	} else if (fork_pid > 0) {
		/* Parent */
		client_sockfd = accept(sock, NULL, NULL);
		ASSERT_NE(-1, client_sockfd);

		iowait.iw_intrfunc = NULL;
		iowait.iw_intrarg = NULL;
		sigemptyset(&mask);
		iowait.iw_sigmask = &mask;

		/* Create control protocol stream. */
		ret = pfc_iostream_create(&stream, client_sockfd,
					  TEST_MISC_BUFSIZE_IN,
					  TEST_MISC_BUFSIZE_OUT, &iowait);
		ASSERT_EQ(0, ret);

		/* Initialize session context. */
		cproto_sess_init(&session, stream, NULL, NULL);

		/* Read a handshake message from the client. */
		ret = cproto_handshake_read(&session);
		ASSERT_EQ(0, ret);

		/* Send a response of handshake. */
		ret = cproto_handshake_write(&session);
		ASSERT_EQ(0, ret);

		ret = cproto_data_read(&session, &data);
		ASSERT_EQ(0, ret);
		ASSERT_EQ(data_ui32, cproto_data_uint32(&data));
		if (ret == 0) {
			cproto_data_free(&data);
		}

		/* invalid response */
		resp = CTRL_RESP_OK;
		si = sizeof(ctrl_cmdresp_t) -1;
		ret = pfc_iostream_write(stream, &resp, &si, NULL);
		ASSERT_EQ(0, ret);

		/* same as ctrl_terminate_response() */
		ret = pfc_iostream_flush(stream, NULL);
		ASSERT_EQ(0, ret);

		ret = pfc_iostream_destroy(stream);
		ASSERT_EQ(0, ret);

		int	status;
		kill_and_wait(fork_pid, 0, status);
		RETURN_ON_ERROR();
		ASSERT_TRUE(WIFEXITED(status));
		ASSERT_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
		VERIFY_CHILD(tmp);
	}
}
