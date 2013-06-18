/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * listmodel.c - Abstract list model operations.
 */

#include <pfc/listmodel.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include "listmodel_impl.h"

/*
 * Cast list instance.
 */
#define	LISTM_MODEL(list)	((listmodel_t *)(list))

/*
 * Synchronized block frame created by pfc_listm_wsync_begin() or
 * pfc_listm_rsync_begin().
 * This list is thread private, so we don't need any lock to access this list.
 */
__thread __pfc_syncblock_t	*listm_sync_frame PFC_ATTR_HIDDEN;

/*
 * Current sort context.
 */
__thread listsort_t		*listm_sort_context PFC_ATTR_HIDDEN;

/*
 * Internal prototypes.
 */
static void	pfc_listiter_init(listiter_t *PFC_RESTRICT iter,
				  listmodel_t *PFC_RESTRICT model,
				  int from, int to);
static int	pfc_listm_iter_next(listiter_t *PFC_RESTRICT iter,
				    listmodel_t *PFC_RESTRICT model,
				    pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_listm_iter_prev(listiter_t *PFC_RESTRICT iter,
				    listmodel_t *PFC_RESTRICT model,
				    pfc_cptr_t *PFC_RESTRICT valuep);

#ifdef	PFC_LP64
static int	pfc_listm64_signed_comp(const void *o1, const void *o2);
#endif	/* PFC_LP64 */

/*
 * int
 * pfc_listm_push(pfc_listm_t PFC_RESTRICT list, pfc_cptr PFC_RESTRICT value)
 *	Push a list element onto the head of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm_push(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	err = pfc_listm_verify_element(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = model->lm_ops->push(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_listm_unref_element(model, value);
	}

	return err;
}

/*
 * int
 * pfc_listm_push_tail(pfc_listm_t PFC_RESTRICT list,
 *		       pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the tail of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm_push_tail(pfc_listm_t PFC_RESTRICT list,
		    pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	err = pfc_listm_verify_element(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = model->lm_ops->push_tail(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_listm_unref_element(model, value);
	}

	return err;
}

/*
 * int
 * __pfc_listm_pop(pfc_listm_t PFC_RESTRICT list,
 *		   pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the first element from the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
int
__pfc_listm_pop(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	return model->lm_ops->pop(model, valuep);
}

/*
 * int
 * __pfc_listm_pop_tail(pfc_listm_t PFC_RESTRICT list,
 *			pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the last element from the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
int
__pfc_listm_pop_tail(pfc_listm_t PFC_RESTRICT list,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	return model->lm_ops->pop_tail(model, valuep);
}

/*
 * int
 * __pfc_listm_first(pfc_listm_t PFC_RESTRICT list,
 *		     pfc_cptr_t *PFC_RESTRICT valuep)
 *	Return the first element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is set to
 *	`*valuep' if `valuep' is not NULL, and then zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
int
__pfc_listm_first(pfc_listm_t PFC_RESTRICT list,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (model->lm_nelems == 0) {
		/* No elements in the list. */
		err = ENOENT;
		goto out;
	}

	err = model->lm_ops->first(model, valuep);

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * __pfc_listm_last(pfc_listm_t PFC_RESTRICT list,
 *		    pfc_cptr_t *PFC_RESTRICT valuep)
 *	Return the last element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is set to
 *	`*valuep' if `valuep' is not NULL, and then zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
