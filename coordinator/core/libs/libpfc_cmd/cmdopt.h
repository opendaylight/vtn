/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Common definitions for command option parser.
 */

#ifndef	_PFC_CMDOPT_H
#define	_PFC_CMDOPT_H

#include <stdio.h>
#include <pfc/base.h>
#include "lbrk.h"

PFC_C_BEGIN_DECL

/*
 * Type of option argument.
 */
typedef enum {
	PFC_CMDOPT_TYPE_NONE,		/* no argument */
	PFC_CMDOPT_TYPE_INT32,		/* signed 32-bit integer argument */
	PFC_CMDOPT_TYPE_UINT32,		/* unsigned 32-bit integer argument */
	PFC_CMDOPT_TYPE_STRING,		/* string argument */
} pfc_cmdopt_type_t;

/*
 * Option identifier that means option table sentinel.
 */
#define	PFC_CMDOPT_EOF		'\0'

/*
 * Option identifier that means help message request.
 */
#define	PFC_CMDOPT_HELP		'?'

/*
 * Option identifier that means usage message request.
 */
#define	PFC_CMDOPT_USAGE	' '

/*
 * Option identifier that means error occurred in pfc_cmdopt_next().
 */
#define	PFC_CMDOPT_ERROR	((char)0xff)

/*
 * Special character which is replaced with whitespace.
 */
#define	PFC_CMDOPT_SPACE	PFC_LBRK_SPACE
#define	PFC_CMDOPT_SPACE_STR	PFC_LBRK_SPACE_STR

/*
 * Struct which defines command option format.
 * pfc_cmdopt_def_t array is used to define option format, and each element
 * represents one option format.
 *
 * cd_ident:
 *	Each pfc_cmdopt_def_t element must have unique character as identifier.
 *	This value will be a return value of pfc_cmdopt_next().
 *	If this value is PFC_CMDOPT_EOF, it is treated as array sentinel.
 *	In other words, all elements after an element which has PFC_CMDOPT_EOF
 *	as identifier are ignored.
 *
 *	Identifier character is also used as short option name.
 *	If identifier character is 'a', its pfc_cmdopt_def_t means "-a" can
 *	be taken as command option, unless PFC_CMDOPT_DEF_LONGONLY is set in
 *	cd_flags.
 *
 *	'?' is considered as request to display help message.
 *	So it must not be set to cd_ident unless PFC_CMDOPT_NOHELP flag is
 *	specified to pfc_cmdopt_init().
 *
 *	' ' is considered as request to display usage message.
 *	So it must not be set to cd_ident unless PFC_CMDOPT_NOUSAGE flag is
 *	specified to pfc_cmdopt_init().
 *
 * cd_name:
 *	Long name option is enabled if this value is not NULL.
 *	If "name" is set to cd_name, "--name" can be taken as command option.
 *
 *	"help" is considered as request to display help message.
 *	So it must not be set to cd_name unless PFC_CMDOPT_NOHELP flag is
 *	specified to pfc_cmdopt_init().
 *
 *	"usage" is considered as request to display usage message.
 *	So it must not be set to cd_name unless PFC_CMDOPT_NOUSAGE flag is
 *	specified to pfc_cmdopt_init().
 *
 * cd_type:
 *	Specify option type.
 *	One of the followings must be specified:
 *
 *	PFC_CMDOPT_TYPE_NONE
 *		This option takes no argument.
 *
 *	PFC_CMDOPT_TYPE_INT32
 *		This option takes a signed 32-bit integer argument.
 *
 *	PFC_CMDOPT_TYPE_UINT32
 *		This option takes an unsigned 32-bit integer argument.
 *
 *	PFC_CMDOPT_TYPE_STRING
 *		This option takes a string argument.
 *
 * cd_flags:
 *	Specify option flags.
 *	Supported flags are the followings:
 *
 *	PFC_CMDOPT_DEF_LONGONLY
 *		Enable only long name option.
 *		Specifying this flag and NULL long name option is undefined
 *		behavior.
 *
 *	PFC_CMDOPT_DEF_HIDDEN
 *		Define this option as hidden option.
 *		This option is never mentioned in help message and usage.
 *
 *	PFC_CMDOPT_DEF_MANDATORY
 *		Indicate that this option is mandatory.
 *		If this option is not specified, pfc_cmdopt_validate()
 *		returns PFC_CMDOPT_E_MANDATORY.
 *
 *	PFC_CMDOPT_DEF_ONCE
 *		Indicate that this option can be specified only once.
 *		Unless this flag, pfc_cmdopt_next() will accept this option
 *		multiple times.
 *
 *	PFC_CMDOPT_DEF_ARRAYDESC
 *		Indicate that a pointer to string array is specified to
 *		cd_desc instead of a string. A string array must be terminated
 *		by NULL.
 *
 * cd_desc:
 *	Brief description of this option.
 *	This is used to generate help message.
 *	NULL can be set to cd_desc.
 *
 * cd_argdesc:
 *	Brief description of argument for this option.
 *	This is used to generate help message.
 *	NULL can be set to cd_argdesc.
 *
 * Remarks:
 * - Description string set to cd_desc or cd_argdesc has the following
 *   restrictions:
 *   + Only US-ASCII characters are allowed.
 *   + Any control sequence is disallowed, but whitespace (' ').
 *   + A tab character ('\t') is treated as a whitespace character.
 *   + All leading and trailing whitespaces and newlines in each line are
 *     ignored.
 *   + Sequence of whitespaces is treated as one whitespace character.
 */
