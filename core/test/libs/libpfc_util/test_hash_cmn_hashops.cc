/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Common test routines for hash that's created with user hash_ops
 */

#include <string.h>
#include "pseudo_rand.hh"
#include "string_util.h"
#include "test_hash_cmn_hashops.hh"


/*
 * private prototypes
 */

static void hashops_test_data_key_free(hashops_test_data_t *test_data,
                                       int i);
static void hashops_test_data_val_free(hashops_test_data_t *test_data,
                                       int n, int i);
static void hashops_test_data_kref_free(hashops_test_data_t *test_data,
                                        int i);
static void hashops_test_data_vref_free(hashops_test_data_t *test_data,
                                        int n, int i);


/*
 * define shorter name version
 */

#define KEY_IS_NORMAL_STRINT HASHOPS_TESTDATA_KEY_IS_NORMAL_STRINT
#define KEY_IS_NORMAL_PTR    HASHOPS_TESTDATA_KEY_IS_NORMAL_PTR
#define KEY_IS_REFPTR        HASHOPS_TESTDATA_KEY_IS_REFPTR
#define VAL_IS_NORMAL_PTR    HASHOPS_TESTDATA_VAL_IS_NORMAL_PTR
#define VAL_IS_REFPTR        HASHOPS_TESTDATA_VAL_IS_REFPTR
#define KEY_HAS_DTOR         HASHOPS_TESTDATA_KEY_HAS_DTOR
#define VAL_HAS_DTOR         HASHOPS_TESTDATA_VAL_HAS_DTOR
#define DECL_MEMBER_ACCESSOR HASHOPS_TESTDATA_DECL_MEMBER_ACCESSOR


/*
 * implementations
 */

pfc_refptr_ops_t *strhash_key_ops = NULL;

static void PFC_FATTR_INIT
init (void)
{
    // set strhash_key_ops
    pfc_refptr_t *str_refptr = pfc_refptr_string_create("");
    strhash_key_ops = (pfc_refptr_ops_t *)pfc_refptr_operation(str_refptr);
    pfc_refptr_put(str_refptr);
}


