/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_frotate.cc - Test for pfc_frotate_openat().
 */

#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#include <string>
#include <map>
#include <sstream>
#include <pfc/util.h>
#include "test.h"
#include "tmpfile.hh"
#include "misc.hh"
#include "random.hh"

#define	TEST_FILE		"testfile"
#define	TEST_LONG_FILE                                                  \
	"1234567890123456789012345678901234567890123456789012345678901234"
#define	TEST_TOO_LONG_FILE                                              \
	"12345678901234567890123456789012345678901234567890123456789012345"

#define	PERM_MASK	07777

typedef std::map<std::string, struct stat>		filemap_t;
typedef std::map<std::string, struct stat>::iterator	filemap_iter_t;

static const char	characters[] = "abcdefghijklmnopqrstuvwxyz";

static void
filemap_set(filemap_t &map, const char *name, uint32_t index, struct stat &st)
{
    std::string	sname;

    if (index != 0) {
        std::ostringstream	stream;

        stream << name << "." << index;
        sname = stream.str();
    }
    else {
        sname = name;
    }

    st.st_mode |= S_IFREG;
    map[sname] = st;
}

static void
filemap_set_stat(filemap_t &map, int dfd, const char *name, uint32_t index,
                 uint32_t newindex)
{
    const char	*fname;
    char	buf[128];

    struct stat	sbuf;
    if (index == 0) {
        fname = name;
    }
    else {
        snprintf(buf, sizeof(buf), "%s.%u", name, index);
        fname = buf;
    }
    ASSERT_EQ(0, fstatat(dfd, fname, &sbuf, 0))
        << "name=" << fname << ", error = " << strerror(errno);

    snprintf(buf, sizeof(buf), "%s.%u", name, newindex);
    std::string	sname(buf);
    map[sname] = sbuf;
}

#define	FILEMAP_SET_STAT(map, dfd, name, index, newindex)       \
    do {                                                        \
        filemap_set_stat(map, dfd, name, index, newindex);      \
        if (::testing::Test::HasFatalFailure()) {               \
            FAIL();                                             \
        }                                                       \
    } while (0)

static void
filemap_test(filemap_t &map, const char *parent, int dfd)
{
    DIR	*dirp(opendir(parent));
    ASSERT_NE(reinterpret_cast<DIR *>(NULL), dirp);
    DirEntRef	dref(dirp);

    struct dirent	dbuf, *dp;
    while (readdir_r(dirp, &dbuf, &dp) == 0) {
        if (dp == NULL) {
            break;
        }
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        std::string	sname(dp->d_name);
        filemap_iter_t	it(map.find(sname));
        if (it == map.end()) {
            FAIL() << "Unexpected file: " << sname;
        }

        struct stat	sbuf;
        ASSERT_EQ(0, fstatat(dfd, dp->d_name, &sbuf, 0));
        struct stat	req(it->second);
        ASSERT_EQ(req.st_size, sbuf.st_size) << "name=" << dp->d_name;
        ASSERT_EQ(req.st_mode, sbuf.st_mode) << "name=" << dp->d_name;
        if (req.st_ino != 0) {
            ASSERT_EQ(req.st_dev, sbuf.st_dev) << "name=" << dp->d_name;
            ASSERT_EQ(req.st_ino, sbuf.st_ino) << "name=" << dp->d_name;
        }

        map.erase(it);
    }

    ASSERT_EQ(0U, map.size());
}

#define	FILEMAP_TEST(map, parent, dfd)          \
    do {                                        \
        filemap_test(map, parent, dfd);         \
        if (::testing::Test::HasFatalFailure()) {       \
            FAIL();                             \
        }                                       \
    } while (0)

static void
read_file_as_string(std::string &str, int fd)
{
    char	buf[128];

    ssize_t	sz(read(fd, buf, sizeof(buf)));
    ASSERT_NE(-1, sz) << "error = " << strerror(errno);
    if (sz == 0) {
        return;
    }

    str.assign(buf, sz);
}

#define	READ_FILE_AS_STRING(str, fd)                    \
    do {                                                \
        read_file_as_string(str, fd);                   \
        if (::testing::Test::HasFatalFailure()) {       \
            FAIL();                                     \
        }                                               \
    } while (0)

