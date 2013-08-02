/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * conf_lexer.c - The lexical analyzer for the PFC configuration file.
 */

#include <string.h>
#include <sys/stat.h>
#include <pfc/debug.h>
#include <pfc/util.h>
#include <pfc/ctype.h>
#include "conf_impl.h"

/* String length enough to represent 64-bit integer. */
#define	CONF_LEXER_INT_STRSIZE		32

/* Buffer size to create error message for string or symbol. */
#define	CONF_LEXER_STR_ERRSIZE		20

/*
 * Determine whether the given character means the start of comment line.
 */
#define	CONF_LEXER_IS_COMMENT(c)	((c) == '#')

/*
 * Push the given character into the temporary buffer, and update buffer index.
 */
#define	CONF_LEXER_BUFFER_PUSH(parser, index, c)		\
	do {							\
		PFC_ASSERT((index) < CONF_PARSER_BUFSIZE);	\
		(parser)->cfp_buffer[(index)] = (c);		\
		(index)++;					\
	} while (0)

/*
 * Push the given character into the array buffer, and update buffer index.
 */
#define	CONF_LEXER_ARRAY_PUSH(array, index, c)				\
	do {								\
		PFC_ASSERT((index) < PFC_ARRAY_CAPACITY(array));	\
		(array)[(index)] = (c);					\
		(index)++;						\
	} while (0)

/*
 * Define special token type which consists of one character.
 */
typedef struct {
	uint8_t			ct_char;	/* character */
	conf_token_type_t	ct_type;	/* token type */
} char_token_t;

static const char_token_t	char_tokens[] = {
	{ ';', TOKEN_SEMI },
	{ '{', TOKEN_LBRACE },
	{ '}', TOKEN_RBRACE },
	{ '[', TOKEN_SQLEFT },
	{ ']', TOKEN_SQRIGHT },
	{ '=', TOKEN_EQUAL },
	{ ',', TOKEN_COMMA },
};

/*
 * Supported backslash escape in quoted string.
 */
typedef struct {
	uint8_t		sb_escape;	/* character after backslash */
	uint8_t		sb_char;	/* actual character */
} str_bslash_t;

static const str_bslash_t	str_backslashes[] = {
	{ 'n', '\n' },
	{ 'r', '\r' },
	{ 't', '\t' },
	{ '\'', '\'' },
	{ '"', '"' },
	{ '\\', '\\' },
};

/*
 * Internal prototypes.
 */
static int	conf_lexer_get(conf_parser_t *parser, pfc_bool_t needed);
static void	conf_lexer_unget(conf_parser_t *parser, uint8_t c);
static int	conf_lexer_next_char(conf_parser_t *parser, pfc_bool_t needed);
static void	conf_lexer_tokenize(conf_parser_t *PFC_RESTRICT parser,
				    conf_token_t *PFC_RESTRICT token,
				    uint8_t first);
static void	conf_lexer_parse_int(conf_parser_t *PFC_RESTRICT parser,
				     conf_token_t *PFC_RESTRICT token,
				     uint8_t c);
static void	conf_lexer_parse_string(conf_parser_t *PFC_RESTRICT parser,
					uint32_t *PFC_RESTRICT bufidxp);
static void	conf_lexer_parse_symbol(conf_parser_t *parser, uint8_t first);
static void	conf_lexer_str_errmsg(conf_parser_t *PFC_RESTRICT parser,
				      uint32_t bufidx, char *PFC_RESTRICT buf,
				      size_t bufsize);

/*
 * void PFC_ATTR_HIDDEN
 * conf_lexer_open(conf_parser_t *parser)
 *	Initialize lexical analyzer for the PFC configuration file.
 *	The given parser context must be initialized in advance.
 *
 * Calling/Exit State:
 *	This function never returns on error.
 */
