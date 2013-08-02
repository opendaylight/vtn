/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * main.c - Main routine of pfc_control command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <pfc/path.h>
#include <cmdopt.h>
#include <cmdutil.h>
#include <conf_impl.h>
#include "pfc_control.h"
#include "ipc.h"

extern const char	copyright[];

/*
 * Program name.
 */
static const char	progname_default[] = "pfc_control";
static const char	*progname = progname_default;

/*
 * The name of the target daemon.
 */
static const char	daemon_name_default[] = "pfcd";
const char		*daemon_name = daemon_name_default;

/*
 * Message prefix.
 */
static const char	*msg_prefix = progname_default;

/*
 * Debugging level.
 */
int		ctrl_debug;

/*
 * Working directory for PFC daemon.
 */
const char	*pfcd_work_dir;

/*
 * PID file path of PFC daemon.
 */
const char	*pfcd_pidfile;

/*
 * Shared string literals.
 */
const char	str_timeout[] = "TIMEOUT";
const char	str_number[] = "NUMBER";
const char	str_size[] = "SIZE";
const char	str_interval[] = "INTERVAL";
const char	str_empty[] = "";
const char	str_comma[] = ",";
const char	str_s[] = "s";
const char	str_prompt_prefix[] = "--- ";

/*
 * I/O timeout on control protocol session.
 */
#define	CTRL_TIMEOUT_DEF	5U
#define	CTRL_TIMEOUT_MIN	1U
#define	CTRL_TIMEOUT_MAX	3600U

uint32_t	ctrl_timeout = CTRL_TIMEOUT_DEF;

/*
 * Error status of pfc_control command.
 */
int		ctrl_err_status = PFC_EX_FATAL;

/*
 * Hook which will be called if the PFC daemon doesn't exist.
 */
void		(*not_running_hook)(void);

/*
 * Global option block handle.
 */
pfc_cfblk_t	pfcd_options;

/*
 * Help message.
 */
#define	HELP_MESSAGE_1		"Control" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2		".\n"
#define	HELP_MESSAGE_3							\
	PFC_LBRK_SPACE_STR "requires one subcommand name as argument.\n" \
	"Available subcommands are listed if no subcommand is "		\
	"specified.\n\n"						\
	"Type '"
#define	HELP_MESSAGE_4							\
	PFC_LBRK_SPACE_STR "<subcommand> --help' for help on a "	\
	"specific subcommand."

/*
 * Command line options.
 */
static const char	*copt_desc[] = {
	"Specify path to configuration file.\n(default: ",
	PFC_PFCD_CONF_PATH,
	")",
	NULL,
};

