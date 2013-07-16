/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_HASH_CMN_HASHOPS_HH
#define _TEST_HASH_CMN_HASHOPS_HH

/*
 * Header file of common test routines for hash that's created with
 * user hash_ops
 */

#include <pfc/hash.h>
#include <pfc/refptr.h>
#include "strint_refptr.h"
#include "hash_util.h"
#include "test_hash.hh"


PFC_C_BEGIN_DECL

/*
 * Import string refptr ops
 */

extern pfc_refptr_ops_t *strhash_key_ops;


/*
 * Constants
 */

#define INVALID_PTR     ((pfc_cptr_t)-123)


/*
 * Type for test data
 */

typedef uint64_t (*cnt_func_t)(void);
typedef pfc_cptr_t (*lastaddr_func_t)(void);

typedef struct {
    // data size
    int nents;
    int val_nsets;

    // operators
    const pfc_refptr_ops_t *kref_ops;
    const pfc_refptr_ops_t *vref_ops;
    const pfc_hash_ops_t *hash_ops;

    // normal pointer keys and values
    // char  **keys;
    // char ***vals;
    // char **vals_at_head;
    // char **vals_at_tail;
    pfc_cptr_t  *keys;
    pfc_cptr_t **vals;
    pfc_cptr_t  *vals_at_head;
    pfc_cptr_t  *vals_at_tail;

    // key / value refptr flags
    uint32_t kref_flag;
    uint32_t vref_flag;
    uint32_t kvref_flags;

    // refptr kesy and values
    pfc_refptr_t  **krefs;
    pfc_refptr_t ***vrefs;
    pfc_refptr_t  **vrefs_at_head;
    pfc_refptr_t  **vrefs_at_tail;

    pfc_bool_t  *skip_key_dtor;
    pfc_bool_t **skip_val_dtor;

    // keys not under control of destructors
    char **permanent_str_keys;

    // functions to get counters
    cnt_func_t get_count_kref_dtor;
    cnt_func_t get_count_kref_equals;
    cnt_func_t get_count_kref_hashfunc;

    cnt_func_t get_count_vref_dtor;

    cnt_func_t get_count_hash_ops_equals;
    cnt_func_t get_count_hash_ops_hashfunc;
    cnt_func_t get_count_hash_ops_key_dtor;
    cnt_func_t get_count_hash_ops_key_dtor_kref;
    cnt_func_t get_count_hash_ops_val_dtor;
    cnt_func_t get_count_hash_ops_val_dtor_vref;

    // functions to get last target address
    lastaddr_func_t get_lastaddr_kref_dtor;
    lastaddr_func_t get_lastaddr_vref_dtor;
    lastaddr_func_t get_lastaddr_hash_ops_key_dtor;
    lastaddr_func_t get_lastaddr_hash_ops_val_dtor;

    // flags to check TP itself
    struct {
        bool put_unreg_key;
        bool put_existing_key;
        bool put_null_key;
        bool put_null_value;
        bool put_null_key_value;

        bool update_unreg_key;
        bool update_existing_key;
        bool update_null_key;
        bool update_null_value;
        bool update_null_key_value;

        bool get_unreg_key;
        bool get_existing_key;
        bool get_existing_key_check;
        bool get_null_key;
        bool get_null_valuep;
        bool get_null_key_valuep;

        bool remove_unreg_key;
        bool remove_existing_key;
    } tp_flags;
} hashops_test_data_t;

typedef enum {
    INPUT_OP_PUT,
    INPUT_OP_UPDATE,
    INPUT_OP_MIX,
} input_op_t;


/*
 * Prototypes for test data
 */

extern hashops_test_data_t *
hashops_test_data_alloc (int nents,
                         int val_nsets,
                         const pfc_refptr_ops_t *kref_ops,
                         const pfc_refptr_ops_t *vref_ops,
                         const pfc_hash_ops_t   *hash_ops);

extern void
hashops_test_data_free (hashops_test_data_t *test_data);

extern void
hashops_test_data_inject_null (hashops_test_data_t *test_data);


/*
 * Macros for test data
 */

#define HASHOPS_SET_FLAG(test_data, flag)       \
    ((test_data)->tp_flags.flag = true)

