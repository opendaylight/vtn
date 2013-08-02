/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_HASH_H
#define	_PFC_HASH_H

/*
 * Definitions for hash table implementation.
 */

#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/syncblock.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

PFC_C_BEGIN_DECL

/*
 * Hash table instance.
 */
struct __pfc_hash;
typedef struct __pfc_hash	*pfc_hash_t;

/*
 * Invalid hash table instance.
 */
#define	PFC_HASH_INVALID		((pfc_hash_t)NULL)

/*
 * Iterator of hash table entries.
 */
struct __pfc_hashiter;
typedef struct __pfc_hashiter	*pfc_hashiter_t;

/*
 * Hash table operations.
 *
 * Remarks:
 *	Don't call PFC hash interface from below operations.
 *	Calling PFC hash interface from hash operations may cause deadlock.
 */
typedef struct {
	/*
	 * pfc_bool_t
	 * equals(pfc_cptr_t o1, pfc_cptr_t o2)  (optional)
	 *	Determine whether the given two hash keys are identical.
	 *	PFC_TRUE must be returned if identical, otherwise PFC_FALSE.
	 *
	 *	If NULL is specified, comparison is done using key addresses.
	 */
	pfc_bool_t	(*equals)(pfc_cptr_t o1, pfc_cptr_t o2);

	/*
	 * uint32_t
	 * hashfunc(pfc_cptr_t key)  (optional)
	 *	Return hash value associated with the given hash key.
	 *	Note that hash value must be immutable.
	 *
	 *	If NULL is specified, hash value is derived from key address.
	 */
	uint32_t	(*hashfunc)(pfc_cptr_t key);

	/*
	 * void
	 * key_dtor(pfc_ptr_t key, uint32_t flags)  (optional)
	 *	Destructor of hash key.
	 *	This function is called when a hash key is removed from
	 *	the hash table. Removed key is passed to `key'.
	 *	NULL can be specified to tell the hash table that no destructor
	 *	is implemented for the hash key.
	 *
	 *	PFC_HASHOP_KEY_REFPTR is set in flags if the specified key is
	 *	a refptr object. Note that its reference counter is managed
	 *	by hash table layer, so this destructor must not change its
	 *	reference counter.
	 */
	void		(*key_dtor)(pfc_ptr_t key, uint32_t flags);

	/*
	 * void
	 * value_dtor(pfc_ptr_t value, uint32_t flags)  (optional)
	 *	Destructor of hash value.
	 *	This function is called when a hash value is removed from
	 *	the hash table. Remove value is passed to `value'.
	 *	NULL can be specified to tell the hash table that no destructor
	 *	is implemented for the hash value.
	 *
	 *	PFC_HASHOP_VAL_REFPTR is set in flags if the specified value is
	 *	a refptr object. Note that its reference counter is managed
	 *	by hash table layer, so this destructor must not change its
	 *	reference counter.
	 */
	void		(*value_dtor)(pfc_ptr_t value, uint32_t flags);
} pfc_hash_ops_t;

/*
 * Hash operation flags.
 */
#define	PFC_HASHOP_KEY_REFPTR	PFC_CONST_U(0x1)	/* key is refptr */
#define	PFC_HASHOP_VAL_REFPTR	PFC_CONST_U(0x2)	/* value is refptr */
#define	PFC_HASHOP_KEYVAL_REFPTR			\
	(PFC_HASHOP_KEY_REFPTR | PFC_HASHOP_VAL_REFPTR)

/*
 * Flags for pfc_hash_create() / pfc_refhash_create().
 */

/*
 * Disable auto-lock mode.
 * By default, all hash table operations are serialized by internal lock.
 * If this flag is specified, you must serialize any hash table operation
 * by yourself because hash table layer doesn't take care of any serialization.
 */
#define	PFC_HASH_NOLOCK		PFC_CONST_U(0x1)

/*
 * Adjust number of hash buckets to the largest prime number within the
 * specified number of buckets.
 */
#define	PFC_HASH_PRIME		PFC_CONST_U(0x2)

/*
 * Maximum number of hash buckets.
 */
#define	PFC_HASH_MAX_NBUCKETS	PFC_CONST_U(0x1000000)

/*
 * Prototypes.
 *
 * Remarks:
 *	Functions which have '__pfc_' prefix are not public.
 */
extern int	pfc_hash_create(pfc_hash_t *PFC_RESTRICT hashp,
				const pfc_hash_ops_t *PFC_RESTRICT ops,
				uint32_t nbuckets, uint32_t flags);
extern int	pfc_refhash_create(pfc_hash_t *PFC_RESTRICT hashp,
				   const pfc_hash_ops_t *PFC_RESTRICT ops,
				   const pfc_refptr_ops_t *PFC_RESTRICT kops,
				   uint32_t nbuckets, uint32_t flags);
extern int	pfc_hash_put(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
			     pfc_cptr_t value, uint32_t opflags);