#define	OPTCHAR_CONF		'C'
#define	OPTCHAR_TIMEOUT		'T'
#define	OPTCHAR_DEBUG		'd'
#define	OPTCHAR_CHADDR		'a'
#define	OPTCHAR_VERSION		'\v'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_CONF, "conf-file", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_ARRAYDESC,
	 (const char *)copt_desc, "FILE"},
	{OPTCHAR_TIMEOUT, "timeout", PFC_CMDOPT_TYPE_UINT32,
	 PFC_CMDOPT_DEF_ONCE,
	 "How long, in seconds, we should wait for response on the control "
	 "protocol session.\n"
	 "(default: 5 seconds)", str_timeout},

	/* Run with debugging message. */
	{OPTCHAR_DEBUG, "debug", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_HIDDEN,
	 NULL, NULL},

	/* Specify IPC channel address. */
	{OPTCHAR_CHADDR, "channel-address", PFC_CMDOPT_TYPE_STRING,
	 PFC_CMDOPT_DEF_HIDDEN | PFC_CMDOPT_DEF_ONCE,
	 NULL, NULL},

	{OPTCHAR_VERSION, "version", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_LONGONLY,
	 "Print version information and exit.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Brief description of arguments.
 */
static const char	ctrl_arg_format[] = "subcommand [...]";

/*
 * Internal prototypes.
 */
static pfc_bool_t	check_privilege(void);
static ctrlcmd_spec_t	*subcmd_find(const char *name);
static ctrlcmd_ret_t	subcmd_dispatch(ctrlcmd_spec_t *PFC_RESTRICT spec,
					int argc, char **PFC_RESTRICT argv);

static void	signal_init(void);
static void	path_verify(const char *path);
static void	sysconf_init(const char *path);
static void	debug_vprintf(int level, const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(2, 0);
static void	dump_version(void) PFC_FATTR_NORETURN;
static void	dump_subcommands(pfc_cmdopt_t *parser) PFC_FATTR_NORETURN;

#define	CTRLCMD_SPEC_DECL(name, cmd, flags)			\
	{							\
		.cs_name	= #name,			\
		.cs_fullname	= NULL,				\
		.cs_command	= (cmd),			\
		.cs_flags	= (flags),			\
		.cs_ctor	= cmd_ctor_##name,		\
		.cs_dtor	= cmd_dtor_##name,		\
		.cs_prepare	= cmd_prepare_##name,		\
		.cs_send	= cmd_send_##name,		\
		.cs_receive	= cmd_receive_##name,		\
		.cs_iowait	= cmd_iowait_##name,		\
	}

#define	CTRLCMD_SPEC_PRIV_DECL(name, cmd)			\
	CTRLCMD_SPEC_DECL(name, cmd, CTRLCMDF_NEED_PRIV)

/*
 * Subcommands.
 */
static ctrlcmd_spec_t	subcommand_spec[] = {
	CTRLCMD_SPEC_DECL(checkconf, CTRL_CMDTYPE_NOP, 0),
	CTRLCMD_SPEC_PRIV_DECL(event, CTRL_CMDTYPE_EVENT),
	CTRLCMD_SPEC_PRIV_DECL(loglevel, CTRL_CMDTYPE_LOGLEVEL),
	CTRLCMD_SPEC_PRIV_DECL(modlist, CTRL_CMDTYPE_MODLIST),
	CTRLCMD_SPEC_DECL(pid, CTRL_CMDTYPE_NOP, 0),
	CTRLCMD_SPEC_PRIV_DECL(ping, CTRL_CMDTYPE_NOP),
	CTRLCMD_SPEC_PRIV_DECL(stop, CTRL_CMDTYPE_NOP),
};

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * copt_desc_update(const char *conffile)
 *	Set default configuration file path into description about "-C" option.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
copt_desc_update(const char *conffile)
{
	copt_desc[1] = conffile;
}

/*
 * int
 * main(int argc, char **argv)
 *	Start routine of pfc_control command.
 */
int
main(int argc, char **argv)
{
	ctrlcmd_spec_t	*spec;
	pfc_cmdopt_t	*parser;
	const char	*conffile, *chaddr = NULL;
	pfc_refptr_t	*defconf, *dname;
	ctrlcmd_ret_t	ret;
	int		argidx, status;
	char		c;

	/* Use C locale. */
	(void)setlocale(LC_ALL, "C");

	/* Set timezone. */
	tzset();

	if (PFC_EXPECT_TRUE(argc > 0)) {
		/* Initialize program name. */
		dname = progname_init(*argv, &progname);
		if (PFC_EXPECT_FALSE(dname == NULL)) {
			fatal("Failed to create daemon name.");
			/* NOTREACHED */
		}

		daemon_name = pfc_refptr_string_value(dname);
	}
	else {
		dname = NULL;
	}

	/* Construct default configuration file path. */
	defconf = pfc_refptr_sprintf(PFC_SYSCONFDIR "/%s.conf", daemon_name);
	if (PFC_EXPECT_FALSE(defconf == NULL)) {
		fatal("Failed to create default configuration file path.");
		/* NOTREACHED */
	}

	conffile = pfc_refptr_string_value(defconf);
	copt_desc_update(conffile);

	/* Initialize signal. */
	signal_init();

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(progname, argc, argv, option_spec,
				 ctrl_arg_format, 0);
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
			ctrl_timeout = pfc_cmdopt_arg_uint32(parser);
			if (PFC_EXPECT_FALSE(ctrl_timeout <
					     CTRL_TIMEOUT_MIN)) {
				fatal("-T: Timeout value must be greater than "
				      "or equal %u.", CTRL_TIMEOUT_MIN);
				/* NOTREACHED */
			}
			else if (PFC_EXPECT_FALSE(ctrl_timeout >
						  CTRL_TIMEOUT_MAX)) {
				fatal("-T: Timeout value must be less than "
				      "or equal %u.", CTRL_TIMEOUT_MAX);
				/* NOTREACHED */
			}
			break;

		case OPTCHAR_DEBUG:
			/* Run as debug mode. */
			ctrl_debug++;
			break;

		case OPTCHAR_CHADDR:
			/* Specify IPC channel address. */
			chaddr = pfc_cmdopt_arg_string(parser);
			break;

		case OPTCHAR_VERSION:
			dump_version();
			/* NOTREACHED */

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help_list(parser, stdout, HELP_MESSAGE_1,
					     daemon_name, HELP_MESSAGE_2,
					     progname, HELP_MESSAGE_3,
					     progname, HELP_MESSAGE_4, NULL);
			exit(PFC_EX_OK);
			/* NOTREACHED */

		case PFC_CMDOPT_ERROR:
			exit(PFC_EX_FATAL);
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

	spec = subcmd_find(*argv);
	if (PFC_EXPECT_FALSE(spec == NULL)) {
		fatal("Unknown subcommand: %s", *argv);
		/* NOTREACHED */
	}

	/* Load system configuration file. */
	sysconf_init(conffile);
	pfc_refptr_put(defconf);

	/* Initialize IPC client. */
	ipc_init(chaddr);

	/* Initialize control protocol client. */
	pfc_ctrl_client_init(pfcd_work_dir, error_v);

	if (ctrl_debug) {
		pfc_ctrl_debug(debug_vprintf);
	}

	/* Dispatch subcommand. */
	status = PFC_EX_OK;
	ret = subcmd_dispatch(spec, argc, argv);
	if (PFC_EXPECT_TRUE(ret == CMDRET_COMPLETE)) {
		status = PFC_EX_OK;
	}
	else {
		status = ctrl_err_status;
	}

	if (PFC_EXPECT_TRUE(dname != NULL)) {
		daemon_name = daemon_name_default;
		pfc_refptr_put(dname);
	}

	return status;
}

/*
 * void
 * fatal(const char *fmt, ...)
 *	Print error message to the standard error output and die.
 */
void
fatal(const char *fmt, ...)
{
	va_list	ap;

	fprintf(stderr, "*** %s: ERROR: ", msg_prefix);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);

	exit(PFC_EX_FATAL);
	/* NOTREACHED */
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

	fprintf(stderr, "*** %s: ERROR: ", msg_prefix);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);
}

