/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * hash.c - Hash table implementation.
 */

#include <string.h>
#include <pfc/hash.h>
#include <pfc/list.h>
#include <pfc/atomic.h>
#include <pfc/synch.h>
#include "refptr_impl.h"
#include "syncblock_impl.h"
#include "hash_impl.h"

/*
 * Internal hash operation flags.
 */
#define	PFC_IHASHOP_UPDATE	0x80000000U	/* update existing entry */
#define	PFC_IHASHOP_FLAGS	0xff000000U	/* reserved hashop flags */

/*
 * Hash value for NULL refptr key.
 */
#define	PFC_HASHVAL_NULL	1U

/*
 * Options for pfc_hash_entry_ref() and pfc_hash_entry_unref().
 */
#define	ENTREF_IGNORE_KEY	0x1U		/* don't touch hash key */
#define	ENTREF_IGNORE_VALUE	0x2U		/* don't touch hash value */

/*
 * Iterator of hash entries.
 */
typedef struct {
	pfc_list_t	phi_list;	/* iterator list */
	pfc_hashtbl_t	*phi_hash;	/* hash table */
	pfc_hashent_t	**phi_prev;	/* previous hash entry */
	pfc_hashent_t	**phi_next;	/* next hash entry */
	uint32_t	phi_index;	/* current bucket index */
	uint32_t	phi_gen;	/* generation number */
} hash_iter_t;

/*
 * Internal prototypes.
 */
static int	pfc_hash_put_entry(pfc_hash_t PFC_RESTRICT hash,
				   pfc_cptr_t key, pfc_cptr_t value,
				   uint32_t opflags);
static void	pfc_hash_replace_entry(pfc_hashtbl_t *PFC_RESTRICT hp,
				       pfc_hashent_t *PFC_RESTRICT oldentp,
				       pfc_hashent_t *PFC_RESTRICT newentp);
static int	pfc_hash_do_remove(pfc_hash_t PFC_RESTRICT hash,
				   pfc_cptr_t PFC_RESTRICT key,
				   pfc_cptr_t *PFC_RESTRICT valuep,
				   uint32_t opflags);
static void	pfc_hash_entry_unref(pfc_hashtbl_t *PFC_RESTRICT hp,
				     pfc_hashent_t *PFC_RESTRICT entp,
				     uint32_t options);
static pfc_hashent_t	*pfc_hash_entry_alloc(pfc_cptr_t key, pfc_cptr_t value,
					      uint32_t flags);
static pfc_hashent_t	**pfc_hash_lookup(pfc_hashtbl_t *hp, pfc_cptr_t key,
					  uint32_t index);
static pfc_hashent_t	**pfc_hash_lookup_refptr(pfc_hashtbl_t *hp,
						 pfc_refptr_t *key,
						 uint32_t index);
static size_t		pfc_hash_clear_l(pfc_hashtbl_t *hp);
static int		pfc_hashiter_get_next(hash_iter_t *PFC_RESTRICT iter,
					      pfc_hashtbl_t *PFC_RESTRICT hp,
					      pfc_cptr_t *PFC_RESTRICT keyp,
					      pfc_cptr_t *PFC_RESTRICT valuep);
static pfc_bool_t	pfc_hash_is_prime(uint32_t number);
static uint32_t		pfc_hash_get_prime(uint32_t max);

/*
 * Dummy hash operation which contains no method.
 * This is used if no hash operation is specified.
 */
static const pfc_hash_ops_t	hashop_default;

/*
 * Synchronized block frame created by pfc_hash_wsync_begin() or
 * pfc_hash_rsync_begin(). This list is thread private, so we don't need
 * any lock to access this list.
 */
static __thread __pfc_syncblock_t	*hash_sync_frame;

/*
 * static inline __pfc_syncblock_t *
 * pfc_hashsync_lookup(pfc_hashtbl_t *hp)
 *	Search for a hash synchronized block on the current context for
 *	the specified hash table
 *
 * Calling/Exit State:
 *	If found, a pointer to synchronized block frame is returned.
 *	NULL is returned if found.
 */
static inline __pfc_syncblock_t *
pfc_hashsync_lookup(pfc_hashtbl_t *hp)
{
	return pfc_syncblock_lookup(hash_sync_frame, (pfc_cptr_t)hp);
}

/*
 * static inline int
 * pfc_hash_rdlock(pfc_hashtbl_t *hp)
 *	Acquire hash table lock in reader mode, if needed.
 *	Returned value must be passed to pfc_hash_unlock().
 */
