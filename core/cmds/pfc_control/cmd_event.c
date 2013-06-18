/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_event.c - "event" subcommand.
 *
 * "event" subcommand sends one or more event to modules in the PFC daemon.
 */

#include <stdio.h>
#include <string.h>
#include <cmdopt.h>
#include <pfc/event.h>
#include <pfc/rbtree.h>
#include <pfc/strtoint.h>
#include "pfc_control.h"
#include "modinfo.h"

/*
 * Help message.
 */
#define	HELP_MESSAGE_1							\
	"Send event to modules in" PFC_LBRK_SPACE_STR
#define	HELP_MESSAGE_2			\
	".\nEach command line argument must be a pair of target module " \
	"name and event type combined with \":\". If target module is "	\
	"omitted, the event is sent to all modules."

/*
 * Brief description of arguments.
 */
static const char	arg_format[] = "EVENT [...]";

/*
 * Command line options.
 */
static const pfc_cmdopt_def_t	option_spec[] = {
	{'y', "yes", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "Do not prompt for confirmation before sending events.", NULL},

	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * A pair of target module and event type.
 */
typedef struct {
	char		mek_name[PFC_MODULE_NAME_MAX + 1];
	uint32_t	mek_type;
} modev_key_t;

typedef const modev_key_t	modev_ckey_t;

struct modev_ent;
typedef struct modev_ent	modev_ent_t;

struct modev_ent {
	modev_key_t	mev_key;	/* pair of module and event type */
	int		mev_result;	/* result */
	pfc_rbnode_t	mev_node;	/* Red-Black tree node */
	modev_ent_t	*mev_next;	/* next link */
};

#define	mev_name	mev_key.mek_name
#define	mev_type	mev_key.mek_type

#define	MODEV_ENT_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), modev_ent_t, mev_node)

/*
 * List of module events to be sent.
 */
typedef struct {
	modev_ent_t	*me_list;	/* list of module events */
	modev_ent_t	**me_last;	/* last entry of module events */
	pfc_rbtree_t	me_tree;	/* Red-Black tree for module events */
	uint32_t	me_count;	/* number of events */
	uint32_t	me_flags;	/* flags */
} modevent_t;

static modevent_t	module_events;

/*
 * Flags for me_flags.
 */
#define	MODEVF_NOPROMPT		0x1U	/* don't prompt */

/*
 * Format of confirmation prompt.
 */
#define	EVENT_COL_INDENT	4
#define	EVENT_COL_TARGET	PFC_MODULE_NAME_MAX
#define	EVENT_COL_TYPE		16
#define	EVENT_COL_RESULT	20

/*
 * Maximum length of string representation of event type.
 */
#define	EVENT_TYPE_BUFSIZE	32

/*
 * Symbolic name of global event type.
 */
static const char	*modevent_globals[] = {
	"RELOAD",		/* PFC_MODEVENT_TYPE_RELOAD */
};

/*
 * Separator of module name and event type.
 */
#define	MODEVENT_SEP		':'

/*
 * True if the module event is broadcast event.
 */
#define	EVENT_IS_BROADCAST(target)	(*(target) == '\0')

/*
 * Internal prototypes.
 */
static int	event_parse(modevent_t *PFC_RESTRICT mep,
			    int argc, char **PFC_RESTRICT argv);
static int	event_parse_type(const char *PFC_RESTRICT arg,
				 uint32_t *PFC_RESTRICT typep);
static int	event_parse_modevent(const char *PFC_RESTRICT arg,
				     modev_key_t *PFC_RESTRICT key);
static int	event_parse_global(const char *PFC_RESTRICT arg,
				   modev_key_t *PFC_RESTRICT key);
static int	event_add(modevent_t *PFC_RESTRICT mep,
			  modev_ckey_t *PFC_RESTRICT key);
static void	event_destroy(modevent_t *mep);
static void	event_putchar(char c, uint32_t count);
static void	event_list(modevent_t *mep, pfc_bool_t do_result);

static const char	*event_type_name(uint32_t type);
static ctrlcmd_ret_t	event_prompt(modevent_t *PFC_RESTRICT mep,
				     cproto_sess_t *PFC_RESTRICT sess);
static int		event_compare(pfc_cptr_t arg1, pfc_cptr_t arg2);
static pfc_cptr_t	event_getkey(pfc_rbnode_t *node);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * modevent_init(modevent_t *mep)
 *	Initialize modevent_t.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
modevent_init(modevent_t *mep)
{
	mep->me_list = NULL;
	mep->me_last = &mep->me_list;
	mep->me_count = 0;
	mep->me_flags = 0;
	pfc_rbtree_init(&mep->me_tree, event_compare, event_getkey);
}

/*
 * ctrlcmd_ret_t
 * cmd_ctor_event(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
 *		  char **PFC_RESTRICT argv)
 *	Constructor of "event" subcommand.
 */
ctrlcmd_ret_t
cmd_ctor_event(const ctrlcmd_spec_t *PFC_RESTRICT spec, int argc,
	       char **PFC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	ctrlcmd_ret_t	ret = CMDRET_FAIL;
	modevent_t	*mep = &module_events;
	int		argidx, err;
	char		c;

	/* Reset module event information. */
	modevent_init(mep);

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(CTRLCMD_FULLNAME(spec), argc, argv,
				 option_spec, arg_format, 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return CMDRET_FAIL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case 'y':
			mep->me_flags |= MODEVF_NOPROMPT;
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

	if (PFC_EXPECT_FALSE(argc == 0)) {
		error("At least one event must be specified.");
		goto out;
	}

	err = event_parse(mep, argc, argv);
	if (PFC_EXPECT_FALSE(err != 0)) {
		ret = CMDRET_FAIL;
		goto out;
	}

	pfc_cmdopt_destroy(parser);

	return CMDRET_CONT;

out:
	pfc_cmdopt_destroy(parser);
	event_destroy(&module_events);

	return ret;
}

/*
 * ctrlcmd_ret_t
 * cmd_prepare_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		     cproto_sess_t *PFC_RESTRICT sess)
 *	Prepare execution of "EVENT" protocol command.
 *
 * Calling/Exit State:
 *	Upon successful completion, CMDRET_CONT is returned.
 *	CMDRET_COMPLETE is returned if the command is canceled due to user
 *	request.
 *	CMDRET_FAIL is returned on failure.
 */
ctrlcmd_ret_t
cmd_prepare_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		  cproto_sess_t *PFC_RESTRICT sess)
{
	modevent_t	*mep = &module_events;
	modev_ent_t	*entp;
	modinfo_t	minfo;
	ctrlcmd_ret_t	ret = CMDRET_FAIL;
	int		err;

	/* Fetch module information. */
	err = modinfo_fetch(sess, &minfo);
	if (PFC_EXPECT_FALSE(err != 0)) {
		modinfo_free(&minfo);

		return CMDRET_FAIL;
	}

	/* Ensure that target module is valid. */
	for (entp = mep->me_list; entp != NULL; entp = entp->mev_next) {
		const char	*name = entp->mev_name;
		modinfo_ent_t	*mip;

		if (EVENT_IS_BROADCAST(name) || PFC_MODULE_IS_CORE(name)) {
			continue;
		}

		mip = modinfo_get(&minfo, name);
		if (PFC_EXPECT_FALSE(mip == NULL)) {
			error("Unknown target module name: %s", name);
			goto out;
		}

		if (PFC_EXPECT_FALSE(mip->mie_state != PMOD_STATE_RUNNING)) {
			error("Target module is not active: %s", name);
			goto out;
		}
	}

	if (mep->me_flags & MODEVF_NOPROMPT) {
		ret = CMDRET_CONT;
	}
	else {
		ret = event_prompt(mep, sess);
	}

out:
	modinfo_free(&minfo);

	return ret;
}

/*
 * int
 * cmd_send_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		  cproto_sess_t *PFC_RESTRICT sess)
 *	Send optional argument for EVENT command.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cmd_send_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
	       cproto_sess_t *PFC_RESTRICT sess)
{
	modevent_t	*mep = &module_events;
	modev_ent_t	*entp;
	int		err;

	/* Send the number of events. */
	err = cproto_data_write_int32(sess, mep->me_count);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to send the number of events: %s",
		      strerror(err));

		return err;
	}

	/* Send pairs of target module and event type. */
	for (entp = mep->me_list; entp != NULL; entp = entp->mev_next) {
		const char	*name = entp->mev_name;
		uint32_t	type = entp->mev_type;

		if (EVENT_IS_BROADCAST(name)) {
			err = cproto_data_write_null(sess);
		}
		else {
			err = cproto_data_write_text(sess, name);
		}
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to send target module name: %s",
			      strerror(err));
			break;
		}

		err = cproto_data_write_int32(sess, type);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to send event type: %s", strerror(err));
			break;
		}
	}

	return err;
}

