/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_modlist.c - "modlist" subcommand.
 *
 * "modlist" subcommand lists PFC modules.
 */

#include <stdio.h>
#include <string.h>
#include <cmdopt.h>
#include <module_impl.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1						\
	"List modules installed for" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	".\nIf '-a' options is not specified, only loaded modules are"	\
	" listed."

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{'a', "all", PFC_CMDOPT_TYPE_NONE, 0,
	 "List all registered modules.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * String representation of module state.
 */
typedef struct {
	const char	*m_name;	/* string representation */
	uint32_t	m_state;	/* state value */
} modstate_t;

#define	MODSTATE_DECL(state)			\
	{					\
		.m_name		= #state + 11,	\
		.m_state	= (state),	\
	}

static const modstate_t	mod_states[] = {
	MODSTATE_DECL(PMOD_STATE_UNLOADED),
	MODSTATE_DECL(PMOD_STATE_INACTIVE),
	MODSTATE_DECL(PMOD_STATE_DEAD),
	MODSTATE_DECL(PMOD_STATE_LOADED),
	MODSTATE_DECL(PMOD_STATE_RUNNING),
	MODSTATE_DECL(PMOD_STATE_IN_INIT),
	MODSTATE_DECL(PMOD_STATE_IN_FINI),
};

/*
 * Width of module name column.
 */
#define	MODLIST_NAME_WIDTH	20

/*
 * List all registered modules if true.
 */
static pfc_bool_t	modlist_all;

/*
 * Mark that means mandatory module.
 */
static const char	mark_mandatory[] = "*";

#define	MARK_MANDATORY_WIDTH	1

/*
 * Internal prototypes.
 */
static const char	*modlist_name(const char *name, uint32_t flags);
static const char	*modlist_state(uint32_t state);

/*
 * ctrlcmd_ret_t
 * cmd_ctor_modlist(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		    char **PFC_RESTRICT argv)
 *	Constructor of "modlist" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_modlist(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
		 char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_CONT;
	int		argidx;
	char		c;

	modlist_all = PFC_FALSE;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(CTRLCMD_FULLNAME(spec), argc, argv,
				 option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return CMDRET_FAIL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'a':
			modlist_all = PFC_TRUE;
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			ret = CMDRET_COMPLETE;
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     NULL);
			ret = CMDRET_COMPLETE;
			goto out;

		case PFC_CMDOPT_ERROR:
			ret = CMDRET_FAIL;
			goto out;

		default:
			error("Failed to parse command line options.");
			ret = CMDRET_FAIL;
			goto out;
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		error("Invalid command line options.");
		ret = CMDRET_FAIL;
		goto out;
	}

	argc -= argidx;
	argv += argidx;
	if (argc != 0) {
		pfc_cmdopt_usage(parser, stderr);
		ret = CMDRET_FAIL;
		goto out;
	}

out:
	pfc_cmdopt_destroy(parser);

	return ret;
}

/*
 * int
 * cmd_receive_modlist(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		       cproto_sess_t *PFC_RESTRICT sess)
 *	Receive successful response of MODLIST command.
 */
int
cmd_receive_modlist(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		    cproto_sess_t *PFC_RESTRICT sess)
{
	int	err = 0;

	do {
		cproto_data_t	modname, state, mflags;
		ctrl_cmdtype_t	type;
		uint32_t	stvalue, flags;

		/* Read module name. */
		err = cproto_data_read(sess, &modname);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to read module name: %s", strerror(err));
			break;
		}

		type = modname.cpd_type;
		if (type == CTRL_PDTYPE_NULL) {
			/* cproto_data_free() is not needed for NULL data. */
			break;
		}

		if (PFC_EXPECT_FALSE(type != CTRL_PDTYPE_TEXT)) {
			error("Unexpected response data type for module name: "
			      "%u", type);
			err = EPROTO;
			goto free_modname;
		}

		/* Read module state. */
		err = cproto_data_read(sess, &state);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to read module state: %s", strerror(err));
			goto free_modname;
		}
		if (PFC_EXPECT_FALSE(state.cpd_type != CTRL_PDTYPE_INT32)) {
			error("Unexpected response data type for module state:"
			      " %u", state.cpd_type);
			err = EPROTO;
			goto free_state;
		}

		/* Read module flags. */
		err = cproto_data_read(sess, &mflags);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to read module flags: %s", strerror(err));
			goto free_state;
		}
		if (PFC_EXPECT_FALSE(mflags.cpd_type != CTRL_PDTYPE_INT32)) {
			error("Unexpected response data type for module flags:"
			      " %u", mflags.cpd_type);
			err = EPROTO;
			goto free_mflags;
		}

		stvalue = cproto_data_uint32(&state);
		flags = cproto_data_uint32(&mflags);
		if (modlist_all) {
			printf("%*s %s\n", -MODLIST_NAME_WIDTH,
			       modlist_name(cproto_data_text(&modname), flags),
			       modlist_state(stvalue));
		}
		else if (stvalue == PMOD_STATE_RUNNING) {
			fputs(cproto_data_text(&modname), stdout);
			if (flags & PMODF_MANDATORY) {
				fputs(mark_mandatory, stdout);
			}
			putc('\n', stdout);
		}

	free_mflags:
		cproto_data_free(&mflags);

	free_state:
		cproto_data_free(&state);

	free_modname:
		cproto_data_free(&modname);
	} while (err == 0);

	return err;
}

/*
 * static const char *
 * modlist_name(const char *name, uint32_t flags)
 *	Return string which denotes module name.
 */
static const char *
modlist_name(const char *name, uint32_t flags)
{
	static char	buf[PFC_MODULE_NAME_MAX + MARK_MANDATORY_WIDTH + 1];

	if ((flags & PMODF_MANDATORY) == 0) {
		return name;
	}

	snprintf(buf, sizeof(buf), "%s%s", name, mark_mandatory);

	return buf;
}

/*
 * static const char *
 * modlist_state(uint32_t state)
 *	Return string representation of the specified module state.
 */
static const char *
modlist_state(uint32_t state)
{
	const modstate_t	*mst;
	static char	buf[32];

	for (mst = mod_states; mst < PFC_ARRAY_LIMIT(mod_states); mst++) {
		if (mst->m_state == state) {
			return mst->m_name;
		}
	}

	/* This should not happen. */
	snprintf(buf, sizeof(buf), "*** UNKNOWN: %u", state);

	return (const char *)buf;
}