static inline int
pfc_hash_rdlock(pfc_hashtbl_t *hp)
{
	int	cookie;

	/*
	 * We don't need to check synchronized block mode because write lock
	 * implies read lock.
	 */
	if ((hp->ph_flags & PFC_HASH_NOLOCK) == 0 &&
	    pfc_hashsync_lookup(hp) == NULL) {
		(void)pfc_rwlock_rdlock(&hp->ph_lock);
		cookie = 1;
	}
	else {
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline int
 * pfc_hash_wrlock(pfc_hashtbl_t *hp)
 *	Acquire hash table lock in writer mode, if needed.
 *	Returned value must be passed to pfc_hash_unlock().
 */
static inline int
pfc_hash_wrlock(pfc_hashtbl_t *hp)
{
	int	cookie = 0;

	if ((hp->ph_flags & PFC_HASH_NOLOCK) == 0) {
		__pfc_syncblock_t	*sbp;

		/* Check to see whether we're in synchronized block. */
		sbp = pfc_hashsync_lookup(hp);
		if (sbp == NULL) {
			(void)pfc_rwlock_wrlock(&hp->ph_lock);
			cookie = 1;
		}
		else {
			/* Writer mode is required. */
			PFC_ASSERT(PFC_SYNCBLOCK_IS_WRITE(sbp));
		}
	}

	return cookie;
}

/*
 * static inline void
 * pfc_hash_unlock(pfc_hashtbl_t *hp, int cookie)
 *	Release hash table lock if needed.
 */
static inline void
pfc_hash_unlock(pfc_hashtbl_t *hp, int cookie)
{
	if (cookie) {
		(void)pfc_rwlock_unlock(&hp->ph_lock);
	}
}

/*
 * static inline void
 * pfc_hash_ref(pfc_hashtbl_t *hp)
 *	Increment reference counter of the hash table.
 */
static inline void
pfc_hash_ref(pfc_hashtbl_t *hp)
{
	PFC_ASSERT(hp->ph_refcnt != 0);
	pfc_atomic_inc_uint32(&hp->ph_refcnt);
}

/*
 * static inline void
 * pfc_hash_unref(pfc_hashtbl_t *hp)
 *	Decrement reference counter of the hash table.
 *	The hash table is destroyed if the counter becomes zero.
 */
static inline void
pfc_hash_unref(pfc_hashtbl_t *hp)
{
	PFC_ASSERT(hp->ph_refcnt != 0);
	if (pfc_atomic_dec_uint32_old(&hp->ph_refcnt) == 1) {
#ifdef	PFC_VERBOSE_DEBUG
		if ((hp->ph_flags & PFC_HASH_NOLOCK) == 0) {
			int	err = pfc_rwlock_destroy(&hp->ph_lock);

			PFC_ASSERT(err == 0);
		}
#endif	/* PFC_VERBOSE_DEBUG */

		if (!PFC_HASH_IS_LIST(hp)) {
			free(hp);
		}
	}
}

/*
 * static inline void
 * pfc_hash_refptr_init(pfc_hashtbl_t *hp, pfc_refptr_t *ref,
 *			pfc_cptr_t object)
 *	Initialize dummy refptr object.
 *	Dummy refptr object is used to search refptr hash table.
 */
static inline void
pfc_hash_refptr_init(pfc_hashtbl_t *hp, pfc_refptr_t *ref, pfc_cptr_t object)
{
	ref->pr_ops = hp->ph_kops;
	ref->pr_refcnt = 1;
	ref->pr_object = (pfc_ptr_t)object;
}

/*
 * static inline pfc_bool_t
 * pfc_hash_ops_equals(pfc_hashtbl_t *hp, pfc_cptr_t o1, pfc_cptr_t o2)
 *	Call `equals' method.
 *
 * Remarks:
 *	This function is optimized for a hash table created by
 *	pfc_hash_create(). It is never called for a refptr hash table.
 */
static inline pfc_bool_t
pfc_hash_ops_equals(pfc_hashtbl_t *hp, pfc_cptr_t o1, pfc_cptr_t o2)
{
	if (hp->ph_ops->equals != NULL) {
		/* Use user-specified equals function. */
		return hp->ph_ops->equals(o1, o2);
	}

	/* Simply compares key addresses. */
	return (o1 == o2) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static inline uint32_t
 * pfc_hash_ops_hashfunc(pfc_hashtbl_t *hp, pfc_cptr_t key)
 *	Call `hashfunc' method.
 */
static inline uint32_t
pfc_hash_ops_hashfunc(pfc_hashtbl_t *hp, pfc_cptr_t key)
{
	if (hp->ph_kops != NULL) {
		pfc_refptr_t	*rkey = (pfc_refptr_t *)key;
		pfc_cptr_t	obj;

		if (PFC_EXPECT_FALSE(rkey == NULL)) {
			/* Use special hash value for NULL refptr key. */
			return PFC_HASHVAL_NULL;
		}

		/*
		 * Need to use hash function in refptr operation passed to
		 * pfc_refhash_create().
		 */
		obj = PFC_REFPTR_VALUE(rkey, pfc_cptr_t);

		return pfc_refptr_ops_hashfunc(hp->ph_kops, obj);
	}

	if (hp->ph_ops->hashfunc != NULL) {
		/* Use user-specified hash function. */
		return hp->ph_ops->hashfunc(key);
	}

	/*
	 * Use default hash function which derives hash value from
	 * object address.
	 */
	return pfc_refptr_hashfunc_default(key);
}

/*
 * static inline void
 * pfc_hash_entry_ref(pfc_hashtbl_t *PFC_RESTRICT hp,
 *		      pfc_hashent_t *PFC_RESTRICT entp, uint32_t options)
 *	Create a reference to the given hash entry.
 *
 *	This function is used to increment reference count if refptr object
 *	is specified as hash key or value.
 *
 *	`options' is a bitmap which determines behavior of this function.
 *	Supported bits are:
 *
 *	ENTREF_IGNORE_KEY
 *		If this bit is set, reference count of the hash key is never
 *		changed.
 *
 *	ENTREF_IGNORE_VALUE
 *		If this bit is set, reference count of the hash value is never
 *		changed.
 */
static inline void
pfc_hash_entry_ref(pfc_hashtbl_t *PFC_RESTRICT hp,
		   pfc_hashent_t *PFC_RESTRICT entp, uint32_t options)
{
	pfc_cptr_t	key = entp->phe_key;
	pfc_cptr_t	value = entp->phe_value;
	const uint32_t	flags = entp->phe_flags;
	const uint32_t	ignore_key = (options & ENTREF_IGNORE_KEY);
	const uint32_t	ignore_val = (options & ENTREF_IGNORE_VALUE);

	if (ignore_key == 0 && (flags & PFC_HASHOP_KEY_REFPTR)) {
		pfc_refptr_t *ref = (pfc_refptr_t *)key;

		/* Increment reference counter of hash key. */
		if (PFC_EXPECT_TRUE(ref != NULL)) {
			pfc_refptr_get(ref);
		}
	}
	if (ignore_val == 0 && (flags & PFC_HASHOP_VAL_REFPTR)) {
		pfc_refptr_t *ref = (pfc_refptr_t *)value;

		/* Increment reference counter of hash value. */
		if (PFC_EXPECT_TRUE(ref != NULL)) {
			pfc_refptr_get(ref);
		}
	}
}

/*
 * static inline void
 * pfc_hash_gen_update(pfc_hashtbl_t *hp)
 *	Bump up generation number of the given hash table.
 *	The caller must hold writer lock of the hash table if needed.
 */
static inline void
pfc_hash_gen_update(pfc_hashtbl_t *hp)
{
	/*
	 * Increment generation number without atomic operation.
	 * If the table doesn't have PFC_HASH_NOLOCK attribute, the caller
	 * should acquire writer lock of the table.
	 * If the table has PFC_HASH_NOLOCK attribute, it's up to the
	 * user of the hash table to serialize hash table operations.
	 */
	hp->ph_gen++;
}

/*
 * static inline void
 * pfc_hashiter_destroy(pfc_hashtbl_t *hp, hash_iter_t *iter, int cookie)
 *	Remove hash iterator.
 *	The caller must acquire hash writer lock if needed, and it is released
 *	on return.
 *
 * Remarks:
 *	If the hash table has been already destroyed, and the given iterator
 *	is the last iterator in the table, the table is destroyed.
 */
static inline void
pfc_hashiter_destroy(pfc_hashtbl_t *hp, hash_iter_t *iter, int cookie)
{
	pfc_list_remove(&iter->phi_list);
	free(iter);
	pfc_hash_unlock(hp, cookie);

	pfc_hash_unref(hp);
}

/*
 * int
 * pfc_hash_create(pfc_hash_t *PFC_RESTRICT hashp,
 *		   const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		   uint32_t nbuckets, uint32_t flags)
 *	Create a hash table.
 *	Data type of hash key is determined by hash operations specified to
 *	`ops'. If NULL is specified to `ops', all hash operations will be done
 *	by using default operation.
 *
 * Calling/Exit State:
 *	Upon successful completion, hash table identifier is set in `*hashp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 *
 * Remarks:
 *	A hash table instance created by this function keeps a hash operation
 *	specified by `ops'. So it must be located in static area.
 *
 *	A hash table created by pfc_hash_create() can't take refptr object
 *	as hash key. Use pfc_refhash_create() if you want to use refptr
 *	as hash key.
 */
int
pfc_hash_create(pfc_hash_t *PFC_RESTRICT hashp,
		const pfc_hash_ops_t *PFC_RESTRICT ops, uint32_t nbuckets,
		uint32_t flags)
{
	return pfc_hash_table_init(hashp, ops, NULL, nbuckets,
				   flags & PFC_IHASH_FLAGS_MASK);
}

/*
 * int
 * pfc_refhash_create(pfc_hash_t *PFC_RESTRICT hashp,
 *		      const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		      const pfc_refptr_ops_t *PFC_RESTRICT kops,
 *		      uint32_t nbuckets, uint32_t flags)
 *	Create a hash table which takes refptr object as hash key.
 *
 *	The hash key in the refptr hash table must be refptr object which
 *	has the same refptr operation pointer with `kops'.
 *	Hash key operation is done by refptr operation. So hash function and
 *	comparator in hash operation are always ignored.
 *
 * Calling/Exit State:
 *	Upon successful completion, hash table identifier is set in
 *	`*hashp', and zero is returned. Otherwise error number which
 *	indicates the cause of error is returned.
 *
 * Remarks:
 *	A hash table instance created by this function keeps a refptr
 *	operation and hash operation pointers. So they must be located in
 *	static area.
 */
int
pfc_refhash_create(pfc_hash_t *PFC_RESTRICT hashp,
		   const pfc_hash_ops_t *PFC_RESTRICT ops,
		   const pfc_refptr_ops_t *PFC_RESTRICT kops,
		   uint32_t nbuckets, uint32_t flags)
{
	if (kops == NULL) {
		/* Use default refptr operation. */
		kops = &refptr_default_ops;
	}

	return pfc_hash_table_init(hashp, ops, kops, nbuckets,
				   flags & PFC_IHASH_FLAGS_MASK);
}

/*
 * int
 * pfc_hash_put(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
 *		pfc_cptr_t value, uint32_t opflags)
 *	Insert the given pair of key and value into the hash table.
 *
 *	If PFC_HASHOP_KEY_REFPTR is set in `opflags', `key' is considered as
 *	refptr object, and its reference counter is incremented while the
 *	hash table keeps it.
 *
 *	If PFC_HASHOP_VAL_REFPTR is set in `opflags', `value' is considered as
 *	refptr object, and its reference counter is incremented while the
 *	hash table keeps it.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	EEXIST is returned if the given hash key already exists in the hash
 *	table.
 */
int
pfc_hash_put(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key, pfc_cptr_t value,
	     uint32_t opflags)
{
	return pfc_hash_put_entry(hash, key, value,
				  opflags & ~PFC_IHASHOP_UPDATE);
}

/*
 * int
 * pfc_hash_update(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
 *		   pfc_cptr_t value, uint32_t opflags)
 *	Update the hash entry specified by the given hash key.
 *	Unlike pfc_hash_put(), existing entry associated with the given key
 *	is removed.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hash_update(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key, pfc_cptr_t value,
		uint32_t opflags)
{
	return pfc_hash_put_entry(hash, key, value,
				  opflags | PFC_IHASHOP_UPDATE);
}

/*
 * int
 * pfc_hash_remove(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
 *		   uint32_t opflags)
 *	Remove a hash entry which has the given key.
 *
 *	If the specified hash table is a refptr hash table created by
 *	pfc_refhash_create(), the caller must specify appropriate hash
 *	operation flag. If PFC_HASHOP_KEY_REFPTR is set in `opflags',
 *	the specified key is considered as a pointer to refptr object.
 *	If not set, the specified key is considered as a pointer to object
 *	which has the same data type as hash keys in the table.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	ENOENT is returned if the given key is not found.
 */
int
pfc_hash_remove(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
		uint32_t opflags)
{
	return pfc_hash_do_remove(hash, key, NULL, opflags);
}

/*
 * int
 * __pfc_hash_delete(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
 *		     pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
 *		     uint32_t opflags)
 *	Remove a hash entry which has the given key, and return value
 *	associated with the key.
 *
 *	If non-NULL value is passed to `valuep', __pfc_hash_delete() sets
 *	the value associated with the given key on successful return.
 *	In this case, hash value destructor is never called, and reference
 *	counter of the value is never changed even if the value is refptr
 *	object.
 *
 *	The call of __pfc_hash_delete() with specifying NULL to `valuep' is
 *	identical to the call of pfc_hash_remove().
 *
 * Calling/Exit State:
 *	Upon successful completion, value associated with the given key
 *	is set to `*valuep', and zero is returned.
 *	ENOENT is returned if the given key is not found.
 */
int
__pfc_hash_delete(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
		  pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
{
	return pfc_hash_do_remove(hash, key, valuep, opflags);
}

/*
 * int
 * __pfc_hash_get(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
 *		  pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
 *	Find a value associated with the given key in the hash table.
 *
 *	If the specified hash table is a refptr hash table created by
 *	pfc_refhash_create(), the caller must specify appropriate hash
 *	operation flag. If PFC_HASHOP_KEY_REFPTR is set in `opflags',
 *	the specified key is considered as a pointer to refptr object.
 *	If not set, the specified key is considered as a pointer to object
 *	which has the same data type as hash keys in the table.
 *
 *	If `valuep' is NULL, a pointer to hash value is not returned.
 *	In that case, this function can be used to detect whether the given
 *	 key exists in the hash table.
 *
 * Calling/Exit State:
 *	Upon successful completion, a value associated with the given key is
 *	set to `*valuep' and zero is returned.
 *	ENOENT is returned if the given key is not found.
 */
int
__pfc_hash_get(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
	       pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**entpp, *entp;
	pfc_refptr_t	rbuf;
	uint32_t	index;
	const char	*strkey = NULL;
	int	err = ENOENT, cookie;

	if (hp->ph_kops != NULL) {
		if (opflags & PFC_HASHOP_KEY_REFPTR) {
			/* Key is a pointer to refptr object. */
			if (PFC_EXPECT_TRUE(key != NULL)) {
				pfc_refptr_t	*ref = (pfc_refptr_t *)key;
				const pfc_refptr_ops_t	*rops =
					pfc_refptr_operation(ref);

				if (PFC_EXPECT_FALSE(rops != hp->ph_kops)) {
					return EINVAL;
				}
			}
		}
		else if (hp->ph_kops == &refptr_string_ops) {
			/* Create a dummy string refptr object. */
			if (PFC_EXPECT_TRUE(key != NULL)) {
				strkey = (const char *)key;
				pfc_hash_refptr_init(hp, &rbuf, key);
				key = (pfc_cptr_t)&rbuf;
			}
		}
		else {
			/* Create a dummy refptr object. */
			pfc_hash_refptr_init(hp, &rbuf, key);
			key = (pfc_cptr_t)&rbuf;
		}
	}

	/* Derive hash value from the given key. */
	if (strkey != NULL) {
		/*
		 * A key passed from the caller is the raw string, and
		 * `key' now points to a dummy refptr string object.
		 * We need to calculate hash value using raw string because
		 * hash function for refptr string doesn't work with
		 * dummy refptr string.
		 */
		index = refptr_string_hashfunc(strkey, strlen(strkey));
	}
	else {
		index = pfc_hash_ops_hashfunc(hp, key);
	}

	/* Acquire reader lock if needed. */
	cookie = pfc_hash_rdlock(hp);

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		err = EBADF;
		goto out;
	}

	/* Search for a key in the hash table. */
	entpp = pfc_hash_lookup(hp, key, index);
	if ((entp = *entpp) != NULL) {
		/* Found. */
		if (valuep != NULL) {
			*valuep = entp->phe_value;
		}
		err = 0;
	}

out:
	pfc_hash_unlock(hp, cookie);

	return err;
}

/*
 * int
 * pfc_hash_clear(pfc_hash_t hash)
 *	Remove all hash entries in the hash table.
 *
 *	Key and value destructor will be called for all hash keys and values.
 *	Reference counters in all refptr objects in the hash table will be
 *	decremented.
 *
 * Calling/Exit State:
 *	Number of removed hash entries is returned.
 */
size_t
pfc_hash_clear(pfc_hash_t hash)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	size_t	nelems;
	int	cookie;

	cookie = pfc_hash_wrlock(hp);
	nelems = pfc_hash_clear_l(hp);
	pfc_hash_unlock(hp, cookie);

	return nelems;
}

/*
 * size_t
 * pfc_hash_get_size(pfc_hash_t hash)
 *	Return number of hash entries in the hash table.
 */
size_t
pfc_hash_get_size(pfc_hash_t hash)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	size_t	ret;
	int	cookie;

	cookie = pfc_hash_rdlock(hp);
	ret = hp->ph_nelems;
	pfc_hash_unlock(hp, cookie);

	return ret;
}

/*
 * uint32_t
 * pfc_hash_get_capacity(pfc_hash_t hash)
 *	Return number of hash buckets in the hash table.
 */
uint32_t
pfc_hash_get_capacity(pfc_hash_t hash)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	uint32_t	ret;
	int	cookie;

	cookie = pfc_hash_rdlock(hp);
	ret = hp->ph_nbuckets;
	pfc_hash_unlock(hp, cookie);

	return ret;
}

