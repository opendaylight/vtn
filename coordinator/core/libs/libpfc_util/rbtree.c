/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * rbtree.c - Red-Black Tree implementation.
 *
 * Red-Black Tree is one of self-balancing binary search tree implementation.
 * The followings are rules to maintain Red-Black Tree.
 *
 * 1. Every tree node in Red-Black Tree is colored by either red or black.
 * 2. Root tree node is black.
 * 3. All leaf nodes are black.
 * 4. Both children of every red node are black. From contraposition, we can
 *    say that parent of every red node is black.
 * 5. Every path from a node in the tree to any of its descendant leaf nodes
 *    contains the same number of black nodes.
 *
 * Typical Red-Black Tree implementation uses nil leaf. That is, every leaf
 * nodes has no data. This implementation uses NULL pointer instead of using
 * dummy nil node.
 */

#include <pfc/rbtree.h>
#include <pfc/debug.h>

/*
 * Set or test color.
 */
#define	RBNODE_SET_RED(np)			\
	do {					\
		(np)->rbn_red = PFC_TRUE;	\
	} while (0)
#define	RBNODE_SET_BLACK(np)			\
	do {					\
		(np)->rbn_red = PFC_FALSE;	\
	} while (0)
#define	RBNODE_IS_RED(np)	((np)->rbn_red)
#define	RBNODE_IS_BLACK(np)	(!((np)->rbn_red))

/*
 * Check whether the node is root node.
 */
#define	RBNODE_IS_ROOT(tree, np)	((np) == (tree)->rb_root)

/*
 * Initialize new tree node.
 */
#define	RBNODE_INIT(np, parent, red)					\
	do {								\
		int	__i;						\
									\
		(np)->rbn_parent = (parent);				\
		(np)->rbn_red = (red);					\
		for (__i = 0; __i < PFC_RBNODE_NCHILDREN; __i++) {	\
			(np)->rbn_children[__i] = NULL;			\
		}							\
	} while (0)

#define	RBNODE_INIT_RED(np, parent)	RBNODE_INIT(np, parent, PFC_TRUE)
#define	RBNODE_INIT_BLACK(np, parent)	RBNODE_INIT(np, parent, PFC_FALSE)

/*
 * Evaluate children link.
 */
#define	RBNODE_LEFT(np)		((np)->rbn_children[PFC_RBNODE_IDX_LEFT])
#define	RBNODE_RIGHT(np)	((np)->rbn_children[PFC_RBNODE_IDX_RIGHT])
#define	RBNODE_CHILD(np, index)	((np)->rbn_children[index])

/*
 * Assertion macro which checks a node specified by array index is black node.
 */
#ifdef	PFC_VERBOSE_DEBUG
#define	RBNODE_BLACK_ASSERT(np)						\
	do {								\
		pfc_rbnode_t	*__np = (np);				\
									\
		PFC_ASSERT(__np == NULL || RBNODE_IS_BLACK(__np));	\
	} while (0)
#else	/* !PFC_VERBOSE_DEBUG */
#define	RBNODE_BLACK_ASSERT(np)		((void)0)
#endif	/* PFC_VERBOSE_DEBUG */

/*
 * Prototype of rotate method.
 */
typedef void	(*rbtree_rotate_t)(pfc_rbtree_t *PFC_RESTRICT tree,
				   pfc_rbnode_t *PFC_RESTRICT node);

/*
 * Internal prototypes.
 */
static int	rbtree_insert(pfc_rbtree_t *PFC_RESTRICT tree,
			      pfc_rbnode_t *PFC_RESTRICT node,
			      pfc_rbnode_t **PFC_RESTRICT removed);
static void	rbtree_rotate_left(pfc_rbtree_t *PFC_RESTRICT tree,
				   pfc_rbnode_t *PFC_RESTRICT node);
static void	rbtree_rotate_right(pfc_rbtree_t *PFC_RESTRICT tree,
				    pfc_rbnode_t *PFC_RESTRICT node);
static pfc_rbnode_t	*rbtree_lookup_node(pfc_rbtree_t *PFC_RESTRICT tree,
					    pfc_cptr_t PFC_RESTRICT key);
static void	rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
				   pfc_rbnode_t *PFC_RESTRICT node);
static void	rbtree_replace_node(pfc_rbtree_t *PFC_RESTRICT tree,
				    pfc_rbnode_t *PFC_RESTRICT oldnode,
				    pfc_rbnode_t *PFC_RESTRICT newnode);
static void	rbtree_swap_node(pfc_rbtree_t *PFC_RESTRICT tree,
				 pfc_rbnode_t *PFC_RESTRICT n1,
				 pfc_rbnode_t *PFC_RESTRICT n2);
