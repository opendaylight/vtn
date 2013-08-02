/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * linkedlist.c - Linked list implementation which can contain abstract object.
 */

#include <pfc/list.h>
#include <pfc/listmodel.h>
#include <pfc/synch.h>
#include <pfc/atomic.h>
#include <pfc/debug.h>
#include "listmodel_impl.h"

/*
 * List element.
 */
typedef struct {
	pfc_list_t	le_list;		/* element link */
	pfc_cptr_t	le_value;		/* value of element */
} llist_elem_t;

#define	LIST2ELEM(list)		PFC_CAST_CONTAINER(list, llist_elem_t, le_list)

/*
 * Linked list instance.
 */
typedef struct {
	listmodel_t		ll_model;	/* common list model. */
	pfc_list_t		ll_head;	/* list head */
	pfc_mutex_t		ll_lock;	/* list mutex */
	pfc_cond_t		ll_cond;	/* condition variable */
	volatile uint32_t	ll_nwaiters;	/* number of waiter threads */
	volatile uint32_t	ll_flags;	/* flags */
} llist_t;

/*
 * Flags for ll_flags.
 */
#define	LLF_SHUTDOWN		PFC_CONST_U(0x1)	/* shutdown */

/*
 * Convert list instance.
 */
#define	MODEL_TO_LLIST(model)	PFC_CAST_CONTAINER(model, llist_t, ll_model)
#define	LLIST_TO_MODEL(headp)	(&(headp)->ll_model)

/*
 * Internal prototypes.
 */
