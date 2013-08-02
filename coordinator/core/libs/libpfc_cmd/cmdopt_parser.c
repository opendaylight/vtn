/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmdopt_parser.c - Command line option parser.
 *
 * Remarks:	Command line option parser is not thread safe.
 */

#include <string.h>
#include <stdarg.h>
#include <pfc/strtoint.h>
#include <pfc/debug.h>
#include "cmdopt_impl.h"

/*
 * Internal definition of help and usage option.
 */
static const pfc_cmdopt_def_t	defopt_help = {
	PFC_CMDOPT_HELP,
	PFC_CMDOPT_LOPT_HELP,
	PFC_CMDOPT_TYPE_NONE,
	0,
	"Display this help message.",
	NULL
};

static const pfc_cmdopt_def_t	defopt_usage = {
	PFC_CMDOPT_USAGE,
	PFC_CMDOPT_LOPT_USAGE,
	PFC_CMDOPT_TYPE_NONE,
	PFC_CMDOPT_DEF_LONGONLY,
	"Display brief usage.",
	NULL
};

/*
 * Static messages.
 */
static const char	msg_sqleft[] = "[";
static const char	msg_sqright[] = "]";
static const char	msg_empty[] = "";
static const char	msg_arg[] = "ARG";
static const char	msg_unknown[] = "<unknown>";

/*
 * Internal prototypes.
 */
static const pfc_cmdopt_def_t	*cmdopt_parse_short(pfc_cmdopt_t *PFC_RESTRICT
						    ctx,
						    char *PFC_RESTRICT arg);
static const pfc_cmdopt_def_t	*cmdopt_parse_long(pfc_cmdopt_t *PFC_RESTRICT
						   ctx,
						   char *PFC_RESTRICT arg);
static const pfc_cmdopt_def_t	*cmdopt_short_lookup(pfc_cmdopt_t *ctx,
						     char optchar);
static const pfc_cmdopt_def_t	*cmdopt_long_lookup(pfc_cmdopt_t *PFC_RESTRICT
						    ctx,
						    char *PFC_RESTRICT optname,
						    char **PFC_RESTRICT argp);

static int	cmdopt_check_duplicated(pfc_cmdopt_t *PFC_RESTRICT ctx,
					const pfc_cmdopt_def_t
					*PFC_RESTRICT opt);
static void	cmdopt_errmsg(pfc_cmdopt_t *PFC_RESTRICT ctx,
			      const char *PFC_RESTRICT fmt, ...)
	PFC_FATTR_PRINTFLIKE(2, 3);
static pfc_bool_t	cmdopt_parse_arg(pfc_cmdopt_t *PFC_RESTRICT ctx,
					 char *PFC_RESTRICT arg);

/*
 * pfc_cmdopt_t *
 * pfc_cmdopt_init(const char *PFC_RESTRICT name, int argc,
 *		   char **PFC_RESTRICT argv,
 *		   const pfc_cmdopt_def_t *PFC_RESTRICT options,
 *		   const char *PFC_RESTRICT argdesc, uint32_t flags)
 *	Initialize command option parser context.
 *
 *	`name' should be a string that represents application name.
 *	 This is used to generate help and error message.
 *
 *	`argc' is number of arguments in `argv', and `argv' is an argument
 *	vector which contains command arguments to be parsed.
 *
 *	`options' is a pfc_cmdopt_def_t array which defines option format.
 *	The last element must have PFC_CMDOPT_EOF in cd_ident.
 *	See comments in pfc/cmdopt.h for more details.
 *
 *	`flags' is a bitmask that specifies parser option.
 *	The following bits can be set:
 *
 *	PFC_CMDOPT_NOHELP
 *		Help message request is ignored.
 *
 *	PFC_CMDOPT_NOUSAGE
 *		Usage request is ignored.
 *
 *	PFC_CMDOPT_NOERRMSG
 *		Don't print error message. If this bit is not set, error
 *		message is written to the standard error output on parse error.
 *
 *	`argdesc' is a message which describes format of command argument.
 *	This is used to print help and usage message. If NULL is specified,
 *	this program is considered that it takes no argument.
 *
 * Calling/Exit State:
 *	Upon successful completion, pfc_cmdopt_init() returns a pointer to
 *	parser context. NULL is returned on error.
 *
 * Remarks:
 *	Storage specified to the following arguments must be kept until
 *	cmdopt_destroy() is called.
 *	  - name
 *	  - argv
 *	  - options
 *	  - argdesc
 */
