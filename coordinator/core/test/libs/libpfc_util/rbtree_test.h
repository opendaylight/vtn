/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Common definitions for Red-Black Tree tests.
 */

#include <pfc/rbtree.h>
#include <pfc/list.h>

#ifndef	_TEST_RBTREE_TEST_H
#define	_TEST_RBTREE_TEST_H

PFC_C_BEGIN_DECL

/*
 * Tree node for test.
 */
typedef struct {
	uint32_t	rt_value;		/* node value */
	uint32_t	rt_seq;			/* sequential ID from zero */
	pfc_list_t	rt_link;		/* link for node list */
	pfc_rbnode_t	rt_node;		/* tree node instance */
} rbtest_t;

/*
 * Convert pointer to tree node to pointer to rbtest_t.
 */
#define	RBTEST_NODE2PTR(node)	PFC_CAST_CONTAINER((node), rbtest_t, rt_node)

#ifndef	__cplusplus

/*
 * Convert pointer to list element to pointer to rbtest_t.
 */
#define	RBTEST_LIST2PTR(elem)	PFC_CAST_CONTAINER((elem), rbtest_t, rt_link)

#endif	/* !__cplusplus */

/*
 * Convert pointer to rbtest_t to tree node pointer.
 */
#define	RBTEST_PTR2NODE(rp)	(&(rp)->rt_node)

/*
 * Return node value as unsigned 32-bit integer.
 */
#define	RBTEST_GETKEY(node)	((uint32_t)(uintptr_t)rbtest_getkey(node))

/*
 * Prototypes.
 */
extern pfc_cptr_t	rbtest_getkey(pfc_rbnode_t *node);
extern int		rbtest_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
extern rbtest_t		*rbtest_alloc(pfc_list_t *head, uint32_t value);
extern void		rbtest_free(rbtest_t *rp);
extern void		rbtest_node_free(pfc_rbnode_t *node);
extern void		rbtest_cleanup(pfc_list_t *head);

PFC_C_END_DECL

#endif	/* !_TEST_RBTREE_TEST_H */
