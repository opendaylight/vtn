/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for miscellaneous utilities.
 */

#ifndef	_TEST_MISC_HH
#define	_TEST_MISC_HH

#include <gtest/gtest.h>
#include <cstdio>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <list>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <pfc/clock.h>
#include <pfc/util.h>

/*
 * User name map.
 */
typedef std::map<uid_t, std::string>   usermap_t;
typedef std::map<std::string, uid_t>   uidmap_t;

/*
 * Group name map.
 */
typedef std::map<gid_t, std::string>   groupmap_t;
typedef std::map<std::string, gid_t>   gidmap_t;

/*
 * Set of group IDs.
 */
typedef std::set<gid_t>                gidset_t;

/*
 * String list.
 */
typedef std::list<std::string>         strlist_t;

/*
 * Temporary buffer.
 * Allocated buffer will be released by destructor.
 */
class TmpBuffer
{
public:
    TmpBuffer(size_t size);
    ~TmpBuffer();

    void	*getAsMuch(void);

    inline size_t
    getSize(void) const
    {
        return _size;
    }

    inline void *
    operator*() const
    {
        return _buffer;
    }

private:
    size_t	_size;
    void	*_buffer;
};

/*
 * Set actual buffer pointer to `ptr', and check result using Google Test's
 * assertion macro.
 */
#define	TMPBUF_ASSERT(tbuf, ptr, type)				\
	do {							\
		(ptr) = reinterpret_cast<type>(*(tbuf));	\
		ASSERT_TRUE((ptr) != NULL);			\
	} while (0)

/*
 * Container class to keep reference to malloc'ed buffer.
 */
class MallocRef
{
public:
    explicit MallocRef(void *buf) : _buffer(buf) {}

    ~MallocRef()
    {
        free(_buffer);
    }

private:
    void   *_buffer;
};

/*
 * Container class to keep reference to a file descriptor.
 * It will be closed by destructor.
 */
class FdRef
{
public:
    FdRef() : _fd(-1), _fdp(NULL) {}
    explicit FdRef(int fd) : _fd(fd), _fdp(NULL) {}

    /*
     * This constructor takes a pointer to file descriptor.
     * Destructor will close if it points valid file descriptor.
     */
    explicit FdRef(int *fdp) : _fd(-1), _fdp(fdp) {}

    ~FdRef()
    {
        if (_fd != -1) {
            (void)close(_fd);
        }
        if (_fdp != NULL) {
            int	f(*_fdp);

            if (f != -1) {
                (void)close(f);
            }
        }
    }

    inline int
    get(void) const
    {
        return _fd;
    }

    inline void
    set(int fd)
    {
        _fd = fd;
    }

private:
    int	_fd;
    int	*_fdp;
};

/*
 * Change current working directory.
 * It will be restored by destructor.
 */
class ChDir
{
public:
    ChDir(const char *path) : _fd(open(".", O_RDONLY)), _error(0)
    {
        if (_fd == -1 || chdir(path) != 0) {
            _error = errno;
        }
    }

    ~ChDir()
    {
        if (_fd != -1) {
            (void)fchdir(_fd);
            (void)close(_fd);
        }
    }

    int
    getError(void) const
    {
        return _error;
    }

private:
    int	_fd;
    int	_error;
};

/*
 * Container class to keep reference to a stdio stream.
 * It will be closed by destructor.
 */
class StdioRef
{
public:
    StdioRef(FILE *fp) : _fp(fp) {}
    StdioRef(const char *file, const char *mode) : _fp(fopen(file, mode)) {}

    FILE *
    operator*() const
    {
        return _fp;
    }

    ~StdioRef()
    {
        if (_fp != NULL) {
            fclose(_fp);
        }
    }
private:
    FILE	*_fp;
};

/*
 * Container class to keep reference to a directory handle.
 * It will be closed by destructor.
 */
class DirEntRef
{
public:
    DirEntRef(DIR *dirp) : _dirp(dirp) {}

    DIR *
    operator*() const
    {
        return _dirp;
    }

    ~DirEntRef()
    {
        if (_dirp != NULL) {
            closedir(_dirp);
        }
    }
private:
    DIR		*_dirp;
};

/*
 * Container class to keep reference to a file.
 * The file will be unlinked by destructor.
 */
class PathRef
{
public:
    PathRef() {}
    PathRef(const char *path) : _path(path) {}

    ~PathRef()
    {
        if (!_path.empty()) {
            (void)pfc_rmpath(_path.c_str());
        }
    }

    inline const char *
    operator*() const
    {
        return (_path.empty()) ? NULL : _path.c_str();
    }

    inline const PathRef &
    operator=(const char *path)
    {
        if (path == NULL || *path == '\0') {
            _path.erase();
        }
        else {
            _path.assign(path);
        }

        return *this;
    }

protected:
    std::string  _path;
};

