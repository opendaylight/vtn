/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * vector.c - Variable length array implementation.
 */

#include <string.h>
#include <pfc/list.h>
#include <pfc/listmodel.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include "listmodel_impl.h"

/*
 * Vector instance.
 */
typedef struct {
	listmodel_t		v_model;	/* common list model. */
	pfc_rwlock_t		v_lock;		/* read/write lock */
	pfc_cptr_t		*v_array;	/* current pointer array */
	int			v_capacity;	/* current capacity */
	int			v_capinc;	/* capacity increment */
} vector_t;

/*
 * Convert list instance.
 */
#define	MODEL_TO_VECTOR(model)	PFC_CAST_CONTAINER(model, vector_t, v_model)
#define	VECTOR_TO_MODEL(vec)	(&(vec)->v_model)

/*
 * Macros to verify vector parameters.
 */
#define	PFC_VECTOR_CAPACITY_IS_VALID(capacity)				\
	(PFC_EXPECT_TRUE((uint32_t)(capacity) <=			\
			 (uint32_t)PFC_VECTOR_MAX_CAPACITY))
#define	PFC_VECTOR_CAPINC_IS_VALID(capinc)			\
	(PFC_EXPECT_TRUE((uint32_t)(capinc) <=			\
			 (uint32_t)PFC_VECTOR_MAX_CAPINC))
/*
 * Return array size in bytes.
 */
#define	PFC_VECTOR_ARRAY_SIZE(capacity)	((capacity) * sizeof(pfc_cptr_t))

/*
 * Copy array elements.
 * The source and destination areas must not overlap.
 */
#define	PFC_VECTOR_ARRAY_COPY(dst, src, nelems)			\
	do {							\
		pfc_cptr_t	*__dp = (dst);			\
		pfc_cptr_t	*__sp = (src);			\
		pfc_cptr_t	*__ep = __dp + (nelems);	\
								\
		for (; __dp < __ep; __sp++, __dp++) {		\
			*(__dp) = *(__sp);			\
		}						\
	} while (0)

#ifndef	PFC_LP64

/*
 * Dummy refptr for 64-bit vector, which means zero.
 */
static const uint64_t	vector_zero;

static pfc_refptr_t	vector_zref_i64 = {
	.pr_ops		= &refptr_int64_ops,
	.pr_object	= (pfc_ptr_t)&vector_zero,
	.pr_refcnt	= 1,
};

static pfc_refptr_t	vector_zref_u64 = {
	.pr_ops		= &refptr_uint64_ops,
	.pr_object	= (pfc_ptr_t)&vector_zero,
	.pr_refcnt	= 1,
};

/*
 * Obtain dummy refptr which represents 64-bit zero value.
 */
#define	PFC_VECTOR_ZFILL_REFPTR(model)				\
	(((model)->lm_flags & LISTMF_INT64) ? &vector_zref_i64	\
	 : &vector_zref_u64)

/*
 * True if the given refptr represents 64-bit zero value.
 */
#define	PFC_VECTOR_IS_REF64_ZERO(value)				\
	(pfc_listm64_eval_refptr((pfc_refptr_t *)(value)) == 0)

#endif	/* !PFC_LP64 */

/*
 * Assertion to ensure that dummy zero refptr will not be destroyed.
 */
static inline void
pfc_vector_zfill_assert(listmodel_t *model, pfc_refptr_t *ref)
{
#if	!defined(PFC_LP64) && defined(PFC_VERBOSE_DEBUG)
	if (PFC_LISTM_IS_64BIT(model) && ref != NULL) {
		if (ref == &vector_zref_i64 || ref == &vector_zref_u64) {
			PFC_ASSERT(ref->pr_refcnt > 1);
		}
	}
#endif	/* !defined(PFC_LP64) && defined(PFC_VERBOSE_DEBUG) */
}

/*
 * static inline pfc_bool_t
 * pfc_vector_zfill_pack(listmodel_t *model, pfc_cptr_t *dst, pfc_cptr_t *src)
 *	Copy non-NULL element.
 *	True is returned if non-NULL element is copied.
 */
static inline pfc_bool_t
pfc_vector_zfill_pack(listmodel_t *model, pfc_cptr_t *dst, pfc_cptr_t *src)
{
	pfc_cptr_t	value = *src;

	if (value == NULL) {
		return PFC_FALSE;
	}

#ifndef	PFC_LP64
	if (PFC_LISTM_IS_64BIT(model) && PFC_VECTOR_IS_REF64_ZERO(value)) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)value;

		pfc_refptr_put(ref);

		return PFC_FALSE;
	}
#endif	/* !PFC_LP64 */

	*dst = value;

	return PFC_TRUE;
}

/*
 * static inline pfc_bool_t
 * pfc_vector_zfill_update(listmodel_t *model, pfc_cptr_t *elem,
 *			   pfc_cptr_t value)
 *	Update element if the destination is empty.
 *	True is returned if updated successfully.
 */
static inline pfc_bool_t
pfc_vector_zfill_update(listmodel_t *model, pfc_cptr_t *elem, pfc_cptr_t value)
{
	pfc_cptr_t	old = *elem;

	if (old == NULL) {
		*elem = value;

		return PFC_TRUE;
	}

#ifndef	PFC_LP64
	if (PFC_LISTM_IS_64BIT(model) && PFC_VECTOR_IS_REF64_ZERO(old)) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)old;

		pfc_refptr_put(ref);
		*elem = value;

		return PFC_TRUE;
	}
#endif	/* !PFC_LP64 */

	return PFC_FALSE;
}

/*
 * static inline pfc_refptr_t *
 * pfc_vector_create_ref64(listmodel_t *model, uint64_t value)
 *	Return a refptr object which represents the given 64-bit value.
 */
static inline pfc_refptr_t *
pfc_vector_create_ref64(listmodel_t *model, uint64_t value)
{
#ifdef	PFC_LP64
	return NULL;
#else	/* !PFC_LP64 */
	pfc_refptr_t	*ref;

	if (value == 0) {
		/* Return dummy refptr which represents zero. */
		ref = PFC_VECTOR_ZFILL_REFPTR(model);
		pfc_refptr_get(ref);
	}
	else {
		ref = pfc_listm_create_ref64(model, value);
	}

	return ref;
#endif	/* PFC_LP64 */
}

/*
 * Internal prototypes.
 */
