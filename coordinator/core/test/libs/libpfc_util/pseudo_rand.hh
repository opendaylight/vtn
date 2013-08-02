/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PSEUDO_RAND_HH
#define _PSEUDO_RAND_HH

#include <pfc/base.h>


/*
 * Definitionof of pseudo radnom namuber generator
 */

PFC_C_BEGIN_DECL

#define PSEUDO_RAND_MIN (0)
#define PSEUDO_RAND_MAX (PFC_LONG_MAX)

struct __prand_gengenerator;
typedef struct __prand_generator *prand_generator_t;

typedef uint32_t prand_mt_seed_t;
typedef prand_mt_seed_t prand_seed_t;

extern prand_generator_t prand_create (prand_seed_t seed);
extern void prand_destroy (prand_generator_t prand_gen);
extern long prand_get_long (prand_generator_t prand_gen);

PFC_C_END_DECL

#endif /* ! _PSEUDO_RAND_HH */