pfc_cmdopt_t *
pfc_cmdopt_init(const char *PFC_RESTRICT name, int argc,
		char **PFC_RESTRICT argv,
		const pfc_cmdopt_def_t *PFC_RESTRICT options,
		const char *PFC_RESTRICT argdesc, uint32_t flags)
{
	pfc_cmdopt_t	*ctx;
	size_t		nopts, visible;
	char		*parsed;
	const pfc_cmdopt_def_t	*opt;

	PFC_ASSERT(name != NULL);
	PFC_ASSERT(argc > 0);
	PFC_ASSERT(argv != NULL);
	PFC_ASSERT(options != NULL);

	/* Determine number of options. */
	visible = 0;
	for (opt = options; opt->cd_ident != PFC_CMDOPT_EOF; opt++) {
		if ((opt->cd_flags & PFC_CMDOPT_DEF_HIDDEN) == 0) {
			visible++;
		}
	}

	nopts = opt - options;

	ctx = (pfc_cmdopt_t *)malloc(sizeof(*ctx));
	if (PFC_EXPECT_FALSE(ctx == NULL)) {
		return NULL;
	}

	parsed = (char *)calloc(nopts, sizeof(char));
	if (PFC_EXPECT_FALSE(parsed == NULL)) {
		free(ctx);

		return NULL;
	}
	ctx->cc_parsed = parsed;
	ctx->cc_nopts = nopts;
	ctx->cc_nvisibles = visible;
	ctx->cc_progname = name;
	ctx->cc_argdesc = argdesc;
	ctx->cc_argc = argc;
	ctx->cc_argv = argv;
	ctx->cc_options = options;
	ctx->cc_lastopt = NULL;
	ctx->cc_bundle = NULL;
	ctx->cc_flags = flags;
	ctx->cc_errcode = PFC_CMDOPT_E_NONE;
	ctx->cc_helpopt = (flags & PFC_CMDOPT_NOHELP) ? NULL : &defopt_help;
	ctx->cc_usageopt = (flags & PFC_CMDOPT_NOUSAGE) ? NULL : &defopt_usage;
	ctx->cc_fallback = NULL;

	/*
	 * Skip first argument which is expected to be a command name.
	 */
	ctx->cc_cursor = 1;

	return ctx;
}

/*
 * void
 * pfc_cmdopt_destroy(pfc_cmdopt_t *ctx)
 *	Destroy command option parser context.
 */
void
pfc_cmdopt_destroy(pfc_cmdopt_t *ctx)
{
	free(ctx->cc_parsed);
	free(ctx);
}

/*
 * void
 * pfc_cmdopt_setfallback(pfc_cmdopt_t *PFC_RESTRICT ctx,
 *			  pfc_cmdopt_fb_t fallback)
 *	Set fallback function, which will be called when the parse finds
 *	unknown option. If NULL is passed to `fallback', the fallback function
 *	is disabled.
 */
void
pfc_cmdopt_setfallback(pfc_cmdopt_t *PFC_RESTRICT ctx,
		       pfc_cmdopt_fb_t fallback)
{
	ctx->cc_fallback = fallback;
}

