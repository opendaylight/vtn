/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_checkconf.c - "checkconf" subcommand.
 *
 * "check" subcommand checks the syntax of PFC daemon configuration files.
 * This subcommand never uses control socket.
 */

#include <unistd.h>
#include <string.h>
#include <cmdopt.h>
#include <pfc/conf.h>
#include <pfc/extcmd.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1							\
	"Check the syntax of configuration files for" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	".\nConfiguration error will be reported to the standard "	\
	"error output."

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Internal prototypes.
 */
static int	checkconf(void);

/*
 * ctrlcmd_ret_t
 * cmd_ctor_checkconf(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		      char **PFC_RESTRICT argv)
 *	Constructor of "checkconf" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_checkconf(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
		   char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_FAIL;
	int		argidx;
	char		c;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(CTRLCMD_FULLNAME(spec), argc, argv,
				 option_spec, NULL, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return CMDRET_FAIL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
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
			goto out;

		default:
			error("Failed to parse command line options.");
			goto out;
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		error("Invalid command line options.");
		goto out;
	}

	argc -= argidx;
	argv += argidx;
	if (argc != 0) {
		pfc_cmdopt_usage(parser, stderr);
		goto out;
	}

	if (PFC_EXPECT_TRUE(checkconf() == 0)) {
		printf("%sConfiguration check has been successfully "
		       "completed.\n", str_prompt_prefix);
		ret = CMDRET_COMPLETE;
	}

out:
	pfc_cmdopt_destroy(parser);

	return ret;
}

/*
 * static int
 * checkconf(void)
 *	Execute pfcd in order to check configuration files.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
checkconf(void)
{
	pfc_extcmd_t	cmd;
	pfc_refptr_t	*rpath;
	const char	*conffile = pfc_sysconf_get_path();
	const char	*path;
	int		err, sig, status;

	PFC_ASSERT(conffile != NULL);

	/* Determine path to the daemon. */
	rpath = pfc_refptr_sprintf(PFC_SBINDIR "/%s", daemon_name);
	if (PFC_EXPECT_FALSE(rpath == NULL)) {
		error("Failed to create path to the daemon.");

		return ENOMEM;
	}

	path = pfc_refptr_string_value(rpath);
	debug_printf(1, "Path = %s", path);
	err = pfc_extcmd_create(&cmd, path, daemon_name);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to create command context: %s", strerror(err));
		goto out;
	}

	err = pfc_extcmd_add_arguments(cmd, "-C", conffile, "-c", NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to add command arguments: %s", strerror(err));
		goto out_cmd;
	}

	/* Append debugging options. */
	if (ctrl_debug > 0) {
		char		dbg[] = {'-', 'd', 'd', 'd', 'd', '\0'};
		uint32_t	idx = ctrl_debug + 1;

		if (idx < PFC_ARRAY_CAPACITY(dbg)) {
			dbg[idx] = '\0';
		}

		err = pfc_extcmd_add_arguments(cmd, dbg, NULL);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to add command arguments: %s",
			      strerror(err));
			goto out_cmd;
		}
	}

	/* Execute "pfcd -c" on child process. */
	err = pfc_extcmd_execute(cmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to execute %s: %s", daemon_name, strerror(err));
		goto out_cmd;
	}

	/* Check the results. */
	sig = pfc_extcmd_get_signal(cmd);
	if (PFC_EXPECT_FALSE(!PFC_EXTCMD_ISERR(sig))) {
		err = EINTR;
		error("%s was killed by signal %d", daemon_name, sig);
		goto out_cmd;
	}

	status = pfc_extcmd_get_status(cmd);
	if (PFC_EXPECT_FALSE(status != PFC_EX_OK)) {
		err = EINVAL;
		error("Configuration check failed.");
		goto out_cmd;
	}
	else {
		PFC_ASSERT(err == 0);
	}

out_cmd:
	pfc_extcmd_destroy(cmd);

out:
	pfc_refptr_put(rpath);

	return err;
}
