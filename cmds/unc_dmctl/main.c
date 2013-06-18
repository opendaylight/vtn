/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * main.c - Main routine of unc_dmctl command.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <pfc/refptr.h>
#include <pfc/extcmd.h>
#include <pfc/log.h>
#include <cmdopt.h>
#include <conf_impl.h>
#include "unc_dmctl.h"
#include "dmconf.h"

extern const char	copyright[];

/*
 * Program name.
 */
#define	PROGNAME		"unc_dmctl"

/*
 * Default path to the configuration file.
 */
#define	UNCD_CONF_PATH		UNC_SYSCONFDIR "/uncd.conf"

/*
 * Path to unc_control command.
 */
#define	UNC_CONTROL		"unc_control"
#define	UNC_CONTROL_PATH	UNC_BINDIR "/" UNC_CONTROL

/*
 * I/O timeout on IPC client session.
 */
#define	IPC_TIMEOUT_DEF		5
#define	IPC_TIMEOUT_MIN		PFC_CONST_U(1)
#define	IPC_TIMEOUT_MAX		PFC_CONST_U(3600)

uint32_t	ipc_timeout;

/*
 * IPC channel address to contact with UNC daemon.
 */
const char	*ipc_channel;

/*
 * Shared string literals.
 */
const char	str_empty[] = "";
const char	str_hyphen[] = "-";
const char	str_file[] = "FILE";
const char	str_secs[] = "SECS";
const char	str_msecs[] = "MSECS";
const char	str_unknown[] = "<unknown>";
const char	str_prompt_prefix[] = "--- ";
const char	str_true[] = "True";
const char	str_false[] = "False";
const char	str_arg_names[] = "[NAME ...]";
const char	str_arg_event[] = "EVENT";
const char	str_err_EPERM[] =
	"You are not allowed to use this subcommand.";

/*
 * String representation of process type.
 */
static const char	*proctype_names[] = {
	"UNSPEC",
	"LOGICAL",
	"PHYSICAL",
	"DRIVER",
};

/*
 * Help message.
 */
#define	HELP_MESSAGE	\
	"Control daemon processes configured to UNC daemon.\n"		\
	PROGNAME " requires one subcommand name as argument.\n"		\
	"Available subcommands are listed if no subcommand is "		\
	"specified.\n\n"						\
	"Type '" PROGNAME " <subcommand> --help' for help on a "	\
	"specific subcommand."

/*
 * Command line options.
 */