static void	pfc_llist_dtor(listmodel_t *model);
static int	pfc_llist_push(listmodel_t *PFC_RESTRICT model,
			       pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_push_tail(listmodel_t *PFC_RESTRICT model,
				    pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_pop(listmodel_t *PFC_RESTRICT model,
			      pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_pop_tail(listmodel_t *PFC_RESTRICT model,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_first(listmodel_t *PFC_RESTRICT model,
				pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_last(listmodel_t *PFC_RESTRICT model,
			       pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_index(listmodel_t *PFC_RESTRICT model,
				pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_index_tail(listmodel_t *PFC_RESTRICT model,
				     pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_remove(listmodel_t *PFC_RESTRICT model,
				 pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_remove_tail(listmodel_t *PFC_RESTRICT model,
				      pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_getat(listmodel_t *PFC_RESTRICT model, int index,
				pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_insertat(listmodel_t *PFC_RESTRICT model, int index,
				   pfc_cptr_t PFC_RESTRICT value);
static int	pfc_llist_removeat(listmodel_t *PFC_RESTRICT model, int index,
				   pfc_cptr_t *PFC_RESTRICT valuep);
static int	pfc_llist_sort(listmodel_t *PFC_RESTRICT model,
			       listsort_t *PFC_RESTRICT lsp);
static void	pfc_llist_clear(listmodel_t *model);
static void	pfc_llist_iter_ctor(listiter_t *PFC_RESTRICT iter);
static void	pfc_llist_iter_next(listiter_t *PFC_RESTRICT iter,
				    pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_llist_iter_prev(listiter_t *PFC_RESTRICT iter,
				    pfc_cptr_t *PFC_RESTRICT valuep);
static void	pfc_llist_iter_rewind(listiter_t *PFC_RESTRICT iter);
static void	pfc_llist_iter_eol(listiter_t *PFC_RESTRICT iter);
static void	pfc_llist_iter_remove(listiter_t *iter);

static void	pfc_llist_lock(listmodel_t *model);
static void	pfc_llist_unlock(listmodel_t *model);

static int	pfc_llist_init(pfc_listm_t *listp,
			       const pfc_refptr_ops_t *refops, uint8_t flags);
static int	pfc_llist_push_element(listmodel_t *PFC_RESTRICT model,
				       pfc_cptr_t PFC_RESTRICT value,
				       pfc_bool_t tail);
static int	pfc_llist_pop_element(listmodel_t *PFC_RESTRICT model,
				      pfc_cptr_t *PFC_RESTRICT valuep,
				      pfc_bool_t tail);
static int	pfc_llist_get_edge(listmodel_t *PFC_RESTRICT model,
				   pfc_cptr_t *PFC_RESTRICT valuep,
				   pfc_bool_t tail);
static llist_elem_t	*pfc_llist_lookup_by_index(llist_t *headp, int index);
static int	pfc_llist_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
					   pfc_cptr_t *PFC_RESTRICT valuep,
					   const pfc_timespec_t
					   *PFC_RESTRICT timeout,
					   pfc_bool_t tail);
static int	pfc_llist64_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
					     uint64_t *PFC_RESTRICT valuep,
					     const pfc_timespec_t
					     *PFC_RESTRICT timeout,
					     pfc_bool_t tail);
static int	pfc_llist_sort_comp(const void *v1, const void *v2);
static int	pfc_llist_sort_refcomp(const void *v1, const void *v2);

/*
 * List model operations for linked list implementation.
 */
static const listmodel_ops_t	llist_ops = {
	.type		= PFC_LISTM_LINKED,

	/* Linked list uses mutex, not rwlock. */
	.has_rwlock	= PFC_FALSE,

	.dtor		= pfc_llist_dtor,
	.push		= pfc_llist_push,
	.push_tail	= pfc_llist_push_tail,
	.pop		= pfc_llist_pop,
	.pop_tail	= pfc_llist_pop_tail,
	.first		= pfc_llist_first,
	.last		= pfc_llist_last,
	.index		= pfc_llist_index,
	.index_tail	= pfc_llist_index_tail,
	.remove		= pfc_llist_remove,
	.remove_tail	= pfc_llist_remove_tail,
	.getat		= pfc_llist_getat,
	.insertat	= pfc_llist_insertat,
	.removeat	= pfc_llist_removeat,
	.clear		= pfc_llist_clear,

	/* Set the same mutex lock function for reader/write lock function. */
	.rdlock		= pfc_llist_lock,
	.wrlock		= pfc_llist_lock,
	.unlock		= pfc_llist_unlock,

	/* Sort operation */
	.sort		= pfc_llist_sort,

	/* Iterator operations */
	.iter_ctor	= pfc_llist_iter_ctor,
	.iter_next	= pfc_llist_iter_next,
	.iter_prev	= pfc_llist_iter_prev,
	.iter_rewind	= pfc_llist_iter_rewind,
	.iter_eol	= pfc_llist_iter_eol,
	.iter_remove	= pfc_llist_iter_remove,
};

/*
 * static inline int
 * PFC_LLIST_LOCK(llist_t *headp)
 *	Acquire the mutex of the list.
 *	Returned value must be passed to PFC_LLIST_UNLOCK().
 */
static inline int
PFC_LLIST_LOCK(llist_t *headp)
{
	int	cookie;

	if (pfc_listm_sync_lookup(LLIST_TO_MODEL(headp)) == NULL) {
		(void)pfc_mutex_lock(&headp->ll_lock);
		cookie = 1;
	}
	else {
		cookie = 0;
	}

	return cookie;
}

/*
 * static inline void
 * PFC_LLIST_UNLOCK(llist_t *headp, int cookie)
 *	Release the mutex of the list.
 */
static inline void
PFC_LLIST_UNLOCK(llist_t *headp, int cookie)
{
	if (cookie) {
		(void)pfc_mutex_unlock(&headp->ll_lock);
	}
}

/*
 * static inline int
 * pfc_llist_timedlock(llist_t *PFC_RESTRICT headp,
 *		       const pfc_timespec_t *PFC_RESTRICT abstime,
 *		       int *cookiep)
 *	Try to acquire list mutex until the given absolute system time.
 *	This function updates `*cookiep'. Its value must be passed to
 *	PFC_LLIST_UNLOCK().
 */
static inline int
pfc_llist_timedlock(llist_t *PFC_RESTRICT headp,
		    const pfc_timespec_t *PFC_RESTRICT abstime,
		    int *cookiep)
{
	if (pfc_listm_sync_lookup(LLIST_TO_MODEL(headp)) != NULL) {
		*cookiep = 0;

		return 0;
	}
	*cookiep = 1;

	return pfc_mutex_timedlock_abs(&headp->ll_lock, abstime);
}

/*
 * static inline llist_elem_t *
 * pfc_llist_element_alloc(pfc_cptr_t value)
 *	Create a new list element which keeps the given value.
 */
static inline llist_elem_t *
pfc_llist_element_alloc(pfc_cptr_t value)
{
	llist_elem_t	*elem;

	elem = (llist_elem_t *)malloc(sizeof(*elem));
	if (PFC_EXPECT_FALSE(elem != NULL)) {
		elem->le_value = value;
	}

	return elem;
}

/*
 * static inline void
 * pfc_llist_element_free(llist_t *headp, llist_elem_t *elem, pfc_bool_t refptr)
 *	Free a list element.
 *
 *	If `refptr' is true, and the specified element contains refptr object,
 *	its reference counter is decremented.
 */
static inline void
pfc_llist_element_free(llist_t *headp, llist_elem_t *elem, pfc_bool_t refptr)
{
	listmodel_t	*model = LLIST_TO_MODEL(headp);

	if (refptr) {
		pfc_listm_refptr_put(elem->le_value);
	}
	pfc_listm_nelem_dec(model);

	free(elem);
}

/*
 * static inline void
 * pfc_llist_wakeup(llist_t *headp)
 *	Wake up one thread which is waiting for a new element.
 *	The caller must call this function with holding list mutex.
 */
static inline void
pfc_llist_wakeup(llist_t *headp)
{
	if (headp->ll_nwaiters) {
		pfc_cond_signal(&headp->ll_cond);
	}
}

/*
 * static inline int
 * pfc_llist_wait(llist_t *PFC_RESTRICT headp,
 *		  const pfc_timespec_t *PFC_RESTRICT abstime)
 *	Block the current thread until at least one list element is added
 *	to the list.
 *	If `abstime' is NULL, this function never returns unless a list
 *	element is added. If not NULL, this function waits until the specified
 *	system absolute time.
 *
 *	The caller must call this function with holding list mutex.
 */
static inline int
pfc_llist_wait(llist_t *PFC_RESTRICT headp,
	       const pfc_timespec_t *PFC_RESTRICT abstime)
{
	listmodel_t	*model = LLIST_TO_MODEL(headp);
	int	err = 0;

	if (model->lm_nelems == 0) {
		do {
			if (headp->ll_flags & LLF_SHUTDOWN) {
				err = ESHUTDOWN;
				break;
			}
			headp->ll_nwaiters++;
			err = pfc_cond_timedwait_abs(&headp->ll_cond,
						     &headp->ll_lock, abstime);
			headp->ll_nwaiters--;

			PFC_MEMORY_RELOAD();
			if (model->lm_nelems != 0) {
				err = 0;
				break;
			}
		} while (err == 0);
	}

	return err;
}

/*
 * int
 * pfc_llist_create(pfc_listm_t *listp)
 *	Create a new linked list instance.
 *
 *	A list instance created by this function treats list element as
 *	a pointer address. You must need to use pfc_llist_create_ref()
 *	if you want to add refptr object into linked list.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_llist_create(pfc_listm_t *listp)
{
	return pfc_llist_init(listp, NULL, 0);
}

/*
 * int
 * pfc_llist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
 *			const pfc_refptr_ops_t *PFC_RESTRICT refops)
 *	Create a new linked list which contains refptr objects.
 *
 *	Refptr operation pointer must be specified to `*refops', and all
 *	list elements must have the same operation pointer.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_llist_create_ref(pfc_listm_t *PFC_RESTRICT listp,
		     const pfc_refptr_ops_t *PFC_RESTRICT refops)
{
	if (refops == NULL) {
		/* Use default refptr operation. */
		refops = &refptr_default_ops;
	}

	return pfc_llist_init(listp, refops, 0);
}

/*
 * int
 * __pfc_llist_pop_wait(pfc_listm_t PFC_RESTRICT list,
 *			pfc_cptr_t *PFC_RESTRICT valuep);
 *	Pop the first element in the list.
 *	If no element in the list, block the current thread until this thread
 *	can pop an element.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_llist_pop_wait(pfc_listm_t PFC_RESTRICT list,
		     pfc_cptr_t *PFC_RESTRICT valuep)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}

	return pfc_llist_pop_element_wait(list, valuep, NULL, PFC_FALSE);
}

/*
 * int
 * __pfc_llist_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
 *			     pfc_cptr_t *PFC_RESTRICT valuep,
 *			     const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Pop the first element in the list.
 *	If no element in the list, block the current thread within the
 *	specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	The following is the common error case.
 *
 *	ETIMEDOUT:	The current thread couldn't pop an element within
 *			the specified timeout period.
 *	EINTR:		The current thread was interrupted by signal.
 */
int
__pfc_llist_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
			  pfc_cptr_t *PFC_RESTRICT valuep,
			  const pfc_timespec_t *PFC_RESTRICT timeout)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}

	return pfc_llist_pop_element_wait(list, valuep, timeout, PFC_FALSE);
}

/*
 * int
 * pfc_llist_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
 *			   pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the last element in the list.
 *	If no element in the list, block the current thread until this thread
 *	can pop an element.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
__pfc_llist_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
			  pfc_cptr_t *PFC_RESTRICT valuep)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}

	return pfc_llist_pop_element_wait(list, valuep, NULL, PFC_TRUE);
}

/*
 * int
 * __pfc_llist_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
 *				  pfc_cptr_t *PFC_RESTRICT valuep,
 *				  const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Pop the last element in the list.
 *	If no element in the list, block the current thread within the
 *	specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	The following is the common error case.
 *
 *	ETIMEDOUT:	The current thread couldn't pop an element within
 *			the specified timeout period.
 *	EINTR:		The current thread was interrupted by signal.
 */
int
__pfc_llist_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
			       pfc_cptr_t *PFC_RESTRICT valuep,
			       const pfc_timespec_t *PFC_RESTRICT timeout)
{
	if (PFC_EXPECT_FALSE(PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}

	return pfc_llist_pop_element_wait(list, valuep, timeout, PFC_TRUE);
}

/*
 * void
 * pfc_llist_shutdown(pfc_listm_t list, pfc_bool_t clear)
 *	Shut down the specified linked list.
 *	If `clear' is PFC_TRUE, all elements in the list are removed.
 *
 *	After the call of pfc_llist_shutdown(), any operation to put a
 *	new element to the list, such as pfc_listm_push(), returns ESHUTDOWN.
 *	Any operation to pop an element with synchronization, such as
 *	pfc_llist_pop_wait(), returns ESHUTDOWN if the list is empty.
 */
void
pfc_llist_shutdown(pfc_listm_t list, pfc_bool_t clear)
{
	listmodel_t	*model = (listmodel_t *)list;
	llist_t		*headp = MODEL_TO_LLIST(model);
	int	cookie;

	/* Ensure the list is linked list. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &llist_ops)) {
		/* Do nothing. */
		return;
	}

	cookie = PFC_LLIST_LOCK(headp);

	if (clear && model->lm_nelems != 0) {
		/* Clear all elements in the list. */
		pfc_llist_clear(model);
		model->lm_nelems = 0;
		pfc_listm_gen_update(model);
	}

	/* Set shutdown flag. */
	headp->ll_flags |= LLF_SHUTDOWN;

	/* Wake one waiter up, and let it wake other waiters. */
	pfc_llist_wakeup(headp);

	PFC_LLIST_UNLOCK(headp, cookie);
}

/*
 * int
 * pfc_llist64_pop_wait(pfc_listm_t PFC_RESTRICT list,
 *			uint64_t *PFC_RESTRICT valuep);
 *	64-bit list version of pfc_llist_pop_wait().
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_llist64_pop_wait(pfc_listm_t PFC_RESTRICT list,
		     uint64_t *PFC_RESTRICT valuep)
{
	return pfc_llist64_pop_element_wait(list, valuep, NULL, PFC_FALSE);
}

/*
 * int
 * pfc_llist64_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
 *			     uint64_t *PFC_RESTRICT valuep,
 *			     const pfc_timespec_t *PFC_RESTRICT timeout)
 *	64-bit list version of pfc_llist_pop_timedwait().
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	The following is the common error case.
 *
 *	ETIMEDOUT:	The current thread couldn't pop an element within
 *			the specified timeout period.
 *	EINTR:		The current thread was interrupted by signal.
 */
int
pfc_llist64_pop_timedwait(pfc_listm_t PFC_RESTRICT list,
			  uint64_t *PFC_RESTRICT valuep,
			  const pfc_timespec_t *PFC_RESTRICT timeout)
{
	return pfc_llist64_pop_element_wait(list, valuep, timeout, PFC_FALSE);
}

/*
 * int
 * pfc_llist64_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
 *			     uint64_t *PFC_RESTRICT valuep)
 *	64-bit list version of pfc_llist_pop_tail_wait().
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_llist64_pop_tail_wait(pfc_listm_t PFC_RESTRICT list,
			  uint64_t *PFC_RESTRICT valuep)
{
	return pfc_llist64_pop_element_wait(list, valuep, NULL, PFC_TRUE);
}

/*
 * int
 * pfc_llist64_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
 *				  uint64_t *PFC_RESTRICT valuep,
 *				  const pfc_timespec_t *PFC_RESTRICT timeout)
 *	Pop the last element in the list.
 *	If no element in the list, block the current thread within the
 *	specified timeout period.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep' if `valuep' is not NULL, and
 *	zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	The following is the common error case.
 *
 *	ETIMEDOUT:	The current thread couldn't pop an element within
 *			the specified timeout period.
 *	EINTR:		The current thread was interrupted by signal.
 */
int
pfc_llist64_pop_tail_timedwait(pfc_listm_t PFC_RESTRICT list,
			       uint64_t *PFC_RESTRICT valuep,
			       const pfc_timespec_t *PFC_RESTRICT timeout)
{
	return pfc_llist64_pop_element_wait(list, valuep, timeout, PFC_TRUE);
}

/*
 * int
 * pfc_llist_create_i64(pfc_listm_t *listp)
 *	Create a new linked list which contains signed 64-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_llist_create_i64(pfc_listm_t *listp)
{
	const uint8_t	flags = LISTMF_INT64;

	return pfc_llist_init(listp, pfc_listm_refops64(flags), flags);
}

/*
 * int
 * pfc_llist_create_u64(pfc_listm_t *listp)
 *	Create a new linked list which contains unsigned 64-bit integer.
 *
 * Calling/Exit State:
 *	Upon successful completion, list identifier is set in `*listp',
 *	and zero is returned. Otherwise error number which indicates the cause
 *	of error is returned.
 */
int
pfc_llist_create_u64(pfc_listm_t *listp)
{
	const uint8_t	flags = LISTMF_UINT64;

	return pfc_llist_init(listp, pfc_listm_refops64(flags), flags);
}

/*
 * static void
 * pfc_llist_dtor(listmodel_t *model)
 *	Destructor of the linked list.
 */
static void
pfc_llist_dtor(listmodel_t *model)
{
	llist_t	*headp = MODEL_TO_LLIST(model);

#ifdef	PFC_VERBOSE_DEBUG
	(void)pfc_mutex_destroy(&headp->ll_lock);
#endif	/* PFC_VERBOSE_DEBUG */

	free(headp);
}

/*
 * static int
 * pfc_llist_push(listmodel_t *PFC_RESTRICT model,
 *		  pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the head of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_llist_push(listmodel_t *PFC_RESTRICT model, pfc_cptr_t PFC_RESTRICT value)
{
	return pfc_llist_push_element(model, value, PFC_FALSE);
}

/*
 * static int
 * pfc_llist_push_tail(listmodel_t *PFC_RESTRICT model,
 *		       pfc_cptr_t PFC_RESTRICT value)
 *	Push a list element onto the tail of the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_llist_push_tail(listmodel_t *PFC_RESTRICT model,
		    pfc_cptr_t PFC_RESTRICT value)
{
	return pfc_llist_push_element(model, value, PFC_TRUE);
}

/*
 * static int
 * pfc_llist_pop(listmodel_t *PFC_RESTRICT model,
 *		 pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the first element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the first element in the list is removed,
 *	and then zero is returned. Removed value is set to `*valuep' if
 *	`valuep' is not NULL.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_llist_pop(listmodel_t *PFC_RESTRICT model, pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_llist_pop_element(model, valuep, PFC_FALSE);
}

/*
 * static int
 * pfc_llist_pop_tail(listmodel_t *PFC_RESTRICT model,
 *		      pfc_cptr_t *PFC_RESTRICT valuep)
 *	Pop the last element in the list.
 *
 * Calling/Exit State:
 *	Upon successful completion, the last element in the list is removed,
 *	and its value is set to `*valuep', and zero is returned.
 *	ENOENT is returned if no element exists in the list.
 */
static int
pfc_llist_pop_tail(listmodel_t *PFC_RESTRICT model,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_llist_pop_element(model, valuep, PFC_TRUE);
}

/*
 * static int
 * pfc_llist_first(listmodel_t *PFC_RESTRICT model,
 *		   pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_llist_first(listmodel_t *PFC_RESTRICT model,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_llist_get_edge(model, valuep, PFC_FALSE);
}

/*
 * static int
 * pfc_llist_last(listmodel_t *PFC_RESTRICT model,
 *		  pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_llist_last(listmodel_t *PFC_RESTRICT model,
	       pfc_cptr_t *PFC_RESTRICT valuep)
{
	return pfc_llist_get_edge(model, valuep, PFC_TRUE);
}

/*
 * static int
 * pfc_llist_index(listmodel_t *PFC_RESTRICT model,
 *		   pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the first occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
static int
pfc_llist_index(listmodel_t *PFC_RESTRICT model,
		pfc_cptr_t PFC_RESTRICT value)
{
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;
	int	index, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		index = PFC_LISTIDX_BAD;
		goto out;
	}

	index = 0;
	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		PFC_LIST_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (pfc_listm_refptr_equals(refops, elem->le_value,
						    obj)) {
				/* Found. */
				goto out;
			}
			index++;
		}
	}
	else {
		PFC_LIST_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (elem->le_value == value) {
				/* Found. */
				goto out;
			}
			index++;
		}
	}

	/* Not found. */
	index = PFC_LISTIDX_NOENT;

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return index;
}