/*
 * char
 * pfc_cmdopt_next(pfc_cmdopt_t *ctx)
 *	Parse next command line option.
 *
 * Calling/Exit State:
 *	If next argument is valid option, corresponding option identifier
 *	defined by pfc_cmdopt_def_t array is returned. If it takes an argument,
 *	it can be derived by calling pfc_cmdopt_arg_int32(), or
 *	pfc_cmdopt_arg_uint32(), or pfc_cmdopt_arg_string().
 *
 *	If PFC_CMDOPT_NOHELP is not specified to pfc_cmdopt_init(),
 *	PFC_CMDOPT_HELP will be returned if help option is parsed.
 *	And if PFC_CMDOPT_NOUSAGE is not specified to pfc_cmdopt_init(),
 *	PFC_CMDOPT_USAGE will be returned if usage option is parsed.
 *
 *	PFC_CMDOPT_EOF is returned if all options are parsed.
 *
 *	PFC_CMDOPT_ERROR is returned if an error occurs.
 *
 * Remarks:
 *	If PFC_CMDOPT_ERROR is returned, the specified context must be
 *	immediately dropped by calling pfc_cmdopt_destroy().
 */
char
pfc_cmdopt_next(pfc_cmdopt_t *ctx)
{
	int	argc = ctx->cc_argc;
	char	**argv = ctx->cc_argv;
	int	cursor = ctx->cc_cursor;
	char	*arg, c;
	const pfc_cmdopt_def_t	*opt;

	if (PFC_EXPECT_FALSE(ctx->cc_errcode != PFC_CMDOPT_E_NONE)) {
		/* Error has already occurred on this context. */
		return PFC_CMDOPT_ERROR;
	}

	if (cursor >= argc) {
		return PFC_CMDOPT_EOF;
	}

	if (ctx->cc_bundle != NULL) {
		/* Unparsed bundled options remain. */
		if ((opt = cmdopt_parse_short(ctx, ctx->cc_bundle)) == NULL) {
			return PFC_CMDOPT_ERROR;
		}

		return opt->cd_ident;
	}

	/* Check whether current argument is an option not not. */
	arg = *(argv + cursor);
	if (*arg != '-' || (c = *(arg + 1)) == '\0') {
		return PFC_CMDOPT_EOF;
	}

	if (c == '-') {
		if (*(arg + 2) == '\0') {
			/* "--" is treated as option terminator. */
			ctx->cc_cursor = cursor + 1;

			return PFC_CMDOPT_EOF;
		}

		/* This should be a long name option. */
		opt = cmdopt_parse_long(ctx, arg + 2);
	}
	else {
		/* This should be a short option. */
		opt = cmdopt_parse_short(ctx, arg + 1);
	}

	if (opt == NULL) {
		return PFC_CMDOPT_ERROR;
	}

	return opt->cd_ident;
}

/*
 * int
 * pfc_cmdopt_errcode(pfc_cmdopt_t *ctx)
 *	Return error code detected by previous call of pfc_cmdopt_next().
 *
 * Calling/Exit State:
 *	PFC_CMDOPT_E_NONE is returned if no error is detected.
 *	Otherwise, one of error code defined in pfc/cmdopt.h is returned.
 */
int
pfc_cmdopt_errcode(pfc_cmdopt_t *ctx)
{
	return ctx->cc_errcode;
}

/*
 * int32_t
 * pfc_cmdopt_arg_int32(pfc_cmdopt_t *ctx)
 *	Return signed 32-bit integer option argument parsed by previous call of
 *	pfc_cmdopt_next().
 *
 * Calling/Exit State:
 *	Parsed signed 32-bit integer value is returned.
 *	If an option parsed by previous call of pfc_cmdopt_next() doesn't have
 *	PFC_CMDOPT_TYPE_INT32 type, zero is returned.
 */
int32_t
pfc_cmdopt_arg_int32(pfc_cmdopt_t *ctx)
{
	const pfc_cmdopt_def_t	*opt = ctx->cc_lastopt;

	if (opt == NULL || opt->cd_type != PFC_CMDOPT_TYPE_INT32) {
		return 0;
	}

	return ctx->cc_optarg.i32;
}

