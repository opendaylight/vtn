/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmd_clevent.c - "clevent" subcommand.
 *
 * "clevent" lets the UNC daemon raise a cluster state change event.
 */

#include <string.h>
#include <cmdopt.h>
#include <pfc/strtoint.h>
#include "unc_dmctl.h"

/*
 * clevent specific exit status.
 */

/* Cluster state is not changed. */
#define	CLEVENT_EX_UNCHANGED	DMCTL_EX_SUBCMD

/* Cluster node is in error state. */
#define	CLEVENT_EX_ERRSTATE	(DMCTL_EX_SUBCMD + 1)

/* Failed to change cluster state. */
#define	CLEVENT_EX_FAILED	(DMCTL_EX_SUBCMD + 2)

/*
 * Exit status table associated with result of CLEVENT service.
 * Array index must be the response code of CLEVENT service.
 */
static const int	clevent_extable[] = {
	DMCTL_EX_OK,			/* LNC_IPC_CLEVENT_OK */
	DMCTL_EX_BUSY,			/* LNC_IPC_CLEVENT_BUSY */
	CLEVENT_EX_UNCHANGED,		/* LNC_IPC_CLEVENT_UNCHANGED */
	DMCTL_EX_FATAL,			/* LNC_IPC_CLEVENT_INVALID */
	CLEVENT_EX_ERRSTATE,		/* LNC_IPC_CLEVENT_ERRSTATE */
	DMCTL_EX_BUSY,			/* LNC_IPC_CLEVENT_TC_BUSY */
	CLEVENT_EX_FAILED,		/* LNC_IPC_CLEVENT_TC_FAILED */
	CLEVENT_EX_FAILED,		/* LNC_IPC_CLEVENT_FAILED */
};

/*
 * Help message.
 */
#define	HELP_MESSAGE							\
	"Let the UNC daemon raise a cluster state change event."

/*
 * Default interval between attempts to connect to the UNC daemon.
 */
#define	CLEV_CONN_INTERVAL	1000			/* 1 second */
#define	CLEV_CONN_INTERVAL_MIN	PFC_CONST_U(1)		/* 1 milliseconds */
#define	CLEV_CONN_INTERVAL_MAX	PFC_CONST_U(60000)	/* 1 minute */

/*
 * Timeout, in seconds, for LNCSTAT service.
 */
#define	CLEV_LNCSTAT_TIMEOUT	PFC_CONST_U(5)

/*
 * Command line options.
 */
#define	OPTCHAR_LIST		'l'
#define	OPTCHAR_INTERVAL	'i'
#define	OPTCHAR_STOPONERR	'E'

static const pfc_cmdopt_def_t	option_spec[] = {
	{OPTCHAR_LIST, "list", PFC_CMDOPT_TYPE_NONE, PFC_CMDOPT_DEF_ONCE,
	 "List supported cluster event types and exit.", NULL},
	{OPTCHAR_INTERVAL, "connect-interval", PFC_CMDOPT_TYPE_UINT32,
	 PFC_CMDOPT_DEF_ONCE,
	 "Interval, in milliseconds, between attempts to connect to the "
	 "UNC daemon.\n(default: " PFC_XSTRINGIFY(CLEV_CONN_INTERVAL)
	 " milliseconds)", str_msecs},
	{OPTCHAR_STOPONERR, "stop-on-error", PFC_CMDOPT_TYPE_NONE,
	 PFC_CMDOPT_DEF_ONCE,
	 "Terminate uncd if uncd fails to change cluster state.", NULL},
	{PFC_CMDOPT_EOF, NULL, PFC_CMDOPT_TYPE_NONE, 0, NULL, NULL}
};

/*
 * Supported cluster event types.
 */
struct clevdesc {
	const char	*ced_name;		/* name of event type */
	const char	*ced_desc;		/* description */
};

typedef const struct clevdesc	clevdesc_t;

#define	CLEVDESC_DECL(name, desc)		\
	{					\
		.ced_name	= name,		\
		.ced_desc	= desc,		\
	}

/*
 * Keep all supported types of cluster state change event.
 * Cluster event type, CLSTAT_EVTYPE_XXX is used as array index.
 */
static clevdesc_t	clevent_types[] = {
	CLEVDESC_DECL("ACT",
		      "Notify that the cluster node has become active."),
};

/*
 * Layout for event type listing.
 */
#define	COL_INDENT		4
#define	COL_EVTYPE		6

/*
 * Context to invoke CLEVENT service.
 */
typedef struct {
	pfc_ipcsess_t	*cc_sess;	/* IPC client session */
	uint32_t	cc_interval;	/* connect interval */
	uint8_t		cc_type;	/* cluster event type */
	uint8_t		cc_mode;	/* internal mode flags */
	pfc_timespec_t	cc_limit;	/* deadline of state transition */
} clevent_ctx_t;

