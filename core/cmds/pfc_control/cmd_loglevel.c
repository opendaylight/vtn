/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_loglevel.c - "loglevel" subcommand.
 *
 * "loglevel" subcommand get or set current logging level.
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <cmdopt.h>
#include <pfc/path.h>
#include <pfc/log.h>
#include "pfc_control.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE							\
	"Display or change current logging level.\n"			\
	"If no argument is specified, the current logging level is "	\
	"displayed.\n"							\
	"If a log level is specified as argument, it is set as new "	\
	"logging level."

/*
 * Brief description of arguments.
 */
static const char	arg_format[] = "[LOGLEVEL]";

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{'r', "reset", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Reset to initial logging level.", NULL},
	{'m', "module", PFC_CMDOPT_TYPE_STRING, PFC_CMDOPT_DEF_ONCE,
	 "Set or get logging level only for the specified module. "
	 "If this option is specified with \"-r\", logging level for the "
	 "specified module will be reset.\n"
	 "This option can not be specified with any other option except "
	 "for \"-r\".", "MODULE"},
	{'M', "module-list", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "List per-module logging level configuration.\n"
	 "This option can not be specified with any other option.", NULL},
	{'R', "module-reset", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Clear all per-module configurations.\n"
	 "This option can not be specified with any other option.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Bits which represent options.
 */
#define	LOGLVL_OPT_RESET		PFC_CONST_U(0x1)
#define	LOGLVL_OPT_MODULE		PFC_CONST_U(0x2)
#define	LOGLVL_OPT_MODLIST		PFC_CONST_U(0x4)
#define	LOGLVL_OPT_MODRESET		PFC_CONST_U(0x8)

#define	LOGLVL_OPT_CANRESET		LOGLVL_OPT_MODULE
#define	LOGLVL_OPT_NOARG						\
	(LOGLVL_OPT_RESET | LOGLVL_OPT_MODLIST | LOGLVL_OPT_MODRESET)

#define	LOGLVL_ERR_BUFSIZE		PFC_CONST_U(128)

#define	LOGLVL_MODNAME_WIDTH		20
#define	LOGLVL_MODNAME_MAX		PFC_CONST_U(31)

/*
 * Exclusive options.
 */
typedef struct {
	char		le_optchar;	/* option character */
	uint32_t	le_mask;	/* option bitmask (LOVLVL_OPT_XX) */
} loglvl_exopts_t;

static const loglvl_exopts_t	exclusive_opts[] = {
	{'r', LOGLVL_OPT_RESET},
	{'m', LOGLVL_OPT_MODULE},
	{'M', LOGLVL_OPT_MODLIST},
	{'R', LOGLVL_OPT_MODRESET},
};

/*
 * Target module name.
 */
static char	*log_modname;

/*
 * New log level.
 */
static int	log_newlevel;

/*
 * Supported logging levels.
 * Array index must be identical to pfc_log_level_t, and it is a value to be
 * sent to the daemon.
 */
typedef struct {
	const char	*lv_name;	/* string representation of the level */
	uint32_t	lv_flags;	/* flags */
} log_level_t;

typedef const log_level_t	log_clevel_t;

/*
 * Flags for lv_flags.
 */
#define	LLVF_NOSYSLOG		0x1U	/* syslog does not support */
#define	LLVF_HIDDEN		0x2U	/* hidden log level */

#define	LOG_LEVEL_DECL_FLAGS(name, flags)	\
	{					\
		.lv_name	= (name),	\
		.lv_flags	= (flags),	\
	}

#define	LOG_LEVEL_DECL(name)		LOG_LEVEL_DECL_FLAGS(name, 0)

static log_clevel_t	loglevels[] = {
	LOG_LEVEL_DECL("fatal"),
	LOG_LEVEL_DECL("error"),
	LOG_LEVEL_DECL("warn"),
	LOG_LEVEL_DECL("notice"),
	LOG_LEVEL_DECL("info"),
	LOG_LEVEL_DECL("debug"),
	LOG_LEVEL_DECL_FLAGS("trace", LLVF_NOSYSLOG),
	LOG_LEVEL_DECL_FLAGS("verbose", LLVF_NOSYSLOG | LLVF_HIDDEN),
};

/*
 * Internal prototypes.
 */
static void	exclusive_option_error(uint32_t opts);
static int	convert_log_level(const char *name);
static void	dump_log_levels(FILE *fp);
static int	list_modlevels(cproto_sess_t *sess);
static int	dump_modlevel(cproto_sess_t *sess);

/*
 * ctrlcmd_ret_t
 * cmd_ctor_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		     char **PFC_RESTRICT argv)
 *	Constructor of "loglevel" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
		  char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_CONT;
	int		lvl, argidx;
	char		c;
	uint32_t	opts = 0, exopts;
	const char	*modname = NULL;

	lvl = CTRL_LOGLVL_NONE;
	log_modname = NULL;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(CTRLCMD_FULLNAME(spec), argc, argv,
				 option_spec, arg_format, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return CMDRET_FAIL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'r':
			lvl = CTRL_LOGLVL_RESET;
			opts |= LOGLVL_OPT_RESET;
			break;

		case 'm':
			modname = pfc_cmdopt_arg_string(parser);
			if (PFC_EXPECT_FALSE(*modname == '\0')) {
				error("Module name must not be empty.");
				ret = CMDRET_FAIL;
				goto out;
			}
			opts |= LOGLVL_OPT_MODULE;
			break;

		case 'M':
			lvl = CTRL_LOGLVL_MODLIST;
			opts |= LOGLVL_OPT_MODLIST;
			break;

		case 'R':
			lvl = CTRL_LOGLVL_MODRESET;
			opts |= LOGLVL_OPT_MODRESET;
			break;

		case PFC_CMDOPT_USAGE:
			pfc_cmdopt_usage(parser, stdout);
			ret = CMDRET_COMPLETE;
			goto out;

		case PFC_CMDOPT_HELP:
			pfc_cmdopt_help(parser, stdout, HELP_MESSAGE);
			dump_log_levels(stdout);
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

	/* Check exclusive option. */
	exopts = opts;
	if (exopts & LOGLVL_OPT_CANRESET) {
		exopts &= ~LOGLVL_OPT_RESET;
	}
	if (PFC_EXPECT_FALSE(!PFC_IS_POW2(exopts))) {
		exclusive_option_error(exopts);
		ret = CMDRET_FAIL;
		goto out;
	}

#ifdef	PFC_VERBOSE_DEBUG
	/* Extra assertions. */
	if (opts & LOGLVL_OPT_MODULE) {
		PFC_ASSERT(modname != NULL &&
			   (lvl == CTRL_LOGLVL_NONE ||
			    lvl == CTRL_LOGLVL_RESET));
	}
	else if (opts & LOGLVL_OPT_MODLIST) {
		PFC_ASSERT(modname == NULL && lvl == CTRL_LOGLVL_MODLIST);
	}
	else if (opts & LOGLVL_OPT_MODRESET) {
		PFC_ASSERT(modname == NULL && lvl == CTRL_LOGLVL_MODRESET);
	}
	else if (opts == LOGLVL_OPT_RESET) {
		PFC_ASSERT(modname == NULL && lvl == CTRL_LOGLVL_RESET);
	}
	else {
		PFC_ASSERT(opts == 0);
		PFC_ASSERT(modname == NULL && lvl == CTRL_LOGLVL_NONE);
	}
#endif	/* PFC_VERBOSE_DEBUG */

	if (modname != NULL) {
		size_t	namelen = strlen(modname);

		if (PFC_EXPECT_FALSE(namelen > LOGLVL_MODNAME_MAX)) {
			error("Too long module name: %s", modname);
			ret = CMDRET_FAIL;
			goto out;
		}

		log_modname = (char *)malloc(namelen + 1);
		if (PFC_EXPECT_FALSE(log_modname == NULL)) {
			error("Failed to copy module name.");
			ret = CMDRET_FAIL;
			goto out;
		}

		strcpy(log_modname, modname);
	}

	if ((argidx = pfc_cmdopt_validate(parser)) == -1) {
		error("Invalid command line options.");
		ret = CMDRET_FAIL;
		goto out;
	}

	argc -= argidx;
	argv += argidx;
	if (argc > 1) {
		pfc_cmdopt_usage(parser, stderr);
		ret = CMDRET_FAIL;
		goto out;
	}

	if (argc == 1) {
		if (PFC_EXPECT_FALSE(opts & LOGLVL_OPT_NOARG)) {
			error("The specified option takes no argument.");
			ret = CMDRET_FAIL;
			goto out;
		}

		/* Convert level name to logging level value. */
		lvl = convert_log_level(*argv);
		if (PFC_EXPECT_FALSE(lvl < 0)) {
			error("Invalid logging level: %s", *argv);
			ret = CMDRET_FAIL;
			goto out;
		}

		debug_printf(1, "New logging level: %s [%d]", *argv, lvl);
	}

	log_newlevel = lvl;

out:
	pfc_cmdopt_destroy(parser);

	if (PFC_EXPECT_FALSE(ret != CMDRET_CONT)) {
		free((void *)log_modname);
		log_modname = NULL;
	}

	return ret;
}

/*
 * int
 * cmd_send_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		     cproto_sess_t *PFC_RESTRICT sess)
 *	Send optional argument for command.
 *
 *	If new logging level is specified, it is sent to the server.
 *	If not, NULL is sent.
 */
int
cmd_send_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		  cproto_sess_t *PFC_RESTRICT sess)
{
	int	err;

	/*
	 * Send target module name.
	 * An empty string will be sent if global logging level is chosen.
	 */
	if (log_modname == NULL) {
		err = cproto_data_write_null(sess);
	}
	else {
		err = cproto_data_write_text(sess, log_modname);
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to send target module name: %s", strerror(err));

		return err;
	}

	if (log_newlevel == CTRL_LOGLVL_NONE) {
		/* Derive current logging level from pfcd. */
		err = cproto_data_write_null(sess);
	}
	else {
		/* Set new logging level to pfcd. */
		err = cproto_data_write_int32(sess, log_newlevel);
	}

	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to send command argument: %s", strerror(err));
	}

	return err;
}

