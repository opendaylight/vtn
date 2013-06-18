/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.c - Common utilities to handle IPC event.
 *
 * Remarks:
 *	Data serialization must be done by the caller.
 */

#include <string.h>
#include "ipc_impl.h"

/*
 * Internal prototypes.
 */
static int	ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
				 pfc_refptr_t *PFC_RESTRICT rname,
				 pfc_ipcevmask_t *PFC_RESTRICT mask);
static void	ipc_evmask_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);
static int	ipc_evmask_create(pfc_rbtree_t *PFC_RESTRICT tree,
				  pfc_refptr_t *PFC_RESTRICT rname,
				  const pfc_ipcevmask_t *PFC_RESTRICT mask);

static ipc_evmask_t	*ipc_evmask_copy(ipc_evmask_t *emp);
static pfc_cptr_t	ipc_evmask_getkey(pfc_rbnode_t *node);

/*
 * void
 * pfc_ipc_evset_init(ipc_evset_t *eset)
 *	Initialize the target event set.
 */
void
pfc_ipc_evset_init(ipc_evset_t *eset)
{
	pfc_rbtree_init(&eset->est_tree, (pfc_rbcomp_t)strcmp,
			ipc_evmask_getkey);
}

/*
 * void
 * pfc_ipc_evset_destroy(ipc_evset_t *eset)
 *	Destroy the target event set.
 *	All ipc_evmask_t nodes in the tree are also destroyed.
 */
void
pfc_ipc_evset_destroy(ipc_evset_t *eset)
{
	pfc_rbtree_clear(&eset->est_tree, ipc_evmask_dtor, NULL);
}

/*
 * int
 * pfc_ipc_evset_copy(ipc_evset_t *PFC_RESTRICT dst,
 *		      ipc_evset_t *PFC_RESTRICT src)
 *	Create a deep copy of the event set specified by `src'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 */
int
pfc_ipc_evset_copy(ipc_evset_t *PFC_RESTRICT dst,
		   ipc_evset_t *PFC_RESTRICT src)
{
	pfc_rbtree_t	*stree = &src->est_tree;
	pfc_rbtree_t	*dtree = &dst->est_tree;
	pfc_rbnode_t	*node;

	pfc_rbtree_init(&dst->est_tree, (pfc_rbcomp_t)strcmp,
			ipc_evmask_getkey);

	node = NULL;
	while ((node = pfc_rbtree_next(stree, node)) != NULL) {
		ipc_evmask_t	*emp;

		emp = ipc_evmask_copy(IPC_EVMASK_NODE2PTR(node));
		if (PFC_EXPECT_FALSE(emp == NULL)) {
			pfc_rbtree_clear(dtree, ipc_evmask_dtor, NULL);

			return ENOMEM;
		}

		PFC_ASSERT_INT(pfc_rbtree_put(dtree, &emp->iem_node), 0);
	}

	return 0;
}

/*
 * int
 * pfc_ipc_evset_merge(ipc_evset_t *PFC_RESTRICT dst,
 *		       ipc_evset_t *PFC_RESTRICT src)
 *	Merge event set in `src' to `dst'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 *
 * Remarks:
 *	This function never reverts changes made to `dst' on error return.
 */
