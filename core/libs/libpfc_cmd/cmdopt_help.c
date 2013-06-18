/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * cmdopt_help.c - Utilities to print command option help message.
 */

#include <string.h>
#include <pfc/debug.h>
#include <pfc/ctype.h>
#include "cmdopt_impl.h"

/*
 * Maximum column width of screen.
 */
#define	LBRK_MAX_WIDTH		78

/*
 * Column index which option description should be printed out.
 */
#define	LBRK_OPTDESC_COLUMN	28

/*
 * Number of whitespaces between option and its description.
 */
#define	LBRK_OPTDESC_MARGIN	3

/*
 * Indention level for messages.
 */
#define	LBRK_INDENT_HELP	4
#define	LBRK_INDENT_USAGE	9

/*
 * Move cursor to the specified position.
 */
#define	LBRK_MOVE_CURSOR(fp, cursor, pos)				\
	do {								\
		int32_t	__c;						\
									\
		for (__c = (cursor); __c < (int32_t)(pos); __c++) {	\
			(void)fputc(' ', fp);				\
		}							\
		(cursor) = (pos);					\
	} while (0)

/*
 * Break line, and skip cursor to the specified column.
 */
#define	LBRK_NEWLINE(fp, cursor, startpos)			\
	do {							\
		(void)fputc('\n', fp);				\
		(cursor) = 0;					\
		LBRK_MOVE_CURSOR(fp, cursor, startpos);		\
	} while (0)

/*
 * Static messages.
 */
static const char	label_usage[] = "Usage: ";
static const char	label_option[] = "Options:";
static const char	label_help_option[] = "Help Options:";
static const char	label_option_arg[] = " [OPTIONS]";

/*
 * Internal prototypes.
 */
static void	lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp,
			       const char *PFC_RESTRICT message);
static void	lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
			   int32_t startpos, const char *PFC_RESTRICT message,
			   uint32_t flags);
static void	lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
				 int32_t startpos,
				 const char **PFC_RESTRICT array,
				 uint32_t flags);

/*
 * static inline FILE *
 * lbrk_init(pfc_lbrk_t *lbp, FILE *fp, int32_t width)
 *	Initialize line break context.
 *
 * Calling/Exit State:
 *	Output file stream is returned.
 */
static inline FILE *
lbrk_init(pfc_lbrk_t *lbp, FILE *fp, int32_t width)
{
	if (fp == NULL) {
		/* Default output stream is stdout. */
		fp = stdout;
	}

	PFC_ASSERT(width > 0);

	lbp->lb_file = fp;
	lbp->lb_cursor = 0;
	lbp->lb_width = width;

	return fp;
}

/*
 * static inline void
 * pfc_cmdopt_print_help(pfc_cmdopt_t *PFC_RESTRICT ctx,
 *			 pfc_lbrk_t *PFC_RESTRICT lbp,
 *			 const pfc_cmdopt_def_t *PFC_RESTRICT opt)
 *	Print help message for the specified option.
 */
static inline void
pfc_cmdopt_print_help(pfc_cmdopt_t *PFC_RESTRICT ctx,
		      pfc_lbrk_t *PFC_RESTRICT lbp,
		      const pfc_cmdopt_def_t *PFC_RESTRICT opt)
{
	/* Print description of the option format in temporary buffer. */
	pfc_cmdopt_opt_desc(ctx, opt, 0);
	lbrk_print(lbp, 0, LBRK_INDENT_HELP, (const char *)ctx->cc_buffer,
		   PFC_LBRK_NOWRAP);

	if (opt->cd_desc != NULL) {
		/* Print description of the option specified by the user.*/
		if (lbp->lb_cursor + LBRK_OPTDESC_MARGIN >
		    LBRK_OPTDESC_COLUMN) {
			/* Insert a newline before description. */
			lbp->lb_cursor = LBRK_OPTDESC_COLUMN + 1;
		}

		if (opt->cd_flags & PFC_CMDOPT_DEF_ARRAYDESC) {
			const char	**array = (const char **)opt->cd_desc;

			lbrk_print_array(lbp, LBRK_OPTDESC_COLUMN,
					 LBRK_OPTDESC_COLUMN, array, 0);
		}
		else {
			lbrk_print(lbp, LBRK_OPTDESC_COLUMN,
				   LBRK_OPTDESC_COLUMN, opt->cd_desc, 0);
		}
	}
}

/*
 * void
 * pfc_cmdopt_help(pfc_cmdopt_t *PFC_RESTRICT ctx, FILE *PFC_RESTRICT fp,
 *		   const char *PFC_RESTRICT message)
 *	Print help message to the specified output stream.
 *	If `fp' is NULL, message will be dumped to the standard output.
 *
 *	`message' is a description of a program which will be printed in
 *	help message with detailed option help. NULL can be specified to
 *	omit program help message.
 */
