/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * plaintext.c - Utilities to format plaintext.
 */

#include <pfc/plaintext.h>
#include <pfc/debug.h>

/*
 * Internal prototypes.
 */
static void	pt_print_column_header(FILE *PFC_RESTRICT fp,
				       const pfc_ptcol_t *PFC_RESTRICT cp);
static void	pt_print_header_separator(FILE *PFC_RESTRICT fp,
					  const pfc_ptcol_t *PFC_RESTRICT cp);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pt_print_column_insets(FILE *PFC_RESTRICT fp,
 *			  const pfc_ptcol_t *PFC_RESTRICT cp)
 * 	Print insets for the given table column.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pt_print_column_insets(FILE *PFC_RESTRICT fp,
		       const pfc_ptcol_t *PFC_RESTRICT cp)
{
	uint32_t	i;

	for (i = 0; i < cp->ptc_insets; i++) {
		putc(' ', fp);
	}
}

/*
 * void
 * pfc_ptcol_print_header(FILE *PFC_RESTRICT fp,
 *			  const pfc_ptcol_t *PFC_RESTRICT cp, uint32_t ncolumns)
 *	Print header for plaintext table.
 *
 *	Table column is defined by pfc_ptcol_t array specified by `cp'.
 *	`ncolumns' must be the number of pfc_ptcol_t array elements.
 *	See comments in pfc/plaintext.h.
 */
void
pfc_ptcol_print_header(FILE *PFC_RESTRICT fp,
		       const pfc_ptcol_t *PFC_RESTRICT cp, uint32_t ncolumns)
{
	const pfc_ptcol_t	*col;

	PFC_ASSERT(ncolumns > 0);

	/* Print column names. */
	for (col = cp; col < cp + ncolumns; col++) {
		pt_print_column_header(fp, col);
	}
	putc('\n', fp);

	/* Print header separator. */
	for (col = cp; col < cp + ncolumns; col++) {
		pt_print_header_separator(fp, col);
	}
	putc('\n', fp);
}

/*
 * static void
 * pt_print_column_header(FILE *PFC_RESTRICT fp,
 *		          const pfc_ptcol_t *PFC_RESTRICT cp)
 *	Print name of the given table column.
 */
static void
pt_print_column_header(FILE *PFC_RESTRICT fp,
		       const pfc_ptcol_t *PFC_RESTRICT cp)
{
	const char	*name = cp->ptc_name;

	if (name == NULL) {
		name = "";
	}

	pt_print_column_insets(fp, cp);
	fprintf(fp, "%*s", cp->ptc_width, name);
}

/*
 * static void
 * pt_print_header_separator(FILE *PFC_RESTRICT fp,
 *			     const pfc_ptcol_t *PFC_RESTRICT cp)
 *	Print plaintext table header separator for the given column.
 */
static void
pt_print_header_separator(FILE *PFC_RESTRICT fp,
			  const pfc_ptcol_t *PFC_RESTRICT cp)
{
	int	i, width = cp->ptc_width;

	pt_print_column_insets(fp, cp);

	if (width < 0) {
		width *= -1;
	}

	for (i = 0; i < width; i++) {
		putc('-', fp);
	}
}