/*
 * void
 * error_v(const char *fmt, va_list ap)
 *	Print error message to the standard error output using va_list
 */
void
error_v(const char *fmt, va_list ap)
{
	fprintf(stderr, "*** %s: ERROR: ", msg_prefix);
	vfprintf(stderr, fmt, ap);
	putc('\n', stderr);
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

	fprintf(stderr, "*** %s: WARNING: ", msg_prefix);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);
}

/*
 * void
 * debug_printf(int level, const char *fmt, ...)
 *	Print debugging message if current debugging level is higher than or
 *	equal to the specified level.
 */
void
debug_printf(int level, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	debug_vprintf(level, fmt, ap);
	va_end(ap);
}

/*
 * void
 * not_running_ipc(const char *chaddr)
 *	Print error message that informs the PFC daemon is not running.
 *
 *	`chaddr' is an IPC channel address provided by the PFC daemon.
 *	If it is not NULL, the channe address is also printed out.
 */
void
not_running_ipc(const char *chaddr)
{
	if (not_running_hook != NULL) {
		(*not_running_hook)();
	}

	if (chaddr == NULL) {
		error("%s is not running.", daemon_name);
	}
	else {
		error("%s is not running at \"%s\".", daemon_name, chaddr);
	}
}

/*
 * void
 * not_allowed(void)
 *	Print error message that informs the subcommand is not permitted.
 */
void
not_allowed(void)
{
	error("You are not allowed to use this subcommand.");
}

/*
 * int
 * open_pidfile(pfc_pidf_t *pfp)
 *	Open PFC daemon's PID file in read-only mode.
 */
int
open_pidfile(pfc_pidf_t *pfp)
{
	int		err;

	err = pidfile_open_rdonly(pfp, pfcd_pidfile);
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			not_running();
		}
		else {
			error("%s: open failed: %s", pfcd_pidfile,
			      strerror(err));
		}

		return err;
	}

	return 0;
}

/*
 * int
 * get_daemon_pid(pfc_pidf_t pf, pid_t *pidp)
 *	Get PFC daemon's process ID.
 *	`pf' must be a valid PID file handle created by open_pidfile().
 *
 * Calling/Exit State:
 *	Upon successful completion, PID of the PFC daemon is set to `*pidp' and
 *	zero is returned. If the daemon is not running, ENOENT is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
get_daemon_pid(pfc_pidf_t pf, pid_t *pidp)
{
	int	err;

	debug_printf(1, "pidfile = %s", pfcd_pidfile);
	err = pidfile_getowner(pf, pidp);
	if (PFC_EXPECT_TRUE(err == 0)) {
		if (*pidp == 0) {
			not_running();

			return ENOENT;
		}

		return 0;
	}

	error("Failed to determine PID for the daemon: %s", strerror(err));

	return err;
}

/*
 * static pfc_bool_t
 * check_privilege(void)
 *	Ensure that the current user can issue subcommand which requires
 *	system privilege.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned if the current user can issue the subcommand
 *	which requires system privilege. PFC_FALSE is returned if not.
 */