int
__pfc_listm_last(pfc_listm_t PFC_RESTRICT list,
		 pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (model->lm_nelems == 0) {
		/* No elements in the list. */
		err = ENOENT;
		goto out;
	}

	err = model->lm_ops->last(model, valuep);

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listm_index(pfc_listm_t PFC_RESTRICT list,
 *		   pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the first occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value is returned on success if found.
 *	List index starts from zero.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listm_index(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return PFC_LISTIDX_INVALID;
	}

	return model->lm_ops->index(model, value);
}

/*
 * int
 * pfc_listm_index_tail(pfc_listm_t PFC_RESTRICT list,
 *			pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the last occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value is returned on success if found.
 *	List index starts from zero.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listm_index_tail(pfc_listm_t PFC_RESTRICT list,
		     pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return PFC_LISTIDX_INVALID;
	}

	return model->lm_ops->index_tail(model, value);
}

/*
 * int
 * pfc_listm_remove(pfc_listm_t PFC_RESTRICT list,
 *		    pfc_cptr_t PFC_RESTRICT value)
 *	Remove the first occurrence of the specified value from the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed.
 *	ENOENT is returned if not found.
 */
int
pfc_listm_remove(pfc_listm_t PFC_RESTRICT list, pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	return model->lm_ops->remove(model, value);
}

/*
 * int
 * pfc_listm_remove_tail(pfc_listm_t PFC_RESTRICT list,
 *			 pfc_cptr_t PFC_RESTRICT value)
 *	Remove the last occurrence of the specified value from the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed.
 *	ENOENT is returned if not found.
 */
int
pfc_listm_remove_tail(pfc_listm_t PFC_RESTRICT list,
		      pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	return model->lm_ops->remove_tail(model, value);
}

/*
 * int
 * __pfc_listm_getat(pfc_listm_t PFC_RESTRICT list, int index,
 *		     pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the list element at the specified index.
 *
 * Calling/Exit State:
 *	Zero is returned if found. If `valuep' is not NULL, the element
 *	is set to `*valuep'.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_listm_getat(pfc_listm_t PFC_RESTRICT list, int index,
		  pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(index >= model->lm_nelems)) {
		err = ENOENT;
		goto out;
	}

	err = model->lm_ops->getat(model, index, valuep);

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listm_insertat(pfc_listm_t PFC_RESTRICT list, int index,
 *		      pfc_cptr_t PFC_RESTRICT value)
 *	Insert the list element at the specified index.
 *	Each element in the list with index greater or equal to the specified
 *	index is shifted upward.
 *
 * Calling/Exit State:
 *	Zero is returned if found.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm_insertat(pfc_listm_t PFC_RESTRICT list, int index,
		   pfc_cptr_t PFC_RESTRICT value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	err = pfc_listm_verify_element(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	err = model->lm_ops->insertat(model, index, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_listm_unref_element(model, value);
	}

	return err;
}

/*
 * int
 * __pfc_listm_removeat(pfc_listm_t PFC_RESTRICT list, int index,
 *			pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove the list element at the specified index.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list element at the specified index
 *	is removed, and then zero is returned. Removed value is set to
 *	`*valuep' if `valuep' is not NULL.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_listm_removeat(pfc_listm_t PFC_RESTRICT list, int index,
		     pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	return model->lm_ops->removeat(model, index, valuep);
}

/*
 * int
 * pfc_listm_clear(pfc_listm_t list)
 *	Remove all elements in the list.
 *
 * Calling/Exit State:
 *	Number of removed elements is returned.
 */
int
pfc_listm_clear(pfc_listm_t list)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	nelems, cookie;

	cookie = pfc_listm_wrlock(model);

	nelems = model->lm_nelems;
	if (nelems != 0) {
		model->lm_ops->clear(model);
		model->lm_nelems = 0;
		pfc_listm_gen_update(model);
	}

	pfc_listm_unlock(model, cookie);

	return nelems;
}

/*
 * int
 * pfc_listm_sort_comp(pfc_listm_t list, pfc_listcomp_t comp)
 *	Sort elements in the list using the specified comparator.
 *	Default comparator is used if NULL is specified as comparator.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm_sort_comp(pfc_listm_t list, pfc_listcomp_t comp)
{
	listmodel_t	*model = LISTM_MODEL(list);
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	refptr_comp_t	refcomp = NULL;
	listsort_t	lsort, **lspp;
	int	err, cookie;

	if (comp == NULL) {
		/* Use default comparator. */
		if (refops != NULL) {
			refcomp = pfc_refptr_get_comparator(refops);
		}
#ifdef	PFC_LP64
		else if (model->lm_flags & LISTMF_INT64) {
			/* Compare elements as signed 64-bit integer. */
			comp = pfc_listm64_signed_comp;
		}
