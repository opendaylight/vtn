/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Internal definitions for command option parser.
 */

#ifndef	_PFC_LIBPFC_CMD_CMDOPT_IMPL_H
#define	_PFC_LIBPFC_CMD_CMDOPT_IMPL_H

#include <stdint.h>
#include <sys/types.h>
#include "cmdopt.h"

/*
 * Internal limit of description message length for an option.
 */
#define	PFC_CMDOPT_DESC_MAXLEN		128U

/*
 * Parser context.
 */
struct pfc_cmdopt {
	const char		*cc_progname;	/* program name */
	const char		*cc_argdesc;	/* command arg description */
	char			**cc_argv;	/* argument vector */
	int			cc_argc;	/* number of arguments */
	int			cc_cursor;	/* current argument index */
	const pfc_cmdopt_def_t	*cc_options;	/* option definitions */
	const pfc_cmdopt_def_t	*cc_lastopt;	/* last-parsed option */
	const pfc_cmdopt_def_t	*cc_helpopt;	/* help option */
	const pfc_cmdopt_def_t	*cc_usageopt;	/* usage option */
	pfc_cmdopt_fb_t		cc_fallback;	/* fallback function */
	char			*cc_parsed;	/* preserve parsed options */
	size_t			cc_nopts;	/* number of options */
	size_t			cc_nvisibles;	/* number of visible options */
	char			*cc_bundle;	/* bundled options */
	uint32_t		cc_flags;	/* option flags */
	int			cc_errcode;	/* last error */

	/*
	 * Last-parsed option argument.
	 */
	union {
		int32_t		i32;
		uint32_t	u32;
		char		*string;
	} cc_optarg;

	/* Temporary buffer to construct message. */
	char			cc_buffer[PFC_CMDOPT_DESC_MAXLEN];
};

/*
 * Option names for auto-help feature.
 */
#define	PFC_CMDOPT_LOPT_USAGE	"usage"
#define	PFC_CMDOPT_LOPT_HELP	"help"

/*
 * Flags for cmdopt_opt_description().
 */
#define	PFC_CMDOPT_DESC_USAGE	0x00000001U	/* usage message */
#define	PFC_CMDOPT_DESC_NOARG	0x00000002U	/* don't print argument */

/*
 * Prototypes.
 */
extern void	pfc_cmdopt_opt_desc(pfc_cmdopt_t *PFC_RESTRICT ctx,
				    const pfc_cmdopt_def_t *PFC_RESTRICT opt,
				    uint32_t flags);

#endif	/* !_PFC_LIBPFC_CMD_CMDOPT_IMPL_H */
