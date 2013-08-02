/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf_map.c - Map implementations used by configuration file parser.
 */

#include <string.h>
#include "conf_impl.h"

/*
 * Block handle ID for the next allocation.
 */
static pfc_cfblk_t	conf_handle_next = (pfc_cfblk_t)1;

/*
 * Internal prototypes.
 */
static pfc_cptr_t	conf_handle_keyfunc(pfc_rbnode_t *node);
static int		conf_handle_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
static pfc_cptr_t	conf_block_keyfunc(pfc_rbnode_t *node);
static void		conf_block_dtor(pfc_rbnode_t *node,
					pfc_ptr_t arg PFC_ATTR_UNUSED);
static int		conf_imapent_alloc(conf_imap_t *PFC_RESTRICT imap,
					   pfc_refptr_t *PFC_RESTRICT rname,
					   conf_imapent_t **PFC_RESTRICT impp);
static int		conf_imapent_merge(conf_imapent_t *PFC_RESTRICT imp,
					   conf_imapent_t *PFC_RESTRICT merged);
static pfc_cptr_t	conf_imapent_keyfunc(pfc_rbnode_t *node);
static void		conf_imapent_dtor(pfc_rbnode_t *node,
					  pfc_ptr_t arg PFC_ATTR_UNUSED);
static pfc_cptr_t	conf_ctent_keyfunc(pfc_rbnode_t *node);
static void		conf_ctent_dtor(pfc_rbnode_t *node,
					pfc_ptr_t arg PFC_ATTR_UNUSED);
static pfc_cptr_t	conf_pnode_keyfunc(pfc_rbnode_t *node);
static void		conf_pnode_dtor(pfc_rbnode_t *node,
					pfc_ptr_t arg PFC_ATTR_UNUSED);

/*
 * Global handle map which keeps pairs of block handle ID and block handle.
 */
conf_hdlmap_t	conf_blocks PFC_ATTR_HIDDEN =
	PFC_RBTREE_INITIALIZER(conf_handle_compare, conf_handle_keyfunc);

/*
 * int PFC_ATTR_HIDDEN
 * conf_hdlmap_put(conf_block_t *bp)
 *	Assign new handle ID for the specified configuration block, and put
 *	it into the handle map.
 *
 * Calling/Exit State:
 *	Upon successful completion, new handle ID is set into bp->cb_handle,
 *	and zero is returned.
 *	ENFILE is returned if no handle ID is available.
 *
 * Remarks:
 *	The caller must call this function with holding parser lock in
 *	writer mode.
 */
