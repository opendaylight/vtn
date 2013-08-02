/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * Definitions for US-ASCII printer with line breaking.
 *
 * Remarks:	Line breaking printer is not thread-safe.
 */

#ifndef	_PFC_LBRK_H
#define	_PFC_LBRK_H

#include <stdio.h>
#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Context to print US-ASCII string with line breaking.
 */
typedef struct {
	FILE		*lb_file;	/* File stream to print string */
	int32_t		lb_cursor;	/* Column index to print character */
	int32_t		lb_width;	/* maximum width of the screen */
} pfc_lbrk_t;

/*
 * Special string which is replaced with whitespace character.
 */
#define	PFC_LBRK_SPACE		((char)0xfe)
#define	PFC_LBRK_SPACE_STR	"\xfe"

/*
 * Flags for pfc_lbrk_print().
 */
#define	PFC_LBRK_NEEDSEP	0x00000001U	/* put word separator */
#define	PFC_LBRK_NEEDCOMMA	0x00000002U	/* put comma as separator */
#define	PFC_LBRK_NOWRAP		0x00000004U	/* don't wrap message */

/*
 * Prototypes.
 */
extern void	pfc_lbrk_init(pfc_lbrk_t *lbp, FILE *fp, int32_t width);
extern void	pfc_lbrk_print_raw(pfc_lbrk_t *PFC_RESTRICT lbp,
				   const char *PFC_RESTRICT message);
extern void	pfc_lbrk_print(pfc_lbrk_t *PFC_RESTRICT lbp, uint32_t indent,
			       int32_t startpos,
			       const char *PFC_RESTRICT message,
			       uint32_t flags);
extern void	pfc_lbrk_print_array(pfc_lbrk_t *PFC_RESTRICT lbp,
				     uint32_t indent, int32_t startpos,
				     const char **PFC_RESTRICT array,
				     uint32_t flags);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_lbrk_reset(pfc_lbrk_t *lbp)
 *	Reset state of line breaking.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_lbrk_reset(pfc_lbrk_t *lbp)
{
	lbp->lb_cursor = 0;
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pfc_lbrk_newline(pfc_lbrk_t *lbp)
 *	Put a newline character.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pfc_lbrk_newline(pfc_lbrk_t *lbp)
{
	pfc_lbrk_reset(lbp);
	putchar('\n');
}

/*
 * static inline int32_t PFC_FATTR_ALWAYS_INLINE
 * pfc_lbrk_getcursor(pfc_lbrk_t *lbp)
 *	Return current cursor position.
 */
static inline int32_t PFC_FATTR_ALWAYS_INLINE
pfc_lbrk_getcursor(pfc_lbrk_t *lbp)
{
	return lbp->lb_cursor;
}

PFC_C_END_DECL

#endif	/* !_PFC_LBRK_H */