#define HASHOPS_TESTDATA_KEY_IS_NORMAL_STRINT(TEST_DATA)    \
    (((TEST_DATA)->kref_ops == NULL) &&                     \
     ( ((TEST_DATA)->hash_ops == strint_kv_hash_ops) ||     \
       IS_HASH_WTCHOPS_FAMILY((TEST_DATA)->hash_ops) ))

#define HASHOPS_TESTDATA_KEY_IS_NORMAL_PTR(TEST_DATA)               \
    (((TEST_DATA)->keys != NULL) && ((TEST_DATA)->krefs == NULL))

#define HASHOPS_TESTDATA_KEY_IS_REFPTR(TEST_DATA)                   \
    (((TEST_DATA)->keys == NULL) && ((TEST_DATA)->krefs != NULL))

#define HASHOPS_TESTDATA_VAL_IS_NORMAL_PTR(TEST_DATA)               \
    (((TEST_DATA)->vals != NULL) && ((TEST_DATA)->vrefs == NULL))

#define HASHOPS_TESTDATA_VAL_IS_REFPTR(TEST_DATA)                   \
    (((TEST_DATA)->vals == NULL) && ((TEST_DATA)->vrefs != NULL))

#define HASHOPS_TESTDATA_KEY_HAS_DTOR(TEST_DATA)            \
    ( ( ((TEST_DATA)->kref_ops       != NULL) &&            \
        ((TEST_DATA)->kref_ops       != strhash_key_ops) && \
        ((TEST_DATA)->kref_ops->dtor != NULL) )             \
      ||                                                    \
      ( ((TEST_DATA)->hash_ops           != NULL) &&        \
        ((TEST_DATA)->hash_ops->key_dtor != NULL) ) )

#define HASHOPS_TESTDATA_VAL_HAS_DTOR(TEST_DATA)        \
    ( ( ((TEST_DATA)->vref_ops       != NULL) &&        \
        ((TEST_DATA)->vref_ops->dtor != NULL) )         \
      ||                                                \
      ( ((TEST_DATA)->hash_ops             != NULL) &&  \
        ((TEST_DATA)->hash_ops->value_dtor != NULL) ) )

#define HASHOPS_TESTDATA_KREF_REFCNT(TEST_DATA, i)  \
    ((TEST_DATA)->krefs[i]->pr_refcnt)              \

#define HASHOPS_TESTDATA_VREF_REFCNT(TEST_DATA, n, i)   \
    ((TEST_DATA)->vrefs[n][i]->pr_refcnt)               \

#define HASHOPS_TESTDATA_KREF_AUTOPUT(TEST_DATA, i)     \
    do {                                                \
        if (!(TEST_DATA)->skip_key_dtor[i]) {           \
            if ((TEST_DATA)->kref_flag) {               \
                pfc_refptr_put((TEST_DATA)->krefs[i]);  \
            }                                           \
            (TEST_DATA)->skip_key_dtor[i] = PFC_TRUE;   \
        };                                              \
    } while(0)                                          \

#define HASHOPS_TESTDATA_VREF_AUTOPUT(TEST_DATA, n, i)      \
    do {                                                    \
        if (!(TEST_DATA)->skip_val_dtor[n][i]) {            \
            if ((TEST_DATA)->vref_flag) {                   \
                pfc_refptr_put((TEST_DATA)->vrefs[n][i]);   \
            }                                               \
            (TEST_DATA)->skip_val_dtor[n][i] = PFC_TRUE;    \
        };                                                  \
    } while(0)                                              \

