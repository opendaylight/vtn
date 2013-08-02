/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _HASH_UTIL_H
#define _HASH_UTIL_H

/*
 * Definitionof of hash test utilities
 */

#include <pfc/refptr.h>
#include <pfc/hash.h>

PFC_C_BEGIN_DECL

/*
 * sets of hash operator functions
 */

extern const pfc_hash_ops_t *hash_wtchops_kv_nofree;
extern const pfc_hash_ops_t *hash_wtchops_kptr;
extern const pfc_hash_ops_t *hash_wtchops_vptr;
extern const pfc_hash_ops_t *hash_wtchops_kref;
extern const pfc_hash_ops_t *hash_wtchops_vref;
extern const pfc_hash_ops_t *hash_wtchops_kptr_vptr;
extern const pfc_hash_ops_t *hash_wtchops_kptr_vref;
extern const pfc_hash_ops_t *hash_wtchops_kref_vptr;
extern const pfc_hash_ops_t *hash_wtchops_kref_vref;

/* `ksintp' measn `Key is StrINT normal Pointer' */
extern const pfc_hash_ops_t *hash_wtchops_ksintp_vptr;
extern const pfc_hash_ops_t *hash_wtchops_ksintp_vref;

#define IS_HASH_WTCHOPS_FAMILY(hash_ops)            \
    ( ((hash_ops) == hash_wtchops_kv_nofree)   ||   \
      ((hash_ops) == hash_wtchops_kptr)        ||   \
      ((hash_ops) == hash_wtchops_vptr)        ||   \
      ((hash_ops) == hash_wtchops_kref)        ||   \
      ((hash_ops) == hash_wtchops_vref)        ||   \
      ((hash_ops) == hash_wtchops_kptr_vptr)   ||   \
      ((hash_ops) == hash_wtchops_kptr_vref)   ||   \
      ((hash_ops) == hash_wtchops_kref_vptr)   ||   \
      ((hash_ops) == hash_wtchops_kref_vref)   ||   \
      ((hash_ops) == hash_wtchops_ksintp_vptr) ||   \
      ((hash_ops) == hash_wtchops_ksintp_vref) )

extern uint64_t   wtchops_get_count_equals();
extern uint64_t   wtchops_get_count_hashfunc();
extern uint64_t   wtchops_get_count_key_dtor();
extern uint64_t   wtchops_get_count_key_dtor_kref();
extern uint64_t   wtchops_get_count_val_dtor();
extern uint64_t   wtchops_get_count_val_dtor_vref();

extern pfc_cptr_t wtchops_get_lastaddr_key_dtor();
extern pfc_cptr_t wtchops_get_lastaddr_val_dtor();


/*
 * trivial refptr operator functions for hash values
 */

extern const pfc_refptr_ops_t *intref_ops;
extern pfc_refptr_t *intref_create(int x);

extern uint64_t   intref_get_count_dtor();
extern pfc_cptr_t intref_get_lastaddr_dtor();

PFC_C_END_DECL

#endif /* !_HASH_UTIL_H */