void PFC_ATTR_HIDDEN
conf_lexer_open(conf_parser_t *parser)
{
	FILE	*fp;
	struct stat	sbuf;

	fp = PFC_FOPEN_CLOEXEC(parser->cfp_path, "r");
	parser->cfp_file = fp;
	if (PFC_EXPECT_FALSE(fp == NULL)) {
		int	err = errno;

		if (err == ENOENT &&
		    (parser->cfp_flags & CONF_PF_NO_ENOENT)) {
			/* Ignore ENOENT error. */
			goto out;
		}

		parser->cfp_errno = err;
		conf_parser_error(parser, "%s: Failed to open: %s",
				  parser->cfp_path, strerror(err));
		/* NOTREACHED */
	}

	if (PFC_EXPECT_FALSE(fstat(fileno(fp), &sbuf) != 0)) {
		int	err = errno;

		parser->cfp_errno = err;
		conf_parser_error(parser, "%s: fstat() failed: %s",
				  parser->cfp_path, strerror(err));
		/* NOTREACHED */
	}

	/* Reject unsafe file. */
	if (PFC_EXPECT_FALSE(sbuf.st_mode & (S_IWGRP | S_IWOTH))) {
		const char	*label = (sbuf.st_mode & S_IWOTH)
			? "world" : "group";

		parser->cfp_errno = EPERM;
		conf_parser_error(parser, "%s: Reject %s writable file.",
				  parser->cfp_path, label);
		/* NOTREACHED */
	}

	parser->cfp_value->cf_flags |= CONF_FF_PRIMARY;

out:
	parser->cfp_chstack_depth = 0;
	parser->cfp_line = 1;
}

/*
 * void PFC_ATTR_HIDDEN
 * conf_lexer_close(conf_parser_t *parser)
 *	Close the lexer context.
 */
void PFC_ATTR_HIDDEN
conf_lexer_close(conf_parser_t *parser)
{
	FILE	*fp = parser->cfp_file;

	if (PFC_EXPECT_TRUE(fp != NULL)) {
		(void)fclose(fp);
	}
}

/*
 * pfc_bool_t PFC_ATTR_HIDDEN
 * conf_lexer_next(conf_parser_t *PFC_RESTRICT parser,
 *		   conf_token_t *PFC_RESTRICT token, pfc_bool_t needed)
 *	Get next token of the configuration file.
 *	If `needed' is true, this function causes exception if EOF is detected.
 *
 * Calling/Exit State:
 *	Upon successful completion, this function sets the token into the
 *	specified token buffer, and PFC_TRUE is returned.
 *	PFC_FALSE is returned if the end of file is detected.
 *
 *	This function never returns on error.
 */
pfc_bool_t PFC_ATTR_HIDDEN
conf_lexer_next(conf_parser_t *PFC_RESTRICT parser,
		conf_token_t *PFC_RESTRICT token, pfc_bool_t needed)
{
	int	comment = 0, ic;

	while ((ic = conf_lexer_get(parser, needed)) != EOF) {
		/* Skip newline. */
		if (ic == '\n') {
			comment = 0;
			continue;
		}
		if (comment != 0) {
			continue;
		}

		/* Skip whitespace. */
		if (pfc_isspace_u(ic)) {
			continue;
		}

		/* Skip comment. */
		if (CONF_LEXER_IS_COMMENT(ic)) {
			comment = 1;
		}
		else {
			break;
		}
	}

	if (ic == EOF) {
		/* End of file. */
		return PFC_FALSE;
	}

	/* Parse this token. */
	conf_lexer_tokenize(parser, token, (uint8_t)ic);

	return PFC_TRUE;
}

/*
 * static int
 * conf_lexer_get(conf_parser_t *parser, pfc_bool_t needed)
 *	Read a character from the configuration file.
 *	If `needed' is true, this function causes exception if EOF is detected.
 *
 * Calling/Exit State:
 *	Upon successful completion, the specified parser context is updated,
 *	and read character is returned.
 *	EOF is returned if end of file is detected.
 *
 *	On error, this function never returns.
 */