/*
 * static int
 * pfc_llist_index_tail(listmodel_t *PFC_RESTRICT model,
 *			pfc_cptr_t PFC_RESTRICT value)
 *	Return the index of the last occurrence of the specified value in
 *	the list.
 *
 * Calling/Exit State:
 *	Index value, starts from zero, is returned if found.
 *	PFC_LISTIDX_NOENT is returned if not found.
 *	PFC_LISTIDX_BAD is returned if the list has been destroyed.
 */
static int
pfc_llist_index_tail(listmodel_t *PFC_RESTRICT model,
		     pfc_cptr_t PFC_RESTRICT value)
{
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;
	int	index, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	if (PFC_EXPECT_FALSE(pfc_listm_verify(model) != 0)) {
		index = PFC_LISTIDX_BAD;
		goto out;
	}

	index = model->lm_nelems;

	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		PFC_LIST_REV_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			index--;
			if (pfc_listm_refptr_equals(refops, elem->le_value,
						    obj)) {
				/* Found. */
				goto out;
			}
		}
	}
	else {
		PFC_LIST_REV_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			index--;
			if (elem->le_value == value) {
				/* Found. */
				goto out;
			}
		}
	}

	/* Not found. */
	index = PFC_LISTIDX_NOENT;

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return index;
}

