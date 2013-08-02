/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_UTIL_SYNCBLOCK_IMPL_H
#define	_PFC_LIBPFC_UTIL_SYNCBLOCK_IMPL_H

/*
 * Internal definitions for synchronized block implementation.
 */

#include <pfc/syncblock.h>
#include <pfc/debug.h>

/* Determine whether the given frame is write lock or not. */
#define	PFC_SYNCBLOCK_IS_WRITE(frame)			\
	((frame)->psb_flags & PFC_SYNCBLOCK_WRITE)

/*
 * static inline __pfc_syncblock_t *
 * pfc_syncblock_lookup(__pfc_syncblock_t *head, pfc_cptr_t target)
 *	Search for a synchronized block for the target object in the
 *	specified synchronized block list.
 *
 * Calling/Exit State:
 *	If found, a pointer to synchronized block frame is returned.
 *	NULL is returned if not found.
 */
static inline __pfc_syncblock_t *
pfc_syncblock_lookup(__pfc_syncblock_t *head, pfc_cptr_t target)
{
	__pfc_syncblock_t	*sbp;

	for (sbp = head; sbp != NULL; sbp = sbp->psb_next) {
		if (sbp->psb_target == target) {
			break;
		}
	}

	return sbp;
}

/*
 * static inline pfc_bool_t
 * pfc_syncblock_enter(__pfc_syncblock_t *head, __pfc_syncblock_t *frame)
 *	Common routine to be run at the start of synchronized block.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if synchronization for the given target should
 *	be acquired.
 */
static inline pfc_bool_t
pfc_syncblock_enter(__pfc_syncblock_t **headp, __pfc_syncblock_t *frame)
{
	__pfc_syncblock_t	*sbp, *head;
	pfc_bool_t	result;

	/*
	 * Determine whether this block is a nested synchronized block
	 * or not.
	 */
	head = *headp;
	sbp = pfc_syncblock_lookup(head, frame->psb_target);
	if (sbp == NULL) {
		/* Link this frame to the current synchronized frame list. */
		frame->psb_next = head;
		*headp = frame;
		result = PFC_TRUE;
	}
	else {
		/*
		 * We don't allow write mode block nested in read mode block.
		 */
		PFC_ASSERT(PFC_SYNCBLOCK_IS_WRITE(sbp) ||
			   !PFC_SYNCBLOCK_IS_WRITE(frame));

		/*
		 * Increment block depth.
		 * We don't need to acquire any lock because all synchronized
		 * block frames are located at current thread's stack.
		 */
		sbp->psb_depth++;
		result = PFC_FALSE;
	}

	return result;
}

/*
 * static inline pfc_bool_t
 * pfc_syncblock_exit(__pfc_syncblock_t **headp, pfc_cptr_t target)
 *	Common routine to be run at the end of synchronized block.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if synchronization for the given target
 *	should be released.
 */
static inline pfc_bool_t
pfc_syncblock_exit(__pfc_syncblock_t **headp, pfc_cptr_t target)
{
	__pfc_syncblock_t	**sbpp, *sbp;
	pfc_bool_t	result;

	/* Search for the first synchronized block. */
	for (sbpp = headp; *sbpp != NULL; sbpp = &((*sbpp)->psb_next)) {
		if ((*sbpp)->psb_target == target) {
			break;
		}
	}

	sbp = *sbpp;
	PFC_ASSERT(sbp != NULL);

	/* Decrement lock depth. */
	sbp->psb_depth--;

	if (sbp->psb_depth == 0) {
		/* Now leaving from all synchronized blocks. */
		*sbpp = sbp->psb_next;
		result = PFC_TRUE;
	}
	else {
		result = PFC_FALSE;
	}

	return result;
}

#endif	/* !_PFC_LIBPFC_UTIL_SYNCBLOCK_IMPL_H */