static int
conf_lexer_get(conf_parser_t *parser, pfc_bool_t needed)
{
	int	ic;
	uint8_t	idx = parser->cfp_chstack_depth;

	if (idx != 0) {
		/* Pop a character from the character stack. */
		PFC_ASSERT(idx <= CONF_CHAR_STACK_DEPTH);
		idx--;
		ic = parser->cfp_chstack[idx];
		parser->cfp_chstack_depth = idx;
	}
	else {
		FILE	*fp = parser->cfp_file;

		if (fp == NULL) {
			/*
			 * This can happen if ENOENT error is ignored by
			 * conf_lexer_open().
			 */
			return EOF;
		}

		ic = getc(fp);
		if (ic == EOF) {
			if (ferror(fp)) {
				/* I/O error. */
				parser->cfp_errno = EIO;
				conf_parser_file_error(parser, "Read error.");
				/* NOTREACHED */
			}

			/* End of file. */
			if (needed) {
				conf_parser_file_error
					(parser, "Unexpected EOF.");
				/* NOTREACHED */
			}

			return ic;
		}
	}

	if (ic == '\n') {
		/* Increment line number. */
		parser->cfp_line++;
		if (PFC_EXPECT_FALSE(parser->cfp_line == 0)) {
			parser->cfp_errno = E2BIG;
			conf_parser_error(parser, "%s: Too large file.",
					  parser->cfp_path);
			/* NOTREACHED */
		}
	}

	return ic;
}

/*
 * static void
 * conf_lexer_unget(conf_parser_t *parser, uint8_t c)
 *	Push back a character to the input stream.
 *	The specified character will be returned by a subsequent call to
 *	conf_lexer_get().
 */
static void
conf_lexer_unget(conf_parser_t *parser, uint8_t c)
{
	uint8_t	idx = parser->cfp_chstack_depth;

	PFC_ASSERT(idx < CONF_CHAR_STACK_DEPTH);
	if (c == '\n') {
		/* Restore line number. */
		PFC_ASSERT(parser->cfp_line != 0);
		parser->cfp_line--;
	}

	parser->cfp_chstack[idx] = c;
	parser->cfp_chstack_depth = idx + 1;
}

/*
 * static int
 * conf_lexer_next_char(conf_parser_t *parser, pfc_bool_t needed)
 *	Read a character which is valid as the first character of the token.
 *
 * Calling/Exit State:
 *	Upon successful completion, the specified parser context is updated,
 *	and read character is returned.
 *	EOF is returned if end of file is detected.
 *
 *	On error, this function never returns.
 */
static int
conf_lexer_next_char(conf_parser_t *parser, pfc_bool_t needed)
{
	int	comment = 0, ic;

	while ((ic = conf_lexer_get(parser, needed)) != EOF) {
		/* Skip newline. */
		if (ic == '\n') {
			comment = 0;
			continue;
		}
		if (comment != 0) {
			continue;
		}

		/* Skip whitespace. */
		if (pfc_isspace_u(ic)) {
			continue;
		}

		/* Skip comment. */
		if (CONF_LEXER_IS_COMMENT(ic)) {
			comment = 1;
		}
		else {
			break;
		}
	}

	return ic;
}

/*
 * static void
 * conf_lexer_tokenize(conf_parser_t *PFC_RESTRICT parser,
 *		       conf_token_t *PFC_RESTRICT token, uint8_t first)
 *	Parse a token which starts with the given character.
 */