extern int	pfc_hash_update(pfc_hash_t PFC_RESTRICT hash,
				pfc_cptr_t key, pfc_cptr_t value,
				uint32_t opflags);
extern int	pfc_hash_remove(pfc_hash_t PFC_RESTRICT hash,
				pfc_cptr_t PFC_RESTRICT key, uint32_t opflags);
extern int	__pfc_hash_delete(pfc_hash_t PFC_RESTRICT hash,
				  pfc_cptr_t PFC_RESTRICT key,
				  pfc_cptr_t *PFC_RESTRICT valuep,
				  uint32_t opflags);
extern int	__pfc_hash_get(pfc_hash_t PFC_RESTRICT hash,
			       pfc_cptr_t PFC_RESTRICT key,
			       pfc_cptr_t *PFC_RESTRICT valuep,
			       uint32_t opflags);
extern size_t	pfc_hash_clear(pfc_hash_t hash);
extern size_t	pfc_hash_get_size(pfc_hash_t hash);
extern uint32_t	pfc_hash_get_capacity(pfc_hash_t hash);
extern int	pfc_hash_set_capacity(pfc_hash_t hash, uint32_t nbuckets);
extern void	__pfc_hash_sync_begin(__pfc_syncblock_t *frame);
extern void	__pfc_hash_sync_end(void *arg);
extern void	pfc_hash_destroy(pfc_hash_t hash);

extern pfc_hashiter_t	pfc_hashiter_get(pfc_hash_t hash);
extern int	__pfc_hashiter_next(pfc_hashiter_t PFC_RESTRICT it,
				    pfc_cptr_t *PFC_RESTRICT keyp,
				    pfc_cptr_t *PFC_RESTRICT valuep);
extern int	pfc_hashiter_remove(pfc_hashiter_t it);
extern void	pfc_hashiter_put(pfc_hashiter_t it);

/*
 * The following hash operations are implemented as inline function
 * to embed destination object size check.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_get(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
	     pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_hash_get(hash, key, valuep, opflags);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_delete(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
		pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_hash_delete(hash, key, valuep, opflags);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hashiter_next(pfc_hashiter_t PFC_RESTRICT it,
		  pfc_cptr_t *PFC_RESTRICT keyp,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(keyp, sizeof(pfc_cptr_t));
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_hashiter_next(it, keyp, valuep);
}

/*
 * Hash operation wrappers for refptr hash table.
 * - `kref' means that key is refptr object.
 * - `vref' means that value is refptr object.
 * - `kvref' means that both key and value are refptr object.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_put_kref(pfc_hash_t PFC_RESTRICT hash, pfc_refptr_t *key,
		  pfc_cptr_t value)
{
	return pfc_hash_put(hash, (pfc_cptr_t)key, value,
			    PFC_HASHOP_KEY_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_put_vref(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
		  pfc_refptr_t *value)
{
	return pfc_hash_put(hash, key, (pfc_cptr_t)value,
			    PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_put_kvref(pfc_hash_t PFC_RESTRICT hash, pfc_refptr_t *key,
		   pfc_refptr_t *value)
{
	return pfc_hash_put(hash, (pfc_cptr_t)key, (pfc_cptr_t)value,
			    PFC_HASHOP_KEYVAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_update_kref(pfc_hash_t PFC_RESTRICT hash, pfc_refptr_t *key,
		     pfc_cptr_t value)
{
	return pfc_hash_update(hash, (pfc_cptr_t)key, value,
			       PFC_HASHOP_KEY_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_update_vref(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
		     pfc_refptr_t *value)
{
	return pfc_hash_update(hash, key, (pfc_cptr_t)value,
			       PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_update_kvref(pfc_hash_t PFC_RESTRICT hash, pfc_refptr_t *key,
		      pfc_refptr_t *value)
{
	return pfc_hash_update(hash, (pfc_cptr_t)key, (pfc_cptr_t)value,
			       PFC_HASHOP_KEYVAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_remove_kref(pfc_hash_t PFC_RESTRICT hash,
		     const pfc_refptr_t *PFC_RESTRICT key)
{
	return pfc_hash_remove(hash, (pfc_cptr_t)key, PFC_HASHOP_KEY_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_delete_kref(pfc_hash_t PFC_RESTRICT hash,
		     const pfc_refptr_t *PFC_RESTRICT key,
		     pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hash_delete(hash, (pfc_cptr_t)key, valuep,
			       PFC_HASHOP_KEY_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hash_get_kref(pfc_hash_t PFC_RESTRICT hash,
		  const pfc_refptr_t *PFC_RESTRICT key,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hash_get(hash, (pfc_cptr_t)key, valuep,
			    PFC_HASHOP_KEY_REFPTR);
}

/*
 * Builtin hash table - string hash
 * The hash key must be a refptr object created by pfc_refptr_string_create().
 */
extern int	pfc_strhash_create(pfc_hash_t *PFC_RESTRICT hashp,
				   const pfc_hash_ops_t *PFC_RESTRICT ops,
				   uint32_t nbuckets, uint32_t flags);
