/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_RBTREE_EX_H
#define	_PFC_RBTREE_EX_H

/*
 * Extended Red-Black Tree, which has read-write lock and node counter.
 *
 * Remarks:
 *	Members of any structure defined in this file are public just for
 *	optimization. The user must not access them directly.
 */

#include <pfc/rbtree.h>
#include <pfc/debug.h>
#include <pfc/synch.h>

PFC_C_BEGIN_DECL

/*
 * Root of extended Red-Black Tree.
 */
typedef struct {
	pfc_rbtree_t	rbx_tree;	/* tree root */
	pfc_rwlock_t	rbx_lock;	/* read-write lock for the tree */
	size_t		rbx_count;	/* node counter */
} pfc_rbtree_ex_t;

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_init(pfc_rbtree_ex_t *PFC_RESTRICT extree, pfc_rbcomp_t comp,
 *		      pfc_rbkeyfunc_t keyfunc)
 *	Initialize extended Red-Black tree.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_init(pfc_rbtree_ex_t *PFC_RESTRICT extree, pfc_rbcomp_t comp,
		   pfc_rbkeyfunc_t keyfunc)
{
	pfc_rbtree_init(&extree->rbx_tree, comp, keyfunc);
	PFC_ASSERT_INT(pfc_rwlock_init(&extree->rbx_lock), 0);
	extree->rbx_count = 0;
}

/*
 * Static initializer of extended Red-Black Tree.
 */
#define	PFC_RBTREE_EX_INITIALIZER(comp, keyfunc)			\
	{								\
		PFC_RBTREE_INITIALIZER(comp, keyfunc),	/* rbx_tree */	\
		PFC_RWLOCK_INITIALIZER,			/* rbx_lock */	\
		0,					/* rbx_count */	\
	}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_rdlock(pfc_rbtree_ex_t *extree)
 *	Acquire reader lock for the specified extended Red-Black Tree.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_rdlock(pfc_rbtree_ex_t *extree)
{
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&extree->rbx_lock), 0);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_wrlock(pfc_rbtree_ex_t *extree)
 *	Acquire writer lock for the specified extended Red-Black Tree.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_wrlock(pfc_rbtree_ex_t *extree)
{
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&extree->rbx_lock), 0);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_unlock(pfc_rbtree_ex_t *extree)
 *	Release reader or writer lock for the specified extended Red-Black
 *	Tree.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_unlock(pfc_rbtree_ex_t *extree)
{
	PFC_ASSERT_INT(pfc_rwlock_unlock(&extree->rbx_lock), 0);
}

/*
 * static inline pfc_rwlock_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_lock(pfc_rbtree_ex_t *extree)
 *	Return a pointer to the read-write lock associated with the specified
 *	extended Red-Black Tree.
 */
static inline pfc_rwlock_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_lock(pfc_rbtree_ex_t *extree)
{
	return &extree->rbx_lock;
}

/*
 * Wrapper APIs which require the caller to hold the tree lock.
 */

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_put_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *		       pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert a tree node specified by `node' to the extended Red-Black Tree
 *	specified by `extree'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if the tree contains a node which has the same
 *	value as `node'.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the tree lock.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_put_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		    pfc_rbnode_t *PFC_RESTRICT node)
{
	int	err = pfc_rbtree_put(&extree->rbx_tree, node);

	if (PFC_EXPECT_TRUE(err == 0)) {
		extree->rbx_count++;
	}

	return err;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_update_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			  pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert a tree node specified by `node' to the extended Red-Black Tree
 *	specified by `extree'.
 *
 *	If the tree contains a node which has the same value as `node',
 *	it is replaced with the specified node.
 *
 * Calling/Exit State:
 *	The previous node which has the same key as `node' is returned.
 *	NULL is returned if the tree doesn't contain a node which has the
 *	same key as `node'.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_update_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		       pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*old = pfc_rbtree_update(&extree->rbx_tree, node);

	if (old == NULL) {
		/* A new tree node has been inserted. */
		extree->rbx_count++;
	}

	return old;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *		       pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the specified key is returned.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		    pfc_cptr_t PFC_RESTRICT key)
{
	return pfc_rbtree_get(&extree->rbx_tree, key);
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_ceil_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			    pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the least key greater than or equal to
 *	the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the least key greater than or equal to
 *	the specified key.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_ceil_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
			 pfc_cptr_t PFC_RESTRICT key)
{
	return pfc_rbtree_get_ceil(&extree->rbx_tree, key);
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_floor_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			     pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the greatest key less than or equal
 *	to the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the greatest key less than or equal
 *	to the specified key.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_floor_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
			  pfc_cptr_t PFC_RESTRICT key)
{
	return pfc_rbtree_get_floor(&extree->rbx_tree, key);
}