/*
 * int
 * cmd_receive_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
 *		     cproto_sess_t *PFC_RESTRICT sess)
 *	Receive optional response for EVENT command.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	-1 is returned if at least one event could not be sent.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
cmd_receive_event(const ctrlcmd_spec_t *PFC_RESTRICT spec,
		  cproto_sess_t *PFC_RESTRICT sess)
{
	modevent_t	*mep = &module_events;
	modev_ent_t	*entp;
	int		result = 0;

	for (entp = mep->me_list; entp != NULL; entp = entp->mev_next) {
		cproto_data_t	data;
		int		err;

		err = cproto_data_read(sess, &data);
		if (PFC_EXPECT_FALSE(err != 0)) {
			error("Failed to read response: %s", strerror(err));

			return err;
		}

		if (PFC_EXPECT_FALSE(data.cpd_type != CTRL_PDTYPE_INT32)) {
			error("Unexpected response type: %u", data.cpd_type);
			cproto_data_free(&data);

			return EPROTO;
		}

		entp->mev_result = cproto_data_uint32(&data);
		cproto_data_free(&data);
		if (PFC_EXPECT_FALSE(entp->mev_result != 0)) {
			result = -1;
		}
	}

	/* Print out results. */
	event_list(mep, PFC_TRUE);

	return result;
}

