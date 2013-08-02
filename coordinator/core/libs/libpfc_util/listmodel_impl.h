/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_LISTMODEL_IMPL_H
#define	_PFC_LIBPFC_UTIL_LISTMODEL_IMPL_H

/*
 * Internal definitions for list model.
 */

#include <pfc/listmodel.h>
#include <pfc/list.h>
#include <pfc/atomic.h>
#include <pfc/refptr.h>
#include "refptr_impl.h"
#include "syncblock_impl.h"

PFC_C_BEGIN_DECL

struct listmodel;
typedef struct listmodel	listmodel_t;

/*
 * Common iterator instance.
 */
typedef struct {
	listmodel_t	*lmi_model;	/* list instance */
	pfc_list_t	lmi_list;	/* iterator list */
	int		lmi_from;	/* from index (inclusive) */
	int		lmi_to;		/* to index (exclusive) */
	int		lmi_nextidx;	/* next index */
	int		lmi_elemidx;	/* index of lmi_element */
	pfc_ptr_t	lmi_element;	/* element previously retrieved */
	pfc_ptr_t	lmi_next;	/* next element */
	uint32_t	lmi_gen;	/* generation number */
} listiter_t;

/*
 * Sort context.
 */
typedef struct {
	pfc_listcomp_t		lms_comp;	/* comparator */
	refptr_comp_t		lms_refcomp;	/* comparator for refptr */
} listsort_t;

/*
 * List model operation.
 */