static void	pfc_vector_dtor(listmodel_t *model);
static int	pfc_vector_push(listmodel_t *PFC_RESTRICT model,
			       pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_push_tail(listmodel_t *PFC_RESTRICT model,
				    pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_pop(listmodel_t *PFC_RESTRICT model,
			      pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_pop_tail(listmodel_t *PFC_RESTRICT model,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_first(listmodel_t *PFC_RESTRICT model,
				pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_last(listmodel_t *PFC_RESTRICT model,
			       pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_index(listmodel_t *PFC_RESTRICT model,
				pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_index_tail(listmodel_t *PFC_RESTRICT model,
				     pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_remove(listmodel_t *PFC_RESTRICT model,
				 pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_remove_tail(listmodel_t *PFC_RESTRICT model,
				      pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_getat(listmodel_t *PFC_RESTRICT model, int index,
				pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_insertat(listmodel_t *PFC_RESTRICT model, int index,
				   pfc_cptr_t PFC_RESTRICT value);
static int	pfc_vector_removeat(listmodel_t *PFC_RESTRICT model, int index,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_vector_sort(listmodel_t *PFC_RESTRICT model,
			       listsort_t *PFC_RESTRICT lsp);
static void	pfc_vector_clear(listmodel_t *model);
static void	pfc_vector_iter_ctor(listiter_t *PFC_RESTRICT iter);
static void	pfc_vector_iter_next(listiter_t *PFC_RESTRICT iter,
				    pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_vector_iter_prev(listiter_t *PFC_RESTRICT iter,
				    pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_vector_iter_rewind(listiter_t *PFC_RESTRICT iter);
static void	pfc_vector_iter_eol(listiter_t *PFC_RESTRICT iter);
static void	pfc_vector_iter_remove(listiter_t *iter);

static void	pfc_vector_rdlock(listmodel_t *model);
static void	pfc_vector_wrlock(listmodel_t *model);
static void	pfc_vector_unlock(listmodel_t *model);

static int	pfc_vector_init(pfc_listm_t *listp,
				const pfc_refptr_ops_t *refops,
				int capacity, int capinc, uint8_t flags);

static int	pfc_vector_shift_up(vector_t *vec, int index,
				    pfc_cptr_t value);
static int	pfc_vector_shift_down(vector_t *PFC_RESTRICT vec, int index,
				      pfc_cptr_t *PFC_RESTRICT valuep);
static pfc_cptr_t	pfc_vector_do_shift_down(vector_t *vec, int index,
						 pfc_bool_t retain);
static int	pfc_vector_set(pfc_listm_t PFC_RESTRICT list, int index,
			       pfc_cptr_t PFC_RESTRICT value,
			       pfc_bool_t update);
static int	pfc_vector_array_alloc(vector_t *vec, int capacity, int capinc);
static int	pfc_vector_change_capacity(vector_t *vec, int capacity);
static int	pfc_vector_lookup_first(vector_t *vec, pfc_cptr_t value);
static int	pfc_vector_lookup_last(vector_t *vec, pfc_cptr_t value);
static void	pfc_vector_zfill(listmodel_t *model, pfc_cptr_t *array,
				 int from, int to);

static int	pfc_vector_sort_comp(const void *v1, const void *v2);
static int	pfc_vector_sort_refcomp(const void *v1, const void *v2);

/*
 * List model operations for linked list implementation.
 */
static const listmodel_ops_t	vector_ops = {
	.type		= PFC_LISTM_VECTOR,
	.has_rwlock	= PFC_TRUE,

	.dtor		= pfc_vector_dtor,
	.push		= pfc_vector_push,
	.push_tail	= pfc_vector_push_tail,
	.pop		= pfc_vector_pop,
	.pop_tail	= pfc_vector_pop_tail,
	.first		= pfc_vector_first,
	.last		= pfc_vector_last,
	.index		= pfc_vector_index,
	.index_tail	= pfc_vector_index_tail,
	.remove		= pfc_vector_remove,
	.remove_tail	= pfc_vector_remove_tail,
	.getat		= pfc_vector_getat,
	.insertat	= pfc_vector_insertat,
	.removeat	= pfc_vector_removeat,
	.clear		= pfc_vector_clear,

	.rdlock		= pfc_vector_rdlock,
	.wrlock		= pfc_vector_wrlock,
	.unlock		= pfc_vector_unlock,

	/* Sort operation */
	.sort		= pfc_vector_sort,

	/* Iterator operations */
	.iter_ctor	= pfc_vector_iter_ctor,
	.iter_next	= pfc_vector_iter_next,
	.iter_prev	= pfc_vector_iter_prev,
	.iter_rewind	= pfc_vector_iter_rewind,
	.iter_eol	= pfc_vector_iter_eol,
	.iter_remove	= pfc_vector_iter_remove,
};

/*
 * Optimized lock operation with considering synchronized block.
 */

/*
 * static inline int
 * PFC_VECTOR_RDLOCK(vector_t *vec)
 *	Acquire reader lock of the vector, if needed.
 *	Returned value must be passed to PFC_VECTOR_UNLOCK().
 */
static inline int
PFC_VECTOR_RDLOCK(vector_t *vec)
{
	int	cookie;

	if (pfc_listm_sync_lookup(VECTOR_TO_MODEL(vec)) == NULL) {
		(void)pfc_rwlock_rdlock(&vec->v_lock);
		cookie = 1;
	}
	else {
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline int
 * PFC_VECTOR_WRLOCK(vector_t *vec)
 *	Acquire writer lock of the vector, if needed.
 *	Returned value must be passed to PFC_VECTOR_UNLOCK().
 */
static inline int
PFC_VECTOR_WRLOCK(vector_t *vec)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	int		cookie;
	__pfc_syncblock_t	*sbp;

	/* Check to see whether we're in synchronized block. */
	sbp = pfc_listm_sync_lookup(model);
	if (sbp == NULL) {
		(void)pfc_rwlock_wrlock(&vec->v_lock);
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
 * PFC_VECTOR_UNLOCK(vector_t *vec, int cookie)
 *	Release lock of the vector, if needed.
 */
static inline void
PFC_VECTOR_UNLOCK(vector_t *vec, int cookie)
{
	if (cookie) {
		(void)pfc_rwlock_unlock(&vec->v_lock);
	}
}

/*
 * static inline int
 * pfc_vector_unref_element(listmodel_t *model, pfc_cptr_t value)
 *	Remove reference to the specified list element.
 *
 *	If the list is refptr list, decrement its reference counter.
 */
static inline void
pfc_vector_unref_element(listmodel_t *model, pfc_cptr_t value)
{
	pfc_vector_zfill_assert(model, (pfc_refptr_t *)value);
	pfc_listm_unref_element(model, value);
}

/*
 * static inline void
 * pfc_vector_element_free(listmodel_t *model, pfc_cptr_t *array, int index)
 *	Remove the element at the specified index.
 */
static inline void
pfc_vector_element_free(listmodel_t *model, pfc_cptr_t *array, int index)
{
	pfc_vector_unref_element(model, *(array + index));
	pfc_listm_nelem_dec(model);
}

/*
 * static inline void
 * pfc_vector_element_clear(listmodel_t *model, pfc_cptr_t *array, int from,
 *			    int to)
 *	Remove array elements at the specified range.
 *	Note that this doesn't change model->lm_nelems.
 */
static inline void
pfc_vector_element_clear(listmodel_t *model, pfc_cptr_t *array, int from,
			 int to)
{
	/*
	 * Only thing to do is to unref refptr objects in the vector.
	 * So there is nothing to do if the vector is non-refptr vector.
	 */
	if (pfc_listm_is_refptr_list(model)) {
		pfc_cptr_t	*limit = array + to;

		array += from;
		for (; array < limit; array++) {
			pfc_vector_zfill_assert(model, (pfc_refptr_t *)*array);
			pfc_listm_refptr_put(*array);
		}
	}
}

/*
 * int
 * pfc_vector_create(pfc_listm_t *listp, int capacity, int capinc)
 *	Create a new vector instance.
 *
 *	A list instance created by this function treats list element as
 *	a pointer address. You must need to use pfc_vector_create_ref()
 *	if you want to add refptr object into vector.
 *
 *	This function tries to allocate a pointer array which can contains
 *	`capacity' elements. `capinc' is used as a hint for array expansion.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_vector_create(pfc_listm_t *listp, int capacity, int capinc)
{
	return pfc_vector_init(listp, NULL, capacity, capinc, 0);
}

/*
 * int
 * pfc_vector_create_ref(pfc_listm_t *PFC_RESTRICT listp,
 *			 const pfc_refptr_ops_t *PFC_RESTRICT refops,
 *			 int capacity, int capinc)
 *	Create a new vector which contains refptr objects.
 *
 *	Refptr operation pointer must be specified to `*refops', and all
 *	list elements must have the same operation pointer. Note that NULL
 *	can also be added to the vector.
 *
 *	This function tries to allocate a pointer array which can contains
 *	`capacity' elements. `capinc' is used as a hint for array expansion.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_vector_create_ref(pfc_listm_t *PFC_RESTRICT listp,
		      const pfc_refptr_ops_t *PFC_RESTRICT refops,
		      int capacity, int capinc)
{
	if (refops == NULL) {
		/* Use default refptr operation. */
		refops = &refptr_default_ops;
	}

	return pfc_vector_init(listp, refops, capacity, capinc, 0);
}

/*
 * int
 * pfc_vector_get_capacity(pfc_listm_t list)
 *	Return current capacity of the vector.
 *
 * Calling/Exit State:
 *	Upon successful completion, current capacity is returned.
 *	PFC_LISTIDX_BAD is returned if the specified vector is not valid.
 */
int
pfc_vector_get_capacity(pfc_listm_t list)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	capacity, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return PFC_LISTIDX_BAD;
	}

	cookie = PFC_VECTOR_RDLOCK(vec);
	capacity = vec->v_capacity;
	PFC_VECTOR_UNLOCK(vec, cookie);

	return capacity;
}

/*
 * int
 * pfc_vector_set_capacity(pfc_listm_t list, int capacity)
 *	Set vector's capacity to the specified value.
 *
 *	This function changes capacity without changing number of elements
 *	in the vector. So the specified capacity must be greater or equal
 *	to number of elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_set_capacity(pfc_listm_t list, int capacity)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	/* Verify the specified capacity. */
	if (!PFC_VECTOR_CAPACITY_IS_VALID(capacity)) {
		return EINVAL;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_change_capacity(vec, capacity);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * int
 * pfc_vector_get_capacity_inc(pfc_listm_t list)
 *	Return current capacity increment value of the vector.
 *
 * Calling/Exit State:
 *	Upon successful completion, current capacity increment value is
 *	returned.
 *	PFC_LISTIDX_BAD is returned if the specified vector is not valid.
 */
int
pfc_vector_get_capacity_inc(pfc_listm_t list)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	capinc, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return PFC_LISTIDX_BAD;
	}

	cookie = PFC_VECTOR_RDLOCK(vec);
	capinc = vec->v_capinc;
	PFC_VECTOR_UNLOCK(vec, cookie);

	return capinc;
}

/*
 * int
 * pfc_vector_set_capacity_inc(pfc_listm_t list, int capinc)
 *	Set capacity increment value of the vector.
 *	New value will affects next expansion of array.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_set_capacity_inc(pfc_listm_t list, int capinc)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	/* Verify the specified value. */
	if (!PFC_VECTOR_CAPINC_IS_VALID(capinc)) {
		return EINVAL;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	vec->v_capinc = capinc;

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * int
 * pfc_vector_set_size(pfc_listm_t list, int size)
 *	Set the size of the vector.
 *
 *	If the new size is greater than the current size, NULL elements are
 *	added to the tail of the vector.
 *	If the new size is less than the current size, all elements at the
 *	index `size' and greater are removed from the vector. Note that
 *	this function doesn't change vector's capacity in that case.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_set_size(pfc_listm_t list, int size)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array;
	int	err, nelems, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	/* Verify the specified size can be accepted as capacity. */
	if (!PFC_VECTOR_CAPACITY_IS_VALID(size)) {
		return EINVAL;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	nelems = model->lm_nelems;
	if (size == nelems) {
		/* Nothing to do. */
		goto out;
	}

	array = vec->v_array;
	if (size > nelems) {
		pfc_cptr_t	*newarray;

		/* Expand the vector, and push NULL elements. */
		err = pfc_vector_array_alloc(vec, size, vec->v_capinc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		newarray = vec->v_array;
		if (array != NULL) {
			/* Copy all elements to new array. */
			PFC_VECTOR_ARRAY_COPY(newarray, array, nelems);

			/* Free old array. */
			free(array);
		}

		/* Initialize new elements as NULL. */
		pfc_vector_zfill(model, newarray, nelems, size);
	}
	else {
		/* Shrink the vector size without changing its capacity. */
		pfc_vector_element_clear(model, array, size, nelems);
	}

	model->lm_nelems = size;
	pfc_listm_gen_update(model);

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * int
 * pfc_vector_trim(pfc_listm_t list)
 *	Trim the vector's capacity to the current number of elements.
 *	This function minimizes array storage of the vector.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_trim(pfc_listm_t list)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_change_capacity(vec, model->lm_nelems);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * int
 * pfc_vector_pack(pfc_listm_t list)
 *	Eliminate all NULL elements from the vector.
 *	Note that this function doesn't trim capacity of the vector.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_pack(pfc_listm_t list)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array, *p, *newp;
	int	err, nelems, count, cookie;

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	nelems = model->lm_nelems;
	if (nelems == 0) {
		/* Nothing to do. */
		goto out;
	}

	/* Allocate a new array. */
	array = (pfc_cptr_t *)malloc(PFC_VECTOR_ARRAY_SIZE(vec->v_capacity));
	if (PFC_EXPECT_FALSE(array == NULL)) {
		err = ENOMEM;
		goto out;
	}

	/* Copy non-NULL elements. */
	newp = array;
	for (p = vec->v_array; p < vec->v_array + nelems; p++) {
		if (pfc_vector_zfill_pack(model, newp, p)) {
			newp++;
		}
	}

	count = newp - array;
	if (nelems == count) {
		/* No need to replace array. */
		free(array);
	}
	else {
		/* Install packed array. */
		free(vec->v_array);
		vec->v_array = array;
		model->lm_nelems = count;
		pfc_listm_gen_update(model);
	}

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * int
 * pfc_vector_setat(pfc_listm_t PFC_RESTRICT list, int index,
 *		    pfc_cptr_t PFC_RESTRICT value)
 *	Set the element at the specified index.
 *	This function fails if a non-NULL element exists at the specified
 *	index.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_setat(pfc_listm_t PFC_RESTRICT list, int index,
		 pfc_cptr_t PFC_RESTRICT value)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}
	if (index < 0) {
		return ENOENT;
	}

	return pfc_vector_set(list, index, value, PFC_FALSE);
}

/*
 * int
 * pfc_vector_updateat(pfc_listm_t PFC_RESTRICT list, int index,
 *		       pfc_cptr_t PFC_RESTRICT value)
 *	Update the element at the specified index.
 *	If a non-NULL element exists at the specified index, it will be
 *	removed from the vector.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector_updateat(pfc_listm_t PFC_RESTRICT list, int index,
		    pfc_cptr_t PFC_RESTRICT value)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}
	if (index < 0) {
		return ENOENT;
	}

	return pfc_vector_set(list, index, value, PFC_TRUE);
}

/*
 * int
 * pfc_vector_create_i64(pfc_listm_t *listp, int capacity, int capinc)
 *	Create a new vector which contains signed 64-bit integer.
 *
 *	This function tries to allocate a pointer array which can contains
 *	`capacity' elements. `capinc' is used as a hint for array expansion.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_vector_create_i64(pfc_listm_t *listp, int capacity, int capinc)
{
	const uint8_t	flags = LISTMF_INT64;

	return pfc_vector_init(listp, pfc_listm_refops64(flags), capacity,
			       capinc, flags);
}

/*
 * int
 * pfc_vector_create_u64(pfc_listm_t *listp, int capacity, int capinc)
 *	Create a new vector which contains unsigned 64-bit integer.
 *
 *	A list instance created by this function treats list element as
 *	a pointer address. You must need to use pfc_vector_create_ref()
 *	if you want to add refptr object into vector.
 *
 *	This function tries to allocate a pointer array which can contains
 *	`capacity' elements. `capinc' is used as a hint for array expansion.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_vector_create_u64(pfc_listm_t *listp, int capacity, int capinc)
{
	const uint8_t	flags = LISTMF_UINT64;

	return pfc_vector_init(listp, pfc_listm_refops64(flags), capacity,
			       capinc, flags);
}

/*
 * int
 * pfc_vector64_setat(pfc_listm_t list, int index, uint64_t value)
 *	64-bit vector version of pfc_vector_setat().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector64_setat(pfc_listm_t list, int index, uint64_t value)
{
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}
	if (index < 0) {
		return ENOENT;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = pfc_vector_set(list, index, (pfc_cptr_t)(uintptr_t)value,
				     PFC_FALSE);
	}
	else {
		listmodel_t	*model = (listmodel_t *)list;
		pfc_refptr_t	*ref = pfc_vector_create_ref64(model, value);
		pfc_cptr_t	rvalue = (pfc_cptr_t)ref;

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}
		err = pfc_vector_set(list, index, rvalue, PFC_FALSE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * int
 * pfc_vector64_updateat(pfc_listm_t list, int index, uint64_t value)
 *	64-bit vector version of pfc_vector_updateat().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_vector64_updateat(pfc_listm_t list, int index, uint64_t value)
{
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}
	if (index < 0) {
		return ENOENT;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		/* LP64 system can keep 64-bit value as pointer. */
		err = pfc_vector_set(list, index, (pfc_cptr_t)(uintptr_t)value,
				     PFC_TRUE);
	}
	else {
		listmodel_t	*model = (listmodel_t *)list;
		pfc_refptr_t	*ref = pfc_vector_create_ref64(model, value);
		pfc_cptr_t	rvalue = (pfc_cptr_t)ref;

		if (PFC_EXPECT_FALSE(ref == NULL)) {
			return ENOMEM;
		}
		err = pfc_vector_set(list, index, rvalue, PFC_TRUE);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * static void
 * pfc_vector_dtor(listmodel_t *model)
 *	Destructor of the vector.
 */
static void
pfc_vector_dtor(listmodel_t *model)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);

#ifdef	PFC_VERBOSE_DEBUG
	{
		int	err;

		err = pfc_rwlock_destroy(&vec->v_lock);
		PFC_ASSERT(err == 0);
	}
#endif	/* PFC_VERBOSE_DEBUG */

	free(vec->v_array);
	free(vec);
}

/*
 * static int
 * pfc_vector_push(listmodel_t *PFC_RESTRICT model,
 *		   pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the head of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_vector_push(listmodel_t *PFC_RESTRICT model, pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_shift_up(vec, 0, value);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_push_tail(listmodel_t *PFC_RESTRICT model,
 *			pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the tail of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_vector_push_tail(listmodel_t *PFC_RESTRICT model,
		     pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array;
	int	nelems, err = 0, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	array = vec->v_array;
	nelems = model->lm_nelems;
	if (nelems >= vec->v_capacity) {
		int		newnelems = nelems + 1;
		pfc_cptr_t	*oldarray = array;

		/* Array must be expanded. */
		err = pfc_vector_array_alloc(vec, newnelems, vec->v_capinc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		array = vec->v_array;
		PFC_ASSERT(newnelems <= vec->v_capacity);

		if (oldarray != NULL) {
			/* Copy existing elements. */
			PFC_VECTOR_ARRAY_COPY(array, oldarray, nelems);

			/* Free old array. */
			free(oldarray);
		}
	}

	/* Install new element. */
	*(array + nelems) = value;
	pfc_listm_nelem_inc(model);

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_pop(listmodel_t *PFC_RESTRICT model,
 *		  pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the first element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_vector_pop(listmodel_t *PFC_RESTRICT model, pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_shift_down(vec, 0, valuep);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_pop_tail(listmodel_t *PFC_RESTRICT model,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the last element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep', and zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_vector_pop_tail(listmodel_t *PFC_RESTRICT model,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array, *tail;
	int	err, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);

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

	/* Remove the element at the tail. */
	array = vec->v_array;
	tail = array + (model->lm_nelems - 1);
	if (valuep != NULL) {
		*valuep = *tail;
	}

	/*
	 * If the vector is 64-bit vector on the 32-bit system, we must not
	 * decrement refptr's reference counter. It will be done by upper
	 * layer.
	 */
	if (!PFC_IS_LP64_SYSTEM() && PFC_LISTM_IS_64BIT(model)) {
		*tail = NULL;
	}
	pfc_vector_element_free(model, tail, 0);

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_first(listmodel_t *PFC_RESTRICT model,
 *		    pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_vector_first(listmodel_t *PFC_RESTRICT model,
		 pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*target;

	PFC_ASSERT(model->lm_nelems > 0);

	target = vec->v_array;
	if (valuep != NULL) {
		*valuep = *target;
	}

	return 0;
}

/*
 * static int
 * pfc_vector_last(listmodel_t *PFC_RESTRICT model,
 *		   pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_vector_last(listmodel_t *PFC_RESTRICT model,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*target;

	PFC_ASSERT(model->lm_nelems > 0);

	target = vec->v_array + (model->lm_nelems - 1);
	if (valuep != NULL) {
		*valuep = *target;
	}

	return 0;
}

/*
 * static int
 * pfc_vector_index(listmodel_t *PFC_RESTRICT model,
 *		    pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the first occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
static int
pfc_vector_index(listmodel_t *PFC_RESTRICT model,
		 pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	index, cookie;

	cookie = PFC_VECTOR_RDLOCK(vec);
	index = pfc_vector_lookup_first(vec, value);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return index;
}

/*
 * static int
 * pfc_vector_index_tail(listmodel_t *PFC_RESTRICT model,
 *			 pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the last occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
static int
pfc_vector_index_tail(listmodel_t *PFC_RESTRICT model,
		      pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	index, cookie;

	cookie = PFC_VECTOR_RDLOCK(vec);
	index = pfc_vector_lookup_last(vec, value);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return index;
}

/*
 * static int
 * pfc_vector_remove(listmodel_t *PFC_RESTRICT model,
 *		     pfc_cptr_t PFC_RESTRICT value)
 *	Remove the first occurrence of the specified value in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed from the list.
 *	ENOENT is returned if not present.
 *	EBADF is returned if the list has been already destroyed.
 */
static int
pfc_vector_remove(listmodel_t *PFC_RESTRICT model,
		  pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err = 0, index, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);

	index = pfc_vector_lookup_first(vec, value);
	if (PFC_EXPECT_FALSE(index == PFC_LISTIDX_BAD)) {
		err = EBADF;
		goto out;
	}
	else if (PFC_EXPECT_FALSE(index == PFC_LISTIDX_NOENT)) {
		err = ENOENT;
		goto out;
	}

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	/* Remove the element. */
	(void)pfc_vector_do_shift_down(vec, index, PFC_FALSE);

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_remove_tail(listmodel_t *PFC_RESTRICT model,
 *			  pfc_cptr_t PFC_RESTRICT value)
 *	Remove the last occurrence of the specified value in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed from the list.
 *	ENOENT is returned if not present.
 *	EBADF is returned if the list has been already destroyed.
 */
static int
pfc_vector_remove_tail(listmodel_t *PFC_RESTRICT model,
		       pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err = 0, index, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);

	index = pfc_vector_lookup_last(vec, value);
	if (PFC_EXPECT_FALSE(index == PFC_LISTIDX_BAD)) {
		err = EBADF;
		goto out;
	}
	else if (PFC_EXPECT_FALSE(index == PFC_LISTIDX_NOENT)) {
		err = ENOENT;
		goto out;
	}

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	/* Remove the element. */
	(void)pfc_vector_do_shift_down(vec, index, PFC_FALSE);

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_getat(listmodel_t *PFC_RESTRICT model, int index,
 *		    pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_vector_getat(listmodel_t *PFC_RESTRICT model, int index,
		 pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array;

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	array = vec->v_array + index;
	if (valuep != NULL) {
		*valuep = *array;
	}

	return 0;
}

/*
 * static int
 * pfc_vector_insertat(listmodel_t *PFC_RESTRICT model, int index,
 *		       pfc_cptr_t PFC_RESTRICT value)
 *	Insert the list element at the specified index.
 *	Each element in the list with index greater or equal to the specified
 *	index is shifted upward.
 *
 * Calling/Exit State:
 *	Zero is returned if found.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_vector_insertat(listmodel_t *PFC_RESTRICT model, int index,
		    pfc_cptr_t PFC_RESTRICT value)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_shift_up(vec, index, value);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_removeat(listmodel_t *PFC_RESTRICT model, int index,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove the list element at the specified index.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list element at the specified index
 *	is removed, and then zero is returned. Removed value is set to
 *	`*valuep' if `valuep' is not NULL.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_vector_removeat(listmodel_t *PFC_RESTRICT model, int index,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	err, cookie;

	cookie = PFC_VECTOR_WRLOCK(vec);
	err = pfc_vector_shift_down(vec, index, valuep);
	PFC_VECTOR_UNLOCK(vec, cookie);

	return err;
}

/*
 * static int
 * pfc_vector_sort(listmodel_t *PFC_RESTRICT model,
 *		   listsort_t *PFC_RESTRICT lsp)
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
pfc_vector_sort(listmodel_t *PFC_RESTRICT model, listsort_t *PFC_RESTRICT lsp)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	nelems = model->lm_nelems;

	PFC_ASSERT(nelems > 0);

	/* Do sort. */
	if (lsp->lms_refcomp) {
		qsort(vec->v_array, nelems, sizeof(pfc_ptr_t),
		      pfc_vector_sort_refcomp);
	}
	else {
		qsort(vec->v_array, nelems, sizeof(pfc_ptr_t),
		      pfc_vector_sort_comp);
	}

	return 0;
}

/*
 * static void
 * pfc_vector_clear(listmodel_t *model)
 *	Remove all elements from the list.
 *	This method is called with holding writer lock of the list.
 */
static void
pfc_vector_clear(listmodel_t *model)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);

	pfc_vector_element_clear(model, vec->v_array, 0, model->lm_nelems);
}

/*
 * static void
 * pfc_vector_iter_ctor(listiter_t *PFC_RESTRICT iter)
 *	Initialize the iterator to iterate linked list.
 */
static void
pfc_vector_iter_ctor(listiter_t *PFC_RESTRICT iter)
{
	/*
	 * Vector can iterate elements using index. So there is nothing
	 * to do here.
	 */
}

/*
 * static void
 * pfc_vector_iter_next(listiter_t *PFC_RESTRICT iter,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the next element in the list iterator, and move iterator's cursor
 *	forward.
 */
static void
pfc_vector_iter_next(listiter_t *PFC_RESTRICT iter,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = iter->lmi_model;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array = vec->v_array + iter->lmi_nextidx;

	if (valuep != NULL) {
		*valuep = *array;
	}
}

/*
 * static void
 * pfc_vector_iter_prev(listiter_t *PFC_RESTRICT iter,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the previous element in the list iterator, and move iterator's
 *	cursor backward.
 */
static void
pfc_vector_iter_prev(listiter_t *PFC_RESTRICT iter,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = iter->lmi_model;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array = vec->v_array + (iter->lmi_nextidx - 1);

	if (valuep != NULL) {
		*valuep = *array;
	}
}

/*
 * static void
 * pfc_vector_iter_rewind(listiter_t *PFC_RESTRICT iter)
 *	Rewind iterator's cursor.
 *	A subsequent call to pfc_listiter_next() will return the first element
 *	in the list.
 */
static void
pfc_vector_iter_rewind(listiter_t *PFC_RESTRICT iter)
{
	/* Nothing to do here. */
}

/*
 * static void
 * pfc_vector_iter_eol(listiter_t *PFC_RESTRICT iter)
 *	Move iterator's cursor to the end of the list.
 *	A subsequent call to pfc_listiter_prev() will return the last element
 *	in the list.
 */
static void
pfc_vector_iter_eol(listiter_t *PFC_RESTRICT iter)
{
	/* Nothing to do here. */
}

/*
 * static void
 * pfc_vector_iter_remove(listiter_t *iter)
 *	Remove the element retrieved by the previous pfc_listiter_next() or
 *	pfc_listiter_prev() call.
 *
 * Remarks:
 *	This function is called with holding writer lock of the list.
 */
static void
pfc_vector_iter_remove(listiter_t *iter)
{
	listmodel_t	*model = iter->lmi_model;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	int	index = iter->lmi_elemidx;

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	/* Remove this element, and shift downward trailing elements. */
	(void)pfc_vector_do_shift_down(vec, index, PFC_FALSE);
}

/*
 * static void
 * pfc_vector_rdlock(listmodel_t *model)
 *	Acquire reader lock of the vector.
 */
static void
pfc_vector_rdlock(listmodel_t *model)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);

	(void)pfc_rwlock_rdlock(&vec->v_lock);
}

/*
 * static void
 * pfc_vector_wrlock(listmodel_t *model)
 *	Acquire writer lock of the vector.
 */
static void
pfc_vector_wrlock(listmodel_t *model)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);

	(void)pfc_rwlock_wrlock(&vec->v_lock);
}

/*
 * static void
 * pfc_vector_unlock(listmodel_t *model)
 *	Release lock of the vector.
 */
static void
pfc_vector_unlock(listmodel_t *model)
{
	vector_t	*vec = MODEL_TO_VECTOR(model);

	(void)pfc_rwlock_unlock(&vec->v_lock);
}

/*
 * static int
 * pfc_vector_init(pfc_listm_t *listp, const pfc_refptr_ops_t *refops,
 *		   int capacity, int capinc, uint8_t flags)
 *	Common routine to create a new vector.
 */
static int
pfc_vector_init(pfc_listm_t *listp, const pfc_refptr_ops_t *refops,
		int capacity, int capinc, uint8_t flags)
{
	int	err;
	vector_t	*vec;
	listmodel_t	*model;
	pfc_cptr_t	*array;

	/* Verify parameters. */
	if (!PFC_VECTOR_CAPACITY_IS_VALID(capacity) ||
	    !PFC_VECTOR_CAPINC_IS_VALID(capinc)) {
		return EINVAL;
	}

	/* Allocate a new vector instance. */
	vec = (vector_t *)malloc(sizeof(*vec));
	if (PFC_EXPECT_FALSE(vec == NULL)) {
		return ENOMEM;
	}
	model = VECTOR_TO_MODEL(vec);

	err = pfc_rwlock_init(&vec->v_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	/* Allocate initial array of the specified capacity. */
	if (capacity == 0) {
		array = NULL;
	}
	else {
		size_t	sz = PFC_VECTOR_ARRAY_SIZE(capacity);

		array = (pfc_cptr_t *)malloc(sz);
		if (PFC_EXPECT_FALSE(array == NULL)) {
			err = ENOMEM;
			goto error;
		}
	}

	pfc_listm_init(model, &vector_ops, refops, flags);
	vec->v_array = array;
	vec->v_capacity = capacity;
	vec->v_capinc = capinc;

	*listp = (pfc_listm_t)model;

	return 0;

error:
	free(vec);

	return err;
}

/*
 * static int
 * pfc_vector_shift_up(vector_t *vec, int index, pfc_cptr_t value)
 *	Shift upward all elements which have greater index than the specified
 *	index, and install the specified value to the specified index.
 *	Number of elements in the list is incremented.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the vector.
 */
static int
pfc_vector_shift_up(vector_t *vec, int index, pfc_cptr_t value)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	pfc_cptr_t	*array, *oldarray, *src, *dst;
	int		err, nelems = model->lm_nelems;
	int	capacity = vec->v_capacity;

	PFC_ASSERT(index >= 0);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		return err;
	}

	if (PFC_EXPECT_FALSE(index > nelems)) {
		/* Index out of range. */
		return ENOENT;
	}

	oldarray = vec->v_array;
	if (nelems >= capacity) {
		int	newnelems = nelems + 1;

		/* Array must be expanded. */
		err = pfc_vector_array_alloc(vec, newnelems, vec->v_capinc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		array = vec->v_array;
		PFC_ASSERT(newnelems <= vec->v_capacity);

		if (oldarray != NULL) {
			/* Copy leading elements. */
			for (src = oldarray, dst = array;
			     dst < array + index; src++, dst++) {
				*dst = *src;
			}
		}
		else {
			PFC_ASSERT(index == 0 && nelems == 0);
			goto out;
		}
	}
	else {
		PFC_ASSERT(oldarray != NULL);
		array = oldarray;
	}

	/* Shift upward trailing elements. */
	PFC_ASSERT(nelems + 1 <= vec->v_capacity);
	for (src = oldarray + nelems - 1, dst = array + nelems;
	     dst > array + index; src--, dst--) {
		*dst = *src;
	}

	if (array != oldarray) {
		free(oldarray);
	}

out:
	/* Install new element. */
	*(array + index) = value;
	pfc_listm_nelem_inc(model);

	return 0;
}

/*
 * static int
 * pfc_vector_shift_down(vector_t *PFC_RESTRICT vec, int index,
 *			 pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove the element at the specified index, and shift downward all
 *	elements which have greater index than the specified index.
 *	Number of elements in the list is decremented.
 *
 * Calling/Exit State:
 *	Upon successful completion, the element at the specified index is
 *	set to `*valuep' if `valuep' is not NULL, and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the vector.
 */
static int
pfc_vector_shift_down(vector_t *PFC_RESTRICT vec, int index,
		      pfc_cptr_t *PFC_RESTRICT valuep)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	pfc_cptr_t	value;
	int		err, nelems = model->lm_nelems;
	pfc_bool_t	retain;

	PFC_ASSERT(index >= 0);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		return err;
	}

	if (PFC_EXPECT_FALSE(index >= nelems)) {
		/* Index out of range. */
		return ENOENT;
	}

	/*
	 * Remove the element at the specified index.
	 * If the vector is 64-bit vector on the 32-bit system, we must not
	 * decrement refptr's reference counter. It will be done by upper
	 * layer.
	 */
	retain = (!PFC_IS_LP64_SYSTEM() && PFC_LISTM_IS_64BIT(model));
	value = pfc_vector_do_shift_down(vec, index, retain);
	if (valuep != NULL) {
		*valuep = value;
	}

	return 0;
}

/*
 * static pfc_cptr_t
 * pfc_vector_do_shift_down(vector_t *vec, int index, pfc_bool_t retain)
 *	Remove the element at the specified index, and shift downward all
 *	elements which have greater index than the specified index.
 *	Number of elements in the list is decremented.
 *
 *	If `retain' is true, refptr's reference counter is not decremented.
 *
 * Calling/Exit State:
 *	The element at the specified index is returned.
 *	Sanity check must be done by the caller.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the vector.
 */
static pfc_cptr_t
pfc_vector_do_shift_down(vector_t *vec, int index, pfc_bool_t retain)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	pfc_cptr_t	*array = vec->v_array, *ep, *next, *target, value;
	int		nelems = model->lm_nelems;

	/* Remove the element at the specified index. */
	target = array + index;
	value = *target;
	if (retain) {
		/* Clear target element not to decrement reference counter. */
		*target = NULL;
	}
	pfc_vector_element_free(model, target, 0);

	/* Shift downward trailing elements. */
	ep = array + nelems;
	for (next = target + 1; next < ep; target++, next++) {
		*target = *next;
	}

	return value;
}

/*
 * static int
 * pfc_vector_set(pfc_listm_t PFC_RESTRICT list, int index,
 *		  pfc_cptr_t PFC_RESTRICT value, pfc_bool_t update)
 *	Set the element at the specified index.
 *	If a non-NULL element exists at the specified index, EEXIST is returned
 *	if `update' is true. Otherwise, current element is removed and new
 *	value is installed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_vector_set(pfc_listm_t PFC_RESTRICT list, int index,
	       pfc_cptr_t PFC_RESTRICT value, pfc_bool_t update)
{
	listmodel_t	*model = (listmodel_t *)list;
	vector_t	*vec = MODEL_TO_VECTOR(model);
	pfc_cptr_t	*array, cur;
	int	err, cookie;

	PFC_ASSERT(index >= 0);

	/* Ensure the list is vector. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &vector_ops)) {
		return EINVAL;
	}

	err = pfc_listm_verify_element(model, value);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	cookie = PFC_VECTOR_WRLOCK(vec);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto out;
	}

	if (PFC_EXPECT_FALSE(index >= model->lm_nelems)) {
		err = ENOENT;
		goto out;
	}

	array = vec->v_array + index;
	cur = *array;
	if (pfc_vector_zfill_update(model, array, value)) {
		pfc_listm_gen_update(model);
	}
	else if (update) {
		/* Remove current element, and install new value. */
		pfc_vector_unref_element(model, cur);
		*array = value;
		pfc_listm_gen_update(model);
	}
	else {
		err = EEXIST;
	}

out:
	PFC_VECTOR_UNLOCK(vec, cookie);

	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_vector_unref_element(model, value);
	}

	return err;
}

/*
 * static int
 * pfc_vector_array_alloc(vector_t *vec, int capacity, int capinc)
 *	Allocate array buffer which has at least the specified capacity.
 *	This function updates vec->v_array and vec->v_capacity on success.
 *	If you need to copy old array elements to new array, you must preserve
 *	them before calling.
 *
 * Calling/Exit State:
 *	Upon successful completion, update array pointer and capacity in
 *	`*vec', and zero is returned.
 *	ENOMEM is returned on failure.
 *
 * Remarks:
 *	- This function must be called with holding writer lock of the vector.
 *	- This function doesn't initialize array elements.
 */
static int
pfc_vector_array_alloc(vector_t *vec, int capacity, int capinc)
{
	pfc_cptr_t	*array;
	int		newcap;
	size_t		sz;

	/* At first, try to allocate array with capacity increment. */
	newcap = capacity + capinc;
	sz = PFC_VECTOR_ARRAY_SIZE(newcap);
	array = (pfc_cptr_t *)malloc(sz);
	if (PFC_EXPECT_TRUE(array != NULL)) {
		goto out;
	}

	if (capinc == 0) {
		return ENOMEM;
	}

	/* Try again without capacity increment. */
	newcap = capacity;
	sz = PFC_VECTOR_ARRAY_SIZE(newcap);
	array = (pfc_cptr_t *)malloc(sz);
	if (PFC_EXPECT_FALSE(array == NULL)) {
		return ENOMEM;
	}

out:
	vec->v_array = array;
	vec->v_capacity = newcap;

	return 0;
}

/*
 * static int
 * pfc_vector_change_capacity(vector_t *vec, int capacity)
 *	Change vector's capacity without changing array elements.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the vector.
 */
static int
pfc_vector_change_capacity(vector_t *vec, int capacity)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	pfc_cptr_t	*oldarray;
	int	err, nelems;

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		return err;
	}

	if (capacity == vec->v_capacity) {
		/* Nothing to do. */
		return 0;
	}

	nelems = model->lm_nelems;
	if (capacity < nelems) {
		/* This function must not change elements in the vector. */
		return EEXIST;
	}

	oldarray = vec->v_array;
	if (capacity == 0) {
		/* We can free array. */
		vec->v_array = NULL;
		vec->v_capacity = 0;
	}
	else {
		/* Allocate a new array. */
		err = pfc_vector_array_alloc(vec, capacity, 0);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		/* Copy elements. */
		PFC_ASSERT(oldarray != NULL);
		PFC_VECTOR_ARRAY_COPY(vec->v_array, oldarray, nelems);
	}
	free(oldarray);

	/*
	 * We don't need to update generation number of the vector.
	 * Vector's iterator can iterate elements even if the vector array
	 * is replaced, as long as elements are not changed.
	 */
	return 0;
}