/*
 * int
 * cmd_receive_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *			cproto_sess_t *PFC_RESTRICT sess)
 *	Receive successful response of LOGLEVEL command.
 */
int
cmd_receive_loglevel(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		     cproto_sess_t *PFC_RESTRICT sess)
{
	cproto_data_t	data;
	uint32_t	level;
	int		err;

	if (log_newlevel == CTRL_LOGLVL_MODLIST) {
		/* List logging level per module. */

		return list_modlevels(sess);
	}

	err = cproto_data_read(sess, &data);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to read LOGLEVEL response: %s", strerror(err));

		return err;
	}
	if (PFC_EXPECT_FALSE(data.cpd_type != CTRL_PDTYPE_INT32)) {
		error("Unexpected response data type from LOGLEVEL: %u",
		      data.cpd_type);
		err = EPROTO;
		goto out;
	}

	level = cproto_data_uint32(&data);
	if (PFC_EXPECT_FALSE(level >= PFC_ARRAY_CAPACITY(loglevels))) {
		error("Unknown log level is returned: %u", level);
		err = EINVAL;
		goto out;
	}

	/* Show current logging level. */
	if (log_newlevel == CTRL_LOGLVL_MODRESET) {
		printf("Per-module logging level has been successfully "
		       "reset.\n");
	}
	else if (log_modname != NULL) {
		printf("Logging Level (%s):  %s\n", log_modname,
		       loglevels[level].lv_name);
	}
	else {
		printf("Logging Level:  %s\n", loglevels[level].lv_name);
	}

