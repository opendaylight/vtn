/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * rbtree_subr.c: Utilities for Red-Black Tree tests.
 */

#include "rbtree_test.h"

/*
 * pfc_cptr_t
 * rbtest_getkey(pfc_rbnode_t *node)
 *	Return key of the specified tree node.
 */
pfc_cptr_t
rbtest_getkey(pfc_rbnode_t *node)
{
	rbtest_t	*rp = RBTEST_NODE2PTR(node);

	return (pfc_cptr_t)(uintptr_t)rp->rt_value;
}

/*
 * int
 * rbtest_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of rbtest_t.
 */
int
rbtest_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
	uint32_t	v1 = (uint32_t)(uintptr_t)arg1;
	uint32_t	v2 = (uint32_t)(uintptr_t)arg2;

	if (v1 == v2) {
		return 0;
	}

	return (v1 < v2) ? -1 : 1;
}

/*
 * rbtest_t *
 * rbtest_alloc(pfc_list_t *head, uint32_t value)
 *	Allocate a tree node that has the specified node value.
 */
rbtest_t *
rbtest_alloc(pfc_list_t *head, uint32_t value)
{
	rbtest_t	*rp;

	rp = (rbtest_t *)malloc(sizeof(*rp));
	if (PFC_EXPECT_TRUE(rp != NULL)) {
		rp->rt_value = value;
		rp->rt_seq = 0;
		pfc_list_push_tail(head, &rp->rt_link);
	}

	return rp;
}

/*
 * void
 * rbtest_free(rbtest_t *rp)
 *	Free the specified tree node.
 */
void
rbtest_free(rbtest_t *rp)
{
	pfc_list_remove(&rp->rt_link);
	free(rp);
}

/*
 * void
 * rbtest_node_free(pfc_rbnode_t *node)
 *	Free the specified Red-Black Tree node.
 */
void
rbtest_node_free(pfc_rbnode_t *node)
{
	rbtest_t	*rp = RBTEST_NODE2PTR(node);

	rbtest_free(rp);
}

/*
 * void
 * rbtest_cleanup(pfc_list_t *head)
 *	Clean up all tree nodes linked to the specified list head.
 */
void
rbtest_cleanup(pfc_list_t *head)
{
	pfc_list_t	*elem;

	while ((elem = pfc_list_pop(head)) != NULL) {
		rbtest_t	*rp = RBTEST_LIST2PTR(elem);

		free(rp);
	}
}