void
pfc_cmdopt_help(pfc_cmdopt_t *PFC_RESTRICT ctx, FILE *PFC_RESTRICT fp,
		const char *PFC_RESTRICT message)
{
	pfc_cmdopt_help_list(ctx, fp, message, NULL);
}

/*
 * void
 * pfc_cmdopt_help_list(pfc_cmdopt_t *PFC_RESTRICT ctx, FILE *PFC_RESTRICT fp,
 *			...)
 *	Print help message to the specified output stream.
 *	If `fp' is NULL, message will be dumped to the standard output.
 *
 *	All arguments after `fp' are treated as string pointers.
 *	They are concatenated into one string, and it is printed out to the
 *	given output stream as a help message. The last argument must be NULL.
 */
void
pfc_cmdopt_help_list(pfc_cmdopt_t *PFC_RESTRICT ctx, FILE *PFC_RESTRICT fp,
		     ...)
{
	pfc_lbrk_t	lb, *lbp;
	va_list		ap;
	const char	*message;
	const pfc_cmdopt_def_t	*opt, *eopt;

	lbp = &lb;
	fp = lbrk_init(lbp, fp, LBRK_MAX_WIDTH);

	/* Print usage label and program name. */
	lbrk_print_raw(lbp, label_usage);
	lbrk_print_raw(lbp, ctx->cc_progname);
	lbrk_print_raw(lbp, label_option_arg);
	if (ctx->cc_argdesc != NULL) {
		lbrk_print(lbp, LBRK_INDENT_USAGE, -1, ctx->cc_argdesc,
			   PFC_LBRK_NEEDSEP);
	}
	(void)fputs("\n\n", fp);
	lbp->lb_cursor = 0;

	va_start(ap, fp);
	for (message = va_arg(ap, const char *); message != NULL;
	     message = va_arg(ap, const char *)) {
		/* Print given message. */
		lbrk_print(lbp, 0, -1, message, 0);
	}
	va_end(ap);

	(void)fputs("\n\n", fp);
	lbp->lb_cursor = 0;

	if (ctx->cc_nvisibles > 0) {
		/* Print description of options. */
		lbrk_print_raw(lbp, label_option);
		(void)fputc('\n', fp);
		lbp->lb_cursor = 0;
		for (opt = ctx->cc_options,
			     eopt = ctx->cc_options + ctx->cc_nopts;
		     opt < eopt; opt++) {
			if (opt->cd_flags & PFC_CMDOPT_DEF_HIDDEN) {
				continue;
			}
			pfc_cmdopt_print_help(ctx, lbp, opt);
		}
		(void)fputs("\n\n", fp);
		lbp->lb_cursor = 0;
	}

	if ((ctx->cc_flags & (PFC_CMDOPT_NOHELP | PFC_CMDOPT_NOUSAGE)) == 0) {
		/* Print description of help options. */
		lbrk_print_raw(lbp, label_help_option);
		(void)fputc('\n', fp);
		lbp->lb_cursor = 0;

		if ((opt = ctx->cc_helpopt) != NULL) {
			pfc_cmdopt_print_help(ctx, lbp, opt);
		}
		if ((opt = ctx->cc_usageopt) != NULL) {
			pfc_cmdopt_print_help(ctx, lbp, opt);
		}
	}
	(void)fputc('\n', fp);
}

/*
 * void
 * pfc_cmdopt_usage(pfc_cmdopt_t *ctx, FILE *fp)
 *	Print option usage message to the specified output stream.
 *	If `fp' is NULL, message will be dumped to the standard output.
 */