/*
 * static int
 * pfc_llist_remove(listmodel_t *PFC_RESTRICT model,
 *		    pfc_cptr_t PFC_RESTRICT value)
 *	Remove the first occurrence of the specified value in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed from the list.
 *	ENOENT is returned if not present.
 *	EBADF is returned if the list has been already destroyed.
 */
static int
pfc_llist_remove(listmodel_t *PFC_RESTRICT model,
		 pfc_cptr_t PFC_RESTRICT value)
{
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;
	int	err, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	err = ENOENT;
	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		PFC_LIST_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (pfc_listm_refptr_equals(refops, elem->le_value,
						    obj)) {
				/* Remove this element from the list. */
				pfc_list_remove(lelem);
				pfc_llist_element_free(headp, elem, PFC_TRUE);
				err = 0;
				break;
			}
		}
	}
	else {
		PFC_LIST_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (elem->le_value == value) {
				/* Remove this element from the list. */
				pfc_list_remove(lelem);
				pfc_llist_element_free(headp, elem, PFC_FALSE);
				err = 0;
				break;
			}
		}
	}

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return err;
}

/*
 * static int
 * pfc_llist_remove_tail(listmodel_t *PFC_RESTRICT model,
 *			 pfc_cptr_t PFC_RESTRICT value)
 *	Remove the last occurrence of the specified value in the list.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified element is removed from the list.
 *	ENOENT is returned if not present.
 *	EBADF is returned if the list has been already destroyed.
 */