/*
 * void
 * cmd_dtor_event(const ctrlcmd_spec_t *spec)
 *	Destructor of "event" subcommand.
 */
void
cmd_dtor_event(const ctrlcmd_spec_t *spec)
{
	event_destroy(&module_events);
}

/*
 * static int
 * event_parse(modevent_t *PFC_RESTRICT mep, int argc,
 *	       char **PFC_RESTRICT argv)
 *	Parse command line arguments as pairs of target module name and
 *	event type.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed module events are stored to `*mep',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
event_parse(modevent_t *PFC_RESTRICT mep, int argc, char **PFC_RESTRICT argv)
{
	PFC_ASSERT(argc > 0);

	for (; argc != 0; argc--, argv++) {
		const char	*arg = *argv;
		modev_key_t	key;
		int		err;

		err = event_parse_modevent(arg, &key);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}

		err = event_add(mep, &key);
		if (PFC_EXPECT_FALSE(err != 0)) {
			return err;
		}
	}

	return 0;
}

/*
 * static uint32_t
 * event_parse_type(const char *PFC_RESTRICT arg,
 *		    uint32_t *PFC_RESTRICT typep)
 *	Parse the given string as event type.
 *
 * Calling/Exit State:
 *	Upon successful completion, value of event type is stored to `*typep',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
event_parse_type(const char *PFC_RESTRICT arg, uint32_t *PFC_RESTRICT typep)
{
	const char	**pp;
	int		err;

	/* Try to parse as symbolic name of global event type. */
	for (pp = modevent_globals; pp < PFC_ARRAY_LIMIT(modevent_globals);
	     pp++) {
		if (strcasecmp(*pp, arg) == 0) {
			*typep = (pp - modevent_globals) +
				PFC_MODEVENT_TYPE_SYSTEM_MIN;

			return 0;
		}
	}

	/* Parse as numeric value. */
	err = pfc_strtou32(arg, typep);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}
	if (PFC_EXPECT_FALSE(!PFC_MODEVENT_TYPE_IS_VALID(*typep))) {
		return EINVAL;
	}

	return 0;
}

/*
 * static int
 * event_parse_modevent(const char *PFC_RESTRICT arg,
 *			modev_key_t *PFC_RESTRICT key)
 *	Parse the given string as module event, which is a pair of the target
 *	module name and event type combined with ":".
 *
 * Calling/Exit State:
 *	Upon successful completion, the target module name and event type is
 *	stored to `*key', and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
event_parse_modevent(const char *PFC_RESTRICT arg,
			modev_key_t *PFC_RESTRICT key)
{
	const char	*sep, *p;
	char		*dst, *limit;
	int		err;

	sep = strchr(arg, MODEVENT_SEP);
	if (PFC_EXPECT_FALSE(sep == NULL)) {
		/* This must be a global event to be broadcasted. */
		return event_parse_global(arg, key);
	}

	limit = PFC_ARRAY_LIMIT(key->mek_name) - 1;
	for (p = arg, dst = key->mek_name; p < sep; p++, dst++) {
		if (PFC_EXPECT_FALSE(dst >= limit)) {
			error("Too long target module name: %s", arg);

			return EINVAL;
		}

		*dst = *p;
	}

	PFC_ASSERT(dst <= limit);
	*dst = '\0';

	/* Parse event type. */
	err = event_parse_type(sep + 1, &key->mek_type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Invalid event type: %s", arg);

		return err;
	}

	return 0;
}

