/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for pfc_hash_update()
 */

#include <string.h>
#include "test_hash_cmn_hashops.hh"


/*
 * define shorter name version
 */

#define TESTDATA_KEY_IS_NORMAL_STRINT HASHOPS_TESTDATA_KEY_IS_NORMAL_STRINT
#define TESTDATA_KEY_IS_NORMAL_PTR    HASHOPS_TESTDATA_KEY_IS_NORMAL_PTR
#define TESTDATA_KEY_IS_REFPTR        HASHOPS_TESTDATA_KEY_IS_REFPTR
#define TESTDATA_VAL_IS_NORMAL_PTR    HASHOPS_TESTDATA_VAL_IS_NORMAL_PTR
#define TESTDATA_VAL_IS_REFPTR        HASHOPS_TESTDATA_VAL_IS_REFPTR
#define TESTDATA_KEY_HAS_DTOR         HASHOPS_TESTDATA_KEY_HAS_DTOR
#define TESTDATA_VAL_HAS_DTOR         HASHOPS_TESTDATA_VAL_HAS_DTOR
#define TESTDATA_DECL_MEMBER_ACCESSOR HASHOPS_TESTDATA_DECL_MEMBER_ACCESSOR
#define TESTDATA_GET_OLD_COUNTS       HASHOPS_TESTDATA_GET_OLD_COUNTS


/*
 * macros
 */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(TEST_DATA)       \
    if ( ((TEST_DATA)->kref_ops       != NULL) &&               \
         ((TEST_DATA)->kref_ops       != strhash_key_ops) &&    \
         ((TEST_DATA)->kref_ops->dtor != NULL) )                \
    {                                                           \
        EXPECT_EQ(old_cnt_kref_dtor, cnt_kref_dtor());          \
    }                                                           \
    if ( ((TEST_DATA)->hash_ops           != NULL)  &&          \
         ((TEST_DATA)->hash_ops->key_dtor != NULL) )            \
    {                                                           \
        EXPECT_EQ(old_cnt_hash_ops_key_dtor,                    \
                  cnt_hash_ops_key_dtor());                     \
        EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref,               \
                  cnt_hash_ops_key_dtor_kref());                \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(TEST_DATA)   \
    if ( ((TEST_DATA)->vref_ops       != NULL) &&           \
         ((TEST_DATA)->vref_ops->dtor != NULL) )            \
    {                                                       \
        EXPECT_EQ(old_cnt_vref_dtor, cnt_vref_dtor());      \
    }                                                       \
    if ( ((TEST_DATA)->hash_ops             != NULL)  &&    \
         ((TEST_DATA)->hash_ops->value_dtor != NULL) )      \
    {                                                       \
        EXPECT_EQ(old_cnt_hash_ops_val_dtor,                \
                  cnt_hash_ops_val_dtor());                 \
        EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref,           \
                  cnt_hash_ops_val_dtor_vref());            \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(TEST_DATA)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC(TEST_DATA, X)              \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->kref_ops       != NULL) &&               \
             ((TEST_DATA)->kref_ops       != strhash_key_ops) &&    \
             ((TEST_DATA)->kref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_EQ(old_cnt_kref_dtor + (X), cnt_kref_dtor());    \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&           \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )            \
        {                                                           \
            EXPECT_EQ(old_cnt_hash_ops_key_dtor + (X),              \
                      cnt_hash_ops_key_dtor());                     \
            if ((TEST_DATA)->kref_flag) {                           \
                EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref + (X),     \
                          cnt_hash_ops_key_dtor_kref());            \
            } else {                                                \
                EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref,           \
                          cnt_hash_ops_key_dtor_kref());            \
            }                                                       \
        }                                                           \
    }
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC(TEST_DATA, X)              \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->kref_ops       != NULL) &&               \
             ((TEST_DATA)->kref_ops       != strhash_key_ops) &&    \
             ((TEST_DATA)->kref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_LE(old_cnt_kref_dtor + (X), cnt_kref_dtor());    \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&           \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )            \
        {                                                           \
            EXPECT_LE(old_cnt_hash_ops_key_dtor + (X),              \
                      cnt_hash_ops_key_dtor());                     \
            if ((TEST_DATA)->kref_flag) {                           \
                EXPECT_LE(old_cnt_hash_ops_key_dtor_kref + (X),     \
                          cnt_hash_ops_key_dtor_kref());            \
            }                                                       \
        }                                                           \
    }
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC_EXCEPT_REF(TEST_DATA, X)   \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->kref_ops       != NULL) &&               \
             ((TEST_DATA)->kref_ops       != strhash_key_ops) &&    \
             ((TEST_DATA)->kref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_EQ(old_cnt_kref_dtor, cnt_kref_dtor());          \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&           \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )            \
        {                                                           \
            EXPECT_EQ(old_cnt_hash_ops_key_dtor + (X),              \
                      cnt_hash_ops_key_dtor());                     \
            if ((TEST_DATA)->kref_flag) {                           \
                EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref + (X),     \
                          cnt_hash_ops_key_dtor_kref());            \
            } else {                                                \
                EXPECT_EQ(old_cnt_hash_ops_key_dtor_kref,           \
                          cnt_hash_ops_key_dtor_kref());            \
            }                                                       \
        }                                                           \
    }
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_KEYDTOR_INC_EXCEPT_REF(TEST_DATA, X)   \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->kref_ops       != NULL) &&               \
             ((TEST_DATA)->kref_ops       != strhash_key_ops) &&    \
             ((TEST_DATA)->kref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_LE(old_cnt_kref_dtor, cnt_kref_dtor());          \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&           \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )            \
        {                                                           \
            EXPECT_LE(old_cnt_hash_ops_key_dtor + (X),              \
                      cnt_hash_ops_key_dtor());                     \
            if ((TEST_DATA)->kref_flag) {                           \
                EXPECT_LE(old_cnt_hash_ops_key_dtor_kref + (X),     \
                          cnt_hash_ops_key_dtor_kref());            \
            } else {                                                \
                EXPECT_LE(old_cnt_hash_ops_key_dtor_kref,           \
                          cnt_hash_ops_key_dtor_kref());            \
            }                                                       \
        }                                                           \
    }
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_VALDTOR_INC(TEST_DATA, X)              \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->vref_ops       != NULL) &&               \
             ((TEST_DATA)->vref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_EQ(old_cnt_vref_dtor + (X), cnt_vref_dtor());    \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops             != NULL) &&         \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )          \
        {                                                           \
            EXPECT_EQ(old_cnt_hash_ops_val_dtor + (X),              \
                      cnt_hash_ops_val_dtor());                     \
            if ((TEST_DATA)->vref_flag) {                           \
                EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref + (X),     \
                          cnt_hash_ops_val_dtor_vref());            \
            } else {                                                \
                EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref,           \
                          cnt_hash_ops_val_dtor_vref());            \
            }                                                       \
        }                                                           \
    }
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_VALDTOR_INC(TEST_DATA, X)              \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->vref_ops       != NULL) &&               \
             ((TEST_DATA)->vref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_LE(old_cnt_vref_dtor + (X), cnt_vref_dtor());    \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops             != NULL) &&         \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )          \
        {                                                           \
            EXPECT_LE(old_cnt_hash_ops_val_dtor + (X),              \
                      cnt_hash_ops_val_dtor());                     \
            if ((TEST_DATA)->vref_flag) {                           \
                EXPECT_LE(old_cnt_hash_ops_val_dtor_vref + (X),     \
                          cnt_hash_ops_val_dtor_vref());            \
            }                                                       \
        }                                                           \
    }
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_COUNT_VALDTOR_INC_EXCEPT_REF(TEST_DATA, X)   \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->vref_ops       != NULL) &&               \
             ((TEST_DATA)->vref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_EQ(old_cnt_vref_dtor, cnt_vref_dtor());          \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops             != NULL) &&         \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )          \
        {                                                           \
            EXPECT_EQ(old_cnt_hash_ops_val_dtor + (X),              \
                      cnt_hash_ops_val_dtor());                     \
            if ((TEST_DATA)->vref_flag) {                           \
                EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref + (X),     \
                          cnt_hash_ops_val_dtor_vref());            \
            } else {                                                \
                EXPECT_EQ(old_cnt_hash_ops_val_dtor_vref,           \
                          cnt_hash_ops_val_dtor_vref());            \
            }                                                       \
        }                                                           \
    }
