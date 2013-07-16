/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _TEST_STRINT_H
#define _TEST_STRINT_H

/*
 * Definitions for strint.
 *   Strint is an integer represented in string for testing
 *   data models using refptr
 */

#include <pfc/refptr.h>
#include <pfc/hash.h>

PFC_C_BEGIN_DECL

#define STRINT_INTVAL_FOR_NULL   ((uint32_t)-1)

extern const pfc_refptr_ops_t *strint_ops;
extern const pfc_hash_ops_t *strint_kv_hash_ops;

extern uint64_t strint_get_count_dtor();
extern uint64_t strint_get_count_key_dtor();
extern uint64_t strint_get_count_key_dtor_kref();
extern uint64_t strint_get_count_val_dtor();
extern uint64_t strint_get_count_val_dtor_vref();
extern uint64_t strint_get_count_compare();
extern uint64_t strint_get_count_equals();
extern uint64_t strint_get_count_hashfunc();

extern pfc_cptr_t strint_get_dtor_last_addr();
extern pfc_cptr_t strint_get_key_dtor_last_addr();
extern pfc_cptr_t strint_get_val_dtor_last_addr();

extern pfc_refptr_t *strint_create_from_int(uint32_t i);

extern void       strint_dtor(pfc_ptr_t sint_obj);
extern void       strint_hash_key_dtor(pfc_ptr_t sint_obj, uint32_t flags);
extern void       strint_hash_val_dtor(pfc_ptr_t sint_obj, uint32_t flags);
extern int        strint_compare(pfc_cptr_t sint_obj1, pfc_cptr_t sint_obj2);
extern pfc_bool_t strint_equals(pfc_cptr_t sint_obj1, pfc_cptr_t sint_obj2);
extern uint32_t   strint_hashfunc(pfc_cptr_t sint_obj);

extern char *   str_of_strint_refptr(pfc_refptr_t *sint_refp);
extern uint32_t int_of_strint_refptr(pfc_refptr_t *sint_refp);

extern uint32_t str_to_int(const char *s);
extern char *   int_to_str(uint32_t i);

PFC_C_END_DECL

#endif /* !_TEST_STRINT_H */
