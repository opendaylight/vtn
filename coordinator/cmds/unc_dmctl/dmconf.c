/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dmconf.c - Daemon configuration management.
 */

#include <string.h>
#include <pfc/conf.h>
#include <unc/liblauncher.h>
#include "unc_dmctl.h"
#include "dmconf.h"

/*
 * Internal prototypes.
 */
static int	dmconf_fetch(lnc_conf_t *cfp);
static int	dmconf_uncd(void);
static void	dmconf_destroy(pfc_rbnode_t *node, pfc_ptr_t arg);

static int	dmorder_put(dmconf_t *UNC_RESTRICT dcp,
			    uint32_t *UNC_RESTRICT orderp, pfc_cfblk_t blk,
			    lnc_ordtype_t type, int index, uint32_t flags);
static void	dmorder_destroy(pfc_rbnode_t *node, pfc_ptr_t arg);

static pfc_cptr_t	dmconf_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	dmorder_getkey(pfc_rbnode_t *node);

/*
 * Flags for dmorder_put().
 */
#define	DMORDF_MANDATORY	PFC_CONST_U(0x1)	/* mandatory */
#define	DMORDF_NOCONF		PFC_CONST_U(0x2)	/* no conf file */

/*
 * Red-Black tree which keeps dmconf_t indexed by name.
 */
static pfc_rbtree_t	dmconf_tree =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, dmconf_getkey);

/*
 * Red-Black tree which keeps daemon configuration indexed by UINT32 order
 * value.
 */
static pfc_rbtree_t	dmconf_order_tree[LNC_ORDER_NTYPES];

/*
 * The number of loaded configurations.
 */
static uint32_t		dmconf_count = UINT32_MAX;

/*
 * static inline pfc_rbtree_t PFC_FATTR_ALWAYS_INLINE *
 * dmconf_order_gettree(lnc_ordtype_t type, int index)
 *	Determine the Red-Black tree which keeps daemon order.
 *
 *	The target tree is determined by the order type `type' and
 *	type index `index'. `index' is always ignored if `type' is
 *	either LNC_ORDTYPE_START or LNC_ORDTYPE_STOP.
 */
static inline pfc_rbtree_t PFC_FATTR_ALWAYS_INLINE *
dmconf_order_gettree(lnc_ordtype_t type, int index)
{
	uint32_t	idx = (uint32_t)type;

	if (LNC_ORDTYPE_HASINDEX(type)) {
		PFC_ASSERT(index >= 0);
		idx += index;
	}

	PFC_ASSERT(idx < PFC_ARRAY_CAPACITY(dmconf_order_tree));

	return &dmconf_order_tree[idx];
}

/*
 * uint32_t
 * dmconf_load(void)
 *	Load all daemon configuration files.
 *
 * Calling/Exit State:
 *	Upon successful completion, the number of loaded configurations is
 *	returned.
 *	UINT32_MAX is returned on failure.
 */
uint32_t
dmconf_load(void)
{
	pfc_refptr_t	*confdir;
	pfc_rbtree_t	*tree;
	lnc_confctx_t	*ctx;
	uint32_t	count, oflags;
	int		err;

	if (PFC_EXPECT_FALSE(dmconf_count != UINT32_MAX)) {
		/* Already loaded. */
		return dmconf_count;
	}

	/* Initialize daemon order tree array. */
	for (tree = dmconf_order_tree;
	     tree < PFC_ARRAY_LIMIT(dmconf_order_tree); tree++) {
		pfc_rbtree_init(tree, pfc_rbtree_uint32_compare,
				dmorder_getkey);
	}

	/* Determine daemon configuration directory. */
	confdir = liblnc_getconfdir();
	if (PFC_EXPECT_FALSE(confdir == NULL)) {
		error("Unable to determine daemon configuration directory.");

		return UINT32_MAX;
	}

	/* Open daemon configuration directory. */
	oflags = (debug_level) ? LIBLNC_OFLAG_LOG : 0;
	err = liblnc_conf_opendir(&ctx, pfc_refptr_string_value(confdir),
				  oflags);
	pfc_refptr_put(confdir);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Unable to open daemon configuration directory: %s",
		      strerror(err));

		return UINT32_MAX;
	}

	/* Register a dummy configuration which represents UNC daemon. */
	err = dmconf_uncd();
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto error;
	}

	count = 0;
	for (;;) {
		lnc_conf_t	dcf;

		err = liblnc_conf_getnext(ctx, &dcf);
		if (err < 0) {
			liblnc_conf_closedir(ctx);
			dmconf_count = count;

			return count;
		}

		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Unable to read daemon configuration directory"
			      ": %s", strerror(err));
			break;
		}

		/* Load daemon configuration. */
		err = dmconf_fetch(&dcf);
		liblnc_conf_close(&dcf);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}

		count++;
	}

	liblnc_conf_closedir(ctx);