hashops_test_data_t *
hashops_test_data_alloc (int nents,
                         int val_nsets,
                         const pfc_refptr_ops_t *kref_ops,
                         const pfc_refptr_ops_t *vref_ops,
                         const pfc_hash_ops_t   *hash_ops)
{
    prand_generator_t prand_gen = prand_create(HASH_RANDOM_SEED);

    PFC_ASSERT(val_nsets >= 0);
    PFC_ASSERT(nents > 0);
    hashops_test_data_t *test_data =
            (hashops_test_data_t *)calloc(1, sizeof(hashops_test_data_t));

    // store arguments
    test_data->nents = nents;
    test_data->val_nsets = val_nsets;
    test_data->hash_ops = hash_ops;
    test_data->kref_ops = kref_ops;
    test_data->vref_ops = vref_ops;

    // check key types
    pfc_bool_t key_is_normal_ptr     = PFC_FALSE;
    pfc_bool_t key_is_normal_strint  = PFC_FALSE;
    pfc_bool_t key_is_strhash_string = PFC_FALSE;
    pfc_bool_t key_is_refptr_strint  = PFC_FALSE;
    if (KEY_IS_NORMAL_STRINT(test_data)) {
        key_is_normal_strint = PFC_TRUE;
    } else if (kref_ops == NULL) {
        key_is_normal_ptr = PFC_TRUE;
    } else if (kref_ops == strhash_key_ops) {
        key_is_strhash_string = PFC_TRUE;
    } else if (kref_ops == strint_ops) {
        key_is_refptr_strint = PFC_TRUE;
    } else {
        fprintf(stderr, "Unexpected key type (init)\n");
        abort();
    }

    // check value types
    pfc_bool_t val_is_normal_ptr    = PFC_FALSE;
    pfc_bool_t val_is_normal_strint = PFC_FALSE;
    pfc_bool_t val_is_refptr_int    = PFC_FALSE;
    if (vref_ops == NULL) {
        if (hash_ops == strint_kv_hash_ops) {
            val_is_normal_strint = PFC_TRUE;
        } else {
            val_is_normal_ptr = PFC_TRUE;
        }
    } else if (vref_ops == intref_ops) {
        val_is_refptr_int = PFC_TRUE;
    } else {
        fprintf(stderr, "Unexpected value type (init)\n");
        abort();
    }

    // set key / value refptr flags
    if ((kref_ops != NULL) && (kref_ops != strhash_key_ops)) {
        test_data->kref_flag = PFC_HASHOP_KEY_REFPTR;
    }
    if (vref_ops != NULL) {
        test_data->vref_flag = PFC_HASHOP_VAL_REFPTR;
    }
    test_data->kvref_flags = test_data->kref_flag | test_data->vref_flag;

    // allocate keys
    if (key_is_normal_ptr || key_is_strhash_string) {
        // allocate random string keys
        char **keys = (char **)calloc(nents, sizeof(char *));
        PFC_ASSERT(keys != NULL);
        for (int i = 0; i < nents; ++i) {
          retry:
            keys[i] = random_string_with_prand_gen(prand_gen, 100, '!', '~');
            PFC_ASSERT(keys[i] != NULL);
            for (int j = 0; j < i; ++j) {
                if (strcmp(keys[i], keys[j]) == 0) {
                    free(keys[i]);
                    goto retry;
                }
            }
        }
        test_data->keys = (pfc_cptr_t *)keys;
    } else if (key_is_normal_strint) {
        // allocate strint keys (non-refptr)
        char **keys = (char **)calloc(nents, sizeof(char *));
        PFC_ASSERT(keys != NULL);
        for (int i = 0; i < nents; ++i) {
            keys[i] = int_to_str(i);
            PFC_ASSERT(keys[i] != NULL);
        }
        test_data->keys = (pfc_cptr_t *)keys;
    } else if (key_is_refptr_strint) {
        // allocate strint refptr keys
        pfc_refptr_t **krefs =
                (pfc_refptr_t **)calloc(nents, sizeof(pfc_refptr_t *));
        PFC_ASSERT(krefs != NULL);
        for (int i = 0; i < nents; ++i) {
            krefs[i] = strint_create_from_int(i);
            PFC_ASSERT(krefs[i] != NULL);
        }
        test_data->krefs = krefs;
    } else {
        fprintf(stderr, "Unexpected key type (alloc key)\n");
        abort();
    }

    // allocate values
    if (val_is_normal_ptr) {
        // allocate nomarl pointer values
        pfc_cptr_t **vals = NULL;
        if (val_nsets > 0) {
            vals = (pfc_cptr_t **)calloc(val_nsets, sizeof(pfc_cptr_t *));
            PFC_ASSERT(vals != NULL);
            for (int n = 0; n < val_nsets; ++n) {
                vals[n] = (pfc_cptr_t *)calloc(nents, sizeof(pfc_cptr_t));
                for (int i = 0; i < nents; ++i) {
                    int *p = (int *)calloc(nents, sizeof(int));
                    PFC_ASSERT(p != NULL);
                    *p = n * nents + i;
                    vals[n][i] = (pfc_cptr_t)p;
                }
            }
        }
        test_data->vals = vals;
        if (vals != NULL) {
            test_data->vals_at_head = vals[0];
            test_data->vals_at_tail = vals[val_nsets-1];
        }
    } else if (val_is_normal_strint) {
        // allocate normal strint values (non-refptr)
        char ***vals = NULL;
        if (val_nsets > 0) {
            vals = (char ***)calloc(val_nsets, sizeof(char **));
            PFC_ASSERT(vals != NULL);
            for (int n = 0; n < val_nsets; ++n) {
                vals[n] = (char **)calloc(nents, sizeof(char *));
                for (int i = 0; i < nents; ++i) {
                    vals[n][i] = int_to_str(nents * n + i);
                    PFC_ASSERT(vals[n][i] != NULL);
                }
            }
        }
        test_data->vals = (pfc_cptr_t **)vals;
        if (vals != NULL) {
            test_data->vals_at_head = (pfc_cptr_t *)vals[0];
            test_data->vals_at_tail = (pfc_cptr_t *)vals[val_nsets-1];
        }
    } else if (val_is_refptr_int) {
        // allocate intref values
        pfc_refptr_t ***vrefs = NULL;
        if (val_nsets > 0) {
            vrefs = (pfc_refptr_t ***)calloc(val_nsets,
                                             sizeof(pfc_refptr_t **));
            PFC_ASSERT(vrefs != NULL);
            for (int n = 0; n < val_nsets; ++n) {
                vrefs[n] = (pfc_refptr_t **)calloc(nents,
                                                   sizeof(pfc_refptr_t *));
                for (int i = 0; i < nents; ++i) {
                    vrefs[n][i] = intref_create(nents * n + i);
                    PFC_ASSERT(vrefs[n][i] != NULL);
                }
            }
        }
        test_data->vrefs = vrefs;
        if (vrefs != NULL) {
            test_data->vrefs_at_head = vrefs[0];
            test_data->vrefs_at_tail = vrefs[val_nsets - 1];
        }
    } else {
        fprintf(stderr, "Unexpected value type (alloc val)\n");
        abort();
    }

    // key destructor control flags
    if (KEY_HAS_DTOR(test_data)) {
        test_data->skip_key_dtor =
                (pfc_bool_t *)calloc(nents, sizeof(pfc_bool_t));
    }

    // value destructor control flags
    if (VAL_HAS_DTOR(test_data)) {
        pfc_bool_t **skip_val_dtor = NULL;
        if (val_nsets > 0) {
            skip_val_dtor =
                    (pfc_bool_t **)calloc(val_nsets, sizeof(pfc_bool_t *));
            for (int n = 0; n < val_nsets; ++n) {
                skip_val_dtor[n] =
                        (pfc_bool_t *)calloc(nents, sizeof(pfc_bool_t));
            }
        }
        test_data->skip_val_dtor = skip_val_dtor;
    }

    // allocate a set of permanent keys
    if (key_is_normal_strint || key_is_strhash_string) {
        char **permanent_str_keys = (char **)calloc(nents, sizeof(char *));
        PFC_ASSERT(permanent_str_keys != NULL);
        for (int i = 0; i < nents; ++i) {
            if (test_data->keys[i] != NULL) {
                permanent_str_keys[i] = strdup((char *)test_data->keys[i]);
                PFC_ASSERT(permanent_str_keys[i] != NULL);
            }
        }
        test_data->permanent_str_keys = permanent_str_keys;
    } else if (key_is_refptr_strint) {
        char **permanent_str_keys = (char **)calloc(nents, sizeof(char *));
        PFC_ASSERT(permanent_str_keys != NULL);
        for (int i = 0; i < nents; ++i) {
            if (test_data->krefs[i] != NULL) {
                const char *s = PFC_REFPTR_VALUE(test_data->krefs[i],
                                                 const char *);
                permanent_str_keys[i] = strdup(s);
                PFC_ASSERT(permanent_str_keys[i] != NULL);
            }
        }
        test_data->permanent_str_keys = permanent_str_keys;
    }

    // set getter functions for refptr keys
    if (kref_ops == NULL || kref_ops == strhash_key_ops) {
        ; // nothing to do
    } else if (kref_ops == strint_ops) {
        test_data->get_count_kref_dtor     = strint_get_count_dtor;
        test_data->get_count_kref_equals   = strint_get_count_equals;
        test_data->get_count_kref_hashfunc = strint_get_count_hashfunc;
        test_data->get_lastaddr_kref_dtor  = strint_get_dtor_last_addr;
    } else {
        fprintf(stderr, "Unexpected kref_ops (set kref getter)\n");
        abort();
    }

    // set getter functions for refptr values
    if (vref_ops == NULL) {
        ; // nothing to do
    } else if (vref_ops == intref_ops) {
        test_data->get_count_vref_dtor    = intref_get_count_dtor;
        test_data->get_lastaddr_vref_dtor = intref_get_lastaddr_dtor;
    } else {
        fprintf(stderr, "Unexpected vref_ops (set vref getter)\n");
        abort();
    }

    // set getter functions associated with hash_ops
    if (hash_ops == NULL) {
        ; // no thing to do
    } else if (hash_ops == strint_kv_hash_ops) {
        // set counter functions for hash watching operators
        test_data->get_count_hash_ops_equals   = strint_get_count_equals;
        test_data->get_count_hash_ops_hashfunc = strint_get_count_hashfunc;
        test_data->get_count_hash_ops_key_dtor = strint_get_count_key_dtor;
        test_data->get_count_hash_ops_val_dtor = strint_get_count_val_dtor;
        test_data->get_count_hash_ops_key_dtor_kref =
                strint_get_count_key_dtor_kref;
        test_data->get_count_hash_ops_val_dtor_vref =
                strint_get_count_val_dtor_vref;

        // set last target address functions
        test_data->get_lastaddr_hash_ops_key_dtor =
                strint_get_key_dtor_last_addr;
        test_data->get_lastaddr_hash_ops_val_dtor =
                strint_get_val_dtor_last_addr;
    } else if (IS_HASH_WTCHOPS_FAMILY(hash_ops)) {
        // set counter functions for hash watching operators
        test_data->get_count_hash_ops_equals   = wtchops_get_count_equals;
        test_data->get_count_hash_ops_hashfunc = wtchops_get_count_hashfunc;
        test_data->get_count_hash_ops_key_dtor = wtchops_get_count_key_dtor;
        test_data->get_count_hash_ops_val_dtor = wtchops_get_count_val_dtor;
        test_data->get_count_hash_ops_key_dtor_kref =
                wtchops_get_count_key_dtor_kref;
        test_data->get_count_hash_ops_val_dtor_vref =
                wtchops_get_count_val_dtor_vref;

        // set last target address functions
        test_data->get_lastaddr_hash_ops_key_dtor =
                wtchops_get_lastaddr_key_dtor;
        test_data->get_lastaddr_hash_ops_val_dtor =
                wtchops_get_lastaddr_val_dtor;
    } else {
        fprintf(stderr, "Unexpected hash_ops (set hashops getter)\n");
        abort();
    }

    // clean up
    prand_destroy(prand_gen);

    return test_data;
}


