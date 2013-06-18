/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * hashlist.c -
 *	List implementation which uses doubly linked list and hash table.
 */

#include <pfc/list.h>
#include <pfc/hash.h>
#include <pfc/synch.h>
#include <pfc/debug.h>
#include "listmodel_impl.h"
#include "hash_impl.h"

#define	LIST2ELEM(list)		PFC_CAST_CONTAINER(list, hlist_elem_t, hle_list)

/*
 * Return internal hash table identifier.
 */
#define	PFC_HASHLIST_TABLE(hlp)	((pfc_hash_t)&(hlp)->hl_hash)

/*
 * Convert list instance.
 */
#define	MODEL_TO_HASHLIST(model)			\
	PFC_CAST_CONTAINER(model, hashlist_t, hl_model)
#define	HASHLIST_TO_MODEL(hlp)		(&(hlp)->hl_model)

/*
 * Internal prototypes.
 */
static void	pfc_hashlist_dtor(listmodel_t *model);
static int	pfc_hashlist_push(listmodel_t *PFC_RESTRICT model,
				  pfc_cptr_t PFC_RESTRICT value);
static int	pfc_hashlist_push_tail(listmodel_t *PFC_RESTRICT model,
				       pfc_cptr_t PFC_RESTRICT value);
static int	pfc_hashlist_pop(listmodel_t *PFC_RESTRICT model,
				 pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_pop_tail(listmodel_t *PFC_RESTRICT model,
				      pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_first(listmodel_t *PFC_RESTRICT model,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_last(listmodel_t *PFC_RESTRICT model,
				  pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_index(listmodel_t *PFC_RESTRICT model,
				   pfc_cptr_t PFC_RESTRICT value);
static int	pfc_hashlist_remove(listmodel_t *PFC_RESTRICT model,
				    pfc_cptr_t PFC_RESTRICT value);
static int	pfc_hashlist_getat(listmodel_t *PFC_RESTRICT model, int index,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_insertat(listmodel_t *PFC_RESTRICT model,
				      int index, pfc_cptr_t PFC_RESTRICT value);
static int	pfc_hashlist_removeat(listmodel_t *PFC_RESTRICT model,
				      int index,
				      pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_hashlist_sort(listmodel_t *PFC_RESTRICT model,
				  listsort_t *PFC_RESTRICT lsp);
static void	pfc_hashlist_clear(listmodel_t *model);
static void	pfc_hashlist_iter_ctor(listiter_t *PFC_RESTRICT iter);
static void	pfc_hashlist_iter_next(listiter_t *PFC_RESTRICT iter,
				       pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_hashlist_iter_prev(listiter_t *PFC_RESTRICT iter,
				       pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_hashlist_iter_rewind(listiter_t *PFC_RESTRICT iter);
static void	pfc_hashlist_iter_eol(listiter_t *PFC_RESTRICT iter);
static void	pfc_hashlist_iter_remove(listiter_t *iter);

static void	pfc_hashlist_rdlock(listmodel_t *model);
static void	pfc_hashlist_wrlock(listmodel_t *model);
static void	pfc_hashlist_unlock(listmodel_t *model);

static int	pfc_hashlist_init(pfc_listm_t *PFC_RESTRICT listp,
				  const pfc_refptr_ops_t *PFC_RESTRICT refops,
				  const pfc_hash_ops_t *PFC_RESTRICT ops,
				  uint32_t nbuckets, uint32_t hflags,
				  uint8_t flags);
static void	pfc_hashlist_element_dtor(hashlist_t *hlp, pfc_cptr_t value);
static void	pfc_hashlist_refptr_dtor(hashlist_t *hlp, pfc_cptr_t value);
static int	pfc_hashlist_push_element(listmodel_t *PFC_RESTRICT model,
					  pfc_cptr_t PFC_RESTRICT value,
					  pfc_bool_t tail);
static int	pfc_hashlist_pop_element(listmodel_t *PFC_RESTRICT model,
					 pfc_cptr_t *PFC_RESTRICT valuep,
					 pfc_bool_t tail);
static int	pfc_hashlist_get_edge(listmodel_t *PFC_RESTRICT model,
				      pfc_cptr_t *PFC_RESTRICT valuep,
				      pfc_bool_t tail);
static hlist_elem_t	*pfc_hashlist_lookup_by_index(hashlist_t *hlp,
						      int index);
static int	pfc_hashlist_sort_comp(const void *v1, const void *v2);
static int	pfc_hashlist_sort_refcomp(const void *v1, const void *v2);

/*
 * List model operations for hash list implementation.
 */
static const listmodel_ops_t	hashlist_ops = {
	.type		= PFC_LISTM_HASH,
	.has_rwlock	= PFC_TRUE,

	.dtor		= pfc_hashlist_dtor,
	.push		= pfc_hashlist_push,
	.push_tail	= pfc_hashlist_push_tail,
	.pop		= pfc_hashlist_pop,
	.pop_tail	= pfc_hashlist_pop_tail,
	.first		= pfc_hashlist_first,
	.last		= pfc_hashlist_last,

	/*
	 * Hash list contains unique elements. So `index' and `index_tail'
	 * must return the same index. `remove' and `remove_tail' removes
	 * the same element.
	 */
	.index		= pfc_hashlist_index,
	.index_tail	= pfc_hashlist_index,
	.remove		= pfc_hashlist_remove,
	.remove_tail	= pfc_hashlist_remove,

	.getat		= pfc_hashlist_getat,
	.insertat	= pfc_hashlist_insertat,
	.removeat	= pfc_hashlist_removeat,
	.clear		= pfc_hashlist_clear,

	.rdlock		= pfc_hashlist_rdlock,
	.wrlock		= pfc_hashlist_wrlock,
	.unlock		= pfc_hashlist_unlock,

	/* Sort operation */
	.sort		= pfc_hashlist_sort,

	/* Iterator operations */
	.iter_ctor	= pfc_hashlist_iter_ctor,
	.iter_next	= pfc_hashlist_iter_next,
	.iter_prev	= pfc_hashlist_iter_prev,
	.iter_rewind	= pfc_hashlist_iter_rewind,
	.iter_eol	= pfc_hashlist_iter_eol,
	.iter_remove	= pfc_hashlist_iter_remove,
};

/*
 * Optimized lock operation with considering synchronized block.
 */

/*
 * static inline int
 * PFC_HASHLIST_RDLOCK(hashlist_t *hlp)
 *	Acquire reader lock of the list, if needed.
 *	Returned value must be passed to PFC_HASHLIST_UNLOCK().
 */
static inline int
PFC_HASHLIST_RDLOCK(hashlist_t *hlp)
{
	int	cookie;

	if (pfc_listm_sync_lookup(HASHLIST_TO_MODEL(hlp)) == NULL) {
		(void)pfc_rwlock_rdlock(&hlp->hl_lock);
		cookie = 1;
	}
	else {
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline int
 * PFC_HASHLIST_WRLOCK(hashlist_t *hlp)
 *	Acquire writer lock of the list, if needed.
 *	Returned value must be passed to PFC_HASHLIST_UNLOCK().
 */
static inline int
PFC_HASHLIST_WRLOCK(hashlist_t *hlp)
{
	listmodel_t	*model = HASHLIST_TO_MODEL(hlp);
	int		cookie;
	__pfc_syncblock_t	*sbp;

	/* Check to see whether we're in synchronized block. */
	sbp = pfc_listm_sync_lookup(model);
	if (sbp == NULL) {
		(void)pfc_rwlock_wrlock(&hlp->hl_lock);
		cookie = 1;
	}
	else {
		/* Writer mode is required. */
		PFC_ASSERT(PFC_SYNCBLOCK_IS_WRITE(sbp));
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline void
 * PFC_HASHLIST_UNLOCK(hashlist_t *hlp, int cookie)
 *	Release lock of the list, if needed.
 */
static inline void
PFC_HASHLIST_UNLOCK(hashlist_t *hlp, int cookie)
{
	if (cookie) {
		(void)pfc_rwlock_unlock(&hlp->hl_lock);
	}
}

/*
 * static inline hlist_elem_t *
 * pfc_hashlist_element_alloc(pfc_cptr_t value)
 *	Create a new list element which keeps the given value.
 */
static inline hlist_elem_t *
pfc_hashlist_element_alloc(pfc_cptr_t value)
{
	hlist_elem_t	*elem;

	elem = (hlist_elem_t *)malloc(sizeof(*elem));
	if (PFC_EXPECT_FALSE(elem != NULL)) {
		elem->hle_value = value;
	}

	return elem;
}

/*
 * int
 * pfc_hashlist_create(pfc_listm_t *PFC_RESTRICT listp,
 *		       const pfc_hash_ops_t *PFC_RESTRICT ops,
 *		       uint32_t nbuckets, uint32_t hflags)
 *
 *	A list instance created by this function treats list element as
 *	a pointer address. You must need to use pfc_hashlist_create_ref()
 *	if you want to add refptr object into hash list.
 *
 *	`ops', `nbuckets', and `hflags' are passed to the constructor
 *	of the internal hash table. Note that key and value destructor in
 *	pfc_hash_ops_t are ignored.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_hashlist_create(pfc_listm_t *PFC_RESTRICT listp,
		    const pfc_hash_ops_t *PFC_RESTRICT ops,
		    uint32_t nbuckets, uint32_t hflags)
{
	return pfc_hashlist_init(listp, NULL, ops, nbuckets, hflags, 0);
}

/*
 * int
 * pfc_hashlist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
 *			   const pfc_refptr_ops_t *PFC_RESTRICT refops,
 *			   uint32_t nbuckets, uint32_t hflags)
 *	Create a new hash list which contains refptr objects.
 *
 *	Refptr operation pointer must be specified to `*refops', and all
 *	list elements must have the same operation pointer.
 *
 *	`refops', `nbuckets', and `hflags' are passed to the constructor
 *	of the internal hash table.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_hashlist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
			const pfc_refptr_ops_t *PFC_RESTRICT refops,
			uint32_t nbuckets, uint32_t hflags)
{
	if (refops == NULL) {
		/* Use default refptr operation. */
		refops = &refptr_default_ops;
	}

	return pfc_hashlist_init(listp, refops, NULL, nbuckets, hflags, 0);
}

/*
 * int
 * pfc_hashlist_get_capacity(pfc_listm_t list)
 *	Return current capacity of the internal hash table.
 *
 * Calling/Exit State:
 *	Upon successful completion, current capacity is returned.
 *	PFC_LISTIDX_BAD is returned if the specified list is not valid.
 */
int
pfc_hashlist_get_capacity(pfc_listm_t list)
{
	listmodel_t	*model = (listmodel_t *)list;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	int	capacity, cookie;

	/* Ensure the list is hash list. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &hashlist_ops)) {
		return PFC_LISTIDX_BAD;
	}

	cookie = PFC_HASHLIST_RDLOCK(hlp);
	capacity = (int)pfc_hash_get_capacity(hash);
	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return capacity;
}

/*
 * int
 * pfc_hashlist_set_capacity(pfc_listm_t list, int capacity)
 *	Set capacity of the internal hash table to the specified value.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_hashlist_set_capacity(pfc_listm_t list, int capacity)
{
	listmodel_t	*model = (listmodel_t *)list;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	int	err, cookie;

	/* Ensure the list is hash list. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &hashlist_ops)) {
		return EINVAL;
	}

	/* Verify the specified capacity. */
	if (PFC_EXPECT_FALSE(capacity <= 0)) {
		return EINVAL;
	}

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_TRUE(err == 0)) {
		err = pfc_hash_set_capacity(hash, capacity);
	}

	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return err;
}

/*
 * int
 * pfc_hashlist_create_i64(pfc_listm_t *PFC_RESTRICT listp, uint32_t nbuckets,
 *			   uint32_t hflags)
 *
 *	Create a new hash list which contains signed 64-bit integer.
 *
 *	`nbuckets', and `hflags' are passed to the constructor of the internal
 *	 hash table.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_hashlist_create_i64(pfc_listm_t *PFC_RESTRICT listp, uint32_t nbuckets,
			uint32_t hflags)
{
	const uint8_t	flags = LISTMF_INT64;

	return pfc_hashlist_init(listp, pfc_listm_refops64(flags), NULL,
				 nbuckets, hflags, flags);
}

/*
 * int
 * pfc_hashlist_create_u64(pfc_listm_t *PFC_RESTRICT listp, uint32_t nbuckets,
 *			   uint32_t hflags)
 *
 *	Create a new hash list which contains unsigned 64-bit integer.
 *
 *	`nbuckets', and `hflags' are passed to the constructor of the internal
 *	 hash table.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_hashlist_create_u64(pfc_listm_t *PFC_RESTRICT listp, uint32_t nbuckets,
			uint32_t hflags)
{
	const uint8_t	flags = LISTMF_UINT64;

	return pfc_hashlist_init(listp, pfc_listm_refops64(flags), NULL,
				 nbuckets, hflags, flags);
}

/*
 * static void
 * pfc_hashlist_dtor(listmodel_t *model)
 *	Destructor of the hash list.
 */
static void
pfc_hashlist_dtor(listmodel_t *model)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);

	PFC_ASSERT(pfc_rwlock_destroy(&hlp->hl_lock) == 0);

	pfc_hash_destroy(PFC_HASHLIST_TABLE(hlp));
	free(hlp);
}

/*
 * static int
 * pfc_hashlist_push(listmodel_t *PFC_RESTRICT model,
 *		     pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the head of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_hashlist_push(listmodel_t *PFC_RESTRICT model,
		  pfc_cptr_t PFC_RESTRICT value)
{
	return pfc_hashlist_push_element(model, value, PFC_FALSE);
}

/*
 * static int
 * pfc_hashlist_push_tail(listmodel_t *PFC_RESTRICT model,
 *		          pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the tail of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_hashlist_push_tail(listmodel_t *PFC_RESTRICT model,
		       pfc_cptr_t PFC_RESTRICT value)
{
	return pfc_hashlist_push_element(model, value, PFC_TRUE);
}

/*
 * static int
 * pfc_hashlist_pop(listmodel_t *PFC_RESTRICT model,
 *		    pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the first element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_hashlist_pop(listmodel_t *PFC_RESTRICT model,
		 pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hashlist_pop_element(model, valuep, PFC_FALSE);
}

/*
 * static int
 * pfc_hashlist_pop_tail(listmodel_t *PFC_RESTRICT model,
 *		         pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the last element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep', and zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_hashlist_pop_tail(listmodel_t *PFC_RESTRICT model,
		      pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hashlist_pop_element(model, valuep, PFC_TRUE);
}

/*
 * static int
 * pfc_hashlist_first(listmodel_t *PFC_RESTRICT model,
 *		      pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the first element in the list.
 *	This function doesn't remove any element.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is set to
 *	`*valuep', and zero is returned.
 *	ENOENT is returned if no element exists in the list.
 *
 * Remarks:
 *	This function is called with holding reader lock of the list.
 */
static int
pfc_hashlist_first(listmodel_t *PFC_RESTRICT model,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hashlist_get_edge(model, valuep, PFC_FALSE);
}

/*
 * static int
 * pfc_hashlist_last(listmodel_t *PFC_RESTRICT model,
 *		     pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the last element in the list.
 *	This function doesn't remove any element.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is set to
 *	`*valuep', and zero is returned.
 *	ENOENT is returned if no element exists in the list.
 *
 * Remarks:
 *	This function is called with holding reader lock of the list.
 */
static int
pfc_hashlist_last(listmodel_t *PFC_RESTRICT model,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_hashlist_get_edge(model, valuep, PFC_TRUE);
}

/*
 * static int
 * pfc_hashlist_index(listmodel_t *PFC_RESTRICT model,
 *		      pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the first occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
static int
pfc_hashlist_index(listmodel_t *PFC_RESTRICT model,
		   pfc_cptr_t PFC_RESTRICT value)
{
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_list_t	*lelem;
	int	index, cookie;

	cookie = PFC_HASHLIST_RDLOCK(hlp);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		index = PFC_LISTIDX_BAD;
		goto out;
	}

	index = 0;
	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		PFC_LIST_FOREACH(&hlp->hl_head, lelem) {
			hlist_elem_t	*elem = LIST2ELEM(lelem);

			if (pfc_listm_refptr_equals(refops, elem->hle_value,
						    obj)) {
				/* Found. */
				goto out;
			}
			index++;
		}
	}
	else {
		PFC_LIST_FOREACH(&hlp->hl_head, lelem) {
			hlist_elem_t	*elem = LIST2ELEM(lelem);

			if (elem->hle_value == value) {
				/* Found. */
				goto out;
			}
			index++;
		}
	}

	/* Not found. */
	index = PFC_LISTIDX_NOENT;

out:
	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return index;
}

/*
 * static int
 * pfc_hashlist_remove(listmodel_t *PFC_RESTRICT model,
 *		       pfc_cptr_t PFC_RESTRICT value)
 *	Remove the first occurrence of the specified value in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed from the list.
 *	ENOENT is returned if not present.
 *	EBADF is returned if the list has been already destroyed.
 */
static int
pfc_hashlist_remove(listmodel_t *PFC_RESTRICT model,
		    pfc_cptr_t PFC_RESTRICT value)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	int	err, cookie;

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);

		/* Remove this element from the table and the list. */
		err = pfc_hash_remove(hash, value, hlp->hl_hopflags);
	}

	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return err;
}

