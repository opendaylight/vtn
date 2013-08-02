/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CONTROL_MODINFO_H
#define	_PFC_CONTROL_MODINFO_H

/*
 * Common definitions to fetch module information.
 */

#include <pfc/base.h>
#include <pfc/rbtree.h>
#include <ctrl_client.h>
#include <module_impl.h>

/*
 * Module information entry.
 */
struct modinfo_ent;
typedef struct modinfo_ent	modinfo_ent_t;

struct modinfo_ent {
	const char	*mie_name;		/* module name */
	uint32_t	mie_flags;		/* module flags */
	pmod_state_t	mie_state;		/* module state */
	modinfo_ent_t	*mie_next;		/* next link */
	pfc_rbnode_t	mie_node;		/* Red-Black tree node */
};

#define	MODINFO_ENT_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), modinfo_ent_t, mie_node)

/*
 * Module information.
 */
typedef struct {
	pfc_rbtree_t	mi_tree;		/* module name tree */
	modinfo_ent_t	*mi_module;		/* list of modules */
} modinfo_t;

/*
 * Prototypes.
 */
extern int		modinfo_fetch(cproto_sess_t *PFC_RESTRICT sess,
				      modinfo_t *PFC_RESTRICT minfo);
extern void		modinfo_free(modinfo_t *minfo);
extern modinfo_ent_t	*modinfo_get(modinfo_t *PFC_RESTRICT minfo,
				     const char *name);

#endif	/* !_PFC_CONTROL_MODINFO_H */
