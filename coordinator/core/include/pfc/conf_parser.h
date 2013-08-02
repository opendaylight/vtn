/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_CONF_PARSER_H
#define	_PFC_CONF_PARSER_H

/*
 * Definitions for PFC configuration file parser.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Type of parameter.
 */
typedef enum {
	PFC_CFTYPE_BYTE = 0,			/* byte */
	PFC_CFTYPE_STRING,			/* string */
	PFC_CFTYPE_BOOL,			/* boolean */
	PFC_CFTYPE_INT32,			/* signed 32-bit integer */
	PFC_CFTYPE_UINT32,			/* unsigned 32-bit integer */
	PFC_CFTYPE_INT64,			/* signed 64-bit integer */
	PFC_CFTYPE_UINT64,			/* unsigned 64-bit integer */
	PFC_CFTYPE_LONG,			/* signed long integer */
	PFC_CFTYPE_ULONG,			/* unsigned long integer */
} pfc_cftype_t;

/*
 * Determine whether the given type represents 64-bit integer or not.
 */
#ifdef	PFC_LP64
#define	PFC_CFTYPE_IS_64BIT_INT(type)					\
	((type) >= PFC_CFTYPE_INT64 && (type) <= PFC_CFTYPE_ULONG)
#else	/* !PFC_LP64 */
#define	PFC_CFTYPE_IS_64BIT_INT(type)					\
	((type) >= PFC_CFTYPE_INT64 && (type) <= PFC_CFTYPE_UINT64)
#endif	/* PFC_LP64 */

/*
 * Maximum length of symbol in the configuration file.
 * Note that the name of block, map, and parameter name are parsed as symbol.
 */
#define	PFC_CF_MAX_SYMLEN		PFC_CONST_U(63)

/*
 * Maximum length of string parameter length.
 */
#define	PFC_CF_MAX_STRLEN		PFC_CONST_U(1023)

/*
 * Maximum number of array elements.
 */
#define	PFC_CF_MAX_ARRAY_SIZE		PFC_CONST_U(256)

/*
 * Common definition of parameter.
 */
typedef struct {
	const char		*cfdp_name;	/* parameter name */
	uint64_t		cfdp_min;	/* minimum value */
	uint64_t		cfdp_max;	/* maximum value */
	pfc_cftype_t		cfdp_type;	/* type of parameter */
	int32_t			cfdp_nelems;	/* number of array elements */
	uint32_t		cfdp_flags;	/* flags */
} pfc_cfdef_param_t;

/*
 * cp_nelems values which have special meaning.
 */

/* Parameter has scalar value, not array. */
#define	PFC_CFPARAM_NELEMS_SCALAR	(0)

/* Parameter has variable-length array. */
#define	PFC_CFPARAM_NELEMS_VARLEN	(-1)

/*
 * Determine whether the given parameter definition represents an array
 * parameter or not.
 */
#define	PFC_CFDEF_PARAM_IS_ARRAY(pdp)				\
	((pdp)->cfdp_nelems != PFC_CFPARAM_NELEMS_SCALAR)

/*
 * Flags for cfdp_flags.
 */
#define	PFC_CFPF_MANDATORY	PFC_CONST_U(0x1)	/* mandatory */

/*
 * Configuration block.
 */
typedef struct {
	const char		*cfdb_name;	/* name of block */
	const pfc_cfdef_param_t	*cfdb_params;	/* parameters */
	uint32_t		cfdb_nparams;	/* number of parameters */
	uint32_t		cfdb_flags;	/* flags */
} pfc_cfdef_block_t;

/*
 * Flags for cb_flags.
 */
#define	PFC_CFBF_MAP		PFC_CONST_U(0x1)	/* map block */
#define	PFC_CFBF_MANDATORY	PFC_CONST_U(0x2)	/* mandatory */

/*
 * Configuration file definition.
 */
typedef struct {
	const pfc_cfdef_block_t	*cfd_block;	/* block definitions */
	uint32_t		cfd_nblocks;	/* number of blocks */
} pfc_cfdef_t;

PFC_C_END_DECL

#endif	/* !_PFC_CONF_PARSER_H */