/*
 * static int
 * pfc_hashlist_getat(listmodel_t *PFC_RESTRICT model, int index,
 *		      pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the list element at the specified index.
 *
 * Calling/Exit State:
 *	Zero is returned if found.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is called with holding reader lock of the list.
 */
static int
pfc_hashlist_getat(listmodel_t *PFC_RESTRICT model, int index,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	hlist_elem_t	*elem;

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	elem = pfc_hashlist_lookup_by_index(hlp, index);
	PFC_ASSERT(elem != NULL);

	if (valuep != NULL) {
		*valuep = elem->hle_value;
	}

	return 0;
}

/*
 * static int
 * pfc_hashlist_insertat(listmodel_t *PFC_RESTRICT model, int index,
 *			 pfc_cptr_t PFC_RESTRICT value)
 *	Insert the list element at the specified index.
 *	Each element in the list with index greater or equal to the specified
 *	index is shifted upward.
 *
 * Calling/Exit State:
 *	Zero is returned if found.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_hashlist_insertat(listmodel_t *PFC_RESTRICT model, int index,
		      pfc_cptr_t PFC_RESTRICT value)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	hlist_elem_t	*elem;
	pfc_list_t	*lhead;
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	int	err, cookie;

	elem = pfc_hashlist_element_alloc(value);
	if (PFC_EXPECT_FALSE(elem == NULL)) {
		return ENOMEM;
	}

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto error;
	}

	if (index == model->lm_nelems) {
		/* Link to the tail. */
		lhead = &hlp->hl_head;
	}
	else {
		hlist_elem_t	*old;

		old = pfc_hashlist_lookup_by_index(hlp, index);
		if (PFC_EXPECT_FALSE(old == NULL)) {
			err = ENOENT;
			goto error;
		}

		/* Link before this element. */
		lhead = &old->hle_list;
	}

	/* Put the value into the hash table. */
	err = pfc_hash_put(hash, value, (pfc_cptr_t)elem, hlp->hl_hopflags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_list_push_tail(lhead, &elem->hle_list);
	pfc_listm_nelem_inc(model);

	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return 0;

error:
	PFC_HASHLIST_UNLOCK(hlp, cookie);
	free(elem);

	return err;
}