static void	rbtree_swap_parent(pfc_rbtree_t *PFC_RESTRICT tree,
				   pfc_rbnode_t *PFC_RESTRICT parent,
				   pfc_rbnode_t *PFC_RESTRICT child);
static void	rbtree_fixup_insert(pfc_rbtree_t *PFC_RESTRICT tree,
				    pfc_rbnode_t *PFC_RESTRICT node);
static void	rbtree_fixup_remove(pfc_rbtree_t *PFC_RESTRICT tree,
				    pfc_rbnode_t *PFC_RESTRICT node);
static pfc_rbnode_t	*rbtree_first(pfc_rbtree_t *tree);
static pfc_rbnode_t	*rbtree_first_leaf(pfc_rbnode_t *np);
static pfc_rbnode_t	*rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree,
				     pfc_rbnode_t *PFC_RESTRICT node);

/*
 * Tree maintenance operations.
 */
typedef struct {
	int		rbops_inner_idx;	/* child index of inner node */
	int		rbops_outer_idx;	/* child index of outer node */
	rbtree_rotate_t	rbops_rotate_inside;	/* rotate inside */
	rbtree_rotate_t	rbops_rotate_outside;	/* rotate outside */
} rbtree_ops_t;

static const rbtree_ops_t	rbtree_ops[] = {
	/* Operations for left side. */
	{
		PFC_RBNODE_IDX_RIGHT,
		PFC_RBNODE_IDX_LEFT,
		rbtree_rotate_right,
		rbtree_rotate_left,
	},

	/* Operations for right side. */
	{
		PFC_RBNODE_IDX_LEFT,
		PFC_RBNODE_IDX_RIGHT,
		rbtree_rotate_left,
		rbtree_rotate_right,
	},
};

/*
 * static inline int
 * rbtree_child_index(pfc_rbnode_t *PFC_RESTRICT parent,
 *		      pfc_rbnode_t *PFC_RESTRICT child)
 *	Return child node index of `child' in the specified parent node.
 */
static inline int
rbtree_child_index(pfc_rbnode_t *PFC_RESTRICT parent,
		   pfc_rbnode_t *PFC_RESTRICT child)
{
	if (RBNODE_LEFT(parent) == child) {
		return PFC_RBNODE_IDX_LEFT;
	}

	PFC_ASSERT(RBNODE_RIGHT(parent) == child);

	return PFC_RBNODE_IDX_RIGHT;
}

/*
 * int
 * pfc_rbtree_put(pfc_rbtree_t *PFC_RESTRICT tree,
 *		  pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert `node' to the Red-Black Tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if the tree contains a node which has the same
 *	value as `node'.
 */