static int
pfc_llist_remove_tail(listmodel_t *PFC_RESTRICT model,
		      pfc_cptr_t PFC_RESTRICT value)
{
	const pfc_refptr_ops_t	*const refops = model->lm_refops;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;
	int		err, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	err = ENOENT;
	if (refops != NULL && value != NULL) {
		pfc_refptr_t	*rv = (pfc_refptr_t *)value;
		pfc_cptr_t	obj = pfc_refptr_value(rv);

		PFC_LIST_REV_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (pfc_listm_refptr_equals(refops, elem->le_value,
						    obj)) {
				/* Remove this element from the list. */
				pfc_list_remove(lelem);
				pfc_llist_element_free(headp, elem, PFC_TRUE);
				err = 0;
				break;
			}
		}
	}
	else {
		PFC_LIST_REV_FOREACH(&headp->ll_head, lelem) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			if (elem->le_value == value) {
				/* Remove this element from the list. */
				pfc_list_remove(lelem);
				pfc_llist_element_free(headp, elem, PFC_FALSE);
				err = 0;
				break;
			}
		}
	}

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return err;
}

/*
 * static int
 * pfc_llist_getat(listmodel_t *PFC_RESTRICT model, int index,
 *		   pfc_cptr_t *PFC_RESTRICT valuep)
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
pfc_llist_getat(listmodel_t *PFC_RESTRICT model, int index,
		pfc_cptr_t *PFC_RESTRICT valuep)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;

	PFC_ASSERT(index >= 0 && index < model->lm_nelems);

	elem = pfc_llist_lookup_by_index(headp, index);
	PFC_ASSERT(elem != NULL);

	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	return 0;
}

/*
 * static int
 * pfc_llist_insertat(listmodel_t *PFC_RESTRICT model, int index,
 *		      pfc_cptr_t PFC_RESTRICT value)
 *	Insert the list element at the specified index.
 *	Each element in the list with index greater or equal to the specified
 *	index is shifted upward.
 *
 * Calling/Exit State:
 *	Zero is returned on success.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_llist_insertat(listmodel_t *PFC_RESTRICT model, int index,
		   pfc_cptr_t PFC_RESTRICT value)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;
	pfc_list_t	*lhead;
	int	err, cookie;

	elem = pfc_llist_element_alloc(value);
	if (PFC_EXPECT_FALSE(elem == NULL)) {
		return ENOMEM;
	}

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto error;
	}

	if (PFC_EXPECT_FALSE(headp->ll_flags & LLF_SHUTDOWN)) {
		/* This list is already shut down. */
		err = ESHUTDOWN;
		goto error;
	}

	if (index == model->lm_nelems) {
		/* Link to the tail. */
		lhead = &headp->ll_head;
	}
	else {
		llist_elem_t	*old;

		old = pfc_llist_lookup_by_index(headp, index);
		if (PFC_EXPECT_FALSE(old == NULL)) {
			err = ENOENT;
			goto error;
		}

		/* Link before this element. */
		lhead = &old->le_list;
	}

	pfc_list_push_tail(lhead, &elem->le_list);
	pfc_listm_nelem_inc(model);

	/* Wake up one waiter. */
	pfc_llist_wakeup(headp);

	PFC_LLIST_UNLOCK(headp, cookie);

	return 0;

error:
	PFC_LLIST_UNLOCK(headp, cookie);
	free(elem);

	return err;
}

/*
 * static int
 * pfc_llist_removeat(listmodel_t *PFC_RESTRICT model, int index,
 *		      pfc_cptr_t *PFC_RESTRICT valuep)
 *	Remove the list element at the specified index.
 *
 * Calling/Exit State:
 *	Upon successful completion, the list element at the specified index
 *	is removed, and then zero is returned. Removed value is set to
 *	`*valuep' if `valuep' is not NULL.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
pfc_llist_removeat(listmodel_t *PFC_RESTRICT model, int index,
		   pfc_cptr_t *PFC_RESTRICT valuep)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;
	int	err, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	elem = pfc_llist_lookup_by_index(headp, index);
	if (PFC_EXPECT_FALSE(elem == NULL)) {
		err = ENOENT;
	}
	else {
		pfc_bool_t	refptr =
			(!PFC_LISTM_IS_64BIT(model) &&
			 pfc_listm_is_refptr_list(model));

		/* Remove this element. */
		if (valuep != NULL) {
			*valuep = elem->le_value;
		}
		pfc_list_remove(&elem->le_list);

		/*
		 * Remarks:
		 *	Note that refptr must not be put if the list is
		 *	64-bit list.
		 */
		pfc_llist_element_free(headp, elem, refptr);
	}

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return err;
}

/*
 * static int
 * pfc_llist_sort(listmodel_t *PFC_RESTRICT model, listsort_t *PFC_RESTRICT lsp)
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
pfc_llist_sort(listmodel_t *PFC_RESTRICT model, listsort_t *PFC_RESTRICT lsp)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
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
	PFC_LIST_FOREACH(&headp->ll_head, lelem) {
		llist_elem_t	*elem = LIST2ELEM(lelem);

		*bp = (pfc_ptr_t)elem;
		bp++;
	}
	PFC_ASSERT(bp == buffer + nelems);

	/* Do sort. */
	if (lsp->lms_refcomp) {
		qsort(buffer, nelems, sizeof(pfc_ptr_t),
		      pfc_llist_sort_refcomp);
	}
	else {
		qsort(buffer, nelems, sizeof(pfc_ptr_t), pfc_llist_sort_comp);
	}

	/* Reconstruct linked list in sorted order. */
	pfc_list_init(&headp->ll_head);
	for (bp = buffer; bp < buffer + nelems; bp++) {
		llist_elem_t	*elem = (llist_elem_t *)*bp;

		pfc_list_push_tail(&headp->ll_head, &elem->le_list);
	}

	free(buffer);

	return 0;
}