static void
conf_lexer_tokenize(conf_parser_t *PFC_RESTRICT parser,
		    conf_token_t *PFC_RESTRICT token, uint8_t first)
{
	const char_token_t	*ctkp, *ectkp;
	const char	*string;
	pfc_refptr_t	*rstr;

	/* Detect token which consists of one character. */
	ectkp = char_tokens + PFC_ARRAY_CAPACITY(char_tokens);
	for (ctkp = char_tokens; ctkp < ectkp; ctkp++) {
		if (first == ctkp->ct_char) {
			token->cft_type = ctkp->ct_type;
			token->cft_value.character = first;

			return;
		}
	}

	if (first == '-' || pfc_isdigit_u(first)) {
		/* Integer token. */
		token->cft_type = TOKEN_INT;
		conf_lexer_parse_int(parser, token, first);

		return;
	}

	if (first == '"') {
		uint32_t	bufidx = 0;

		/* String token. */
		token->cft_type = TOKEN_STRING;
		conf_lexer_parse_string(parser, &bufidx);

		/* Try to concatenate subsequent string token. */
		while (1) {
			int	ic;
			uint8_t	c;

			ic = conf_lexer_next_char(parser, PFC_FALSE);
			if (ic == EOF) {
				break;
			}

			c = (uint8_t)ic;
			if (c != '"') {
				conf_lexer_unget(parser, c);
				break;
			}

			conf_lexer_parse_string(parser, &bufidx);
		}

		parser->cfp_buffer[bufidx] = '\0';
		string = (const char *)parser->cfp_buffer;
		rstr = pfc_refptr_string_create(string);
		if (PFC_EXPECT_FALSE(rstr == NULL)) {
			parser->cfp_errno = ENOMEM;
			conf_parser_file_error(parser,
					       "No memory for a string token.");
			/* NOTREACHED */
		}

		token->cft_value.string = rstr;

		return;
	}

	/* This token must be a symbol. */
	conf_lexer_parse_symbol(parser, first);
	string = (const char *)parser->cfp_buffer;
	rstr = pfc_refptr_string_create(string);
	if (PFC_EXPECT_FALSE(rstr == NULL)) {
		parser->cfp_errno = ENOMEM;
		conf_parser_file_error(parser,
				       "No memory for a symbol token.");
		/* NOTREACHED */
	}

	token->cft_type = TOKEN_SYMBOL;
	token->cft_value.string = rstr;
}

/*
 * static void
 * conf_lexer_parse_int(conf_parser_t *PFC_RESTRICT parser,
 *			conf_token_t *PFC_RESTRICT token, uint8_t c)
 *	Parse integer token which starts with the specified character.
 */
static void
conf_lexer_parse_int(conf_parser_t *PFC_RESTRICT parser,
		     conf_token_t *PFC_RESTRICT token, uint8_t c)
{
	uint8_t		buf[CONF_LEXER_INT_STRSIZE];
	uint32_t	base, bufidx = 0;
	uint64_t	value, max;
	pfc_bool_t	negative;
	int		ic;

	if (c == '-') {
		/* Negative integer. */
		negative = PFC_TRUE;
		c = (uint8_t)conf_lexer_get(parser, PFC_TRUE);
	}
	else {
		negative = PFC_FALSE;
	}
	token->cft_negative = negative;

	if (c == '0') {
		CONF_LEXER_ARRAY_PUSH(buf, bufidx, c);
		ic = conf_lexer_get(parser, PFC_FALSE);
		if (PFC_EXPECT_FALSE(ic == EOF)) {
			token->cft_negative = PFC_FALSE;
			token->cft_value.integer = 0;

			return;
		}

		c = (uint8_t)ic;
		if (c == 'x' || c == 'X') {
			/* Hexadecimal number. */
			base = 16;
			CONF_LEXER_ARRAY_PUSH(buf, bufidx, c);
			c = (uint8_t)conf_lexer_get(parser, PFC_TRUE);
		}
		else {
			/* Octal number. */
			base = 8;
		}
	}
	else {
		/* Decimal number. */
		base = 10;
	}

	value = 0;
	max = UINT64_MAX / base;
	do {
		uint64_t	nextval;
		uint32_t	v;

		if (PFC_EXPECT_FALSE(bufidx >= PFC_ARRAY_CAPACITY(buf))) {
			buf[CONF_LEXER_INT_STRSIZE - 1] = '\0';
			conf_parser_file_error(parser, "Too long integer: %s",
					       buf);
			/* NOTREACHED */
		}

		CONF_LEXER_ARRAY_PUSH(buf, bufidx, c);
		if (pfc_isdigit_u(c)) {
			v = c - '0';
		}
		else if (c >= 'a' && c <= 'f') {
			v = c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F') {
			v = c - 'A' + 10;
		}
		else {
			/* End of integer token. */
			conf_lexer_unget(parser, c);
			bufidx--;
			break;
		}

		if (PFC_EXPECT_FALSE(v >= base)) {
			conf_parser_file_error
				(parser, "Invalid character in integer: %c", c);
			/* NOTREACHED */
		}

		nextval = value * base + v;
		if (PFC_EXPECT_FALSE(value > max || nextval < value)) {
			if (bufidx >= PFC_ARRAY_CAPACITY(buf)) {
				bufidx = PFC_ARRAY_CAPACITY(buf) - 1;
			}
			buf[bufidx] = '\0';
			parser->cfp_errno = ERANGE;
			conf_parser_file_error
				(parser, "Integer overflow near \"%s\".",
				 buf);
			/* NOTREACHED */
		}
		value = nextval;

		ic = conf_lexer_get(parser, PFC_FALSE);
		c = (uint8_t)ic;
	} while (ic != EOF);

	if (PFC_EXPECT_FALSE(base == 16 && bufidx == 2)) {
		buf[bufidx] = '\0';
		conf_parser_file_error(parser, "Invalid integer token: %s",
				       buf);
		/* NOTREACHED */
	}

	if (negative && value != (uint64_t)CONF_INT64_SIGN) {
		int64_t	v;

		if (CONF_INT64_IS_NEGATIVE(value)) {
			parser->cfp_errno = ERANGE;
			conf_parser_file_error(parser, "Integer underflow: %s",
					       buf);
			/* NOTREACHED */
		}

		v = (int64_t)value * -1;
		value = (uint64_t)v;
	}

	token->cft_value.integer = value;
}