/*
 * int
 * pfc_hash_set_capacity(pfc_hash_t hash, uint32_t nbuckets)
 *	Change number of hash buckets.
 *	If PFC_HASH_PRIME attribute is set, number of hash buckets is adjusted
 *	to the largest prime number within the given number.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hash_set_capacity(pfc_hash_t hash, uint32_t nbuckets)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**table, **entpp;
	int	err = 0, cookie;

	/* Adjust number of hash buckets if requested. */
	if (hp->ph_flags & PFC_HASH_PRIME) {
		nbuckets = pfc_hash_get_prime(nbuckets);
	}
	if (PFC_EXPECT_FALSE(nbuckets == 0 ||
			     nbuckets > PFC_HASH_MAX_NBUCKETS)) {
		return EINVAL;
	}

	/* Allocate a new hash table. */
	table = (pfc_hashent_t **)malloc(sizeof(pfc_hashent_t *) * nbuckets);
	if (PFC_EXPECT_FALSE(table == NULL)) {
		return ENOMEM;
	}
	for (entpp = table; entpp < table + nbuckets; entpp++) {
		*entpp = NULL;
	}

	/* Acquire writer lock if needed. */
	cookie = pfc_hash_wrlock(hp);

	if (nbuckets == hp->ph_nbuckets) {
		/* Nothing to do. */
		goto backout;
	}

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		err = EBADF;
		goto backout;
	}

	/* Iterate current hash buckets. */
	for (entpp = hp->ph_table; entpp < hp->ph_table + hp->ph_nbuckets;
	     entpp++) {
		pfc_hashent_t	*entp, *next;

		for (entp = *entpp; entp != NULL; entp = next) {
			uint32_t	index;
			pfc_hashent_t	**newentpp;

			next = entp->phe_next;

			/* Calculate new hash index. */
			index = pfc_hash_ops_hashfunc(hp, entp->phe_key) %
				nbuckets;
			newentpp = table + index;

			/* Link to new table. */
			entp->phe_next = *newentpp;
			*newentpp = entp;
		}
	}

	/* Release old table, and install new one. */
	free(hp->ph_table);
	hp->ph_table = table;
	hp->ph_nbuckets = nbuckets;
	pfc_hash_gen_update(hp);

	pfc_hash_unlock(hp, cookie);

	return 0;

backout:
	pfc_hash_unlock(hp, cookie);
	free(table);

	return err;
}

/*
 * void
 * __pfc_hash_sync_begin(__pfc_syncblock_t *frame)
 *	Start hash table synchronized block.
 *	This function is assumed to be called via pfc_hash_wsync_begin() or
 *	pfc_hash_rsync_begin().
 */
void
__pfc_hash_sync_begin(__pfc_syncblock_t *frame)
{
	pfc_hashtbl_t		*hp = (pfc_hashtbl_t *)frame->psb_target;

	if (hp->ph_flags & PFC_HASH_NOLOCK) {
		/*
		 * Increment reference counter to protect hash table against
		 * hash destroy in synchronized block.
		 */
		pfc_hash_ref(hp);
		return;
	}

	if (pfc_syncblock_enter(&hash_sync_frame, frame)) {
		/* Acquire lock. */
		if (PFC_SYNCBLOCK_IS_WRITE(frame)) {
			(void)pfc_rwlock_wrlock(&hp->ph_lock);
		}
		else {
			(void)pfc_rwlock_rdlock(&hp->ph_lock);
		}

		pfc_hash_ref(hp);
	}
}

/*
 * void
 * __pfc_hash_sync_end(void *arg)
 *	Quit hash table synchronized block.
 *	Actual lock is released if the given frame is the last frame.
 */
void
__pfc_hash_sync_end(void *arg)
{
	__pfc_syncblock_t	*frame = (__pfc_syncblock_t *)arg;
	pfc_hashtbl_t		*hp = (pfc_hashtbl_t *)frame->psb_target;

	if (hp->ph_flags & PFC_HASH_NOLOCK) {
		pfc_hash_unref(hp);
		return;
	}

	if (pfc_syncblock_exit(&hash_sync_frame, (pfc_cptr_t)hp)) {
		/* Now leaving from all synchronized blocks. */
		(void)pfc_rwlock_unlock(&hp->ph_lock);
		pfc_hash_unref(hp);
	}
}

/*
 * pfc_hashiter_t
 * pfc_hashiter_get(pfc_hash_t hash)
 *	Create a new hash entry iterator.
 *
 * Calling/Exit State:
 *	Upon successful completion, hash entry iterator is returned.
 *	NULL is returned on error.
 */
pfc_hashiter_t
pfc_hashiter_get(pfc_hash_t hash)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	hash_iter_t	*iter;
	int	cookie;

	iter = (hash_iter_t *)malloc(sizeof(*iter));
	if (PFC_EXPECT_FALSE(iter == NULL)) {
		return NULL;
	}

	iter->phi_hash = hp;
	iter->phi_index = 0;

	/* Acquire writer lock if needed. */
	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		pfc_hash_unlock(hp, cookie);
		free(iter);

		return NULL;
	}

	/* Initialize entry pointer. */
	iter->phi_prev = NULL;
	iter->phi_next = hp->ph_table;

	/* Copy current generation number. */
	iter->phi_gen = hp->ph_gen;

	/* Insert iterator into the active iterator list. */
	pfc_list_push(&hp->ph_iterator, &iter->phi_list);

	pfc_hash_ref(hp);
	pfc_hash_unlock(hp, cookie);

	return (pfc_hashiter_t)iter;
}

/*
 * int
 * __pfc_hashiter_next(pfc_hashiter_t PFC_RESTRICT it,
 *		       pfc_cptr_t *PFC_RESTRICT keyp,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get next key and value from the hash entry iterator.
 *
 * Calling/Exit State:
 *	Upon successful completion, key and value of the next entry are set
 *	to `*keyp' and `*valuep', and zero is returned.
 *
 *	EINVAL is returned if modification of the hash table is detected.
 *	ENOENT is returned if no more hash entry exists.
 *	EBADF is returned if the table has been destroyed.
 *
 * Remarks:
 *	Iterator is automatically disposed if any error is detected.
 *	The caller must not touch iterator if this function returns error.
 */
int
__pfc_hashiter_next(pfc_hashiter_t PFC_RESTRICT it,
		    pfc_cptr_t *PFC_RESTRICT keyp,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	hash_iter_t	*iter = (hash_iter_t *)it;
	pfc_hashtbl_t	*hp = iter->phi_hash;
	int	err, cookie;

	/*
	 * Acquire writer lock to serialize access to the table and
	 * iterator.
	 */
	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(PFC_HASH_IS_U64(hp))) {
		/*
		 * This function can't iterate uint64 hash table.
		 * You must use pfc_hashiter_uint64_next().
		 */
		err = EINVAL;
		goto error;
	}

	err = pfc_hashiter_get_next(iter, hp, keyp, valuep);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_hash_unlock(hp, cookie);

	return 0;

error:
	/* Remove iterator on error. */
	pfc_hashiter_destroy(hp, iter, cookie);

	return err;
}

