/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_status.c - "status" subcommand.
 *
 * "status" subcommand shows status of daemons launched by the launcher module.
 */

#include <pfc/plaintext.h>
#include <cmdopt.h>
#include <lbrk.h>
#include "unc_dmctl.h"
#include "dmconf.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE							\
	"Show status of daemons launched by the launcher module.\n"	\
	"List status of all daemons if no daemon name is specified to "	\
	"argument."

/*
 * Command line options.
 */
#define	OPTCHAR_VERBOSE		'v'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_VERBOSE, "verbose", PFC_CMDOPT_TYPE_NONE, 0,
	 "Verbose output.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Column layout.
 */
#define	COL_NAME		20
#define	COL_TYPE		12
#define	COL_CHANNEL		16
#define	COL_PID			8

static const pfc_ptcol_t	stat_header[] = {
	PFC_PTCOL_INSETS_DECL("Name", COL_NAME, 0),
	PFC_PTCOL_DECL("Type", COL_TYPE),
	PFC_PTCOL_DECL("IPC Channel", COL_CHANNEL),
	PFC_PTCOL_DECL("PID", COL_PID),
};

#define	COL_VLABEL		20
#define	COL_VVAL_POS		(COL_VLABEL + 2)
#define	COL_VSEP		60

/*
 * Number of stderr log lines to be printed out.
 */
#define	STDERR_NLINES		PFC_CONST_U(5)

/*
 * Internal prototypes.
 */
static int	status_print(pfc_ipcsess_t *sess, uint32_t count);
static int	status_print_verbose(pfc_ipcsess_t *sess, uint32_t count,
				     uint32_t verbose);
static int	status_print_verbose_proc(pfc_ipcsess_t *UNC_RESTRICT sess,
					  uint32_t *UNC_RESTRICT indexp,
					  uint32_t pid);
static void	status_print_verbose_stderr(const char *errlog,
					    uint32_t verbose);

/*
 * int
 * cmd_status(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
 *	Run "status" subcommand.
 */
int
cmd_status(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	pfc_ipcsess_t	*sess;
	pfc_ipcid_t	svid = LNC_IPC_SVID_STATUS;
	pfc_ipcresp_t	resp;
	pfc_listm_t	strlist = PFC_LISTM_INVALID;
	uint32_t	verbose = 0, count;
	int		argidx, err, ret = DMCTL_EX_FATAL;
	char		c;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(spec->cs_fullname, argc, argv,
				 option_spec, str_arg_names, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return DMCTL_EX_FATAL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_VERBOSE:
			verbose++;
			svid = LNC_IPC_SVID_VSTATUS;
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

	if (PFC_EXPECT_FALSE((argidx = pfc_cmdopt_validate(parser)) == -1)) {
		error("Invalid command line options.");
		goto out;
	}

	argc -= argidx;
	argv += argidx;
	pfc_cmdopt_destroy(parser);
	parser = NULL;

	/* Create an IPC client session. */
	err = ipc_getsess(&sess, svid, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Set daemon names to the client session. */
	err = uniqstr_create(&strlist);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out_sess;
	}
	for (; argc > 0; argc--, argv++) {
		const char	*arg = *argv;

		err = uniqstr_append(strlist, arg);
		if (err == EEXIST) {
			continue;
		}
		else if (PFC_EXPECT_FALSE(err != 0)) {
			goto out_sess;
		}

		err = pfc_ipcclnt_output_string(sess, arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to add daemon name to the client "
			      "session: %s", strerror(err));
			goto out_sess;
		}
	}

	/* Query daemon status to UNC daemon. */
	ret = ipc_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK)) {
		goto out_sess;
	}

	if (PFC_EXPECT_FALSE(resp != LNC_IPC_STATUS_OK)) {
		ret = DMCTL_EX_FATAL;
		if (resp == LNC_IPC_STATUS_ERROR) {
			const char	*emsg;

			/* The launcher module sent an error message. */
			err = pfc_ipcclnt_getres_string(sess, 0, &emsg);
			if (PFC_EXPECT_FALSE(err != 0)) {
				error("Failed to get an error message: %s",
				      strerror(err));
			}
			else {
				error("%s", emsg);
			}
		}
		else {
			error("Failed to derive daemon status from the "
			      "UNC daemon.");
		}

		goto out_sess;
	}

	count = pfc_ipcclnt_getrescount(sess);
	if (PFC_EXPECT_FALSE(count == 0)) {
		printf("%sNo daemon is configured.\n", str_prompt_prefix);
	}
	else {
		ret = (verbose) ? status_print_verbose(sess, count, verbose)
			: status_print(sess, count);
	}

