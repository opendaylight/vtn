/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tmpfile.cc - Temporary file.
 */

#include <cerrno>
#include <cstdarg>
#include <sstream>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include "tmpfile.hh"
#include "misc.hh"
#include "test.h"

/*
 * Permission bits for a temporary file.
 */
#define	TMPFILE_PERM		0600

/*
 * Permission bits for a temporary directory.
 */
#define	TMPDIR_PERM		0700

/*
 * Internal buffer size.
 */
#define	TMPFILE_BUFSIZE		128

/*
 * Internal prototypes.
 */
static void   is_writable(const char *path, struct stat &root, bool &writable);

/*
 * TmpFile::TmpFile(const char *base)
 *	Constructor of temporary file instance.
 *	`base' is used as basename of temporary file path.
 *	An integer suffix will be appended to the real file path.
 */
TmpFile::TmpFile(const char *base)
    : _path(base), _fp(NULL)
{
    PFC_MUTEX_INIT(&_mutex);
}

/*
 * TmpFile::~TmpFile()
 *	Destructor of temporary file instance.
 */
TmpFile::~TmpFile()
{
    if (_fp != NULL) {
        (void)fclose(_fp);
        (void)unlink(_path.c_str());
    }
}

/*
 * int
 * TmpFile::createFile(void)
 *	Create a temporary file.
 *	Created file will be removed by destructor.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpFile::createFile(void)
{
    pfc_mutex_lock(&_mutex);

    const int	oflags(O_CREAT | O_EXCL | O_TRUNC | O_RDWR);
    int	err(0);
    if (_fp != NULL) {
        // Already created.
        err = EEXIST;
        goto out;
    }

    int	fd;
    for (int index(0); true; index++) {
        std::ostringstream stream;

        stream << _path << "." << index;
        std::string	p(stream.str());
        fd = open(p.c_str(), oflags, TMPFILE_PERM);
        if (fd != -1) {
            _path = p;
            err = 0;
            break;
        }

        err = errno;
        if (err != EEXIST) {
            goto out;
        }
    }

    errno = 0;
    _fp = fdopen(fd, "r+");
    if (_fp == NULL) {
        err = errno;
        if (err == 0) {
            err = ENOMEM;
        }
        (void)close(fd);
        (void)unlink(_path.c_str());
    }

out:
    pfc_mutex_unlock(&_mutex);

    return err;
}

/*
 * void
 * TmpFile::print(const char *fmt, ...)
 *	Print out data to the temporary file using printf() format.
 */
void
TmpFile::print(const char *fmt, ...)
{
    va_list	ap;

    pfc_mutex_lock(&_mutex);
    if (_fp != NULL) {
        va_start(ap, fmt);
        vfprintf(_fp, fmt, ap);
        va_end(ap);
    }
    pfc_mutex_unlock(&_mutex);
}

/*
 * void
 * TmpFile::flush(void)
 *	Flush file stream associated to the temporary file.
 */
void
TmpFile::flush(void)
{
    pfc_mutex_lock(&_mutex);
    if (_fp != NULL) {
        fflush(_fp);
    }
    pfc_mutex_unlock(&_mutex);
}

/*
 * int
 * TmpFile::getSize(size_t &size)
 *	Obtain size of the temporary file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpFile::getSize(size_t &size)
{
    pfc_mutex_lock(&_mutex);

    int	err;
    if (_fp == NULL) {
        err = EINVAL;
    }
    else {
        struct stat	sbuf;

        if (fstat(fileno(_fp), &sbuf) == -1) {
            err = errno; 
        }
        else {
            err = 0;
            size = sbuf.st_size;
        }
    }

    pfc_mutex_unlock(&_mutex);

    return err;
}

/*
 * int
 * TmpFile::readAsString(std::string &str)
 *	Read contents of the temporary file as single string.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpFile::readAsString(std::string &str)
{
    pfc_mutex_lock(&_mutex);

    int	err(0);
    if (_fp != NULL) {
        char	buf[TMPFILE_BUFSIZE];
        int	fd(fileno(_fp));
        off_t	off(lseek(fd, 0, SEEK_CUR));

        if (off == -1) {
            err = errno;
            goto out;
        }

        if (lseek(fd, 0, SEEK_SET) == -1) {
            err = errno;
            (void)fseek(_fp, off, SEEK_SET);
            goto out;
        }

        for (;;) {
            ssize_t	sz(read(fd, buf, sizeof(buf)));

            if (sz == -1) {
                err = errno;
                break;
            }
            if (sz == 0) {
                break;
            }
            str.append(buf, sz);
        }
        (void)fseek(_fp, off, SEEK_SET);
    }

out:
    pfc_mutex_unlock(&_mutex);

    return err;
}

/*
 * int
 * TmpFile::getDescriptor(void) const
 *	Return file descriptor associated with the temporary file.
 *	-1 is returned if a temporary file is not yet created.
 */