/*
 * int
 * pfc_hashiter_remove(pfc_hashiter_t it)
 *	Remove the last hash entry returned by the previous call of
 *	pfc_hashiter_next().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	EINVAL is returned if modification of the hash table is detected.
 *	EBADF is returned if the table has been destroyed.
 *	ENOENT is returned if pfc_hashiter_next() is not yet called, or
 *	the entry is already removed by pfc_hashiter_remove().
 */
int
pfc_hashiter_remove(pfc_hashiter_t it)
{
	hash_iter_t	*iter = (hash_iter_t *)it;
	pfc_hashtbl_t	*hp = iter->phi_hash;
	pfc_hashent_t	**entpp, *entp;
	int	err, cookie;

	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		err = EBADF;
		goto error;
	}

	if (PFC_EXPECT_FALSE(hp->ph_gen != iter->phi_gen)) {
		/* Hash table has been changed. */
		err = EINVAL;
		goto error;
	}

	if ((entpp = iter->phi_prev) == NULL) {
		/*
		 * pfc_hashiter_next() is not yet called, or
		 * already removed.
		 */
		err = ENOENT;
		goto error;
	}

	/* Remove a hash entry previously retrieved by pfc_hashiter_next(). */
	entp = *entpp;
	*entpp = entp->phe_next;
	pfc_hash_entry_unref(hp, entp, 0);
	free(entp);

	PFC_ASSERT(hp->ph_nelems != 0);
	hp->ph_nelems--;
	pfc_hash_gen_update(hp);

	/* Update iterator. */
	iter->phi_gen = hp->ph_gen;
	iter->phi_prev = NULL;
	iter->phi_next = entpp;

	pfc_hash_unlock(hp, cookie);

	return 0;

error:
	pfc_hash_unlock(hp, cookie);

	return err;
}

/*
 * void
 * pfc_hashiter_put(pfc_hashiter_t it
 *	Dispose hash entry iterator.
 *	If you want to terminate hash entry iteration before pfc_hashiter_next()
 *	returns an error, you must dispose iterator by calling this function.
 */
void
pfc_hashiter_put(pfc_hashiter_t it)
{
	hash_iter_t	*iter = (hash_iter_t *)it;
	pfc_hashtbl_t	*hp = iter->phi_hash;
	int	cookie;

	cookie = pfc_hash_wrlock(hp);
	pfc_hashiter_destroy(hp, iter, cookie);
}

/*
 * void
 * pfc_hash_destroy(pfc_hash_t hash)
 *	Destroy the given hash table.
 *	The caller must guarantee that no one refers the table.
 */
void
pfc_hash_destroy(pfc_hash_t hash)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**table;
	int	cookie;

	cookie = pfc_hash_wrlock(hp);

	/* Clear hash table. */
	(void)pfc_hash_clear_l(hp);
	table = hp->ph_table;
	hp->ph_table = NULL;
	hp->ph_nbuckets = 0;

	pfc_hash_unlock(hp, cookie);

	free(table);
	pfc_hash_unref(hp);
}

/*
 * int
 * pfc_strhash_create(pfc_hash_t *PFC_RESTRICT hashp,
 *		      const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		      uint32_t nbuckets, uint32_t flags)
 *	Create a refptr hash table which takes string refptr object as
 *	hash key.
 */
int
pfc_strhash_create(pfc_hash_t *PFC_RESTRICT hashp,
		   const pfc_hash_ops_t *PFC_RESTRICT ops,
		   uint32_t nbuckets, uint32_t flags)
{
	return pfc_hash_table_init(hashp, ops, &refptr_string_ops, nbuckets,
				   flags & PFC_IHASH_FLAGS_MASK);
}

/*
 * int
 * pfc_strhash_put(pfc_hash_t PFC_RESTRICT hash, const char *key,
 *		   pfc_cptr_t value, uint32_t opflags)
 *	Insert the given pair of string key and value into the string hash
 *	table.
 *	This function creates a refptr string internally, and uses it as
 *	a hash key.
 */
int
pfc_strhash_put(pfc_hash_t PFC_RESTRICT hash, const char *key,
		pfc_cptr_t value, uint32_t opflags)
{
	pfc_refptr_t	*ref;
	int	err;

	if (key == NULL) {
		ref = NULL;
	}
	else {
		ref = pfc_refptr_string_create(key);
		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}
	}

	opflags &= ~PFC_IHASHOP_UPDATE;
	opflags |= PFC_HASHOP_KEY_REFPTR;
	err = pfc_hash_put_entry(hash, (pfc_cptr_t)ref, value, opflags);
	if (ref != NULL) {
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_strhash_update(pfc_hash_t PFC_RESTRICT hash, const char *key,
 *		      pfc_cptr_t value, uint32_t opflags)
 *	Update the hash entry specified by the given string.
 *	This function creates a refptr string internally, and uses it as
 *	a hash key.
 */
int
pfc_strhash_update(pfc_hash_t PFC_RESTRICT hash, const char *key,
		   pfc_cptr_t value, uint32_t opflags)
{
	pfc_refptr_t	*ref;
	int	err;

	if (key == NULL) {
		ref = NULL;
	}
	else {
		ref = pfc_refptr_string_create(key);
		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}
	}

	opflags |= (PFC_IHASHOP_UPDATE | PFC_HASHOP_KEY_REFPTR);
	err = pfc_hash_put_entry(hash, (pfc_cptr_t)ref, value, opflags);
	if (ref != NULL) {
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_u64hash_create(pfc_hash_t *PFC_RESTRICT hashp,
 *		      const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		      uint32_t nbuckets, uint32_t flags)
 *	Create a hash table which takes unsigned 64-bit integer as hash key.
 */
int
pfc_u64hash_create(pfc_hash_t *PFC_RESTRICT hashp,
		   const pfc_hash_ops_t *PFC_RESTRICT ops,
		   uint32_t nbuckets, uint32_t flags)
{
	const pfc_refptr_ops_t	*refops;

	if (PFC_IS_LP64_SYSTEM()) {
		/*
		 * A pointer can represent 64-bit value. So the default
		 * hash operation can handle 64-bit key.
		 */
		refops = NULL;
	}
	else {
		/*
		 * Use uint64_t refptr as hash key because a pointer can't
		 * represent 64-bit value.
		 */
		refops = &refptr_uint64_ops;
	}

	return pfc_hash_table_init(hashp, ops, refops, nbuckets,
				   flags | PFC_IHASH_U64);
}

/*
 * int
 * pfc_u64hash_put(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
 *		   pfc_cptr_t value, uint32_t opflags)
 *	Insert the given pair of uint64 key and value into the hash table.
 */
int
pfc_u64hash_put(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		pfc_cptr_t value, uint32_t opflags)
{
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hash))) {
		return EINVAL;
	}

	opflags &= ~PFC_IHASHOP_UPDATE;

	if (PFC_IS_LP64_SYSTEM()) {
		opflags &= ~PFC_HASHOP_KEY_REFPTR;
		err = pfc_hash_put_entry(hash, (pfc_cptr_t)(uintptr_t)key,
					 value, opflags);
	}
	else {
		pfc_refptr_t	*ref;

		/* Need to create uint64 refptr for this entry. */
		ref = pfc_refptr_uint64_create(key);
		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		opflags |= PFC_HASHOP_KEY_REFPTR;
		err = pfc_hash_put_entry(hash, (pfc_cptr_t)ref, value,
					 opflags);
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_u64hash_update(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
 *		      pfc_cptr_t value, uint32_t opflags)
 *	Update the hash entry specified by the given uint64 key.
 */
int
pfc_u64hash_update(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		   pfc_cptr_t value, uint32_t opflags)
{
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hash))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		opflags &= ~PFC_HASHOP_KEY_REFPTR;
		opflags |= PFC_IHASHOP_UPDATE;
		err = pfc_hash_put_entry(hash, (pfc_cptr_t)(uintptr_t)key,
					 value, opflags);
	}
	else {
		pfc_refptr_t	*ref;

		/* Need to create uint64 refptr for this entry. */
		ref = pfc_refptr_uint64_create(key);
		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		opflags |= (PFC_IHASHOP_UPDATE | PFC_HASHOP_KEY_REFPTR);
		err = pfc_hash_put_entry(hash, (pfc_cptr_t)ref, value,
					 opflags);
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_u64hash_remove(pfc_hash_t PFC_RESTRICT hash, uint64_t key)
 *	Remove a hash entry which has the given uint64 key.
 */
int
pfc_u64hash_remove(pfc_hash_t PFC_RESTRICT hash, uint64_t key)
{
	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hash))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		return pfc_hash_remove(hash, (pfc_cptr_t)(uintptr_t)key, 0);
	}

	/*
	 * uint64 refptr keeps a pointer to uint64_t.
	 * So we can pass address of the specified key.
	 * pfc_hash_remove() will create a dummy uint64 refptr for this search.
	 */
	return pfc_hash_remove(hash, (pfc_cptr_t)&key, 0);
}

/*
 * int
 * __pfc_u64hash_delete(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
 *			pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove a hash entry which has the given uint64 key, and return value
 *	associated with the key.
 *
 *	If non-NULL value is passed to `valuep', __pfc_u64hash_delete() sets
 *	the value associated with the given key on successful return.
 *	In this case, hash value destructor is never called, and reference
 *	counter of the value is never changed even if the value is refptr
 *	object.
 *
 *	The call of __pfc_u64hash_delete() with specifying NULL to `valuep'
 *	is identical to pfc_u64hash_remove().
 */
int
__pfc_u64hash_delete(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		     pfc_cptr_t *PFC_RESTRICT valuep)
{
	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hash))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		return pfc_hash_do_remove(hash, (pfc_cptr_t)(uintptr_t)key,
					  valuep, 0);
	}

	/*
	 * uint64 refptr keeps a pointer to uint64_t.
	 * So we can pass address of the specified key.
	 * pfc_hash_do_remove() will create a dummy uint64 refptr for this
	 * search.
	 */
	return pfc_hash_do_remove(hash, (pfc_cptr_t)&key, valuep, 0);
}

/*
 * int
 * __pfc_u64hash_get(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
 *		     pfc_cptr_t *PFC_RESTRICT valuep)
 *	Find a value associated with the given uint64 key.
 */