/*
 * static void
 * pfc_llist_clear(listmodel_t *model)
 *	Remove all elements from the list.
 *	This method is called with holding writer lock of the list.
 */
static void
pfc_llist_clear(listmodel_t *model)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;

	if (pfc_listm_is_refptr_list(model)) {
		while ((lelem = pfc_list_pop(&headp->ll_head)) != NULL) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			pfc_llist_element_free(headp, elem, PFC_TRUE);
		}
	}
	else {
		while ((lelem = pfc_list_pop(&headp->ll_head)) != NULL) {
			llist_elem_t	*elem = LIST2ELEM(lelem);

			pfc_llist_element_free(headp, elem, PFC_FALSE);
		}
	}
}

/*
 * static void
 * pfc_llist_iter_ctor(listiter_t *PFC_RESTRICT iter)
 *	Initialize the iterator to iterate linked list.
 */
static void
pfc_llist_iter_ctor(listiter_t *PFC_RESTRICT iter)
{
	listmodel_t	*model = iter->lmi_model;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_list_t	*lelem;
	int	from = iter->lmi_from;

	/* Set next list element into the iterator. */
	if (from == 0) {
		lelem = headp->ll_head.pl_next;
	}
	else if (from >= model->lm_nelems) {
		lelem = &headp->ll_head;
	}
	else {
		llist_elem_t	*elem;

		elem = pfc_llist_lookup_by_index(headp, from);
		PFC_ASSERT(elem != NULL);
		lelem = &elem->le_list;
	}

	/*
	 * Iterator for linked list keeps next pfc_list_t element in
	 * lmi_next, and previously retrieved linked list element in
	 * lmi_element.
	 */
	iter->lmi_next = (pfc_ptr_t)lelem;
	iter->lmi_element = NULL;

	return;
}

/*
 * static void
 * pfc_llist_iter_next(listiter_t *PFC_RESTRICT iter,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the next element in the list iterator, and move iterator's cursor
 *	forward.
 */
static void
pfc_llist_iter_next(listiter_t *PFC_RESTRICT iter,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	pfc_list_t	*lelem;
	llist_elem_t	*elem;
#ifdef	PFC_VERBOSE_DEBUG
	listmodel_t	*model = iter->lmi_model;
	llist_t		*headp = MODEL_TO_LLIST(model);
#endif	/* PFC_VERBOSE_DEBUG */

	lelem = (pfc_list_t *)iter->lmi_next;
	PFC_ASSERT(lelem != &headp->ll_head);

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	/* Update iterator. */
	iter->lmi_next = (pfc_ptr_t)lelem->pl_next;
	iter->lmi_element = (pfc_ptr_t)elem;
}

/*
 * static void
 * pfc_llist_iter_prev(listiter_t *PFC_RESTRICT iter,
 *		       pfc_cptr_t *PFC_RESTRICT valuep)
 *	Get the previous element in the list iterator, and move iterator's
 *	cursor backward.
 */
static void
pfc_llist_iter_prev(listiter_t *PFC_RESTRICT iter,
		    pfc_cptr_t *PFC_RESTRICT valuep)
{
	pfc_list_t	*lelem;
	llist_elem_t	*elem;
#ifdef	PFC_VERBOSE_DEBUG
	listmodel_t	*model = iter->lmi_model;
	llist_t		*headp = MODEL_TO_LLIST(model);
#endif	/* PFC_VERBOSE_DEBUG */

	lelem = (pfc_list_t *)iter->lmi_next;
	lelem = lelem->pl_prev;
	PFC_ASSERT(lelem != &headp->ll_head);

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	/* Update iterator. */
	iter->lmi_next = (pfc_ptr_t)lelem;
	iter->lmi_element = (pfc_ptr_t)elem;
}

/*
 * static void
 * pfc_llist_iter_rewind(listiter_t *PFC_RESTRICT iter)
 *	Rewind iterator's cursor.
 *	A subsequent call to pfc_listiter_next() will return the first element
 *	in the list.
 */
static void
pfc_llist_iter_rewind(listiter_t *PFC_RESTRICT iter)
{
	/* We can simply call iterator's constructor. */
	pfc_llist_iter_ctor(iter);
}

/*
 * static void
 * pfc_llist_iter_eol(listiter_t *PFC_RESTRICT iter)
 *	Move iterator's cursor to the end of the list.
 *	A subsequent call to pfc_listiter_prev() will return the last element
 *	in the list.
 */
static void
pfc_llist_iter_eol(listiter_t *PFC_RESTRICT iter)
{
	listmodel_t	*model = iter->lmi_model;
	llist_t		*headp = MODEL_TO_LLIST(model);

	iter->lmi_next = &headp->ll_head;
	iter->lmi_element = NULL;
}

/*
 * static void
 * pfc_llist_iter_remove(listiter_t *iter)
 *	Remove the element retrieved by the previous pfc_listiter_next() or
 *	pfc_listiter_prev() call.
 *
 * Remarks:
 *	This function is called with holding writer lock of the list.
 */
static void
pfc_llist_iter_remove(listiter_t *iter)
{
	listmodel_t	*model = iter->lmi_model;
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem = (llist_elem_t *)iter->lmi_element;
	pfc_list_t	*lelem;

	PFC_ASSERT(elem != NULL);

	/* Remove this element. */
	lelem = (pfc_list_t *)iter->lmi_next;
	if (lelem == &elem->le_list) {
		iter->lmi_next = (pfc_ptr_t)lelem->pl_next;
	}
	pfc_list_remove(&elem->le_list);
	pfc_llist_element_free(headp, elem, pfc_listm_is_refptr_list(model));

	/* Update iterator. */
	iter->lmi_element = NULL;
}