/*
 * uint32_t
 * pfc_cmdopt_arg_uint32(pfc_cmdopt_t *ctx)
 *	Return unsigned 32-bit integer option argument parsed by previous
 *	call of pfc_cmdopt_next().
 *
 * Calling/Exit State:
 *	Parsed unsigned 32-bit integer value is returned.
 *	If an option parsed by previous call of pfc_cmdopt_next() doesn't have
 *	PFC_CMDOPT_TYPE_UINT32 type, zero is returned.
 */
uint32_t
pfc_cmdopt_arg_uint32(pfc_cmdopt_t *ctx)
{
	const pfc_cmdopt_def_t	*opt = ctx->cc_lastopt;

	if (opt == NULL || opt->cd_type != PFC_CMDOPT_TYPE_UINT32) {
		return 0;
	}

	return ctx->cc_optarg.u32;
}

/*
 * const char *
 * pfc_cmdopt_arg_string(pfc_cmdopt_t *ctx)
 *	Return string option argument parsed by previous call of
 *	pfc_cmdopt_next().
 *
 * Calling/Exit State:
 *	A pointer to parsed string is returned.
 *	If an option parsed by previous call of pfc_cmdopt_next() doesn't have
 *	PFC_CMDOPT_TYPE_STRING, NULL is returned.
 *
 * Remarks:
 *	Returned string is a part of argument vector specified to
 *	pfc_cmdopt_init().
 */
const char *
pfc_cmdopt_arg_string(pfc_cmdopt_t *ctx)
{
	const pfc_cmdopt_def_t	*opt = ctx->cc_lastopt;

	if (opt == NULL || opt->cd_type != PFC_CMDOPT_TYPE_STRING) {
		return NULL;
	}

	return (const char *)ctx->cc_optarg.string;
}

/*
 * int
 * pfc_cmdopt_validate(pfc_cmdopt_t *ctx)
 *	Validate parser context.
 *	The main purpose of this function is to check whether all mandatory
 *	options have been specified or not.
 *
 * Calling/Exit State:
 *	Upon successful completion, pfc_cmdopt_validate() returns argument
 *	vector index for the first command argument. -1 is returned on error.
 */
int
pfc_cmdopt_validate(pfc_cmdopt_t *ctx)
{
	const pfc_cmdopt_def_t	*opt;
	char	*parsed = ctx->cc_parsed;

	for (opt = ctx->cc_options; opt->cd_ident != PFC_CMDOPT_EOF;
	     opt++, parsed++) {
		if ((opt->cd_flags & PFC_CMDOPT_DEF_MANDATORY) &&
		    *parsed == PFC_CMDOPT_EOF) {
			ctx->cc_errcode = PFC_CMDOPT_E_MANDATORY;
			pfc_cmdopt_opt_desc(ctx, opt, PFC_CMDOPT_DESC_NOARG);
			cmdopt_errmsg(ctx, "%s: Missing mandatory option.",
				      ctx->cc_buffer);

			return -1;
		}
	}

	return ctx->cc_cursor;
}

/*
 * static const pfc_cmdopt_def_t *
 * cmdopt_parse_short(pfc_cmdopt_t *PFC_RESTRICT ctx, char *PFC_RESTRICT arg)
 *	Parse short option.
 *
 * Calling/Exit State:
 *	A pointer to cmdopt_def_t is returned if the specified argument is
 *	a valid option. Otherwise NULL is returned.
 *
 * Remarks:
 *	The caller must eliminate leading '-' character in option string.
 */