error:
	/* Clean up loaded configuration. */
	dmconf_cleanup();

	return UINT32_MAX;
}

/*
 * void
 * dmconf_cleanup(void)
 *	Clean up all loaded daemon configurations.
 */
void
dmconf_cleanup(void)
{
	pfc_rbtree_t	*tree;

	for (tree = dmconf_order_tree;
	     tree < PFC_ARRAY_LIMIT(dmconf_order_tree); tree++) {
		pfc_rbtree_clear(tree, dmorder_destroy, NULL);
	}

	pfc_rbtree_clear(&dmconf_tree, dmconf_destroy, NULL);
	dmconf_count = UINT32_MAX;
}

/*
 * dmconf_t *
 * dmconf_get(const char *name)
 *	Return a pointer to dmconf_t associated with the given daemon name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to dmconf_t is returned.
 *	Otherwise NULL is returned.
 */
dmconf_t *
dmconf_get(const char *name)
{
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(&dmconf_tree, (pfc_cptr_t)name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		return NULL;
	}

	return DMCONF_NODE2PTR(node);
}

/*
 * void
 * dmconf_iterate(dmconf_iter_t iterator, pfc_ptr_t arg)
 *	Iterate daemon configurations in name order.
 *
 *	`arg' is an arbitrary data to be passed to the given iterator.
 */
void
dmconf_iterate(dmconf_iter_t iterator, pfc_ptr_t arg)
{
	pfc_rbnode_t	*node = NULL;

	while ((node = pfc_rbtree_next(&dmconf_tree, node)) != NULL) {
		dmconf_t	*dcp = DMCONF_NODE2PTR(node);

		(*iterator)(dcp, arg);
	}
}

/*
 * void
 * dmconf_order_iterate(dmconf_iter_t iterator, lnc_ordtype_t type, int index,
 *			pfc_ptr_t arg)
 *	Iterate daemon configurations in order of UINT32 order value.
 *
 *	The target tree is determined by the order type `type' and
 *	type index `index'. `index' is always ignored if `type' is
 *	either LNC_ORDTYPE_START or LNC_ORDTYPE_STOP.
 *
 *	`arg' is an arbitrary data to be passed to the given iterator.
 */
void
dmconf_order_iterate(dmconf_iter_t iterator, lnc_ordtype_t type, int index,
		     pfc_ptr_t arg)
{
	pfc_rbtree_t	*tree = dmconf_order_gettree(type, index);
	pfc_rbnode_t	*node = NULL;

	while ((node = pfc_rbtree_next(tree, node)) != NULL) {
		dmorder_t	*dop = DMORDER_NODE2PTR(node);

		(*iterator)(dop->dco_conf, arg);
	}
}

