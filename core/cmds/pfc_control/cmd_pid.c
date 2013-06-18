/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_pid.c - "pid" subcommand.
 *
 * "pid" subcommand shows the process ID of the PFC daemon.
 * This subcommand never uses control socket.
 */

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <cmdopt.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1					\
	"Display process ID of" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2							\
	".\nIf '-c' option is specified, command line arguments are also " \
	"displayed."

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{'c', "command-line", PFC_CMDOPT_TYPE_NONE, 0,
	 "Show command line arguments.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Internal prototypes.
 */
static int	pid_show(pfc_pidf_t pf, pfc_bool_t do_cmdline);

/*
 * ctrlcmd_ret_t
 * cmd_ctor_pid(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		 char **PFC_RESTRICT argv)
 *	Constructor of "pid" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_pid(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
	     char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_COMPLETE;
	pfc_pidf_t	pf;
	pfc_bool_t	do_cmdline = PFC_FALSE;
	int		argidx, err;
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
		case 'c':
			do_cmdline = PFC_TRUE;
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     NULL);
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

	/* Open PID file in read-only mode. */
	err = open_pidfile(&pf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
		goto out;
	}

	/* Show daemon's PID. */
	err = pid_show(pf, do_cmdline);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
	}
	pidfile_close(pf);

out:
	pfc_cmdopt_destroy(parser);

	return ret;
}

/*
 * static int
 * pid_show(pfc_pidf_t pf, pfc_bool_t do_cmdline)
 *	Print the PFC daemon's process ID to the standard output.
 *	If `do_cmdline' is true, command line arguments of the PFC daemon are
 *	also printed.
 */
static int
pid_show(pfc_pidf_t pf, pfc_bool_t do_cmdline)
{
	pid_t	pid;
	int	err;

	err = get_daemon_pid(pf, &pid);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	printf("%u\n", pid);

	if (do_cmdline) {
		pfc_listm_t	list;
		int		index, size;

		/* Show command line arguments. */
		err = pfc_proc_getcmdline(pid, &list);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to obtain command line arguments: %s",
			      strerror(err));

			return err;
		}

		size = pfc_listm_get_size(list);
		for (index = 0; index < size; index++) {
			pfc_cptr_t	v;
			pfc_refptr_t	*rstr;

			PFC_ASSERT_INT(pfc_listm_getat(list, index, &v), 0);
			rstr = (pfc_refptr_t *)v;
			if (index != 0) {
				putc(' ', stdout);
			}
			fputs(pfc_refptr_string_value(rstr), stdout);
		}
		putc('\n', stdout);
		pfc_listm_destroy(list);
	}

	return 0;
}
