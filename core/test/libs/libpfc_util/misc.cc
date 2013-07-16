/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * misc.cc - Miscellaneous utilities.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pfc/base.h>
#include "misc.hh"

#define	ARGV_DELTA		8U
#define	ARGV_INT_SIZE		32U

#define	EXASSERT_BUFSIZE	128

/*
 * TmpBuffer::TmpBuffer(size_t size)
 *	Constructor of temporary buffer of the specified size.
 */
TmpBuffer::TmpBuffer(size_t size)
    : _size(size), _buffer(malloc(size))
{
}

/*
 * TmpBuffer::~TmpBuffer()
 *	Destructor of temporary buffer.
 */
TmpBuffer::~TmpBuffer()
{
    free(_buffer);
}

/*
 * void *
 * TmpBuffer::getAsMuch()
 *	Try to allocate buffer as much as possible.
 */
void *
TmpBuffer::getAsMuch()
{
    void	*ret(_buffer);

    if (ret == NULL) {
        for (size_t sz(_size); sz > 0; sz >>= 1) {
            ret = malloc(sz);
            if (ret != NULL) {
                _size = sz;
                _buffer = ret;
                break;
            }
        }
    }

    return ret;
}

/*
 * ArgVector::ArgVector()
 *	Construct argument vector instance.
 */
ArgVector::ArgVector()
    : _argv(NULL), _capacity(0), _size(0)
{
}

/*
 * ArgVector::~ArgVector()
 *	Destructor of argument vector.
 */
ArgVector::~ArgVector()
{
    for (size_t i(0); i < _size; i++) {
        free(reinterpret_cast<void *>(*(_argv + i)));
    }

    free(reinterpret_cast<void *>(_argv));
}

/*
 * bool
 * ArgVector::push(const char *arg)
 *	Push a string argument to the vector.
 *
 * Calling/Exit State:
 *	True is returned on success, false on failure.
 */
bool
ArgVector::push(const char *arg)
{
    char	*str(strdup(arg));

    if (str == NULL) {
        return false;
    }

    bool	ret(pushArgument(str));
    if (!ret) {
        free(str);
    }

    return ret;
}

/*
 * bool
 * ArgVector::push(int arg)
 *	Push an integer argument to the vector.
 *
 * Calling/Exit State:
 *	True is returned on success, false on failure.
 */
bool
ArgVector::push(int arg)
{
    char	*ptr(reinterpret_cast<char *>(malloc(ARGV_INT_SIZE)));

    if (ptr == NULL) {
        return false;
    }

    snprintf(ptr, ARGV_INT_SIZE, "%d", arg);
    bool	ret(pushArgument(ptr));
    if (!ret) {
        free(ptr);
    }

    return ret;
}

/*
 * bool
 * ArgVector::expand(size_t newsize)
 *	Expand internal argument vector array.
 *
 * Calling/Exit State:
 *	True is returned on success, false on failure.
 */
bool
ArgVector::expand(size_t newsize)
{
    void	*ptr(realloc(reinterpret_cast<void *>(_argv),
                             sizeof(char *) * newsize));
    if (ptr == NULL) {
        return false;
    }

    _argv = reinterpret_cast<char **>(ptr);
    _capacity = newsize;

    return true;
}

/*
 * bool
 * ArgVector::pushArgument(char *arg)
 *	Push an argument to the vector.
 *
 * Calling/Exit State:
 *	True is returned on success, false on failure.
 */
bool
ArgVector::pushArgument(char *arg)
{
    size_t	newsize = _size + 2;	// 2 more element is needed.

    if (newsize > _capacity && !expand(newsize + ARGV_DELTA)) {
        return false;
    }

    *(_argv + _size) = arg;
    *(_argv + _size + 1) = NULL;
    _size++;

    return true;
}

/*
 * void
 * ex_assert(int required, int value, const char *stmt, bool result,
 *	     const char *file, int line) throw(std::runtime_error)
 * ex_assert(unsigned int required, unsigned int value, const char *stmt,
 *	     bool result, const char *file, int line) throw(std::runtime_error)
 * ex_assert(long required, long value, const char *stmt, bool result,
 *	     const char *file, int line) throw(std::runtime_error)
 * ex_assert(unsigned long required, unsigned long value, const char *stmt,
 *	     bool result, const char *file, int line) throw(std::runtime_error)
 * ex_assert(long long required, long long value, const char *stmt, bool result,
 *	     const char *file, int line) throw(std::runtime_error)
 * ex_assert(unsigned long long required, unsigned long long value,
 *	     const char *stmt, bool result, const char *file, int line)
 *          throw(std::runtime_error)
 *	Check identity of the two value.
 *	If `result' is true, `value' must be identical to `required'.
 *	If `result is false,m `value' must not be identical to `required'.
 *	std::runtime_error is thrown if the check fails.
 */
void
ex_assert(int required, int value, const char *stmt, bool result,
          const char *file, int line) throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%d, actual=%d",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(unsigned int required, unsigned int value, const char *stmt,
          bool result, const char *file, int line) throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%u, actual=%u",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(long required, long value, const char *stmt, bool result,
          const char *file, int line) throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%ld, actual=%ld",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(unsigned long required, unsigned long value, const char *stmt,
          bool result, const char *file, int line) throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%lu, actual=%lu",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(long long required, long long value, const char *stmt, bool result,
          const char *file, int line) throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%lld, actual=%lld",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(unsigned long long required, unsigned long long value,
          const char *stmt, bool result, const char *file, int line)
    throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%llu, actual=%llu",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

void
ex_assert(void *required, void *value, const char *stmt, bool result,
          const char *file, int line)
    throw(std::runtime_error)
{
    bool	check((required == value) ? true : false);

    if (check != result) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%p, actual=%p",
                 file, line, stmt, required, value);
        throw std::runtime_error(buf);
    }
}

/*
 * static void
 * ex_assert_bool(const char *stmt, bool required, bool value,
 *		  const char *file, int line) throw(std::runtime_error)
 *	Ensure that `result' equals `value'.
 *	std::runtime_error is thrown if the check fails.
 */
void
ex_assert_bool(const char *stmt, bool required, bool value,
               const char *file, int line) throw(std::runtime_error)
{
    if (required != value) {
        char	buf[EXASSERT_BUFSIZE];

        snprintf(buf, sizeof(buf), "%s:%d: %s: required=%s, actual=%s",
                 file, line, stmt,
                 (required) ? "true" : "false",
                 (value) ? "true" : "false");
        throw std::runtime_error(buf);
    }
}

/*
 * void
 * stringify(gidset_t &set, std::string &str)
 *      Set string representation of the given gidset_t into `str'.	
 */
void
stringify(gidset_t &set, std::string &str)
{
    std::ostringstream  os;

    os << "[";

    const char *sep(NULL);
    for (gidset_t::iterator it(set.begin()); it != set.end(); it++) {
        if (sep != NULL) {
            os << sep;
        }
        os << *it;
        sep = ", ";
    }

    os << "]";

    str.assign(os.str());
}