typedef struct {
	/*
	 * Type of list implementation.
	 */
	const pfc_listm_type_t	type;

	/*
	 * Determine read/write lock is supported or not.
	 */
	const pfc_bool_t	has_rwlock;

	/*
	 * List destructor.
	 */
	void	(*dtor)(listmodel_t *model);

	/*
	 * Push the specified value onto the head of the list.
	 */
	int	(*push)(listmodel_t *PFC_RESTRICT model,
			pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Push the specified value onto the tail of the list.
	 */
	int	(*push_tail)(listmodel_t *PFC_RESTRICT model,
			     pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Pop the first element from the list.
	 */
	int	(*pop)(listmodel_t *PFC_RESTRICT model,
		       pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Pop the last element from the list.
	 */
	int	(*pop_tail)(listmodel_t *PFC_RESTRICT model,
			    pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Get the first element from the list without removing.
	 * This method will be called with holding reader lock of the list.
	 */
	int	(*first)(listmodel_t *PFC_RESTRICT model,
			 pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Get the last element from the list without removing.
	 * This method will be called with holding reader lock of the list.
	 */
	int	(*last)(listmodel_t *PFC_RESTRICT model,
			pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Return the index of the first occurrence of the specified value in
	 * the list.
	 */
	int	(*index)(listmodel_t *PFC_RESTRICT model,
			 pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Return the index of the last occurrence of the specified value in
	 * the list.
	 */
	int	(*index_tail)(listmodel_t *PFC_RESTRICT model,
			      pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Remove the first occurrence of the specified value in the list.
	 */
	int	(*remove)(listmodel_t *PFC_RESTRICT model,
			  pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Remove the last occurrence of the specified value in the list.
	 */
	int	(*remove_tail)(listmodel_t *PFC_RESTRICT model,
			       pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Get the list element at the specified index.
	 * This method will be called with holding reader lock of the list.
	 */
	int	(*getat)(listmodel_t *PFC_RESTRICT model, int index,
			 pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Insert the list element at the specified index.
	 */
	int	(*insertat)(listmodel_t *PFC_RESTRICT model, int index,
			    pfc_cptr_t PFC_RESTRICT value);

	/*
	 * Remove the list element at the specified index.
	 */
	int	(*removeat)(listmodel_t *PFC_RESTRICT model, int index,
			    pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Remove all elements in the list.
	 * This method will be called with holding writer lock of the list.
	 */
	void	(*clear)(listmodel_t *model);

	/*
	 * Acquire reader lock of the list.
	 */
	void	(*rdlock)(listmodel_t *model);

	/*
	 * Acquire writer lock of the list.
	 */
	void	(*wrlock)(listmodel_t *model);

	/*
	 * Release lock of the list.
	 */
	void	(*unlock)(listmodel_t *model);

	/*
	 * Sort the list using specified comparator.
	 * This method is called with holding writer lock of the list.
	 */
	int	(*sort)(listmodel_t *PFC_RESTRICT model,
			listsort_t *PFC_RESTRICT lsp);

	/*
	 * Initialize the list iterator.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_ctor)(listiter_t *PFC_RESTRICT iter);

	/*
	 * Destroy the specified iterator.
	 * This method is called with holding writer lock of the list.
	 * NULL can be set if no iterator's destructor is needed.
	 */
	void	(*iter_dtor)(listiter_t *iter);

	/*
	 * Return the next element in the list iterator.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_next)(listiter_t *PFC_RESTRICT iter,
			     pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Return the previous element in the list iterator.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_prev)(listiter_t *PFC_RESTRICT iter,
			     pfc_cptr_t *PFC_RESTRICT valuep);

	/*
	 * Rewind iterator's cursor.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_rewind)(listiter_t *PFC_RESTRICT iter);

	/*
	 * Move iterator's cursor to the end of the list.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_eol)(listiter_t *PFC_RESTRICT iter);

	/*
	 * Remove element retrieved by the previous next or prev operation.
	 * This method is called with holding writer lock of the list.
	 */
	void	(*iter_remove)(listiter_t *PFC_RESTRICT iter);
} listmodel_ops_t;

/*
 * Common list instance.
 */
struct listmodel {
	const listmodel_ops_t	*lm_ops;	/* list operations */
	const pfc_refptr_ops_t	*lm_refops;	/* refptr ops for elements */
	pfc_list_t		lm_iterator;	/* iterator list */
	int			lm_nelems;	/* number of elements */
	uint32_t		lm_refcnt;	/* reference counter */
	uint32_t		lm_gen;		/* generation number */
	volatile uint8_t	lm_valid;	/* list is valid if true */
	uint8_t			lm_flags;	/* flags */
};

/*
 * Internal list model flags.
 */
#define	LISTMF_INT64		0x01U		/* signed 64-bit list */
#define	LISTMF_UINT64		0x02U		/* unsigned 64-bit list */

/*
 * Determine whether the list contains 64-bit integer or not.
 */
#define	PFC_LISTM_IS_64BIT(model)				\
	(((listmodel_t *)(model))->lm_flags & (LISTMF_INT64 | LISTMF_UINT64))

/*
 * Synchronized block frame created by pfc_listm_wsync_begin() or
 * pfc_listm_rsync_begin().
 */
extern __thread __pfc_syncblock_t	*listm_sync_frame;

/*
 * Current sort context.
 */
extern __thread listsort_t		*listm_sort_context;

/*
 * Initialize abstract list model instance.
 */
static inline void
pfc_listm_init(listmodel_t *model, const listmodel_ops_t *ops,
	       const pfc_refptr_ops_t *refops, uint8_t flags)
{
	model->lm_ops = ops;
	model->lm_refops = refops;
	model->lm_nelems = 0;
	model->lm_refcnt = 1;
	model->lm_gen = 0;
	model->lm_valid = PFC_TRUE;
	model->lm_flags = flags;
	pfc_list_init(&model->lm_iterator);
}

/*
 * static inline __pfc_syncblock_t *
 * pfc_listm_sync_lookup(listmodel_t *model)
 *	Search for the list synchronized block on the current context for
 *	the specified list.
 *
 * Calling/Exit State:
 *	If found, a pointer to synchronized block frame is returned.
 *	NULL is returned if not found.
 */
static inline __pfc_syncblock_t *
pfc_listm_sync_lookup(listmodel_t *model)
{
	return pfc_syncblock_lookup(listm_sync_frame, (pfc_cptr_t)model);
}

/*
 * static inline int
 * pfc_listm_rdlock(listmodel_t *model)
 *	Acquire read lock of the specified list.
 *	Returned value must be passed to pfc_listm_unlock().
 */
static inline int
pfc_listm_rdlock(listmodel_t *model)
{
	int	cookie;

	if (pfc_listm_sync_lookup(model) == NULL) {
		model->lm_ops->rdlock(model);
		cookie = 1;
	}
	else {
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline int
 * pfc_listm_wrlock(listmodel_t *model)
 *	Acquire write lock of the specified list.
 *	Returned value must be passed to pfc_listm_unlock().
 */
static inline int
pfc_listm_wrlock(listmodel_t *model)
{
	__pfc_syncblock_t	*sbp;
	int	cookie;

	sbp = pfc_listm_sync_lookup(model);
	if (sbp == NULL) {
		model->lm_ops->wrlock(model);
		cookie = 1;
	}
	else {
		/* Write mode is required. */
		PFC_ASSERT(PFC_SYNCBLOCK_IS_WRITE(sbp));
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline void
 * pfc_listm_unlock(listmodel_t *model, int cookie)
 *	Release lock of the specified list.
 */
static inline void
pfc_listm_unlock(listmodel_t *model, int cookie)
{
	if (cookie) {
		model->lm_ops->unlock(model);
	}
}

/*
 * static inline void
 * pfc_listm_ref(listmodel_t *model)
 *	Increment reference counter of the list.
 */
static inline void
pfc_listm_ref(listmodel_t *model)
{
	PFC_ASSERT(model->lm_refcnt != 0);
	pfc_atomic_inc_uint32(&model->lm_refcnt);
}

/*
 * static inline void
 * pfc_listm_unref(listmodel_t *model)
 *	Decrement reference counter of the list.
 *	The list model is destroyed if the counter becomes zero.
 */
static inline void
pfc_listm_unref(listmodel_t *model)
{
	PFC_ASSERT(model->lm_refcnt != 0);
	if (pfc_atomic_dec_uint32_old(&model->lm_refcnt) == 1) {
		void	(*dtor)(listmodel_t *model) = model->lm_ops->dtor;

		(*dtor)(model);
	}
}

/*
 * static inline void
 * pfc_listm_gen_update(listmodel_t *model)
 *	Update generation number of the list.
 *	The caller must call this function with holding write lock of the list.
 */
static inline void
pfc_listm_gen_update(listmodel_t *model)
{
	model->lm_gen++;
}

/*
 * static inline int
 * pfc_listm_verify(listmodel_t *model)
 *	Verify the list model.
 */
static inline int
pfc_listm_verify(listmodel_t *model)
{
	if (PFC_EXPECT_FALSE(!model->lm_valid)) {
		/* The list has been destroyed. */
		return EBADF;
	}

	return 0;
}

/*
 * static pfc_bool_t
 * pfc_listm_is_refptr_list(listmodel_t *model)
 *	Return true if the specified list is refptr list.
 */
static pfc_bool_t
pfc_listm_is_refptr_list(listmodel_t *model)
{
	return (model->lm_refops != NULL) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static inline int
 * pfc_listm_verify_element(listmodel_t *model, pfc_cptr_t value)
 *	Verify that the list can contain the specified value.
 *
 *	If the list is refptr list, ensure that the value has the same
 *	refptr operation, and increment its reference counter.
 */
static inline int
pfc_listm_verify_element(listmodel_t *model, pfc_cptr_t value)
{
	const pfc_refptr_ops_t	*refops = model->lm_refops;

	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)value;

		if (refops != pfc_refptr_operation(ref)) {
			return EINVAL;
		}
		pfc_refptr_get(ref);
	}

	return 0;
}

/*
 * static inline void
 * pfc_listm_refptr_put(pfc_cptr_t value)
 *	Decrement reference counter of the list element.
 *	The specified argument must be a NULL, or refptr object.
 */
static inline void
pfc_listm_refptr_put(pfc_cptr_t value)
{
	if (value != NULL) {
		pfc_refptr_t	*ref = (pfc_refptr_t *)value;

		pfc_refptr_put(ref);
	}
}

/*
 * static inline int
 * pfc_listm_unref_element(listmodel_t *model, pfc_cptr_t value)
 *	Remove reference to the specified list element.
 *
 *	If the list is refptr list, decrement its reference counter.
 */
static inline int
pfc_listm_unref_element(listmodel_t *model, pfc_cptr_t value)
{
	if (pfc_listm_is_refptr_list(model)) {
		pfc_listm_refptr_put(value);
	}

	return 0;
}

/*
 * static inline void
 * pfc_listm_nelem_inc(listmodel_t *model)
 *	Increment number of elements in the list.
 *	The caller must call this function with holding write lock of the list.
 */
static inline void
pfc_listm_nelem_inc(listmodel_t *model)
{
	model->lm_nelems++;
	PFC_ASSERT(model->lm_nelems > 0);
	pfc_listm_gen_update(model);
}

/*
 * static inline void
 * pfc_listm_nelem_dec(listmodel_t *model)
 *	Decrement number of elements in the list.
 *	The caller must call this function with holding write lock of the list.
 */
static inline void
pfc_listm_nelem_dec(listmodel_t *model)
{
	PFC_ASSERT(model->lm_nelems > 0);
	model->lm_nelems--;
	pfc_listm_gen_update(model);
}

/*
 * static inline pfc_bool_t
 * pfc_listm_refptr_equals(const pfc_refptr_ops_t *refops, pfc_cptr_t elem,
 *			   pfc_cptr_t value)
 *	Determine whether the list element specified by `elem' contains
 *	a refptr object whose value is identical to `value'.
 *
 *	The caller must ensure that `value' is not NULL.
 */
static inline pfc_bool_t
pfc_listm_refptr_equals(const pfc_refptr_ops_t *PFC_RESTRICT refops,
			pfc_cptr_t elem, pfc_cptr_t value)
{
	pfc_refptr_t	*eref = (pfc_refptr_t *)elem;
	pfc_cptr_t	ev;

	if (PFC_EXPECT_FALSE(eref == NULL)) {
		return PFC_FALSE;
	}

	ev = PFC_REFPTR_VALUE(eref, pfc_cptr_t);

	return pfc_refptr_ops_equals(refops, ev, value);
}

/*
 * static inline const pfc_refptr_ops_t *
 * pfc_listm_refops64(uint8_t flags)
 *	Return suitable refops pointer for 64-bit integer list.
 */
static inline const pfc_refptr_ops_t *
pfc_listm_refops64(uint8_t flags)
{
#ifndef	PFC_LP64
	if (flags == LISTMF_INT64) {
		/* Signed 64-bit list. */
		return &refptr_int64_ops;
	}
	if (flags == LISTMF_UINT64) {
		/* Unsigned 64-bit list. */
		return &refptr_uint64_ops;
	}
#endif	/* !PFC_LP64 */

	return NULL;
}

/*
 * static inline pfc_refptr_t *
 * pfc_listm_create_ref64(listmodel_t *model, uint64_t value)
 *	Return a refptr object which represents the given 64-bit value.
 */
static inline pfc_refptr_t *
pfc_listm_create_ref64(listmodel_t *model, uint64_t value)
{
#ifdef	PFC_LP64
	return NULL;
#else	/* !PFC_LP64 */
	if (model->lm_flags & LISTMF_INT64) {
		return pfc_refptr_int64_create((int64_t)value);
	}

	PFC_ASSERT(model->lm_flags & LISTMF_UINT64);

	return pfc_refptr_uint64_create(value);
#endif	/* PFC_LP64 */
}

/*
 * static inline uint64_t
 * pfc_listm64_eval_refptr(pfc_refptr_t *ref)
 *	Return value in 64-bit list.
 *	This is valid only on 32-bit system.
 */
static inline uint64_t
pfc_listm64_eval_refptr(pfc_refptr_t *ref)
{
#ifdef	PFC_LP64
	return 0;
#else	/* !PFC_LP64 */
	PFC_ASSERT(ref != NULL);

	return pfc_refptr_uint64_value(ref);
#endif	/* PFC_LP64 */
}

/*
 * static inline void
 * pfc_listm64_unref_refptr(pfc_refptr_t *ref, uint64_t *valuep)
 *	Decrement reference counter of refptr in 64-bit list.
 *	Its value is stored to `*valuep'.
 *	This is valid only on 32-bit system.
 */
static inline void
pfc_listm64_unref_refptr(pfc_refptr_t *ref, uint64_t *valuep)
{
#ifndef	PFC_LP64
	PFC_ASSERT(ref != NULL);
	if (valuep != NULL) {
		*valuep = pfc_refptr_uint64_value(ref);
	}
	pfc_refptr_put(ref);
#endif	/* !PFC_LP64 */
}

/*
 * Prototypes.
 */
extern int	pfc_listm_refptr_comparator(listsort_t *lsp, const void *v1,
					    const void *v2);

PFC_C_END_DECL

#endif	/* !_PFC_LIBPFC_UTIL_LISTMODEL_IMPL_H */