/*
 * static int
 * pfc_hashlist_removeat(listmodel_t *PFC_RESTRICT model, int index,
 *			 pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove the list element at the specified index.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list element at the specified index
 *	is removed, and then zero is returned. Removed value is set to
 *	`*valuep' if `valuep' is not NULL.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_hashlist_removeat(listmodel_t *PFC_RESTRICT model, int index,
		      pfc_cptr_t *PFC_RESTRICT valuep)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	int	err, cookie;

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_TRUE(err == 0)) {
		hlist_elem_t	*elem;
		pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);

		elem = pfc_hashlist_lookup_by_index(hlp, index);
		if (PFC_EXPECT_FALSE(elem == NULL)) {
			err = ENOENT;
		}
		else {
			/* Remove this element. */
			if (valuep != NULL) {
				*valuep = elem->hle_value;
			}

			if (!PFC_IS_LP64_SYSTEM() &&
			    PFC_LISTM_IS_64BIT(model)) {
				pfc_refptr_t	*ref =
					(pfc_refptr_t *)elem->hle_value;

				/*
				 * If the list is 64-bit vector on the 32-bit
				 * system, we must not decrement refptr's
				 * reference counter. It will be done by upper
				 * layer.
				 */
				pfc_refptr_get(ref);
			}
			PFC_ASSERT_INT(pfc_hash_remove(hash, elem->hle_value,
						       hlp->hl_hopflags), 0);
		}
	}

	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return err;
}