int
pfc_ipc_evset_merge(ipc_evset_t *PFC_RESTRICT dst,
		    ipc_evset_t *PFC_RESTRICT src)
{
	pfc_rbtree_t	*stree = &src->est_tree;
	pfc_rbnode_t	*node;

	node = NULL;
	while ((node = pfc_rbtree_next(stree, node)) != NULL) {
		ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);
		int		err;

		err = ipc_evset_refadd(dst, emp->iem_name, &emp->iem_mask);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * int
 * pfc_ipc_evset_mergenew(ipc_evset_t *PFC_RESTRICT dst,
 *			  ipc_evset_t *PFC_RESTRICT eset,
 *			  ipc_evset_t *PFC_RESTRICT src)
 *	Merge pairs of name and event mask bits in `src', which are not found
 *	in `eset', to `dst'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 *
 * Remarks:
 *	This function never reverts changes made to `dst' on error return.
 */
int
pfc_ipc_evset_mergenew(ipc_evset_t *PFC_RESTRICT dst,
		       ipc_evset_t *PFC_RESTRICT eset,
		       ipc_evset_t *PFC_RESTRICT src)
{
	pfc_rbtree_t	*etree = &eset->est_tree;
	pfc_rbtree_t	*stree = &src->est_tree;
	pfc_rbnode_t	*node;

	node = NULL;
	while ((node = pfc_rbtree_next(stree, node)) != NULL) {
		ipc_evmask_t	*semp = IPC_EVMASK_NODE2PTR(node);
		ipc_evmask_t	*eemp;
		pfc_refptr_t	*rname = semp->iem_name;
		const char	*name = pfc_refptr_string_value(rname);
		pfc_ipcevmask_t	mask;
		pfc_rbnode_t	*n;
		int		err;

		n = pfc_rbtree_get(etree, (pfc_cptr_t)name);
		if (n == NULL) {
			/* This name does not exist in eset. */
			err = ipc_evset_refadd(dst, rname, &semp->iem_mask);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return err;
			}

			continue;
		}

		/* Determine mask bits that is not set in eset. */
		eemp = IPC_EVMASK_NODE2PTR(n);
		mask = ~(eemp->iem_mask) & semp->iem_mask;
		if (mask != 0) {
			/* This mask bits must be merged. */
			err = ipc_evset_refadd(dst, rname, &mask);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return err;
			}
		}
	}

	return 0;
}

/*
 * int
 * pfc_ipc_evset_add(ipc_evset_t *PFC_RESTRICT eset,
 *		     const char *PFC_RESTRICT name,
 *		     const pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Add a pair of IPC service name and event mask to the target event set.
 *
 *	If the service name specified by `service' already exists, the event
 *	mask specified by `mask' is merged to existing event mask.
 *
 *	Specifying NULL to `mask' is identical to specify the event mask with
 *	full bit set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 *
 * Remarks:
 *	The caller must guarantee that the IPC service name specified to `name'
 *	is valid.
 */
int
pfc_ipc_evset_add(ipc_evset_t *PFC_RESTRICT eset,
		  const char *PFC_RESTRICT name,
		  const pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	pfc_rbtree_t	*tree = &eset->est_tree;
	pfc_rbnode_t	*node;
	ipc_evmask_t	*emp;

	PFC_ASSERT(name != NULL && strlen(name) <= IPC_SERVICE_NAMELEN_MAX);

	/* Search for the ipc_evmask_t node associated with the given name. */
	node = pfc_rbtree_get(tree, (pfc_cptr_t)name);
	if (node == NULL) {
		pfc_refptr_t	*rname;
		int		err;

		/* Allocate a new node. */
		rname = pfc_refptr_string_create(name);
		if (PFC_EXPECT_FALSE(rname == NULL)) {
			return ENOMEM;
		}

		err = ipc_evmask_create(tree, rname, mask);
		if (PFC_EXPECT_FALSE(err != 0)) {
			pfc_refptr_put(rname);
		}

		return err;
	}

	/* Add mask bits to this ipc_evmask_t node. */
	emp = IPC_EVMASK_NODE2PTR(node);
	if (mask == NULL) {
		emp->iem_mask = PFC_IPC_EVENT_MASK_FILL;
	}
	else {
		emp->iem_mask |= (*mask);
	}

	return 0;
}

/*
 * int
 * pfc_ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
 *			pfc_refptr_t *PFC_RESTRICT rname,
 *			pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Add a pair of IPC service name and event mask to the target event set.
 *
 *	This function takes the IPC service name as refptr string.
 *	The reference counter of the refptr string is incremented on
 *	successful return.
 *
 *	Specifying NULL to `mask' is identical to specify the event mask with
 *	full bit set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 *
 * Remarks:
 *	The caller must guarantee that the IPC service name specified to
 *	`rname' is valid.
 */
int
pfc_ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
		     pfc_refptr_t *PFC_RESTRICT rname,
		     pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	return ipc_evset_refadd(eset, rname, mask);
}

/*
 * void
 * pfc_ipc_evset_remove(ipc_evset_t *PFC_RESTRICT eset,
 *			const char *PFC_RESTRICT name,
 *			pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Remove the event mask specified by `mask' from ipc_evmask_t node
 *	associated with the IPC service name `name'.
 *
 *	Event mask associated with `name' is removed if NULL is specified to
 *	`mask'.
 */
void
pfc_ipc_evset_remove(ipc_evset_t *PFC_RESTRICT eset,
		     const char *PFC_RESTRICT name,
		     pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	pfc_rbtree_t	*tree = &eset->est_tree;
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(tree, (pfc_cptr_t)name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);

		if (mask == NULL) {
			emp->iem_mask = 0;
		}
		else {
			emp->iem_mask &= ~(*mask);
		}

		if (emp->iem_mask == 0) {
			/* Remove this node. */
			pfc_rbtree_remove_node(tree, node);
		}
	}
}

/*
 * ipc_evmask_t *
 * pfc_ipc_evset_lookup(ipc_evset_t *PFC_RESTRICT eset,
 *			const char *PFC_RESTRICT name, pfc_ipcevtype_t type)
 *	Search for an ipc_evmask_t instance in the specified event set,
 *	which matches the specified IPC service name and event type.
 *
 *	Event mask test is omitted if IPC_EVTYPE_NONE is specified to `type'.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to ipc_evmask_t is returned if found.
 *	NULL is returned if not found.
 */
ipc_evmask_t *
pfc_ipc_evset_lookup(ipc_evset_t *PFC_RESTRICT eset,
		     const char *PFC_RESTRICT name, pfc_ipcevtype_t type)
{
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(&eset->est_tree, (pfc_cptr_t)name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);

		if (type == IPC_EVTYPE_NONE ||
		    IPC_EVMASK_TEST_TYPE(emp->iem_mask, type)) {
			return emp;
		}
	}

	return NULL;
}