/*
 * static void
 * pfc_llist_lock(listmodel_t *model)
 *	Acquire the list mutex.
 */
static void
pfc_llist_lock(listmodel_t *model)
{
	llist_t	*headp = MODEL_TO_LLIST(model);

	(void)pfc_mutex_lock(&headp->ll_lock);
}

/*
 * static void
 * pfc_llist_unlock(listmodel_t *model)
 *	Release the list mutex.
 */
static void
pfc_llist_unlock(listmodel_t *model)
{
	llist_t	*headp = MODEL_TO_LLIST(model);

	(void)pfc_mutex_unlock(&headp->ll_lock);
}

/*
 * static int
 * pfc_llist_init(pfc_listm_t *listp, const pfc_refptr_ops_t *refops,
 *		  uint8_t flags)
 *	Common routine to create a new linked list.
 */
static int
pfc_llist_init(pfc_listm_t *listp, const pfc_refptr_ops_t *refops,
	       uint8_t flags)
{
	int	err;
	llist_t		*headp;
	listmodel_t	*model;

	/* Allocate list instance. */
	headp = (llist_t *)malloc(sizeof(*headp));
	if (PFC_EXPECT_FALSE(headp == NULL)) {
		return ENOMEM;
	}
	model = LLIST_TO_MODEL(headp);

	err = PFC_MUTEX_INIT(&headp->ll_lock);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	err = pfc_cond_init(&headp->ll_cond);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	pfc_listm_init(model, &llist_ops, refops, flags);

	pfc_list_init(&headp->ll_head);
	headp->ll_nwaiters = 0;
	headp->ll_flags = 0;

	*listp = (pfc_listm_t)model;

	return 0;

error:
	free(headp);

	return err;
}

/*
 * static int
 * pfc_llist_push_element(listmodel_t *PFC_RESTRICT model,
 *			  pfc_cptr_t PFC_RESTRICT value, pfc_bool_t tail)
 *	Common operation to push a new element to the list.
 *	If `tail' is true, an element is added to the tail of the list,
 *	otherwise the head of the list.
 */
static int
pfc_llist_push_element(listmodel_t *PFC_RESTRICT model,
		       pfc_cptr_t PFC_RESTRICT value, pfc_bool_t tail)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;
	int	err, cookie;

	elem = pfc_llist_element_alloc(value);
	if (PFC_EXPECT_FALSE(elem == NULL)) {
		return ENOMEM;
	}

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This list model is invalid. */
		goto error;
	}

	if (PFC_EXPECT_FALSE(headp->ll_flags & LLF_SHUTDOWN)) {
		/* This list is already shut down. */
		err = ESHUTDOWN;
		goto error;
	}

	if (tail) {
		/* Link element onto the tail of the list. */
		pfc_list_push_tail(&headp->ll_head, &elem->le_list);
	}
	else {
		/* Link element onto the head of the list. */
		pfc_list_push(&headp->ll_head, &elem->le_list);
	}
	pfc_listm_nelem_inc(model);

	/* Wake up one waiter. */
	pfc_llist_wakeup(headp);

	PFC_LLIST_UNLOCK(headp, cookie);

	return 0;

error:
	PFC_LLIST_UNLOCK(headp, cookie);
	free(elem);

	return err;
}

/*
 * static int
 * pfc_llist_pop_element(listmodel_t *PFC_RESTRICT model,
 *			 pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
 *	Common operation to pop an entry from the list.
 *	If `tail' is true, the list element at the tail of the list is removed
 *	from the list, otherwise the head of the list.
 */
static int
pfc_llist_pop_element(listmodel_t *PFC_RESTRICT model,
		      pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;
	pfc_list_t	*lelem;
	pfc_bool_t	unref;
	int	err, cookie;

	cookie = PFC_LLIST_LOCK(headp);

	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	if (tail) {
		/* Unlink the last element. */
		lelem = pfc_list_pop_tail(&headp->ll_head);
	}
	else {
		/* Unlink the first element. */
		lelem = pfc_list_pop(&headp->ll_head);
	}

	if (PFC_EXPECT_FALSE(lelem == NULL)) {
		err = ENOENT;
		goto out;
	}

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	/*
	 * Remarks:
	 *	Note that refptr must not be put if the list is 64-bit list.
	 */
	unref = (!PFC_LISTM_IS_64BIT(model) && pfc_listm_is_refptr_list(model));
	pfc_llist_element_free(headp, elem, unref);

out:
	PFC_LLIST_UNLOCK(headp, cookie);

	return err;
}

/*
 * static int
 * pfc_llist_get_edge(listmodel_t PFC_RESTRICT model,
 *		      pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
 *	Common operation to get an entry at the edge of the list.
 *	If `tail' is true, the last element of the list is set to `*valuep',
 *	otherwise the first.
 *
 * Remarks:
 *	This function is called with holding reader lock of the list.
 */
static int
pfc_llist_get_edge(listmodel_t *PFC_RESTRICT model,
		   pfc_cptr_t *PFC_RESTRICT valuep, pfc_bool_t tail)
{
	llist_t		*headp = MODEL_TO_LLIST(model);
	llist_elem_t	*elem;
	pfc_list_t	*lelem;

	PFC_ASSERT(model->lm_nelems > 0);
	PFC_ASSERT(!pfc_list_is_empty(&headp->ll_head));

	if (tail) {
		/* Get the last element. */
		lelem = headp->ll_head.pl_prev;
	}
	else {
		/* Get the first element. */
		lelem = headp->ll_head.pl_next;
	}
	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	return 0;
}

/*
 * static llist_elem_t *
 * pfc_llist_lookup_by_index(llist_t *headp, int index)
 *	Search for the list element at the specified index.
 *
 * Calling/Exit State:
 *	Pointer to the list element at the specified index is returned
 *	if present. NULL is returned if not present.
 *
 * Remarks:
 *	The caller must call this function with holding list mutex.
 */