int
TmpFile::getDescriptor(void) const
{
    return (_fp == NULL) ? -1 : fileno(_fp);
}

/*
 * const char *
 * TmpFile::getPath(void) const
 *	Return a pointer to a temporary file path.
 *	NULL is returned if a temporary file is not yet created.
 */
const char *
TmpFile::getPath(void) const
{
    return (_fp == NULL) ? NULL : _path.c_str();
}

/*
 * void
 * TmpFile::search(const char *pattern, bool &found)
 *      Search for a pattern in the temporary file.
 *      true is set to `found' if the pattern is found.
 */
void
TmpFile::search(const char *pattern, bool &found)
{
    found = false;

    FILE  *fp(_fp);
    ASSERT_TRUE(fp != NULL);
    rewind(fp);

    char   line[1024];
    RegEx  regex(pattern);
    RETURN_ON_ERROR();

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (regex.search(line)) {
            found = true;
            break;
        }
    }

    rewind(fp);
}

/*
 * TmpDir::TmpDir(const char *base, bool setpath)
 *	Constructor of temporary directory instance.
 *
 *	If `setpath' is false, `base' is used as basename of temporary
 *	directory path. An integer suffix will be appended to the real file
 *	path. Created directory is guaranted that it contains no file.
 *
 *	If `setpath' is true, `base' is treated as temporary directory itself.
 *	It's useful to clean up existing directory.
 *
 * Remarks:
 *	This class doesn't create parent directories contained in `base'.
 */
TmpDir::TmpDir(const char *base, bool setpath)
    : _base(base)
{
    if (setpath) {
        _path = base;
    }
}

/*
 * TmpDir::~TmpDir()
 *	Destructor of temporary directory instance.
 */
TmpDir::~TmpDir()
{
    (void)cleanup();
}

/*
 * int
 * TmpDir::createDirectory(void)
 *	Create a temporary directory.
 *	Created directory will be removed by destructor.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpDir::createDirectory(void)
{
    if (_path.size() != 0) {
        // Already created.
        return EEXIST;
    }

    int	err(0);
    for (int index(0); true; index++) {
        std::ostringstream stream;

        stream << _base << "." << index;
        std::string	p(stream.str());

        if (mkdir(p.c_str(), TMPDIR_PERM) == 0) {
            _path = p;
            err = 0;
            break;
        }

        err = errno;
        if (err != EEXIST) {
            break;
        }
    }

    return err;
}

/*
 * const char *
 * TmpDir::getPath(void) const
 *	Return a pointer to a temporary directory path.
 *	NULL is returned if a temporary directory is not yet created.
 */
const char *
TmpDir::getPath(void) const
{
    const char	*path(_path.c_str());

    return (*path == '\0') ? NULL : path;
}

/*
 * int
 * TmpDir::cleanup(void)
 *	Clean up all files under the temporary directory associated with
 *	this instance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpDir::cleanup(void)
{
    if (_path.size() == 0) {
        // createDirectory() is not yet called.
        return 0;
    }

    TmpBuffer	tbuf(sizeof(struct dirent));
    struct dirent	*buf(reinterpret_cast<struct dirent *>(*tbuf));

    if (buf == NULL) {
        return ENOMEM;
    }

    return cleanup(_path, buf);
}

/*
 * int
 * TmpDir::cleanup(std::string dir, struct dirent *buf)
 *	Clean up all files under the specified directory.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
TmpDir::cleanup(std::string dir, struct dirent *buf)
{
    const char	*dpath(dir.c_str());

    // Ensure that we can modify the specified directory.
    if (chmod(dpath, TMPDIR_PERM) == -1) {
        int	err(errno);

        if (err == ENOENT) {
            err = 0;
        }

        return err;
    }

    DIR	*dirp(opendir(dpath));
    if (dirp == NULL) {
        int	err(errno);

        if (err == ENOENT) {
            err = 0;
        }

        return err;
    }

    int	err(0);
    struct dirent	*dp;
    std::list<std::string>	dirlist;

    while (readdir_r(dirp, buf, &dp) == 0) {
        char	*name(dp->d_name);

        if (dp == NULL) {
            break;
        }
        if (*name == '.' &&
            (*(name + 1) == '\0' ||
             (*(name + 1) == '.' && *(name + 2) == '\0'))) {
            continue;
        }

        std::string	path = dir + "/" + name;
        const char	*p(path.c_str());
        struct stat	sbuf;
        if (lstat(p, &sbuf) == -1) {
            err = errno;
            break;
        }
        if (S_ISDIR(sbuf.st_mode)) {
            dirlist.push_back(path);
        }
        else if (unlink(p) != 0) {
            err = errno;
            break;
        }
    }

    closedir(dirp);

    if (err == 0) {
        for (std::list<std::string>::iterator it(dirlist.begin());
             it != dirlist.end(); it++) {
            std::string	d(*it);

            err = cleanup(d, buf);
            if (err != 0) {
                break;
            }
	}

        if (err == 0 && rmdir(dpath) != 0) {
            err = errno;
        }
    }

    return err;
}

/*
 * void
 * RenameFile::rename(const char *path)
 *      Rename the specified file to temporary name.
 */