static pfc_bool_t
check_privilege(void)
{
	pfc_pidf_t	pf;
	pid_t		pid;
	int		err;

	/* Ensure that the daemon is running. */
	err = open_pidfile(&pf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	err = get_daemon_pid(pf, &pid);
	pidfile_close(pf);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return PFC_FALSE;
	}

	/* Ensure that the program can access to the daemon control port. */
	err = pfc_ctrl_check_permission();
	if (PFC_EXPECT_FALSE(err != 0)) {
		if (err == ENOENT) {
			not_running();
		}
		else if (err == EACCES) {
			not_allowed();
		}
		else {
			error("Unable to check permission: %s", strerror(err));
		}

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static ctrlcmd_spec_t *
 * subcmd_find(const char *name)
 *	Search for a subcommand definition associated with the specified name.
 */
static ctrlcmd_spec_t *
subcmd_find(const char *name)
{
	ctrlcmd_spec_t	*spec;

	for (spec = subcommand_spec; spec < PFC_ARRAY_LIMIT(subcommand_spec);
	     spec++) {
		if (strcmp(spec->cs_name, name) == 0) {
			return spec;
		}
	}

	return NULL;
}

/*
 * static ctrlcmd_ret_t
 * subcmd_dispatch(subcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		   char **PFC_RESTRICT argv)
 *	Dispatch subcommand.
 *
 * Calling/Exit State:
 *	Upon successful completion, CMDRET_COMPLETE is returned.
 *	Otherwise CMDRET_FAIL is returned.
 */
static ctrlcmd_ret_t
subcmd_dispatch(ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
		char **PFC_RESTRICT argv)
{
	pfc_refptr_t	*fullname;
	ctrlcmd_ret_t	ret;
	cproto_sess_t	session;
	pfc_iowait_t	iowait, *iwp;
	int		err;

	fullname = pfc_refptr_sprintf("%s %s", progname, spec->cs_name);
	if (PFC_EXPECT_FALSE(fullname == NULL)) {
		fatal("Failed to construct subcommand name.");
		/* NOTREACHED */
	}

	spec->cs_fullname = fullname;
	msg_prefix = pfc_refptr_string_value(fullname);

	/* Call constructor. */
	ret = spec->cs_ctor(spec, argc, argv);
	if (ret != CMDRET_CONT) {
		goto out_msg;
	}

	if (spec->cs_flags & CTRLCMDF_NEED_PRIV) {
		/* This subcommand requires privilege. */
		if (!check_privilege()) {
			ret = CMDRET_FAIL;
			goto out;
		}
	}

	if (spec->cs_iowait != NULL) {
		/* Use subcommand specific I/O wait handler. */
		iwp = &iowait;
		spec->cs_iowait(spec, iwp);
	}
	else {
		iwp = NULL;
	}

	/* Create control protocol session. */
	debug_printf(1, "ctrl_timeout = %u", ctrl_timeout);
	err = client_create(&session, iwp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
		goto out;
	}

	if (spec->cs_prepare != NULL) {
		ret = spec->cs_prepare(spec, &session);
		if (PFC_EXPECT_FALSE(ret != CMDRET_CONT)) {
			goto out;
		}
	}

	/* Execute subcommand. */
	ret = client_execute(&session, spec);
	pfc_ctrl_client_destroy(&session);

out:
	if (spec->cs_dtor != NULL) {
		spec->cs_dtor(spec);
	}

out_msg:
	msg_prefix = progname;
	spec->cs_fullname = NULL;
	pfc_refptr_put(fullname);

	return ret;
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
 *	Import system configuration.
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

	pfcd_options = pfc_sysconf_get_block("options");
	pfcd_work_dir = pfc_conf_get_string(pfcd_options, "work_dir",
					    PFC_PFCD_WORKDIR_PATH);
	pfcd_pidfile = pidfile_default_path();
}

/*
 * static void
 * debug_vprintf(int level, const char *fmt, va_list ap)
 *	Print debugging message in vprintf(3) format, if current debugging
 *	level is higher than or equal to the specified level.
 */
static void
debug_vprintf(int level, const char *fmt, va_list ap)
{
	PFC_ASSERT(level > 0);

	if (level <= ctrl_debug) {
		int	i;

		for (i = 0; i < level; i++) {
			putc('*', stderr);
		}
		putc(' ', stderr);
		vfprintf(stderr, fmt, ap);
		putc('\n', stderr);
	}
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
	       progname, PFC_PRODUCT_NAME,
	       PFC_VERSION_STRING, PFC_BUILD_TYPE_SUFFIX, copyright);

	exit(PFC_EX_OK);
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
	const	ctrlcmd_spec_t	*spec;

	pfc_cmdopt_usage(parser, stdout);

	printf("\nSubcommands:\n");
	for (spec = subcommand_spec; spec < PFC_ARRAY_LIMIT(subcommand_spec);
	     spec++) {
		printf("    %s\n", spec->cs_name);
	}

	exit(PFC_EX_OK);
	/* NOTREACHED */
}
