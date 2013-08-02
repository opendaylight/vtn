/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for pseudo radnom namuber generator
 */

#include "test.h"
#include "pseudo_rand.hh"


TEST(pseudo_rand, create_destroy)
{
    prand_generator_t prand_gen = 0;
    prand_gen = prand_create(1);
    EXPECT_NE((prand_generator_t)0, prand_gen);
    prand_destroy(prand_gen);
}


TEST(pseudo_rand, get_long)
{
    prand_generator_t prand_gen = 0;
    prand_gen = prand_create(1);
    EXPECT_NE((prand_generator_t)0, prand_gen);

    int ntrials = 10000;
    int conflict = 0;
    long prev = prand_get_long(prand_gen);
    for (int i = 0; i < ntrials; ++i) {
        long x = prand_get_long(prand_gen);
        if (x == prev) ++conflict;
        prev = x;
    }
    EXPECT_LT(conflict, ntrials / 1000);

    prand_destroy(prand_gen);
}


TEST(pseudo_rand, min_max)
{
    prand_generator_t prand_gen = 0;
    prand_gen = prand_create(1);
    EXPECT_NE((prand_generator_t)0, prand_gen);

    int ntrials = 10000;
    for (int i = 0; i < ntrials; ++i) {
        long x = prand_get_long(prand_gen);
        EXPECT_GE(x, PSEUDO_RAND_MIN);
        EXPECT_LE(x, PSEUDO_RAND_MAX);
    }

    prand_destroy(prand_gen);
}


TEST(pseudo_rand, average)
{
    prand_generator_t prand_gen = 0;
    prand_gen = prand_create(1);
    EXPECT_NE((prand_generator_t)0, prand_gen);

    int ntrials = 10000;
    long average = 0; 
    for (int i = 0; i < ntrials; ++i) {
        long x = prand_get_long(prand_gen);
        average += x / ntrials;
    }
    long expect_average =
            (PSEUDO_RAND_MAX - PSEUDO_RAND_MIN) / 2 + PSEUDO_RAND_MIN;
    long margin = (PSEUDO_RAND_MAX - PSEUDO_RAND_MIN) / 10;
    EXPECT_GE(average, expect_average - margin);
    EXPECT_LE(average, expect_average + margin);

    prand_destroy(prand_gen);
}


TEST(pseudo_rand, same_seeds)
{
    uint32_t seed = 10;

    prand_generator_t prand_gen1 = prand_create(seed);
    EXPECT_NE((prand_generator_t)0, prand_gen1);

    prand_generator_t prand_gen2 = prand_create(seed);
    EXPECT_NE((prand_generator_t)0, prand_gen2);

    int ntrials = 10000;
    for (int i = 0; i < ntrials; ++i) {
        long x = prand_get_long(prand_gen1);
        long y = prand_get_long(prand_gen2);
        EXPECT_EQ(x, y);
    }
    
    prand_destroy(prand_gen1);
    prand_destroy(prand_gen2);
}


TEST(pseudo_rand, different_seeds)
{
    uint32_t seed1 = 20;
    uint32_t seed2 = 21;

    prand_generator_t prand_gen1 = prand_create(seed1);
    EXPECT_NE((prand_generator_t)0, prand_gen1);

    prand_generator_t prand_gen2 = prand_create(seed2);
    EXPECT_NE((prand_generator_t)0, prand_gen2);

    int ntrials = 10000;
    int conflict = 0;
    for (int i = 0; i < ntrials; ++i) {
        long x = prand_get_long(prand_gen1);
        long y = prand_get_long(prand_gen2);
        if (x == y) ++conflict;
    }
    EXPECT_LT(conflict, ntrials / 10);
    
    prand_destroy(prand_gen1);
    prand_destroy(prand_gen2);
}