/*
 * static int
 * event_parse_global(const char *PFC_RESTRICT arg,
 *		      modev_key_t *PFC_RESTRICT key)
 *	Parse the given string as global broadcast event.
 *
 * Calling/Exit State:
 *	Upon successful completion, parsed module events are stored to `*key',
 *	and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
event_parse_global(const char *PFC_RESTRICT arg, modev_key_t *PFC_RESTRICT key)
{
	int	err;

	err = event_parse_type(arg, &key->mek_type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Invalid event type: %s", arg);

		return err;
	}

	if (PFC_EXPECT_FALSE(PFC_MODEVENT_TYPE_IS_LOCAL(key->mek_type))) {
		error("Event type of broadcast event must be a global event "
		      "type, which is greater than %u.",
		      PFC_MODEVENT_TYPE_LOCAL_MAX);

		return EINVAL;
	}

	key->mek_name[0] = '\0';

	return 0;
}

/*
 * static int
 * event_add(modevent_t *PFC_RESTRICT mep, modev_ckey_t *PFC_RESTRICT key)
 *	Add the given pair of module name and event type to modevent_t.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
event_add(modevent_t *PFC_RESTRICT mep, modev_ckey_t *PFC_RESTRICT key)
{
	modev_ent_t	*entp;
	int		err;

	entp = (modev_ent_t *)malloc(sizeof(*entp));
	if (PFC_EXPECT_FALSE(entp == NULL)) {
		error("No memory for module event entry.");

		return ENOMEM;
	}

	entp->mev_key = *key;
	entp->mev_next = NULL;
	entp->mev_result = 0;

	err = pfc_rbtree_put(&mep->me_tree, &entp->mev_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		/* Eliminate duplicates. */
		PFC_ASSERT(err == EEXIST);
		free(entp);
	}
	else {
		*(mep->me_last) = entp;
		mep->me_last = &entp->mev_next;
		mep->me_count++;
	}

	return 0;
}

/*
 * static void
 * event_destroy(modevent_t *mep)
 *	Free up resources held by modevent_t specified by `mep'.
 */
static void
event_destroy(modevent_t *mep)
{
	modev_ent_t	*entp, *next;

	for (entp = mep->me_list; entp != NULL; entp = next) {
		next = entp->mev_next;
		free(entp);
	}

	mep->me_list = NULL;
}

/*
 * static void
 * event_putchar(char c, uint32_t count)
 *	Print out the given character to the standard output.
 */
static void
event_putchar(char c, uint32_t count)
{
	for (; count > 0; count--) {
		putchar(c);
	}
}

/*
 * static void
 * event_list(modevent_t *mep, pfc_bool_t do_result)
 *	List the contents of module event list to the standard output.
 *
 *	If PFC_TRUE is specified to `do_result', value of mev_result is
 *	printed out.
 */
static void
event_list(modevent_t *mep, pfc_bool_t do_result)
{
	modev_ent_t	*entp;

	/* Print out the header. */
	putchar('\n');
	event_putchar(' ', EVENT_COL_INDENT);
	printf("%*s %*s",
	       -EVENT_COL_TARGET, "Target Module",
	       -EVENT_COL_TYPE, "Event Type");
	if (do_result) {
		printf(" %*s\n", -EVENT_COL_RESULT, "Result");
	}
	else {
		putchar('\n');
	}

	event_putchar(' ', EVENT_COL_INDENT);
	event_putchar('-', EVENT_COL_TARGET);
	putchar(' ');
	event_putchar('-', EVENT_COL_TYPE);

	if (do_result) {
		putchar(' ');
		event_putchar('-', EVENT_COL_RESULT);
	}
	putchar('\n');

	/* Print out the content of module events. */
	for (entp = mep->me_list; entp != NULL; entp = entp->mev_next) {
		uint32_t	type = entp->mev_type;
		const char	*tname = event_type_name(type);
		const char	*name = entp->mev_name;
		char		buf[EVENT_TYPE_BUFSIZE];

		event_putchar(' ', EVENT_COL_INDENT);
		if (EVENT_IS_BROADCAST(name)) {
			name = "<all modules>";
		}

		if (tname == NULL) {
			snprintf(buf, sizeof(buf), "%u", type);
		}
		else {
			snprintf(buf, sizeof(buf), "%u (%s)", type, tname);
		}

		printf("%*s %*s",
		       -EVENT_COL_TARGET, name,
		       -EVENT_COL_TYPE, buf);

		if (do_result) {
			int		result = entp->mev_result;

			if (result == 0) {
				fputs(" OK", stdout);
			}
			else {
				printf(" FAILED (%s)", strerror(result));
			}
		}

		putchar('\n');
	}
}