void
hashops_test_data_free (hashops_test_data_t *test_data)
{
    int nents     = test_data->nents;
    int val_nsets = test_data->val_nsets;
    DECL_MEMBER_ACCESSOR(test_data);

    // release normal values
    if (vals != NULL) {
        for (int n = 0; n < val_nsets; n++) {
            if (vals[n] != NULL) {
                for (int i = 0; i < nents; ++i) {
                    hashops_test_data_val_free(test_data, n, i);
                }
                free(vals[n]);
            }
        }
        free(vals);
    }

    // release refptr values
    if (vrefs != NULL) {
        for (int n = 0; n < val_nsets; n++) {
            if (vrefs[n] != NULL) {
                for (int i = 0; i < nents; ++i) {
                    hashops_test_data_vref_free(test_data, n, i);
                }
                free(vrefs[n]);
            }
        }
        free(vrefs);
    }

    // release normal keys
    if (keys != NULL) {
        for (int i = 0; i < nents; ++i) {
            hashops_test_data_key_free(test_data, i);
        }
        free(keys);
    }

    // release refptr keys
    if (krefs != NULL) {
        for (int i = 0; i < nents; ++i) {
            hashops_test_data_kref_free(test_data, i);
        }
        free(krefs);
    }

    // release key / value dtor control flags
    if (skip_val_dtor != NULL) {
        for (int n = 0; n < val_nsets; n++) {
            if (skip_val_dtor[n] != NULL) {
                free(skip_val_dtor[n]);
            }
        }
        free(skip_val_dtor);
    }

    if (skip_key_dtor != NULL) {
        free(skip_key_dtor);
    }

    // release permanent keys
    if (permanent_str_keys != NULL) {
        for (int i = 0; i < nents; ++i) {
            if (permanent_str_keys[i] != NULL) {
                free(permanent_str_keys[i]);
            }
        }
        free(permanent_str_keys);
    }

    // release top
    free(test_data);
}