/*
 * static int
 * ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
 *		    pfc_refptr_t *PFC_RESTRICT rname,
 *		    pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Add a pair of IPC service name and event mask to the target event set.
 *
 *	See comments on pfc_ipc_evset_refadd().
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 */
static int
ipc_evset_refadd(ipc_evset_t *PFC_RESTRICT eset,
		 pfc_refptr_t *PFC_RESTRICT rname,
		 pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	pfc_rbtree_t	*tree = &eset->est_tree;
	pfc_rbnode_t	*node;
	ipc_evmask_t	*emp;
	const char	*name;

	PFC_ASSERT(rname != NULL &&
		   pfc_refptr_string_length(rname) <= IPC_SERVICE_NAMELEN_MAX);

	/* Search for the ipc_evmask_t node associated with the given name. */
	name = pfc_refptr_string_value(rname);
	node = pfc_rbtree_get(tree, (pfc_cptr_t)name);
	if (node == NULL) {
		int	err;

		/* Allocate a new node. */
		err = ipc_evmask_create(tree, rname, mask);
		if (PFC_EXPECT_TRUE(err == 0)) {
			pfc_refptr_get(rname);
		}

		return err;
	}

	/* Add mask bits to this ipc_evmask_t node. */
	emp = IPC_EVMASK_NODE2PTR(node);
	if (mask == NULL) {
		emp->iem_mask = PFC_IPC_EVENT_MASK_FILL;
	}
	else {
		emp->iem_mask |= (*mask);
	}

	return 0;
}

/*
 * static void
 * ipc_evmask_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of ipc_evmask_t.
 */
static void
ipc_evmask_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);

	pfc_refptr_put(emp->iem_name);
	free(emp);
}

/*
 * static int
 * ipc_evmask_create(pfc_rbtree_t *PFC_RESTRICT tree,
 *		     pfc_refptr_t *PFC_RESTRICT rname,
 *		     const pfc_ipcevmask_t *PFC_RESTRICT mask)
 *	Create a new ipc_evmask_t node, and put it to the specified event
 *	mask tree.
 *
 *	Specifying NULL to `mask' is identical to specify the event mask with
 *	full bit set.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOMEM is returned if no memory is available.
 *
 * Remarks:
 *	The caller must guarantee that no node is associated with the specified
 *	IPC service name in the event mask tree.
 */
static int
ipc_evmask_create(pfc_rbtree_t *PFC_RESTRICT tree,
		  pfc_refptr_t *PFC_RESTRICT rname,
		  const pfc_ipcevmask_t *PFC_RESTRICT mask)
{
	ipc_evmask_t	*emp;

	emp = (ipc_evmask_t *)malloc(sizeof(*emp));
	if (PFC_EXPECT_FALSE(emp == NULL)) {
		return ENOMEM;
	}

	emp->iem_name = rname;
	emp->iem_mask = (mask == NULL) ? PFC_IPC_EVENT_MASK_FILL : *mask;

	PFC_ASSERT_INT(pfc_rbtree_put(tree, &emp->iem_node), 0);

	return 0;
}

/*
 * static ipc_evmask_t *
 * ipc_evmask_copy(ipc_evmask_t *emp)
 *	Create a deep copy of the specified event mask node.
 *	NULL is returned on failure.
 */
static ipc_evmask_t *
ipc_evmask_copy(ipc_evmask_t *emp)
{
	ipc_evmask_t	*newemp;

	newemp = (ipc_evmask_t *)malloc(sizeof(*newemp));
	if (PFC_EXPECT_TRUE(newemp != NULL)) {
		pfc_refptr_t	*rname = emp->iem_name;

		pfc_refptr_get(rname);
		newemp->iem_name = rname;
		newemp->iem_mask = emp->iem_mask;
	}

	return newemp;
}

/*
 * static pfc_cptr_t
 * ipc_evmask_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified Red-Black tree node.
 *	`node' must be a pointer to iem_node in ipc_evmask_t.
 */
static pfc_cptr_t
ipc_evmask_getkey(pfc_rbnode_t *node)
{
	ipc_evmask_t	*emp = IPC_EVMASK_NODE2PTR(node);

	return (pfc_cptr_t)pfc_refptr_string_value(emp->iem_name);
}