#endif	/* PFC_LP64 */
		else {
			/*
			 * Use default refptr comparator, which compares
			 * values as address.
			 */
			comp = pfc_refptr_compare_default;
		}
	}

	cookie = pfc_listm_wrlock(model);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		goto out;
	}

	if (model->lm_nelems < 2) {
		/* No need to sort. */
		goto out;
	}

	/* Initialize sort context. */
	lsort.lms_comp = comp;
	lsort.lms_refcomp = refcomp;

	/* Install sort context. */
	lspp = &listm_sort_context;
	*lspp = &lsort;

	/* Do sort. */
	err = model->lm_ops->sort(model, &lsort);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/* Update generation number. */
		pfc_listm_gen_update(model);
	}

	/* Clear sort context. */
	*lspp = NULL;

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_listm_refptr_comparator(listsort_t *lsp, const void *v1, const void *v2)
 *	Refptr object comparator.
 *	This function assumes that the given refptr objects have the same
 *	refptr operation.
 */
int PFC_ATTR_HIDDEN
pfc_listm_refptr_comparator(listsort_t *lsp, const void *v1, const void *v2)
{
	pfc_refptr_t	*r1 = (pfc_refptr_t *)v1;
	pfc_refptr_t	*r2 = (pfc_refptr_t *)v2;
	const void	*o1, *o2;

	if (r1 == NULL || r2 == NULL) {
		/* Treat NULL as the least value. */
		if (r1 == r2) {
			return 0;
		}

		return (r1 < r2) ? -1 : 1;
	}

	o1 = pfc_refptr_value(r1);
	o2 = pfc_refptr_value(r2);

	return lsp->lms_refcomp(o1, o2);
}

/*
 * int
 * pfc_listm_get_size(pfc_listm_t list)
 *	Return number of elements in the list.
 */
int
pfc_listm_get_size(pfc_listm_t list)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	nelems, cookie;

	cookie = pfc_listm_rdlock(model);
	nelems = model->lm_nelems;
	pfc_listm_unlock(model, cookie);

	return nelems;
}

/*
 * pfc_listm_type_t
 * pfc_listm_get_type(pfc_listm_t list)
 *	Return identifier of the list implementation.
 */
pfc_listm_type_t
pfc_listm_get_type(pfc_listm_t list)
{
	listmodel_t	*model = LISTM_MODEL(list);

	return model->lm_ops->type;
}

/*
 * void
 * pfc_listm_destroy(pfc_listm_t list)
 *	Destroy the specified list.
 *	The caller must guarantee that no one refers the list.
 */
void
pfc_listm_destroy(pfc_listm_t list)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	cookie;

	cookie = pfc_listm_wrlock(model);

	/* Remove all elements. */
	model->lm_ops->clear(model);

	/* Mark the list as destroyed. */
	model->lm_valid = PFC_FALSE;

	pfc_listm_unlock(model, cookie);

	/* Release the list if no one refers the list. */
	pfc_listm_unref(model);

	return;
}

/*
 * void
 * __pfc_listm_sync_begin(__pfc_syncblock_t *frame)
 *	Start the list synchronization block.
 *	This function is assumed to be called via pfc_listm_rsync_begin() or
 *	pfc_listm_wsync_begin().
 */
void
__pfc_listm_sync_begin(__pfc_syncblock_t *frame)
{
	listmodel_t	*model = (listmodel_t *)frame->psb_target;

	if (!model->lm_ops->has_rwlock) {
		/* Force to update the frame as writer mode. */
		frame->psb_flags |= PFC_SYNCBLOCK_WRITE;
	}

	if (pfc_syncblock_enter(&listm_sync_frame, frame)) {
		const listmodel_ops_t *const ops = model->lm_ops;

		/* Acquire lock. */
		if (PFC_SYNCBLOCK_IS_WRITE(frame)) {
			ops->wrlock(model);
		}
		else {
			ops->rdlock(model);
		}
		pfc_listm_ref(model);
	}
}

/*
 * void
 * __pfc_listm_sync_end(void *arg)
 *	Quit the list synchronized block.
 *	Actual lock is released if the given frame is the last frame.
 */
void
__pfc_listm_sync_end(void *arg)
{
	__pfc_syncblock_t	*frame = (__pfc_syncblock_t *)arg;
	listmodel_t	*model = (listmodel_t *)frame->psb_target;

	if (pfc_syncblock_exit(&listm_sync_frame, (pfc_cptr_t)model)) {
		/* Now leaving from all synchronized blocks. */
		model->lm_ops->unlock(model);
		pfc_listm_unref(model);
	}
}

