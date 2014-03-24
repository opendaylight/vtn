/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_proc.cc - Test for PFC process utilities.
 */

#include <string>
#include <unistd.h>
#include <pfc/util.h>
#include <pfc/listmodel.h>
#include "test.h"
#include "misc.hh"
#include "random.hh"

using namespace std;

/*
 * static void
 * test_closefrom(uint32_t num, uint32_t from)
 *   This function called fork(2).
 */
static void
test_closefrom(uint32_t num, uint32_t from)
{
    pid_t pid;

    pid = fork();
    if (pid == 0) {
        /* Test main */
        uint32_t i;
        pfc_listm_t list;
        DIR *dir;
        struct dirent *dep;
        ostringstream oss;
        string str_pid;

        oss << getpid();
        str_pid = "/proc/" + oss.str() + "/fd";

        ASSERT_EQ(0, pfc_llist_create_u64(&list));

        /* Create file descriptor. */
        for (i = 0; i < num; i++) {
            ASSERT_NE(-1, open("/dev/null", O_RDWR))
                << "*** ERROR: " << strerror(errno);
        }

        /* Check opening file descriptor. */
        dir = opendir(str_pid.c_str());
        dep = readdir(dir);
        while (dep != NULL) {
            string filename = dep->d_name;
            int fd;

            /* ".", ".." is skip.  */
            if (filename == "." || filename == "..") {
                dep = readdir(dir);
                continue;
            }

            /* Convert string to integer. */
            istringstream is(filename);
            is >> fd;
            ASSERT_EQ(0, pfc_listm64_push_tail(list, (uint64_t)fd));

            /* Check next file. */
            dep = readdir(dir);
        }
        ASSERT_EQ(0, closedir(dir));

        /* Close file descriptor. */
        pfc_closefrom(from);

        /* Check opening file descriptor. */
        dir = opendir(str_pid.c_str());
        dep = readdir(dir);
        while (dep != NULL) {
            string filename = dep->d_name;
            int fd;

            /* ".", ".." is skip.  */
            if (filename == "." || filename == "..") {
                dep = readdir(dir);
                continue;
            }

            /* Convert string to integer. */
            istringstream is(filename);
            is >> fd;

            /* Check the value which is less than 'from'. */
            if (dirfd(dir) != fd) {
                ASSERT_LT(fd, from);
            }
            /* Check the value which is in the list. */
            ASSERT_LE(0, pfc_listm64_index(list, (uint64_t)fd));

            /* Check next file. */
            dep = readdir(dir);
        }
        ASSERT_EQ(0, closedir(dir));

        /* Child process exit. */
        exit(0);
    } else {
        int status;
        ASSERT_LT(0, wait(&status));
        ASSERT_EQ(0, WEXITSTATUS(status));
        ASSERT_EQ(0, WIFSIGNALED(status));
    }
}

/*
 * Create a zombie process.
 */
class Zombie
{
public:
    Zombie() : _pid(static_cast<pid_t>(-1)) {}

    ~Zombie()
    {
        if (_pid != static_cast<pid_t>(-1)) {
            waitpid(_pid, NULL, 0);
        }
    }

    void setup(void);

    inline pid_t
    operator*() const
    {
        return _pid;
    }

private:
    pid_t  _pid;
};

void
Zombie::setup(void)
{
    int  fds[2];
    ASSERT_EQ(0, pipe(fds)) << "*** ERROR: " << strerror(errno);
    FdRef rfd(fds[0]), wfd(fds[1]);

    _pid = fork();
    ASSERT_NE(static_cast<pid_t>(-1), _pid)
        << "*** ERROR: " << strerror(errno);
    if (_pid == 0) {
        _exit(0);
    }

    ASSERT_EQ(0, close(wfd.get())) << "*** ERROR: " << strerror(errno);
    wfd.set(-1);

    char c;
    ASSERT_EQ(0, read(rfd.get(), &c, 1));
}

/*
 * Test for pfc_closefrom().
 *   This test called fork(2).
 */
TEST(proc, pfc_closefrom)
{
    int i;
    uint32_t max;
    RandomGenerator rand;

    /* Max number of open file descriptor. */
    max = 50;
    for (i = 0; i < 10; i++) {
        uint32_t num;
        uint32_t from;
        rand.randomInteger(num, max);
        rand.randomInteger(from, num);
        test_closefrom(num, from);
    }
}

/*
 * Test case for zombie process.
 */
TEST(proc, zombie)
{
    // pfc_proc_getcmdline() must fail.
    {
        Zombie z;
        z.setup();
        RETURN_ON_ERROR();

        pfc_listm_t list;
        ASSERT_EQ(ENODATA, pfc_proc_getcmdline(*z, &list));
    }
}
