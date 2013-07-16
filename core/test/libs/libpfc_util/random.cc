/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * random.cc - Random integer generator.
 */

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include "random.hh"

/*
 * Path to random device file.
 */
#define	RANDOM_DEVICE		"/dev/urandom"

/*
 * Maximum length of error message.
 */
#define	RANDOM_ERRSIZE		128

/*
 * RandomError::RandomError(const std::string &msg, int err)
 *	Create exception instance which represents failure of RandomGenerator.
 */
RandomError::RandomError(const std::string &msg, int err)
    : std::runtime_error(msg), _error(err)
{
}

/*
 * RandomGenerator::RandomGenerator()
 *	Constructor of random integer generator.
 */
RandomGenerator::RandomGenerator()
    : _randFd(open(RANDOM_DEVICE, O_RDONLY)), _openError(0)
{
    if (_randFd == -1) {
        _openError = errno;
    }
}

/*
 * RandomGenerator::~RandomGenerator()
 *	Destructor of random integer generator.
 */
RandomGenerator::~RandomGenerator()
{
    if (_randFd != -1) {
        (void)close(_randFd);
    }
}

/*
 * void
 * RandomGenerator::fillRandom(uint8_t *ptr, size_t size)
 *	Fill random bits into the specified buffer.
 *	RandomError will be thrown on error.
 */
void
RandomGenerator::fillRandom(uint8_t *ptr, size_t size) throw(RandomError)
{
    ssize_t	remains(static_cast<ssize_t>(size));

    if (_randFd == -1) {
        throw RandomError("open(" RANDOM_DEVICE ") failed", _openError);
    }

    do {
        ssize_t	nbytes(read(_randFd, ptr, remains));
        if (nbytes == -1) {
            throw RandomError("read(random) failed", errno);
        }
        ptr += nbytes;
        remains -= nbytes;
    } while (remains != 0);
}