#define	CLEVENT_CTX_INIT(ctx, type, mode, interval)			\
	do {								\
		pfc_timespec_t	*__limit = &(ctx)->cc_limit;		\
									\
		(ctx)->cc_sess = NULL;					\
		(ctx)->cc_type = (type);				\
		(ctx)->cc_mode = (mode);				\
		(ctx)->cc_interval = (interval);			\
									\
		PFC_ASSERT_INT(pfc_clock_gettime(__limit), 0);		\
		__limit->tv_sec += ipc_timeout;				\
		debug_printf(1, "interval=%u, timeout=%u, limit=%lu.%lu", \
			     (interval), ipc_timeout,			\
			     __limit->tv_sec, __limit->tv_nsec);	\
	} while (0)

#define	CLEVENT_CTX_FINI(ctx)						\
	do {								\
		pfc_ipcsess_t	*__sess = (ctx)->cc_sess;		\
									\
		if (__sess != NULL) {					\
			PFC_ASSERT_INT(pfc_ipcclnt_sess_destroy(__sess), \
				       0);				\
		}							\
	} while (0)

/*
 * Internal mode flags for cc_mode.
 */
#define	CLEV_MODE_LIST		PFC_CONST_U(0x01)
#define	CLEV_MODE_STOPONERR	PFC_CONST_U(0x02)

/*
 * Internal prototypes.
 */
static void	clevent_list(void);
static int	clevent_convert(const char *UNC_RESTRICT name,
				uint8_t *UNC_RESTRICT typep);
static int	clevent_setup(clevent_ctx_t *ctx);
static int	clevent_checkstate(pfc_ipcsess_t *sess);
static int	clevent_invoke(clevent_ctx_t *ctx);

/*
 * int
 * cmd_clevent(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
 *	Run "clevent" subcommand.
 */
int
cmd_clevent(cmdspec_t *UNC_RESTRICT spec, int argc, char **UNC_RESTRICT argv)
{
	pfc_cmdopt_t	*parser;
	clevent_ctx_t	ctx;
	uint32_t	interval = CLEV_CONN_INTERVAL;
	uint8_t		mode = 0, type;
	int		argidx, err, ret = DMCTL_EX_FATAL;
	char		c;

	/* Create command line option parser. */
	parser = pfc_cmdopt_init(spec->cs_fullname, argc, argv,
				 option_spec, "STATE", 0);
	if (PFC_EXPECT_FALSE(parser == NULL)) {
		error("Failed to create option parser.");

		return DMCTL_EX_FATAL;
	}

	while ((c = pfc_cmdopt_next(parser)) != PFC_CMDOPT_EOF) {
		switch (c) {
		case OPTCHAR_LIST:
			mode |= CLEV_MODE_LIST;
			break;

		case OPTCHAR_STOPONERR:
			mode |= CLEV_MODE_STOPONERR;
			break;

		case OPTCHAR_INTERVAL:
			interval = pfc_cmdopt_arg_uint32(parser);
			if (PFC_EXPECT_FALSE(interval <
					     CLEV_CONN_INTERVAL_MIN)) {
				error("-%c: Interval value must be greater "
				      "than or equal %u.", OPTCHAR_INTERVAL,
				      CLEV_CONN_INTERVAL_MIN);
				goto out;
			}
			else if (PFC_EXPECT_FALSE(interval >
						  CLEV_CONN_INTERVAL_MAX)) {
				error("-%c: Interval value must be less than "
				      "or equal %u.", OPTCHAR_INTERVAL,
				      CLEV_CONN_INTERVAL_MAX);
				goto out;
			}
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

	if (mode & CLEV_MODE_LIST) {
		clevent_list();
		ret = DMCTL_EX_OK;
		goto out;
	}

	if (PFC_EXPECT_FALSE(argc != 1)) {
		if (argc == 0) {
			error("No cluster event type is specified.");
		}
		else {
			error("Only one cluster event type must be "
			      "specified.");
		}
		goto out;
	}

	err = clevent_convert(*argv, &type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Invalid cluster event type: %s", *argv);
		goto out;
	}

	if (PFC_EXPECT_FALSE(interval >= ipc_timeout * PFC_CLOCK_MILLISEC)) {
		error("Connection interval (%u) must be less than "
		      "session timeout (%u).",
		      interval, ipc_timeout * PFC_CLOCK_MILLISEC);
		goto out;
	}

	/* Invoke CLEVENT service to change cluster state. */
	CLEVENT_CTX_INIT(&ctx, type, mode, interval);
	ret = clevent_invoke(&ctx);
	mode = ctx.cc_mode;
	CLEVENT_CTX_FINI(&ctx);

	if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK) &&
	    (mode & CLEV_MODE_STOPONERR)) {
		pfc_hostaddr_t	haddr;
		int		htype;

		ipc_gethostaddr(&haddr);
		htype = pfc_hostaddr_gettype(&haddr);
		PFC_ASSERT(htype != AF_UNSPEC);
		if (PFC_EXPECT_TRUE(htype == AF_UNIX)) {
			warning("Terminate the UNC daemon.");
			(void)uncd_stop();
		}
	}

out:
	if (parser != NULL) {
		pfc_cmdopt_destroy(parser);
	}

	return ret;
}