/*
 * Helper class to construct argument vector.
 */
class ArgVector
{
public:
    ArgVector();
    ~ArgVector();
    bool	push(const char *arg);
    bool	push(int arg);

    inline char **
    operator*() const
    {
        return _argv;
    }

private:
    bool	expand(size_t newsize);
    bool	pushArgument(char *arg);
    char	**_argv;
    size_t	_capacity;
    size_t	_size;
};

/*
 * Compiled regular expression.
 */
class RegEx
{
public:
    RegEx(const char *pattern)
    {
        compile(pattern);
    }

    ~RegEx()
    {
        regfree(&_reg);
    }

    inline bool
    search(const char *text)
    {
        int  ret(regexec(&_reg, text, 0, NULL, 0));

        return (ret == 0) ? true : false;
    }

private:
    inline void
    compile(const char *pattern)
    {
        int ret(regcomp(&_reg, pattern, REG_EXTENDED | REG_NOSUB));

        if (PFC_EXPECT_FALSE(ret != 0)) {
            char  buf[128];

            regerror(ret, &_reg, buf, sizeof(buf));
            FAIL() << "regcomp(" << pattern << ") failed: " << buf;
        }
    }

    regex_t   _reg;       // compiled regular expression
};

/*
 * Throw runtime_error.
 */
#define	THROW_ERRNO(msg, err)                                           \
    do {                                                                \
        std::ostringstream	__stream;                               \
        __stream << __FILE__ << ":" << __LINE__ << ": " << (msg);       \
        if ((err) != 0) {                                               \
            __stream << ": " << strerror(err);                          \
        }                                                               \
        throw std::runtime_error(__stream.str());                       \
    } while (0)

#define	THROW_MESSAGE(msg)	THROW_ERRNO(msg, 0)

/*
 * Assertions which uses std::runtime_error.
 */
extern void	ex_assert(int required, int value, const char *stmt,
                          bool result, const char *file, int line)
    throw(std::runtime_error);
extern void	ex_assert(unsigned int required, unsigned int value,
                          const char *stmt, bool result, const char *file,
                          int line)
    throw(std::runtime_error);
extern void	ex_assert(long required, long value, const char *stmt,
                          bool result, const char *file, int line)
    throw(std::runtime_error);
extern void	ex_assert(unsigned long required, unsigned long value,
                          const char *stmt, bool result, const char *file,
                          int line)
    throw(std::runtime_error);
extern void	ex_assert(long long required, long long value, const char *stmt,
                          bool result, const char *file, int line)
    throw(std::runtime_error);
extern void	ex_assert(unsigned long long required, unsigned long long value,
                          const char *stmt, bool result, const char *file,
                          int line)
    throw(std::runtime_error);
extern void	ex_assert(void *required, void *value, const char *stmt,
                          bool result, const char *file, int line)
    throw(std::runtime_error);
extern void	ex_assert_bool(const char *stmt, bool required,
                               bool value, const char *file, int line)
    throw(std::runtime_error);

#define	EXASSERT_EQ(required, value)                                    \
    ex_assert((required), (value), #required " == " #value, true,       \
              __FILE__, __LINE__)

#define	EXASSERT_NE(required, value)                                    \
    ex_assert((required), (value), #required " != " #value, false,      \
              __FILE__, __LINE__)

#define	EXASSERT_TRUE(value)                                    \
    ex_assert_bool(#value, true, (value) ? true : false,        \
                   __FILE__, __LINE__)

#define	EXASSERT_FALSE(value)                                    \
    ex_assert_bool(#value, false, (value) ? true : false,        \
                   __FILE__, __LINE__)

/*
 * All file permission bits, including sticky bit.
 */
#define	MODE_ALLPERMS		(07777)

/*
 * Return file permission bits in mode_t.
 */
#define	MODE_GETPERMS(mode)	((mode) & MODE_ALLPERMS)

/*
 * True if the specified pfc_timespec_t keeps the specified second period
 * with considering error.
 */
#define	TIMESPEC_EQUALS(tsp, req, error)                                \
    (((tsp)->tv_sec == (req) && (tsp)->tv_nsec <= (error)) ||           \
     ((req) != 0 && (tsp)->tv_sec == (req) - 1 && (tsp)->tv_nsec >= (error)))

/*
 * Prototypes.
 */
extern void     stringify(gidset_t &set, std::string &str);

/*
 * Ensure that the given two gidset_t are identical.
 * This funciton is needed for STL implementation that does not provide
 * '<<' operator of std::set.
 */
#define ASSERT_GIDSET_EQ(expected, set)         \
    do {                                        \
        if ((expected) != (set)) {              \
            std::string  __estr, __str;         \
            stringify((expected), __estr);      \
            stringify((set), __str);            \
            ASSERT_EQ(__estr, __str);           \
        }                                       \
    } while (0)

#endif	/* !_TEST_MISC_HH */