int
__pfc_u64hash_get(pfc_hash_t PFC_RESTRICT hash, uint64_t key,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hash))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		return __pfc_hash_get(hash, (pfc_cptr_t)(uintptr_t)key,
				      valuep, 0);
	}

	/*
	 * uint64 refptr keeps a pointer to uint64_t.
	 * So we can pass address of the specified key.
	 * __pfc_hash_get() will create a dummy uint64 refptr for this search.
	 */
	return __pfc_hash_get(hash, (pfc_cptr_t)&key, valuep, 0);
}

/*
 * int
 * __pfc_hashiter_uint64_next(pfc_hashiter_t PFC_RESTRICT it,
 *			      uint64_t *PFC_RESTRICT keyp,
 *			      pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get next key and value from the uint64 hash entry iterator.
 *
 * Calling/Exit State:
 *	Upon successful completion, key and value of the next entry are set
 *	to `*keyp' and `*valuep', and zero is returned.
 *
 *	EINVAL is returned if modification of the hash table is detected.
 *	ENOENT is returned if no more hash entry exists.
 *	EBADF is returned if the table has been destroyed.
 *
 * Remarks:
 *	Iterator is automatically disposed if any error is detected.
 *	The caller must not touch iterator if this function returns error.
 */
int
__pfc_hashiter_uint64_next(pfc_hashiter_t PFC_RESTRICT it,
			   uint64_t *PFC_RESTRICT keyp,
			   pfc_cptr_t *PFC_RESTRICT valuep)
{
	hash_iter_t	*iter = (hash_iter_t *)it;
	pfc_hashtbl_t	*hp = iter->phi_hash;
	int	err, cookie;

	/*
	 * Acquire writer lock to serialize access to the table and
	 * iterator.
	 */
	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(!PFC_HASH_IS_U64(hp))) {
		/* This function is provided only for uint64 hash. */
		err = EINVAL;
		goto error;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		err = pfc_hashiter_get_next(iter, hp, (pfc_cptr_t *)keyp,
					    valuep);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}
	}
	else {
		pfc_cptr_t	key;
		pfc_refptr_t	*ref;

		err = pfc_hashiter_get_next(iter, hp, &key, valuep);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error;
		}

		if (keyp != NULL) {
			/* The key should be a uint64 refptr. */
			ref = (pfc_refptr_t *)key;
			*keyp = pfc_refptr_uint64_value(ref);
		}
	}

	pfc_hash_unlock(hp, cookie);

	return 0;

error:
	/* Remove iterator on error. */
	pfc_hashiter_destroy(hp, iter, cookie);

	return err;
}

/*
 * int
 * pfc_hash_table_init(pfc_hashl_t *PFC_RESTRICT hashp,
 *		       const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		       const pfc_refptr_ops_t *PFC_RESTRICT kops,
 *		       uint32_t nbuckets, uint32_t flags)
 *	Common routine to create a new hash table.
 */
int PFC_ATTR_HIDDEN
pfc_hash_table_init(pfc_hash_t *PFC_RESTRICT hashp,
		    const pfc_hash_ops_t *PFC_RESTRICT ops,
		    const pfc_refptr_ops_t *PFC_RESTRICT kops,
		    uint32_t nbuckets, uint32_t flags)
{
	pfc_hashtbl_t	*hp;
	pfc_hashent_t	**table, **etable;
	int	err;

	PFC_ASSERT(hashp != NULL);

	/* Adjust number of hash buckets if requested. */
	if (flags & PFC_HASH_PRIME) {
		nbuckets = pfc_hash_get_prime(nbuckets);
	}

	if (PFC_EXPECT_FALSE(nbuckets == 0 ||
			     nbuckets > PFC_HASH_MAX_NBUCKETS)) {
		return EINVAL;
	}

	if (flags & PFC_IHASH_LIST) {
		/*
		 * This table is for internal use of hash list.
		 * Table buffer should be allocated in conjunction with
		 * list instance. Table serialization is done by the list lock.
		 */
		hp = (pfc_hashtbl_t *)(*hashp);
		PFC_ASSERT(hp != NULL);
		flags |= PFC_HASH_NOLOCK;
	}
	else {
		/* Allocate a new hash table instance. */
		hp = (pfc_hashtbl_t *)malloc(sizeof(*hp));
		if (PFC_EXPECT_FALSE(hp == NULL)) {
			return ENOMEM;
		}

		if ((flags & PFC_HASH_NOLOCK) == 0) {
			/* Initialize hash table lock. */
			err = pfc_rwlock_init(&hp->ph_lock);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto error;
			}
		}
	}

	/* Allocate hash table. */
	table = (pfc_hashent_t **)malloc(sizeof(pfc_hashent_t *) * nbuckets);
	if (PFC_EXPECT_FALSE(table == NULL)) {
		err = ENOMEM;
		goto error;
	}

	if (ops == NULL) {
		/* Use dummy hash operation. */
		ops = &hashop_default;
	}

	hp->ph_table = table;
	hp->ph_ops = ops;
	hp->ph_kops = kops;
	hp->ph_nelems = 0;
	hp->ph_nbuckets = nbuckets;
	hp->ph_flags = flags;
	hp->ph_gen = 0;
	hp->ph_refcnt = 1;
	pfc_list_init(&hp->ph_iterator);

	etable = table + nbuckets;
	for (; table < etable; table++) {
		*table = NULL;
	}

	*hashp = (pfc_hash_t)hp;

	return 0;

error:
	if ((flags & PFC_IHASH_LIST) == 0) {
		free(hp);
	}

	return err;
}

/*
 * static int
 * pfc_hash_put_entry(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
 *		      pfc_cptr_t value, uint32_t flags)
 *	Insert the given hash entry into the hash table.
 *
 *	If PFC_HASHENT_UPDATE is set in `flags', existing entry which has the
 *	same key is removed. If not set, EEXIST is returned without
 *	modifying hash table.
 *
 *	PFC_HASHENT_REFPTR_VALUE must be set if the given hash value is a
 *	pointer to refptr object.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	A new hash entry specified to `newentp' is destroyed if this function
 *	returns error.
 */
static int
pfc_hash_put_entry(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t key,
		   pfc_cptr_t value, uint32_t opflags)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**entpp, *newentp, *oldentp;
	uint32_t	index;
	pfc_bool_t	replaced = PFC_FALSE;
	int	err = EEXIST, cookie;

	if (hp->ph_kops != NULL) {
		pfc_refptr_t	*kref = (pfc_refptr_t *)key;

		/*
		 * Hash key must be refptr object, and it must have the same
		 * refptr operation with one specified to pfc_refhash_create().
		 */
		if (PFC_EXPECT_FALSE((opflags & PFC_HASHOP_KEY_REFPTR) == 0 ||
				     (kref != NULL &&
				      pfc_refptr_operation(kref) !=
				      hp->ph_kops))) {
			return EINVAL;
		}
	}

	/* Allocate a new hash entry. */
	newentp = pfc_hash_entry_alloc(key, value, opflags);
	if (PFC_EXPECT_FALSE(newentp == NULL)) {
		return ENOMEM;
	}

	/* Derive hash value from the given key. */
	index = pfc_hash_ops_hashfunc(hp, key);

	/* Acquire writer lock if needed. */
	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		err = EBADF;
		goto error;
	}

	/* Check to see whether the given key exists in the table. */
	entpp = pfc_hash_lookup(hp, key, index);
	if ((oldentp = *entpp) != NULL) {
		if (opflags & PFC_IHASHOP_UPDATE) {
			/* Replace hash entry. */
			pfc_hash_replace_entry(hp, oldentp, newentp);
			replaced = PFC_TRUE;
		}
		else {
			/* Reject duplicated key. */
			goto error;
		}
	}
	else {
		hp->ph_nelems++;
		PFC_ASSERT(hp->ph_nelems != 0);
	}

	/* Enter new hash entry. */
	*entpp = newentp;
	if (!replaced) {
		pfc_hash_entry_ref(hp, newentp, 0);
	}
	pfc_hash_gen_update(hp);

	pfc_hash_unlock(hp, cookie);

	return 0;

error:
	pfc_hash_unlock(hp, cookie);
	free(newentp);

	return err;
}

/*
 * static void
 * pfc_hash_replace_entry(pfc_hashtbl_t *PFC_RESTRICT hp,
 *			  pfc_hashent_t *PFC_RESTRICT oldentp,
 *			  pfc_hashent_t *PFC_RESTRICT newentp)
 *	Replace hash table entry.
 *	This function removes `oldentp' from hash collision chain, and
 *	insert `newentp' instead.
 *
 *
 * Remarks:
 *	- This is an internal function of pfc_hash_put_entry().
 *
 *	- This function always creates references to key and value in
 *	  `newentp'. So pfc_hash_put_entry() must not call
 *	  pfc_hash_entry_ref() if it calls this function.
 */
static void
pfc_hash_replace_entry(pfc_hashtbl_t *PFC_RESTRICT hp,
		       pfc_hashent_t *PFC_RESTRICT oldentp,
		       pfc_hashent_t *PFC_RESTRICT newentp)
{
	uint32_t	options = 0;

	/* Replace old entry with new one. */
	newentp->phe_next = oldentp->phe_next;

	/*
	 * Check to see whether old key and value should be retained in the
	 * hash table.
	 *
	 * If the key object in the hash table is passed to pfc_hash_update(),
	 * we must not call key destructor and not change reference to the key
	 * because it is retained in the hash table. The same thing can be
	 * said of the hash value.
	 */
	if (oldentp->phe_key == newentp->phe_key) {
		options |= ENTREF_IGNORE_KEY;
	}

	if (oldentp->phe_value == newentp->phe_value) {
		options |= ENTREF_IGNORE_VALUE;
	}

	/* Make references to new key and value. */
	pfc_hash_entry_ref(hp, newentp, options);

	/* Remove old entry. */
	pfc_hash_entry_unref(hp, oldentp, options);
	free(oldentp);
}