typedef struct {
	char			cd_ident;	/* option identifier */
	const char		*cd_name;	/* long option name */
	pfc_cmdopt_type_t	cd_type;	/* type of this option */
	uint32_t		cd_flags;	/* option flags */
	const char		*cd_desc;	/* description for help */
	const char		*cd_argdesc;	/* arg description for help */
} pfc_cmdopt_def_t;

/*
 * Flags for cd_flags.
 */
#define	PFC_CMDOPT_DEF_LONGONLY		0x00000001U	/* disable short opt. */
#define	PFC_CMDOPT_DEF_HIDDEN		0x00000002U	/* hidden */
#define	PFC_CMDOPT_DEF_MANDATORY	0x00000004U	/* mandatory */
#define	PFC_CMDOPT_DEF_ONCE		0x00000008U	/* only once */
#define	PFC_CMDOPT_DEF_ARRAYDESC	0x00000010U	/* cd_desc is array */

/*
 * Parser context.
 */
struct pfc_cmdopt;
typedef struct pfc_cmdopt	pfc_cmdopt_t;

/*
 * Flags for pfc_cmdopt_init().
 */
#define	PFC_CMDOPT_NOHELP	0x00000001	/* disable "--help" option. */
#define	PFC_CMDOPT_NOUSAGE	0x00000002	/* disable "--usage" option. */
#define	PFC_CMDOPT_NOERRMSG	0x00000004	/* disable error message */

/*
 * Parser error code.
 */
#define	PFC_CMDOPT_E_NONE		0	/* No error */
#define	PFC_CMDOPT_E_BADOPT		1	/* Unknown option */
#define	PFC_CMDOPT_E_NOARG		2	/* Argument is required */
#define	PFC_CMDOPT_E_HASARG		3	/* No argument is required */
#define	PFC_CMDOPT_E_BADNUMBER		4	/* Bad integer */
#define	PFC_CMDOPT_E_BADRANGE		5	/* Bad integer range */
#define	PFC_CMDOPT_E_MANDATORY		6	/* Missing mandatory option */
#define	PFC_CMDOPT_E_DUPLICATE		7	/* option is duplicated */

/*
 * Prototype of fallback function.
 *
 * If the parser finds an option which is not defined in pfc_cmdopt_def_t
 * array, it calls the fallback function with specifying the option.
 * The option string is passed to `name', and its length is passed to
 * `namelen'. If the option is a long option, PFC_TRUE is passed to `longopt'.
 * PFC_FALSE is passed to `longopt' if it is a short option.
 * If the fallback function returns a non-NULL pointer, the parser uses it
 * as a pointer to pfc_cmdopt_def_t which defines behavior of the option.
 *
 * Note that `name' may not be terminated by '\0'. So the fallback function
 * must not pass `name' to strcmp().
 */
typedef const pfc_cmdopt_def_t	*(*pfc_cmdopt_fb_t)
	(const char *name, size_t namelen, pfc_bool_t longopt);

/*
 * Prototypes
 */
extern pfc_cmdopt_t	*pfc_cmdopt_init(const char *PFC_RESTRICT name,
					 int argc, char **PFC_RESTRICT argv,
					 const pfc_cmdopt_def_t *PFC_RESTRICT
					 options,
					 const char *PFC_RESTRICT argdesc,
					 uint32_t flags);
extern void		pfc_cmdopt_destroy(pfc_cmdopt_t *ctx);
extern void		pfc_cmdopt_setfallback(pfc_cmdopt_t *PFC_RESTRICT ctx,
					       pfc_cmdopt_fb_t fallback);
extern char		pfc_cmdopt_next(pfc_cmdopt_t *ctx);
extern int		pfc_cmdopt_errcode(pfc_cmdopt_t *ctx);
extern int32_t		pfc_cmdopt_arg_int32(pfc_cmdopt_t *ctx);
extern uint32_t		pfc_cmdopt_arg_uint32(pfc_cmdopt_t *ctx);
extern const char	*pfc_cmdopt_arg_string(pfc_cmdopt_t *ctx);
extern int		pfc_cmdopt_validate(pfc_cmdopt_t *ctx);
extern void		pfc_cmdopt_help(pfc_cmdopt_t *PFC_RESTRICT ctx,
					FILE *PFC_RESTRICT fp,
					const char *PFC_RESTRICT message);
extern void		pfc_cmdopt_help_list(pfc_cmdopt_t *PFC_RESTRICT ctx,
					     FILE *PFC_RESTRICT fp, ...);
extern void		pfc_cmdopt_usage(pfc_cmdopt_t *PFC_RESTRICT ctx,
					 FILE *PFC_RESTRICT fp);

PFC_C_END_DECL

#endif	/* !_PFC_CMDOPT_H */