#else  /* !PARALLEL_TEST */
#define TESTDATA_CHECK_COUNT_VALDTOR_INC_EXCEPT_REF(TEST_DATA, X)   \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                         \
        if ( ((TEST_DATA)->vref_ops       != NULL) &&               \
             ((TEST_DATA)->vref_ops->dtor != NULL) )                \
        {                                                           \
            EXPECT_LE(old_cnt_vref_dtor, cnt_vref_dtor());          \
        }                                                           \
        if ( ((TEST_DATA)->hash_ops             != NULL) &&         \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )          \
        {                                                           \
            EXPECT_LE(old_cnt_hash_ops_val_dtor + (X),              \
                      cnt_hash_ops_val_dtor());                     \
            if ((TEST_DATA)->vref_flag) {                           \
                EXPECT_LE(old_cnt_hash_ops_val_dtor_vref + (X),     \
                          cnt_hash_ops_val_dtor_vref());            \
            } else {                                                \
                EXPECT_LE(old_cnt_hash_ops_val_dtor_vref,           \
                          cnt_hash_ops_val_dtor_vref());            \
            }                                                       \
        }                                                           \
    }
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_KEYDTOR(TEST_DATA, ADDR)    \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                 \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&   \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )    \
        {                                                   \
            EXPECT_EQ((pfc_cptr_t)(ADDR),                   \
                      lastaddr_hash_ops_key_dtor());        \
        }                                                   \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_KEYDTOR(TEST_DATA, ADDR)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_VALDTOR(TEST_DATA, ADDR)    \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                 \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&   \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )  \
        {                                                   \
            EXPECT_EQ((pfc_cptr_t)(ADDR),                   \
                      lastaddr_hash_ops_val_dtor());        \
        }                                                   \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_VALDTOR(TEST_DATA, ADDR)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_KEYDTOR_REF(TEST_DATA, REF_ADDR, OBJ_ADDR) \
    if (TESTDATA_KEY_HAS_DTOR(TEST_DATA)) {                             \
        if ( ((TEST_DATA)->kref_ops       != NULL) &&                   \
             ((TEST_DATA)->kref_ops       != strhash_key_ops) &&        \
             ((TEST_DATA)->kref_ops->dtor != NULL) )                    \
        {                                                               \
            EXPECT_EQ((pfc_cptr_t)(OBJ_ADDR), lastaddr_kref_dtor());    \
        }                                                               \
        if ( ((TEST_DATA)->hash_ops           != NULL) &&               \
             ((TEST_DATA)->hash_ops->key_dtor != NULL) )                \
        {                                                               \
            EXPECT_EQ((pfc_cptr_t)(REF_ADDR),                           \
                      lastaddr_hash_ops_key_dtor());                    \
        }                                                               \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_KEYDTOR_REF(TEST_DATA, REF_ADDR, OBJ_ADDR)
#endif  /* !PARALLEL_TEST */

#ifndef PARALLEL_TEST
#define TESTDATA_CHECK_LASTADDR_VALDTOR_REF(TEST_DATA, REF_ADDR, OBJ_ADDR) \
    if (TESTDATA_VAL_HAS_DTOR(TEST_DATA)) {                             \
        if ( ((TEST_DATA)->vref_ops       != NULL) &&                   \
             ((TEST_DATA)->vref_ops->dtor != NULL) )                    \
        {                                                               \
            EXPECT_EQ((pfc_cptr_t)(OBJ_ADDR), lastaddr_vref_dtor());    \
        }                                                               \
        if ( ((TEST_DATA)->hash_ops             != NULL) &&             \
             ((TEST_DATA)->hash_ops->value_dtor != NULL) )              \
        {                                                               \
            EXPECT_EQ((pfc_cptr_t)(REF_ADDR),                           \
                      lastaddr_hash_ops_val_dtor());                    \
        }                                                               \
    }
#else   /* !PARALLEL_TEST */
#define TESTDATA_CHECK_LASTADDR_VALDTOR_REF(TEST_DATA, REF_ADDR, OBJ_ADDR)
#endif  /* !PARALLEL_TEST */


/*
 * types
 */

typedef uint32_t (*hashfunc_t)(pfc_cptr_t);
typedef pfc_bool_t (*equals_t)(pfc_cptr_t, pfc_cptr_t);


/*
 * constants
 */

static const int REPLACE___START_MARK              = 0;
static const int REPLACE_KEEP_NULL                 = 0;
static const int REPLACE_KEEP_SAME_REFNULL         = 1;
static const int REPLACE_KEEP_SAME_NONNULL         = 2;
static const int REPLACE_CHANGE_NULL_TO_NONNULL    = 3;
static const int REPLACE_CHANGE_NONNULL_TO_NULL    = 4;
static const int REPLACE_CHANGE_NONNULL_TO_NONNULL = 5;
static const int REPLACE___END_MARK                = 6;

#define FOREACH_REPLACE_TYPE(VAR_NAME)          \
    for (int VAR_NAME = REPLACE___START_MARK;   \
         VAR_NAME < REPLACE___END_MARK;         \
         ++(VAR_NAME))

PFC_ATTR_UNUSED
static const char *REPLACE_NAME[] = {
    "keep (NULL)",
    "keep (ref-NULL)",
    "keep (non-NULL)",
    "change (NULL -> non-NULL)",
    "change (non-NULL -> NULL)",
    "change (non-NULL -> non-NULL)",
};


/*
 * common test rutounes
 */

static int
set_keys (int key_replace_type,
          hashops_test_data_t *test_data,
          pfc_cptr_t *old_keyp,
          pfc_cptr_t *new_keyp,
          pfc_bool_t *free_flagp)
{
    const pfc_refptr_ops_t *kref_ops = test_data->kref_ops;
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    *free_flagp = PFC_FALSE;

    switch (key_replace_type) {
        case REPLACE_KEEP_NULL:
            *old_keyp = NULL;
            *new_keyp = NULL;
            break;
        case REPLACE_KEEP_SAME_REFNULL:
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                pfc_refptr_t *refnull = pfc_refptr_create(kref_ops, NULL);
                *old_keyp = refnull;
                *new_keyp = refnull;
            } else {
                // unsupported
                return 1;
            }
            break;
        case REPLACE_KEEP_SAME_NONNULL:
            if (TESTDATA_KEY_IS_NORMAL_PTR(test_data)) {
                *old_keyp = keys[0];
                keys[0] = NULL;
                if (TESTDATA_KEY_HAS_DTOR(test_data)) {
                    skip_key_dtor[0] = PFC_TRUE;
                    if (kref_ops == strhash_key_ops) {
                        *free_flagp = PFC_TRUE;
                    }
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else {
                *old_keyp = (pfc_cptr_t)krefs[0];
                skip_key_dtor[0] = PFC_TRUE;
            }
            *new_keyp = *old_keyp;
            break;
        case REPLACE_CHANGE_NULL_TO_NONNULL:
        case REPLACE_CHANGE_NONNULL_TO_NULL: {
            // create a non-NULL key that isn't NULL but equals to
            // NULL in the context of `equals()' function
            pfc_cptr_t nonnull_key;
            pfc_cptr_t null_key;
            uint32_t strint_val = STRINT_INTVAL_FOR_NULL;
            if (kref_ops == strint_ops) {
                // unsupported
                return 1;
            } else if (TESTDATA_KEY_IS_NORMAL_STRINT(test_data)) {
                nonnull_key = (pfc_cptr_t)int_to_str(strint_val);
                null_key    = (pfc_cptr_t)NULL;
                if (TESTDATA_KEY_HAS_DTOR(test_data)) {
                    if (kref_ops == strhash_key_ops) {
                        *free_flagp = PFC_TRUE;
                    }
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else {
                // unsupported
                return 1;
            }

            if (key_replace_type == REPLACE_CHANGE_NULL_TO_NONNULL) {
                *old_keyp = null_key;
                *new_keyp = nonnull_key;
            } else {
                *old_keyp = nonnull_key;
                *new_keyp = null_key;
            }

            break;
        }
        case REPLACE_CHANGE_NONNULL_TO_NONNULL: {
            int strint_val = 123;
            const char *str = "key string for replacement";
            if (kref_ops == strint_ops) {
                *old_keyp = (pfc_cptr_t)strint_create_from_int(strint_val);
                *new_keyp = (pfc_cptr_t)strint_create_from_int(strint_val);
            } else if (TESTDATA_KEY_IS_NORMAL_STRINT(test_data)) {
                *old_keyp = (pfc_cptr_t)int_to_str(strint_val);
                *new_keyp = (pfc_cptr_t)int_to_str(strint_val);
                if (TESTDATA_KEY_HAS_DTOR(test_data)) {
                    if (kref_ops == strhash_key_ops) {
                        *free_flagp = PFC_TRUE;
                    }
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else if (kref_ops == strhash_key_ops) {
                *old_keyp = (pfc_cptr_t)strdup(str);
                *new_keyp = (pfc_cptr_t)strdup(str);
                *free_flagp = PFC_TRUE;
            } else {
                // unsupported
                return 1;
            }
            break;
        }
        default:
            fprintf(stderr, "unexpected key replace type (%d)\n",
                    key_replace_type);
            abort();
    }

    return 0;
}


static void
set_vals (int val_replace_type,
          hashops_test_data_t *test_data,
          pfc_cptr_t *old_valp,
          pfc_cptr_t *new_valp,
          pfc_bool_t *free_flagp)
{
    const pfc_refptr_ops_t *vref_ops = test_data->vref_ops;
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    *free_flagp = PFC_FALSE;

    switch (val_replace_type) {
        case REPLACE_KEEP_NULL:
            *old_valp = NULL;
            *new_valp = NULL;
            break;
        case REPLACE_KEEP_SAME_REFNULL:
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                pfc_refptr_t *refnull = pfc_refptr_create(vref_ops, NULL);
                *old_valp = refnull;
                *new_valp = refnull;
            } else {
                // unsupported
                abort();
            }
            break;
        case REPLACE_KEEP_SAME_NONNULL:
            if (TESTDATA_VAL_IS_NORMAL_PTR(test_data)) {
                *old_valp = vals[0][0];
                vals[0][0] = NULL;
                if (TESTDATA_VAL_HAS_DTOR(test_data)) {
                    skip_val_dtor[0][0] = PFC_TRUE;
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else {
                *old_valp = (pfc_cptr_t)vrefs[0][0];
                skip_val_dtor[0][0] = PFC_TRUE;
            }
            *new_valp = *old_valp;
            break;
        case REPLACE_CHANGE_NULL_TO_NONNULL:
        case REPLACE_CHANGE_NONNULL_TO_NULL: {
            pfc_cptr_t nonnull_val;
            if (TESTDATA_VAL_IS_NORMAL_PTR(test_data)) {
                nonnull_val = vals[0][0];
                vals[0][0] = NULL;
                if (TESTDATA_VAL_HAS_DTOR(test_data)) {
                    skip_val_dtor[0][0] = PFC_TRUE;
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else {
                nonnull_val = (pfc_cptr_t)vrefs[0][0];
                skip_val_dtor[0][0] = PFC_TRUE;
            }

            if (val_replace_type == REPLACE_CHANGE_NULL_TO_NONNULL) {
                *old_valp = NULL;
                *new_valp = nonnull_val;
            } else {
                *old_valp = nonnull_val;
                *new_valp = NULL;
            }

            break;
        }
        case REPLACE_CHANGE_NONNULL_TO_NONNULL:
            if (TESTDATA_VAL_IS_NORMAL_PTR(test_data)) {
                *old_valp = vals[0][0];
                *new_valp = vals[0][1];
                vals[0][0] = NULL;
                vals[0][1] = NULL;
                if (TESTDATA_VAL_HAS_DTOR(test_data)) {
                    skip_val_dtor[0][0] = PFC_TRUE;
                    skip_val_dtor[0][1] = PFC_TRUE;
                } else {
                    *free_flagp = PFC_TRUE;
                }
            } else {
                *old_valp = (pfc_cptr_t)vrefs[0][0];
                *new_valp = (pfc_cptr_t)vrefs[0][1];
                skip_val_dtor[0][0] = PFC_TRUE;
                skip_val_dtor[0][1] = PFC_TRUE;
            }
            break;
        default:
            fprintf(stderr, "unexpected val replace type (%d)\n",
                    val_replace_type);
            abort();
    }
}


static void
test_replace_one_entry (pfc_hash_t hash,
                        int nbuckets,
                        const pfc_refptr_ops_t *kref_ops,
                        const pfc_refptr_ops_t *vref_ops,
                        const pfc_hash_ops_t   *hash_ops,
                        int key_replace_type,
                        int val_replace_type)
{
#ifdef  TP_DEBUG_LOG
    fprintf(stderr, "===================================\n");
    int name_depth = 30;
    fprintf(stderr, "try key: %-*s   val: %-*s\n",
            name_depth, REPLACE_NAME[key_replace_type],
            name_depth, REPLACE_NAME[val_replace_type]);
#endif  /* TP_DEBUG_LOG */

    // allocate test data
    hashops_test_data_t *test_data =
            hashops_test_data_alloc(4, 1, kref_ops, vref_ops, hash_ops);
    if (test_data == NULL) abort();
    TESTDATA_DECL_MEMBER_ACCESSOR(test_data);

    if ( (TESTDATA_KEY_IS_NORMAL_PTR(test_data) &&
          (key_replace_type == REPLACE_KEEP_SAME_REFNULL)) ||
         (TESTDATA_VAL_IS_NORMAL_PTR(test_data) &&
          (val_replace_type == REPLACE_KEEP_SAME_REFNULL)) )
    {
        // unsupported
        hashops_test_data_free(test_data);
        return;
    }

    // setup keys and values
    pfc_bool_t flag_key_free = PFC_FALSE;
    pfc_cptr_t old_key, new_key;
    int set_keys_err = set_keys(key_replace_type,
                                test_data,
                                &old_key,
                                &new_key,
                                &flag_key_free);
    if (set_keys_err != 0) {
        hashops_test_data_free(test_data);
        return;
    }

#ifndef	PARALLEL_TEST
    pfc_refptr_t *old_kref = NULL;
    pfc_cptr_t old_kobj = NULL;
    if (TESTDATA_KEY_IS_REFPTR(test_data) && (old_key != NULL)) {
        old_kref = (pfc_refptr_t *)old_key;
        old_kobj = PFC_REFPTR_VALUE(old_kref, pfc_cptr_t);
    }    
#endif	/* !PARALLEL_TEST */

    pfc_bool_t flag_val_free = PFC_FALSE;
    pfc_cptr_t old_val, new_val;
    set_vals(val_replace_type,
             test_data,
             &old_val,
             &new_val,
             &flag_val_free);

#ifndef	PARALLEL_TEST
    pfc_refptr_t *old_vref = NULL;
    pfc_cptr_t old_vobj = NULL;
    if (TESTDATA_VAL_IS_REFPTR(test_data) && (old_val != NULL)) {
        old_vref = (pfc_refptr_t *)old_val;
        old_vobj = PFC_REFPTR_VALUE(old_vref, pfc_cptr_t);
    }    
#endif	/* !PARALLEL_TEST */

    // check TP itself
    if (TESTDATA_KEY_IS_REFPTR(test_data)) {
        if (old_key != NULL) {
            EXPECT_EQ(kref_ops, pfc_refptr_operation((pfc_refptr_t *)old_key));
        }
        if (new_key != NULL) {
            EXPECT_EQ(kref_ops, pfc_refptr_operation((pfc_refptr_t *)new_key));
        }
    }

    switch (key_replace_type) {
        case REPLACE_KEEP_NULL:
            PFC_ASSERT(old_key == new_key);
            PFC_ASSERT(old_key == NULL);
            PFC_ASSERT(new_key == NULL);
            break;
        case REPLACE_KEEP_SAME_REFNULL:
            PFC_ASSERT(old_key == new_key);
            PFC_ASSERT(old_key != NULL);
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                PFC_ATTR_UNUSED pfc_refptr_t *old_kref = (pfc_refptr_t *)old_key;
                PFC_ASSERT(old_kref->pr_refcnt == 1);
                PFC_ASSERT(pfc_refptr_value(old_kref) == NULL);
            }
            break;
        case REPLACE_KEEP_SAME_NONNULL:
            PFC_ASSERT(old_key == new_key);
            PFC_ASSERT(old_key != NULL);
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                PFC_ATTR_UNUSED pfc_refptr_t *old_kref = (pfc_refptr_t *)old_key;
                PFC_ASSERT(old_kref->pr_refcnt == 1);
                PFC_ASSERT(pfc_refptr_value(old_kref) != NULL);
            }
            break;
        case REPLACE_CHANGE_NULL_TO_NONNULL:
            PFC_ASSERT(old_key != new_key);
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_object == NULL);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_object != NULL);
            } else {
                PFC_ASSERT(new_key != NULL);
                PFC_ASSERT(old_key == NULL);
            }
            break;
        case REPLACE_CHANGE_NONNULL_TO_NULL:
            PFC_ASSERT(old_key != new_key);
            PFC_ASSERT(old_key != NULL);
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_object != NULL);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_object == NULL);
            } else {
                PFC_ASSERT(new_key == NULL);
            }
            break;
        case REPLACE_CHANGE_NONNULL_TO_NONNULL:
            PFC_ASSERT(old_key != new_key);
            PFC_ASSERT(new_key != NULL);
            PFC_ASSERT(old_key != NULL);
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)old_key)->pr_object != NULL);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)new_key)->pr_object != NULL);
            }
            break;
        default: abort();
    }

    switch (key_replace_type) {
        case REPLACE_CHANGE_NULL_TO_NONNULL:
        case REPLACE_CHANGE_NONNULL_TO_NULL:
            PFC_ASSERT(kref_ops != strhash_key_ops);
            // keep going to next case
        case REPLACE_CHANGE_NONNULL_TO_NONNULL:
            if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                pfc_refptr_t *old_kref = (pfc_refptr_t *)old_key;
                pfc_refptr_t *new_kref = (pfc_refptr_t *)new_key;
                PFC_ATTR_UNUSED pfc_cptr_t old_obj = pfc_refptr_value(old_kref);
                PFC_ATTR_UNUSED pfc_cptr_t new_obj = pfc_refptr_value(new_kref);

                PFC_ATTR_UNUSED hashfunc_t hashfunc = kref_ops->hashfunc;
                PFC_ASSERT(hashfunc != NULL);
                PFC_ASSERT(hashfunc(old_obj) == hashfunc(new_obj));

                PFC_ATTR_UNUSED equals_t equals = kref_ops->equals;
                PFC_ASSERT(equals   != NULL);
                PFC_ASSERT(equals(old_obj, new_obj));
                PFC_ASSERT((pfc_refptr_equals(old_kref, new_kref)));
            } else {
                if (kref_ops == strhash_key_ops) {
                    PFC_ASSERT(strcmp((char *)old_key, (char *)new_key) == 0);
                } else {
                    PFC_ASSERT(hash_ops != NULL);
                    PFC_ASSERT(hash_ops->equals != NULL);

                    PFC_ATTR_UNUSED hashfunc_t hashfunc = hash_ops->hashfunc;
                    PFC_ASSERT(hashfunc != NULL);
                    PFC_ASSERT(hashfunc(old_key) == hashfunc(new_key));

                    PFC_ATTR_UNUSED equals_t equals = hash_ops->equals;
                    PFC_ASSERT(equals != NULL);
                    PFC_ASSERT(equals(old_key, new_key));
                }
            }
            break;
    }

    if (TESTDATA_VAL_IS_REFPTR(test_data)) {
        if (old_val != NULL) {
            EXPECT_EQ(vref_ops, pfc_refptr_operation((pfc_refptr_t *)old_val));
        }
        if (new_val != NULL) {
            EXPECT_EQ(vref_ops, pfc_refptr_operation((pfc_refptr_t *)new_val));
        }
    }

    switch (val_replace_type) {
        case REPLACE_KEEP_NULL:
            PFC_ASSERT(old_val == new_val);
            PFC_ASSERT(old_val == NULL);
            break;
        case REPLACE_KEEP_SAME_REFNULL:
            PFC_ASSERT(old_val == new_val);
            PFC_ASSERT(old_val != NULL);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                PFC_ATTR_UNUSED pfc_refptr_t *old_vref = (pfc_refptr_t *)old_val;
                PFC_ASSERT(old_vref->pr_refcnt == 1);
                PFC_ASSERT(pfc_refptr_value(old_vref) == NULL);
            }
            break;
        case REPLACE_KEEP_SAME_NONNULL:
            PFC_ASSERT(old_val == new_val);
            PFC_ASSERT(old_val != NULL);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                PFC_ATTR_UNUSED pfc_refptr_t *old_vref = (pfc_refptr_t *)old_val;
                PFC_ASSERT(old_vref->pr_refcnt == 1);
                PFC_ASSERT(pfc_refptr_value(old_vref) != NULL);
            }
            break;
        case REPLACE_CHANGE_NULL_TO_NONNULL:
            PFC_ASSERT(old_val != new_val);
            PFC_ASSERT(old_val == NULL);
            PFC_ASSERT(new_val != NULL);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)new_val)->pr_refcnt == 1);
            }
            break;
        case REPLACE_CHANGE_NONNULL_TO_NULL:
            PFC_ASSERT(old_val != new_val);
            PFC_ASSERT(new_val == NULL);
            PFC_ASSERT(old_val != NULL);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)old_val)->pr_refcnt == 1);
            }
            break;
        case REPLACE_CHANGE_NONNULL_TO_NONNULL:
            PFC_ASSERT(old_val != new_val);
            PFC_ASSERT(new_val != NULL);
            PFC_ASSERT(old_val != NULL);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                PFC_ASSERT(((pfc_refptr_t *)old_val)->pr_refcnt == 1);
                PFC_ASSERT(((pfc_refptr_t *)new_val)->pr_refcnt == 1);
            }
            break;
        default: abort();
    }

    // get hash size
    size_t size_before_put = pfc_hash_get_size(hash);

    // put an old entry
    if (kref_ops == strhash_key_ops) {
        EXPECT_EQ(0, pfc_strhash_put(hash, (char *)old_key, old_val,
                                     kvref_flags));
    } else {
        EXPECT_EQ(0, pfc_hash_put(hash, old_key, old_val, kvref_flags));
    }

    // down refcnt
    if (TESTDATA_KEY_IS_REFPTR(test_data)) {
        if (old_key != NULL) {
            pfc_refptr_put((pfc_refptr_t *)old_key);
            // fprintf(stderr, "put old refptr key (new refcnt = %u)\n",
            //         ((pfc_refptr_t *)old_key)->pr_refcnt);
        }
    }

    if (TESTDATA_VAL_IS_REFPTR(test_data)) {
        if (old_val != NULL) {
            pfc_refptr_put((pfc_refptr_t *)old_val);
            // fprintf(stderr, "put old refptr val (new refcnt = %u)\n",
            //         ((pfc_refptr_t *)old_val)->pr_refcnt);
        }
    }

    // check hash size
    EXPECT_EQ(size_before_put + 1, pfc_hash_get_size(hash));

    // check get with old key
    if (kref_ops == strhash_key_ops) {
        EXPECT_EQ(0, pfc_strhash_get(hash, (char *)old_key, NULL));
    } else {
        EXPECT_EQ(0, pfc_hash_get(hash, old_key, NULL, kvref_flags));
    }

    // check get with new key
    if (kref_ops == strhash_key_ops) {
        EXPECT_EQ(0, pfc_strhash_get(hash, (char *)new_key, NULL));
    } else {
        EXPECT_EQ(0, pfc_hash_get(hash, new_key, NULL, kvref_flags));
    }

    // get old counts
    TESTDATA_GET_OLD_COUNTS(test_data);

    // do replace
    if (kref_ops == strhash_key_ops) {
        EXPECT_EQ(0, pfc_strhash_update(hash, (char *)new_key, new_val,
                                        kvref_flags));
    } else {
        EXPECT_EQ(0, pfc_hash_update(hash, new_key, new_val,
                                     kvref_flags));
    }

    // check hash size
    EXPECT_EQ(size_before_put + 1, pfc_hash_get_size(hash));

    // check key dtor counts
    if (kref_ops != strhash_key_ops) {
        switch (key_replace_type) {
            case REPLACE_KEEP_NULL:
            case REPLACE_KEEP_SAME_REFNULL:
            case REPLACE_KEEP_SAME_NONNULL:
                TESTDATA_CHECK_COUNT_KEYDTOR_UNCHANGED(test_data);
                break;
            case REPLACE_CHANGE_NULL_TO_NONNULL:
                if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                    TESTDATA_CHECK_COUNT_KEYDTOR_INC_EXCEPT_REF(test_data, 1);
                } else {
                    TESTDATA_CHECK_COUNT_KEYDTOR_INC(test_data, 1);
                }
                break;
            case REPLACE_CHANGE_NONNULL_TO_NULL:
            case REPLACE_CHANGE_NONNULL_TO_NONNULL:
                TESTDATA_CHECK_COUNT_KEYDTOR_INC(test_data, 1);
                if (TESTDATA_KEY_IS_REFPTR(test_data)) {
                    TESTDATA_CHECK_LASTADDR_KEYDTOR_REF(test_data,
                                                        old_kref, old_kobj);
                } else {
                    TESTDATA_CHECK_LASTADDR_KEYDTOR(test_data, old_key);
                }
                break;
            default:
                fprintf(stderr, "unexpected key replace type (%d)\n",
                        key_replace_type);
                abort();
        }
    }

    // check val dtor counts
    switch (val_replace_type) {
        case REPLACE_KEEP_NULL:
        case REPLACE_KEEP_SAME_REFNULL:
        case REPLACE_KEEP_SAME_NONNULL:
            TESTDATA_CHECK_COUNT_VALDTOR_UNCHANGED(test_data);
            break;
        case REPLACE_CHANGE_NULL_TO_NONNULL:
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                TESTDATA_CHECK_COUNT_VALDTOR_INC_EXCEPT_REF(test_data, 1);
            } else {
                TESTDATA_CHECK_COUNT_VALDTOR_INC(test_data, 1);
            }
            break;
        case REPLACE_CHANGE_NONNULL_TO_NULL:
        case REPLACE_CHANGE_NONNULL_TO_NONNULL:
            TESTDATA_CHECK_COUNT_VALDTOR_INC(test_data, 1);
            if (TESTDATA_VAL_IS_REFPTR(test_data)) {
                TESTDATA_CHECK_LASTADDR_VALDTOR_REF(test_data,
                                                    old_vref, old_vobj);
            } else {
                TESTDATA_CHECK_LASTADDR_VALDTOR(test_data, old_val);
            }
            break;
        default:
            fprintf(stderr, "unexpected val replace type (%d)\n",
                    val_replace_type);
            abort();
    }

    // down refcnt
    if (TESTDATA_KEY_IS_REFPTR(test_data)) {
        if ((new_key != NULL) && (new_key != old_key)) {
            pfc_refptr_put((pfc_refptr_t *)new_key);
            // fprintf(stderr, "put new refptr key (new refcnt = %u)\n",
            //         ((pfc_refptr_t *)new_key)->pr_refcnt);
        }
    }

    if (TESTDATA_VAL_IS_REFPTR(test_data)) {
        if ((new_val != NULL) && (new_val != old_val)) {
            pfc_refptr_put((pfc_refptr_t *)new_val);
            // fprintf(stderr, "put new refptr val (new refcnt = %u)\n",
            //         ((pfc_refptr_t *)new_val)->pr_refcnt);
        }
    }

    // clean up
    // fprintf(stderr, "-- pfc_hash_clear -----------------\n");
    pfc_hash_clear(hash);

    if (flag_key_free) {
        if (old_key != NULL) {
            free((pfc_ptr_t)old_key);
        }

        if ((new_key != NULL) && (new_key != old_key)) {
            free((pfc_ptr_t)new_key);
        }
    }

    if (flag_val_free) {
        if (old_val != NULL) {
            free((pfc_ptr_t)old_val);
        }

        if ((new_val != NULL) && (new_val != old_val)) {
            free((pfc_ptr_t)new_val);
        }
    }

    // fprintf(stderr, "-- hashops_test_data_free ---------\n");
    hashops_test_data_free(test_data);