static const pfc_cmdopt_def_t *
cmdopt_parse_short(pfc_cmdopt_t *PFC_RESTRICT ctx, char *PFC_RESTRICT arg)
{
	char		optchar = *arg;
	const pfc_cmdopt_def_t	*opt;

	if ((opt = cmdopt_short_lookup(ctx, optchar)) == NULL) {
		cmdopt_errmsg(ctx, "Unknown option: -%c", optchar);
		ctx->cc_errcode = PFC_CMDOPT_E_BADOPT;

		return NULL;
	}

	arg++;
	ctx->cc_lastopt = opt;

	if (!cmdopt_check_duplicated(ctx, opt)) {
		return NULL;
	}

	if (opt->cd_type == PFC_CMDOPT_TYPE_NONE) {
		/* This option takes no argument. */
		if (*arg != '\0') {
			/*
			 * Option bundling is supported.
			 * This character should be a bundled short option.
			 */
			ctx->cc_bundle = arg;
		}
		else {
			ctx->cc_bundle = NULL;
			ctx->cc_cursor++;
		}
	}
	else {
		char	*oarg;
		int	nextcur = ctx->cc_cursor + 1;

		/*
		 * This option takes one argument.
		 * Assuming that option character is 'a', option argument can
		 * be specified as follows:
		 *
		 *	-aARG
		 *	-a ARG
		 */
		if (*arg == '\0') {
			/* Argument is specified by next argument. */
			if (nextcur >= ctx->cc_argc) {
				cmdopt_errmsg(ctx, "-%c requires an argument.",
					      optchar);
				ctx->cc_errcode = PFC_CMDOPT_E_NOARG;

				return NULL;
			}
			oarg = *(ctx->cc_argv + nextcur);
			nextcur++;
		}
		else {
			oarg = arg;
		}

		if (!cmdopt_parse_arg(ctx, oarg)) {
			return NULL;
		}

		ctx->cc_bundle = NULL;
		ctx->cc_cursor = nextcur;
	}

	return opt;
}

/*
 * static const pfc_cmdopt_def_t *
 * cmdopt_parse_long(pfc_cmdopt_t *PFC_RESTRICT ctx, char *PFC_RESTRICT arg)
 *	Parse long name option.
 *
 * Calling/Exit State:
 *	A pointer to cmdopt_def_t is returned if the specified argument is
 *	a valid option. Otherwise NULL is returned.
 *
 * Remarks:
 *	The caller must eliminate all leading '-' characters in option string.
 */
static const pfc_cmdopt_def_t *
cmdopt_parse_long(pfc_cmdopt_t *PFC_RESTRICT ctx, char *PFC_RESTRICT arg)
{
	const pfc_cmdopt_def_t	*opt;
	char		*oarg;

	/* Long options are never bundled. */
	ctx->cc_bundle = NULL;

	if ((opt = cmdopt_long_lookup(ctx, arg, &oarg)) == NULL) {
		cmdopt_errmsg(ctx, "Unknown option: --%s", arg);
		ctx->cc_errcode = PFC_CMDOPT_E_BADOPT;

		return NULL;
	}

	ctx->cc_lastopt = opt;
	if (!cmdopt_check_duplicated(ctx, opt)) {
		return NULL;
	}

	if (opt->cd_type == PFC_CMDOPT_TYPE_NONE) {
		/* This option takes no argument. */
		if (oarg != NULL) {
			cmdopt_errmsg(ctx, "--%s takes no argument.",
				      opt->cd_name);
			ctx->cc_errcode = PFC_CMDOPT_E_HASARG;

			return NULL;
		}

		ctx->cc_cursor++;
	}
	else {
		int	nextcur = ctx->cc_cursor + 1;

		/*
		 * This option takes one argument.
		 * Assuming that option name is "name", option argument can
		 * be specified as follows:
		 *
		 *	--name=ARG
		 *	--name ARG
		 */
		if (oarg == NULL) {
			/* Argument is specified by next argument. */
			if (nextcur >= ctx->cc_argc) {
				cmdopt_errmsg(ctx,
					      "--%s requires an argument.",
					      opt->cd_name);
				ctx->cc_errcode = PFC_CMDOPT_E_NOARG;

				return NULL;
			}
			oarg = *(ctx->cc_argv + nextcur);
			nextcur++;
		}

		if (!cmdopt_parse_arg(ctx, oarg)) {
			return NULL;
		}

		ctx->cc_cursor = nextcur;
	}

	return opt;
}