/*
 * static int
 * pfc_hashlist_sort(listmodel_t *PFC_RESTRICT model,
 *		     listsort_t *PFC_RESTRICT lsp)
 *	Sort elements in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function is called with holding writer lock of the list.
 */
static int
pfc_hashlist_sort(listmodel_t *PFC_RESTRICT model, listsort_t *PFC_RESTRICT lsp)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_ptr_t	*buffer, *bp;
	pfc_list_t	*lelem;
	int	nelems = model->lm_nelems;

	PFC_ASSERT(nelems > 0);

	/* Allocate sort buffer. */
	buffer = (pfc_ptr_t *)malloc(sizeof(pfc_cptr_t) * nelems);
	if (PFC_EXPECT_FALSE(buffer == NULL)) {
		return ENOMEM;
	}

	/* Copy all elements into the sort buffer. */
	bp = buffer;
	PFC_LIST_FOREACH(&hlp->hl_head, lelem) {
		hlist_elem_t	*elem = LIST2ELEM(lelem);

		*bp = (pfc_ptr_t)elem;
		bp++;
	}
	PFC_ASSERT(bp == buffer + nelems);

	/* Do sort. */
	if (lsp->lms_refcomp) {
		qsort(buffer, nelems, sizeof(pfc_ptr_t),
		      pfc_hashlist_sort_refcomp);
	}
	else {
		qsort(buffer, nelems, sizeof(pfc_ptr_t),
		      pfc_hashlist_sort_comp);
	}

	/* Reconstruct linked list in sorted order. */
	pfc_list_init(&hlp->hl_head);
	for (bp = buffer; bp < buffer + nelems; bp++) {
		hlist_elem_t	*elem = (hlist_elem_t *)*bp;

		pfc_list_push_tail(&hlp->hl_head, &elem->hle_list);
	}

	free(buffer);

	return 0;
}