#ifdef  TP_DEBUG_LOG
    int name_depth = 30;
    fprintf(stderr, "succ  key: %-*s   val: %-*s\n",
            name_depth, REPLACE_NAME[key_replace_type],
            name_depth, REPLACE_NAME[val_replace_type]);
#endif  /* TP_DEBUG_LOG */
}


static void
test_replace_entries (const pfc_refptr_ops_t *kref_ops,
                      const pfc_refptr_ops_t *vref_ops,
                      const pfc_hash_ops_t   *hash_ops)
{
    // create hash tabule
    int nbuckets = 31;
    pfc_hash_t hash;
    if (kref_ops == NULL) {
        EXPECT_EQ(0, pfc_hash_create(&hash, hash_ops, nbuckets, 0));
    } else if (kref_ops == strhash_key_ops) {
        EXPECT_EQ(0, pfc_strhash_create(&hash, hash_ops, nbuckets, 0));
    } else if (kref_ops != NULL) {
        EXPECT_EQ(0, pfc_refhash_create(&hash, hash_ops, kref_ops,
                                        nbuckets, 0));
    }

    // call pfc_hash_update for each mode
    FOREACH_REPLACE_TYPE(key_replace_type) {
        FOREACH_REPLACE_TYPE(val_replace_type) {
            test_replace_one_entry(hash, nbuckets,
                                   kref_ops, vref_ops, hash_ops,
                                   key_replace_type, val_replace_type);
        }
    }

    // clean up hash table
    pfc_hash_destroy(hash);
}


/*
 * test cases
 */

TEST(hash, replace_kref_vref_hashops)
{
    test_replace_entries(strint_ops, intref_ops, hash_wtchops_kref_vref);
}

TEST(hash, replace_kref_vref)
{
    test_replace_entries(strint_ops, intref_ops, NULL);
}

TEST(hash, replace_kref_hashops)
{
    test_replace_entries(strint_ops, NULL, hash_wtchops_kptr);
}

TEST(hash, replace_str_vref_hashops)
{
    test_replace_entries(strhash_key_ops, intref_ops, hash_wtchops_kptr_vref);
}

TEST(hash, replace_str_vref)
{
    test_replace_entries(strhash_key_ops, intref_ops, NULL);
}

TEST(hash, replace_str_hashops)
{
    test_replace_entries(strhash_key_ops, NULL, hash_wtchops_kptr);
}

TEST(hash, replace_vref_hashops)
{
    test_replace_entries(NULL, intref_ops, hash_wtchops_ksintp_vref);
}

TEST(hash, replace_vref)
{
    test_replace_entries(NULL, intref_ops, NULL);
}

TEST(hash, replace_hashops)
{
    test_replace_entries(NULL, NULL, hash_wtchops_ksintp_vptr);
}