/*
 * static const char *
 * event_type_name(uint32_t type)
 *	Return the symbolic name of the given event type.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non NULL pointer to the symbolic name
 *	of the given event type is returned.
 *	NULL is returned if the given event type is unknown.
 */
static const char *
event_type_name(uint32_t type)
{
	const char	**pp;

	if (type < PFC_MODEVENT_TYPE_SYSTEM_MIN ||
	    type > PFC_MODEVENT_TYPE_SYSTEM_MAX) {
		return NULL;
	}

	type -= PFC_MODEVENT_TYPE_SYSTEM_MIN;
	pp = &modevent_globals[type];
	PFC_ASSERT(pp < PFC_ARRAY_LIMIT(modevent_globals));

	return *pp;
}

/*
 * static ctrlcmd_ret_t
 * event_prompt(modevent_t *PFC_RESTRICT mep, cproto_sess_t *PFC_RESTRICT sess)
 *	Prompt the user for whether to proceed.
 *
 * Calling/Exit State:
 *	CMDRET_CONT is returned if the user wants to proceed.
 *	CMDRET_COMPLETE is returned if the user wants to cancel.
 *	CMDRET_FAIL is returned on failure.
 */
static ctrlcmd_ret_t
event_prompt(modevent_t *PFC_RESTRICT mep, cproto_sess_t *PFC_RESTRICT sess)
{
	uint32_t	nevents = mep->me_count;
	int		ret, err;

	/* Destroy the session in order not to block other protocol session. */
	pfc_ctrl_client_destroy(sess);

	/* Print out module events to be sent. */
	event_list(mep, PFC_FALSE);

	printf("\n%s%u event%s listed above will be sent.\n",
	       str_prompt_prefix, nevents, ENGLISH_PLUAL(nevents));
	ret = prompt();
	if (PFC_EXPECT_FALSE(ret < 0)) {
		error("Failed to read answer.");

		return CMDRET_FAIL;
	}
	else if (ret == 0) {
		printf("%sCanceled.\n", str_prompt_prefix);

		return CMDRET_COMPLETE;
	}

	/* Reconnect the session. */
	err = client_create(sess, NULL);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return CMDRET_FAIL;
	}

	return CMDRET_CONT;
}

/*
 * static int
 * event_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
 *	Compare given two pairs of module name and event type.
 *
 *	`arg1' and `arg2' must be pointers to modev_ckey_t.
 */
static int
event_compare(pfc_cptr_t arg1, pfc_cptr_t arg2)
{
	modev_ckey_t	*mk1 = (modev_ckey_t *)arg1;
	modev_ckey_t	*mk2 = (modev_ckey_t *)arg2;
	const char	*name1 = mk1->mek_name;
	const char	*name2 = mk2->mek_name;
	uint32_t	t1, t2;
	int		ret;

	ret = strcmp(name1, name2);
	if (ret != 0) {
		return ret;
	}

	/* Compare event type. */
	t1 = mk1->mek_type;
	t2 = mk2->mek_type;

	if (t1 == t2) {
		return 0;
	}

	return (t1 < t2) ? -1 : 1;
}

/*
 * static pfc_cptr_t
 * event_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified module event.
 *	`node' must be a pointer to mev_key in modevent_t.
 */
static pfc_cptr_t
event_getkey(pfc_rbnode_t *node)
{
	modev_ent_t	*entp = MODEV_ENT_NODE2PTR(node);

	return (pfc_cptr_t)&entp->mev_key;
}
