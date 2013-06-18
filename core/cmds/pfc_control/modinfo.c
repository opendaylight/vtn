/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * modinfo.c - Fetch module information.
 */

#include <string.h>
#include "modinfo.h"
#include "pfc_control.h"

/*
 * Internal prototypes.
 */
static int		modinfo_receive(cproto_sess_t *sess, pfc_ptr_t arg);
static pfc_cptr_t	modinfo_ent_getkey(pfc_rbnode_t *node);

/*
 * Control protocol command operations for MODLIST.
 */
static const pfc_ctrl_ops_t	modinfo_client_ops = {
	.receive	= modinfo_receive,
};

/*
 * int
 * modinfo_fetch(cproto_sess_t *PFC_RESTRICT sess,
 *		 modinfo_t *PFC_RESTRICT minfo)
 *	Fetch module information from the PFC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, module information is stored to `*minfo',
 *	and zero is returned.
 *	1 is returned if the PFC daemon returns an error response.
 *	Otherwise a negative error number which indicates the cause of error
 *	is returned.
 */
int
modinfo_fetch(cproto_sess_t *PFC_RESTRICT sess, modinfo_t *PFC_RESTRICT minfo)
{
	pfc_rbtree_t	*tree = &minfo->mi_tree;

	pfc_rbtree_init(tree, (pfc_rbcomp_t)strcmp, modinfo_ent_getkey);
	minfo->mi_module = NULL;

	return pfc_ctrl_client_execute(sess, CTRL_CMDTYPE_MODLIST,
				       &modinfo_client_ops, minfo);
}

/*
 * void
 * modinfo_free(modinfo_t *minfo)
 *	Free up resources held by the module information specified by `minfo'.
 */
void
modinfo_free(modinfo_t *minfo)
{
	pfc_rbtree_t	*tree = &minfo->mi_tree;
	modinfo_ent_t	*mip, *next;

	for (mip = minfo->mi_module; mip != NULL; mip = next) {
		next = mip->mie_next;
		free((void *)mip->mie_name);
		free(mip);
	}

	pfc_rbtree_init(tree, (pfc_rbcomp_t)strcmp, modinfo_ent_getkey);
	minfo->mi_module = NULL;
}

/*
 * modinfo_ent_t *
 * modinfo_get(modinfo_t *PFC_RESTRICT minfo, const char *name)
 *	Return a pointer to module information entry associated with the module
 *	name specified by `name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non NULL pointer to module information
 *	entry is returned.
 *	NULL is returned if the given module name is not found in the module
 *	information specified by `minfo'.
 */
modinfo_ent_t *
modinfo_get(modinfo_t *PFC_RESTRICT minfo, const char *name)
{
	pfc_rbtree_t	*tree = &minfo->mi_tree;
	pfc_rbnode_t	*node;

	node = pfc_rbtree_get(tree, name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		return MODINFO_ENT_NODE2PTR(node);
	}

	return NULL;
}

/*
 * static int
 * modinfo_receive(cproto_sess_t *sess, pfc_ptr_t arg)
 *	Receive successful response of MODLIST command.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
modinfo_receive(cproto_sess_t *sess, pfc_ptr_t arg)
{
	modinfo_t	*minfo = (modinfo_t *)arg;
	modinfo_ent_t	*mip = NULL, **nextp = &minfo->mi_module;
	pfc_rbtree_t	*tree = &minfo->mi_tree;
	int		err = 0;

	for (;;) {
		cproto_data_t	data;
		ctrl_cmdtype_t	type;

		/* Read module name. */
		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to read module name: %s", strerror(err));
			break;
		}

		if ((type = data.cpd_type) == CTRL_PDTYPE_NULL) {
			/* cproto_data_free() is not needed for NULL data. */
			break;
		}

		if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_TEXT)) {
			cproto_data_free(&data);
			error("Unexpected response data type for module name: "
			      "%u", type);
			err = EPROTO;
			break;
		}

		mip = (modinfo_ent_t *)malloc(sizeof(*mip));
		if (PFC_EXPECT_FALSE(mip == NULL)) {
			error("No memory for module information.");
			goto error;
		}

		/*
		 * Don't call cproto_data_free() for module name.
		 * This string will be released by modinfo_free().
		 */
		mip->mie_name = cproto_data_text(&data);
		mip->mie_next = NULL;

		/* Read module state. */
		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			cproto_data_free(&data);
			error("Failed to read module state: %s", strerror(err));
			goto error;
		}
		if (PFC_EXPECT_FALSE(data.cpd_type != CTRL_PDTYPE_INT32)) {
			cproto_data_free(&data);
			error("Unexpected response data type for module state:"
			      " %u", data.cpd_type);
			err = EPROTO;
			goto error;
		}

		mip->mie_state = cproto_data_uint32(&data);
		cproto_data_free(&data);

		/* Read module flags. */
		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			cproto_data_free(&data);
			error("Failed to read module flags: %s", strerror(err));
			goto error;
		}
		if (PFC_EXPECT_FALSE(data.cpd_type != CTRL_PDTYPE_INT32)) {
			cproto_data_free(&data);
			error("Unexpected response data type for module flags:"
			      " %u", data.cpd_type);
			err = EPROTO;
			goto error;
		}

		mip->mie_flags = cproto_data_uint32(&data);
		cproto_data_free(&data);

		*nextp = mip;
		nextp = &mip->mie_next;

		err = pfc_rbtree_put(tree, &mip->mie_node);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == EEXIST);
			error("Duplicated module name: %s", mip->mie_name);
			mip = NULL;
			goto error;
		}
	}

	return 0;

error:
	if (mip != NULL) {
		free((void *)mip->mie_name);
		free(mip);
	}

	modinfo_free(minfo);

	return err;
}

/*
 * static pfc_cptr_t
 * modinfo_ent_getkey(pfc_rbnode_t *node)
 *	Return the key of the module information entry.
 *	`node' must be a pointer to mie_node in modinfo_ent_t.
 */
static pfc_cptr_t
modinfo_ent_getkey(pfc_rbnode_t *node)
{
	modinfo_ent_t	*mip = MODINFO_ENT_NODE2PTR(node);

	return (pfc_cptr_t)mip->mie_name;
}