int
pfc_rbtree_put(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbnode_t *PFC_RESTRICT node)
{
	return rbtree_insert(tree, node, NULL);
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_update(pfc_rbtree_t *PFC_RESTRICT tree,
 *		     pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert `node' to the Red-Black Tree.
 *	If the tree contains a node which has the same value as `node',
 *	it is replaced with the specified node.
 *
 * Calling/Exit State:
 *	The previous node which has the same key as `node' is returned.
 *	NULL is returned if the tree doesn't contain a node which has the
 *	same key as `node'.
 */
pfc_rbnode_t *
pfc_rbtree_update(pfc_rbtree_t *PFC_RESTRICT tree,
		  pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*removed;

	PFC_ASSERT_INT(rbtree_insert(tree, node, &removed), 0);

	return removed;
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_get(pfc_rbtree_t *PFC_RESTRICT tree, pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the specified key is returned.
 *	NULL is returned if not found.
 */
pfc_rbnode_t *
pfc_rbtree_get(pfc_rbtree_t *PFC_RESTRICT tree, pfc_cptr_t PFC_RESTRICT key)
{
	return rbtree_lookup_node(tree, key);
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_get_ceil(pfc_rbtree_t *PFC_RESTRICT tree,
 *		       pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the least key greater than or equal to
 *	the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the least key greater than or equal to
 *	the specified key.
 *	NULL is returned if not found.
 */
pfc_rbnode_t *
pfc_rbtree_get_ceil(pfc_rbtree_t *PFC_RESTRICT tree,
		    pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*np;

	np = tree->rb_root;
	while (np != NULL) {
		int	ret = tree->rb_comp(key, tree->rb_keyfunc(np));

		if (ret < 0) {
			if (RBNODE_LEFT(np) != NULL) {
				np = RBNODE_LEFT(np);
				continue;
			}

			/*
			 * This node is greater than the specified key, and
			 * there is no node whose key is less than this node.
			 * This is what I'm looking for.
			 */
			break;
		}
		else if (ret > 0) {
			pfc_rbnode_t	*child;

			if (RBNODE_RIGHT(np) != NULL) {
				np = RBNODE_RIGHT(np);
				continue;
			}

			/*
			 * This node is less than the specified key, and there
			 * is no node whose key is greater than this node.
			 * Look for the ancestor which is linked to right side
			 * of its parent.
			 */
			child = np;
			np = np->rbn_parent;
			while (np != NULL && RBNODE_RIGHT(np) == child) {
				child = np;
				np = np->rbn_parent;
			}
			break;
		}
		else {
			/* This node has the same value as the key. */
			break;
		}
	}

	return np;
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_get_floor(pfc_rbtree_t *PFC_RESTRICT tree,
 *		        pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the greatest key less than or equal
 *	to the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the greatest key less than or equal
 *	to the specified key.
 *	NULL is returned if not found.
 */
pfc_rbnode_t *
pfc_rbtree_get_floor(pfc_rbtree_t *PFC_RESTRICT tree,
		     pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*np;

	np = tree->rb_root;
	while (np != NULL) {
		int	ret = tree->rb_comp(key, tree->rb_keyfunc(np));

		if (ret > 0) {
			if (RBNODE_RIGHT(np) != NULL) {
				np = RBNODE_RIGHT(np);
				continue;
			}

			/*
			 * This node is less than the specified key, and there
			 * is no node whose key is greater than this node.
			 * This is what I'm looking for.
			 */
			break;
		}
		else if (ret < 0) {
			pfc_rbnode_t	*child;

			if (RBNODE_LEFT(np) != NULL) {
				np = RBNODE_LEFT(np);
				continue;
			}

			/*
			 * This node is greater than the specified key, and
			 * there is no node whose key is less than this node.
			 * Look for the ancestor which is linked to left side
			 * of its parent.
			 */
			child = np;
			np = np->rbn_parent;
			while (np != NULL && RBNODE_LEFT(np) == child) {
				child = np;
				np = np->rbn_parent;
			}
			break;
		}
		else {
			/* This node has the same value as the key. */
			break;
		}
	}

	return np;
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_remove(pfc_rbtree_t *PFC_RESTRICT tree,
 *		     pfc_cptr_t PFC_RESTRICT key)
 *	Remove the tree node associated with the specified key.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to removed tree node is returned.
 *	NULL is returned if not found.
 */
pfc_rbnode_t *
pfc_rbtree_remove(pfc_rbtree_t *PFC_RESTRICT tree, pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node = rbtree_lookup_node(tree, key);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		rbtree_remove_node(tree, node);
	}

	return node;
}

/*
 * void
 * pfc_rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
 *			  pfc_rbnode_t *PFC_RESTRICT node)
 *	Remove the specified tree node.
 *	The node must be linked to the specified Red-Black Tree.
 */
void
pfc_rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
		       pfc_rbnode_t *PFC_RESTRICT node)
{
	PFC_ASSERT(rbtree_lookup_node(tree, tree->rb_keyfunc(node)) == node);

	rbtree_remove_node(tree, node);
}

/*
 * pfc_rbnode_t *
 * pfc_rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree,
 *		   pfc_rbnode_t *PFC_RESTRICT node)
 *	Return next node of the specified node, which has the least key
 *	greater than the specified key.
 *
 *	If `node' is NULL, the first node, which is the least node in the
 *	tree, is returned.
 *
 * Calling/Exit State:
 *	The next node of the specified node is returned.
 *	NULL is returned if the last node in the tree is specified.
 *
 * Remarks:
 *	The caller must guarantee that the tree is never changed during the
 *	call of pfc_rbtree_next(). Tree update during the call of
 *	pfc_rbtree_next() causes undefined behavior.
 */
pfc_rbnode_t *
pfc_rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree,
		pfc_rbnode_t *PFC_RESTRICT node)
{
	if (node == NULL) {
		return rbtree_first(tree);
	}

	return rbtree_next(tree, node);
}

/*
 * void
 * pfc_rbtree_clear(pfc_rbtree_t *tree, pfc_rbcallback_t dtor, pfc_ptr_t arg)
 *	Clear Red-Black tree.
 *	The tree will be empty after this function call returns.
 *
 *	All nodes in the tree pointed by `tree' is removed.
 *	if `dtor' is not NULL, it is called with specifying each tree node
 *	removed by this function. `arg' is passed to the destructor.
 *
 * Remarks:
 *	Function specified by `dtor' must not refer Red-Black tree pointed by
 *	`tree'.
 */
void
pfc_rbtree_clear(pfc_rbtree_t *tree, pfc_rbcallback_t dtor, pfc_ptr_t arg)
{
	pfc_rbnode_t	*np = tree->rb_root;

	while (np != NULL) {
		pfc_rbnode_t	*parent;

		/* Find the least leaf node, and remove it. */
		np = rbtree_first_leaf(np);
		PFC_ASSERT(np != NULL);

		parent = np->rbn_parent;
		if (parent == NULL) {
			PFC_ASSERT(RBNODE_IS_ROOT(tree, np));
		}
		else {
			int	index = rbtree_child_index(parent, np);

			RBNODE_CHILD(parent, index) = NULL;
		}

		/* Call destructor. */
		if (dtor != NULL) {
			(*dtor)(np, arg);
		}

		np = parent;
	}

	/* Make the tree be empty. */
	tree->rb_root = NULL;
}

/*
 * int
 * pfc_rbtree_int32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of signed 32-bit integer.
 *	Both arguments are considered as int32_t value.
 *
 * Calling/Exit State:
 *	This function returns an integer less than, equal to, or greater than
 *	zero if the first argument is respectively less than, equal to, or
 *	greater than the second.
 */
int
pfc_rbtree_int32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
	int32_t	v1 = (int32_t)(uintptr_t)arg1;
	int32_t	v2 = (int32_t)(uintptr_t)arg2;

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * int
 * pfc_rbtree_uint32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of unsigned 32-bit integer.
 *	Both arguments are considered as uint32_t value.
 *
 * Calling/Exit State:
 *	This function returns an integer less than, equal to, or greater than
 *	zero if the first argument is respectively less than, equal to, or
 *	greater than the second.
 */
int
pfc_rbtree_uint32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
	uint32_t	v1 = (uint32_t)(uintptr_t)arg1;
	uint32_t	v2 = (uint32_t)(uintptr_t)arg2;

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * int
 * pfc_rbtree_int64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of signed 64-bit integer.
 *	Both arguments must be result of PFC_RBTREE_KEY64().
 *
 * Calling/Exit State:
 *	This function returns an integer less than, equal to, or greater than
 *	zero if the first argument is respectively less than, equal to, or
 *	greater than the second.
 */
int
pfc_rbtree_int64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
#ifdef	PFC_LP64
	int64_t	v1 = (int64_t)(uintptr_t)arg1;
	int64_t	v2 = (int64_t)(uintptr_t)arg2;
#else	/* !PFC_LP64 */
	int64_t	*p1 = (int64_t *)arg1;
	int64_t	*p2 = (int64_t *)arg2;
	int64_t	v1 = *p1;
	int64_t	v2 = *p2;
#endif	/* PFC_LP64 */

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * int
 * pfc_rbtree_uint64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of unsigned 64-bit integer.
 *	Both arguments must be result of PFC_RBTREE_KEY64().
 *
 * Calling/Exit State:
 *	This function returns an integer less than, equal to, or greater than
 *	zero if the first argument is respectively less than, equal to, or
 *	greater than the second.
 */
int
pfc_rbtree_uint64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
#ifdef	PFC_LP64
	uint64_t	v1 = (uint64_t)(uintptr_t)arg1;
	uint64_t	v2 = (uint64_t)(uintptr_t)arg2;
#else	/* !PFC_LP64 */
	uint64_t	*p1 = (uint64_t *)arg1;
	uint64_t	*p2 = (uint64_t *)arg2;
	uint64_t	v1 = *p1;
	uint64_t	v2 = *p2;
#endif	/* PFC_LP64 */

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * static int
 * rbtree_insert(pfc_rbtree_t *PFC_RESTRICT tree,
 *		 pfc_rbnode_t *PFC_RESTRICT node,
 *		 pfc_rbnode_t **PFC_RESTRICT removed)
 *	Insert `node' to the Red-Black Tree.
 *
 *	If `removed' is not NULL, rbtree_insert() tries to remove node which
 *	has the same value as `node' before insertion. Removed node is set
 *	into `*removed'. NULL is set to `*removed' if no duplicate is found.
 *	If `removed' is NULL, insertion fails if the same node is already
 *	linked to the tree.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if `removed' is NULL and the tree contains a node
 *	which has the same value as `node'.
 */
static int
rbtree_insert(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbnode_t *PFC_RESTRICT node,
	      pfc_rbnode_t **PFC_RESTRICT removed)
{
	pfc_rbnode_t	*parent, *np;
	pfc_cptr_t	key;
	int	index;

	if (tree->rb_root == NULL) {
		/* Insert as root node. */
		RBNODE_INIT_BLACK(node, NULL);
		tree->rb_root = node;

		if (removed != NULL) {
			*removed = NULL;
		}

		return 0;
	}

	/* Search for a proper place to insert. */
	np = tree->rb_root;
	key = tree->rb_keyfunc(node);
	do {
		int	ret;

		parent = np;
		ret = tree->rb_comp(key, tree->rb_keyfunc(np));
		if (ret < 0) {
			index = PFC_RBNODE_IDX_LEFT;
			np = RBNODE_CHILD(np, index);
		}
		else if (ret > 0) {
			index = PFC_RBNODE_IDX_RIGHT;
			np = RBNODE_CHILD(np, index);
		}
		else if (removed != NULL) {
			/*
			 * Replace node. We can inherit color because node
			 * value is the same.
			 */
			*removed = np;
			rbtree_replace_node(tree, np, node);

			return 0;
		}
		else {
			return EEXIST;
		}
	} while (np != NULL);

	/* Insert new node as red. */
	RBNODE_CHILD(parent, index) = node;
	RBNODE_INIT_RED(node, parent);

	/* Rebalance the tree. */
	rbtree_fixup_insert(tree, node);

	if (removed != NULL) {
		*removed = NULL;
	}

	return 0;
}

/*
 * static void
 * rbtree_rotate_left(pfc_rbtree_t *PFC_RESTRICT tree,
 *		      pfc_rbnode_t *PFC_RESTRICT node)
 *	Rotate tree node to left.
 */
static void
rbtree_rotate_left(pfc_rbtree_t *PFC_RESTRICT tree,
		   pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*left, *right, *parent;

	right = RBNODE_RIGHT(node);
	left = RBNODE_LEFT(right);
	RBNODE_RIGHT(node) = left;
	if (left != NULL) {
		left->rbn_parent = node;
	}

	parent = node->rbn_parent;
	right->rbn_parent = parent;
	if (parent == NULL) {
		PFC_ASSERT(RBNODE_IS_ROOT(tree, node));
		tree->rb_root = right;
	}
	else if (RBNODE_LEFT(parent) == node) {
		RBNODE_LEFT(parent) = right;
	}
	else {
		PFC_ASSERT(RBNODE_RIGHT(parent) == node);
		RBNODE_RIGHT(parent) = right;
	}

	RBNODE_LEFT(right) = node;
	node->rbn_parent = right;
}

/*
 * static void
 * rbtree_rotate_right(pfc_rbtree_t *PFC_RESTRICT tree,
 *		       pfc_rbnode_t *PFC_RESTRICT node)
 *	Rotate tree node to right.
 */
static void
rbtree_rotate_right(pfc_rbtree_t *PFC_RESTRICT tree,
		    pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*left, *right, *parent;

	left = RBNODE_LEFT(node);
	right = RBNODE_RIGHT(left);
	RBNODE_LEFT(node) = right;
	if (right != NULL) {
		right->rbn_parent = node;
	}

	parent = node->rbn_parent;
	left->rbn_parent = parent;
	if (parent == NULL) {
		PFC_ASSERT(RBNODE_IS_ROOT(tree, node));
		tree->rb_root = left;
	}
	else if (RBNODE_RIGHT(parent) == node) {
		RBNODE_RIGHT(parent) = left;
	}
	else {
		PFC_ASSERT(RBNODE_LEFT(parent) == node);
		RBNODE_LEFT(parent) = left;
	}

	RBNODE_RIGHT(left) = node;
	node->rbn_parent = left;
}

/*
 * static pfc_rbnode_t *
 * rbtree_lookup_node(pfc_rbtree_t *PFC_RESTRICT tree,
 *		      pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the specified key is returned.
 *	NULL is returned if not found.
 */
static pfc_rbnode_t *
rbtree_lookup_node(pfc_rbtree_t *PFC_RESTRICT tree,
		   pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*np;

	np = tree->rb_root;
	while (np != NULL) {
		int	ret = tree->rb_comp(key, tree->rb_keyfunc(np));

		if (ret < 0) {
			np = RBNODE_LEFT(np);
		}
		else if (ret > 0) {
			np = RBNODE_RIGHT(np);
		}
		else {
			return np;
		}
	}

	return NULL;
}

/*
 * static void
 * rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
 *		      pfc_rbnode_t *PFC_RESTRICT node)
 *	Remove the specified node from the tree.
 */
static void
rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
		   pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*left = RBNODE_LEFT(node), *right = RBNODE_RIGHT(node);
	pfc_rbnode_t	*repl, *parent;
	int	index;

	if (left != NULL && right != NULL) {
		pfc_rbnode_t	*successor = left;

		/* Search for a successor of the node. */
		while (RBNODE_RIGHT(successor) != NULL) {
			successor = RBNODE_RIGHT(successor);
		}

		/*
		 * Swap the node with successor.
		 * Node color is bound to the node position. So the color of
		 * successor is copied to the node, and vice versa.
		 */
		rbtree_swap_node(tree, node, successor);
		left = RBNODE_LEFT(node);
		right = RBNODE_RIGHT(node);
	}

	PFC_ASSERT(left == NULL || right == NULL);
	repl = (left != NULL) ? left : right;
	parent = node->rbn_parent;

	if (repl != NULL) {
		/* Link replacement node to parent. */
		repl->rbn_parent = parent;
		if (parent == NULL) {
			PFC_ASSERT(RBNODE_IS_ROOT(tree, node));
			tree->rb_root = repl;
		}
		else {
			index = rbtree_child_index(parent, node);
			RBNODE_CHILD(parent, index) = repl;
		}

		if (RBNODE_IS_BLACK(node)) {
			rbtree_fixup_remove(tree, repl);
		}
	}
	else if (parent == NULL) {
		/* Root is removed. */
		PFC_ASSERT(RBNODE_IS_ROOT(tree, node));
		tree->rb_root = NULL;
	}
	else {
		/*
		 * The node to be removed has no child.
		 * It can be simply removed.
		 */
		if (RBNODE_IS_BLACK(node)) {
			rbtree_fixup_remove(tree, node);
			parent = node->rbn_parent;
			if (parent == NULL) {
				PFC_ASSERT(RBNODE_IS_ROOT(tree, node));
				return;
			}
		}

		index = rbtree_child_index(parent, node);
		RBNODE_CHILD(parent, index) = NULL;
	}
}

/*
 * static void
 * rbtree_replace_node(pfc_rbtree_t *PFC_RESTRICT tree,
 *		       pfc_rbnode_t *PFC_RESTRICT oldnode,
 *		       pfc_rbnode_t *PFC_RESTRICT newnode)
 *	Replace tree nodes.
 *	Node color is inherited from old node.
 */
static void
rbtree_replace_node(pfc_rbtree_t *PFC_RESTRICT tree,
		    pfc_rbnode_t *PFC_RESTRICT oldnode,
		    pfc_rbnode_t *PFC_RESTRICT newnode)
{
	pfc_rbnode_t	*parent = oldnode->rbn_parent;
	int	i;

	/* Inherit node color. */
	newnode->rbn_red = oldnode->rbn_red;

	/* Update parent link. */
	if (parent != NULL) {
		i = rbtree_child_index(parent, oldnode);
		RBNODE_CHILD(parent, i) = newnode;
	}
	else {
		PFC_ASSERT(RBNODE_IS_ROOT(tree, oldnode));
		tree->rb_root = newnode;
	}
	newnode->rbn_parent = parent;

	/* Update children links. */
	for (i = 0; i < PFC_RBNODE_NCHILDREN; i++) {
		pfc_rbnode_t	*c = RBNODE_CHILD(oldnode, i);

		if (c != NULL) {
			PFC_ASSERT(c->rbn_parent == oldnode);
			c->rbn_parent = newnode;
		}
		RBNODE_CHILD(newnode, i) = c;
	}
}

/*
 * static void
 * rbtree_swap_node(pfc_rbtree_t *PFC_RESTRICT tree,
 *		    pfc_rbnode_t *PFC_RESTRICT n1,
 *		    pfc_rbnode_t *PFC_RESTRICT n2)
 *	Swap position of two tree nodes.
 *	Note that node color is swapped.
 */
static void
rbtree_swap_node(pfc_rbtree_t *PFC_RESTRICT tree,
		 pfc_rbnode_t *PFC_RESTRICT n1,
		 pfc_rbnode_t *PFC_RESTRICT n2)
{
	pfc_rbnode_t	saved, *parent;
	int	i;

	if (n1->rbn_parent == n2) {
		rbtree_swap_parent(tree, n2, n1);

		return;
	}
	if (n2->rbn_parent == n1) {
		rbtree_swap_parent(tree, n1, n2);

		return;
	}

	/* Preserve attributes of n1. */
	saved = *n1;

	/* Replace n2 with n1. */
	rbtree_replace_node(tree, n2, n1);

	/* Copy n1 attributes to n2. */
	*n2 = saved;

	/* Update link for n2. */
	parent = n2->rbn_parent;
	if (parent != NULL) {
		i = rbtree_child_index(parent, n1);
		RBNODE_CHILD(parent, i) = n2;
	}
	else {
		PFC_ASSERT(RBNODE_IS_ROOT(tree, n1));
		tree->rb_root = n2;
	}

	/* Update children links. */
	for (i = 0; i < PFC_RBNODE_NCHILDREN; i++) {
		pfc_rbnode_t	*c = RBNODE_CHILD(n2, i);

		if (c != NULL) {
			PFC_ASSERT(c->rbn_parent == n1);
			c->rbn_parent = n2;
		}
	}
}

/*
 * static void
 * rbtree_swap_parent(pfc_rbtree_t *PFC_RESTRICT tree,
 *		      pfc_rbnode_t *PFC_RESTRICT parent,
 *		      pfc_rbnode_t *PFC_RESTRICT child)
 *	Swap position of parent and child nodes.
 *	Note that node color is swapped.
 */
static void
rbtree_swap_parent(pfc_rbtree_t *PFC_RESTRICT tree,
		   pfc_rbnode_t *PFC_RESTRICT parent,
		   pfc_rbnode_t *PFC_RESTRICT child)
{
	pfc_rbnode_t	*p;
	pfc_bool_t	color;
	int	i;

	PFC_ASSERT(child->rbn_parent == parent);

	/* Update parent link. */
	p = parent->rbn_parent;
	child->rbn_parent = p;
	parent->rbn_parent = child;
	if (p == NULL) {
		PFC_ASSERT(RBNODE_IS_ROOT(tree, parent));
		tree->rb_root = child;
	}
	else {
		i = rbtree_child_index(p, parent);
		RBNODE_CHILD(p, i) = child;
	}

	/* Update children links. */
	for (i = 0; i < PFC_RBNODE_NCHILDREN; i++) {
		pfc_rbnode_t	*pc = RBNODE_CHILD(parent, i);
		pfc_rbnode_t	*cc = RBNODE_CHILD(child, i);

		if (pc == child) {
			RBNODE_CHILD(child, i) = parent;
		}
		else {
			RBNODE_CHILD(child, i) = pc;
			if (pc != NULL) {
				pc->rbn_parent = child;
			}
		}

		RBNODE_CHILD(parent, i) = cc;
		if (cc != NULL) {
			cc->rbn_parent = parent;
		}
	}

	/* Swap node color. */
	color = parent->rbn_red;
	parent->rbn_red = child->rbn_red;
	child->rbn_red = color;
}

/*
 * static void
 * rbtree_fixup_insert(pfc_rbtree_t *PFC_RESTRICT tree,
 *		       pfc_rbnode_t *PFC_RESTRICT node)
 *	Rebalance tree nodes after insertion of new node.
 */
static void
rbtree_fixup_insert(pfc_rbtree_t *PFC_RESTRICT tree,
		    pfc_rbnode_t *PFC_RESTRICT node)
{
	while (node != NULL && !RBNODE_IS_ROOT(tree, node) &&
	       RBNODE_IS_RED(node->rbn_parent)) {
		const rbtree_ops_t	*ops;
		pfc_rbnode_t	*parent = node->rbn_parent;
		pfc_rbnode_t	*grand, *uncle;
		int	index, inner;

		/* Parent is red, and red node must have parent. */
		grand = parent->rbn_parent;
		PFC_ASSERT(grand != NULL);

		/* Determine position where a new node is inserted. */
		index = rbtree_child_index(grand, parent);
		ops = &rbtree_ops[index];
		inner = ops->rbops_inner_idx;
		uncle = RBNODE_CHILD(grand, inner);
		if (uncle != NULL && RBNODE_IS_RED(uncle)) {
			RBNODE_SET_BLACK(parent);
			RBNODE_SET_BLACK(uncle);
			RBNODE_SET_RED(grand);

			/* More rebalancing is needed from grand parent. */
			node = grand;
			continue;
		}

		if (RBNODE_CHILD(parent, inner) == node) {
			/* Rotate tree nodes to outside. */
			node = parent;
			ops->rbops_rotate_outside(tree, node);
			parent = node->rbn_parent;
			if (parent == NULL) {
				continue;
			}
			RBNODE_SET_BLACK(parent);

			grand = parent->rbn_parent;
			if (grand == NULL) {
				continue;
			}
		}
		else {
			RBNODE_SET_BLACK(parent);
		}
		RBNODE_SET_RED(grand);

		/* Rotate tree nodes to inside. */
		ops->rbops_rotate_inside(tree, grand);
	}

	/* Root is always black. */
	RBNODE_SET_BLACK(tree->rb_root);
}

/*
 * static void
 * rbtree_fixup_insert(pfc_rbtree_t *PFC_RESTRICT tree,
 *		       pfc_rbnode_t *PFC_RESTRICT node)
 *	Rebalance tree nodes after deletion of the node.
 */
static void
rbtree_fixup_remove(pfc_rbtree_t *PFC_RESTRICT tree,
		    pfc_rbnode_t *PFC_RESTRICT node)
{
	while (!RBNODE_IS_ROOT(tree, node) && RBNODE_IS_BLACK(node)) {
		const rbtree_ops_t	*ops;
		pfc_rbnode_t	*parent = node->rbn_parent, *sibling;
		pfc_rbnode_t	*sinner, *souter;
		int	index, inner, outer;

		PFC_ASSERT(parent != NULL);

		/* Determine which side the node is contained. */
		index = rbtree_child_index(parent, node);
		ops = &rbtree_ops[index];
		inner = ops->rbops_inner_idx;
		outer = ops->rbops_outer_idx;

		sibling = RBNODE_CHILD(parent, inner);
		if (sibling == NULL) {
			node = parent;
			continue;
		}

		if (RBNODE_IS_RED(sibling)) {
			/*
			 * Sibling node is red. So the parent and children of
			 * sibling are black. In this case, color of sibling
			 * and parent node needs to be swapped, and parent
			 * subtree needs to be rotated to outside.
			 */
			PFC_ASSERT(RBNODE_IS_BLACK(parent));
			RBNODE_BLACK_ASSERT(RBNODE_LEFT(sibling));
			RBNODE_BLACK_ASSERT(RBNODE_RIGHT(sibling));

			RBNODE_SET_BLACK(sibling);
			RBNODE_SET_RED(parent);
			ops->rbops_rotate_outside(tree, parent);
			parent = node->rbn_parent;
			sibling = RBNODE_CHILD(parent, inner);
			if (sibling == NULL) {
				node = parent;
				continue;
			}
		}

		sinner = RBNODE_CHILD(sibling, inner);
		souter = RBNODE_CHILD(sibling, outer);

		if (sinner == NULL || RBNODE_IS_BLACK(sinner)) {
			if (souter == NULL || RBNODE_IS_BLACK(souter)) {
				/*
				 * Both children of sibling are black.
				 * In this case, sibling must be turned to red.
				 * If parent is red, more rebalance is needed.
				 */
				RBNODE_SET_RED(sibling);
				node = parent;
				continue;
			}

			/*
			 * Outer children of sibling must be black, and sibling
			 * must be red, and subtree of sibling must be rotated
			 * to inside.
			 */
			RBNODE_SET_BLACK(souter);
			RBNODE_SET_RED(sibling);
			ops->rbops_rotate_inside(tree, sibling);

			PFC_ASSERT(RBNODE_CHILD(parent, inner) == souter);
			sibling = souter;
			sinner = RBNODE_CHILD(sibling, inner);
		}

		/* Swap color of parent and sibling. */
		PFC_ASSERT(parent == node->rbn_parent);
		PFC_ASSERT(RBNODE_IS_BLACK(sibling));
		sibling->rbn_red = parent->rbn_red;
		RBNODE_SET_BLACK(parent);

		/* Turn inner child of sibling to black. */
		if (sinner != NULL) {
			RBNODE_SET_BLACK(sinner);
		}

		/* Rotate parent subtree to outside. */
		ops->rbops_rotate_outside(tree, parent);

		/* Ensure that the root node is black. */
		node = tree->rb_root;
		break;
	}

	RBNODE_SET_BLACK(node);
}

/*
 * static pfc_rbnode_t *
 * rbtree_first(pfc_rbtree_t *tree)
 *	Return the first tree node, which is the least node in the tree.
 *	NULL is returned if the tree is empty.
 */
static pfc_rbnode_t *
rbtree_first(pfc_rbtree_t *tree)
{
	pfc_rbnode_t	*np;

	np = tree->rb_root;
	if (np != NULL) {
		while (RBNODE_LEFT(np) != NULL) {
			np = RBNODE_LEFT(np);
		}
	}

	return np;
}

/*
 * static pfc_rbnode_t *
 * rbtree_first_leaf(pfc_rbnode_t *np)
 *	Return the first leaf node in the subtree, which is the least node
 *	and has no child.
 *
 *	`np' must not be NULL, so this function should never return NULL.
 */
static pfc_rbnode_t *
rbtree_first_leaf(pfc_rbnode_t *np)
{
	for (;;) {
		while (RBNODE_LEFT(np) != NULL) {
			np = RBNODE_LEFT(np);
		}
		if (RBNODE_RIGHT(np) == NULL) {
			break;
		}
		np = RBNODE_RIGHT(np);
	}

	return np;
}

/*
 * static pfc_rbnode_t *
 * rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbnode_t *PFC_RESTRICT node)
 *	Search for a node which has the least key greater than the specified
 *	node key.
 *	NULL is returned if not found.
 */
static pfc_rbnode_t *
rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*np, *child;

	np = RBNODE_RIGHT(node);
	if (np != NULL) {
		/* Search for the least node in the right subtree. */
		while (RBNODE_LEFT(np) != NULL) {
			np = RBNODE_LEFT(np);
		}

		return np;
	}

	/*
	 * The specified node is the least node in the subtree.
	 * Look for the ancestor which is linked to right side of its parent.
	 */
	child = node;
	np = node->rbn_parent;
	while (np != NULL && RBNODE_RIGHT(np) == child) {
		child = np;
		np = np->rbn_parent;
	}

	return np;
}