/*
 * static void
 * conf_lexer_parse_string(conf_parser_t *PFC_RESTRICT parser,
 *			   uint32_t *PFC_RESTRICT bufidxp)
 *	Parse string token.
 *
 *	This function copies a string into the temporary buffer,
 *	parser->cfp_buffer. `*bufidxp' must contain start index of the buffer.
 *	`*bufidxp' is updated to the end boundary of string.
 *
 * Remarks:
 *	This function never terminate the string in the buffer.
 *	The caller must do it.
 */
static void
conf_lexer_parse_string(conf_parser_t *PFC_RESTRICT parser,
			uint32_t *PFC_RESTRICT bufidxp)
{
	const str_bslash_t	*endbp;
	uint32_t	bufidx = *bufidxp;
	char	errbuf[CONF_LEXER_STR_ERRSIZE];

	endbp = str_backslashes + PFC_ARRAY_CAPACITY(str_backslashes);

	while (1) {
		int	ic;
		uint8_t	c;

		if (PFC_EXPECT_FALSE(bufidx >= CONF_PARSER_BUFSIZE)) {
			conf_lexer_str_errmsg(parser, CONF_PARSER_BUFSIZE,
					      errbuf, sizeof(errbuf));
			conf_parser_file_error(parser, "Too long string: %s",
					       errbuf);
			/* NOTREACHED */
		}

		ic = conf_lexer_get(parser, PFC_FALSE);
		if (PFC_EXPECT_FALSE(ic == EOF)) {
			goto unterminated;
		}

		c = (uint8_t)ic;
		if (c == '"') {
			break;
		}
		if (c == '\\') {
			const str_bslash_t	*bp;

			/* Escaped by backslash. */
			ic = conf_lexer_get(parser, PFC_FALSE);
			if (PFC_EXPECT_FALSE(ic == EOF)) {
				goto unterminated;
			}
			c = (uint8_t)ic;

			for (bp = str_backslashes;
			     bp < endbp && bp->sb_escape != c; bp++);

			if (PFC_EXPECT_FALSE(bp >= endbp)) {
				conf_parser_file_error
					(parser,
					 "Unsupported backslash escape: "
					 "\"\\%c\"", c);
				/* NOTREACHED */
			}
			CONF_LEXER_BUFFER_PUSH(parser, bufidx, bp->sb_char);
		}
		else {
			CONF_LEXER_BUFFER_PUSH(parser, bufidx, c);
		}
	}

	*bufidxp = bufidx;

	return;

unterminated:
	conf_parser_file_error(parser, "Unterminated string.");
	/* NOTREACHED */
}

