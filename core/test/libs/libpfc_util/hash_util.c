/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Implementation of hash test utilities
 */

#include "hash_util.h"
#include "strint_refptr.h"
#include <pfc/atomic.h>


/*
 * macros for atomic operations on counters and last target address traces
 */

#define DEF_GET_COUNT(PREFIX, BASENAME)                             \
    uint64_t PREFIX ## _get_count_ ## BASENAME ()                   \
    {                                                               \
        uint64_t *p = (uint64_t *)&(PREFIX ## _count_ ## BASENAME); \
        return pfc_atomic_read_uint64(p);                           \
    }

#define DEF_INC_COUNT(PREFIX, BASENAME)                             \
    static void PREFIX ## _inc_count_ ## BASENAME ()                \
    {                                                               \
        uint64_t *p = (uint64_t *)&(PREFIX ## _count_ ## BASENAME); \
        pfc_atomic_inc_uint64(p);                                   \
    }

#define DEF_GET_LASTADDR(PREFIX, BASENAME)                              \
    pfc_cptr_t PREFIX ## _get_lastaddr_ ## BASENAME ()                  \
    {                                                                   \
        uint64_t *p = (uint64_t *)&(PREFIX ## _lastaddr_ ## BASENAME);  \
        uint64_t u64_addr = pfc_atomic_read_uint64(p);                  \
        return (pfc_cptr_t)(uintptr_t)u64_addr;                         \
    }

#define DEF_SET_LASTADDR(PREFIX, BASENAME)                              \
    static void PREFIX ## _set_lastaddr_ ## BASENAME (pfc_cptr_t addr)  \
    {                                                                   \
        uint64_t *p = (uint64_t *)&(PREFIX ## _lastaddr_ ## BASENAME);  \
        uint64_t u64_addr = (uint64_t)(uintptr_t)addr;                  \
        pfc_atomic_swap_uint64(p, u64_addr);                            \
    }


/*
 * hash operator functions for watching
 */

/* prototypes */

static pfc_bool_t wtchops_equals(pfc_cptr_t obj1, pfc_cptr_t obj2);
static pfc_bool_t wtchops_equals_strint(pfc_cptr_t obj1, pfc_cptr_t obj2);
static uint32_t  wtchops_hashfunc(pfc_cptr_t obj);
static uint32_t  wtchops_hashfunc_strint(pfc_cptr_t obj);
static void      wtchops_key_dtor_nofree(pfc_ptr_t obj, uint32_t flags);
static void      wtchops_key_dtor_free(pfc_ptr_t obj, uint32_t flags);
static void      wtchops_key_dtor_refput(pfc_ptr_t obj, uint32_t flags);
static void      wtchops_val_dtor_nofree(pfc_ptr_t obj, uint32_t flags);
static void      wtchops_val_dtor_free(pfc_ptr_t obj, uint32_t flags);
static void      wtchops_val_dtor_refput(pfc_ptr_t obj, uint32_t flags);


/* definitions of operator sets */

static const pfc_hash_ops_t hash_wtchops_kv_nofree_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_nofree,
    .value_dtor = wtchops_val_dtor_nofree,
};
const pfc_hash_ops_t *hash_wtchops_kv_nofree = &hash_wtchops_kv_nofree_body;

static const pfc_hash_ops_t hash_wtchops_kptr_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_free,
    .value_dtor = NULL,
};
const pfc_hash_ops_t *hash_wtchops_kptr = &hash_wtchops_kptr_body;

static const pfc_hash_ops_t hash_wtchops_vptr_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = NULL,
    .value_dtor = wtchops_val_dtor_free,
};
const pfc_hash_ops_t *hash_wtchops_vptr = &hash_wtchops_vptr_body;

static const pfc_hash_ops_t hash_wtchops_kref_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_refput,
    .value_dtor = NULL,
};
const pfc_hash_ops_t *hash_wtchops_kref = &hash_wtchops_kref_body;

static const pfc_hash_ops_t hash_wtchops_vref_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = NULL,
    .value_dtor = wtchops_val_dtor_refput,
};
const pfc_hash_ops_t *hash_wtchops_vref = &hash_wtchops_vref_body;

static const pfc_hash_ops_t hash_wtchops_kptr_vptr_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_free,
    .value_dtor = wtchops_val_dtor_free,
};
const pfc_hash_ops_t *hash_wtchops_kptr_vptr = &hash_wtchops_kptr_vptr_body;

static const pfc_hash_ops_t hash_wtchops_kptr_vref_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_free,
    .value_dtor = wtchops_val_dtor_refput,
};
const pfc_hash_ops_t *hash_wtchops_kptr_vref = &hash_wtchops_kptr_vref_body;

static const pfc_hash_ops_t hash_wtchops_kref_vptr_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_refput,
    .value_dtor = wtchops_val_dtor_free,
};
const pfc_hash_ops_t *hash_wtchops_kref_vptr = &hash_wtchops_kref_vptr_body;