/*
 * static int
 * pfc_vector_lookup_first(vector_t *vec, pfc_cptr_t value)
 *	Return the index of the first occurrence of the specified value
 *	in the vector.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the vector.
 */
static int
pfc_vector_lookup_first(vector_t *vec, pfc_cptr_t value)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	pfc_cptr_t	*array, *ep;
	int	index;

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		/* This list model is invalid. */
		return PFC_LISTIDX_BAD;
	}

	array = vec->v_array;
	ep = array + model->lm_nelems;
	index = PFC_LISTIDX_NOENT;

	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		for (; array < ep; array++) {
			if (pfc_listm_refptr_equals(refops, *array, obj)) {
				/* Found. */
				index = array - vec->v_array;
				break;
			}
		}
	}
	else {
		for (; array < ep; array++) {
			if (*array == value) {
				/* Found. */
				index = array - vec->v_array;
				break;
			}
		}
	}

	return index;
}

/*
 * static int
 * pfc_vector_lookup_last(vector_t *vec, pfc_cptr_t value)
 *	Return the index of the last occurrence of the specified value
 *	in the vector.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the vector.
 */
static int
pfc_vector_lookup_last(vector_t *vec, pfc_cptr_t value)
{
	listmodel_t	*model = VECTOR_TO_MODEL(vec);
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	pfc_cptr_t	*array, *ep;
	int	index;

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		/* This list model is invalid. */
		return PFC_LISTIDX_BAD;
	}

	ep = vec->v_array;
	array = ep + (model->lm_nelems - 1);
	index = PFC_LISTIDX_NOENT;

	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		for (; array >= ep; array--) {
			if (pfc_listm_refptr_equals(refops, *array, obj)) {
				/* Found. */
				index = array - ep;
				break;
			}
		}
	}
	else {
		for (; array >= ep; array--) {
			if (*array == value) {
				/* Found. */
				index = array - ep;
				break;
			}
		}
	}

	return index;
}