out:
	cproto_data_free(&data);

	return err;
}

/*
 * void
 * cmd_dtor_loglevel(const ctrlcmd_spec_t *spec)
 *	Destructor of "loglevel" subcommand.
 */
void
cmd_dtor_loglevel(const ctrlcmd_spec_t *spec)
{
	free((void *)log_modname);
}

/*
 * static void
 * exclusive_option_error(uint32_t opts)
 *	Show error message which reports two or more than exclusive options
 *	are specified.
 */
static void
exclusive_option_error(uint32_t opts)
{
	const loglvl_exopts_t	*exopts;
	char		buf[LOGLVL_ERR_BUFSIZE], *p;
	size_t		bufsize;
	const char	*sep = str_empty;
	int		slen;

	slen = snprintf(buf, sizeof(buf),
			"Specified options are exclusive: ");
	PFC_ASSERT(slen > 0 && (size_t)slen < sizeof(buf));

	p = buf + slen;
	bufsize = sizeof(buf) - slen;

	for (exopts = exclusive_opts; exopts < PFC_ARRAY_LIMIT(exclusive_opts);
	     exopts++) {
		if (opts & exopts->le_mask) {
			slen = snprintf(p, bufsize, "%s-%c",
					sep, exopts->le_optchar);
			PFC_ASSERT(slen > 0 && (size_t)slen < bufsize);
			p += slen;
			bufsize -= slen;
			sep = str_comma;
		}
	}

	error(buf);
}

