/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_PLAINTEXT_H
#define	_PFC_PLAINTEXT_H

/*
 * Common definition for utilities to format plaintext.
 */

#include <stdio.h>
#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Column definition of plaintext table.
 */
typedef struct {
	/*
	 * Name of column.
	 * NULL means that this column has no name.
	 */
	const char	*ptc_name;

	/*
	 * Width of column.
	 * Sign represents the alignment of column name.
	 * If positive value is specified, the column name is aligned to right.
	 * If negative, the column name is aligned to left.
	 */
	int		ptc_width;

	/*
	 * The number of whitespace to be inserted before this column.
	 */
	uint32_t	ptc_insets;
} pfc_ptcol_t;

/*
 * Static initializer for pfc_ptcol_t.
 */
#define	PFC_PTCOL_INSETS_DECL(name, width, insets)	\
	{						\
		name,					\
		width,				 	\
		insets,		  			\
	}

#define	PFC_PTCOL_DECL(name, width)	PFC_PTCOL_INSETS_DECL(name, width, 1)

/*
 * Prototypes.
 */
extern void	pfc_ptcol_print_header(FILE *PFC_RESTRICT fp,
				       const pfc_ptcol_t *PFC_RESTRICT cp,
				       uint32_t ncolumns);

PFC_C_END_DECL

#endif	/* !_PFC_PLAINTEXT_H */