static int
pfc_hash_do_remove(pfc_hash_t PFC_RESTRICT hash, pfc_cptr_t PFC_RESTRICT key,
		   pfc_cptr_t *PFC_RESTRICT valuep, uint32_t opflags)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**entpp, *entp;
	pfc_refptr_t	rbuf;
	uint32_t	index;
	const char	*strkey = NULL;
	int	err = ENOENT, cookie;

	if (hp->ph_kops != NULL) {
		if (opflags & PFC_HASHOP_KEY_REFPTR) {
			/* Key is a pointer to refptr object. */
			if (PFC_EXPECT_TRUE(key != NULL)) {
				pfc_refptr_t	*ref = (pfc_refptr_t *)key;
				const pfc_refptr_ops_t	*rops =
					pfc_refptr_operation(ref);

				if (PFC_EXPECT_FALSE(rops != hp->ph_kops)) {
					return EINVAL;
				}
			}
		}
		else if (hp->ph_kops == &refptr_string_ops) {
			/* Create a dummy string refptr object. */
			if (PFC_EXPECT_TRUE(key != NULL)) {
				strkey = (const char *)key;
				pfc_hash_refptr_init(hp, &rbuf, key);
				key = (pfc_cptr_t)&rbuf;
			}
		}
		else {
			/* Create a dummy refptr object. */
			pfc_hash_refptr_init(hp, &rbuf, key);
			key = (pfc_cptr_t)&rbuf;
		}
	}

	/* Derive hash value from the given key. */
	if (strkey != NULL) {
		/*
		 * A key passed from the caller is the raw string, and
		 * `key' now points to a dummy refptr string object.
		 * We need to calculate hash value using raw string because
		 * hash function for refptr string doesn't work with
		 * dummy refptr string.
		 */
		index = refptr_string_hashfunc(strkey, strlen(strkey));
	}
	else {
		index = pfc_hash_ops_hashfunc(hp, key);
	}

	/* Acquire writer lock if needed. */
	cookie = pfc_hash_wrlock(hp);

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		err = EBADF;
		goto out;
	}

	/* Search for a key in the hash table. */
	entpp = pfc_hash_lookup(hp, key, index);
	if ((entp = *entpp) != NULL) {
		uint32_t	options;

		/* Remove this entry. */
		*entpp = entp->phe_next;

		if (valuep == NULL) {
			options = 0;
		}
		else {
			/*
			 * The caller wants removed value. So this hash value
			 * must be left alone.
			 */
			options = ENTREF_IGNORE_VALUE;
			*valuep = entp->phe_value;
		}

		pfc_hash_entry_unref(hp, entp, options);
		free(entp);

		PFC_ASSERT(hp->ph_nelems != 0);
		hp->ph_nelems--;

		pfc_hash_gen_update(hp);
		err = 0;
	}

out:
	pfc_hash_unlock(hp, cookie);

	return err;
}

/*
 * static void
 * pfc_hash_entry_unref(pfc_hashtbl_t *PFC_RESTRICT hp,
 *			pfc_hashent_t *PFC_RESTRICT entp, uint32_t options)
 *	Remove reference to the given hash entry.
 *
 *	If a key to be removed is a refptr object, its reference counter is
 *	decremented. The same thing can be said of hash value.
 *
 *	`options' is a bitmap which determines behavior of this function.
 *	Supported bits are:
 *
 *	ENTREF_IGNORE_KEY
 *		This bit means the key in the specified entry will be retained
 *		in the hash table. So key destructor is never called, and
 *		reference counter of refptr key is never changed.
 *
 *	ENTREF_IGNORE_VALUE
 *		This bit means the value in the specified entry will be
 *		retained in the hash table. So value destructor is never
 *		called, and reference counter of refptr key is never changed.
 */
static void
pfc_hash_entry_unref(pfc_hashtbl_t *PFC_RESTRICT hp,
		     pfc_hashent_t *PFC_RESTRICT entp, uint32_t options)
{
	pfc_cptr_t	key = entp->phe_key;
	pfc_cptr_t	value = entp->phe_value;
	const uint32_t	flags = entp->phe_flags;
	const uint32_t	ignore_key = (options & ENTREF_IGNORE_KEY);
	const uint32_t	ignore_val = (options & ENTREF_IGNORE_VALUE);

	if (PFC_HASH_IS_LIST(hp)) {
		hashlist_t	*hlp = PFC_HASH_TO_LIST(hp);

		/*
		 * Call destructor of hash list element.
		 * Key and value destructors are ignored.
		 */
		hlp->hl_elem_dtor(hlp, value);
	}
	else {
		/* Call key destructor if defined. */
		if (ignore_key == 0 && hp->ph_ops->key_dtor != NULL) {
			hp->ph_ops->key_dtor((pfc_ptr_t)key, flags);
		}

		/* Call value destructor if defined. */
		if (ignore_val == 0 && hp->ph_ops->value_dtor != NULL) {
			hp->ph_ops->value_dtor((pfc_ptr_t)value, flags);
		}
	}

	if (ignore_key == 0 && (flags & PFC_HASHOP_KEY_REFPTR)) {
		pfc_refptr_t *ref = (pfc_refptr_t *)key;

		/* Decrement reference counter of hash key. */
		if (PFC_EXPECT_TRUE(ref != NULL)) {
			pfc_refptr_put(ref);
		}
	}
	if (ignore_val == 0 && (flags & PFC_HASHOP_VAL_REFPTR)) {
		pfc_refptr_t *ref = (pfc_refptr_t *)value;

		/* Decrement reference counter of hash value. */
		if (PFC_EXPECT_TRUE(ref != NULL)) {
			pfc_refptr_put(ref);
		}
	}
}

/*
 * static pfc_hashent_t *
 * pfc_hash_entry_alloc(pfc_cptr_t key, pfc_cptr_t value, uint32_t opflags)
 *	Allocate a new hash entry to keep pair of the given key and value.
 *	NULL is returned on failure.
 */
static pfc_hashent_t *
pfc_hash_entry_alloc(pfc_cptr_t key, pfc_cptr_t value, uint32_t opflags)
{
	pfc_hashent_t	*entp;

	entp = (pfc_hashent_t *)malloc(sizeof(*entp));
	if (PFC_EXPECT_TRUE(entp != NULL)) {
		entp->phe_key = key;
		entp->phe_value = value;
		entp->phe_next = NULL;
		entp->phe_flags = (opflags & ~PFC_IHASHOP_FLAGS);
	}

	return entp;
}

/*
 * static pfc_hashent_t **
 * pfc_hash_lookup(pfc_hashtbl_t *hp, pfc_cptr_t key, uint32_t index)
 *	Search for a hash entry that has the given key in the hash table.
 *	The caller must specify result of pfc_hash_ops_hashfunc() to `index'.
 *
 * Calling/Exit State:
 *	Pointer which contains pointer to hash entry is returned.
 *	If found, a valid pointer to hash entry is contained in returned
 *	address. If not found, NULL is contained in returned address.
 *	Modifying pointer in returned address affects the hash table.
 *
 * Remarks:
 *	Appropriate lock associated with the given hash table must be acquired
 *	by the caller.
 */
static pfc_hashent_t **
pfc_hash_lookup(pfc_hashtbl_t *hp, pfc_cptr_t key, uint32_t index)
{
	pfc_hashent_t	**entpp;

	index %= hp->ph_nbuckets;

	if (hp->ph_kops != NULL) {
		return pfc_hash_lookup_refptr(hp, (pfc_refptr_t *)key, index);
	}
	else {
		for (entpp = hp->ph_table + index; *entpp != NULL;
		     entpp = &((*entpp)->phe_next)) {
			pfc_hashent_t	*entp = *entpp;

			if (pfc_hash_ops_equals(hp, key, entp->phe_key)) {
				break;
			}
		}
	}

	return entpp;
}

/*
 * static pfc_hashent_t **
 * pfc_hash_lookup_refptr(pfc_hashtbl_t *hp, pfc_cptr_t key, uint32_t index)
 *	Search for a hash entry that has the given refptr key in the hash table.
 *
 * Calling/Exit State:
 *	Pointer which contains pointer to hash entry is returned.
 *	If found, a valid pointer to hash entry is contained in returned
 *	address. If not found, NULL is contained in returned address.
 *	Modifying pointer in returned address affects the hash table.
 *
 * Remarks:
 *	pfc_hash_lookup_refptr() is an internal function of pfc_hash_lookup(),
 *	and it is called if the hash table is refptr hash.
 */
static pfc_hashent_t **
pfc_hash_lookup_refptr(pfc_hashtbl_t *hp, pfc_refptr_t *key, uint32_t index)
{
	pfc_hashent_t	**entpp;

	if (key == NULL) {
		/*
		 * NULL key must be processed separately in order to
		 * distinguish NULL key from NULL object in refptr.
		 */
		for (entpp = hp->ph_table + index; *entpp != NULL;
		     entpp = &((*entpp)->phe_next)) {
			pfc_hashent_t	*entp = *entpp;

			if (entp->phe_key == NULL) {
				break;
			}
		}
	}
	else {
		pfc_cptr_t	k = PFC_REFPTR_VALUE(key, pfc_cptr_t);

		/* We need to use hash function in refptr operation. */
		for (entpp = hp->ph_table + index; *entpp != NULL;
		     entpp = &((*entpp)->phe_next)) {
			pfc_hashent_t	*entp = *entpp;
			pfc_refptr_t	*r = (pfc_refptr_t *)entp->phe_key;
			pfc_cptr_t	rk;

			if (r == NULL) {
				continue;
			}
			rk = PFC_REFPTR_VALUE(r, pfc_cptr_t);
			if (pfc_refptr_ops_equals(hp->ph_kops, k, rk)) {
				break;
			}
		}
	}

	return entpp;
}

/*
 * static size_t
 * pfc_hash_clear_l(pfc_hashtbl_t *hp)
 *	Remove all hash entries in the hash table.
 *	The caller must hold hash table lock if needed.
 */