static void
hashops_test_data_key_free (hashops_test_data_t *test_data, int i)
{
    // fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
    PFC_ATTR_UNUSED const pfc_refptr_ops_t *kref_ops = test_data->kref_ops;
    PFC_ATTR_UNUSED const pfc_hash_ops_t   *hash_ops = test_data->hash_ops;
    DECL_MEMBER_ACCESSOR(test_data);
    // fprintf(stderr, "%s(%d) kvref_flags = %x\n",
    //         __FILE__, __LINE__, kvref_flags);
    // fprintf(stderr, "%s(%d) KEY_HAS_DTOR(test_data) = %d\n",
    //         __FILE__, __LINE__, KEY_HAS_DTOR(test_data));

    if (KEY_HAS_DTOR(test_data)) {
        if ((keys[i] != NULL) && !skip_key_dtor[i]) {
            if ((hash_ops != NULL) && (hash_ops->key_dtor != NULL)) {
                hash_ops->key_dtor((pfc_ptr_t)keys[i], kvref_flags);
            }
            keys[i] = NULL;
            skip_key_dtor[i] = PFC_TRUE;
        }
    } else {
        if (keys[i] != NULL) {
            free((pfc_ptr_t)keys[i]);
            keys[i] = NULL;
        }
    }
}


static void
hashops_test_data_val_free (hashops_test_data_t *test_data, int n, int i)
{
    // fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
    PFC_ATTR_UNUSED const pfc_hash_ops_t   *hash_ops = test_data->hash_ops;
    DECL_MEMBER_ACCESSOR(test_data);
    // fprintf(stderr, "%s(%d) kvref_flags = %x\n",
    //         __FILE__, __LINE__, kvref_flags);
    // fprintf(stderr, "%s(%d) VAL_HAS_DTOR(test_data) = %d\n",
    //         __FILE__, __LINE__, VAL_HAS_DTOR(test_data));

    if (VAL_HAS_DTOR(test_data)) {
        if ((vals[n][i] != NULL) && !skip_val_dtor[n][i]) {
            if ((hash_ops != NULL) && (hash_ops->value_dtor != NULL)) {
                hash_ops->value_dtor((pfc_ptr_t)vals[n][i], kvref_flags);
            }
            vals[n][i] = NULL;
            skip_val_dtor[n][i] = PFC_TRUE;
        }
    } else {
        if (vals[n][i] != NULL) {
            free((pfc_ptr_t)vals[n][i]);
            vals[n][i] = NULL;
        }
    }
}


