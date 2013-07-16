/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_sysconf.cc - Test for functions defined in libpfc_util/sysconf.c
 */

#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/times.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <sstream>
#include <stdexcept>
#include <pfc/util.h>
#include "misc.hh"

#define	SYSCONF_TEST_LOOP	1024

#if	defined(__i386) || defined(__x86_64)

/*
 * Page size is 4 kilobytes on i386 and x86_64.
 */
#define	SYS_PAGESIZE	0x1000

/*
 * Test case of pfc_get_pagesize().
 */
TEST(sysconf,get_pagesize)
{
    for (int i(0); i < SYSCONF_TEST_LOOP; i++) {
        ASSERT_EQ(static_cast<size_t>(SYS_PAGESIZE), pfc_get_pagesize());
    }
}

#endif	/* defined(__i386) || defined(__x86_64) */

#ifdef	__linux

/*
 * Path to sysfs file which contains online CPUs.
 */
#define	SYSFS_ONLINE_PATH	"/sys/devices/system/cpu/online"

/*
 * Temporary buffer size to read contents of SYSFS_ONLINE_PATH.
 */
#define	SYSFS_ONLINE_BUFSIZE	128

/*
 * Class to obtain number of online CPUs via sysfs.
 */
class OnlineCpu
{
public:
    OnlineCpu() : _fd(-1) {}
    ~OnlineCpu()
    {
        if (_fd != -1) {
            close(_fd);
        }
    }

    uint32_t	get(void) throw(std::runtime_error);

private:
    int	_fd;
};

/*
 * uint32_t
 * OnlineCpu::get(void)
 *	Return number of online CPUs.
 */
uint32_t
OnlineCpu::get(void) throw(std::runtime_error)
{
    int	fd(_fd);

    if (fd == -1) {
        fd = open(SYSFS_ONLINE_PATH, O_RDONLY);
        if (fd == -1) {
            int	err(errno);

            if (err == ENOENT) {
                // sysfs is not mounted, or the kernel is too old.
                throw std::runtime_error(SYSFS_ONLINE_PATH " does not exist.");
            }

            THROW_ERRNO("open(online) failed", err);
        }
        _fd = fd;
    }

    // Read contents of online file.
    std::string	contents;
    for (;;) {
        char	buffer[SYSFS_ONLINE_BUFSIZE];

        ssize_t	nbytes(read(fd, buffer, sizeof(buffer)));
        if (nbytes == -1) {
            THROW_ERRNO("read(online) failed", errno);
        }
        if (nbytes == 0) {
            break;
        }
        contents.append(buffer, nbytes);
    }

    const char		*ptr(contents.c_str());
    uint32_t		count(0);
    unsigned long	start(0);
    bool		has_start(false);

    for (;;) {
        char	*p;

        errno = 0;
        unsigned long	cpu(strtoul(ptr, &p, 10));
        if (errno != 0 || p == ptr) {
            std::ostringstream stream;
            stream << "Unexpected integer: " << contents;
            throw std::runtime_error(stream.str());
        }
	ptr = p + 1;

        char	c(*p);
        if (c == '-') {
            if (has_start) {
                std::ostringstream stream;
                stream << "Unexpected CPU range format: " << contents;
                throw std::runtime_error(stream.str());
            }

            // Start of CPU number range.
            start = cpu;
            has_start = true;

            continue;
        }

        if (has_start) {
            if (start >= cpu) {
                std::ostringstream stream;
                stream << "Invalid CPU range: " << contents;
                throw std::runtime_error(stream.str());
            }
            count += (cpu - start + 1);
        }
        else {
            count++;
        }

        has_start = false;
        if (c == '\n' || c == '\0') {
            break;
        }
        if (c != ',') {
            std::ostringstream stream;
            stream << "Unexpected character: " << contents;
            throw std::runtime_error(stream.str());
        }
    }

    return count;
}

/*
 * Test case of pfc_get_online_cpus().
 */
TEST(sysconf,get_online_cpus)
{
    OnlineCpu	system;
    uint32_t	online;
    try {
        online = system.get();
    }
    catch (const std::runtime_error &e) {
        FAIL() << e.what();
    }

    ASSERT_EQ(online, pfc_get_online_cpus());
}

#endif	/* __linux */

/*
 * Test case of pfc_get_ngroups_max().
 */
TEST(sysconf, get_ngroups_max)
{
    long ngroups(sysconf(_SC_NGROUPS_MAX));
    ASSERT_LT(0, ngroups);

    ASSERT_EQ(static_cast<uint32_t>(ngroups), pfc_get_ngroups_max());
}