/*
 * static void
 * pfc_hashlist_clear(listmodel_t *model)
 *	Remove all elements from the list.
 *	This method is called with holding writer lock of the list.
 */
static void
pfc_hashlist_clear(listmodel_t *model)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);

	PFC_ASSERT_INT(pfc_hash_clear(hash), model->lm_nelems);
}

/*
 * static void
 * pfc_hashlist_iter_ctor(listiter_t *PFC_RESTRICT iter)
 *	Initialize the iterator to iterate linked list.
 */
static void
pfc_hashlist_iter_ctor(listiter_t *PFC_RESTRICT iter)
{
	listmodel_t	*model = iter->lmi_model;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_list_t	*lelem;
	int	from = iter->lmi_from;

	/* Set next list element into the iterator. */
	if (from == 0) {
		lelem = hlp->hl_head.pl_next;
	}
	else if (from >= model->lm_nelems) {
		lelem = &hlp->hl_head;
	}
	else {
		hlist_elem_t	*elem;

		elem = pfc_hashlist_lookup_by_index(hlp, from);
		PFC_ASSERT(elem != NULL);
		lelem = &elem->hle_list;
	}

	/*
	 * Iterator for hash list keeps next pfc_list_t element in
	 * lmi_next, and previously retrieved linked list element in
	 * lmi_element.
	 */
	iter->lmi_next = (pfc_ptr_t)lelem;
	iter->lmi_element = NULL;

	return;
}

/*
 * static void
 * pfc_hashlist_iter_next(listiter_t *PFC_RESTRICT iter,
 *			  pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the next element in the list iterator, and move iterator's cursor
 *	forward.
 */