static size_t
pfc_hash_clear_l(pfc_hashtbl_t *hp)
{
	pfc_hashent_t	**entpp;
	size_t		nelems;

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		return 0;
	}

	/* Iterate hash buckets. */
	nelems = hp->ph_nelems;
	for (entpp = hp->ph_table; entpp < hp->ph_table + hp->ph_nbuckets;
	     entpp++) {
		pfc_hashent_t	*entp, *next;

		for (entp = *entpp; entp != NULL; entp = next) {
			next = entp->phe_next;

			/* Remove this entry. */
			pfc_hash_entry_unref(hp, entp, 0);
			free(entp);
#ifdef	PFC_VERBOSE_DEBUG
			PFC_ASSERT(hp->ph_nelems != 0);
			hp->ph_nelems--;
#endif	/* PFC_VERBOSE_DEBUG */
		}
		*entpp = NULL;
	}
	pfc_hash_gen_update(hp);

	hp->ph_nelems = 0;

	return nelems;
}

/*
 * static int
 * pfc_hashiter_get_next(hash_iter_t *PFC_RESTRICT iter,
 *			 pfc_hashtbl_t *PFC_RESTRICT *hp,
 *			 pfc_cptr_t *PFC_RESTRICT keyp,
 *			 pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get next key and value from the hash entry iterator.
 *	This is internal function of __pfc_hashiter_next() and
 *	__pfc_hashiter_uint64_next().
 *
 *	This function is called with holding write lock of the hash table.
 */
static int
pfc_hashiter_get_next(hash_iter_t *PFC_RESTRICT iter,
		      pfc_hashtbl_t *PFC_RESTRICT hp,
		      pfc_cptr_t *PFC_RESTRICT keyp,
		      pfc_cptr_t *PFC_RESTRICT valuep)
{
	pfc_hashent_t	**entpp, *entp;
	uint32_t	index;

	if (PFC_EXPECT_FALSE(hp->ph_table == NULL)) {
		/* The table has been destroyed. */
		return EBADF;
	}

	if (PFC_EXPECT_FALSE(hp->ph_gen != iter->phi_gen)) {
		/* Hash table has been changed. */
		return EINVAL;
	}

	entpp = iter->phi_next;
	index = iter->phi_index;
	while ((entp = *entpp) == NULL) {
		/* Choose next index. */
		index++;
		if (index >= hp->ph_nbuckets) {
			/* No more buckets. */
			return ENOENT;
		}

		entpp = hp->ph_table + index;
	}

	if (keyp != NULL) {
		*keyp = entp->phe_key;
	}
	if (valuep != NULL) {
		*valuep = entp->phe_value;
	}

	/* Update iterator for the next search. */
	iter->phi_index = index;
	iter->phi_next = &entp->phe_next;
	iter->phi_prev = entpp;

	return 0;
}

/*
 * static uint32_t
 * pfc_hash_get_prime(uint32_t max)
 *	Return the largest prime number within the given number.
 */
static uint32_t
pfc_hash_get_prime(uint32_t max)
{
	if (max <= 3) {
		return max;
	}

	if ((max % 2) == 0) {
		/* Ensure that max starts from an odd number. */
		max--;
	}

	for (; max > 3 && !pfc_hash_is_prime(max); max -= 2);

	return max;
}

/*
 * static pfc_bool_t
 * pfc_hash_is_prime(uint32_t number)
 *	Determine whether the given number is a prime number.
 */
static pfc_bool_t
pfc_hash_is_prime(uint32_t number)
{
	uint32_t	i;

	if (number == 2) {
		return PFC_TRUE;
	}
	if (number < 2) {
		return PFC_FALSE;
	}

	/* All even numbers except 2 are not prime number. */
	if ((number % 2) == 0) {
		return PFC_FALSE;
	}

	for (i = 3; i * i <= number; i += 2) {
		if ((number % i) == 0) {
			return PFC_FALSE;
		}
	}

	return PFC_TRUE;
}

#define	HASH_STATE_PERCENT(numerator, denominator)		\
	((double)(numerator) * 100 / (double)(denominator))

/*
 * Width of report columns.
 */
#define	HASH_REPCOL_INDENT		4
#define	HASH_REPCOL_INDEX		6
#define	HASH_REPCOL_LENGTH		10
#define	HASH_REPCOL_RATE		7
#define	HASH_REPCOL_RATE_PERCENT	(HASH_REPCOL_RATE + 2)
#define	HASH_REPCOL_SUM			10
#define	HASH_REPCOL_LENGTH_RATE					\
	(HASH_REPCOL_LENGTH + HASH_REPCOL_RATE_PERCENT + 3)
#define	HASH_REPCOL_SUM_RATE					\
	(HASH_REPCOL_SUM + HASH_REPCOL_RATE_PERCENT + 3)

/*
 * Labels.
 */
static const char	str_empty[] = "";
static const char	str_chain[] = "Bucket Chains";
static const char	str_long_chain[] = "Long Bucket Chains";
static const char	str_short_chain[] = "Short Bucket Chains";
static const char	str_chain_len[] = "Chain Length";
static const char	str_nbuckets[] = "Number of Buckets";
static const char	str_ratio[] = "Ratio";
static const char	str_nentries[] = "Number of Entries";
static const char	str_index[] = "Index";
static const char	str_sum_entries[] = "Sum of Entries";

/*
 * static void
 * hash_report_print_header(FILE *fp)
 *	Print header of hash table report.
 */
static void
hash_report_print_header(FILE *fp)
{
	static const int	columns[] = {
		HASH_REPCOL_INDEX,
		HASH_REPCOL_LENGTH_RATE,
		HASH_REPCOL_SUM_RATE,
	};
	const int	*colp;

	fprintf(fp, "\n%*s%*s %*s %*s\n%*s",
		HASH_REPCOL_INDENT, str_empty,
		HASH_REPCOL_INDEX, str_index,
		HASH_REPCOL_LENGTH_RATE, str_chain_len,
		HASH_REPCOL_SUM_RATE, str_sum_entries,
		HASH_REPCOL_INDENT - 1, str_empty);

	for (colp = columns; colp < PFC_ARRAY_LIMIT(columns); colp++) {
		int	i, width = *colp;

		putc(' ', fp);
		for (i = 0; i < width; i++) {
			putc('-', fp);
		}
	}
	putc('\n', fp);
}

/*
 * void
 * pfc_hash_report(pfc_hash_t hash)
 *	Print bucket chain statistics of the specified hash table to the file
 *	specified by the FILE pointer.
 */
void
pfc_hash_report(pfc_hash_t PFC_RESTRICT hash, FILE *PFC_RESTRICT fp)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**entpp;
	uint32_t	index, nelems, sum;
	int	cookie;

	cookie = pfc_hash_rdlock(hp);

	sum = 0;
	nelems = (uint32_t)hp->ph_nelems;
	fprintf(fp, "  *** Hash table statistics: table=%p, %u entries\n",
		hp, nelems);

	if (nelems != 0) {
		hash_report_print_header(fp);
		for (index = 0, entpp = hp->ph_table;
		     entpp < hp->ph_table + hp->ph_nbuckets; index++, entpp++) {
			pfc_hashent_t	*entp;
			uint32_t	count;

			for (count = 0, entp = *entpp; entp != NULL;
			     count++, entp = entp->phe_next);
			sum += count;
			fprintf(fp, "%*s%*u %*u (%*.3f %%) %*u (%*.3f %%)\n",
				HASH_REPCOL_INDENT, str_empty,
				HASH_REPCOL_INDEX, index,
				HASH_REPCOL_LENGTH, count,
				HASH_REPCOL_RATE,
				HASH_STATE_PERCENT(count, nelems),
				HASH_REPCOL_SUM, sum,
				HASH_REPCOL_RATE,
				HASH_STATE_PERCENT(sum, nelems));
		}
	}

	pfc_hash_unlock(hp, cookie);
}

/*
 * Summary data of hash table state.
 */
typedef struct {
	uint32_t	hse_collision;	/* hash collisions */
	uint32_t	hse_count;	/* number of hash indices */
} hsent_t;

typedef struct {
	hsent_t		*hs_summaries;	/* summary array */
	uint32_t	hs_nelems;	/* number of summary elements */
	uint32_t	hs_curelems;	/* current number of summaries */
} hash_summary_t;

/*
 * Default number of summary array elements.
 */
#define	HASH_SUMMARY_DEFAULT_NELEMS	5U

/*
 * Width of summary columns.
 */
#define	HASH_SUMCOL_INDENT		8
#define	HASH_SUMCOL_LABEL		20
#define	HASH_SUMCOL_LENGTH		10
#define	HASH_SUMCOL_COUNT		10
#define	HASH_SUMCOL_RATE		7
#define	HASH_SUMCOL_RATE_PERCENT	(HASH_SUMCOL_RATE + 2)
#define	HASH_SUMCOL_LENGTH_RATE					\
	(HASH_SUMCOL_LENGTH + HASH_SUMCOL_RATE_PERCENT + 3)
#define	HASH_SUMCOL_COUNT_RATE					\
	(HASH_SUMCOL_COUNT + HASH_SUMCOL_RATE_PERCENT + 3)
#define	HASH_SUMCOL_RATIO		HASH_SUMCOL_RATE_PERCENT

/*
 * static void
 * hash_summary_init(hash_summary_t *sump, uint32_t nelems)
 *	Initialize hash summary data.
 */
static void
hash_summary_init(hash_summary_t *sump, uint32_t nelems)
{
	sump->hs_nelems = nelems;
	sump->hs_curelems = 0;
	sump->hs_summaries = (nelems == 0)
		? NULL
		: (hsent_t *)malloc(sizeof(hsent_t) * nelems);
}

/*
 * static void
 * hash_summary_free(hash_summary_t *sump)
 *	Release all resources reserved by hash_summary_init().
 */
static void
hash_summary_free(hash_summary_t *sump)
{
	free(sump->hs_summaries);
}

/*
 * static void
 * hash_summary_push(hash_summary_t *sump, uint32_t collision, pfc_bool_t large)
 *	Record large collision number into hash table summary array.
 *
 *	If `large' is PFC_TRUE, larger collision numbers are recorded in the
 *	summary array. If PFC_FALSE, smaller collision numbers are recorded.
 */