static void
test_always_rotate(int dirfd, const char *parent, const uint32_t maxrotate)
{
    const uint32_t	maxsize(0);
    Umask u;
    mode_t		mode(0600);

    for (uint32_t loop(0); loop < maxrotate + 5; loop++, mode++) {
        const uint32_t	rotate(std::min(loop, maxrotate));

        // File rotation must be always performed.
        filemap_t	map;
        struct stat	sbuf;
        sbuf.st_size = 0;
        sbuf.st_mode = mode;
        sbuf.st_dev = 0;
        sbuf.st_ino = 0;
        filemap_set(map, TEST_LONG_FILE, 0, sbuf);

        for (uint32_t i(0); i < rotate; i++) {
            FILEMAP_SET_STAT(map, dirfd, TEST_LONG_FILE, i, i + 1);
        }

        int	fd(pfc_frotate_openat(dirfd, TEST_LONG_FILE, mode, maxsize,
                                      maxrotate));
        ASSERT_NE(-1, fd) << "error = " << strerror(errno);
        FdRef	fref(fd);
        FILEMAP_TEST(map, parent, dirfd);

        ASSERT_EQ(static_cast<ssize_t>(loop), write(fd, characters, loop));
        ASSERT_EQ(0, fstatat(dirfd, TEST_LONG_FILE, &sbuf, 0));
        ASSERT_EQ(static_cast<size_t>(loop), static_cast<size_t>(sbuf.st_size));
    }
}

static void
test_maxsize10_rotate(int dirfd, const char *parent, const uint32_t maxrotate)
{
    const uint32_t	maxsize(10);
    Umask u;

    for (uint32_t loop(0); loop < maxrotate + 5; loop++) {
        const uint32_t	rotate(std::min(loop, maxrotate));

        // Append characters until the size limit.
        std::string	str;
        const mode_t	mode_base(0600);
        mode_t		mode(mode_base);
        for (uint32_t i(0); i < maxsize; i++, mode++) {
            filemap_t	map;
            struct stat	sbuf;
            sbuf.st_size = i;
            sbuf.st_mode = mode_base;
            sbuf.st_dev = 0;
            sbuf.st_ino = 0;
            filemap_set(map, TEST_FILE, 0, sbuf);

            for (uint32_t j(1); j <= rotate; j++) {
                FILEMAP_SET_STAT(map, dirfd, TEST_FILE, j, j);
            }

            int	fd(pfc_frotate_openat(dirfd, TEST_FILE, mode, maxsize,
                                      maxrotate));
            ASSERT_NE(-1, fd) << "error = " << strerror(errno);
            FdRef	fref(fd);
            FILEMAP_TEST(map, parent, dirfd);

            // A file must be opened with O_APPEND.
            char	c(characters[i]);
            ASSERT_EQ(1, write(fd, &c, 1));
            str.append(1, characters[i]);

            std::string	fstr;
            {
                int	fd2(pfc_openat_cloexec(dirfd, TEST_FILE, O_RDONLY));
                ASSERT_NE(-1, fd2) << "error = " << strerror(errno);
                FdRef	fref2(fd2);
                READ_FILE_AS_STRING(fstr, fd2);
            }

            ASSERT_STREQ(str.c_str(), fstr.c_str());
        }

        // This call must cause file rotation.
        {
            filemap_t	map;
            struct stat	sbuf;
            mode = mode_base;
            sbuf.st_size = 0;
            sbuf.st_mode = mode;
            sbuf.st_dev = 0;
            sbuf.st_ino = 0;
            filemap_set(map, TEST_FILE, 0, sbuf);

            if (maxrotate > 0) {
                for (uint32_t i(0); i <= std::min(rotate, maxrotate - 1);
                     i++) {
                    FILEMAP_SET_STAT(map, dirfd, TEST_FILE, i, i + 1);
                }
            }

            int	fd(pfc_frotate_openat(dirfd, TEST_FILE, mode, maxsize,
                                      maxrotate));
            ASSERT_NE(-1, fd) << "error = " << strerror(errno);
            FdRef	fref(fd);
            FILEMAP_TEST(map, parent, dirfd);
        }
    }
}

/*
 * Test with the following parameters:
 *	maxsize = 0
 *	rotate  = 0
 */
TEST(frotate, always_rotate0)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_always_rotate(dirfd, parent, 0);
}

/*
 * Test with the following parameters:
 *	maxsize = 0
 *	rotate  = 1
 */
TEST(frotate, always_rotate1)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_always_rotate(dirfd, parent, 1);
}

/*
 * Test with the following parameters:
 *	maxsize = 0
 *	rotate  = 2
 */
TEST(frotate, always_rotate2)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_always_rotate(dirfd, parent, 2);
}

/*
 * Test with the following parameters:
 *	maxsize = 0
 *	rotate  = 10
 */
TEST(frotate, always_rotate10)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_always_rotate(dirfd, parent, 10);
}

/*
 * Test with the following parameters:
 *	maxsize = 10
 *	rotate  = 0
 */
TEST(frotate, rotate0)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_maxsize10_rotate(dirfd, parent, 0);
}

/*
 * Test with the following parameters:
 *	maxsize = 10
 *	rotate  = 1
 */
TEST(frotate, rotate1)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_maxsize10_rotate(dirfd, parent, 1);
}