void
RenameFile::rename(const char *path)
{
    if (_renamed) {
        // Already renamed.
        return;
    }

    // Determine parent directory path and file name.
    uint32_t   len(strlen(path));
    const char *p;
    for (p = path + len - 1; p >= path; p--) {
        if (*p == '/') {
            break;
        }
    }

    std::string  parent;
    if (p < path) {
        parent = ".";
        _fname = path;
    }
    else if (p == path) {
        parent = "/";
        _fname = path;
    }
    else {
        parent = std::string(path, p - path);
        _fname = std::string(p + 1); 
    }

    static uint32_t  fname_index;
    uint32_t index(pfc_atomic_inc_uint32_old(&fname_index));

    char  buf[128];
    snprintf(buf, sizeof(buf), "_renamed_%u.%u", getpid(), index);
    _saved = buf;

    _dfd = open(parent.c_str(), O_RDONLY);
    ASSERT_NE(-1, _dfd) << "*** ERROR: " << strerror(errno);

    // Determine whether the specified file exists or not.
    int ret(access(path, F_OK));
    if (ret == -1) {
        ASSERT_EQ(ENOENT, errno);
        _renamed = true;
        _saved.clear();
        return;
    }

    ASSERT_EQ(0, renameat(_dfd, _fname.c_str(), _dfd, _saved.c_str()))
        << "*** ERROR: " << strerror(errno);

    _renamed = true;
}

/*
 * RenameFile::~RenameFile()
 *      Restore renamed file.
 */
RenameFile::~RenameFile()
{
    if (_renamed) {
        const char  *fname(_fname.c_str());
        (void)pfc_rmpathat(_dfd, fname);
        if (_saved.size() != 0) {
            (void)renameat(_dfd, _saved.c_str(), _dfd, fname);
        }
    }

    if (_dfd != -1) {
        (void)close(_dfd);
    }
}

/*
 * void
 * has_fs_capability(bool &privileged)
 *      Determine whether the current process has DAC_OVERRIDE capability
 *      or not.
 */
void
has_fs_capability(bool &privileged)
{
    static int  cap(-1);
    if (cap >= 0) {
        privileged = (cap != 0);
        return;
    }

    privileged = false;
    TmpFile tmpf("rmpath_test_cap");
    ASSERT_EQ(0, tmpf.createFile());
    int fd(tmpf.getDescriptor());
    ASSERT_NE(-1, fd);
    ASSERT_EQ(0, fchmod(fd, 0400)) << "*** ERROR: " << strerror(errno);

    fd = open(tmpf.getPath(), O_RDWR);
    if (fd != -1) {
        ASSERT_EQ(0, close(fd));
        cap = 1;
        privileged = true;
    }
    else {
        ASSERT_EQ(EACCES, errno);
        cap = 0;
    }
}

/*
 * void
 * is_testdir_safe(bool &safe)
 *      Determine whether the test directory is located at the safe path.
 */
void
is_testdir_safe(bool &safe)
{
    static int  result(-1);
    if (result >= 0) {
        safe = (result != 0);
        return;
    }

    safe = false;

    // Ensure that the root directory is safe.
    struct stat root;
    ASSERT_EQ(0, stat("/", &root));
    if (root.st_mode & (S_IWGRP | S_IWOTH)) {
        result = 0;
        return;
    }

    // Ensure the current directory and parent directories are safe.
    bool writable;
    is_writable(".", root, writable);
    RETURN_ON_ERROR();
    if (writable) {
        result = 0;
        return;
    }

    // Ensure object directory and parent directories are safe.
    is_writable(OBJDIR, root, writable);
    RETURN_ON_ERROR();
    if (writable) {
        result = 0;
        return;
    }

    result = 1;
    safe = true;
}

/*
 * static void
 * is_writable(const char *path, struct stat &root, bool &writable)
 *      Determine whether the specified path is group or world writable
 *      or not.
 */
static void
is_writable(const char *path, struct stat &root, bool &writable)
{
    std::string spath(path);
    struct stat sbuf;

    writable = false;
    for (;;) {
        ASSERT_EQ(0, stat(spath.c_str(), &sbuf));
        if (sbuf.st_dev == root.st_dev && sbuf.st_ino == root.st_ino) {
            break;
        }

        if (sbuf.st_mode & (S_IWGRP | S_IWOTH)) {
            writable = true;
            return;
        }
        spath.append("/..");
    }
}
