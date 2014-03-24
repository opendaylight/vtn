/*
 * Copyright (c) 2010-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIST_H
#define	_PFC_LIST_H

#include <stdlib.h>
#include <pfc/base.h>

/*
 * Simple doubly linked list.
 *
 * Remarks:
 *	Serialization must be implemented by user.
 */

PFC_C_BEGIN_DECL

struct pfc_list;
typedef	struct pfc_list		pfc_list_t;

struct pfc_list {
	pfc_list_t	*pl_next;
	pfc_list_t	*pl_prev;
};

#define	PFC_LIST_INITIALIZER(list)	{ &(list), &(list) }

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_init(pfc_list_t *list)
 *	Initialize head of doubly linked list.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_init(pfc_list_t *list)
{
	list->pl_next = list;
	list->pl_prev = list;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_push(pfc_list_t *head, pfc_list_t *elem)
 *	Add the given list element to the head of the list.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_push(pfc_list_t *head, pfc_list_t *elem)
{
	elem->pl_next = head->pl_next;
	elem->pl_prev = head;
	head->pl_next->pl_prev = elem;
	head->pl_next = elem;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_push_tail(pfc_list_t *head, pfc_list_t *elem)
 *	Add the given list element to the tail of the list.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_push_tail(pfc_list_t *head, pfc_list_t *elem)
{
	elem->pl_next = head;
	elem->pl_prev = head->pl_prev;
	head->pl_prev->pl_next = elem;
	head->pl_prev = elem;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_remove(pfc_list_t *elem)
 *	Remove the given element from the list.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_remove(pfc_list_t *elem)
{
	elem->pl_next->pl_prev = elem->pl_prev;
	elem->pl_prev->pl_next = elem->pl_next;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_list_is_empty(pfc_list_t *head)
 *	PFC_TRUE is returned if the list is empty.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_list_is_empty(pfc_list_t *head)
{
	return (head->pl_next == head) ? PFC_TRUE : PFC_FALSE;
}

/*
 * static inline pfc_list_t * PFC_FATTR_ALWAYS_INLINE
 * pfc_list_pop(pfc_list_t *head)
 *	Unlink the first element in the list, and returns it.
 *	NULL is returned if no element exists.
 */
static inline pfc_list_t * PFC_FATTR_ALWAYS_INLINE
pfc_list_pop(pfc_list_t *head)
{
	pfc_list_t	*elem;

	if (pfc_list_is_empty(head)) {
		elem = NULL;
	}
	else {
		elem = head->pl_next;
		head->pl_next = elem->pl_next;
		elem->pl_next->pl_prev = head;
	}

	return elem;
}

/*
 * static inline pfc_list_t * PFC_FATTR_ALWAYS_INLINE
 * pfc_list_pop_tail(pfc_list_t *head)
 *	Unlink the last element in the list, and returns it.
 *	NULL is returned if no element exists.
 */
static inline pfc_list_t * PFC_FATTR_ALWAYS_INLINE
pfc_list_pop_tail(pfc_list_t *head)
{
	pfc_list_t	*elem;

	if (pfc_list_is_empty(head)) {
		elem = NULL;
	}
	else {
		elem = head->pl_prev;
		head->pl_prev = elem->pl_prev;
		elem->pl_prev->pl_next = head;
	}

	return elem;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_replace(pfc_list_t *oelem, pfc_list_t *nelem)
 *	Replace list element pointed by `oelem' with `nelem'.
 *
 * Remarks:
 *	This function corrupts `oelem' if `oelem' is empty.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_replace(pfc_list_t *oelem, pfc_list_t *nelem)
{
	nelem->pl_next = oelem->pl_next;
	nelem->pl_prev = oelem->pl_prev;
	oelem->pl_next->pl_prev = nelem;
	oelem->pl_prev->pl_next = nelem;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_list_move_all(pfc_list_t *head, pfc_list_t *newhead)
 *	Move all elements in the list pointed by `head' to `newhead'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_list_move_all(pfc_list_t *head, pfc_list_t *newhead)
{
	if (pfc_list_is_empty(head)) {
		pfc_list_init(newhead);
	}
	else {
		pfc_list_replace(head, newhead);
	}
}

/*
 * List iterator.
 */
#define	PFC_LIST_FOREACH_FROM(head, from, var)				\
	for ((var) = (from); (var) != (head); (var) = (var)->pl_next)
#define	PFC_LIST_REV_FOREACH_FROM(head, from, var)			\
	for ((var) = (from); (var) != (head); (var) = (var)->pl_prev)

#define	PFC_LIST_FOREACH(head, var)				\
	PFC_LIST_FOREACH_FROM(head, (head)->pl_next, var)
#define	PFC_LIST_REV_FOREACH(head, var)				\
	PFC_LIST_REV_FOREACH_FROM(head, (head)->pl_prev, var)

/*
 * List iterator that allow removal of `var'.
 * One more variable must be specified to preserve next or previous link.
 */
#define	PFC_LIST_FOREACH_SAFE_FROM(head, from, var, next)		\
	for ((var) = (from);						\
	     (var) != (head) && ((next) = (var)->pl_next, 1);           \
	     (var) = (next))
#define	PFC_LIST_REV_FOREACH_SAFE_FROM(head, from, var, prev)		\
	for ((var) = (from);						\
	     (var) != (head) && ((prev) = (var)->pl_prev, 1);           \
	     (var) = (prev))

#define	PFC_LIST_FOREACH_SAFE(head, var, next)				\
	PFC_LIST_FOREACH_SAFE_FROM(head, (head)->pl_next, var, next)
#define	PFC_LIST_REV_FOREACH_SAFE(head, var, prev)			\
	PFC_LIST_REV_FOREACH_SAFE_FROM(head, (head)->pl_prev, var, prev)

PFC_C_END_DECL

#endif	/* !_PFC_LIST_H */