/*
 * Test with the following parameters:
 *	maxsize = 10
 *	rotate  = 2
 */
TEST(frotate, rotate2)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_maxsize10_rotate(dirfd, parent, 2);
}

/*
 * Test with the following parameters:
 *	maxsize = 10
 *	rotate  = 10
 */
TEST(frotate, rotate10)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    const char	*parent(tdir.getPath());
    int	dirfd(open(parent, O_RDONLY));
    ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
    FdRef	dfd(dirfd);

    test_maxsize10_rotate(dirfd, parent, 10);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 0
 *	rotate  = 0
 */
TEST(frotate, cwd_always_rotate0)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_always_rotate(PFC_AT_FDCWD, ".", 0);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 0
 *	rotate  = 1
 */
TEST(frotate, cwd_always_rotate1)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_always_rotate(PFC_AT_FDCWD, ".", 1);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 0
 *	rotate  = 2
 */
TEST(frotate, cwd_always_rotate2)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_always_rotate(PFC_AT_FDCWD, ".", 2);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 0
 *	rotate  = 10
 */
TEST(frotate, cwd_always_rotate10)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_always_rotate(PFC_AT_FDCWD, ".", 10);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 10
 *	rotate  = 0
 */
TEST(frotate, cwd_rotate0)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_maxsize10_rotate(PFC_AT_FDCWD, ".", 0);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 10
 *	rotate  = 1
 */
TEST(frotate, cwd_rotate1)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_maxsize10_rotate(PFC_AT_FDCWD, ".", 1);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 10
 *	rotate  = 2
 */
TEST(frotate, cwd_rotate2)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_maxsize10_rotate(PFC_AT_FDCWD, ".", 2);
}

/*
 * Test with the following parameters:
 *      dfd     = PFC_AT_FDCWD
 *	maxsize = 10
 *	rotate  = 10
 */
TEST(frotate, cwd_rotate10)
{
    Umask  umask;

    TmpDir	tdir("_frotate");
    ASSERT_EQ(0, tdir.createDirectory());
    ChDir	cwd(tdir.getPath());

    test_maxsize10_rotate(PFC_AT_FDCWD, ".", 10);
}

/*
 * Test for file rotation with sparse rotated files.
 */
TEST(frotate, sparse)
{
    Umask  umask;

    const uint32_t	rotate(20);
    RandomGenerator	rand;
    Umask u;

    for (int loop(0); loop < 100; loop++) {
        TmpDir	tdir("_frotate");
        ASSERT_EQ(0, tdir.createDirectory());
        int	dirfd(open(tdir.getPath(), O_RDONLY));
        ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
        FdRef	dfd(dirfd);

        // Create rotated files.
        filemap_t	map;
        uint32_t	nfiles, cnt(0);
        mode_t		mode(0600);
        bool		last1(false), last2(false);
        RANDOM_INTEGER_MAX(rand, nfiles, rotate);
        do {
            uint32_t	index;
            RANDOM_INTEGER_MAX(rand, index, rotate + 1);
            const char	*name;
            char	buf[64];
            if (index == 0) {
                name = TEST_FILE;
            }
            else {
                snprintf(buf, sizeof(buf), "%s.%u", TEST_FILE, index);
                name = buf;
            }

            std::string	sname(name);
            filemap_iter_t	it(map.find(sname));
            if (it != map.end()) {
                continue;
            }

            int	fd(pfc_openat_cloexec(dirfd, name, O_RDWR|O_CREAT|O_TRUNC,
                                      mode));
            ASSERT_NE(-1, fd) << "error = " << strerror(errno);
            FdRef	fref(fd);
            ssize_t	sz(index + 1);
            ASSERT_EQ(sz, write(fd, characters, sz));
            if (index == rotate) {
                last1 = true;
            }
            else if (index == rotate - 1) {
                last2 = true;
            }
            if (index < rotate) {
                FILEMAP_SET_STAT(map, dirfd, TEST_FILE, index, index + 1);
            }
            mode++;
            cnt++;
        } while (cnt < nfiles);

        if (last1 && !last2) {
            FILEMAP_SET_STAT(map, dirfd, TEST_FILE, rotate, rotate);
        }

        // Perform file rotation.
        struct stat	sbuf;
        sbuf.st_size = 0;
        sbuf.st_mode = mode;
        sbuf.st_dev = 0;
        sbuf.st_ino = 0;
        filemap_set(map, TEST_FILE, 0, sbuf);

        int	fd(pfc_frotate_openat(dirfd, TEST_FILE, mode, 0, rotate));
        ASSERT_NE(-1, fd) << "error = " << strerror(errno);
        FdRef	fref(fd);
        FILEMAP_TEST(map, tdir.getPath(), dirfd);
    }
}

/*
 * Test case for error.
 */
