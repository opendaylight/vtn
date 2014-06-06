/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_rmpath.cc - Test for pfc_rmpath() and pfc_rmpathat().
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <pfc/util.h>
#include "test.h"
#include "tmpfile.hh"
#include "misc.hh"

#define	RMPATH_TEST_DEPTH	4
#define	RMPATH_PATH_MAX		1024

static int
create_file(const char *dir, const char *name, mode_t perm=0644)
{
    std::string path = std::string(dir) + "/" + name;
    int	fd(open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, perm));
    if (fd == -1) {
        return errno;
    }
    (void)close(fd);

    return 0;
}

static int
create_symlink(const char *dir, const char *name)
{
    std::string	path = std::string(dir) + "/" + name;
    if (symlink("/foo/bar/not_exist", path.c_str()) != 0) {
        return errno;
    }

    return 0;
}

static int
create_pipe(const char *dir, const char *name)
{
    std::string	path = std::string(dir) + "/" + name;
    if (mknod(path.c_str(), S_IFIFO | 0644, 0) != 0) {
        return errno;
    }

    return 0;
}

static int
create_socket(const char *dir, const char *name)
{
    std::string	path = std::string(dir) + "/" + name;
    if (mknod(path.c_str(), S_IFSOCK | 0644, 0) != 0) {
        return errno;
    }

    return 0;
}

static void
create_tree(const char *dir, uint32_t depth, uint32_t curdepth=0)
{
    ASSERT_EQ(0, create_file(dir, "a"));
    ASSERT_EQ(0, create_file(dir, "b"));
    ASSERT_EQ(0, create_symlink(dir, "c"));
    ASSERT_EQ(0, create_file(dir, "d"));
    ASSERT_EQ(0, create_symlink(dir, "e"));
    ASSERT_EQ(0, create_pipe(dir, "f"));
    ASSERT_EQ(0, create_socket(dir, "g"));

    if (depth > curdepth) {
        curdepth++;

        for (int i = 0; i < 5; i++) {
            char	dpath[RMPATH_PATH_MAX];

            snprintf(dpath, sizeof(dpath), "%s/dir%d", dir, i);
            ASSERT_EQ(0, mkdir(dpath, 0755))
                << "errno = " << errno << ": (" << strerror(errno) << ")";
            create_tree(dpath, depth, curdepth);
        }
    }
}

/*
 * pfc_rmpath() test
 * - Successful test using relative path.
 */
TEST(rmpath, rel)
{
    Umask umask;

    // Non recursive case.
    {
        TmpDir	tdir("_rmpath");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        struct stat	sb;

        ASSERT_EQ(0, create_file(base, "file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        std::string sdir(base);
        std::string path = sdir + "/file";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/symlink";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/pipe";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        // Directory should be empty. */
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
   }

    // Recursive case.
    for (int depth = 0; depth <= RMPATH_TEST_DEPTH; depth++) {
        TmpDir	tdir("_rmpath");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, depth, 0);
        if (HasFatalFailure()) {
            return;
        }

        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
    }
}

/*
 * pfc_rmpath() test
 * - Successful test using absolute path.
 */
TEST(rmpath, abs)
{
    Umask umask;

    char	cwd[RMPATH_PATH_MAX];
    ASSERT_NE(reinterpret_cast<char *>(NULL), getcwd(cwd, sizeof(cwd)));

   // Non recursive case.
    {
        std::string	cpath(cwd);
        cpath += "/_rmpath";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        struct stat	sb;

        ASSERT_EQ(0, create_file(base, "file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        std::string sdir(base);
        std::string path = sdir + "/file";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/symlink";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/pipe";
        ASSERT_EQ(0, pfc_rmpath(path.c_str()));

        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        // Directory should be empty. */
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
   }

    // Recursive case.
    for (int depth = 0; depth <= RMPATH_TEST_DEPTH; depth++) {
        std::string	cpath(cwd);
        cpath += "/_rmpath";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, depth, 0);
        if (HasFatalFailure()) {
            return;
        }

        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
    }
}

/*
 * pfc_rmpath() test
 * - Error test.
 */
TEST(rmpath, error)
{
    Umask umask;

    ASSERT_EQ(EFAULT, pfc_rmpath(NULL));
    ASSERT_EQ(ENOENT, pfc_rmpath(""));

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    int eacces((privileged) ? 0 : EACCES);

    // ENOENT test.
    {
        std::string	basedir;
        {
            TmpDir	tdir("_rmpath");
            ASSERT_EQ(0, tdir.createDirectory());
            const char	*base(tdir.getPath());
            basedir = base;

            std::string sdir(base);
            std::string path = sdir + "/file1";
            ASSERT_EQ(ENOENT, pfc_rmpath(path.c_str()));
            path = sdir + "/file2";
            ASSERT_EQ(ENOENT, pfc_rmpath(path.c_str()));
            path = sdir + "/dir/file";
            ASSERT_EQ(ENOENT, pfc_rmpath(path.c_str()));
        }

        ASSERT_EQ(ENOENT, pfc_rmpath(basedir.c_str()));
    }

    // EACCES test. (Directory containing the target file is read only)
    {
        TmpDir	tdir("_mkdir1");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        ASSERT_EQ(0, create_file(base, "file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        std::string sdir(base);
        ASSERT_EQ(0, chmod(sdir.c_str(), 0555));

        std::string path = sdir + "/file";
        ASSERT_EQ(eacces, pfc_rmpath(path.c_str()));

        path = sdir + "/symlink";
        ASSERT_EQ(eacces, pfc_rmpath(path.c_str()));

        path = sdir + "/pipe";
        ASSERT_EQ(eacces, pfc_rmpath(path.c_str()));

        ASSERT_EQ(0, chmod(sdir.c_str(), 0755));

        // Directory should be empty. */
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);

    }

    // EACCES test. (directory is read only)
    {
        TmpDir	tdir("_mkdir2");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0400))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpath(base));
    }

    // EACCES test. (sub directory is read only)
    {
        TmpDir	tdir("_mkdir3");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	subdir(base);
        subdir += "/dir";
        ASSERT_EQ(0, mkdir(subdir.c_str(), 0755));

        create_tree(subdir.c_str(), 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0555))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpath(base));
    }
}

