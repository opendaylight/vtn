/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_list.c - "list" subcommand.
 *
 * "list" subcommand lists daemon configurations.
 * This subcommand never uses IPC client session.
 */

#include <pfc/plaintext.h>
#include <cmdopt.h>
#include <lbrk.h>
#include "unc_dmctl.h"
#include "dmconf.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE				\
	"List daemon process configuration."

/*
 * Command line options.
 */
#define	OPTCHAR_START_ORDER	's'
#define	OPTCHAR_STOP_ORDER	'S'
#define	OPTCHAR_CLEVENT_ORDER	'c'
#define	OPTCHAR_ALL		'a'
#define	OPTCHAR_NOBREAK		'N'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_START_ORDER, "start-order", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE, "Show starting order of daemons.", NULL},
	{OPTCHAR_STOP_ORDER, "stop-order", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE, "Show stopping order of daemons.", NULL},
	{OPTCHAR_CLEVENT_ORDER, "clevent-order", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE, "Show order of cluster event delivery.", NULL},
	{OPTCHAR_ALL, "all", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Show all attributes of each daemon.", NULL},
	{OPTCHAR_NOBREAK, "no-line-break", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE | PFC_CMDOPT_DEF_HIDDEN,
	 "Disable line breaking.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

#define	MODE_START_ORDER	PFC_CONST_U(0x1)
#define	MODE_STOP_ORDER		PFC_CONST_U(0x2)
#define	MODE_CLEVENT_ORDER	PFC_CONST_U(0x4)
#define	MODE_ALL		PFC_CONST_U(0x8)
#define	MODE_NOBREAK		PFC_CONST_U(0x10)

/*
 * Column layout.
 */
#define	COL_NAME		20
#define	COL_TYPE_INSETS		1
#define	COL_TYPE_POS		(COL_NAME + COL_TYPE_INSETS)
#define	COL_TYPE		12
#define	COL_DESC_INSETS		2
#define	COL_DESC_POS		(COL_TYPE_POS + COL_TYPE + COL_DESC_INSETS)
#define	COL_DESC		(DMCTL_COL_WIDTH - COL_DESC_POS)

static const pfc_ptcol_t	conf_header[] = {
	PFC_PTCOL_INSETS_DECL("Name", COL_NAME, 0),
	PFC_PTCOL_INSETS_DECL("Type", COL_TYPE, COL_TYPE_INSETS),
	PFC_PTCOL_INSETS_DECL("Description", -COL_DESC, COL_DESC_INSETS),
};

#define	COL_ORDER_INSETS	4
#define	COL_ORDER		12
#define	COL_NAME_INSETS		2

static const pfc_ptcol_t	order_header[] = {
	PFC_PTCOL_INSETS_DECL("Order", COL_ORDER, COL_ORDER_INSETS),
	PFC_PTCOL_INSETS_DECL("Name", -COL_NAME, COL_NAME_INSETS),
};

#define	COL_ALL_INSETS		2
#define	COL_ALL_TAG		20

#define	COL_ALL_CMD_INSETS	(COL_ALL_INSETS + 4)
#define	COL_ALL_CMD						\
	(COL_ALL_TAG + COL_ALL_INSETS - COL_ALL_CMD_INSETS)

typedef struct {
	dmconf_iter_t	l_iterator;
	pfc_lbrk_t	l_brk;
	uint32_t	l_mode;
	uint32_t	l_count;
} listctx_t;

#define	LISTCTX_INIT(ctx)						\
	do {								\
		(ctx)->l_iterator = print_dmconf;			\
		(ctx)->l_mode = 0;					\
		(ctx)->l_count = 0;					\
		pfc_lbrk_init(&(ctx)->l_brk, stdout, DMCTL_COL_WIDTH);	\
	} while (0)

#define	OFF2PTR(off)		((pfc_ptr_t)(uintptr_t)(off))
#define	PTR2OFF(ptr)		((uint32_t)(uintptr_t)(ptr))

/*
 * Context to list daemons in "stop_order".
 */
typedef struct {
	uint32_t	ls_offset;	/* offset to dc_stop_order */
	pfc_bool_t	ls_uncd;	/* true if uncd is printed out */
} list_stop_t;

#define	LIST_STOP_INIT(lsp)						\
	do {								\
		(lsp)->ls_offset = offsetof(dmconf_t, dc_stop_order);	\
		(lsp)->ls_uncd = PFC_FALSE;				\
	} while (0)
/*
 * Name of cluster event types.
 */
static const char	*clevent_names[] = {
	"ACT",		/* CLSTAT_EVTYPE_ACT */
};

/*
 * Internal prototypes.
 */
static int	print_dmconf_byname(listctx_t *UNC_RESTRICT lctx,
				    const char *UNC_RESTRICT name);
static void	print_dmconf(dmconf_t *dcp, pfc_ptr_t arg);
static void	print_dmconf_all(dmconf_t *dcp, pfc_ptr_t arg);
static void	print_order(dmconf_t *dcp, pfc_ptr_t arg);
static void	print_stop_order(dmconf_t *dcp, pfc_ptr_t arg);
static void	print_dmcmd(lnc_cmdmap_t *UNC_RESTRICT cmap, const char *label);
static void	print_qstr(const char *str);

/*
 * int
 * cmd_list(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
 *	Run "list" subcommand.
 */
int
cmd_list(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	pfc_listm_t	strlist = PFC_LISTM_INVALID;
	listctx_t	lctx;
	uint32_t	nconf, offset;
	int		argidx, ret = DMCTL_EX_FATAL;
	char		c;

	LISTCTX_INIT(&lctx);

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(spec->cs_fullname, argc, argv,
				 option_spec, str_arg_names, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return DMCTL_EX_FATAL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_START_ORDER:
			lctx.l_mode |= MODE_START_ORDER;
			break;

		case OPTCHAR_STOP_ORDER:
			lctx.l_mode |= MODE_STOP_ORDER;
			break;

		case OPTCHAR_CLEVENT_ORDER:
			lctx.l_mode |= MODE_CLEVENT_ORDER;
			break;

		case OPTCHAR_ALL:
			lctx.l_mode |= MODE_ALL;
			lctx.l_iterator = print_dmconf_all;
			break;

		case OPTCHAR_NOBREAK:
			lctx.l_mode |= MODE_NOBREAK;
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

	/* Load daemon configurations. */
	nconf = dmconf_load();
	if (PFC_EXPECT_FALSE(nconf == UINT32_MAX)) {
		goto out;
	}

	if (PFC_EXPECT_FALSE(nconf == 0)) {
		printf("%sNo daemon is configured.\n", str_prompt_prefix);
		goto succeeded;
	}

	if ((lctx.l_mode & MODE_ALL) == 0) {
		pfc_ptcol_print_header(stdout, conf_header,
				       PFC_ARRAY_CAPACITY(conf_header));
	}

	if (argc != 0) {
		int		err;

		err = uniqstr_create(&strlist);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		/* List requested daemon configurations. */
		for (; argc != 0; argc--, argv++) {
			const char	*arg = *argv;

			err = uniqstr_append(strlist, arg);
			if (err == EEXIST) {
				continue;
			}
			else if (PFC_EXPECT_FALSE(err != 0)) {
				goto out;
			}

			err = print_dmconf_byname(&lctx, arg);
			if (PFC_EXPECT_FALSE(err != 0)) {
				goto out;
			}
		}
	}
	else {
		/* List all configurations. */
		dmconf_iterate(lctx.l_iterator, &lctx);
	}

	if (lctx.l_mode & MODE_START_ORDER) {
		fputs("\n* Starting Order\n\n", stdout);
		pfc_ptcol_print_header(stdout, order_header,
				       PFC_ARRAY_CAPACITY(order_header));
		offset = offsetof(dmconf_t, dc_start_order);
		dmconf_order_iterate(print_order, LNC_ORDTYPE_START, 0,
				     OFF2PTR(offset));
	}

	if (lctx.l_mode & MODE_STOP_ORDER) {
		list_stop_t	lstop;

		fputs("\n* Stopping Order\n\n", stdout);
		pfc_ptcol_print_header(stdout, order_header,
				       PFC_ARRAY_CAPACITY(order_header));
		LIST_STOP_INIT(&lstop);
		dmconf_order_iterate(print_stop_order, LNC_ORDTYPE_STOP, 0,
				     &lstop);
	}

	if (lctx.l_mode & MODE_CLEVENT_ORDER) {
		const uint32_t	ncols = PFC_ARRAY_CAPACITY(order_header);
		uint8_t		evtype;

		fputs("\n* Cluster Event Order\n", stdout);
		offset = offsetof(dmconf_t, dc_clevent_order);

		for (evtype = 0; evtype < CLSTAT_NEVTYPES;
		     evtype++, offset += sizeof(uint32_t)) {
			printf("\n  + Type: %s\n", clevent_names[evtype]);

			pfc_ptcol_print_header(stdout, order_header, ncols);
			dmconf_order_iterate(print_order, LNC_ORDTYPE_CLEVENT,
					     evtype, OFF2PTR(offset));
		}
	}

succeeded:
	ret = DMCTL_EX_OK;

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
 * print_dmconf_byname(listctx_t *UNC_RESTRICT lctx,
 *		       const char *UNC_RESTRICT name)
 *	Print daemon configuration associated with the given name.
 */
static int
print_dmconf_byname(listctx_t *UNC_RESTRICT lctx,
		    const char *UNC_RESTRICT name)
{
	dmconf_t	*dcp;

	dcp = dmconf_get(name);
	if (PFC_EXPECT_FALSE(dcp == NULL)) {
		error("Unknown daemon name: %s", name);

		return EINVAL;
	}

	if (dcp->dc_data == NULL) {
		dcp->dc_data = (pfc_ptr_t)1;
		lctx->l_iterator(dcp, lctx);
	}

	return 0;
}

/*
 * static void
 * print_dmconf(dmconf_t *dcp, pfc_ptr_t arg)
 *	Print daemon configuration.
 */
static void
print_dmconf(dmconf_t *dcp, pfc_ptr_t arg)
{
	listctx_t	*lctx = (listctx_t *)arg;
	uint32_t	mode = lctx->l_mode;
	const char	*tstr = proctype_getname(dcp->dc_type);

	if (mode & MODE_NOBREAK) {
		printf("%*s%*s%*s%*s%s\n",
		       COL_NAME, DMCONF_NAME(dcp),
		       COL_TYPE_INSETS, str_empty,
		       COL_TYPE, tstr,
		       -COL_DESC_INSETS, str_empty,
		       DMCONF_DESC(dcp));
	}
	else {
		pfc_lbrk_t	*lbp = &lctx->l_brk;
		char		line[128];

		snprintf(line, sizeof(line), "%*s%*s%*s%*s",
			 COL_NAME, DMCONF_NAME(dcp),
			 COL_TYPE_INSETS, str_empty,
			 COL_TYPE, tstr,
			 -COL_DESC_INSETS, str_empty);

		pfc_lbrk_print_raw(lbp, line);
		pfc_lbrk_print(lbp, COL_DESC_POS, -1, DMCONF_DESC(dcp), 0);
		pfc_lbrk_newline(lbp);
	}
}

/*
 * static void
 * print_dmconf_all(dmconf_t *dcp, pfc_ptr_t arg)
 *	Print all attributes of the given daemon configuration.
 */
static void
print_dmconf_all(dmconf_t *dcp, pfc_ptr_t arg)
{
	uint32_t	*ordp;
	const char	*sep = str_empty;
	listctx_t	*lctx = (listctx_t *)arg;

	if (lctx->l_count != 0) {
		putchar('\n');
	}

	printf("* Name: %s\n", DMCONF_NAME(dcp));

	printf("%*s%*s = %s\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Description",
	       DMCONF_DESC(dcp));
	printf("%*s%*s = %u (%s)\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Process Type",
	       dcp->dc_type, proctype_getname(dcp->dc_type));
	printf("%*s%*s = %s\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "UNC daemon",
	       (dcp->dc_uncd) ? str_true : str_false);
	print_dmcmd(dcp->dc_command, "Command");
	printf("%*s%*s = %s\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Start Wait",
	       (dcp->dc_start_wait) ? str_true : str_false);
	printf("%*s%*s = %u (msec)\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Start Timeout",
	       dcp->dc_start_timeout);
	printf("%*s%*s = %u\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Starting Order",
	       dcp->dc_start_order);

	if (dcp->dc_stop != NULL) {
		print_dmcmd(dcp->dc_stop, "Stop Command");
	}

	printf("%*s%*s = %s (%d)\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Stop Signal",
	       DMCONF_STOP_SIGNAL(dcp), dcp->dc_stop_sig);
	printf("%*s%*s = %u (msec)\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Stop Timeout",
	       dcp->dc_stop_timeout);
	printf("%*s%*s = %u\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Stopping Order",
	       dcp->dc_stop_order);

	printf("%*s%*s = [",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Cluster Event Order");
	for (ordp = dcp->dc_clevent_order;
	     ordp < PFC_ARRAY_LIMIT(dcp->dc_clevent_order); ordp++) {
		printf("%s%u", sep, *ordp);
		sep = ", ";
	}
	fputs("]\n", stdout);

	printf("%*s%*s = %d\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, "Stderr Rotation",
	       dcp->dc_stderr_rotate);

	lctx->l_count++;
}

/*
 * static void
 * print_order(dmconf_t *dcp, pfc_ptr_t arg)
 *	Print order value in the daemon configuration.
 */
static void
print_order(dmconf_t *dcp, pfc_ptr_t arg)
{
	uint32_t	off = PTR2OFF(arg);

	printf("%*s%*u%*s%s\n",
	       COL_ORDER_INSETS, str_empty,
	       COL_ORDER, DMCONF_FETCH_UINT32(dcp, off),
	       COL_NAME_INSETS, str_empty,
	       DMCONF_NAME(dcp));
}

/*
 * static void
 * print_stop_order(dmconf_t *dcp, pfc_ptr_t arg)
 *	Print "stop_order" value in the daemon configuration.
 */
static void
print_stop_order(dmconf_t *dcp, pfc_ptr_t arg)
{
	list_stop_t	*lsp = (list_stop_t *)arg;
	uint32_t	order = DMCONF_FETCH_UINT32(dcp, lsp->ls_offset);

	if (!lsp->ls_uncd && order >= LNC_ORDER_UNCD) {
		/* Insert UNC daemon here. */
		printf("%*s%*s%*s%s\n",
		       COL_ORDER_INSETS, str_empty,
		       COL_ORDER, str_hyphen,
		       COL_NAME_INSETS, str_empty,
		       "Modules in uncd");
		lsp->ls_uncd = PFC_TRUE;
	}

	printf("%*s%*u%*s%s\n",
	       COL_ORDER_INSETS, str_empty,
	       COL_ORDER, order,
	       COL_NAME_INSETS, str_empty,
	       DMCONF_NAME(dcp));
}

/*
 * static void
 * print_dmcmd(lnc_cmdmap_t *UNC_RESTRICT cmap, const char *label)
 *	Print contents of dmcmd_t.
 */
static void
print_dmcmd(lnc_cmdmap_t *UNC_RESTRICT cmap, const char *label)
{
	pfc_listm_t	args = cmap->cm_args;
	int		argc, i;

	printf("%*s%*s = {\n",
	       -COL_ALL_INSETS, str_empty,
	       -COL_ALL_TAG, label);

	printf("%*s%*s = %s\n",
	       -COL_ALL_CMD_INSETS, str_empty,
	       -COL_ALL_CMD, "Path",
	       LIBLNC_CMDMAP_PATH(cmap));

	argc = pfc_listm_get_size(args);
	for (i = 0; i < argc; i++) {
		pfc_refptr_t	*arg;
		char		buf[32];

		PFC_ASSERT_INT(pfc_listm_getat(args, i, (pfc_cptr_t *)&arg),
			       0);
		snprintf(buf, sizeof(buf), "Argv[%d]", i);
		printf("%*s%*s = ",
		       -COL_ALL_CMD_INSETS, str_empty,
		       -COL_ALL_CMD, buf);

		print_qstr(pfc_refptr_string_value(arg));
		putchar('\n');
	}

	printf("%*s}\n",
	       -COL_ALL_INSETS, str_empty);
}

/*
 * static void
 * print_qstr(const char *str)
 *	Print the given string as quoted string.
 */
static void
print_qstr(const char *str)
{
	char	c;

	putchar('"');
	for (; (c = *str) != '\0'; str++) {
		if (c == '"') {
			putchar('\\');
		}
		putchar(c);
	}
	putchar('"');
}