int PFC_ATTR_HIDDEN
conf_hdlmap_put(conf_block_t *bp)
{
	conf_hdlmap_t	*hdlmap = &conf_blocks;
	pfc_cfblk_t	start;

	PFC_ASSERT(bp->cb_handle == PFC_CFBLK_INVALID);

	/* Assign new handle ID. */
	start = conf_handle_next;
	PFC_ASSERT(start != PFC_CFBLK_INVALID);
	bp->cb_handle = start;

	do {
		pfc_cfblk_t	id;
		int		err;

		/* Try to put this handle into the handle map. */
		err = pfc_rbtree_put(hdlmap, &bp->cb_hdlnode);

		/* Prepare ID for next allocation. */
		id = bp->cb_handle + 1;
		if (PFC_EXPECT_TRUE(id == PFC_CFBLK_INVALID)) {
			id++;
		}
		if (PFC_EXPECT_TRUE(err == 0)) {
			conf_handle_next = id;

			return 0;
		}
		PFC_ASSERT(err == EEXIST);

		/* Handle ID may be overflowed. Try next ID. */
		bp->cb_handle = id;
	} while (bp->cb_handle != start);

	/* No handle ID is available. */
	bp->cb_handle = PFC_CFBLK_INVALID;

	return ENFILE;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_blkmap_init(conf_blkmap_t *blkmap)
 *	Initialize parameter block map which keeps pairs of parameter block
 *	name and block handle instance.
 */
void PFC_ATTR_HIDDEN
conf_blkmap_init(conf_blkmap_t *blkmap)
{
	pfc_rbtree_init(blkmap, (pfc_rbcomp_t)strcmp, conf_block_keyfunc);
}

/*
 * int PFC_ATTR_HIDDEN
 * conf_blkmap_alloc(conf_blkmap_t *PFC_RESTRICT blkmap,
 *		     pfc_refptr_t *PFC_RESTRICT name,
 *		     conf_block_t *PFC_RESTRICT *bpp)
 *	Allocate a new block handle, and enter it into the specified block map.
 *
 *	`name' must be a pfc_refptr_t pointer which keeps parameter block name
 *	or parameter map key.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to new block handle is set to
 *	`*bpp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function never changes the reference counter of `name'.
 */
int PFC_ATTR_HIDDEN
conf_blkmap_alloc(conf_blkmap_t *PFC_RESTRICT blkmap,
		  pfc_refptr_t *PFC_RESTRICT name,
		  conf_block_t *PFC_RESTRICT *bpp)
{
	conf_block_t	*bp;
	int		err;

	/* Allocate a block handle instance. */
	bp = (conf_block_t *)malloc(sizeof(*bp));
	if (PFC_EXPECT_FALSE(bp == NULL)) {
		return ENOMEM;
	}

	/*
	 * Keep string refptr without decrementing its counter.
	 * It will be decremented by destructor.
	 */
	bp->cb_name = name;

	/* Register new block handle into the block map. */
	err = pfc_rbtree_put(blkmap, &bp->cb_blknode);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(bp);

		return err;
	}

	/* Initialize parameter map in the handle. */
	conf_pmap_init(&bp->cb_params);
	bp->cb_handle = PFC_CFBLK_INVALID;

	*bpp = bp;

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_blkmap_clear(conf_blkmap_t *blkmap)
 *	Remove all block handles in the specified block map.
 */
void PFC_ATTR_HIDDEN
conf_blkmap_clear(conf_blkmap_t *blkmap)
{
	pfc_rbtree_clear(blkmap, conf_block_dtor, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_blkmap_merge(conf_blkmap_t *PFC_RESTRICT blkmap,
 *		     conf_blkmap_t *PFC_RESTRICT merged)
 *	Merge block handles in `merged' to `blkmap'.
 *
 *	All duplicated handles in `merged' will be unlinked from
 *	conf_file_t->cf_blklist chain, and will be destroyed.
 *
 * Remarks:
 *	This function must be called with holding parser lock in writer mode.
 */
void PFC_ATTR_HIDDEN
conf_blkmap_merge(conf_blkmap_t *PFC_RESTRICT blkmap,
		  conf_blkmap_t *PFC_RESTRICT merged)
{
	pfc_rbnode_t	*node;

	while ((node = pfc_rbtree_next(merged, NULL)) != NULL) {
		conf_block_t	*bp;
		int		err;

		/* Remove a node from map to be merged. */
		pfc_rbtree_remove_node(merged, node);

		/* Try to put this node to blkmap. */
		err = pfc_rbtree_put(blkmap, node);
		if (err == 0) {
			continue;
		}
		PFC_ASSERT(err == EEXIST);

		/* Remove duplicated block. */
		bp = CONF_BLKNODE2BLOCK(node);
		pfc_list_remove(&bp->cb_list);
		conf_hdlmap_remove(bp);
		conf_block_dtor(node, NULL);
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_imap_init(conf_imap_t *imap)
 *	Initialize intermediate map which keeps pairs of refptr string and
 *	parameter block map.
 */
void PFC_ATTR_HIDDEN
conf_imap_init(conf_imap_t *imap)
{
	pfc_rbtree_init(imap, (pfc_rbcomp_t)strcmp, conf_imapent_keyfunc);
}

/*
 * int PFC_ATTR_HIDDEN
 * conf_imap_alloc(conf_imap_t *PFC_RESTRICT imap, pfc_refptr_t *rname,
 *		   pfc_refptr_t *key, conf_blkmap_t *PFC_RESTRICT *blkmapp)
 *	Allocate new intermediate map entry for the specified name.
 *
 *	`rname' must be a pfc_refptr_t pointer which keeps key string for
 *	a new entry. `key' must be a pfc_refptr_t pointer which represents
 *	a map key for a new block map.
 *
 *	If the specified name already exists in the map, this function will
 *	set a pointer to conf_blkmap_t associated with the name to `*blkmapp'.
 *	Otherwise, this function will try to allocate a new entry.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to block map associated with
 *	the specified name is set to `*blkmapp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	The reference counter of `rname' is not changed if a new map entry
 *	is allocated. Otherwise it is decremented.
 */
int PFC_ATTR_HIDDEN
conf_imap_alloc(conf_imap_t *PFC_RESTRICT imap, pfc_refptr_t *rname,
		pfc_refptr_t *key, conf_blkmap_t *PFC_RESTRICT *blkmapp)
{
	const char	*name = pfc_refptr_string_value(rname);
	pfc_rbnode_t	*node = pfc_rbtree_get(imap, name);
	conf_imapent_t	*imp;
	int		err;

	if (node == NULL) {
		/* Allocate a new entry. */
		err = conf_imapent_alloc(imap, rname, &imp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}
	else {
		/* Use existing entry. */
		imp = CONF_NODE2IMAPENT(node);

		/*
		 * In this case, reference counter of rname must be
		 * decremented.
		 */
		pfc_refptr_put(rname);
	}

	/* Insert the given map key to the map entry. */
	err = pfc_listm_push_tail(imp->cie_keys, key);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (node == NULL) {
			node = &imp->cie_node;

			/* Revert changes made to imap. */
			pfc_rbtree_remove_node(imap, node);
			conf_imapent_dtor(node, NULL);
		}
	}

	*blkmapp = &imp->cie_blkmap;

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_imap_clear(conf_imap_t *imap)
 *	Remove all entries in the intermediate map.
 *	All parameter block handles in the map are also removed.
 */
void PFC_ATTR_HIDDEN
conf_imap_clear(conf_imap_t *imap)
{
	pfc_rbtree_clear(imap, conf_imapent_dtor, NULL);
}

/*
 * int PFC_ATTR_HIDDEN
 * conf_imap_merge(conf_imap_t *PFC_RESTRICT imap,
 *		   conf_imap_t *PFC_RESTRICT merged)
 *	Merge map handles in `merged' to `imap'.
 *
 *	All duplicated map handles in `merged' will be removed.
 *	All duplicated block handles in `merged' will be unlinked from
 *	conf_file_t->cf_blklist chain, and will be destroyed.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	Note that both `imap' and `merged' are broken on error.
 *
 * Remarks:
 *	This function must be called with holding parser lock in writer mode.
 */
int PFC_ATTR_HIDDEN
conf_imap_merge(conf_imap_t *PFC_RESTRICT imap,
		conf_imap_t *PFC_RESTRICT merged)
{
	pfc_rbnode_t	*node;

	while ((node = pfc_rbtree_next(merged, NULL)) != NULL) {
		conf_imapent_t	*imp = CONF_NODE2IMAPENT(node);
		conf_imapent_t	*nimp;
		const char	*name = pfc_refptr_string_value(imp->cie_name);
		pfc_rbnode_t	*nd;
		int		err;

		/* Remove a node from map to be merged. */
		pfc_rbtree_remove_node(merged, node);

		/* Look for the same map block in imap. */
		nd = pfc_rbtree_get(imap, name);
		if (nd == NULL) {
			/* This intermediate map can be put to imap. */
			PFC_ASSERT_INT(pfc_rbtree_put(imap, node), 0);
			continue;
		}

		/* Merge intermediate map. */
		nimp = CONF_NODE2IMAPENT(nd);
		err = conf_imapent_merge(nimp, imp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		/* This intermediate map is no longer used. */
		conf_imapent_dtor(node, NULL);
	}

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_ctmap_init(conf_ctmap_t *ctmap)
 *	Initialize container map, which keeps pairs of string key and
 *	anonymous pointer.
 */
void PFC_ATTR_HIDDEN
conf_ctmap_init(conf_ctmap_t *ctmap)
{
	pfc_rbtree_init(ctmap, (pfc_rbcomp_t)strcmp, conf_ctent_keyfunc);
}

/*
 * int PFC_ATTR_HIDDEN
 * conf_ctmap_put(conf_ctmap_t *PFC_RESTRICT ctmap,
 *		  const char *PFC_RESTRICT key, pfc_cptr_t PFC_RESTRICT value)
 *	Insert the given pair of string key and anonymous pointer value into
 *	the container map.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	A pointer to string key is put into container entry directly.
 *	So the caller must specify a pointer to static string literal as key.
 */
int PFC_ATTR_HIDDEN
conf_ctmap_put(conf_ctmap_t *PFC_RESTRICT ctmap, const char *PFC_RESTRICT key,
	       pfc_cptr_t PFC_RESTRICT value)
{
	conf_ctent_t	*ccp;
	int		err;

	ccp = (conf_ctent_t *)malloc(sizeof(*ccp));
	if (PFC_EXPECT_FALSE(ccp == NULL)) {
		return ENOMEM;
	}

	ccp->cc_key = key;
	ccp->cc_value = value;

	err = pfc_rbtree_put(ctmap, &ccp->cc_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		free(ccp);
	}

	return err;
}

/*
 * int PFC_ATTR_HIDDEN
 * conf_ctmap_remove(conf_ctmap_t *PFC_RESTRICT ctmap,
 *		     const char *PFC_RESTRICT key)
 *	Remove a string key from the container map.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	ENOENT is returned if the given key is not found.
 */
int PFC_ATTR_HIDDEN
conf_ctmap_remove(conf_ctmap_t *PFC_RESTRICT ctmap,
		  const char *PFC_RESTRICT key)
{
	pfc_rbnode_t	*node = pfc_rbtree_remove(ctmap, key);

	if (PFC_EXPECT_FALSE(node == NULL)) {
		return ENOENT;
	}

	conf_ctent_dtor(node, NULL);

	return 0;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_ctmap_clear(conf_ctmap_t *ctmap)
 *	Remove all pairs of string key and pointer value in the container map.
 */
void PFC_ATTR_HIDDEN
conf_ctmap_clear(conf_ctmap_t *ctmap)
{
	pfc_rbtree_clear(ctmap, conf_ctent_dtor, NULL);
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_pmap_init(conf_pmap_t *pmap)
 *	Initialize parameter map which keeps pairs of parameter name and its
 *	value.
 */
void PFC_ATTR_HIDDEN
conf_pmap_init(conf_pmap_t *pmap)
{
	pfc_rbtree_init(pmap, (pfc_rbcomp_t)strcmp, conf_pnode_keyfunc);
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_pmap_clear(conf_pmap_t *pmap)
 *	Remove all parameters in the specified parameter map.
 */
void PFC_ATTR_HIDDEN
conf_pmap_clear(conf_pmap_t *pmap)
{
	pfc_rbtree_clear(pmap, conf_pnode_dtor, NULL);
}

/*
 * static pfc_cptr_t
 * conf_handle_keyfunc(pfc_rbnode_t *node)
 *	Return key value for the specified Red-Black tree node.
 *	`node' must be a pointer to conf_block_t->cb_hdlnode.
 */
static pfc_cptr_t
conf_handle_keyfunc(pfc_rbnode_t *node)
{
	conf_block_t	*bp = CONF_HDLNODE2BLOCK(node);

	/* Return handle ID as key value. */
	return (pfc_cptr_t)(uintptr_t)bp->cb_handle;
}

/*
 * static int
 * conf_handle_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Comparator of conf_block_t in handle map.
 *	`arg1' and `arg2' must be handle IDs.
 *
 * Calling/Exit State:
 *	The comparator must return an integer less than, equal to,
 *	or greater than zero if the first argument is considered to be
 *	respectively less than, equal to, or greater than the second.
 */
static int
conf_handle_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
	pfc_cfblk_t	h1 = (pfc_cfblk_t)(uintptr_t)arg1;
	pfc_cfblk_t	h2 = (pfc_cfblk_t)(uintptr_t)arg2;

	if (h1 == h2) {
		return 0;
	}

	return (h1 < h2) ? -1 : 1;
}

/*
 * static pfc_cptr_t
 * conf_block_keyfunc(pfc_rbnode_t *node)
 *	Return key value for the specified Red-Black tree node.
 *	`node' must be a pointer to conf_block_t->cb_blknode.
 */
static pfc_cptr_t
conf_block_keyfunc(pfc_rbnode_t *node)
{
	conf_block_t	*bp = CONF_BLKNODE2BLOCK(node);

	return (pfc_cptr_t)pfc_refptr_string_value(bp->cb_name);
}

/*
 * static void
 * conf_block_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of block handle instance.
 *	`node' must be a pointer to conf_block_t->cb_blknode.
 *
 * Remarks:
 *	Note that this function never unlinks conf_block_t instance from
 *	conf_file_t->cf_blklist chain. The caller must be responsible for
 *	maintenance of conf_file_t->cf_blklist chain.
 */
static void
conf_block_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	conf_block_t	*bp = CONF_BLKNODE2BLOCK(node);

	conf_pmap_clear(&bp->cb_params);
	pfc_refptr_put(bp->cb_name);
	free(bp);
}

/*
 * static int
 * conf_imapent_alloc(conf_imap_t *PFC_RESTRICT imap,
 *		      pfc_refptr_t *PFC_RESTRICT rname,
 *		      conf_imapent_t **PFC_RESTRICT impp)
 *	Create a new intermediate map entry.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to a new intermediate map entry
 *	is set to the buffer pointed by `impp', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
conf_imapent_alloc(conf_imap_t *PFC_RESTRICT imap,
		   pfc_refptr_t *PFC_RESTRICT rname,
		   conf_imapent_t **PFC_RESTRICT impp)
{
	conf_imapent_t	*imp;
	int		err;

	/* Allocate a new intermediate map entry. */
	imp = (conf_imapent_t *)malloc(sizeof(*imp));
	if (PFC_EXPECT_FALSE(imp == NULL)) {
		err = ENOMEM;
		goto error;
	}

	/* Create a vector to keep map keys. */
	err = PFC_VECTOR_CREATE_REF(&imp->cie_keys, pfc_refptr_string_ops());
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_imp;
	}

	/*
	 * Keep string refptr without decrementing its counter.
	 * It will be decremented by destructor.
	 */
	imp->cie_name = rname;

	/* Insert a new map entry to the map. */
	err = pfc_rbtree_put(imap, &imp->cie_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error_imp;
	}

	/* Initialize internal block map. */
	conf_blkmap_init(&imp->cie_blkmap);
	*impp = imp;

	return 0;

error_imp:
	free(imp);

error:
	pfc_refptr_put(rname);

	/* Suppress warning blamed by old gcc. */
	*impp = NULL;

	return err;
}

/*
 * static int
 * conf_imapent_merge(conf_imapent_t *PFC_RESTRICT imp,
 *		      conf_imapent_t *PFC_RESTRICT merged)
 *	Merge intermediate map specified by `merged' to `imp'.
 *
 *	All duplicated map handles in `merged' will be removed.
 *	All duplicated block handles in `merged' will be unlinked from
 *	conf_file_t->cf_blklist chain, and will be destroyed.
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *	Note that both `imp' and `merged' are broken on error.
 *
 * Remarks:
 *	This function must be called with holding parser lock in writer mode.
 */
static int
conf_imapent_merge(conf_imapent_t *PFC_RESTRICT imp,
		   conf_imapent_t *PFC_RESTRICT merged)
{
	conf_blkmap_t	*blkmap = &imp->cie_blkmap;
	conf_blkmap_t	*mblk = &merged->cie_blkmap;
	pfc_rbnode_t	*node;
#ifdef	PFC_VERBOSE_DEBUG
	int		count = 0;
#endif	/* PFC_VERBOSE_DEBUG */

	while ((node = pfc_rbtree_next(mblk, NULL)) != NULL) {
		conf_block_t	*bp = CONF_BLKNODE2BLOCK(node);
		int		err;

#ifdef	PFC_VERBOSE_DEBUG
		count++;
#endif	/* PFC_VERBOSE_DEBUG */

		/* Remove a node from map to be merged. */
		pfc_rbtree_remove_node(mblk, node);

		/* Try to put this node to blkmap. */
		err = pfc_rbtree_put(blkmap, node);
		if (err == 0) {
			/* Append map key to the key list. */
			err = pfc_listm_push_tail(imp->cie_keys, bp->cb_name);
			if (PFC_EXPECT_FALSE(err != 0)) {
				return err;
			}
		}
		else {
			PFC_ASSERT(err == EEXIST);

			/* Remove duplicated block. */
			pfc_list_remove(&bp->cb_list);
			conf_hdlmap_remove(bp);
			conf_block_dtor(node, NULL);
		}
	}

	PFC_ASSERT_INT(pfc_listm_clear(merged->cie_keys), count);

	return 0;
}

/*
 * static pfc_cptr_t
 * conf_block_keyfunc(pfc_rbnode_t *node)
 *	Return key value for the specified Red-Black tree node.
 *	`node' must be a pointer to pfc_rbnode_t embedded in conf_imapent_t.
 */
static pfc_cptr_t
conf_imapent_keyfunc(pfc_rbnode_t *node)
{
	conf_imapent_t	*imp = CONF_NODE2IMAPENT(node);

	return (pfc_cptr_t)pfc_refptr_string_value(imp->cie_name);
}

/*
 * static void
 * conf_imapent_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of conf_imapent_t.
 *	`node' must be a pointer to pfc_rbnode_t embedded in conf_imapent_t.
 */
static void
conf_imapent_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	conf_imapent_t	*imp = CONF_NODE2IMAPENT(node);

	pfc_listm_destroy(imp->cie_keys);
	conf_blkmap_clear(&imp->cie_blkmap);
	pfc_refptr_put(imp->cie_name);
	free(imp);
}

/*
 * static pfc_cptr_t
 * conf_ctent_keyfunc(pfc_rbnode_t *node)
 *	Return key value for the specified Red-Black tree node.
 *	`node' must be a pointer to pfc_rbnode_t embedded in conf_ctent_t.
 */
static pfc_cptr_t
conf_ctent_keyfunc(pfc_rbnode_t *node)
{
	conf_ctent_t	*ccp = CONF_NODE2CTENT(node);

	return (pfc_cptr_t)ccp->cc_key;
}

/*
 * static void
 * conf_ctent_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of container map entry.
 */
static void
conf_ctent_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	conf_ctent_t	*ccp = CONF_NODE2CTENT(node);

	free(ccp);
}

/*
 * static pfc_cptr_t
 * conf_pnode_keyfunc(pfc_rbnode_t *node)
 *	Return key value for the specified Red-Black tree node embedded in
 *	conf_pnode_t.
 */
static pfc_cptr_t
conf_pnode_keyfunc(pfc_rbnode_t *node)
{
	conf_pnode_t	*pnp = CONF_NODE2PNODE(node);
	conf_param_t	*param = &pnp->cpn_param;

	return (pfc_cptr_t)param->cp_def->cfdp_name;
}

/*
 * static void
 * conf_pnode_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of conf_pnode_t.
 *	This function is called if parameter instance is removed from
 *	parameter map.
 */
static void
conf_pnode_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	conf_pnode_t		*pnp = CONF_NODE2PNODE(node);
	conf_param_t		*param = &pnp->cpn_param;
	const pfc_cfdef_param_t	*pdp = param->cp_def;
	pfc_cftype_t	type;

	PFC_ASSERT(pdp != NULL);

	type = pdp->cfdp_type;

	if (PFC_CFDEF_PARAM_IS_ARRAY(pdp)) {
		pfc_ptr_t	array = param->cp_value.a;

		/* Array value */
		if (array != NULL && type == PFC_CFTYPE_STRING) {
			pfc_refptr_t	**rarray = (pfc_refptr_t **)array;
			pfc_refptr_t	**rpp;

			for (rpp = rarray; rpp < rarray + param->cp_nelems;
			     rpp++) {
				pfc_refptr_t	*str = *rpp;

				PFC_ASSERT(str != NULL);
				pfc_refptr_put(str);
			}
		}

		free(array);
	}
	else {
		/* Scalar value */
		if (type == PFC_CFTYPE_STRING) {
			pfc_refptr_t	*str = CONF_VALUE_STRING(param);

			if (str != NULL) {
				pfc_refptr_put(str);
			}
		}
	}

	free(param);
}