static void
hash_summary_push(hash_summary_t *sump, uint32_t collision, pfc_bool_t large)
{
	hsent_t		*hsp = sump->hs_summaries, *endp;
	hsent_t		*src, *dst;
	uint32_t	nelems = sump->hs_nelems;
	uint32_t	curelems = sump->hs_curelems;

	if (PFC_EXPECT_FALSE(hsp == NULL)) {
		return;
	}

	if (curelems == 0) {
		/* This is the first element. */
		hsp->hse_collision = collision;
		hsp->hse_count = 1;
		sump->hs_curelems = 1;

		return;
	}

	endp = hsp + curelems;
	for (; hsp < endp; hsp++) {
		if (hsp->hse_collision == collision) {
			/*
			 * The collision number is already register in the
			 * summary array.
			 */
			hsp->hse_count++;
			return;
		}
		if ((large && hsp->hse_collision < collision) ||
		    (!large && hsp->hse_collision > collision)) {
			/*
			 * The collision number entry must be inserted just
			 * before this element.
			 */
			break;
		}
	}

	if (hsp >= endp) {
		if (curelems < nelems) {
			/* Push this collision number to the tail. */
			hsp->hse_collision = collision;
			hsp->hse_count = 1;
			sump->hs_curelems = curelems + 1;
		}
		return;
	}

	/* Shift summary elements. */
	dst = endp;
	if (dst >= sump->hs_summaries + nelems) {
		PFC_ASSERT(dst == sump->hs_summaries + nelems);
		dst--;
	}
	else {
		sump->hs_curelems = curelems + 1;
		PFC_ASSERT(sump->hs_curelems <= sump->hs_nelems);
	}

	for (src = dst - 1; src >= hsp; src--, dst--) {
		dst->hse_collision = src->hse_collision;
		dst->hse_count = src->hse_count;
	}

	hsp->hse_collision = collision;
	hsp->hse_count = 1;
}

/*
 * static void
 * hash_summary_print_header(FILE *fp)
 *	Print header of hash table summary.
 */
static void
hash_summary_print_header(FILE *fp)
{
	static const int	columns[] = {
		HASH_SUMCOL_LENGTH_RATE,
		HASH_SUMCOL_COUNT_RATE,
		HASH_SUMCOL_RATIO,
	};
	const int	*colp;

	fprintf(fp, "\n%*s%*s %*s %*s\n%*s",
		HASH_SUMCOL_INDENT, str_empty,
		HASH_SUMCOL_LENGTH_RATE, str_chain_len,
		HASH_SUMCOL_COUNT_RATE, str_nbuckets,
		HASH_SUMCOL_RATIO, str_ratio,
		HASH_SUMCOL_INDENT - 1, str_empty);

	for (colp = columns; colp < PFC_ARRAY_LIMIT(columns); colp++) {
		int	i, width = *colp;

		putc(' ', fp);
		for (i = 0; i < width; i++) {
			putc('-', fp);
		}
	}
	putc('\n', fp);
}

/*
 * static inline void
 * hash_summary_print_entry(pfc_hashtbl_t *PFC_RESTRICT hp,
 *			    hsent_t *PFC_RESTRICT hsp, FILE *PFC_RESTRICT fp)
 *	Print one summary entry.
 */
static inline void
hash_summary_print_entry(pfc_hashtbl_t *PFC_RESTRICT hp,
			 hsent_t *PFC_RESTRICT hsp, FILE *PFC_RESTRICT fp)
{
	uint32_t	nbuckets = hp->ph_nbuckets;
	uint32_t	nelems = hp->ph_nelems;
	uint32_t	col = hsp->hse_collision;
	uint32_t	count = hsp->hse_count;

	fprintf(fp, "%*s%*u (%*.3f %%) %*u (%*.3f %%) %*.3f %%\n",
		HASH_SUMCOL_INDENT, str_empty,
		HASH_SUMCOL_LENGTH, col,
		HASH_SUMCOL_RATE, HASH_STATE_PERCENT(col, nelems),
		HASH_SUMCOL_COUNT, count,
		HASH_SUMCOL_RATE, HASH_STATE_PERCENT(count, nbuckets),
		HASH_SUMCOL_RATE, HASH_STATE_PERCENT(col * count, nelems));
}

/*
 * static void
 * hash_summary_print(pfc_hashtbl_t *PFC_RESTRICT hp,
 *		      hash_summary_t *PFC_RESTRICT sump, FILE *PFC_RESTRICT fp,
 *		      uint32_t start, pfc_bool_t reverse)
 *	Print summary of hash table state.
 *
 *	`start' is the start index of summary array.
 *	If `reverse' is PFC_TRUE, summary array elements are printed in reverse
 *	order.
 */
static void
hash_summary_print(pfc_hashtbl_t *PFC_RESTRICT hp,
		   hash_summary_t *PFC_RESTRICT sump, FILE *PFC_RESTRICT fp,
		   uint32_t start, pfc_bool_t reverse)
{
	hsent_t		*hsp;

	hsp = sump->hs_summaries + start;

	if (reverse) {
		for (; hsp >= sump->hs_summaries; hsp--) {
			hash_summary_print_entry(hp, hsp, fp);
		}
	}
	else {
		hsent_t	*endp = sump->hs_summaries + sump->hs_curelems;

		for (; hsp < endp; hsp++) {
			hash_summary_print_entry(hp, hsp, fp);
		}
	}
}

/*
 * void
 * pfc_hash_report_summary(pfc_hash_t PFC_RESTRICT hash, FILE *PFC_RESTRICT fp,
 *			   uint32_t nsummaries)
 *	Print summary of hash table state to the file specified by the FILE
 *	pointer.
 *
 *	`nsummaries' determines number of summary entries to be dumped.
 */
void
pfc_hash_report_summary(pfc_hash_t PFC_RESTRICT hash, FILE *PFC_RESTRICT fp,
			uint32_t nsummaries)
{
	pfc_hashtbl_t	*hp = (pfc_hashtbl_t *)hash;
	pfc_hashent_t	**entpp;
	hash_summary_t	larger, smaller;
	uint32_t	nelems, empty, nbuckets;
	int	cookie;

	cookie = pfc_hash_rdlock(hp);

	nelems = (uint32_t)hp->ph_nelems;
	nbuckets = hp->ph_nbuckets;

	if (PFC_EXPECT_FALSE(nsummaries == 0)) {
		nsummaries = HASH_SUMMARY_DEFAULT_NELEMS;
	}

	if (nsummaries > (nbuckets >> 1)) {
		/* We can report state of all buckets. */
		nsummaries = nbuckets;
		hash_summary_init(&larger, nsummaries);
		hash_summary_init(&smaller, 0);
	}
	else {
		hash_summary_init(&larger, nsummaries);
		hash_summary_init(&smaller, nsummaries);
	}

	fprintf(fp, "  *** Summary of hash table state (%p)\n"
		"    - %*s:  %u\n"
		"    - %*s:  %u\n",
		hp,
		-HASH_SUMCOL_LABEL, str_nbuckets, nbuckets,
		-HASH_SUMCOL_LABEL, str_nentries,
		(uint32_t)hp->ph_nelems);

	if (PFC_EXPECT_FALSE(larger.hs_summaries == NULL ||
			     (nsummaries != nbuckets &&
			      smaller.hs_summaries == NULL) ||
			     nelems == 0)) {
		goto out;
	}

	empty = 0;
	for (entpp = hp->ph_table; entpp < hp->ph_table + nbuckets; entpp++) {
		pfc_hashent_t	*entp;
		uint32_t	count;

		for (count = 0, entp = *entpp; entp != NULL;
		     count++, entp = entp->phe_next);

		if (count == 0) {
			empty++;
		}
		else {
			hash_summary_push(&larger, count, PFC_TRUE);
			hash_summary_push(&smaller, count, PFC_FALSE);
		}
	}

	fprintf(fp, "    - %*s:  %u (%7.3f %%)\n",
		-HASH_SUMCOL_LABEL, "Empty Buckets",
		empty, HASH_STATE_PERCENT(empty, nbuckets));

	/* Check to see whether we can merge larger and smaller collisions. */
	PFC_ASSERT(larger.hs_curelems != 0);
	if (smaller.hs_curelems != 0) {
		hsent_t		*hsp, *min;
		uint32_t	mincol;

		min = larger.hs_summaries + larger.hs_curelems - 1;
		mincol = min->hse_collision;
		for (hsp = smaller.hs_summaries + smaller.hs_curelems - 1;
		     hsp >= smaller.hs_summaries; hsp--) {
			if (hsp->hse_collision == mincol) {
				break;
			}
		}

		if (hsp >= smaller.hs_summaries) {
			/* We can merge summaries. */
			fprintf(fp, "    - %*s:\n",
				-HASH_SUMCOL_LABEL, str_chain);
			hash_summary_print_header(fp);
			hash_summary_print(hp, &larger, fp, 0, PFC_FALSE);
			if (hsp != smaller.hs_summaries) {
				uint32_t	start;

				start = hsp - smaller.hs_summaries - 1;
				hash_summary_print(hp, &smaller, fp, start,
						   PFC_TRUE);
			}
		}
		else {
			fprintf(fp, "    - %*s:\n",
				-HASH_SUMCOL_LABEL, str_long_chain);
			hash_summary_print_header(fp);
			hash_summary_print(hp, &larger, fp, 0, PFC_FALSE);
			fprintf(fp, "\n    - %*s:\n",
				-HASH_SUMCOL_LABEL, str_short_chain);
			hash_summary_print_header(fp);
			hash_summary_print(hp, &smaller, fp, 0, PFC_FALSE);
		}
	}
	else {
		fprintf(fp, "    - %*s:\n",
			-HASH_SUMCOL_LABEL, str_chain);
		hash_summary_print_header(fp);
		hash_summary_print(hp, &larger, fp, 0, PFC_FALSE);
	}

out:
	pfc_hash_unlock(hp, cookie);
	hash_summary_free(&larger);
	hash_summary_free(&smaller);
}