/*
 * pfc_rmpathat() test
 * - Successful test using relative path.
 */
TEST(rmpath, at_rel)
{
    Umask umask;

    // Non recursive case.
    {
        TmpDir	tdir("_rmpath");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        int	dirfd(open(base, O_RDONLY));
        ASSERT_NE(-1, dirfd);
        FdRef	dfd(dirfd);
        struct stat	sb;

        ASSERT_EQ(0, create_file(base, "file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        ASSERT_EQ(0, pfc_rmpathat(dirfd, "file"));
        std::string sdir(base);
        std::string path = sdir + "/file";
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        ASSERT_EQ(0, pfc_rmpathat(dirfd, "symlink"));
        path = sdir + "/symlink";
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        ASSERT_EQ(0, pfc_rmpathat(dirfd, "pipe"));
        path = sdir + "/pipe";
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        // Directory should be empty. */
        const char	*dname(strrchr(base, '/'));
        if (dname == NULL) {
            dname = base;
        }
        else {
            dname++;
        }
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpathat(PFC_AT_FDCWD, dname));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
   }

    // Recursive case.
    TmpDir	testdir("_rmpathat");
    ASSERT_EQ(0, testdir.createDirectory());
    const char	*testbase(testdir.getPath());
    int	dirfd(open(testbase, O_RDONLY));
    ASSERT_NE(-1, dirfd);
    FdRef	dfd(dirfd);

    for (int depth = 0; depth <= RMPATH_TEST_DEPTH; depth++) {
        const char	*dname("test_dir");
        ASSERT_EQ(0, mkdirat(dirfd, dname, 0755));

        std::string	s(testbase);
        s += "/";
        s += dname;
        TmpDir	tdir(s.c_str(), true);
        const char	*base(tdir.getPath());

        create_tree(base, depth, 0);
        if (HasFatalFailure()) {
            return;
        }

        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpathat(dirfd, dname));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
    }
}

/*
 * pfc_rmpathat() test
 * - Successful test using absolute path.
 */