/*
 * static int
 * dmconf_fetch(lnc_conf_t *cfp)
 *	Fetch daemon configuration from the given configuration file handle.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
dmconf_fetch(lnc_conf_t *cfp)
{
	dmconf_t	*dcp;
	lnc_errmsg_t	emsg;
	pfc_conf_t	cf = cfp->lcf_conf;
	pfc_cfblk_t	blk;
	const char	*str;
	uint32_t	clidx;
	int		err;

	dcp = (dmconf_t *)malloc(sizeof(*dcp));
	if (PFC_EXPECT_FALSE(dcp == NULL)) {
		error("%s: Unable to allocate buffer for daemon configuration.",
		      pfc_refptr_string_value(cfp->lcf_name));

		return ENOMEM;
	}

	dcp->dc_name = cfp->lcf_name;
	pfc_refptr_get(dcp->dc_name);
	blk = pfc_conf_get_block(cf, "daemon");

	dcp->dc_type = LIBLNC_DMCONF_GET(blk, uint32, process_type);
	dcp->dc_uncd = LIBLNC_DMCONF_GET(blk, bool, uncd);
	dcp->dc_start_wait = LIBLNC_DMCONF_GET(blk, bool, start_wait);
	dcp->dc_start_timeout = LIBLNC_DMCONF_GET(blk, uint32, start_timeout);
	dcp->dc_stop_timeout = LIBLNC_DMCONF_GET(blk, uint32, stop_timeout);
	dcp->dc_stderr_rotate = LIBLNC_DMCONF_GET(blk, int32, stderr_rotate);
	dcp->dc_data = NULL;

	str = LIBLNC_DMCONF_GET(blk, string, stop_signal);
	dcp->dc_stop_sig = liblnc_getsignal(str);
	if (PFC_EXPECT_FALSE(dcp->dc_stop_sig == 0)) {
		error("%s.stop_signal: Unknown signal name: %s",
		      DMCONF_NAME(dcp), str);
		err = EINVAL;
		goto error;
	}

	dcp->dc_stop_signal = pfc_refptr_string_create(str);
	if (PFC_EXPECT_FALSE(dcp->dc_stop_signal == NULL)) {
		/* This should never happen. */
		error("%s: Unable to copy stop_signal value.",
		      DMCONF_NAME(dcp));
		err = ENOMEM;
		goto error;
	}

	str = pfc_conf_get_string(blk, "description", str_unknown);
	dcp->dc_desc = pfc_refptr_string_create(str);
	if (PFC_EXPECT_FALSE(dcp->dc_desc == NULL)) {
		/* This should never happen. */
		error("%s: Unable to copy description.", DMCONF_NAME(dcp));
		err = ENOMEM;
		goto error_sig;
	}

	/* Fetch daemon command. */
	str = LIBLNC_DMCONF_GET(blk, string, command);
	PFC_ASSERT(str != NULL);
	err = liblnc_cmdmap_create(&dcp->dc_command, cfp, str, &emsg);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("%s.%s: %s: %s", DMCONF_NAME(dcp), str,
		      emsg.le_message, strerror(err));
		goto error_desc;
	}

	/* Fetch command to stop the daemon. */
	str = LIBLNC_DMCONF_GET(blk, string, stop);
	if (str != NULL) {
		err = liblnc_cmdmap_create(&dcp->dc_stop, cfp, str, &emsg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("%s.%s: %s: %s", DMCONF_NAME(dcp), str,
			      emsg.le_message, strerror(err));
			goto error_command;
		}
	}
	else {
		dcp->dc_stop = NULL;
	}

	/* Register the daemon to dmconf_tree. */
	err = pfc_rbtree_put(&dmconf_tree, &dcp->dc_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* This should never happen. */
		PFC_ASSERT(err == EEXIST);
		error("%s: Duplicated daemon name.", DMCONF_NAME(dcp));
		dmconf_destroy(&dcp->dc_node, NULL);

		return err;
	}

	/* Register the daemon to the starting order tree. */
	err = dmorder_put(dcp, &dcp->dc_start_order, blk, LNC_ORDTYPE_START,
			  0, DMORDF_MANDATORY);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Register the daemon to the stopping order tree. */
	err = dmorder_put(dcp, &dcp->dc_stop_order, blk, LNC_ORDTYPE_STOP, 0,
			  DMORDF_MANDATORY);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Register the daemon to cluster event order tree. */
	for (clidx = 0; clidx < CLSTAT_NEVTYPES; clidx++) {
		err = dmorder_put(dcp, &dcp->dc_clevent_order[clidx], blk,
				  LNC_ORDTYPE_CLEVENT, clidx, 0);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
	}

	return err;

error_command:
	liblnc_cmdmap_destroy(dcp->dc_command);

error_desc:
	pfc_refptr_put(dcp->dc_desc);

error_sig:
	pfc_refptr_put(dcp->dc_stop_signal);

error:
	pfc_refptr_put(dcp->dc_name);
	free(dcp);

	return err;
}

/*
 * static int
 * dmconf_uncd(void)
 *	Create a dummy configuration which represents UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
dmconf_uncd(void)
{
	dmconf_t	*dcp;
	uint32_t	clidx;
	int		err = 0;

	dcp = (dmconf_t *)malloc(sizeof(*dcp));
	if (PFC_EXPECT_FALSE(dcp == NULL)) {
		error("Unable to allocate buffer for UNC daemon "
		      "configuration.");

		return ENOMEM;
	}

	memset(dcp, 0, sizeof(*dcp));

	dcp->dc_name = pfc_refptr_string_create("uncd");
	if (PFC_EXPECT_FALSE(dcp->dc_name == NULL)) {
		error("Unable to copy UNC daemon's name.");
		err = ENOMEM;
		goto error;
	}

	/*
	 * Register the UNC daemon to cluster event order tree with
	 * specifying LNC_ORDER_UNCD.
	 */
	for (clidx = 0; clidx < CLSTAT_NEVTYPES; clidx++) {
		dcp->dc_clevent_order[clidx] = LNC_ORDER_UNCD;
		err = dmorder_put(dcp, &dcp->dc_clevent_order[clidx],
				  PFC_CFBLK_INVALID,  LNC_ORDTYPE_CLEVENT,
				  clidx, DMORDF_NOCONF);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/*
			 * dmconf_order_tree is cleaned up by
			 * dmconf_cleanup().
			 */
			goto error_name;
		}
	}

	return err;

error_name:
	pfc_refptr_put(dcp->dc_name);

error:
	free(dcp);

	return err;
}

/*
 * static void
 * dmconf_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destroy dmconf_t.
 *	This function will be called via pfc_rbtree_clear().
 *
 *	`node' must be a pointer to dc_node in dmconf_t.
 */