/*
 * pfc_listiter_t
 * pfc_listiter_create(pfc_listm_t list)
 *	Create a new list iterator to iterate whole list.
 *
 * Calling/Exit State:
 *	Upon successful completion, list iterator is returned.
 *	NULL is returned on failure.
 */
pfc_listiter_t
pfc_listiter_create(pfc_listm_t list)
{
	listmodel_t	*model = LISTM_MODEL(list);
	listiter_t	*iter;
	int	cookie;

	/* Allocate iterator. */
	iter = (listiter_t *)malloc(sizeof(*iter));
	if (PFC_EXPECT_FALSE(iter == NULL)) {
		return NULL;
	}

	/* Acquire writer lock. */
	cookie = pfc_listm_wrlock(model);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		/* This list has been destroyed. */
		pfc_listm_unlock(model, cookie);
		free(iter);

		return NULL;
	}

	/* Initialize iterator. */
	pfc_listiter_init(iter, model, 0, model->lm_nelems);

	pfc_listm_unlock(model, cookie);

	return (pfc_listiter_t)iter;
}

/*
 * pfc_listiter_t
 * pfc_listiter_create_range(pfc_listm_t list, int from, int to)
 *	Create a new list iterator to iterate the list between the element
 *	at `from' (inclusive) and `to' (exclusive).
 *
 * Calling/Exit State:
 *	Upon successful completion, list iterator is returned.
 *	NULL is returned on failure.
 */
pfc_listiter_t
pfc_listiter_create_range(pfc_listm_t list, int from, int to)
{
	listmodel_t	*model = LISTM_MODEL(list);
	listiter_t	*iter;
	int	cookie;

	/* Allocate iterator. */
	iter = (listiter_t *)malloc(sizeof(*iter));
	if (PFC_EXPECT_FALSE(iter == NULL)) {
		return NULL;
	}

	/* Acquire writer lock. */
	cookie = pfc_listm_wrlock(model);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		/* This list has been destroyed. */
		pfc_listm_unlock(model, cookie);
		free(iter);

		return NULL;
	}

	if (PFC_EXPECT_FALSE(from < 0 || to > model->lm_nelems ||
			     from > to)) {
		/* Invalid range. */
		pfc_listm_unlock(model, cookie);
		free(iter);

		return NULL;
	}

	/* Initialize iterator. */
	pfc_listiter_init(iter, model, from, to);

	pfc_listm_unlock(model, cookie);

	return (pfc_listiter_t)iter;
}

/*
 * void
 * pfc_listiter_destroy(pfc_listiter_t it)
 *	Destroy the list iterator.
 */
void
pfc_listiter_destroy(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	cookie;

	cookie = pfc_listm_wrlock(model);

	/* Remove the specified iterator from the active list. */
	pfc_list_remove(&iter->lmi_list);

	/* Destroy the iterator. */
	if (model->lm_ops->iter_dtor != NULL) {
		model->lm_ops->iter_dtor(iter);
	}
	free(iter);

	pfc_listm_unlock(model, cookie);

	pfc_listm_unref(model);
}

