/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Implementation of string utilities
 */

#include "string_util.h"

char *
random_string (int max_len, char min_ch, char max_ch)
{
    return random_string_with_prand_gen(NULL, max_len, min_ch, max_ch);
}


char *
random_string_with_prand_gen (prand_generator_t prand_gen,
                              int max_len, char min_ch, char max_ch)
{
    int len;
    if (prand_gen == NULL) {
        len = random() % max_len + 1;
    } else {
        len = prand_get_long(prand_gen) % max_len + 1;
    }
    char *s = (char *)calloc(max_len + 1, sizeof(char));
    if (s == NULL) abort();
    if (min_ch > max_ch) abort();

    int i;
    for (i = 0; i < len; ++i) {
        int offset;
        if (prand_gen == NULL) {
            offset = random() % (max_ch + 1 - min_ch);
        } else {
            offset = prand_get_long(prand_gen) % (max_ch + 1 - min_ch);
        }
        s[i] = min_ch + offset;
    }

    return s;
}


pfc_bool_t
string_equals (pfc_cptr_t x, pfc_cptr_t y)
{
    return (string_comp(x, y) == 0);
}


int
string_comp (pfc_cptr_t x, pfc_cptr_t y)
{
    return strcmp((const char *)x, (const char *)y);
}