static void
pfc_hashlist_iter_next(listiter_t *PFC_RESTRICT iter,
		       pfc_cptr_t *PFC_RESTRICT valuep)
{
	pfc_list_t	*lelem;
	hlist_elem_t	*elem;
#ifdef	PFC_VERBOSE_DEBUG
	listmodel_t	*model = iter->lmi_model;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
#endif	/* PFC_VERBOSE_DEBUG */

	lelem = (pfc_list_t *)iter->lmi_next;
	PFC_ASSERT(lelem != &hlp->hl_head);

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->hle_value;
	}

	/* Update iterator. */
	iter->lmi_next = (pfc_ptr_t)lelem->pl_next;
	iter->lmi_element = (pfc_ptr_t)elem;
}

/*
 * static void
 * pfc_hashlist_iter_prev(listiter_t *PFC_RESTRICT iter,
 *			  pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the previous element in the list iterator, and move iterator's
 *	cursor backward.
 */
static void
pfc_hashlist_iter_prev(listiter_t *PFC_RESTRICT iter,
		       pfc_cptr_t *PFC_RESTRICT valuep)
{
	pfc_list_t	*lelem;
	hlist_elem_t	*elem;
#ifdef	PFC_VERBOSE_DEBUG
	listmodel_t	*model = iter->lmi_model;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
#endif	/* PFC_VERBOSE_DEBUG */

	lelem = (pfc_list_t *)iter->lmi_next;
	lelem = lelem->pl_prev;
	PFC_ASSERT(lelem != &hlp->hl_head);

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->hle_value;
	}

	/* Update iterator. */
	iter->lmi_next = (pfc_ptr_t)lelem;
	iter->lmi_element = (pfc_ptr_t)elem;
}

/*
 * static void
 * pfc_hashlist_iter_rewind(listiter_t *PFC_RESTRICT iter)
 *	Rewind iterator's cursor.
 *	A subsequent call to pfc_listiter_next() will return the first element
 *	in the list.
 */
static void
pfc_hashlist_iter_rewind(listiter_t *PFC_RESTRICT iter)
{
	/* We can simply call iterator's constructor. */
	pfc_hashlist_iter_ctor(iter);
}

/*
 * static void
 * pfc_hashlist_iter_eol(listiter_t *PFC_RESTRICT iter)
 *	Move iterator's cursor to the end of the list.
 *	A subsequent call to pfc_listiter_prev() will return the last element
 *	in the list.
 */
static void
pfc_hashlist_iter_eol(listiter_t *PFC_RESTRICT iter)
{
	listmodel_t	*model = iter->lmi_model;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);

	iter->lmi_next = &hlp->hl_head;
	iter->lmi_element = NULL;
}

/*
 * static void
 * pfc_hashlist_iter_remove(listiter_t *iter)
 *	Remove the element retrieved by the previous pfc_listiter_next() or
 *	pfc_listiter_prev() call.
 *
 * Remarks:
 *	This function is called with holding writer lock of the list.
 */
static void
pfc_hashlist_iter_remove(listiter_t *iter)
{
	listmodel_t	*model = iter->lmi_model;
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	hlist_elem_t	*elem = (hlist_elem_t *)iter->lmi_element;
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	pfc_list_t	*lelem;

	PFC_ASSERT(elem != NULL);

	/* Remove this element. */
	lelem = (pfc_list_t *)iter->lmi_next;
	if (lelem == &elem->hle_list) {
		iter->lmi_next = (pfc_ptr_t)lelem->pl_next;
	}

	PFC_ASSERT_INT(pfc_hash_remove(hash, elem->hle_value,
				       hlp->hl_hopflags), 0);

	/* Update iterator. */
	iter->lmi_element = NULL;
}

/*
 * static void
 * pfc_hashlist_rdlock(listmodel_t *model)
 *	Acquire reader lock of the list.
 */
static void
pfc_hashlist_rdlock(listmodel_t *model)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);

	(void)pfc_rwlock_rdlock(&hlp->hl_lock);
}

/*
 * static void
 * pfc_hashlist_wrlock(listmodel_t *model)
 *	Acquire writer lock of the list.
 */
static void
pfc_hashlist_wrlock(listmodel_t *model)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);

	(void)pfc_rwlock_wrlock(&hlp->hl_lock);
}

/*
 * static void
 * pfc_hashlist_unlock(listmodel_t *model)
 *	Release the list lock.
 */
static void
pfc_hashlist_unlock(listmodel_t *model)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);

	(void)pfc_rwlock_unlock(&hlp->hl_lock);
}

/*
 * static int
 * pfc_hashlist_init(pfc_listm_t *PFC_RESTRICT listp,
 *		     const pfc_refptr_ops_t *PFC_RESTRICT refops,
 *		     const pfc_hash_ops_t *PFC_RESTRICT ops, uint32_t nbuckets,
 *		     uint32_t hflags, uint8_t flags)
 *	Common routine to create a new hash list.
 */
