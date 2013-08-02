/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for random integer generator.
 */

#ifndef	_TEST_RANDOM_HH
#define	_TEST_RANDOM_HH

#include <stdint.h>
#include <string>
#include <cstring>
#include <stdexcept>
#include <gtest/gtest.h>
#include <sys/types.h>

/*
 * Exception class which represents failure of random generator.
 */
class RandomError
    : public std::runtime_error
{
public:
    explicit RandomError(const std::string &msg, int err = 0);

    /*
     * Return error number.
     * Zero is returned if no error number is set.
     */
    inline int
    getError(void) const
    {
        return _error;
    }

private:
    /*
     * Error number.
     */
    int	_error;
};

/*
 * Random integer generator.
 */
class RandomGenerator
{
public:
    RandomGenerator();
    ~RandomGenerator();

    /*
     * Fill the specified integer with random bits.
     * If `max' is not zero, generated value never exceeds `max'.
     */
    template <class T>
    inline void
    randomInteger(T &value, T max = 0) throw(RandomError)
    {
        T	v;
        uint8_t	*ptr(reinterpret_cast<uint8_t *>(&v));

        fillRandom(ptr, sizeof(v));

        if (max != 0) {
            value = v % max;
        }
        else {
            value = v;
        }
    }

    void	fillRandom(uint8_t *ptr, size_t size) throw(RandomError);

private:
    /*
     * File descriptor of random device file.
     */
    int	_randFd;

    /*
     * Error number on device open.
     * Non-zero value is stored on device open error.
     */
    int	_openError;
};

/*
 * Helper macro to generate random integer.
 */
#define	RANDOM_INTEGER(rand, value)                                     \
    do {                                                                \
        try {                                                           \
            (rand).randomInteger(value);                                \
        }                                                               \
        catch (const RandomError &__e) {                                \
            int	__err(__e.getError());                                  \
                                                                        \
            if (__err == 0) {                                           \
                FAIL() << __e.what();                                   \
            }                                                           \
            else {                                                      \
                FAIL() << __e.what() << ": " << strerror(__err);        \
            }                                                           \
        }                                                               \
    } while (0)

#define	RANDOM_INTEGER_MAX(rand, value, max)                            \
    do {                                                                \
        try {                                                           \
            (rand).randomInteger(value, max);                           \
        }                                                               \
        catch (const RandomError &__e) {                                \
            int	__err(__e.getError());                                  \
                                                                        \
            if (__err == 0) {                                           \
                FAIL() << __e.what();                                   \
            }                                                           \
            else {                                                      \
                FAIL() << __e.what() << ": " << strerror(__err);        \
            }                                                           \
        }                                                               \
    } while (0)

#endif	/* !_TEST_RANDOM_HH */
