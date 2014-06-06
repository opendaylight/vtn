/*
 * Copyright (c) 2011-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_mkdir.cc - Test for pfc_mkdir().
 */

#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <pfc/util.h>
#include "test.h"
#include "tmpfile.hh"

#define	PERM_MASK		07777
#define	TEST_PATH_MAX		1024

/*
 * Successful test using relative path.
 */
TEST(mkdir, rel)
{
    Umask	umask;
    mode_t	perms[] = {0777, 0755, 0750, 01777};
    mode_t	*pp;
    for (pp = perms; pp < PFC_ARRAY_LIMIT(perms); pp++) {
        mode_t	perm(*pp);
        TmpDir	tdir("_mkdir");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        ASSERT_EQ(0, rmdir(base));

        ASSERT_EQ(0, pfc_mkdir(base, perm));

        struct stat	sbuf;
        ASSERT_EQ(0, lstat(base, &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(base, perm));
        ASSERT_EQ(0, lstat(base, &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        std::string	path(base);
        path += "/dir1";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        path += "/dir2/dir3/";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        path = "_mkdir_1";
        TmpDir	tdir1(path.c_str(), true);
        path += "/dir1/dir2/dir3/dir4/dir5/dir6";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);
    }
}

/*
 * Successful test using absolute path.
 */
TEST(mkdir, abs)
{
    Umask	umask;
    char	cwd[TEST_PATH_MAX];
    ASSERT_NE(reinterpret_cast<char *>(NULL), getcwd(cwd, sizeof(cwd)));

    mode_t	perms[] = {0777, 0755, 0750, 01777};
    mode_t	*pp;
    for (pp = perms; pp < PFC_ARRAY_LIMIT(perms); pp++) {
        mode_t	perm(*pp);
        std::string	path(cwd);
        path += "/_mkdir";
        TmpDir	tdir(path.c_str());
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());
        ASSERT_EQ(0, rmdir(base));

        ASSERT_EQ(0, pfc_mkdir(base, perm));

        struct stat	sbuf;
        ASSERT_EQ(0, lstat(base, &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(base, 0777));
        ASSERT_EQ(0, lstat(base, &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(static_cast<mode_t>(0777), sbuf.st_mode & PERM_MASK);

        path = base;
        path += "/dir1";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        path += "/dir2/dir3/";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);

        path = cwd;
        path += "/_mkdir_1";
        TmpDir	tdir1(path.c_str(), true);
        path += "/dir1/dir2/dir3/dir4/dir5/dir6";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), perm));
        ASSERT_EQ(0, lstat(path.c_str(), &sbuf));
        ASSERT_TRUE(S_ISDIR(sbuf.st_mode));
        ASSERT_EQ(perm, sbuf.st_mode & PERM_MASK);
    }
}

/*
 * Error test.
 */
TEST(mkdir, error)
{
    Umask umask;

    ASSERT_EQ(EINVAL, pfc_mkdir(NULL, 0755));
    ASSERT_EQ(EINVAL, pfc_mkdir("", 0755));

    bool privileged;
    has_fs_capability(privileged);
    RETURN_ON_ERROR();

    int eacces((privileged) ? 0 : EACCES);

    // EACCES test.
    {
        TmpDir	tdir("_mkdir");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	path(base);
        path += "/dir1";
        ASSERT_EQ(0, pfc_mkdir(path.c_str(), 0500));

        path += "/dir2/dir3";
        ASSERT_EQ(eacces, pfc_mkdir(path.c_str(), 0755));
    }

    // EEXIST and ENOTDIR test.
    {
        TmpDir	tdir("_mkdir");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	path(base);
        path += "/dir1";
        int	fd(open(path.c_str(), O_RDWR | O_CREAT, 0644));
        ASSERT_NE(-1, fd);
        (void)close(fd);

        ASSERT_EQ(EEXIST, pfc_mkdir(path.c_str(), 0755));

        path += "/dir2";
        ASSERT_EQ(ENOTDIR, pfc_mkdir(path.c_str(), 0755));
        path += "/dir3";
        ASSERT_EQ(ENOTDIR, pfc_mkdir(path.c_str(), 0755));
    }

    // EEXIST on symbolic link test.
    {
        TmpDir	tdir("_mkdir");
        ASSERT_EQ(0, tdir.createDirectory());
        const char	*base(tdir.getPath());

        std::string	path(base);
        path += "/a";
        ASSERT_EQ(0, symlink("b", path.c_str()));
        ASSERT_EQ(EEXIST, pfc_mkdir(path.c_str(), 0755));

        path = base;
        path += "/b";
        ASSERT_EQ(0, symlink("a", path.c_str()));
        ASSERT_EQ(EEXIST, pfc_mkdir(path.c_str(), 0755));
    }
}