static int
pfc_hashlist_init(pfc_listm_t *PFC_RESTRICT listp,
		  const pfc_refptr_ops_t *PFC_RESTRICT refops,
		  const pfc_hash_ops_t *PFC_RESTRICT ops, uint32_t nbuckets,
		  uint32_t hflags, uint8_t flags)
{
	hashlist_t	*hlp;
	listmodel_t	*model;
	pfc_hash_t	hash;
	uint32_t	hopflags;
	hashlist_dtor_t	dtor;
	int	err;

	/* Allocate list instance. */
	hlp = (hashlist_t *)malloc(sizeof(*hlp));
	if (PFC_EXPECT_FALSE(hlp == NULL)) {
		return ENOMEM;
	}

	hflags &= PFC_IHASH_FLAGS_MASK;
	hflags |= PFC_IHASH_LIST;

	if (refops != NULL) {
		/* Create a refptr hash table. Ignore hash operations. */
		hopflags = PFC_HASHOP_KEY_REFPTR;
		dtor = pfc_hashlist_refptr_dtor;
		ops = NULL;
	}
	else {
		/* Create a normal hash table. */
		hopflags = 0;
		dtor = pfc_hashlist_element_dtor;
	}

	/* Initialize internal hash table. */
	hash = PFC_HASHLIST_TABLE(hlp);
	err = pfc_hash_table_init(&hash, ops, refops, nbuckets, hflags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_free;
	}
	PFC_ASSERT(hash == PFC_HASHLIST_TABLE(hlp));

	model = HASHLIST_TO_MODEL(hlp);

	err = pfc_rwlock_init(&hlp->hl_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_listm_init(model, &hashlist_ops, refops, flags);

	pfc_list_init(&hlp->hl_head);
	hlp->hl_hopflags = hopflags;
	hlp->hl_elem_dtor = dtor;

	*listp = (pfc_listm_t)model;

	return 0;

error:
	pfc_hash_destroy(hash);

error_free:
	free(hlp);

	return err;
}

/*
 * static void
 * pfc_hashlist_element_dtor(hashlist_t *hlp, pfc_cptr_t value)
 *	Destructor of list element.
 *	This function is called if the list is non-refptr list.
 */
static void
pfc_hashlist_element_dtor(hashlist_t *hlp, pfc_cptr_t value)
{
	listmodel_t	*model = HASHLIST_TO_MODEL(hlp);
	hlist_elem_t	*elem = (hlist_elem_t *)value;
	pfc_list_t	*lelem = &elem->hle_list;

	if (lelem->pl_next != NULL) {
		/* Unlink from the list. */
		pfc_list_remove(lelem);
	}

	pfc_listm_nelem_dec(model);
	free(elem);
}

/*
 * static void
 * pfc_hashlist_refptr_dtor(hashlist_t *hlp, pfc_cptr_t value)
 *	Destructor of list element.
 *	This function is called if the list is refptr list.
 */
static void
pfc_hashlist_refptr_dtor(hashlist_t *hlp, pfc_cptr_t value)
{
	listmodel_t	*model = HASHLIST_TO_MODEL(hlp);
	hlist_elem_t	*elem = (hlist_elem_t *)value;
	pfc_list_t	*lelem = &elem->hle_list;

	if (lelem->pl_next != NULL) {
		/* Unlink from the list. */
		pfc_list_remove(lelem);
	}

	pfc_listm_refptr_put(elem->hle_value);
	pfc_listm_nelem_dec(model);
	free(elem);
}

/*
 * static int
 * pfc_hashlist_push_element(listmodel_t *PFC_RESTRICT model,
 *			     pfc_cptr_t PFC_RESTRICT value, pfc_bool_t tail)
 *	Common operation to push a new element to the list.
 *	If `tail' is true, an element is added to the tail of the list,
 *	otherwise the head of the list.
 */
static int
pfc_hashlist_push_element(listmodel_t *PFC_RESTRICT model,
			  pfc_cptr_t PFC_RESTRICT value, pfc_bool_t tail)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	hlist_elem_t	*elem;
	int	err, cookie;

	elem = pfc_hashlist_element_alloc(value);
	if (PFC_EXPECT_FALSE(elem == NULL)) {
		return ENOMEM;
	}

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto error;
	}

	/* Put the value into the hash table. */
	err = pfc_hash_put(hash, value, (pfc_cptr_t)elem, hlp->hl_hopflags);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	if (tail) {
		/* Link element onto the tail of the list. */
		pfc_list_push_tail(&hlp->hl_head, &elem->hle_list);
	}
	else {
		/* Link element onto the head of the list. */
		pfc_list_push(&hlp->hl_head, &elem->hle_list);
	}
	pfc_listm_nelem_inc(model);

	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return 0;

error:
	PFC_HASHLIST_UNLOCK(hlp, cookie);
	free(elem);

	return err;
}

/*
 * static int
 * pfc_hashlist_pop_element(listmodel_t *PFC_RESTRICT model,
 *			    pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
 *	Common operation to pop an entry from the list.
 *	If `tail' is true, the list element at the tail of the list is removed
 *	from the list, otherwise the head of the list.
 */
