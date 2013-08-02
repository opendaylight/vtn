/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * property.c - PFC daemon system property.
 */

#include <string.h>
#include <pfc/rbtree.h>
#include <pfc/util.h>
#include <pfc/log.h>
#include <pfc/debug.h>
#include "property_impl.h"

/*
 * Property node.
 */
typedef struct {
	const char	*p_key;		/* key string */
	const char	*p_value;	/* value */
	pfc_rbnode_t	p_node;		/* Red-Black tree node */
	char		p_buffer[0];	/* buffer for key and value */
} propent_t;

#define	PROP_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), propent_t, p_node)

#define	PROP_ENTSIZE(klen, vlen)			\
	(sizeof(propent_t) + (klen) + 1 + (vlen) + 1)

/*
 * Internal prototypes.
 */
static pfc_cptr_t	prop_getkey(pfc_rbnode_t *node);
static void		prop_dtor(pfc_rbnode_t *node, pfc_ptr_t arg);

/*
 * Red-Black tree which keeps pairs of property key and value.
 * Currently, the PFC system property is read-only.
 * So this tree is not synchronized.
 */
static pfc_rbtree_t	prop_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, prop_getkey);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * prop_setstring(char *dst, const char *str, size_t len)
 *	Copy string to the buffer pointed by `dst'.
 *
 *	`len' must be the length of the string specified by `str'.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
prop_setstring(char *dst, const char *str, size_t len)
{
#ifdef	PFC_VERBOSE_DEBUG
	size_t	sz = pfc_strlcpy(dst, str, len + 1);

	PFC_ASSERT(sz == len);
#else	/* !PFC_VERBOSE_DEBUG */
	(void)pfc_strlcpy(dst, str, len + 1);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * const char *
 * pfc_prop_get(const char *key)
 *	Return the value associated with the given key in the PFC daemon
 *	system property.
 *
 * Calling/Exit State:
 *	A non-NULL pointer to value string is returned if found.
 *	NULL is returned if not found.
 */
const char *
pfc_prop_get(const char *key)
{
	pfc_rbnode_t	*node = pfc_rbtree_get(&prop_tree, key);

	if (PFC_EXPECT_TRUE(node != NULL)) {
		propent_t	*pep = PROP_NODE2PTR(node);

		return pep->p_value;
	}

	return NULL;
}

/*
 * int
 * pfc_prop_set(const char *key, const char *value)
 *	Associate the key string specified by `key' with the value string
 *	specified by `value' in the PFC daemon system property.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This is not a public interface.
 */
int
pfc_prop_set(const char *key, const char *value)
{
	size_t		klen = strlen(key);
	size_t		vlen = strlen(value);
	size_t		entsize = PROP_ENTSIZE(klen, vlen);
	char		*kptr, *vptr;
	propent_t	*pep;
	pfc_rbnode_t	*old;

	pep = (propent_t *)malloc(entsize);
	if (PFC_EXPECT_FALSE(pep == NULL)) {
		return ENOMEM;
	}

	kptr = pep->p_buffer;
	vptr = pep->p_buffer + klen + 1;
	prop_setstring(kptr, key, klen);
	prop_setstring(vptr, value, vlen);
	pep->p_key = kptr;
	pep->p_value= vptr;

	old = pfc_rbtree_update(&prop_tree, &pep->p_node);
	if (old != NULL) {
		prop_dtor(old, NULL);
	}

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_prop_fini(void)
 *	Finalize the PFC daemon system property.
 */
void PFC_ATTR_HIDDEN
pfc_prop_fini(void)
{
	pfc_rbtree_clear(&prop_tree, prop_dtor, NULL);
}

/*
 * static pfc_cptr_t
 * prop_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified property node.
 *	`node' must be a pointer to p_node in propent_t.
 */
static pfc_cptr_t
prop_getkey(pfc_rbnode_t *node)
{
	propent_t	*pep = PROP_NODE2PTR(node);

	return (pfc_cptr_t)pep->p_key;
}

/*
 * static void
 * prop_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destroy the specified property node.
 *	`node' must be a pointer to p_node in propent_t.
 */
static void
prop_dtor(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	propent_t	*pep = PROP_NODE2PTR(node);

	free(pep);
}