static const pfc_hash_ops_t hash_wtchops_kref_vref_body = {
    .equals     = wtchops_equals,
    .hashfunc   = wtchops_hashfunc,
    .key_dtor   = wtchops_key_dtor_refput,
    .value_dtor = wtchops_val_dtor_refput,
};
const pfc_hash_ops_t *hash_wtchops_kref_vref = &hash_wtchops_kref_vref_body;

static const pfc_hash_ops_t hash_wtchops_ksintp_vptr_body = {
    .equals     = wtchops_equals_strint,
    .hashfunc   = wtchops_hashfunc_strint,
    .key_dtor   = wtchops_key_dtor_free,
    .value_dtor = wtchops_val_dtor_free,
};
const pfc_hash_ops_t *hash_wtchops_ksintp_vptr = &hash_wtchops_ksintp_vptr_body;

static const pfc_hash_ops_t hash_wtchops_ksintp_vref_body = {
    .equals     = wtchops_equals_strint,
    .hashfunc   = wtchops_hashfunc_strint,
    .key_dtor   = wtchops_key_dtor_free,
    .value_dtor = wtchops_val_dtor_refput,
};
const pfc_hash_ops_t *hash_wtchops_ksintp_vref = &hash_wtchops_ksintp_vref_body;


/* counters and their getters / incrementers */

static volatile uint64_t wtchops_count_equals        = 0U;
static volatile uint64_t wtchops_count_hashfunc      = 0U;
static volatile uint64_t wtchops_count_key_dtor      = 0U;
static volatile uint64_t wtchops_count_key_dtor_kref = 0U;
static volatile uint64_t wtchops_count_val_dtor      = 0U;
static volatile uint64_t wtchops_count_val_dtor_vref = 0U;

DEF_GET_COUNT(wtchops, equals);        /* wtchops_get_count_equals        */
DEF_GET_COUNT(wtchops, hashfunc);      /* wtchops_get_count_hashfunc      */
DEF_GET_COUNT(wtchops, key_dtor);      /* wtchops_get_count_key_dtor      */
DEF_GET_COUNT(wtchops, key_dtor_kref); /* wtchops_get_count_key_dtor_kref */
DEF_GET_COUNT(wtchops, val_dtor);      /* wtchops_get_count_val_dtor      */
DEF_GET_COUNT(wtchops, val_dtor_vref); /* wtchops_get_count_val_dtor_vref */

DEF_INC_COUNT(wtchops, equals);        /* wtchops_inc_count_equals        */
DEF_INC_COUNT(wtchops, hashfunc);      /* wtchops_inc_count_hashfunc      */
DEF_INC_COUNT(wtchops, key_dtor);      /* wtchops_inc_count_key_dtor      */
DEF_INC_COUNT(wtchops, key_dtor_kref); /* wtchops_inc_count_key_dtor_kref */
DEF_INC_COUNT(wtchops, val_dtor);      /* wtchops_inc_count_val_dtor      */
DEF_INC_COUNT(wtchops, val_dtor_vref); /* wtchops_inc_count_val_dtor_vref */


/* last target addresses as uint64_t and their getters / setters */

static volatile uint64_t wtchops_lastaddr_key_dtor = 0U;
static volatile uint64_t wtchops_lastaddr_val_dtor = 0U;

DEF_GET_LASTADDR(wtchops, key_dtor); /* wtchops_get_lastaddr_key_dtor */
DEF_GET_LASTADDR(wtchops, val_dtor); /* wtchops_get_lastaddr_val_dtor */

DEF_SET_LASTADDR(wtchops, key_dtor); /* wtchops_set_lastaddr_key_dtor */
DEF_SET_LASTADDR(wtchops, val_dtor); /* wtchops_set_lastaddr_val_dtor */


/* operator functions */

static pfc_bool_t
wtchops_equals (pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    wtchops_inc_count_equals();
    if  (obj1 == obj2) {
        return PFC_TRUE;
    } else {
        return PFC_FALSE;
    }
}


static pfc_bool_t
wtchops_equals_strint (pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    wtchops_inc_count_equals();
    uint32_t i1 = str_to_int((char *)obj1);
    uint32_t i2 = str_to_int((char *)obj2);
    return (i1 == i2);
}


static uint32_t
wtchops_hashfunc (pfc_cptr_t obj)
{
    wtchops_inc_count_hashfunc();
    return (uint32_t)(uintptr_t)obj;
}


static uint32_t
wtchops_hashfunc_strint (pfc_cptr_t obj)
{
    wtchops_inc_count_hashfunc();
    uint32_t hashval = (uint32_t)str_to_int((char *)obj);
    return hashval;
}