static void
dmconf_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	dmconf_t	*dcp = DMCONF_NODE2PTR(node);

	pfc_refptr_put(dcp->dc_name);
	pfc_refptr_put(dcp->dc_desc);
	pfc_refptr_put(dcp->dc_stop_signal);

	liblnc_cmdmap_destroy(dcp->dc_command);
	if (dcp->dc_stop != NULL) {
		liblnc_cmdmap_destroy(dcp->dc_stop);
	}

	free(dcp);
}

/*
 * static int
 * dmorder_put(dmconf_t *UNC_RESTRICT dcp, uint32_t *UNC_RESTRICT orderp,
 *	       pfc_cfblk_t blk, lnc_ordtype_t type, int index, uint32_t flags)
 *	Register the given daemon configuration to the Red-Black tree
 *	indexed by UINT32 order value.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Order value is stored to the buffer specified by `orderp'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
dmorder_put(dmconf_t *UNC_RESTRICT dcp, uint32_t *UNC_RESTRICT orderp,
	    pfc_cfblk_t blk, lnc_ordtype_t type, int index, uint32_t flags)
{
	dmorder_t	*dop;
	const char	*param = liblnc_order_getparamname(type);
	pfc_rbtree_t	*tree;
	uint32_t	order;
	int		err;

	if (flags & DMORDF_NOCONF) {
		order = *orderp;
		PFC_ASSERT(order != LNC_ORDER_INVALID);
	}
	else {
		order = (LNC_ORDTYPE_HASINDEX(type))
			? pfc_conf_array_uint32at(blk, param, index,
						  LNC_ORDER_INVALID)
			: pfc_conf_get_uint32(blk, param, LNC_ORDER_INVALID);
		if (order == LNC_ORDER_INVALID) {
			if (PFC_EXPECT_FALSE(flags & DMORDF_MANDATORY)) {
				/* This should never happen. */
				error("%s: \"%s\" is not defined.",
				      DMCONF_NAME(dcp), param);

				return EINVAL;
			}

			/* Don't register invalid order value. */
			*orderp = order;

			return 0;
		}

		*orderp = order;
	}

	dop = (dmorder_t *)malloc(sizeof(*dop));
	if (PFC_EXPECT_FALSE(dop == NULL)) {
		error("%s.%s: Unable to allocate order tree node.",
		      DMCONF_NAME(dcp), param);

		return ENOMEM;
	}

	dop->dco_order = order;
	dop->dco_conf = dcp;

	tree = dmconf_order_gettree(type, index);
	err = pfc_rbtree_put(tree, &dop->dco_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		pfc_rbnode_t	*node;
		dmorder_t	*conflict;
		const char	*idx;
		char		buf[32];

		PFC_ASSERT(err == EEXIST);
		node = pfc_rbtree_get(tree, DMCONF_INTKEY(order));
		PFC_ASSERT(node != NULL);

		conflict = DMORDER_NODE2PTR(node);
		PFC_ASSERT(conflict->dco_order == order);

		idx = liblnc_order_getindexstr(type, index, buf, sizeof(buf));
		error("%s: \"%s%s\" conflicts with \"%s\": value=%u",
		      DMCONF_NAME(dcp), param, idx,
		      DMCONF_NAME(conflict->dco_conf), order);
		free(dop);
		err = EINVAL;
	}

	return err;
}

/*
 * static void
 * dmorder_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
 *	Destroy dmorder_t.
 *	This function will be called via pfc_rbtree_clear().
 *
 *	`node' must be a pointer to dco_node in dmorder_t.
 */
static void
dmorder_destroy(pfc_rbnode_t *node, pfc_ptr_t PFC_ATTR_UNUSED arg)
{
	dmorder_t	*dop = DMORDER_NODE2PTR(node);

	free(dop);
}

/*
 * static pfc_cptr_t
 * dmconf_getkey(pfc_rbnode_t *node)
 *	Return the key of the given dmconf_t node.
 *	`node' must be a pointer to dc_node in dmconf_t.
 */
static pfc_cptr_t
dmconf_getkey(pfc_rbnode_t *node)
{
	dmconf_t	*dcp = DMCONF_NODE2PTR(node);

	return (pfc_cptr_t)DMCONF_NAME(dcp);
}

/*
 * static pfc_cptr_t
 * dmorder_getkey(pfc_rbnode_t *node)
 *	Return the key of the given dmorder_t node.
 *	`node' must be a pointer to dco_node in dmorder_t.
 */
static pfc_cptr_t
dmorder_getkey(pfc_rbnode_t *node)
{
	dmorder_t	*dop = DMORDER_NODE2PTR(node);

	return DMCONF_INTKEY(dop->dco_order);
}