/*
 * static int
 * cmdopt_check_duplicated(pfc_cmdopt_t *PFC_RESTRICT ctx,
 *			   const pfc_cmdopt_def_t *PFC_RESTRICT opt)
 *	Check whether the option is specified multiple times or not.
 *
 * Calling/Exit State:
 *	Zero is returned if the specified option has already been detected,
 *	and multiple option is not allowed. Otherwise 1 is returned.
 */
static int
cmdopt_check_duplicated(pfc_cmdopt_t *PFC_RESTRICT ctx,
			const pfc_cmdopt_def_t *PFC_RESTRICT opt)
{
	if (opt != ctx->cc_helpopt && opt != ctx->cc_usageopt &&
	    (opt->cd_flags & PFC_CMDOPT_DEF_ONCE)) {
		size_t	idx;
		char	*parsed;

		idx = (size_t)(opt - ctx->cc_options);
		parsed = ctx->cc_parsed + idx;
		if (*parsed != PFC_CMDOPT_EOF) {
			ctx->cc_errcode = PFC_CMDOPT_E_DUPLICATE;
			pfc_cmdopt_opt_desc(ctx, opt, PFC_CMDOPT_DESC_NOARG);
			cmdopt_errmsg(ctx, "%s: Must be specified only once.",
				      ctx->cc_buffer);

			return 0;
		}
		*parsed = opt->cd_ident;
	}

	return 1;
}

/*
 * static const pfc_cmdopt_def_t *
 * cmdopt_short_lookup(pfc_cmdopt_t *ctx, char optchar)
 *	Look up short option in pfc_cmdopt_def_t array specified to
 *	pfc_cmdopt_init().
 *
 * Calling/Exit State:
 *	A pointer to pfc_cmdopt_def_t element is returned if found.
 *	NULL is returned if not found.
 */
static const pfc_cmdopt_def_t *
cmdopt_short_lookup(pfc_cmdopt_t *ctx, char optchar)
{
	const pfc_cmdopt_def_t	*opt, *eopt;

	for (opt = ctx->cc_options, eopt = ctx->cc_options + ctx->cc_nopts;
	     opt < eopt; opt++) {
		if ((opt->cd_flags & PFC_CMDOPT_DEF_LONGONLY) == 0 &&
		    opt->cd_ident == optchar) {
			return opt;
		}
	}

	/* Check whether help option is specified or not. */
	if ((opt = ctx->cc_helpopt) != NULL && opt->cd_ident == optchar) {
		return opt;
	}

	if (ctx->cc_fallback != NULL) {
		/* Call fallback function. */
		opt = ctx->cc_fallback(&optchar, 1, PFC_FALSE);
		if (opt != NULL) {
			return opt;
		}
	}

	return NULL;
}

/*
 * static const pfc_cmdopt_def_t *
 * cmdopt_long_lookup(pfc_cmdopt_t *PFC_RESTRICT ctx,
 *		      char *PFC_RESTRICT optname, char **PFC_RESTRICT argp)
 *	Look up long name option in pfc_cmdopt_def_t array specified to
 *	pfc_cmdopt_init().
 *
 * Calling/Exit State:
 *	A pointer to pfc_cmdopt_def_t element is returned if found.
 *	If the given option contains an argument like "name=argument",
 *	a pointer to argument is set to `*argp'. NULL is set to `*argp'
 *	if no argument is contained.
 *
 *	NULL is returned if not found.
 */