/*
 * int
 * __pfc_listiter_next(pfc_listiter_t PFC_RESTRICT it,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the next element in the list iterator, and move iterator's cursor
 *	forward.
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 */
int
__pfc_listiter_next(pfc_listiter_t PFC_RESTRICT it,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	/*
	 * Acquire writer lock to serialize access to the list and
	 * this iterator.
	 */
	cookie = pfc_listm_wrlock(model);
	err = pfc_listm_iter_next(iter, model, valuep);
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * __pfc_listiter_prev(pfc_listiter_t PFC_RESTRICT it,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the previous element in the list iterator, and move iterator's
 *	cursor backward.
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 */
int
__pfc_listiter_prev(pfc_listiter_t PFC_RESTRICT it,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	/*
	 * Acquire writer lock to serialize access to the list and
	 * this iterator.
	 */
	cookie = pfc_listm_wrlock(model);
	err = pfc_listm_iter_prev(iter, model, valuep);
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listiter_next_index(pfc_listiter_t it)
 *	Return index of the element which will be returned by a subsequent
 *	pfc_listiter_next() call.
 *
 * Calling/Exit State:
 *	Index of the next element is returned if present.
 *	PFC_LISTIDX_NOENT is returned if no more element.
 *	PFC_LISTIDX_CHANGED is returned if the list has been changed.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listiter_next_index(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	index, cookie;

	cookie = pfc_listm_rdlock(model);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		index = PFC_LISTIDX_BAD;
		goto out;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		index = PFC_LISTIDX_CHANGED;
		goto out;
	}

	index = iter->lmi_nextidx;
	PFC_ASSERT(index >= iter->lmi_from);

	if (PFC_EXPECT_FALSE(index >= iter->lmi_to)) {
		/* No more element. */
		index = PFC_LISTIDX_NOENT;
	}

out:
	pfc_listm_unlock(model, cookie);

	return index;
}

/*
 * int
 * pfc_listiter_prev_index(pfc_listiter_t it)
 *	Return index of the element which will be returned by a subsequent
 *	pfc_listiter_prev() call.
 *
 * Calling/Exit State:
 *	Index of the previous element is returned if present.
 *	PFC_LISTIDX_NOENT is returned if no more element.
 *	PFC_LISTIDX_CHANGED is returned if the list has been changed.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listiter_prev_index(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	index, cookie;

	cookie = pfc_listm_rdlock(model);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		index = PFC_LISTIDX_BAD;
		goto out;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		index = PFC_LISTIDX_CHANGED;
		goto out;
	}

	PFC_ASSERT(iter->lmi_nextidx <= iter->lmi_to);

	index = iter->lmi_nextidx - 1;
	if (PFC_EXPECT_FALSE(index < iter->lmi_from)) {
		/* No more element. */
		index = PFC_LISTIDX_NOENT;
	}

out:
	pfc_listm_unlock(model, cookie);

	return index;
}

/*
 * int
 * pfc_listiter_rewind(pfc_listiter_t it)
 *	Rewind iterator's cursor.
 *	A subsequent call to pfc_listiter_next() will return the first element
 *	in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if succeeded.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	EBADF is returned if the list has been destroyed.
 */
int
pfc_listiter_rewind(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	cookie = pfc_listm_wrlock(model);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		goto error;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		err = EINVAL;
		goto error;
	}

	/* Rewind cursor. */
	iter->lmi_nextidx = iter->lmi_from;
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;
	model->lm_ops->iter_rewind(iter);

error:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listiter_eol(pfc_listiter_t it)
 *	Move iterator's cursor to the end of the list.
 *	A subsequent call to pfc_listiter_prev() will return the last element
 *	in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if succeeded.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	EBADF is returned if the list has been destroyed.
 */
int
pfc_listiter_eol(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	cookie = pfc_listm_wrlock(model);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		goto error;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		err = EINVAL;
		goto error;
	}

	/* Rewind cursor. */
	iter->lmi_nextidx = iter->lmi_to;
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;
	model->lm_ops->iter_eol(iter);

error:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listiter_remove(pfc_listiter_t it)
 *	Remove the element retrieved by the previous pfc_listiter_next() or
 *	pfc_listiter_prev() call.
 *
 * Calling/Exit State:
 *	Zero is returned if the element has been removed.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	EBADF is returned if the list has been destroyed.
 *	ENOENT is returned if pfc_listiter_next() or pfc_listiter_remove() is
 *	not yet called, or the element has been already removed.
 */
int
pfc_listiter_remove(pfc_listiter_t it)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	cookie = pfc_listm_wrlock(model);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		err = EINVAL;
		goto out;
	}

	if (PFC_EXPECT_FALSE(iter->lmi_elemidx < 0)) {
		/* Not yet retrieved, or already removed. */
		err = ENOENT;
		goto out;
	}

	/* Remove the element. */
	model->lm_ops->iter_remove(iter);

	/* Update iterator. */
	iter->lmi_gen = model->lm_gen;
	iter->lmi_nextidx = iter->lmi_elemidx;
	iter->lmi_to--;
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;
	PFC_ASSERT(iter->lmi_nextidx >= iter->lmi_from);

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * static void
 * pfc_listiter_init(listiter_t *PFC_RESTRICT iter,
 *		     listmodel_t *PFC_RESTRICT model, int from, int to)
 *	Initialize iterator instance.
 */