/*
 * static void
 * clevent_list(void)
 *	List supported cluster state change events.
 */
static void
clevent_list(void)
{
	clevdesc_t	*cdesc;

	printf("Supported cluster event types:\n");

	for (cdesc = clevent_types; cdesc < PFC_ARRAY_LIMIT(clevent_types);
	     cdesc++) {
		printf("%*s%*s %s\n",
		       COL_INDENT, str_empty,
		       -COL_EVTYPE, cdesc->ced_name, cdesc->ced_desc);
	}
}

/*
 * static int
 * clevent_convert(const char *UNC_RESTRICT name, uint8_t *UNC_RESTRICT typep)
 *	Convert name of cluster event type into event type value.
 *
 * Calling/Exit State:
 *	Upon successful completion, an event type is set to the buffer
 *	pointed by `typep', and zero is returned.
 *	Otherwise EINVAL is returned.
 */
static int
clevent_convert(const char *UNC_RESTRICT name, uint8_t *UNC_RESTRICT typep)
{
	clevdesc_t	*cdesc;
	uint32_t	type;
	int		err;

	/* Try to parse the given name as symbolic name of a event. */
	for (cdesc = clevent_types; cdesc < PFC_ARRAY_LIMIT(clevent_types);
	     cdesc++) {
		if (strcasecmp(name, cdesc->ced_name) == 0) {
			*typep = (uint8_t)(cdesc - clevent_types);

			return 0;
		}
	}

	/* Try to convert the given name as unsigned integer value. */
	err = pfc_strtou32(name, &type);
	if (PFC_EXPECT_TRUE(err == 0 &&
			    type < PFC_ARRAY_CAPACITY(clevent_types))) {
		*typep = (uint8_t)type;
	}
	else {
		err = EINVAL;
	}

	return err;
}

/*
 * static int
 * clevent_setup(clevent_ctx_t *ctx)
 *	Set up IPC client session to send CLEVENT service request.
 *
 *	This function blocks the calling thread until the IPC client session
 *	is established.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
clevent_setup(clevent_ctx_t *ctx)
{
	pfc_timespec_t	timeout = {CLEV_LNCSTAT_TIMEOUT, 0}, iv;
	pfc_ipcsess_t	*sess;
	int		err;

	/* Create an IPC client session to invoke LNCSTAT service. */
	err = ipc_getsess(&sess, LNC_IPC_SVID_LNCSTAT, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return DMCTL_EX_FATAL;
	}

	pfc_clock_msec2time(&iv, ctx->cc_interval);
	ctx->cc_sess = sess;

	for (;;) {
		pfc_timespec_t	remains;
		int		status;

		/* Invoke LNCSTAT service. */
		status = clevent_checkstate(sess);
		if (PFC_EXPECT_TRUE(status >= 0)) {
			return status;
		}

		/*
		 * Ensure that the deadline specified by `limit' does not
		 * pass.
		 */
		err = pfc_clock_isexpired(&remains, &ctx->cc_limit);
		if (PFC_EXPECT_FALSE(err != 0)) {
			PFC_ASSERT(err == ETIMEDOUT);
			error("Failed to connect to the UNC daemon.");

			return DMCTL_EX_TIMEDOUT;
		}

		/* Reset the IPC client session. */
		err = pfc_ipcclnt_sess_reset(sess, LNC_IPC_SERVICE,
					     LNC_IPC_SVID_LNCSTAT);
		if (PFC_EXPECT_FALSE(err != 0)) {
			/* This should never happen. */
			error("Failed to reset IPC client session: %s",
			      strerror(err));

			return DMCTL_EX_FATAL;
		}

		if (pfc_clock_compare(&remains, &iv) <= 0) {
			iv = remains;
		}

		nanosleep(&iv, NULL);
	}

	return DMCTL_EX_FATAL;
}

/*
 * static int
 * clevent_checkstate(pfc_ipcsess_t *sess)
 *	Check whether the launcher module is available or not.
 *
 *	`sess' must be an IPC client session associated with the UNC daemon.
 *	Session timeout must be configured by the caller.
 *
 * Calling/Exit State:
 *	DMCTL_EX_OK is returned if the launcher module is available.
 *	DMCTL_EX_UNDEFINED is returned if the launcher module is not yet
 *	available.
 *
 *	An exit status, which is greater than zero, is returned on fatal error.
 */