static const pfc_cmdopt_def_t *
cmdopt_long_lookup(pfc_cmdopt_t *PFC_RESTRICT ctx, char *PFC_RESTRICT optname,
		   char **PFC_RESTRICT argp)
{
	const pfc_cmdopt_def_t	*opt, *eopt;
	char	*eq;

	/*
	 * `optname' may contains '=' character to specify option argument.
	 */
	if ((eq = strchr(optname, '=')) == NULL) {
		for (opt = ctx->cc_options,
			     eopt = ctx->cc_options + ctx->cc_nopts;
		     opt < eopt; opt++) {
			const char	*name = opt->cd_name;

			if (name != NULL && strcmp(name, optname) == 0) {
				*argp = NULL;

				return opt;
			}
		}
	}
	else {
		size_t	namelen = eq - optname;

		/*
		 * `optname' contains option argument. So we need to compare
		 * long option name with eliminating option argument.
		 */
		for (opt = ctx->cc_options,
			     eopt = ctx->cc_options + ctx->cc_nopts;
		     opt < eopt; opt++) {
			const char	*name = opt->cd_name;

			if (name != NULL &&
			    strncmp(name, optname, namelen) == 0 &&
			    *(name + namelen) == '\0') {
				char	*arg = eq + 1;

				*argp = (*arg == '\0') ? NULL : arg;

				return opt;
			}
		}
	}

	/* Check whether help option is specified or not. */
	if ((opt = ctx->cc_helpopt) != NULL &&
	    strcmp(opt->cd_name, optname) == 0) {
		*argp = NULL;

		return opt;
	}

	/* Check whether usage option is specified or not. */
	if ((opt = ctx->cc_usageopt) != NULL &&
	    strcmp(opt->cd_name, optname) == 0) {
		*argp = NULL;

		return opt;
	}

	if (ctx->cc_fallback != NULL) {
		size_t	namelen;

		/* Call fallback function. */
		namelen = (eq == NULL)
			? strlen(optname)
			: (size_t)(eq - optname);
		opt = ctx->cc_fallback(optname, namelen, PFC_TRUE);
		if (opt != NULL) {
			if (eq == NULL) {
				*argp = NULL;
			}
			else {
				char	*arg = eq + 1;

				*argp = (*arg == '\0') ? NULL : arg;
			}

			return opt;
		}
	}

	return NULL;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_cmdopt_opt_desc(pfc_cmdopt_t *PFC_RESTRICT ctx,
 *		       const pfc_cmdopt_def_t *PFC_RESTRICT opt,
 *		       uint32_t flags)
 *	Construct description message that represents format of the specified
 *	option. Message is stored to `ctx->cc_buffer'.
 *
 *	`flags' is a bitmask that controls behavior of this function:
 *
 *	PFC_CMDOPT_DESC_USAGE:
 *		Construct message to be displayed as usage message.
 *
 *	PFC_CMDOPT_DESC_NOARG:
 *		Indicate that argument should not be contained in message.
 */
void PFC_ATTR_HIDDEN
pfc_cmdopt_opt_desc(pfc_cmdopt_t *PFC_RESTRICT ctx,
		    const pfc_cmdopt_def_t *PFC_RESTRICT opt,
		    uint32_t flags)
{
	const char	*argdesc, *lb, *rb;

	if ((flags & PFC_CMDOPT_DESC_USAGE) &&
	    (opt->cd_flags & PFC_CMDOPT_DEF_MANDATORY) == 0) {
		/*
		 * Enclose a message using square bracket if this option
		 * is not mandatory.
		 */
		lb = msg_sqleft;
		rb = msg_sqright;
	}
	else {
		lb = rb = msg_empty;
	}

	if (opt->cd_type == PFC_CMDOPT_TYPE_NONE ||
	    (flags & PFC_CMDOPT_DESC_NOARG)) {
		argdesc = NULL;
	}
	else {
		argdesc = (opt->cd_argdesc != NULL)
			? opt->cd_argdesc : msg_arg;
	}

	if (opt->cd_flags & PFC_CMDOPT_DEF_LONGONLY) {
		const char	*name = opt->cd_name;

		if (name == NULL) {
			/*
			 * This can happen when no name is defined in
			 * pfc_cmdopt_def_t returned by the fallback function.
			 */
			name = msg_unknown;
		}
		if (argdesc == NULL) {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s--%s%s", lb, name, rb);
		}
		else {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s--%s %s%s", lb, name, argdesc, rb);
		}
	}
	else if (opt->cd_name == NULL) {
		if (argdesc == NULL) {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s-%c%s", lb, opt->cd_ident, rb);
		}
		else {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s-%c %s%s", lb, opt->cd_ident,
				       argdesc, rb);
		}
	}
	else {
		if (argdesc == NULL) {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s-%c|--%s%s", lb, opt->cd_ident,
				       opt->cd_name, rb);
		}
		else {
			(void)snprintf(ctx->cc_buffer, sizeof(ctx->cc_buffer),
				       "%s-%c|--%s %s%s", lb, opt->cd_ident,
				       opt->cd_name, argdesc, rb);
		}
	}
}