TEST(frotate, error)
{
    Umask  umask;

    typedef struct {
        size_t		maxsize;
        uint32_t	rotate;
    } frotate_perm_t;

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    // Invalid argument.
    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, NULL, 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);

    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, "", 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);

    // Length of file name must be within 64 bytes.
    {
        TmpDir	tdir("_frotate");
        ASSERT_EQ(0, tdir.createDirectory());
        int	dirfd(open(tdir.getPath(), O_RDONLY));
        ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
        FdRef	dfd(dirfd);

        int	fd(pfc_frotate_openat(dirfd, TEST_LONG_FILE, 0644, 0, 0));
        ASSERT_NE(-1, fd) << "error = " << strerror(errno);
        ASSERT_EQ(0, close(fd));

        errno = 0;
        ASSERT_EQ(-1, pfc_frotate_openat(dirfd, TEST_TOO_LONG_FILE, 0644,
                                         0, 0));
        ASSERT_EQ(ENAMETOOLONG, errno);
    }

    // File name must not contain '/'.
    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, "/", 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);
    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, "/foo", 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);
    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, "foo/", 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);
    errno = 0;
    ASSERT_EQ(-1, pfc_frotate_openat(PFC_AT_FDCWD, "foo/bar", 0644, 0, 0));
    ASSERT_EQ(EINVAL, errno);

    frotate_perm_t	parameters[] = {
        {0, 0},
        {0, 10},
        {10, 0},
        {10, 10},
    };

    // Directory file descriptor is already closed.
    {
        int	fd(open("/dev/null", O_RDWR));

        ASSERT_NE(-1, fd) << "error = " << strerror(errno);
        ASSERT_EQ(0, close(fd));

        for (frotate_perm_t *param(parameters);
             param < PFC_ARRAY_LIMIT(parameters); param++) {
            errno = 0;
            ASSERT_EQ(-1, pfc_frotate_openat(fd, TEST_FILE, 0644,
                                             param->maxsize, param->rotate))
                << "maxsize=" << param->maxsize
                << ", rotate=" << param->rotate;
            ASSERT_EQ(EBADF, errno);
        }
    }

    // Not directory file descriptor is specified.
    {
        TmpFile	tmpf("frotate_test");

        ASSERT_EQ(0, tmpf.createFile());
        int	fd(tmpf.getDescriptor());
        ASSERT_NE(-1, fd) << "error = " << strerror(errno);

        for (frotate_perm_t *param(parameters);
             param < PFC_ARRAY_LIMIT(parameters); param++) {
            errno = 0;
            ASSERT_EQ(-1, pfc_frotate_openat(fd, TEST_FILE, 0644,
                                             param->maxsize, param->rotate))
                << "maxsize=" << param->maxsize
                << ", rotate=" << param->rotate;
            ASSERT_EQ(ENOTDIR, errno);
        }
    }

    // Parent directory is read only.
    if (!privileged) {
        TmpDir	tdir("_frotate");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*parent(tdir.getPath());
        int	dirfd(open(parent, O_RDONLY));
        ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
        FdRef	dfd(dirfd);
        ASSERT_EQ(0, fchmod(dirfd, 0555)) << "error = " << strerror(errno);

        for (frotate_perm_t *param(parameters);
             param < PFC_ARRAY_LIMIT(parameters); param++) {
            errno = 0;
            ASSERT_EQ(-1, pfc_frotate_openat(dirfd, TEST_FILE, 0644,
                                             param->maxsize, param->rotate))
                << "maxsize=" << param->maxsize
                << ", rotate=" << param->rotate;
            ASSERT_EQ(EACCES, errno);
        }

        ASSERT_EQ(0, fchmod(dirfd, 0444)) << "error = " << strerror(errno);

        for (frotate_perm_t *param(parameters);
             param < PFC_ARRAY_LIMIT(parameters); param++) {
            errno = 0;
            ASSERT_EQ(-1, pfc_frotate_openat(dirfd, TEST_FILE, 0644,
                                             param->maxsize, param->rotate))
                << "maxsize=" << param->maxsize
                << ", rotate=" << param->rotate;
            ASSERT_EQ(EACCES, errno);
        }
    }

    // The target file is not regular file.
    {
        TmpDir	tdir("_frotate");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*parent(tdir.getPath());
        int	dirfd(open(parent, O_RDONLY));
        ASSERT_NE(-1, dirfd) << "error = " << strerror(errno);
        FdRef	dfd(dirfd);
        ASSERT_EQ(0, symlinkat("__not_exist__", dirfd, TEST_FILE))
            << "error = " << strerror(errno);

        errno = 0;
        ASSERT_EQ(-1, pfc_frotate_openat(dirfd, TEST_FILE, 0644, 10, 0));
        ASSERT_EQ(EPERM, errno);
    }
}