void
pfc_cmdopt_usage(pfc_cmdopt_t *ctx, FILE *fp)
{
	pfc_lbrk_t	lb, *lbp;
	const pfc_cmdopt_def_t	*opt, *eopt;

	lbp = &lb;
	fp = lbrk_init(lbp, fp, LBRK_MAX_WIDTH);

	/* Print usage label and program name. */
	lbrk_print_raw(lbp, label_usage);
	lbrk_print(lbp, LBRK_INDENT_USAGE, LBRK_INDENT_USAGE,
		   ctx->cc_progname, PFC_LBRK_NOWRAP);
	lbrk_print_raw(lbp, label_option_arg);
	if (ctx->cc_argdesc != NULL) {
		lbrk_print(lbp, LBRK_INDENT_USAGE, -1, ctx->cc_argdesc,
			   PFC_LBRK_NEEDSEP);
	}
	(void)fputc('\n', fp);
	lbp->lb_cursor = 0;

	/* Print brief usage message of all options. */
	lbrk_print_raw(lbp, label_option);
	for (opt = ctx->cc_options, eopt = ctx->cc_options + ctx->cc_nopts;
	     opt < eopt; opt++) {
		if (opt->cd_flags & PFC_CMDOPT_DEF_HIDDEN) {
			continue;
		}
		pfc_cmdopt_opt_desc(ctx, opt, PFC_CMDOPT_DESC_USAGE);
		lbrk_print(lbp, LBRK_INDENT_USAGE, -1,
			   (const char *)ctx->cc_buffer,
			   PFC_LBRK_NEEDSEP|PFC_LBRK_NOWRAP);
	}

	if (ctx->cc_usageopt != NULL) {
		/* Print usage for usage option. */
		pfc_cmdopt_opt_desc(ctx, ctx->cc_usageopt,
				    PFC_CMDOPT_DESC_USAGE);
		lbrk_print(lbp, LBRK_INDENT_USAGE, -1,
			   (const char *)ctx->cc_buffer,
			   PFC_LBRK_NEEDSEP|PFC_LBRK_NOWRAP);
	}
	if (ctx->cc_helpopt != NULL) {
		/* Print usage for help option. */
		pfc_cmdopt_opt_desc(ctx, ctx->cc_helpopt,
				    PFC_CMDOPT_DESC_USAGE);
		lbrk_print(lbp, LBRK_INDENT_USAGE, -1,
			   (const char *)ctx->cc_buffer,
			   PFC_LBRK_NEEDSEP|PFC_LBRK_NOWRAP);
	}

	(void)fputc('\n', fp);
}

/*
 * void
 * pfc_lbrk_init(pfc_lbrk_t *lbp, FILE *fp, int32_t width)
 *	Initialize line break context.
 *
 *	The standard output is used if NULL is specified to `fp'.
 *
 * Remarks:
 *	`width' must be greater than zero.
 */
void
pfc_lbrk_init(pfc_lbrk_t *lbp, FILE *fp, int32_t width)
{
	lbrk_init(lbp, fp, width);
}

/*
 * void
 * pfc_lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp,
 *		      const char *PFC_RESTRICT message)
 *	Print the message specified by `message' without line breaking.
 */
void
pfc_lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp,
		   const char *PFC_RESTRICT message)
{
	lbrk_print_raw(lbp, message);
}

/*
 * void
 * pfc_lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
 *		  int32_t startpos, const char *PFC_RESTRICT message,
 *		  uint32_t flags)
 *	Print the message specified by `message' with line breaking.
 *
 *	See comments on lbrk_print() for more details.
 */
void
pfc_lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent, int32_t startpos,
	       const char *PFC_RESTRICT message, uint32_t flags)
{
	lbrk_print(lbp, indent, startpos, message, flags);
}

/*
 * void
 * pfc_lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
 *			int32_t startpos, const char **PFC_RESTRICT array,
 *			uint32_t flags)
 *	Concatenate all strings in `array', and print it with line breaking.
 *	The string array specified by `array' must be terminated by NULL.
 *
 *	See comments on lbrk_print() for more defails about arguments.
 */
void
pfc_lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
		     int32_t startpos, const char **PFC_RESTRICT array,
		     uint32_t flags)
{
	lbrk_print_array(lbp, indent, startpos, array, flags);
}

/*
 * static void
 * lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp,
 *		  const char *PFC_RESTRICT message)
 *	Print the message specified by `message' without line breaking.
 */
static void
lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp, const char *PFC_RESTRICT message)
{
	FILE	*fp = lbp->lb_file;
	int32_t	cursor = lbp->lb_cursor;
	uint8_t	c;

	for (c = (uint8_t)*message; c != '\0';
	     message++, c = (uint8_t)*message) {
		if (c == (uint8_t)PFC_LBRK_SPACE) {
			c = ' ';
		}
		(void)fputc(c, fp);
		if (c == '\n') {
			cursor = 0;
		}
		else {
			cursor++;
		}
	}

	lbp->lb_cursor = cursor;
}

/*
 * static void
 * lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent, int32_t startpos,
 *	      const char *PFC_RESTRICT message, uint32_t flags)
 *	Print the message specified by `message' with line breaking.
 *
 *	`indent' is a number of whitespaces that should be inserted after
 *	line breaking.
 *
 *	`startpos' is a column index where the message should start.
 *	If the current column index is larger than `startpos', newline
 *	character is inserted, and the cursor is skipped to `startpos'.
 *
 *	`flags' is a bitmask that controls behavior of this function.
 *	The following flags can be specified:
 *
 *	PFC_LBRK_NEEDSEP:
 *		Indicates that a whitespace should be inserted before message.
 *
 *	PFC_LBRK_NEEDCOMMA:
 *		Indicates that a comma should be inserted after message.
 *
 *	PFC_LBRK_NOWRAP:
 *		Indicates that the given message should be printed without
 *		wrapping. If this flag is specified, lbrk_print() may adjust
 *		start column index, but it guarantees that whole message is
 *		printed in the same line.
 *
 * Remarks:
 *	Consecutive whitespace in `message' will be compressed to one
 *	whitespace. You must use PFC_LBRK_SPACE_STR if you want to print
 *	explicit whitespaces. All PFC_LBRK_SPACE_STR in `message' will be
 *	simply replaced with whitespaces.
 */
