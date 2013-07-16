/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Implementation of strint.
 */

#include "strint_refptr.h"
#include <pfc/strtoint.h>
#include <pfc/atomic.h>

#define MAX_STR_LEN  11     /* log10(2^32) -> 9.6330 */

static const pfc_refptr_ops_t strint_ops_body = {
    .dtor     = strint_dtor,
    .compare  = strint_compare,
    .equals   = strint_equals,
    .hashfunc = strint_hashfunc,
};
const pfc_refptr_ops_t *strint_ops = &strint_ops_body;

static const pfc_hash_ops_t strint_kv_hash_ops_body = {
    .equals     = strint_equals,
    .hashfunc   = strint_hashfunc,
    .key_dtor   = strint_hash_key_dtor,
    .value_dtor = strint_hash_val_dtor,
};
const pfc_hash_ops_t *strint_kv_hash_ops = &strint_kv_hash_ops_body;

static volatile uint64_t call_count_dtor          = 0U;
static volatile uint64_t call_count_key_dtor_kref = 0U;
static volatile uint64_t call_count_key_dtor      = 0U;
static volatile uint64_t call_count_val_dtor_vref = 0U;
static volatile uint64_t call_count_val_dtor      = 0U;
static volatile uint64_t call_count_compare       = 0U;
static volatile uint64_t call_count_equals        = 0U;
static volatile uint64_t call_count_hashfunc      = 0U;

static volatile uint64_t dtor_last_addr     = 0U;
static volatile uint64_t key_dtor_last_addr = 0U;
static volatile uint64_t val_dtor_last_addr = 0U;

uint64_t
strint_get_count_dtor ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_dtor);
}

uint64_t
strint_get_count_key_dtor ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_key_dtor);
}

uint64_t
strint_get_count_key_dtor_kref ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_key_dtor_kref);
}

uint64_t
strint_get_count_val_dtor ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_val_dtor);
}

uint64_t
strint_get_count_val_dtor_vref ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_val_dtor_vref);
}

uint64_t
strint_get_count_compare ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_compare);
}

uint64_t
strint_get_count_equals ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_equals);
}

uint64_t
strint_get_count_hashfunc ()
{
    return pfc_atomic_read_uint64((uint64_t *)&call_count_hashfunc);
}

pfc_cptr_t
strint_get_dtor_last_addr ()
{
    uint64_t u64_addr = pfc_atomic_read_uint64((uint64_t *)&dtor_last_addr);
    return (pfc_cptr_t)(uintptr_t)u64_addr;
}

static void
strint_set_dtor_last_addr (pfc_cptr_t addr)
{
    uint64_t u64_addr = (uint64_t)(uintptr_t)addr;
    pfc_atomic_swap_uint64((uint64_t *)&dtor_last_addr, u64_addr);
}

pfc_cptr_t
strint_get_key_dtor_last_addr ()
{
    uint64_t u64_addr = pfc_atomic_read_uint64((uint64_t *)&key_dtor_last_addr);
    return (pfc_cptr_t)(uintptr_t)u64_addr;
}

static void
strint_set_key_dtor_last_addr (pfc_cptr_t addr)
{
    uint64_t u64_addr = (uint64_t)(uintptr_t)addr;
    pfc_atomic_swap_uint64((uint64_t *)&key_dtor_last_addr, u64_addr);
}

pfc_cptr_t
strint_get_val_dtor_last_addr ()
{
    uint64_t u64_addr = pfc_atomic_read_uint64((uint64_t *)&val_dtor_last_addr);
    return (pfc_cptr_t)(uintptr_t)u64_addr;
}

static void
strint_set_val_dtor_last_addr (pfc_cptr_t addr)
{
    uint64_t u64_addr = (uint64_t)(uintptr_t)addr;
    pfc_atomic_swap_uint64((uint64_t *)&val_dtor_last_addr, u64_addr);
}


pfc_refptr_t *
strint_create_from_int (uint32_t i)
{
    pfc_refptr_t *obj = NULL;

    char *s = int_to_str(i);
    if (s == NULL) goto end;

    obj = pfc_refptr_create(strint_ops, s);
  end:
    return obj;
}


