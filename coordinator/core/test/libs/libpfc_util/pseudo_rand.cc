/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Pseudo radnom namuber generator
 */

#include "pseudo_rand.hh"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


/*
 * type definitions
 */

// typedef boost::minstd_rand base_gen_t;
typedef boost::mt19937 base_gen_t;
typedef boost::uniform_int<long> dist_t;
typedef boost::variate_generator<base_gen_t&, dist_t> die_t;
        
struct __prand_generator {
    base_gen_t *base_gen;
    die_t *die;
};


/*
 * implementations of public function
 */

prand_generator_t
prand_create (prand_seed_t seed)
{
    prand_generator_t prand_gen = new struct __prand_generator();
    prand_gen->base_gen = new base_gen_t(static_cast<prand_mt_seed_t>(seed));
    dist_t dist(PSEUDO_RAND_MIN, PSEUDO_RAND_MAX);
    prand_gen->die = new die_t(*prand_gen->base_gen, dist);
    return prand_gen;
}


void
prand_destroy (prand_generator_t prand_gen)
{
    delete prand_gen->die;
    delete prand_gen->base_gen;
    delete prand_gen;
}


long
prand_get_long (prand_generator_t prand_gen)
{
    return (*prand_gen->die)();
}