static int
pfc_hashlist_pop_element(listmodel_t *PFC_RESTRICT model,
			 pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	hlist_elem_t	*elem;
	pfc_list_t	*lelem;
	pfc_hash_t	hash = PFC_HASHLIST_TABLE(hlp);
	int	err, cookie;

	cookie = PFC_HASHLIST_WRLOCK(hlp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	if (tail) {
		/* Unlink the last element. */
		lelem = pfc_list_pop_tail(&hlp->hl_head);
	}
	else {
		/* Unlink the first element. */
		lelem = pfc_list_pop(&hlp->hl_head);
	}

	if (PFC_EXPECT_FALSE(lelem == NULL)) {
		err = ENOENT;
		goto out;
	}

	/*
	 * Clear next and previous link to tell the destructor that the element
	 * has been unlinked.
	 */
	lelem->pl_next = lelem->pl_prev = NULL;

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->hle_value;
	}

	if (!PFC_IS_LP64_SYSTEM() && PFC_LISTM_IS_64BIT(model)) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)elem->hle_value;

		/*
		 * If the list is 64-bit vector on the 32-bit system,
		 * we must not decrement refptr's reference counter.
		 * It will be done by upper layer.
		 */
		pfc_refptr_get(ref);
	}

	/* Remove from the table and the list. */
	PFC_ASSERT_INT(pfc_hash_remove(hash, elem->hle_value,
				       hlp->hl_hopflags), 0);

out:
	PFC_HASHLIST_UNLOCK(hlp, cookie);

	return err;
}

/*
 * static int
 * pfc_hashlist_get_edge(listmodel_t PFC_RESTRICT model,
 *			 pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
 *	Common operation to get an entry at the edge of the list.
 *	If `tail' is true, the last element of the list is set to `*valuep',
 *	otherwise the first.
 *
 * Remarks:
 *	This function is called with holding reader lock of the list.
 */
static int
pfc_hashlist_get_edge(listmodel_t *PFC_RESTRICT model,
		      pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
{
	hashlist_t	*hlp = MODEL_TO_HASHLIST(model);
	hlist_elem_t	*elem;
	pfc_list_t	*lelem;

	PFC_ASSERT(model->lm_nelems > 0);
	PFC_ASSERT(!pfc_list_is_empty(&hlp->hl_head));

	if (tail) {
		/* Get the last element. */
		lelem = hlp->hl_head.pl_prev;
	}
	else {
		/* Get the first element. */
		lelem = hlp->hl_head.pl_next;
	}
	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->hle_value;
	}

	return 0;
}

/*
 * static hlist_elem_t *
 * pfc_hashlist_lookup_by_index(hashlist_t *hlp, int index)
 *	Search for the list element at the specified index.
 *
 * Calling/Exit State:
 *	Pointer to the list element at the specified index is returned
 *	if present. NULL is returned if not present.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock
 *	of the list.
 */
static hlist_elem_t *
pfc_hashlist_lookup_by_index(hashlist_t *hlp, int index)
{
	listmodel_t	*model = HASHLIST_TO_MODEL(hlp);
	pfc_list_t	*lelem;
	int	cnt, nelems;

	nelems = model->lm_nelems;
	if (PFC_EXPECT_FALSE(index < 0 || index >= nelems)) {
		return NULL;
	}

	if (index < (nelems >> 1)) {
		/* Search from the head. */
		cnt = 0;
		PFC_LIST_FOREACH(&hlp->hl_head, lelem) {
			if (cnt == index) {
				break;
			}
			cnt++;
		}
	}
	else {
		/* Search from the tail. */
		cnt = nelems;
		PFC_LIST_REV_FOREACH(&hlp->hl_head, lelem) {
			cnt--;
			if (cnt == index) {
				break;
			}
		}
	}

	PFC_ASSERT(lelem != &hlp->hl_head);

	return LIST2ELEM(lelem);
}

/*
 * static int
 * pfc_hashlist_sort_comp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 */
static int
pfc_hashlist_sort_comp(const void *v1, const void *v2)
{
	hlist_elem_t	*e1 = *((hlist_elem_t **)v1);
	hlist_elem_t	*e2 = *((hlist_elem_t **)v2);
	const void	*o1 = e1->hle_value;
	const void	*o2 = e2->hle_value;
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return lsp->lms_comp(o1, o2);
}

/*
 * static int
 * pfc_hashlist_sort_refcomp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 *	This comparator is used if the list is refptr list.
 */
static int
pfc_hashlist_sort_refcomp(const void *v1, const void *v2)
{
	hlist_elem_t	*e1 = *((hlist_elem_t **)v1);
	hlist_elem_t	*e2 = *((hlist_elem_t **)v2);
	const void	*o1 = e1->hle_value;
	const void	*o2 = e2->hle_value;
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return pfc_listm_refptr_comparator(lsp, o1, o2);
}
