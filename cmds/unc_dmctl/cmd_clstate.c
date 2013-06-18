/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_clstate.c - "clstate" subcommand.
 *
 * "clstate" shows current cluster node state.
 */

#include <string.h>
#include <cmdopt.h>
#include "unc_dmctl.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE							\
	"Show current cluster node state."

/*
 * Command line options.
 */
#define	OPTCHAR_SYSACT		's'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_SYSACT, "sysact", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Show cluster system state.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Strings which represent cluster node state.
 * Array index must be cluster event type, defined as CLSTAT_EVTYPE_XXX.
 */
static const char	*clstate_strings[] = {
	"ACT",			/* CLSTAT_EVTYPE_ACT */
};

/*
 * Strings which represent additional cluster node state.
 * Array index must be additional cluster state, defined as LNC_CLSTAT_EX_XXX.
 */
static const char	*clstate_exstrings[] = {
	"",			/* LNC_CLSTAT_EX_STABLE */
	"_TRANS",		/* LNC_CLSTAT_EX_TRANS */
	"_ERROR",		/* LNC_CLSTAT_EX_ERROR */
};

/*
 * Internal prototypes.
 */
static int	clstate_execute(pfc_bool_t do_sysact);
static int	clstate_print(lnc_clstate_t state);

/*
 * int
 * cmd_clstate(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
 *	Run "clstate" subcommand.
 */
int
cmd_clstate(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	pfc_bool_t	do_sysact = PFC_FALSE;
	int		ret = DMCTL_EX_FATAL;
	char		c;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(spec->cs_fullname, argc, argv,
				 option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return DMCTL_EX_FATAL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_SYSACT:
			do_sysact = PFC_TRUE;
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			ret = DMCTL_EX_OK;
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help(parser, stdout, HELP_MESSAGE);
			ret = DMCTL_EX_OK;
			goto out;

		case PFC_CMDOPT_ERROR:
			goto out;

		default:
			error("Failed to parse command line options.");
			goto out;
		}
	}

	if (PFC_EXPECT_FALSE(pfc_cmdopt_validate(parser) == -1)) {
		error("Invalid command line options.");
		goto out;
	}

	pfc_cmdopt_destroy(parser);
	parser = NULL;

	ret = clstate_execute(do_sysact);

out:
	if (parser != NULL) {
		pfc_cmdopt_destroy(parser);
	}

	return ret;
}

/*
 * static int
 * clstate_execute(pfc_bool_t do_sysact)
 *	Derive current cluster node state from the UNC daemon, and print it.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
clstate_execute(pfc_bool_t do_sysact)
{
	pfc_ipcsess_t	*sess;
	pfc_ipcresp_t	resp;
	int		err, ret;

	/* Create an IPC client session. */
	err = ipc_getsess(&sess, LNC_IPC_SVID_CLSTATE, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return DMCTL_EX_FATAL;
	}

	/*
	 * Invoke IPC service in the UNC daemon.
	 */
	ret = ipc_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK)) {
		goto out;
	}

	ret = clstate_print((lnc_clstate_t)resp);
	if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK)) {
		goto out;
	}

	if (do_sysact) {
		uint8_t	sysact;

		/* Print cluster system state. */
		err = pfc_ipcclnt_getres_uint8(sess, 0, &sysact);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to get cluster system state: %s",
			      strerror(err));
			ret = DMCTL_EX_FATAL;
			goto out;
		}

		printf("%s\n", (sysact) ? "TRUE" : "FALSE");
	}

out:
	PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);

	return ret;
}

/*
 * static int
 * clstate_print(lnc_clstate_t state)
 *	Print cluster node state specified by `state'.
 */
static int
clstate_print(lnc_clstate_t state)
{
	uint32_t	type, exstatus;

	PFC_ASSERT(CLSTAT_NEVTYPES == PFC_ARRAY_CAPACITY(clstate_strings));
	PFC_ASSERT(LNC_CLSTAT_EX_NUM == PFC_ARRAY_CAPACITY(clstate_exstrings));

	if (state == LNC_CLSTATE_INITIAL) {
		puts("INITIAL");

		return DMCTL_EX_OK;
	}

	type = LNC_CLSTAT_GETEVTYPE(state);
	if (PFC_EXPECT_FALSE(type >= CLSTAT_NEVTYPES)) {
		goto unexpected;
	}

	exstatus = LNC_CLSTAT_GETEXSTATUS(state);
	if (PFC_EXPECT_FALSE(exstatus >= LNC_CLSTAT_EX_NUM)) {
		goto unexpected;
	}

	printf("%s%s\n", clstate_strings[type], clstate_exstrings[exstatus]);

	return DMCTL_EX_OK;

unexpected:
	error("Unexpected cluster node state: 0x%x", state);

	return DMCTL_EX_FATAL;
}
