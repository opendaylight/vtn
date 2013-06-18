/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_HASH_IMPL_H
#define	_PFC_LIBPFC_UTIL_HASH_IMPL_H

/*
 * Internal definitions for hash table implementation.
 */

#include <pfc/base.h>
#include <pfc/list.h>
#include <pfc/hash.h>
#include <pfc/synch.h>
#include "refptr_impl.h"
#include "listmodel_impl.h"

PFC_C_BEGIN_DECL

/*
 * Special value destructor used by hash list.
 */
struct hashlist;
typedef struct hashlist		hashlist_t;
typedef void	(*hashlist_dtor_t)(hashlist_t *hlp, pfc_cptr_t value);

/*
 * A pair of key and value in the hash table.
 */
struct pfc_hashent;
typedef struct pfc_hashent	pfc_hashent_t;

struct pfc_hashent {
	pfc_cptr_t	phe_key;	/* hash key */
	pfc_cptr_t	phe_value;	/* value associated with the key */
	pfc_hashent_t	*phe_next;	/* link for hash collision chain */
	uint32_t	phe_flags;	/* hash entry flags */
};

/*
 * Hash table instance.
 */
typedef struct {
	pfc_hashent_t		**ph_table;	/* hash table */
	const pfc_hash_ops_t	*ph_ops;	/* hash table operations */
	const pfc_refptr_ops_t	*ph_kops;	/* refptr ops for hash key */
	size_t			ph_nelems;	/* number of elements */
	pfc_list_t		ph_iterator;	/* iterator list */
	pfc_rwlock_t		ph_lock;	/* table lock */
	uint32_t		ph_nbuckets;	/* number of hash buckets */
	uint32_t		ph_flags;	/* hash table attributes */
	uint32_t		ph_gen;		/* generation number */
	uint32_t		ph_refcnt;	/* reference counter */
} pfc_hashtbl_t;

/*
 * Element of the hash list.
 */
typedef struct {
	pfc_list_t		hle_list;	/* element link */
	pfc_cptr_t		hle_value;	/* value of element */
} hlist_elem_t;

/*
 * Hash list instance.
 */
struct hashlist {
	listmodel_t		hl_model;	/* common list model. */
	pfc_hashtbl_t		hl_hash;	/* internal hash table */
	pfc_list_t		hl_head;	/* list head */
	hashlist_dtor_t		hl_elem_dtor;	/* element destructor */
	pfc_rwlock_t		hl_lock;	/* read/write lock */
	uint32_t		hl_hopflags;	/* hash operation flags */
};

/*
 * Internal hash table flags.
 */
#define	PFC_IHASH_U64		0x80000000U	/* uint64 hash */
#define	PFC_IHASH_LIST		0x40000000U	/* hash list internal */
#define	PFC_IHASH_FLAGS_MASK	0x00ffffffU	/* public flag mask */

/*
 * Determine whether the given table is a uint64 hash or not.
 */
#define	PFC_HASH_IS_U64(hash)					\
	(((pfc_hashtbl_t *)(hash))->ph_flags & PFC_IHASH_U64)

/*
 * Determine whether the given table is hash list internal table.
 */
#define	PFC_HASH_IS_LIST(hash)					\
	(((pfc_hashtbl_t *)(hash))->ph_flags & PFC_IHASH_LIST)

/*
 * Convert list internal table address into list address.
 */
#define	PFC_HASH_TO_LIST(hash)				\
	PFC_CAST_CONTAINER(hash, hashlist_t, hl_hash)

/*
 * Prototypes.
 */
extern int	pfc_hash_table_init(pfc_hash_t *PFC_RESTRICT hashp,
				    const pfc_hash_ops_t *PFC_RESTRICT ops,
				    const pfc_refptr_ops_t *PFC_RESTRICT kops,
				    uint32_t nbuckets, uint32_t flags);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_HASH_IMPL_H */