static void
wtchops_key_dtor_nofree (pfc_ptr_t obj, uint32_t flags)
{
    wtchops_inc_count_key_dtor();
    if (flags & PFC_HASHOP_KEY_REFPTR) {
        wtchops_inc_count_key_dtor_kref();
    }
    wtchops_set_lastaddr_key_dtor(obj);
#ifdef  LOG_WTCHOPS_KEY_DTOR
    fprintf(stderr, "%s: obj = %p\n", __func__, obj);
#endif  /* LOG_WTCHOPS_KEY_DTOR */
}


static void
wtchops_key_dtor_free (pfc_ptr_t obj, uint32_t flags)
{
    // PFC_ASSERT(!(flags & PFC_HASHOP_KEY_REFPTR));
    // memo:
    //   strhash internally uses refptr and calls destructor
    //   with PFC_HASHOP_KEY_REFPTR flag
    wtchops_key_dtor_nofree(obj, flags);
    if (!(flags & PFC_HASHOP_KEY_REFPTR)) {
        if (obj != NULL) {
            free(obj);
        }
    }
}


static void
wtchops_key_dtor_refput (pfc_ptr_t obj, uint32_t flags)
{
    wtchops_key_dtor_nofree(obj, flags);
    if (!(flags & PFC_HASHOP_KEY_REFPTR)) {
        if (obj != NULL) {
            pfc_refptr_put((pfc_refptr_t *)obj);
        }
    }
}


static void
wtchops_val_dtor_nofree (pfc_ptr_t obj, uint32_t flags)
{
    wtchops_inc_count_val_dtor();
    if (flags & PFC_HASHOP_VAL_REFPTR) {
        wtchops_inc_count_val_dtor_vref();
    }
    wtchops_set_lastaddr_val_dtor(obj);
#ifdef  LOG_WTCHOPS_VAL_DTOR
    fprintf(stderr, "%s: obj = %p\n", __func__, obj);
#endif  /* LOG_WTCHOPS_KEY_DTOR */
}


static void
wtchops_val_dtor_free (pfc_ptr_t obj, uint32_t flags)
{
    PFC_ASSERT(!(flags & PFC_HASHOP_VAL_REFPTR));
    wtchops_val_dtor_nofree(obj, flags);
    if (obj != NULL) {
        free(obj);
    }
}


static void
wtchops_val_dtor_refput (pfc_ptr_t obj, uint32_t flags)
{
    /* fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__); */
    /* fprintf(stderr, "%s(%d) flags = %x\n", __FILE__, __LINE__, flags); */
    wtchops_val_dtor_nofree(obj, flags);
    if (!(flags & PFC_HASHOP_VAL_REFPTR)) {
        if (obj != NULL) {
            pfc_refptr_put((pfc_refptr_t *)obj);
        }
    }
}


/*
 * refptr operator functions for hash values
 */

/* prototypes */

static void       intref_dtor(pfc_ptr_t obj);
static int        intref_compare(pfc_cptr_t obj1, pfc_cptr_t obj2);
static pfc_bool_t intref_equals(pfc_cptr_t obj1, pfc_cptr_t obj2);
static uint32_t   intref_hashfunc(pfc_cptr_t obj);


/* definition of operator set */

static const pfc_refptr_ops_t intref_ops_body = {
    .dtor       = intref_dtor,
    .compare    = intref_compare,
    .equals     = intref_equals,
    .hashfunc   = intref_hashfunc,
};
const pfc_refptr_ops_t *intref_ops = &intref_ops_body;


/* counter and its getter / incrementer */

static volatile uint64_t intref_count_dtor = 0U;

DEF_GET_COUNT(intref, dtor);  /* intref_get_count_dtor */
DEF_INC_COUNT(intref, dtor);  /* intref_inct_count_dtor */


/* last target address as uint64_t and its getters / setters */

static volatile uint64_t intref_lastaddr_dtor = 0U;

DEF_GET_LASTADDR(intref, dtor);  /* intref_get_lastaddr_dtor */
DEF_SET_LASTADDR(intref, dtor);  /* intref_set_lastaddr_dtor */


/* constructor */

pfc_refptr_t *
intref_create (int x)
{
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) abort();
    *p = x;

    return pfc_refptr_create(intref_ops, (pfc_ptr_t)p);
}


/* operator functions */

static void
intref_dtor (pfc_ptr_t obj)
{
    intref_inc_count_dtor();
    intref_set_lastaddr_dtor(obj);

    if (obj != NULL) {
        free(obj);
    }
}

static int
intref_compare (pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    fprintf(stderr, "%s() is not implemented\n", __func__);
    abort();
}

static pfc_bool_t
intref_equals (pfc_cptr_t obj1, pfc_cptr_t obj2)
{
    fprintf(stderr, "%s() is not implemented\n", __func__);
    abort();
}

static uint32_t
intref_hashfunc (pfc_cptr_t obj)
{
    fprintf(stderr, "%s() is not implemented\n", __func__);
    abort();
}