/*
 * static inline pfc_rbnode_t *
 * pfc_rbtree_ex_remove_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			  pfc_cptr_t PFC_RESTRICT key)
 *	Remove the tree node which has the key specified by `key'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to removed tree node is returned.
 *	NULL is returned if the specified key is not found in the tree.
 *
 * Remarks:
 *	This function must be called with holding writer lock of the tree lock.
 */
static inline pfc_rbnode_t *
pfc_rbtree_ex_remove_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		       pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node = pfc_rbtree_remove(&extree->rbx_tree, key);

	if (PFC_EXPECT_FALSE(node != NULL)) {
		extree->rbx_count--;
	}

	return node;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_remove_node_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			       pfc_rbnode_t *PFC_RESTRICT node)
 *	Remove the specified tree node from the tree.
 *
 * Remarks:
 *	- This function must be called with holding writer lock of the tree
 *	  lock.
 *
 *	- The must guarantee that the node specified by `node' exists in the
 *	  tree specified by `extree'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_remove_node_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
			    pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbtree_remove_node(&extree->rbx_tree, node);
	extree->rbx_count--;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_next_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			pfc_rbnode_t *PFC_RESTRICT node)
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
 *	- The caller must guarantee that the tree is never changed during
 *	  sequence of the pfc_rbtree_next() call. Tree update during sequence of
 *	  the pfc_rbtree_next() call causes undefined behavior.
 *
 *	- This function must be called with holding reader or writer lock of
 *	  the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_next_l(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		     pfc_rbnode_t *PFC_RESTRICT node)
{
	return pfc_rbtree_next(&extree->rbx_tree, node);
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_clear_l(pfc_rbtree_ex_t *extree,
 *			 pfc_rbcallback_t dtor, pfc_ptr_t arg)
 *	Clear Red-Black tree.
 *	The tree will be empty after this function call returns.
 *
 *	All nodes in the tree pointed by `tree' is removed.
 *	if `dtor' is not NULL, it is called with specifying each tree node
 *	removed by this function. `arg' is passed to the destructor.
 *
 * Remarks:
 *	- This function must be called with holding writer lock of the tree
 *	  lock.
 *
 *	- Note that the specified destructor is called with holding the lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_clear_l(pfc_rbtree_ex_t *extree,
		      pfc_rbcallback_t dtor, pfc_ptr_t arg)
{
	pfc_rbtree_clear(&extree->rbx_tree, dtor, arg);
	extree->rbx_count = 0;
}

/*
 * static inline size_t PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_get_size_l(pfc_rbtree_ex_t *extree)
 *	Return the number of tree nodes in the extended Red-Black Tree
 *	specified `extree'.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the tree lock.
 */
static inline size_t PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_get_size_l(pfc_rbtree_ex_t *extree)
{
	return extree->rbx_count;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_isempty_l(pfc_rbtree_ex_t *extree)
 *	Return PFC_TRUE if no tree node exists in the specified tree.
 *
 * Remarks:
 *	This function must be called with holding reader or writer lock of
 *	the tree lock.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_isempty_l(pfc_rbtree_ex_t *extree)
{
	return pfc_rbtree_isempty(&extree->rbx_tree);
}

/*
 * Wrapper APIs which holds the tree lock internally.
 */

/*
 * static inline int PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_put(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *		       pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert a tree node specified by `node' to the extended Red-Black Tree
 *	specified by `extree'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	EEXIST is returned if the tree contains a node which has the same
 *	value as `node'.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline int PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_put(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		  pfc_rbnode_t *PFC_RESTRICT node)
{
	int	err;

	pfc_rbtree_ex_wrlock(extree);
	err = pfc_rbtree_ex_put_l(extree ,node);
	pfc_rbtree_ex_unlock(extree);

	return err;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_update(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			pfc_rbnode_t *PFC_RESTRICT node)
 *	Insert a tree node specified by `node' to the extended Red-Black Tree
 *	specified by `extree'.
 *
 *	If the tree contains a node which has the same value as `node',
 *	it is replaced with the specified node.
 *
 * Calling/Exit State:
 *	The previous node which has the same key as `node' is returned.
 *	NULL is returned if the tree doesn't contain a node which has the
 *	same key as `node'.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_update(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		     pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*old;

	pfc_rbtree_ex_wrlock(extree);
	old = pfc_rbtree_ex_update_l(extree, node);
	pfc_rbtree_ex_unlock(extree);

	return old;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *		     pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the specified key is returned.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		  pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node;

	pfc_rbtree_ex_rdlock(extree);
	node = pfc_rbtree_ex_get_l(extree, key);
	pfc_rbtree_ex_unlock(extree);

	return node;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_ceil(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			  pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the least key greater than or equal to
 *	the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the least key greater than or equal to
 *	the specified key.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_ceil(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		       pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node;

	pfc_rbtree_ex_rdlock(extree);
	node = pfc_rbtree_ex_get_ceil_l(extree, key);
	pfc_rbtree_ex_unlock(extree);

	return node;
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_get_floor(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			   pfc_cptr_t PFC_RESTRICT key)
 *	Returns the tree node which has the greatest key less than or equal
 *	to the specified key.
 *
 * Calling/Exit State:
 *	A pointer to tree node which has the greatest key less than or equal
 *	to the specified key.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_get_floor(pfc_rbtree_ex_t *PFC_RESTRICT extree,
			pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node;

	pfc_rbtree_ex_rdlock(extree);
	node = pfc_rbtree_ex_get_floor_l(extree, key);
	pfc_rbtree_ex_unlock(extree);

	return node;
}

/*
 * static inline pfc_rbnode_t *
 * pfc_rbtree_ex_remove(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			pfc_cptr_t PFC_RESTRICT key)
 *	Remove the tree node which has the key specified by `key'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to removed tree node is returned.
 *	NULL is returned if the specified key is not found in the tree.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_rbnode_t *
pfc_rbtree_ex_remove(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		     pfc_cptr_t PFC_RESTRICT key)
{
	pfc_rbnode_t	*node;

	pfc_rbtree_ex_wrlock(extree);
	node = pfc_rbtree_ex_remove_l(extree, key);
	pfc_rbtree_ex_unlock(extree);

	return node;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_remove_node(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *			     pfc_rbnode_t *PFC_RESTRICT node)
 *	Remove the specified tree node from the tree.
 *
 * Remarks:
 *	- This function must be called without holding the tree lock.
 *
 *	- The must guarantee that the node specified by `node' exists in the
 *	  tree specified by `extree'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_remove_node(pfc_rbtree_ex_t *PFC_RESTRICT extree,
			  pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbtree_ex_wrlock(extree);
	pfc_rbtree_ex_remove_node_l(extree, node);
	pfc_rbtree_ex_unlock(extree);
}

/*
 * static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
 * pfc_rbtree_ex_next(pfc_rbtree_ex_t *PFC_RESTRICT extree,
 *		      pfc_rbnode_t *PFC_RESTRICT node)
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
 *	- This function must be called without holding the tree lock.
 *
 *	- If you want to iterate tree nodes in the tree, you must use
 *	  pfc_rbtree_ex_next_l() with holding the tree lock.
 */
static inline pfc_rbnode_t PFC_FATTR_ALWAYS_INLINE *
pfc_rbtree_ex_next(pfc_rbtree_ex_t *PFC_RESTRICT extree,
		   pfc_rbnode_t *PFC_RESTRICT node)
{
	pfc_rbnode_t	*next;

	pfc_rbtree_ex_rdlock(extree);
	next = pfc_rbtree_next(&extree->rbx_tree, node);
	pfc_rbtree_ex_unlock(extree);

	return next;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_clear(pfc_rbtree_ex_t *extree,
 *		       pfc_rbcallback_t dtor, pfc_ptr_t arg)
 *	Clear Red-Black tree.
 *	The tree will be empty after this function call returns.
 *
 *	All nodes in the tree pointed by `tree' is removed.
 *	if `dtor' is not NULL, it is called with specifying each tree node
 *	removed by this function. `arg' is passed to the destructor.
 *
 * Remarks:
 *	- This function must be called without holding the tree lock.
 *
 *	- Note that the specified destructor is called with holding writer lock
 *	  of the tree lock.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_clear(pfc_rbtree_ex_t *extree,
		    pfc_rbcallback_t dtor, pfc_ptr_t arg)
{
	pfc_rbtree_ex_wrlock(extree);
	pfc_rbtree_ex_clear_l(extree, dtor, arg);
	pfc_rbtree_ex_unlock(extree);
}

/*
 * static inline size_t PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_get_size(pfc_rbtree_ex_t *extree)
 *	Return the number of tree nodes in the extended Red-Black Tree
 *	specified `extree'.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline size_t PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_get_size(pfc_rbtree_ex_t *extree)
{
	size_t	size;

	pfc_rbtree_ex_rdlock(extree);
	size = pfc_rbtree_ex_get_size_l(extree);
	pfc_rbtree_ex_unlock(extree);

	return size;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_ex_isempty(pfc_rbtree_ex_t *extree)
 *	Return PFC_TRUE if no tree node exists in the specified tree.
 *
 * Remarks:
 *	This function must be called without holding the tree lock.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_ex_isempty(pfc_rbtree_ex_t *extree)
{
	pfc_bool_t	ret;

	pfc_rbtree_ex_rdlock(extree);
	ret = pfc_rbtree_ex_isempty_l(extree);
	pfc_rbtree_ex_unlock(extree);

	return ret;
}

PFC_C_END_DECL

#endif	/* _PFC_RBTREE_EX_H */
