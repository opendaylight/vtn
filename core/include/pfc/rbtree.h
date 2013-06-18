/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_RBTREE_H
#define	_PFC_RBTREE_H

/*
 * Definitions for Red-Black Tree implementation.
 *
 * Remarks:
 *	- This implementation is not synchronized. Serialization of tree
 *	  maintenance must be implemented by user.
 *
 *	- Members of any structure defined in this file are public just for
 *	  optimization. The user must not access them directly.
 */

#include <errno.h>
#include <pfc/base.h>
#include <pfc/debug.h>

PFC_C_BEGIN_DECL

/* Indices of children node. */
#define	PFC_RBNODE_IDX_LEFT	0
#define	PFC_RBNODE_IDX_RIGHT	1
#define	PFC_RBNODE_NCHILDREN	2

/*
 * Tree node.
 * pfc_rbnode_t is designed to be embedded in another struct.
 * PFC_CAST_CONTAINER() is useful to convert a pointer to pfc_rbnode_t to
 * a pointer to struct which contains pfc_rbnode_t.
 */
struct pfc_rbnode;
typedef struct pfc_rbnode	pfc_rbnode_t;

struct pfc_rbnode {
	pfc_rbnode_t	*rbn_parent;		/* parent node */
	pfc_rbnode_t	*rbn_children[PFC_RBNODE_NCHILDREN];	/* children */
	pfc_bool_t	rbn_red;		/* True if this node is red */

	/*
	 * Define padding explicitly.
	 *
	 * Remarks:
	 *	Don't change layout of pfc_rbnode_t because libpfc_license
	 *	uses this padding.
	 */
	char		rbn_pad[sizeof(void *) - 1];
};

/*
 * Function which returns key value associated with the specified node.
 */
typedef pfc_cptr_t	(*pfc_rbkeyfunc_t)(pfc_rbnode_t *node);

/*
 * Tree node key comparator.
 * Returned value is a signed integer, just like qsort(3) comparator.
 * The comparator must return an integer less than, equal to, or greater than
 * zero if the first argument is considered to be respectively less than,
 * equal to, or greater than the second.
 */
typedef int	(*pfc_rbcomp_t)(pfc_cptr_t arg1, pfc_cptr_t arg2);

/*
 * Tree node callback function.
 */
typedef void	(*pfc_rbcallback_t)(pfc_rbnode_t *node, pfc_ptr_t arg);

/*
 * Root of Red-Black Tree.
 */
typedef struct {
	pfc_rbnode_t	*rb_root;		/* root node */
	pfc_rbcomp_t	rb_comp;		/* comparator */
	pfc_rbkeyfunc_t	rb_keyfunc;		/* function to obtain key */
} pfc_rbtree_t;

/*
 * Prototypes.
 */
extern int		pfc_rbtree_put(pfc_rbtree_t *PFC_RESTRICT tree,
				       pfc_rbnode_t *PFC_RESTRICT node);
extern pfc_rbnode_t	*pfc_rbtree_update(pfc_rbtree_t *PFC_RESTRICT tree,
					   pfc_rbnode_t *PFC_RESTRICT node);
extern pfc_rbnode_t	*pfc_rbtree_get(pfc_rbtree_t *PFC_RESTRICT tree,
					pfc_cptr_t PFC_RESTRICT key);
extern pfc_rbnode_t	*pfc_rbtree_get_ceil(pfc_rbtree_t *PFC_RESTRICT tree,
					     pfc_cptr_t PFC_RESTRICT key);
extern pfc_rbnode_t	*pfc_rbtree_get_floor(pfc_rbtree_t *PFC_RESTRICT tree,
					      pfc_cptr_t PFC_RESTRICT key);
extern pfc_rbnode_t	*pfc_rbtree_remove(pfc_rbtree_t *PFC_RESTRICT tree,
					   pfc_cptr_t PFC_RESTRICT key);
extern void		pfc_rbtree_remove_node(pfc_rbtree_t *PFC_RESTRICT tree,
					       pfc_rbnode_t *PFC_RESTRICT node);
extern pfc_rbnode_t	*pfc_rbtree_next(pfc_rbtree_t *PFC_RESTRICT tree,
					 pfc_rbnode_t *PFC_RESTRICT node);
extern void		pfc_rbtree_clear(pfc_rbtree_t *tree,
					 pfc_rbcallback_t dtor, pfc_ptr_t arg);

/*
 * Built-in comparators.
 */
extern int	pfc_rbtree_int32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
extern int	pfc_rbtree_uint32_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
extern int	pfc_rbtree_int64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
extern int	pfc_rbtree_uint64_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);

#ifdef	PFC_LP64
#define	pfc_rbtree_long_compare		pfc_rbtree_int64_compare
#define	pfc_rbtree_ulong_compare	pfc_rbtree_uint64_compare
#else	/* !PFC_LP64 */
#define	pfc_rbtree_long_compare		pfc_rbtree_int32_compare
#define	pfc_rbtree_ulong_compare	pfc_rbtree_uint32_compare
#endif	/* PFC_LP64 */

/*
 * PFC_RBTREE_KEY64(variable)
 *	Create 64-bit integer key for Red-Black Tree.
 *
 *	This macro is provided to keep source compatibility between LP64 and
 *	ILP32 system. On LP64 system, PFC_RBTREE_KEY64() is simply evaluated
 *	as the given argument. On ILP32 system, it is evaluated as a pointer
 *	to the given argument. So an argument must be a 64-bit integer
 *	variable, not integer constant.
 *
 *	If you want to use pfc_rbtree_int64_compare() or
 *	pfc_rbtree_uint64_compare() as Red-Black Tree comparator,
 *	pfc_rbkeyfunc_t must return result of PFC_RBTREE_KEY64().
 *	In addition, result of PFC_RBTREE_KEY64() must be specified to
 *	Red-Black Tree APIs, such as pfc_rbtree_get(),  as node key.
 */
#ifdef	PFC_LP64
#define	PFC_RBTREE_KEY64(variable)				\
	(PFC_ASSERT(sizeof(variable) == sizeof(int64_t)),	\
	 ((pfc_cptr_t)(uintptr_t)(variable)))
#else	/* !PFC_LP64 */
#define	PFC_RBTREE_KEY64(variable)				\
	(PFC_ASSERT(sizeof(variable) == sizeof(int64_t)),	\
	 ((pfc_cptr_t)&(variable)))
#endif	/* PFC_LP64 */

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_init(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbcomp_t comp,
 *		   pfc_rbkeyfunc_t keyfunc)
 *	Initialize Red-Black tree.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_init(pfc_rbtree_t *PFC_RESTRICT tree, pfc_rbcomp_t comp,
		pfc_rbkeyfunc_t keyfunc)
{
	tree->rb_root = NULL;
	tree->rb_comp = comp;
	tree->rb_keyfunc = keyfunc;
}

/*
 * Static initializer of Red-Black tree.
 */
#define	PFC_RBTREE_INITIALIZER(comp, keyfunc)		\
	{						\
		NULL,		/* rb_root */		\
		(comp),		/* rb_comp */		\
		(keyfunc)	/* rb_keyfunc */	\
	}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pfc_rbtree_isempty(pfc_rbtree_t *tree)
 *	Return PFC_TRUE if no tree node exists in the specified tree.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pfc_rbtree_isempty(pfc_rbtree_t *tree)
{
	return (tree->rb_root == NULL) ? PFC_TRUE : PFC_FALSE;
}

PFC_C_END_DECL

#endif	/* !_PFC_RBTREE_H */