void
strint_dtor (pfc_ptr_t sint_obj)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_dtor);
    strint_set_dtor_last_addr(sint_obj);
#ifdef LOG_STRINT_DTOR
    if (sint_obj == NULL) {
        fprintf(stderr, "%s: sint_obj = NULL\n", __func__);
    } else {
        fprintf(stderr, "%s: sint_obj = %p = \"%s\"\n",
                __func__, sint_obj, (char *)sint_obj);
    }
#endif /* LOG_STRINT_DTOR */
    if (sint_obj != NULL)
        free(sint_obj);
}


void
strint_hash_key_dtor (pfc_ptr_t sint_obj, uint32_t flags)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_key_dtor);
    strint_set_key_dtor_last_addr(sint_obj);
    if (flags && PFC_HASHOP_KEY_REFPTR) {
        pfc_atomic_inc_uint64((uint64_t *)&call_count_key_dtor_kref);
    } else {
        strint_dtor(sint_obj);
    }
}


void
strint_hash_val_dtor (pfc_ptr_t sint_obj, uint32_t flags)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_val_dtor);
    strint_set_val_dtor_last_addr(sint_obj);
    if (flags && PFC_HASHOP_VAL_REFPTR) {
        pfc_atomic_inc_uint64((uint64_t *)&call_count_val_dtor_vref);
    } else {
        strint_dtor(sint_obj);
    }
}


int
strint_compare (pfc_cptr_t sint_obj1, pfc_cptr_t sint_obj2)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_compare);
    uint32_t i1 = str_to_int((char *)sint_obj1);
    uint32_t i2 = str_to_int((char *)sint_obj2);
    return (i1 == i2) ? 0 : ((i1 > i2) ? 1 : -1);
}


pfc_bool_t
strint_equals (pfc_cptr_t sint_obj1, pfc_cptr_t sint_obj2)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_equals);
    uint32_t i1 = str_to_int((char *)sint_obj1);
    uint32_t i2 = str_to_int((char *)sint_obj2);
    return (i1 == i2);
}


uint32_t
strint_hashfunc (pfc_cptr_t sint_obj)
{
    pfc_atomic_inc_uint64((uint64_t *)&call_count_hashfunc);
    // PFC_ASSERT(s != NULL);
    // if (sint_obj == NULL) return 0;
    uint32_t hashval = (uint32_t)str_to_int((char *)sint_obj);
    // fprintf(stderr, "%s: sint_obj = %p -> %u\n",
    //        __func__, sint_obj, hashval);
    return hashval;
}


char *
str_of_strint_refptr (pfc_refptr_t *sint_refp)
{
    char *s = PFC_REFPTR_VALUE(sint_refp, char *);
    // PFC_ASSERT(s != NULL);
    return s;
}


uint32_t
int_of_strint_refptr (pfc_refptr_t *sint_refp)
{
    return str_to_int(str_of_strint_refptr(sint_refp));
}


uint32_t
str_to_int (const char *s)
{
    if (s == NULL) return STRINT_INTVAL_FOR_NULL;
    uint32_t i = 12345;
    int err = pfc_strtou32(s, &i);
    if (err != 0) {
#ifdef LOG_STR_TO_INT
        if (s == NULL) {
            fprintf(stderr, "%s: s = NULL\n", __func__);
        } else {
            fprintf(stderr, "%s: s = %p = \"%s\"\n", __func__, s, s);
        }
#endif /* LOG_STR_TO_INT */
        PFC_ASSERT(PFC_FALSE);
        abort();
    }
    return i;
}


char *
int_to_str(uint32_t i)
{
    char *s = (char *)calloc(1, MAX_STR_LEN);
    PFC_ASSERT(s != NULL);
    snprintf(s, MAX_STR_LEN, "%u", i);
#ifdef LOG_INT_TO_STR
    fprintf(stderr, "%s: s = %p = \"%s\"\n", __func__, s, s);
#endif /* LOG_INT_TO_STR */
    return s;
}