#define HASHOPS_TESTDATA_DECL_MEMBER_ACCESSOR(TEST_DATA)            \
    PFC_ATTR_UNUSED                                                 \
    pfc_cptr_t  *keys          = (TEST_DATA)->keys;                 \
    PFC_ATTR_UNUSED                                                 \
    pfc_cptr_t **vals          = (TEST_DATA)->vals;                 \
    PFC_ATTR_UNUSED                                                 \
    pfc_cptr_t  *vals_at_head  = (TEST_DATA)->vals_at_head;         \
    PFC_ATTR_UNUSED                                                 \
    pfc_cptr_t  *vals_at_tail  = (TEST_DATA)->vals_at_tail;         \
    PFC_ATTR_UNUSED                                                 \
    uint32_t kref_flag         = (TEST_DATA)->kref_flag;            \
    PFC_ATTR_UNUSED                                                 \
    uint32_t vref_flag         = (TEST_DATA)->vref_flag;            \
    PFC_ATTR_UNUSED                                                 \
    uint32_t kvref_flags       = (TEST_DATA)->kvref_flags;          \
    PFC_ATTR_UNUSED                                                 \
    pfc_refptr_t **krefs       = (TEST_DATA)->krefs;                \
    PFC_ATTR_UNUSED                                                 \
    pfc_refptr_t ***vrefs      = (TEST_DATA)->vrefs;                \
    PFC_ATTR_UNUSED                                                 \
    pfc_refptr_t  **vrefs_at_head = (TEST_DATA)->vrefs_at_head;     \
    PFC_ATTR_UNUSED                                                 \
    pfc_refptr_t  **vrefs_at_tail = (TEST_DATA)->vrefs_at_tail;     \
    PFC_ATTR_UNUSED                                                 \
    pfc_bool_t  *skip_key_dtor = (TEST_DATA)->skip_key_dtor;        \
    PFC_ATTR_UNUSED                                                 \
    pfc_bool_t **skip_val_dtor = (TEST_DATA)->skip_val_dtor;        \
    PFC_ATTR_UNUSED                                                 \
    char **permanent_str_keys = (TEST_DATA)->permanent_str_keys;    \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_kref_dtor =                                      \
            (TEST_DATA)->get_count_kref_dtor;                       \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_kref_equals =                                    \
            (TEST_DATA)->get_count_kref_equals;                     \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_kref_hashfunc =                                  \
            (TEST_DATA)->get_count_kref_hashfunc;                   \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_vref_dtor =                                      \
            (TEST_DATA)->get_count_vref_dtor;                       \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_equals =                                \
            (TEST_DATA)->get_count_hash_ops_equals;                 \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_hashfunc =                              \
            (TEST_DATA)->get_count_hash_ops_hashfunc;               \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_key_dtor =                              \
            (TEST_DATA)->get_count_hash_ops_key_dtor;               \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_val_dtor =                              \
            (TEST_DATA)->get_count_hash_ops_val_dtor;               \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_key_dtor_kref =                         \
            (TEST_DATA)->get_count_hash_ops_key_dtor_kref;          \
    PFC_ATTR_UNUSED                                                 \
    cnt_func_t cnt_hash_ops_val_dtor_vref =                         \
            (TEST_DATA)->get_count_hash_ops_val_dtor_vref;          \
    PFC_ATTR_UNUSED                                                 \
    lastaddr_func_t lastaddr_kref_dtor =                            \
            (TEST_DATA)->get_lastaddr_kref_dtor;                    \
    PFC_ATTR_UNUSED                                                 \
    lastaddr_func_t lastaddr_hash_ops_key_dtor =                    \
            (TEST_DATA)->get_lastaddr_hash_ops_key_dtor;            \
    PFC_ATTR_UNUSED                                                 \
    lastaddr_func_t lastaddr_vref_dtor =                            \
            (TEST_DATA)->get_lastaddr_vref_dtor;                    \
    PFC_ATTR_UNUSED                                                 \
    lastaddr_func_t lastaddr_hash_ops_val_dtor =                    \
            (TEST_DATA)->get_lastaddr_hash_ops_val_dtor;

#define HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, COUNTER_NAME) \
    PFC_ATTR_UNUSED uint64_t old_cnt_ ## COUNTER_NAME =             \
            (((TEST_DATA)->get_count_ ## COUNTER_NAME != NULL)      \
             ? (TEST_DATA)->get_count_ ## COUNTER_NAME()            \
             : 0);

#define HASHOPS_TESTDATA_GET_OLD_COUNTS(TEST_DATA)                      \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, kref_dtor);           \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, kref_equals);         \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, kref_hashfunc);       \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, vref_dtor);           \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_equals);     \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_hashfunc);   \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_key_dtor);   \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_val_dtor);   \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_key_dtor_kref); \
    HASHOPS_TESTDATA_GET_ONE_OLD_COUNT(TEST_DATA, hash_ops_val_dtor_vref);


PFC_C_END_DECL

#endif  /* !_TEST_HASH_CMN_HASHOPS_HH */