static void
pfc_listiter_init(listiter_t *PFC_RESTRICT iter,
		  listmodel_t *PFC_RESTRICT model, int from, int to)
{
	/* Copy current generation number. */
	iter->lmi_gen = model->lm_gen;

	/* Insert iterator onto the active iterator list. */
	pfc_list_push(&model->lm_iterator, &iter->lmi_list);

	/* Initialize iterator. */
	iter->lmi_model = model;
	iter->lmi_from = from;
	iter->lmi_to = to;
	iter->lmi_nextidx = from;
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;
	model->lm_ops->iter_ctor(iter);

	pfc_listm_ref(model);
}

/*
 * static int
 * pfc_listm_iter_next(listiter_t *PFC_RESTRICT iter,
 *		       listmodel_t *PFC_RESTRICT model,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Common routine for pfc_listiter_next().
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the list.
 */
static int
pfc_listm_iter_next(listiter_t *PFC_RESTRICT iter,
		    listmodel_t *PFC_RESTRICT model,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	int	err, index;

	/*
	 * Reset lmi_elemidx so that subsequent iterator remove fails
	 * if iterator prev fails.
	 */
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		return err;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		return EINVAL;
	}

	index = iter->lmi_nextidx;
	PFC_ASSERT(index >= iter->lmi_from);

	if (PFC_EXPECT_FALSE(index >= iter->lmi_to)) {
		/* No more element. */
		return ENOENT;
	}

	/* Get next element, and move cursor forward. */
	model->lm_ops->iter_next(iter, valuep);
	iter->lmi_elemidx = index;
	iter->lmi_nextidx = index + 1;

	return 0;
}

/*
 * static int
 * pfc_listm_iter_prev(listiter_t *PFC_RESTRICT iter,
 *		       listmodel_t *PFC_RESTRICT model,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Common routine for pfc_listiter_prev().
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the list.
 */
static int
pfc_listm_iter_prev(listiter_t *PFC_RESTRICT iter,
		    listmodel_t *PFC_RESTRICT model,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	int	err, index;

	/*
	 * Reset lmi_elemidx so that subsequent iterator remove fails
	 * if iterator prev fails.
	 */
	iter->lmi_elemidx = PFC_LISTIDX_NOENT;

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list has been destroyed. */
		return err;
	}

	if (PFC_EXPECT_FALSE(model->lm_gen != iter->lmi_gen)) {
		/* The list has been changed. */
		return EINVAL;
	}

	PFC_ASSERT(iter->lmi_nextidx <= iter->lmi_to);

	index = iter->lmi_nextidx - 1;
	if (PFC_EXPECT_FALSE(index < iter->lmi_from)) {
		/* No more element. */
		return ENOENT;
	}

	/* Get previous element, and move cursor backward. */
	model->lm_ops->iter_prev(iter, valuep);
	iter->lmi_elemidx = index;
	iter->lmi_nextidx = index;

	return 0;
}

/*
 * Below are 64-bit list specific operations.
 */

/*
 * int
 * pfc_listm64_push(pfc_listm_t list, uint64_t value)
 *	64-bit list version of pfc_listm_push().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm64_push(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->push(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);
		pfc_cptr_t	rvalue = (pfc_cptr_t)ref;

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		err = model->lm_ops->push(model, rvalue);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * int
 * pfc_listm64_push_tail(pfc_listm_t list, uint64_t value)
 *	64-bit list version of pfc_listm_push_tail().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm64_push_tail(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			push_tail(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);
		pfc_cptr_t	rvalue = (pfc_cptr_t)ref;

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		err = model->lm_ops->push_tail(model, rvalue);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * int
 * pfc_listm64_pop(pfc_listm_t PFC_RESTRICT list,
 *		   uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_pop().
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
int
pfc_listm64_pop(pfc_listm_t PFC_RESTRICT list, uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->pop(model, (pfc_cptr_t *)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = model->lm_ops->pop(model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/*
			 * List backend doesn't decrement reference counter
			 * on 64-bit list. So we must decrement here.
			 */
			pfc_listm64_unref_refptr(ref, valuep);
		}
	}

	return err;
}