#define	OPTCHAR_CONF		'C'
#define	OPTCHAR_TIMEOUT		'T'
#define	OPTCHAR_CHANNEL		'c'
#define	OPTCHAR_DEBUG		'd'
#define	OPTCHAR_VERSION		'\v'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_CONF, "conf-file", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE,
	 "Specify path to configuration file.\n(default: "
	 UNCD_CONF_PATH ")", str_file},
	{OPTCHAR_TIMEOUT, "timeout", PFC_CMDOPT_TYPE_UINT32,
	 PFC_CMDOPT_DEF_ONCE,
	 "Specify I/O timeout in seconds.\n(default: "
	 PFC_XSTRINGIFY(IPC_TIMEOUT_DEF) " seconds)", str_secs},
	{OPTCHAR_CHANNEL, "channel-addr", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_HIDDEN, NULL, NULL},
	{OPTCHAR_DEBUG, "debug", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_HIDDEN, NULL, NULL},
	{OPTCHAR_VERSION, "version", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_LONGONLY,
	 "Print version information and exit.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Brief description of arguments.
 */
static const char	arg_format[] = "subcommand [...]";

/*
 * Define subcommands.
 */
#define	CMDSPEC_DECL(name, timeout, flags)		\
	{						\
		.cs_name	= #name,		\
		.cs_fullname	= PROGNAME " " #name,	\
		.cs_timeout	= timeout,		\
		.cs_flags	= flags,		\
		.cs_func	= cmd_##name,		\
	}

#define	CMDSPEC_IPC_DECL(name)				\
	CMDSPEC_DECL(name, IPC_TIMEOUT_DEF, 0)
#define	CMDSPEC_NOIPC_DECL(name)			\
	CMDSPEC_DECL(name, IPC_TIMEOUT_DEF, CMDF_NOIPC)
#define	CMDSPEC_HIDDEN_DECL(name, timeout)		\
	CMDSPEC_DECL(name, timeout, CMDF_HIDDEN)

static cmdspec_t	subcommand_spec[] = {
	CMDSPEC_NOIPC_DECL(list),
	CMDSPEC_IPC_DECL(status),
	CMDSPEC_HIDDEN_DECL(clevent, CLEVENT_TIMEOUT),
	CMDSPEC_IPC_DECL(clstate),
};

static cmdspec_t	*current_cmd;

/*
 * Debugging level.
 */
uint32_t	debug_level;

/*
 * Internal prototypes.
 */
static void		fatal(const char *fmt, ...) PFC_FATTR_PRINTFLIKE(1, 2)
	PFC_FATTR_NORETURN;
static void		print_error(const char *UNC_RESTRICT label,
				    const char *UNC_RESTRICT fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0);
static void		dump_version(void) PFC_FATTR_NORETURN;
static void		dump_subcommands(pfc_cmdopt_t *parser)
	PFC_FATTR_NORETURN;
static void		signal_init(void);
static cmdspec_t	*subcmd_find(const char *name);
static void		path_verify(const char *path);
static void		sysconf_init(const char *path);

/*
 * int
 * main(int argc, char **argv)
 *	Start routine of unc_dmctl command.
 */
int
main(int argc, char **argv)
{
	pfc_cmdopt_t	*parser;
	cmdspec_t	*spec;
	const char	*conffile = UNCD_CONF_PATH;
	int		argidx, ret;
	char		c;

	/* Use C locale. */
	(void)setlocale(LC_ALL, "C");

	/* Set timezone. */
	tzset();

	/* Initialize signal. */
	signal_init();

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(PROGNAME, argc, argv, option_spec,
				 arg_format, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		fatal("Failed to create option parser.");
		/* NOTREACHED */
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_CONF:
			conffile = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*conffile == '\0')) {
				fatal("Configuration file path is empty.");
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_TIMEOUT:
			ipc_timeout = pfc_cmdopt_arg_uint32(parser);
			if (PFC_EXPECT_FALSE(ipc_timeout <
					     IPC_TIMEOUT_MIN)) {
				fatal("-%c: Timeout value must be greater than "
				      "or equal %u.", OPTCHAR_TIMEOUT,
				      IPC_TIMEOUT_MIN);
				/* NOTREACHED */
			}
			else if (PFC_EXPECT_FALSE(ipc_timeout >
						  IPC_TIMEOUT_MAX)) {
				fatal("-%c: Timeout value must be less than "
				      "or equal %u.", OPTCHAR_TIMEOUT,
				      IPC_TIMEOUT_MAX);
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_CHANNEL:
			ipc_channel = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*ipc_channel == '\0')) {
				fatal("-%c: IPC channel name must not be "
				      "empty.", OPTCHAR_CHANNEL);
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_DEBUG:
			debug_level++;
			break;

		case OPTCHAR_VERSION:
			dump_version();
			/* NOTREACHED */

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			exit(DMCTL_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help(parser, stdout, HELP_MESSAGE);
			exit(DMCTL_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			exit(DMCTL_EX_FATAL);
			/* NOTREACHED */

		default:
			fatal("Failed to parse command line options.");
			/* NOTREACHED */
		}
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		fatal("Invalid command line options.");
		/* NOTREACHED */
	}

	argc -= argidx;
	argv += argidx;
	if (argc == 0) {
		dump_subcommands(parser);
		/* NOTREACHED */
	}
	pfc_cmdopt_destroy(parser);

	if (debug_level) {
		pfc_log_conf_t	cf;
		pfc_log_level_t	lvl = (debug_level == 1)
			? PFC_LOGLVL_DEBUG
			: PFC_LOGLVL_VERBOSE;

		pfc_logconf_early(&cf, PFC_CFBLK_INVALID, PROGNAME, stderr,
				  lvl, NULL);
		pfc_log_sysinit(&cf);
	}

	/* Determine subcommand. */
	spec = subcmd_find(*argv);
	if (PFC_EXPECT_FALSE(spec == NULL)) {
		fatal("Unknown subcommand: %s", *argv);
		/* NOTREACHED */
	}

	if (ipc_timeout == 0) {
		/* Use default timeout. */
		ipc_timeout = spec->cs_timeout;
	}

	/* Load system configuration file. */
	sysconf_init(conffile);

	if ((spec->cs_flags & CMDF_NOIPC) == 0) {
		int	err;

		/* Initialize IPC client. */
		err = ipc_init();
		if (PFC_EXPECT_FALSE(err != 0)) {
			return DMCTL_EX_FATAL;
		}
	}

	/* Dispatch subcommand. */
	current_cmd = spec;
	ret = spec->cs_func(spec, argc, argv);
	current_cmd = NULL;

	/* Tear down the command. */
	if ((spec->cs_flags & CMDF_NOIPC) == 0) {
		ipc_fini();
	}
	dmconf_cleanup();

	return ret;
}

/*
 * void
 * error(const char *fmt, ...)
 *	Print error message to the standard error output.
 */
void
error(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	print_error("ERROR", fmt, ap);
	va_end(ap);
}

/*
 * void
 * warning(const char *fmt, ...)
 *	Print warning message to the standard error output.
 */
void
warning(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	print_error("WARNING", fmt, ap);
	va_end(ap);
}

/*
 * void
 * debug_printf(uint32_t level, const char *fmt, ...)
 *	Print debugging message if current debugging level is higher than or
 *	equal to the specified level.
 */
void
debug_printf(uint32_t level, const char *fmt, ...)
{
	PFC_ASSERT(level > 0);

	if (level <= debug_level) {
		va_list		ap;
		uint32_t	i;

		for (i = 0; i < level; i++) {
			putc('*', stderr);
		}
		putc(' ', stderr);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		putc('\n', stderr);
	}
}

/*
 * int
 * uncd_stop(void)
 *	Stop the UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
uncd_stop(void)
{
	pfc_extcmd_t	ecmd;
	int		err, status;

	err = pfc_extcmd_create(&ecmd, UNC_CONTROL_PATH, UNC_CONTROL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to create command context for %s: %s",
		      UNC_CONTROL, strerror(err));

		return err;
	}

	err = pfc_extcmd_add_arguments(ecmd, "stop", "-q", NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to append arguments for %s: %s",
		      UNC_CONTROL, strerror(err));
		goto out;
	}

	err = pfc_extcmd_execute(ecmd);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to execute %s: %s", UNC_CONTROL, strerror(err));
		goto out;
	}

	status = pfc_extcmd_get_status(ecmd);
	if (PFC_EXPECT_FALSE(status != 0)) {
		if (status == PFC_EXTCMD_ERR_UNSPEC) {
			int	sig = pfc_extcmd_get_signal(ecmd);

			PFC_ASSERT(sig > 0);
			error("%s was killed by signal %d.", UNC_CONTROL, sig);
		}
		else {
			PFC_ASSERT(status > 0);
			error("%s exited with status %d.", UNC_CONTROL,
			      status);
			err = EIO;
		}
	}

out:
	PFC_ASSERT_INT(pfc_extcmd_destroy(ecmd), 0);

	return err;
}

/*
 * const char *
 * proctype_getname(uint32_t type)
 *	Return string representation of the given process type.
 */
const char *
proctype_getname(uint32_t type)
{
	if (PFC_EXPECT_FALSE(type >= PFC_ARRAY_CAPACITY(proctype_names))) {
		static char	buf[32];

		/* This should never happen. */
		snprintf(buf, sizeof(buf), "%s (%u)", str_unknown, type);

		return buf;
	}

	return proctype_names[type];
}

/*
 * static void
 * fatal(const char *fmt, ...)
 *	Print error message to the standard error output and die.
 */
static void
fatal(const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s: ERROR: ", PROGNAME);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);

	exit(DMCTL_EX_FATAL);
	/* NOTREACHED */
}

/*
 * static void
 * print_error(const char *UNC_RESTRICT label, const char *UNC_RESTRICT fmt,
 *	       va_list ap)
 *	Print message with label to the standard error output.
 */
static void
print_error(const char *UNC_RESTRICT label, const char *UNC_RESTRICT fmt,
	    va_list ap)
{
	cmdspec_t	*spec = current_cmd;

	fprintf(stderr, "*** %s", PROGNAME);
	if (spec != NULL) {
		fprintf(stderr, ": %s", spec->cs_name);
	}
	fprintf(stderr, ": %s: ", label);
	vfprintf(stderr, fmt, ap);
	putc('\n', stderr);
}

/*
 * static void
 * dump_version(void)
 *	Dump system version and exit.
 */
static void
dump_version(void)
{
	printf("%s (%s) %s%s\n\n%s\n",
	       PROGNAME, UNC_PRODUCT_NAME,
	       UNC_VERSION_STRING, UNC_BUILD_TYPE_SUFFIX, copyright);

	exit(DMCTL_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * dump_subcommands(pfc_cmdopt_t *parser)
 *	Dump available subcommands and exit.
 */
static void
dump_subcommands(pfc_cmdopt_t *parser)
{
	cmdspec_t	*spec;

	pfc_cmdopt_usage(parser, stdout);

	printf("\nSubcommands:\n");
	for (spec = subcommand_spec; spec < PFC_ARRAY_LIMIT(subcommand_spec);
	     spec++) {
		if ((spec->cs_flags & CMDF_HIDDEN) == 0) {
			printf("    %s\n", spec->cs_name);
		}
	}

	exit(DMCTL_EX_OK);
	/* NOTREACHED */
}

/*
 * static void
 * signal_init(void)
 *	Initialize signal configuration.
 */
static void
signal_init(void)
{
	struct sigaction	act;
	sigset_t	mask;

	/* Ignore SIGPIPE. */
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (PFC_EXPECT_FALSE(sigaction(SIGPIPE, &act, NULL) != 0)) {
		fatal("sigaction(SIGPIPE) failed: %s", strerror(errno));
		/* NOTREACHED */
	}

	/* Reset SIGCHLD. */
	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (PFC_EXPECT_FALSE(sigaction(SIGCHLD, &act, NULL) != 0)) {
		fatal("sigaction(SIGCHLD) failed: %s", strerror(errno));
		/* NOTREACHED */
	}

	/* Unmask all signals. */
	sigemptyset(&mask);
	if (PFC_EXPECT_FALSE(sigprocmask(SIG_SETMASK, &mask, NULL) != 0)) {
		fatal("sigprocmask() failed: %s", strerror(errno));
		/* NOTREACHED */
	}
}

/*
 * static cmdspec_t *
 * subcmd_find(const char *name)
 *	Search for a subcommand definition associated with the specified name.
 */
static cmdspec_t *
subcmd_find(const char *name)
{
	cmdspec_t	*spec;

	for (spec = subcommand_spec; spec < PFC_ARRAY_LIMIT(subcommand_spec);
	     spec++) {
		if (strcmp(spec->cs_name, name) == 0) {
			return spec;
		}
	}

	return NULL;
}

/*
 * static void
 * path_verify(const char *path)
 *	Ensure that the specified file path can be used safely.
 *
 * Calling/Exit State:
 *	Program exits on error.
 */
static void
path_verify(const char *path)
{
	char	*cpath;
	int	err;

	/*
	 * We need to copy the specified path because pfc_is_safepath()
	 * always breaks the string.
	 */
	cpath = strdup(path);
	if (PFC_EXPECT_FALSE(cpath == NULL)) {
		fatal("Failed to duplicate file path.");
		/* NOTREACHED */
	}

	err = pfc_is_safepath(cpath);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			fatal("%s: File does not exist.", path);
		}
		else {
			fatal("%s: File is unsafe.: %s: %s",
			      path, cpath, strerror(err));
		}
		/* NOTREACHED */
	}
	free(cpath);
}

/*
 * static void
 * sysconf_init(const char *path)
 *	Import system configuration from the given file.
 */
static void
sysconf_init(const char *path)
{
	pfc_refptr_t	*rconf;
	int		err;

	/* Ensure that the configuration file is safe. */
	path_verify(path);

	rconf = pfc_refptr_string_create(path);
	if (PFC_EXPECT_FALSE(rconf == NULL)) {
		fatal("Failed to duplicate configuration file path.");
		/* NOTREACHED */
	}

	/* Load PFC system configuration file. */
	err = pfc_sysconf_init(rconf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		fatal("Failed to load system configuration file: %s",
		      strerror(err));
		/* NOTREACHED */
	}
	pfc_refptr_put(rconf);
}