/*
 * static int
 * convert_log_level(const char *name)
 *	Convert logging level name to the value of logging level.
 *	-1 is returned if failed.
 */
static int
convert_log_level(const char *name)
{
	log_clevel_t	*lvl;

	for (lvl = loglevels; lvl < PFC_ARRAY_LIMIT(loglevels); lvl++) {
		if (strcasecmp(lvl->lv_name, name) == 0) {
			int	level = (int)(lvl - loglevels);

			return level;
		}
	}

	return -1;
}

/*
 * static void
 * dump_log_levels(FILE *fp)
 *	Dump all supported logging levels to the specified file pointer.
 */
static void
dump_log_levels(FILE *fp)
{
	log_clevel_t	*lvl;

	fprintf(fp, "\nLogging Levels:\n");

	for (lvl = loglevels; lvl < PFC_ARRAY_LIMIT(loglevels); lvl++) {
		if ((lvl->lv_flags & LLVF_HIDDEN) == 0) {
			fprintf(fp, "    %s\n", lvl->lv_name);
		}
	}
}

static int
list_modlevels(cproto_sess_t *sess)
{
	cproto_data_t	data;
	uint32_t	count;
	int		err;

	/*
	 * The first data must be the number of pfc_log_modconf_t array
	 * elements.
	 */
	err = cproto_data_read(sess, &data);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to read the number of array elements: %s",
		      strerror(err));

		return err;
	}
	if (PFC_EXPECT_FALSE(data.cpd_type != CTRL_PDTYPE_INT32)) {
		error("Unexpected response data type for the number of array "
		      "elements: %u", data.cpd_type);
		err = EPROTO;
		goto out;
	}

	count = cproto_data_uint32(&data);
	if (count == 0) {
		printf("Per-module logging level is not configured.\n");
		err = 0;
		goto out;
	}

	do {
		err = dump_modlevel(sess);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
		count--;
	} while (count > 0);

out:
	cproto_data_free(&data);

	return err;
}

static int
dump_modlevel(cproto_sess_t *sess)
{
	cproto_data_t	dname, dlevel;
	uint32_t	level;
	int		err;

	/* Fetch module name. */
	err = cproto_data_read(sess, &dname);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to read module name: %s", strerror(err));

		return err;
	}
	if (PFC_EXPECT_FALSE(dname.cpd_type != CTRL_PDTYPE_TEXT)) {
		error("Unexpected data type for module name: %u",
		      dname.cpd_type);
		err = EPROTO;
		goto out_dname;
	}

	/* Fetch logging level for this module. */
	err = cproto_data_read(sess, &dlevel);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to read logging level %s", strerror(err));
		goto out_dname;
	}
	if (PFC_EXPECT_FALSE(dlevel.cpd_type != CTRL_PDTYPE_INT32)) {
		error("Unexpected data type for logging level: %u",
		      dlevel.cpd_type);
		err = EPROTO;
		goto out_dlevel;
	}

	level = cproto_data_uint32(&dlevel);
	if (PFC_EXPECT_TRUE(level < PFC_ARRAY_CAPACITY(loglevels))) {
		printf("%*s %s\n",
		       -LOGLVL_MODNAME_WIDTH, cproto_data_text(&dname),
		       loglevels[level].lv_name);
	}
	else {
		/* This should not happen. */
		printf("%*s <%u>\n",
		       -LOGLVL_MODNAME_WIDTH, cproto_data_text(&dname), level);
	}

out_dlevel:
	cproto_data_free(&dlevel);

out_dname:
	cproto_data_free(&dname);

	return err;
}