/*
 * static void
 * cmdopt_errmsg(pfc_cmdopt_t *PFC_RESTRICT ctx, const char *PFC_RESTRICT fmt,
 *		 ...)
 *	Print error message to the standard error output.
 *	Message can be specified as printf(3) style.
 *
 * Remarks:
 *	cmdopt_errmsg() does nothing if PFC_CMDOPT_NOERRMSG is specified to
 *	pfc_cmdopt_init().
 */
static void
cmdopt_errmsg(pfc_cmdopt_t *PFC_RESTRICT ctx, const char *PFC_RESTRICT fmt, ...)
{
	va_list	ap;

	if (ctx->cc_flags & PFC_CMDOPT_NOERRMSG) {
		return;
	}

	fprintf(stderr, "*** %s: ERROR: ", ctx->cc_progname);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputc('\n', stderr);
}

/*
 * static pfc_bool_t
 * cmdopt_parse_arg(pfc_cmdopt_t *ctx, char *arg)
 *	Parse option argument.
 *	Parsed argument is set into the specified context.
 *
 * Calling/Exit State:
 *	PFC_TRUE is returned on success. Otherwise PFC_FALSE is returned.
 */
static pfc_bool_t
cmdopt_parse_arg(pfc_cmdopt_t *ctx, char *arg)
{
	const pfc_cmdopt_def_t	*opt = ctx->cc_lastopt;
	int	err;

	if (opt->cd_type == PFC_CMDOPT_TYPE_STRING) {
		ctx->cc_optarg.string = arg;

		return PFC_TRUE;
	}

	if (opt->cd_type == PFC_CMDOPT_TYPE_INT32) {
		err = pfc_strtoi32(arg, &ctx->cc_optarg.i32);
	}
	else {
		PFC_ASSERT(opt->cd_type == PFC_CMDOPT_TYPE_UINT32);
		err = pfc_strtou32(arg, &ctx->cc_optarg.u32);
	}

	if (PFC_EXPECT_TRUE(err == 0)) {
		return PFC_TRUE;
	}

	pfc_cmdopt_opt_desc(ctx, opt, PFC_CMDOPT_DESC_NOARG);
	if (err == ERANGE) {
		ctx->cc_errcode = PFC_CMDOPT_E_BADRANGE;
		cmdopt_errmsg(ctx, "%s: Argument out of range: %s",
			      ctx->cc_buffer, arg);
	}
	else {
		ctx->cc_errcode = PFC_CMDOPT_E_BADNUMBER;
		cmdopt_errmsg(ctx, "%s: Bad integer: %s",
			      ctx->cc_buffer, arg);
	}

	return PFC_FALSE;
}