static int
clevent_checkstate(pfc_ipcsess_t *sess)
{
	pfc_ipcresp_t	resp;
	int		err;

	/* Invoke LNCSTAT service. */
	err = pfc_ipcclnt_sess_invoke(sess, &resp);
	if (PFC_EXPECT_FALSE(err != 0)) {
		int	status;

		if (PFC_EXPECT_FALSE(err == EPERM)) {
			/* This is an unrecoverable error. */
			error("%s", str_err_EPERM);
			status = DMCTL_EX_AUTH;
		}
		else {
			debug_printf(2, "Failed to invoke LNCSTAT service: %s",
				     strerror(err));
			status = DMCTL_EX_UNDEFINED;
		}

		return status;
	}

	debug_printf(2, "LNCSTAT returned: resp=0x%x", resp);

	if (PFC_EXPECT_FALSE(resp & LNC_IPC_LNCSTAT_FINI)) {
		error("The UNC daemon will be down.");

		return DMCTL_EX_NOTRUNNING;
	}

	debug_printf(2, "UNC daemon is running.");

	return DMCTL_EX_OK;
}

/*
 * static int
 * clevent_invoke(clevent_ctx_t *ctx)
 *	Invoke CLEVENT service to send a cluster event type to the UNC daemon.
 *
 * Calling/Exit State:
 *	Upon successful completion, DMCTL_EX_OK is returned.
 *	Otherwise exit status of the program is returned.
 */
static int
clevent_invoke(clevent_ctx_t *ctx)
{
	pfc_ipcsess_t	*sess;
	pfc_ipcresp_t	resp;
	pfc_timespec_t	timeout;
	uint32_t	sec;
	int		err, ret;

	/* Set up the context for CLEVENT service. */
	ret = clevent_setup(ctx);
	if (PFC_EXPECT_FALSE(ret != DMCTL_EX_OK)) {
		return ret;
	}

	/* Determine client session timeout. */
	err = pfc_clock_isexpired(&timeout, &ctx->cc_limit);
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT(err == ETIMEDOUT);
		error("Operation timed out.");

		return DMCTL_EX_TIMEDOUT;
	}

	/* Set client session timeout. */
	sess = ctx->cc_sess;
	debug_printf(2, "Trying to invoke CLEVENT service: timeout=%lu.%lu",
		     timeout.tv_sec, timeout.tv_nsec);
	err = pfc_ipcclnt_sess_settimeout(sess, &timeout);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to set IPC session timeout: %s",
		      strerror(err));

		return DMCTL_EX_FATAL;
	}

	/* Reset the given client session for CLEVENT service. */
	err = pfc_ipcclnt_sess_reset(sess, LNC_IPC_SERVICE,
				     LNC_IPC_SVID_CLEVENT);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to reset the client session: %s", strerror(err));

		return DMCTL_EX_FATAL;
	}

	/*
	 * Append the given event type to the additional data array in
	 * the IPC client session.
	 */
	err = pfc_ipcclnt_output_uint8(sess, ctx->cc_type);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to add event type to the client session: %s",
		      strerror(err));

		return DMCTL_EX_FATAL;
	}

	/* Append timeout in seconds. */
	sec = (uint32_t)timeout.tv_sec;
	if ((pfc_ulong_t)timeout.tv_nsec >= (PFC_CLOCK_NANOSEC >> 1)) {
		sec++;
	}
	debug_printf(1, "CLEVENT timeout=%u", sec);
	err = pfc_ipcclnt_output_uint32(sess, sec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		error("Failed to add timeout value to the client session: %s",
		      strerror(err));

		return DMCTL_EX_FATAL;
	}

	/*
	 * Invoke LNC_IPC_SVID_CLEVENT service in the UNC daemon.
	 * LNC_IPC_SVID_CLEVENT service lets the UNC daemon raise internal
	 * cluster state change events.
	 */
	ret = ipc_invoke(sess, &resp);
	if (PFC_EXPECT_TRUE(ret == DMCTL_EX_OK)) {
		debug_printf(1, "CLEVENT response=%d", resp);
		if (PFC_EXPECT_TRUE(resp == LNC_IPC_CLEVENT_OK)) {
			/* Succeeded. */
			return ret;
		}

		/* Check response code. */
		if (PFC_EXPECT_FALSE((uint32_t)resp >=
				     PFC_ARRAY_CAPACITY(clevent_extable))) {
			error("CLEVENT service returned unexpected value: %d",
			      resp);
			ret = DMCTL_EX_FATAL;
		}
		else {
			ret = clevent_extable[resp];
			error("CLEVENT service failed: %d", resp);
		}

		if (resp == LNC_IPC_CLEVENT_TC_BUSY) {
			/* Disable stop-on-error flag. */
			ctx->cc_mode &= ~CLEV_MODE_STOPONERR;
		}
	}

	return ret;
}