static void
hashops_test_data_kref_free (hashops_test_data_t *test_data, int i)
{
    // fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
    PFC_ATTR_UNUSED const pfc_refptr_ops_t *kref_ops = test_data->kref_ops;
    PFC_ATTR_UNUSED const pfc_hash_ops_t   *hash_ops = test_data->hash_ops;
    DECL_MEMBER_ACCESSOR(test_data);
    // fprintf(stderr, "%s(%d) kvref_flags = %x\n",
    //         __FILE__, __LINE__, kvref_flags);
    // fprintf(stderr, "%s(%d) KEY_HAS_DTOR(test_data) = %d\n",
    //         __FILE__, __LINE__, KEY_HAS_DTOR(test_data));

    if (KEY_HAS_DTOR(test_data)) {
        if ((krefs[i] != NULL) && !skip_key_dtor[i]) {
            PFC_ASSERT(((pfc_refptr_t *)krefs[i])->pr_refcnt == 1);
            if ((hash_ops != NULL) && (hash_ops->key_dtor != NULL)) {
                hash_ops->key_dtor(krefs[i], kvref_flags);
            }
            PFC_ASSERT(((pfc_refptr_t *)krefs[i])->pr_refcnt == 1);
            pfc_refptr_put(krefs[i]);
            krefs[i] = NULL;
            skip_key_dtor[i] = PFC_TRUE;
        }
    } else {
        if (krefs[i] != NULL) {
            pfc_refptr_put(krefs[i]);
            krefs[i] = NULL;
        }
    }
}