TEST(rmpath, at_abs)
{
    Umask umask;

    // Remarks:
    //     `dirfd' must be always ignored if an absolute path is specified.

    char	cwd[RMPATH_PATH_MAX];
    ASSERT_NE(reinterpret_cast<char *>(NULL), getcwd(cwd, sizeof(cwd)));

   // Non recursive case.
    {
        std::string	cpath(cwd);
        cpath += "/._rmpath";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        struct stat	sb;

        ASSERT_EQ(0, create_file(base, "..file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        std::string sdir(base);
        std::string path = sdir + "/..file";
        ASSERT_EQ(0, pfc_rmpathat(0, path.c_str()));
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/symlink";
        ASSERT_EQ(0, pfc_rmpathat(0, path.c_str()));
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        path = sdir + "/pipe";
        ASSERT_EQ(0, pfc_rmpathat(0, path.c_str()));
        ASSERT_EQ(-1, stat(path.c_str(), &sb));
        ASSERT_EQ(ENOENT, errno);

        // Directory should be empty. */
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpathat(10, base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
   }

    // Recursive case.
    for (int depth = 0; depth <= RMPATH_TEST_DEPTH; depth++) {
        std::string	cpath(cwd);
        cpath += "/_rmpath";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, depth, 0);
        if (HasFatalFailure()) {
            return;
        }

        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        if (is_under_valgrind()) {
            // openat(2) emulated by valgrind may cause EBADF error if -1 is
            // specified to dirfd, even absolute path is specified.
            ASSERT_EQ(0, pfc_rmpathat(AT_FDCWD, base));
        }
        else {
            ASSERT_EQ(0, pfc_rmpathat(-1, base));
        }
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
    }
}

/*
 * pfc_rmpathat() test
 * - Error test.
 */
TEST(rmpath, error_at)
{
    Umask umask;

    ASSERT_EQ(EFAULT, pfc_rmpathat(0, NULL));
    ASSERT_EQ(EBADF, pfc_rmpathat(-1, "tmp"));
    ASSERT_EQ(ENOENT, pfc_rmpathat(0, ""));

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    int eacces((privileged) ? 0 : EACCES);

    // ENOENT test.
    {
        TmpDir	tdir("_rmpath");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        int	dirfd(open(base, O_RDONLY));
        ASSERT_NE(-1, dirfd);
        FdRef	dfd(dirfd);

        ASSERT_EQ(ENOENT, pfc_rmpathat(dirfd, "file1"));
        ASSERT_EQ(ENOENT, pfc_rmpathat(0, "/file1"));
        ASSERT_EQ(ENOENT, pfc_rmpathat(dirfd, "dir/file"));
        ASSERT_EQ(ENOENT, pfc_rmpathat(0, "/dir/file"));
    }

    // EACCES test. (Directory containing the target file is read only)
    {
        TmpDir	tdir("_mkdir1");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        int	dirfd(open(base, O_RDONLY));
        ASSERT_NE(-1, dirfd);
        FdRef	dfd(dirfd);

        ASSERT_EQ(0, create_file(base, "file"));
        ASSERT_EQ(0, create_symlink(base, "symlink"));
        ASSERT_EQ(0, create_pipe(base, "pipe"));
        ASSERT_EQ(0, create_socket(base, "socket"));

        std::string sdir(base);
        ASSERT_EQ(0, chmod(sdir.c_str(), 0555));

        ASSERT_EQ(eacces, pfc_rmpathat(dirfd, "file"));
        ASSERT_EQ(eacces, pfc_rmpathat(dirfd, "symlink"));
        ASSERT_EQ(eacces, pfc_rmpathat(dirfd, "pipe"));

        ASSERT_EQ(0, chmod(sdir.c_str(), 0755));

        // Directory should be empty. */
        struct stat	sbuf;
        ASSERT_EQ(0, stat(base, &sbuf));
        ASSERT_EQ(0, pfc_rmpath(base));
        ASSERT_EQ(-1, stat(base, &sbuf));
        ASSERT_EQ(ENOENT, errno);
    }

    // EACCES test. (directory is read only) <using relative path>
    {
        TmpDir	tdir("_mkdir2");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0400))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpathat(PFC_AT_FDCWD, base));
    }

    // EACCES test. (directory is read only) <using absolute path>
    {
        char	cwd[RMPATH_PATH_MAX];
        ASSERT_NE(reinterpret_cast<char *>(NULL), getcwd(cwd, sizeof(cwd)));
        std::string	cpath(cwd);
        cpath += "/_mkdir3";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        create_tree(base, 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0400))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpathat(0, base));
    }

    // EACCES test. (sub directory is read only) [using relative path]
    {
        TmpDir	tdir("_mkdir4");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	subdir(base);
        subdir += "/dir";
        ASSERT_EQ(0, mkdir(subdir.c_str(), 0755));

        create_tree(subdir.c_str(), 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0555))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpathat(PFC_AT_FDCWD, base));
    }
    // EACCES test. (sub directory is read only) [using absolute path]
    {
        char	cwd[RMPATH_PATH_MAX];
        ASSERT_NE(reinterpret_cast<char *>(NULL), getcwd(cwd, sizeof(cwd)));
        std::string	cpath(cwd);
        cpath += "/_mkdir5";
        TmpDir	tdir(cpath.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	subdir(base);
        subdir += "/dir";
        ASSERT_EQ(0, mkdir(subdir.c_str(), 0755));

        create_tree(subdir.c_str(), 0, 0);
        if (HasFatalFailure()) {
            return;
        }

        ASSERT_EQ(0, chmod(base, 0555))
            << "chmod() failed: errno = "
            << errno << " (" << strerror(errno) << ")";

        ASSERT_EQ(eacces, pfc_rmpathat(0, base));
    }
}
