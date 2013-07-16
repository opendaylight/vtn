/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _STRING_UTIL_H
#define _STRING_UTIL_H

/*
 * string utilities for tests
 */

#include <string.h>
#include <pfc/base.h>
#include "pseudo_rand.hh"

PFC_C_BEGIN_DECL

extern char *random_string (int max_len, char min_ch, char max_ch);
extern char *random_string_with_prand_gen (prand_generator_t prand_gen,
                                           int max_len, char min_ch, char max_ch);
extern pfc_bool_t string_equals (pfc_cptr_t x, pfc_cptr_t y);
extern int string_comp (pfc_cptr_t x, pfc_cptr_t y);

PFC_C_END_DECL

#endif  /* !_STRING_UTIL_H */