static llist_elem_t *
pfc_llist_lookup_by_index(llist_t *headp, int index)
{
	listmodel_t	*model = LLIST_TO_MODEL(headp);
	pfc_list_t	*lelem;
	int	cnt, nelems;

	nelems = model->lm_nelems;
	if (PFC_EXPECT_FALSE(index < 0 || index >= nelems)) {
		return NULL;
	}

	if (index < (nelems >> 1)) {
		/* Search from the head. */
		cnt = 0;
		PFC_LIST_FOREACH(&headp->ll_head, lelem) {
			if (cnt == index) {
				break;
			}
			cnt++;
		}
	}
	else {
		/* Search from the tail. */
		cnt = nelems;
		PFC_LIST_REV_FOREACH(&headp->ll_head, lelem) {
			cnt--;
			if (cnt == index) {
				break;
			}
		}
	}

	PFC_ASSERT(lelem != &headp->ll_head);

	return LIST2ELEM(lelem);
}

/*
 * static int
 * pfc_llist_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
 *			      pfc_cptr_t *PFC_RESTRICT valuep,
 *			      const pfc_timespec_t *PFC_RESTRICT timeout,
 *			      pfc_bool_t tail)
 *	Common operation to wait for a new element to be popped.
 *	If `tail' is true, the list element at the tail of the list is removed
 *	from the list, otherwise the head of the list.
 */
static int
pfc_llist_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
			   pfc_cptr_t *PFC_RESTRICT valuep,
			   const pfc_timespec_t *PFC_RESTRICT timeout,
			   pfc_bool_t tail)
{
	listmodel_t	*model = (listmodel_t *)list;
	llist_t		*headp = MODEL_TO_LLIST(model);
	pfc_timespec_t	tspec, *abstime;
	llist_elem_t	*elem;
	pfc_list_t	*lelem;
	pfc_bool_t	unref;
	int	err, cookie;

	/* Ensure the list is linked list. */
	if (PFC_EXPECT_FALSE(model->lm_ops != &llist_ops)) {
		return EINVAL;
	}

	/* Convert timeout period into system absolute time. */
	if (timeout != NULL) {
		abstime = &tspec;
		err = pfc_clock_abstime(abstime, timeout);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		abstime = NULL;
	}

	/* Acquire mutex with specifying timeout. */
	err = pfc_llist_timedlock(headp, abstime, &cookie);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Hold the list so that it is not released by destroy method. */
	pfc_listm_ref(model);

	/* Ensure that the list is still valid. */
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Wait for at least one list element is added. */
	err = pfc_llist_wait(headp, abstime);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/*
	 * We need to verify the list again because pfc_llist_wai() might
	 * have released the lock.
	 */
	err = pfc_listm_verify(model);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	if (tail) {
		/* Unlink the last element. */
		lelem = pfc_list_pop_tail(&headp->ll_head);
	}
	else {
		/* Unlink the first element. */
		lelem = pfc_list_pop(&headp->ll_head);
	}
	PFC_ASSERT(lelem != NULL);

	elem = LIST2ELEM(lelem);
	if (valuep != NULL) {
		*valuep = elem->le_value;
	}

	/*
	 * Remarks:
	 *	Note that refptr must not be put if the list is 64-bit list.
	 */
	unref = (!PFC_LISTM_IS_64BIT(model) && pfc_listm_is_refptr_list(model));
	pfc_llist_element_free(headp, elem, unref);

out:
	if (model->lm_nelems != 0 || (headp->ll_flags & LLF_SHUTDOWN)) {
		/* Wake up one waiter. */
		pfc_llist_wakeup(headp);
	}

	PFC_LLIST_UNLOCK(headp, cookie);
	pfc_listm_unref(model);

	return err;
}

/*
 * static int
 * pfc_llist64_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
 *				uint64_t *PFC_RESTRICT valuep,
 *				const pfc_timespec_t *PFC_RESTRICT timeout,
 *				pfc_bool_t tail)
 *	64-bit list version of pfc_llist_pop_element_wait().
 */
static int
pfc_llist64_pop_element_wait(pfc_listm_t PFC_RESTRICT list,
			     uint64_t *PFC_RESTRICT valuep,
			     const pfc_timespec_t *PFC_RESTRICT timeout,
			     pfc_bool_t tail)
{
	int	err;

	if (PFC_EXPECT_FALSE(!PFC_LISTM_IS_64BIT(list))) {
		return EINVAL;
	}

	if (PFC_IS_LP64_SYSTEM()) {
		pfc_cptr_t	*ptr = (pfc_cptr_t *)(uintptr_t)valuep;

		/* LP64 system can keep 64-bit value as pointer. */
		err = pfc_llist_pop_element_wait(list, ptr, timeout, tail);
	}
	else {
		pfc_refptr_t	*ref;

		err = pfc_llist_pop_element_wait(list, (pfc_cptr_t *)&ref,
						 timeout, tail);
		if (PFC_EXPECT_TRUE(err == 0)) {
			/*
			 * List backend doesn't decrement reference counter
			 * on 64-bit list. So we must decrement here.
			 */
			PFC_ASSERT(ref->pr_refcnt == 1);
			if (valuep != NULL) {
				*valuep = pfc_listm64_eval_refptr(ref);
			}
			pfc_refptr_put(ref);
		}
	}

	return err;
}

/*
 * static int
 * pfc_llist_sort_comp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 */
static int
pfc_llist_sort_comp(const void *v1, const void *v2)
{
	llist_elem_t	*e1 = *((llist_elem_t **)v1);
	llist_elem_t	*e2 = *((llist_elem_t **)v2);
	const void	*o1 = e1->le_value;
	const void	*o2 = e2->le_value;
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return lsp->lms_comp(o1, o2);
}

/*
 * static int
 * pfc_llist_sort_refcomp(const void *v1, const void *v2)
 *	List comparator specified to qsort(3).
 *	This comparator is used if the list is refptr list.
 */
static int
pfc_llist_sort_refcomp(const void *v1, const void *v2)
{
	llist_elem_t	*e1 = *((llist_elem_t **)v1);
	llist_elem_t	*e2 = *((llist_elem_t **)v2);
	const void	*o1 = e1->le_value;
	const void	*o2 = e2->le_value;
	listsort_t	*lsp = listm_sort_context;

	PFC_ASSERT(lsp != NULL);

	return pfc_listm_refptr_comparator(lsp, o1, o2);
}