/*
 * static void
 * pfc_vector_zfill(listmodel_t *model, pfc_cptr_t *array, int from, int to)
 *	Fill the given array with zero.
 */
static void
pfc_vector_zfill(listmodel_t *model, pfc_cptr_t *array, int from, int to)
{
	pfc_cptr_t	zero, *p, *ep;

#ifdef	PFC_LP64
	zero = NULL;
#else	/* !PFC_LP64 */
	if (PFC_LISTM_IS_64BIT(model)) {
		pfc_refptr_t	*ref;
		int	size = to - from;

		/*
		 * NULL element is not allowed for 64-bit list on
		 * 32-bit system. Set dummy refptr object instead.
		 */
		ref = PFC_VECTOR_ZFILL_REFPTR(model);

		pfc_atomic_add_uint32(&ref->pr_refcnt, size);
		PFC_ASSERT(ref->pr_refcnt >= (uint32_t)size);

		zero = (pfc_cptr_t)ref;
	}
	else {
		zero = NULL;
	}
#endif	/* PFC_LP64 */

	p = array + from;
	ep = array + to;
	for (; p < ep; p++) {
		*p = zero;
	}
}

/*
 * static int
 * pfc_vector_sort_comp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 */
static int
pfc_vector_sort_comp(const void *v1, const void *v2)
{
	const void	*o1 = *((const void **)v1);
	const void	*o2 = *((const void **)v2);
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return lsp->lms_comp(o1, o2);
}

/*
 * static int
 * pfc_vector_sort_refcomp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 *	This comparator is used if the list is refptr list.
 */
static int
pfc_vector_sort_refcomp(const void *v1, const void *v2)
{
	const void	*o1 = *((const void **)v1);
	const void	*o2 = *((const void **)v2);
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return pfc_listm_refptr_comparator(lsp, o1, o2);
}