static void
lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent, int32_t startpos,
	   const char *PFC_RESTRICT message, uint32_t flags)
{
	FILE		*fp = lbp->lb_file;
	int32_t		cursor = lbp->lb_cursor, wsep, comma;
	const int32_t	maxw = lbp->lb_width;
	uint8_t		c;

	if (startpos >= 0) {
		if (startpos < cursor) {
			/* Need to break line before printing message. */
			(void)fputc('\n', fp);
			cursor = 0;
		}
		LBRK_MOVE_CURSOR(fp, cursor, startpos);
	}

	wsep = (flags & PFC_LBRK_NEEDSEP) ? 1 : 0;
	comma = (flags & PFC_LBRK_NEEDCOMMA) ? 1 : 0;

	/* Skip leading whitespaces. */
	for (c = (uint8_t)*message; pfc_isspace_u(c);
	     message++, c = (uint8_t)*message);

	if (flags & PFC_LBRK_NOWRAP) {
		size_t	len;
		int32_t		nextcur;
		const char	*endp;

		/*
		 * Print message without wrapping.
		 * We assume that no newline character is contained in
		 * the message.
		 */

		/* Eliminate trailing whitespaces. */
		len = strlen(message);
		for (endp = message + len - 1; endp >= message; endp--) {
			c = (uint8_t)*endp;
			if (!pfc_isspace_u(c)) {
				break;
			}
		}

		len = endp - message + 1;
		nextcur = cursor + len + wsep + comma;

		if (comma) {
			(void)fputc(',', fp);
			cursor++;
		}
		if (nextcur > maxw) {
			/*
			 * Need to break line.
			 * Insert indention at the beginning of next line
			 * if needed.
			 */
			LBRK_NEWLINE(fp, cursor, indent);
		}
		else if (wsep) {
			/* Need to print a whitespace as word separator. */
			(void)fputc(' ', fp);
			cursor++;
		}

		for (; message <= endp; message++) {
			uint8_t	c = (uint8_t)*message;

			if (c == (uint8_t)PFC_LBRK_SPACE) {
				c = ' ';
			}
			(void)fputc(c, fp);
			cursor++;
		}

		lbp->lb_cursor = cursor;
		return;
	}

	while ((c = (uint8_t)*message) != '\0') {
		size_t		wlen;
		uint8_t		w;
		int32_t		nextcur;
		const char	*msg = message + 1, *p;

		/* Count word length. */
		for (w = (uint8_t)*msg; w != '\0' && !pfc_isspace_u(w);
		     msg++, w = (uint8_t)*msg);
		wlen = msg - message;
		nextcur = cursor + wlen;
		if (wsep) {
			nextcur++;
		}

		if (comma) {
			(void)fputc(',', fp);
			cursor++;
		}
		if (nextcur > maxw) {
			/*
			 * Need to break line.
			 * Insert indention at the beginning of next line
			 * if needed.
			 */
			LBRK_NEWLINE(fp, cursor, indent);
		}
		else if (wsep) {
			/* Need to print a whitespace as word separator. */
			(void)fputc(' ', fp);
			cursor++;
		}

		/* Print current word. */
		for (p = message; p < msg; p++) {
			w = (uint8_t)*p;
			if (w == (uint8_t)PFC_LBRK_SPACE) {
				w = ' ';
			}
			(void)fputc(w, fp);
			cursor++;
		}

		/* Skip sequence of whitespaces. */
		wsep = 0;
		for (w = (uint8_t)*msg; pfc_isspace_u(w);
		     msg++, w = (uint8_t)*msg) {
			if (w == '\n') {
				wsep = 0;
				LBRK_NEWLINE(fp, cursor, indent);
			}
			else {
				wsep = 1;
			}
		}
		message = msg;
	}

	lbp->lb_cursor = cursor;
}

/*
 * static void
 * lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
 *		    int32_t startpos, const char **PFC_RESTRICT array,
 *		    uint32_t flags)
 *	Concatenate all strings in `array', and print it with line breaking.
 *	The string array specified by `array' must be terminated by NULL.
 *
 *	See comments on lbrk_print() for more defails about arguments.
 */
static void
lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
		 int32_t startpos, const char **PFC_RESTRICT array,
		 uint32_t flags)
{
	for (; *array != NULL; array++) {
		lbrk_print(lbp, indent, startpos, *array, flags);
		startpos = -1;
	}
}