/*
 * int
 * pfc_listm64_pop_tail(pfc_listm_t PFC_RESTRICT list,
 *			uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_pop_tail().
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
int
pfc_listm64_pop_tail(pfc_listm_t PFC_RESTRICT list,
		     uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->pop_tail(model, (pfc_cptr_t *)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = model->lm_ops->pop_tail(model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/*
			 * List backend doesn't decrement reference counter
			 * on 64-bit list. So we must decrement here.
			 */
			pfc_listm64_unref_refptr(ref, valuep);
		}
	}

	return err;
}

/*
 * int
 * pfc_listm64_first(pfc_listm_t PFC_RESTRICT list,
 *		     uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_first().
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is set to
 *	`*valuep' if `valuep' is not NULL, and then zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
int
pfc_listm64_first(pfc_listm_t PFC_RESTRICT list, uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (model->lm_nelems == 0) {
		/* No elements in the list. */
		err = ENOENT;
		goto out;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			first(model, (pfc_cptr_t *)(uintptr_t)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = model->lm_ops->first(model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0) && valuep != NULL) {
			*valuep = pfc_listm64_eval_refptr(ref);
		}
	}

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listm64_last(pfc_listm_t PFC_RESTRICT list,
 *		    uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_last().
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is set to
 *	`*valuep' if `valuep' is not NULL, and then zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
int
pfc_listm64_last(pfc_listm_t PFC_RESTRICT list, uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (model->lm_nelems == 0) {
		/* No elements in the list. */
		err = ENOENT;
		goto out;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			last(model, (pfc_cptr_t *)(uintptr_t)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = model->lm_ops->last(model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0) && valuep != NULL) {
			*valuep = pfc_listm64_eval_refptr(ref);
		}
	}

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listm64_index(pfc_listm_t list, uint64_t value)
 *	64-bit list version of pfc_listm_index().
 *
 * Calling/Exit State:
 *	Index value is returned on success if found.
 *	List index starts from zero.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listm64_index(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	index;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return PFC_LISTIDX_INVALID;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		index = model->lm_ops->
			index(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return PFC_LISTIDX_NOMEM;
		}

		index = model->lm_ops->index(model, (pfc_cptr_t)ref);
		pfc_refptr_put(ref);
	}

	return index;
}

/*
 * int
 * pfc_listm64_index_tail(pfc_listm_t list, uint64_t value)
 *	64-bit list version of pfc_listm_index_tail().
 *
 * Calling/Exit State:
 *	Index value is returned on success if found.
 *	List index starts from zero.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
int
pfc_listm64_index_tail(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	index;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return PFC_LISTIDX_INVALID;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		index = model->lm_ops->
			index_tail(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return PFC_LISTIDX_NOMEM;
		}

		index = model->lm_ops->index_tail(model, (pfc_cptr_t)ref);
		pfc_refptr_put(ref);
	}

	return index;
}

/*
 * int
 * pfc_listm64_remove(pfc_listm_t list, uint64_t value)
 *	64-bit list version of pfc_listm_remove().
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed.
 *	ENOENT is returned if not found.
 */
int
pfc_listm64_remove(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			remove(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		err = model->lm_ops->remove(model, (pfc_cptr_t)ref);
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_listm64_remove_tail(pfc_listm_t PFC_RESTRICT list, uint64_t value)
 *	64-bit list version of pfc_listm_remove_tail().
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed.
 *	ENOENT is returned if not found.
 */
int
pfc_listm64_remove_tail(pfc_listm_t list, uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			remove_tail(model, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		err = model->lm_ops->remove_tail(model, (pfc_cptr_t)ref);
		pfc_refptr_put(ref);
	}

	return err;
}

/*
 * int
 * pfc_listm64_getat(pfc_listm_t PFC_RESTRICT list, int index,
 *		     uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_getat().
 *
 * Calling/Exit State:
 *	Zero is returned if found. If `valuep' is not NULL, the element
 *	is set to `*valuep'.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm64_getat(pfc_listm_t PFC_RESTRICT list, int index,
		  uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err, cookie;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	cookie = pfc_listm_rdlock(model);
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(index >= model->lm_nelems)) {
		err = ENOENT;
		goto out;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			getat(model, index, (pfc_cptr_t *)(uintptr_t)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = model->lm_ops->getat(model, index, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0) && valuep != NULL) {
			*valuep = pfc_listm64_eval_refptr(ref);
		}
	}

out:
	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listm64_insertat(pfc_listm_t PFC_RESTRICT list, int index,
 *			uint64_t value)
 *	64-bit list version of pfc_listm_insertat().
 *
 * Calling/Exit State:
 *	Zero is returned if found.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm64_insertat(pfc_listm_t PFC_RESTRICT list, int index,
		     uint64_t value)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = model->lm_ops->
			insertat(model, index, (pfc_cptr_t)(uintptr_t)value);
	}
	else {
		pfc_refptr_t	*ref = pfc_listm_create_ref64(model, value);

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}

		err = model->lm_ops->insertat(model, index, (pfc_cptr_t)ref);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * int
 * pfc_listm64_removeat(pfc_listm_t PFC_RESTRICT list, int index,
 *			uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listm_removeat().
 *
 * Calling/Exit State:
 *	Upon successful completion, the list element at the specified index
 *	is removed, and then zero is returned. Removed value is set to
 *	`*valuep' if `valuep' is not NULL.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_listm64_removeat(pfc_listm_t PFC_RESTRICT list, int index,
		     uint64_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = LISTM_MODEL(list);
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}
	if (PFC_EXPECT_FALSE(index < 0)) {
		return ENOENT;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err =  model->lm_ops->removeat(model, index,
					       (pfc_cptr_t *)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err =  model->lm_ops->removeat(model, index,
					       (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/*
			 * List backend doesn't decrement reference counter
			 * on 64-bit list. So we must decrement here.
			 */
			pfc_listm64_unref_refptr(ref, valuep);
		}
	}

	return err;
}