extern int	pfc_strhash_put(pfc_hash_t PFC_RESTRICT hash, const char *key,
				pfc_cptr_t value, uint32_t opflags);
extern int	pfc_strhash_update(pfc_hash_t PFC_RESTRICT hash,
				   const char *key, pfc_cptr_t value,
				   uint32_t opflags);

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_strhash_put_vref(pfc_hash_t PFC_RESTRICT hash, const char *key,
		     pfc_refptr_t *value)
{
	return pfc_strhash_put(hash, key, (pfc_cptr_t )value,
			       PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_strhash_update_vref(pfc_hash_t PFC_RESTRICT hash, const char *key,
			pfc_refptr_t *value)
{
	return pfc_strhash_update(hash, key, (pfc_cptr_t)value,
				  PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_strhash_remove(pfc_hash_t PFC_RESTRICT hash, const char *PFC_RESTRICT key)
{
	return pfc_hash_remove(hash, (pfc_cptr_t)key, 0);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_strhash_delete(pfc_hash_t PFC_RESTRICT hash, const char *PFC_RESTRICT key,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hash_delete(hash, (pfc_cptr_t)key, valuep, 0);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_strhash_get(pfc_hash_t PFC_RESTRICT hash, const char *PFC_RESTRICT key,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hash_get(hash, (pfc_cptr_t)key, valuep, 0);
}

/*
 * Builtin hash table - uint64 hash
 * Internal hash table implementation depends on the system ILP model.
 * uint64 hash must be operated by uint64-specialized operations listed below.
 */
extern int	pfc_u64hash_create(pfc_hash_t *PFC_RESTRICT hashp,
				   const pfc_hash_ops_t *PFC_RESTRICT ops,
				   uint32_t nbuckets, uint32_t flags);
extern int	pfc_u64hash_put(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
				pfc_cptr_t value, uint32_t opflags);
extern int	pfc_u64hash_update(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
				   pfc_cptr_t value, uint32_t opflags);
extern int	pfc_u64hash_remove(pfc_hash_t PFC_RESTRICT hash, uint64_t key);
extern int	__pfc_u64hash_delete(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
				     pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_u64hash_get(pfc_hash_t PFC_RESTRICT hash,
				  uint64_t key,
				  pfc_cptr_t *PFC_RESTRICT valuep);
extern int	__pfc_hashiter_uint64_next(pfc_hashiter_t PFC_RESTRICT it,
					   uint64_t *PFC_RESTRICT keyp,
					   pfc_cptr_t *PFC_RESTRICT valuep);

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_u64hash_put_vref(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		     pfc_refptr_t *value)
{
	return pfc_u64hash_put(hash, key, (pfc_cptr_t )value,
			       PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_u64hash_update_vref(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
			pfc_refptr_t *value)
{
	return pfc_u64hash_update(hash, key, (pfc_cptr_t)value,
				  PFC_HASHOP_VAL_REFPTR);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_u64hash_delete(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_u64hash_delete(hash, key, valuep);
}

static inline int PFC_FATTR_ALWAYS_INLINE
pfc_u64hash_get(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_u64hash_get(hash, key, valuep);
}

/*
 * uint64 hash specific hash entry iterator.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_hashiter_uint64_next(pfc_hashiter_t PFC_RESTRICT it,
			 uint64_t *PFC_RESTRICT keyp,
			 pfc_cptr_t *PFC_RESTRICT valuep)
{
	PFC_PTR_OBJSIZE_ASSERT(keyp, sizeof(uint64_t));
	PFC_PTR_OBJSIZE_ASSERT(valuep, sizeof(pfc_cptr_t));

	return __pfc_hashiter_uint64_next(it, keyp, valuep);
}

/*
 * Start hash table synchronized block.
 * pfc_hash_wsync_begin() starts synchronized block as writer mode,
 * pfc_hash_rsync_begin() starts as reader.
 *
 * pfc_hash_wsync_begin(), pfc_hash_rsync_begin() and pfc_hash_sync_end()
 * are implemented as macros and must always be used in matching pairs
 * at the same block.
 */
#define	pfc_hash_wsync_begin(hash)				\
	__pfc_wsync_block_begin(hash, __pfc_hash_sync_begin,	\
				__pfc_hash_sync_end)

#define	pfc_hash_rsync_begin(hash)				\
	__pfc_rsync_block_begin(hash, __pfc_hash_sync_begin,	\
				__pfc_hash_sync_end)

#define	pfc_hash_sync_end()	__pfc_sync_block_end()

extern void	pfc_hash_report(pfc_hash_t PFC_RESTRICT hash,
				FILE *PFC_RESTRICT fp);
extern void	pfc_hash_report_summary(pfc_hash_t PFC_RESTRICT hash,
					FILE *PFC_RESTRICT fp,
					uint32_t nsummaries);

PFC_C_END_DECL

#endif	/* !_PFC_HASH_H */