out_sess:
	if (sess != NULL) {
		PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(sess), 0);
	}

out:
	if (parser != NULL) {
		pfc_cmdopt_destroy(parser);
	}
	if (strlist != PFC_LISTM_INVALID) {
		pfc_listm_destroy(strlist);
	}

	return ret;
}

/*
 * static int
 * status_print(pfc_ipcsess_t *sess, uint32_t count)
 *	Print daemon status sent by UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
status_print(pfc_ipcsess_t *sess, uint32_t count)
{
	uint32_t	i;

	pfc_ptcol_print_header(stdout, stat_header,
			       PFC_ARRAY_CAPACITY(stat_header));

	for (i = 0; i < count; i++) {
		lncipc_dmstat_t	stat;
		const char	*channel;
		char		pidbuf[32];
		uint32_t	pid;
		int		err;

		err = PFC_IPCCLNT_GETRES_STRUCT(sess, i, lncipc_dmstat, &stat);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to fetch daemon status at index %u: %s",
			      i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		channel = (stat.ids_channel[0] == '\0')
			? str_hyphen : (const char *)stat.ids_channel;
		if ((pid = stat.ids_pid) == 0) {
			pidbuf[0] = '-';
			pidbuf[1] = '\0';
		}
		else {
			snprintf(pidbuf, sizeof(pidbuf), "%u", pid);
		}
		printf("%*s %*s %*s %*s\n",
		       COL_NAME, stat.ids_name,
		       COL_TYPE, proctype_getname(stat.ids_type),
		       COL_CHANNEL, channel,
		       COL_PID, pidbuf);
	}

	return DMCTL_EX_OK;
}

/*
 * static int
 * status_print_verbose(pfc_ipcsess_t *sess, uint32_t count, uint32_t verbose)
 *	Print verbose daemon status sent by UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
status_print_verbose(pfc_ipcsess_t *sess, uint32_t count, uint32_t verbose)
{
	pfc_bool_t	first = PFC_TRUE;
	pfc_lbrk_t	lb;
	uint32_t	i = 0;

	pfc_lbrk_init(&lb, stdout, DMCTL_COL_WIDTH);

	for (;;) {
		lncipc_dmstat_t	stat;
		const char	*channel, *cfpath, *desc, *errlog;
		char		label[COL_VLABEL + 3];
		int		err, ret;

		err = PFC_IPCCLNT_GETRES_STRUCT(sess, i, lncipc_dmstat, &stat);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to fetch daemon status at index %u: %s",
			      i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		i++;
		err = pfc_ipcclnt_getres_string(sess, i, &cfpath);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to fetch configuration file path at "
			      "index %u: %s", i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		i++;
		err = pfc_ipcclnt_getres_string(sess, i, &desc);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to fetch description at index %u: %s",
			      i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		i++;
		err = pfc_ipcclnt_getres_string(sess, i, &errlog);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to fetch stderr logs at index %u: %s",
			      i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		if (!first) {
			int	cnt;

			for (cnt = 0; cnt < COL_VSEP; cnt++) {
				putchar('-');
			}
			putchar('\n');
		}
		first = PFC_FALSE;

		printf("%*s: %s\n", COL_VLABEL, "Name", stat.ids_name);

		snprintf(label, sizeof(label), "%*s: ",
			 COL_VLABEL, "Description");
		pfc_lbrk_print_raw(&lb, label);
		pfc_lbrk_print(&lb, COL_VVAL_POS, -1, desc, 0);
		pfc_lbrk_newline(&lb);

		printf("%*s: %s\n", COL_VLABEL, "Process Type",
		       proctype_getname(stat.ids_type));
		printf("%*s: %s\n", COL_VLABEL, "Configuration File", cfpath);

		channel = (stat.ids_channel[0] == '\0')
			? str_hyphen : (const char *)stat.ids_channel;
		printf("%*s: %s\n", COL_VLABEL, "IPC Channel Name", channel);

		ret = status_print_verbose_proc(sess, &i, stat.ids_pid);
		if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK)) {
			return ret;
		}

		status_print_verbose_stderr(errlog, verbose);

		if (i == count) {
			break;
		}
	}

	return DMCTL_EX_OK;
}

/*
 * static int
 * status_print_verbose_proc(pfc_ipcsess_t *UNC_RESTRICT sess,
 *			     uint32_t *UNC_RESTRICT indexp, uint32_t pid)
 *	Print process status in verbose daemon status.
 *	`indexp' must points uint32_t which keeps additional data index to
 *	process start time (tv_sec).
 *
 * Calling/Exit State:
 *	Upon successful completion, additional data index for next data is
 *	set to the buffer pointed by `indexp', and DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
status_print_verbose_proc(pfc_ipcsess_t *UNC_RESTRICT sess,
			  uint32_t *UNC_RESTRICT indexp, uint32_t pid)
{
	uint64_t	sec, nsec;
	uint32_t	argc, i = *indexp, j;
	pfc_timespec_t	ts;
	char		timebuf[48];
	int		err;

	if (pid == 0) {
		/* Daemon process is not running. */
		*indexp = i + 1;

		return DMCTL_EX_OK;
	}

	i++;
	err = pfc_ipcclnt_getres_uint64(sess, i, &sec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to fetch start time (sec) at index %u: %s",
		      i, strerror(err));

		return DMCTL_EX_FATAL;
	}

	i++;
	err = pfc_ipcclnt_getres_uint64(sess, i, &nsec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to fetch start time (nsec) at index %u: %s",
		      i, strerror(err));

		return DMCTL_EX_FATAL;
	}

	i++;
	err = pfc_ipcclnt_getres_uint32(sess, i, &argc);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to fetch the number of arguments at index %u: "
		      "%s", i, strerror(err));

		return DMCTL_EX_FATAL;
	}

	printf("%*s: %u\n", COL_VLABEL, "Process ID", pid);

	ts.tv_sec = sec;
	ts.tv_nsec = nsec;
	err = pfc_time_clock_asctime(&ts, timebuf, sizeof(timebuf));
	if (PFC_EXPECT_FALSE(err != 0)) {
		snprintf(timebuf, sizeof(timebuf), "%" PFC_PFMT_u64 ".%"
			 PFC_PFMT_u64, sec, nsec);
	}

	printf("%*s: %s\n", COL_VLABEL, "Since", timebuf);

	for (j = 0; j < argc; j++) {
		char		label[COL_VLABEL + 3];
		const char	*arg;

		i++;
		err = pfc_ipcclnt_getres_string(sess, i, &arg);
		if (PFC_EXPECT_FALSE(err != 0)) {
			fflush(stdout);
			fputc('\n', stderr);
			error("Failed to fetch process argument at index %u: "
			      "%s\n", i, strerror(err));

			return DMCTL_EX_FATAL;
		}

		snprintf(label, sizeof(label), "Argv[%u]", j);
		printf("%*s: %s\n", COL_VLABEL, label, arg);
	}

	*indexp = i + 1;

	return DMCTL_EX_OK;
}

/*
 * static void
 * status_print_verbose_stderr(const char *errlog, uint32_t verbose)
 *	Print stderr logs.
 */
static void
status_print_verbose_stderr(const char *errlog, uint32_t verbose)
{
	const char	*p, *label = "Standard Error Logs";
	size_t		len;
	uint32_t	cnt;
	pfc_bool_t	newline = PFC_FALSE;

	if (errlog == NULL) {
		return;
	}

	len = strlen(errlog);
	p = errlog + len - 1;
	if (*p == '\n') {
		p--;
		if (p == errlog) {
			return;
		}

		newline = PFC_TRUE;
	}

	if (verbose > 1) {
		printf("\n%s:\n", label);
		fputs(errlog, stdout);
		if (!newline) {
			putchar('\n');
		}

		return;
	}


	for (cnt = 0; p >= errlog; p--) {
		if (*p == '\n') {
			cnt++;
			if (cnt >= STDERR_NLINES) {
				break;
			}
		}
	}

	p++;
	printf("\n%s (last %u lines):\n", label, STDERR_NLINES);
	fputs(p, stdout);
	if (!newline) {
		putchar('\n');
	}
}