/*
 * static void
 * conf_lexer_parse_symbol(conf_parser_t *parser, uint8_t first)
 *	Parse symbol token.
 *
 *	This function copies a string into the temporary buffer.
 *	Unlike conf_lexer_parse_string(), this function always copies a string
 *	to the top of temporary buffer.
 */
static void
conf_lexer_parse_symbol(conf_parser_t *parser, uint8_t first)
{
	uint32_t	bufidx = 0;
	char	errbuf[CONF_LEXER_STR_ERRSIZE];
	int	ic;

	/* Symbol must start with an alphabet, or underscore. */
	if (!pfc_isalpha_u(first) && first != '_') {
		conf_parser_file_error(parser, "Invalid character: %c", first);
		/* NOTREACHED */
	}

	CONF_LEXER_BUFFER_PUSH(parser, bufidx, first);
	while ((ic = conf_lexer_get(parser, PFC_FALSE)) != EOF) {
		uint8_t	c = (uint8_t)ic;

		/*
		 * Symbol must consist of alphabets, digits, and
		 * underscores.
		 */
		if (!pfc_isalnum_u(c) && c != '_') {
			conf_lexer_unget(parser, c);
			break;
		}

		if (PFC_EXPECT_FALSE(bufidx >= PFC_CF_MAX_SYMLEN)) {
			conf_lexer_str_errmsg(parser, PFC_CF_MAX_SYMLEN,
					      errbuf, sizeof(errbuf));
			conf_parser_file_error(parser, "Too long symbol: %s",
					       errbuf);
			/* NOTREACHED */
		}
		CONF_LEXER_BUFFER_PUSH(parser, bufidx, c);
	}

	/* Terminate symbol. */
	parser->cfp_buffer[bufidx] = '\0';
}

/*
 * static void
 * conf_lexer_str_errmsg(conf_parser_t *PFC_RESTRICT parser, uint32_t bufidx,
 *			 char *PFC_RESTRICT buf, size_t bufsize)
 *	Construct a string to be shown in error message.
 *
 *	This function copies a string in the temporary buffer into the
 *	specified buffer. Its length must be specified by `bufidx'.
 */
static void
conf_lexer_str_errmsg(conf_parser_t *PFC_RESTRICT parser, uint32_t bufidx,
		      char *PFC_RESTRICT buf, size_t bufsize)
{
	const str_bslash_t	*endbp;
	char	*src, *srcend, *dst, *limit;

	src = (char *)parser->cfp_buffer;
	srcend = src + bufidx;
	dst = buf;
	limit = dst + bufsize;
	endbp = str_backslashes + PFC_ARRAY_CAPACITY(str_backslashes);

	for (; src < srcend; src++) {
		const str_bslash_t	*bp;
		char	c = *src;

		for (bp = str_backslashes; bp < endbp && bp->sb_char != c;
		     bp++);
		if (bp < endbp) {
			*dst = '\\';
			dst++;
			if (dst >= limit) {
				break;
			}
			*dst = bp->sb_escape;
			dst++;
			if (dst >= limit) {
				break;
			}
		}
		else {
			*dst = c;
			dst++;
			if (dst >= limit) {
				break;
			}
		}
	}

	if (dst < limit) {
		*dst = '\0';
	}
	else {
		dst = buf + bufsize - 4;
		*dst = '.';
		*(dst + 1) = '.';
		*(dst + 2) = '.';
		*(dst + 3) = '\0';
	}
}