static void
hashops_test_data_vref_free (hashops_test_data_t *test_data, int n, int i)
{
    // fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
    PFC_ATTR_UNUSED const pfc_refptr_ops_t *vref_ops = test_data->vref_ops;
    PFC_ATTR_UNUSED const pfc_hash_ops_t   *hash_ops = test_data->hash_ops;
    DECL_MEMBER_ACCESSOR(test_data);
    // fprintf(stderr, "%s(%d) kvref_flags = %x\n",
    //         __FILE__, __LINE__, kvref_flags);
    // fprintf(stderr, "%s(%d) VAL_HAS_DTOR(test_data) = %d\n",
    //         __FILE__, __LINE__, VAL_HAS_DTOR(test_data));

    if (VAL_HAS_DTOR(test_data)) {
        if ((vrefs[n][i] != NULL) && !skip_val_dtor[n][i]) {
            PFC_ASSERT(((pfc_refptr_t *)vrefs[n][i])->pr_refcnt == 1);
            if ((hash_ops != NULL) && (hash_ops->value_dtor != NULL)) {
                hash_ops->value_dtor(vrefs[n][i], kvref_flags);
            }
            PFC_ASSERT(((pfc_refptr_t *)vrefs[n][i])->pr_refcnt == 1);
            pfc_refptr_put(vrefs[n][i]);
            vrefs[n][i] = NULL;
            skip_val_dtor[n][i] = PFC_TRUE;
        }
    } else {
        if (vrefs[n][i] != NULL) {
            pfc_refptr_put(vrefs[n][i]);
            vrefs[n][i] = NULL;
        }
    }
}


void
hashops_test_data_inject_null (hashops_test_data_t *test_data)
{
    int nents     = test_data->nents;
    int val_nsets = test_data->val_nsets;
    DECL_MEMBER_ACCESSOR(test_data);

    // keep `nents and' `val_nsets' more than '2'
    if (nents < 2) abort();
    if (val_nsets < 2) abort();

    // inject a NULL key case
    int null_k_idx = 0;
    if (KEY_IS_NORMAL_PTR(test_data)) {
        hashops_test_data_key_free(test_data, null_k_idx);
        keys[null_k_idx] = NULL;
    } else if (KEY_IS_REFPTR(test_data)) {
        hashops_test_data_kref_free(test_data, null_k_idx);
        krefs[null_k_idx] = NULL;
    } else {
        fprintf(stderr, "Unexpected key type\n");
        abort();
    }

    // inject NULL value cases
    for (int i = 0; i < nents; ++i) {
        for (int n = 0; n < val_nsets; ++n) {
            if ((n % 2 + i % 2) == 1) {
                if (VAL_IS_NORMAL_PTR(test_data)) {
                    hashops_test_data_val_free(test_data, n, i);
                    vals[n][i] = NULL;
                } else if (VAL_IS_REFPTR(test_data)) {
                    hashops_test_data_vref_free(test_data, n, i);
                    vrefs[n][i] = NULL;
                } else {
                    fprintf(stderr, "Unexpected value type\n");
                    abort();
                }
            }
        }
    }
}