/*
 * int
 * pfc_listiter64_next(pfc_listiter_t PFC_RESTRICT it,
 *			 uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listiter_next().
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 */
int
pfc_listiter64_next(pfc_listiter_t PFC_RESTRICT it,
		    uint64_t *PFC_RESTRICT valuep)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	/*
	 * Acquire writer lock to serialize access to the list and
	 * this iterator.
	 */
	cookie = pfc_listm_wrlock(model);

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = pfc_listm_iter_next(iter, model, (pfc_cptr_t *)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = pfc_listm_iter_next(iter, model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0) && valuep != NULL) {
			*valuep = pfc_listm64_eval_refptr(ref);
		}
	}

	pfc_listm_unlock(model, cookie);

	return err;
}

/*
 * int
 * pfc_listiter64_prev(pfc_listiter_t PFC_RESTRICT it,
 *			 uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_listiter_prev().
 *
 * Calling/Exit State:
 *	Upon successful completion, the element value is set to `*valuep'
 *	if `valuep' is not NULL, and zero is returned.
 *
 *	EINVAL is returned if modification of the list is detected.
 *	ENOENT is returned if no more element exists.
 *	EBADF is returned if the list has been destroyed.
 */
int
pfc_listiter64_prev(pfc_listiter_t PFC_RESTRICT it,
		    uint64_t *PFC_RESTRICT valuep)
{
	listiter_t	*iter = (listiter_t *)it;
	listmodel_t	*model = iter->lmi_model;
	int	err, cookie;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(model))) {
		return EINVAL;
	}

	/*
	 * Acquire writer lock to serialize access to the list and
	 * this iterator.
	 */
	cookie = pfc_listm_wrlock(model);

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = pfc_listm_iter_prev(iter, model, (pfc_cptr_t *)valuep);
	}
	else {
		pfc_refptr_t	*ref;

		err = pfc_listm_iter_prev(iter, model, (pfc_cptr_t *)&ref);
		if (PFC_EXPECT_TRUE(err == 0) && valuep != NULL) {
			*valuep = pfc_listm64_eval_refptr(ref);
		}
	}

	pfc_listm_unlock(model, cookie);

	return err;
}

#ifdef	PFC_LP64

/*
 * static int
 * pfc_listm64_signed_comp(const void *o1, const void *o2)
 *	Sort comparator for signed 64-bit list.
 */
static int
pfc_listm64_signed_comp(const void *o1, const void *o2)
{
	int64_t	i1 = (int64_t)o1;
	int64_t	i2 = (int64_t)o2;

	if (i1 == i2) {
		return 0;
	}

	return (i1 < i2) ? -1 : 1;
}

#endif	/* PFC_LP64 */
