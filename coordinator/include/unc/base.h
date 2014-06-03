/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_BASE_H
#define	_UNC_BASE_H

/*
 * Basic definition for UNC software.
 * This header assumes to be included from all UNC sources.
 */

#include <pfc/base.h>

/* Import build configuration. */
#include <unc/config.h>

/* Declare "C" block for C++ sources. */
#define	UNC_C_BEGIN_DECL	PFC_C_BEGIN_DECL
#define	UNC_C_END_DECL		PFC_C_END_DECL

UNC_C_BEGIN_DECL

/* Create UNC software version number. */
#define	UNC_VERSION_MAKE_NUMBER(major, minor, rev, patch)	\
	(((uint64_t)((major) & 0xffffU) << 48) |		\
	 ((uint64_t)((minor) & 0xffffU) << 32) |		\
	 ((uint64_t)((rev) & 0xffffU) << 16) |			\
	 (uint64_t)((patch) & 0xffffU))

/* Unsigned 64-bit value which represents UNC software version. */
#define	UNC_VERSION_NUMBER						\
	UNC_VERSION_MAKE_NUMBER(UNC_VERSION_MAJOR,			\
				UNC_VERSION_MINOR,			\
				UNC_VERSION_REVISION,			\
				UNC_VERSION_PATCHLEVEL)

/* Parse bit field in UNC_VERSION_NUMBER. */
#define	UNC_VERSION_GET_MAJOR(ver)			\
	((uint16_t)(((uint64_t)(ver) >> 48) & 0xffffU))
#define	UNC_VERSION_GET_MINOR(ver)			\
	((uint16_t)(((uint64_t)(ver) >> 32) & 0xffffU))
#define	UNC_VERSION_GET_REVISION(ver)			\
	((uint16_t)(((uint64_t)(ver) >> 16) & 0xffffU))
#define	UNC_VERSION_GET_PATCHLEVEL(ver)			\
	((uint16_t)((uint64_t)(ver) & 0xffffU))

/* C99 restrict keyword. */
#define	UNC_RESTRICT		PFC_RESTRICT

/*
 * Determine whether long int and pointer are 64-bit long or not.
 */
#ifdef	PFC_LP64
#define	UNC_LP64	1
#else	/* !PFC_LP64 */
#undef	UNC_LP64
#endif	/* PFC_LP64 */

/*
 * String literals for printf(3) conversion specifier.
 */

#ifdef	UNC_LP64

/*
 * ODBC data types.
 */

/* DWORD is defined as unsigned int. */
#define	UNC_PFMT_DWORD			"u"
#define	UNC_PFMT_xDWORD			"x"
#define	UNC_PFMT_XDWORD			"X"

/* SDWORD is defined as int. */
#define	UNC_PFMT_SDWORD			"d"
#define	UNC_PFMT_xSDWORD		"x"
#define	UNC_PFMT_XSDWORD		"X"

/* UDWORD is defined as unsigned int. */
#define	UNC_PFMT_UDWORD			"u"
#define	UNC_PFMT_xUDWORD		"x"
#define	UNC_PFMT_XUDWORD		"X"

/* SQLINTEGER is defined as int. */
#define	UNC_PFMT_SQLINTEGER		"d"
#define	UNC_PFMT_xSQLINTEGER		"x"
#define	UNC_PFMT_XSQLINTEGER		"X"

/* SQLUINTEGER is defined as unsigned int. */
#define	UNC_PFMT_SQLUINTEGER		"u"
#define	UNC_PFMT_xSQLUINTEGER		"x"
#define	UNC_PFMT_XSQLUINTEGER		"X"

/* ODBCINT64 is defined as long. */
#define	UNC_PFMT_ODBCINT64		"ld"
#define	UNC_PFMT_xODBCINT64		"lX"
#define	UNC_PFMT_XODBCINT64		"lX"

/* UODBCINT64 is defined as unsigned long. */
#define	UNC_PFMT_UODBCINT64		"lu"
#define	UNC_PFMT_xUODBCINT64		"lX"
#define	UNC_PFMT_XUODBCINT64		"lX"

/* SQLLEN is defined as long. */
#define	UNC_PFMT_SQLLEN			"ld"
#define	UNC_PFMT_xSQLLEN		"lx"
#define	UNC_PFMT_XSQLLEN		"lX"

#else	/* !UNC_LP64 */

/*
 * ODBC data types.
 */

/* DWORD is defined as unsigned long. */
#define	UNC_PFMT_DWORD			"lu"
#define	UNC_PFMT_xDWORD			"lx"
#define	UNC_PFMT_XDWORD			"lX"

/* SDWORD is defined as long int. */
#define	UNC_PFMT_SDWORD			"ld"
#define	UNC_PFMT_xSDWORD		"lx"
#define	UNC_PFMT_XSDWORD		"lX"

/* UDWORD is defined as unsigned long int. */
#define	UNC_PFMT_UDWORD			"lu"
#define	UNC_PFMT_xUDWORD		"lx"
#define	UNC_PFMT_XUDWORD		"lX"

/* SQLINTEGER is defined as long. */
#define	UNC_PFMT_SQLINTEGER		"ld"
#define	UNC_PFMT_xSQLINTEGER		"lx"
#define	UNC_PFMT_XSQLINTEGER		"lX"

/* SQLUINTEGER is defined as unsigned long. */
#define	UNC_PFMT_SQLUINTEGER		"lu"
#define	UNC_PFMT_xSQLUINTEGER		"lx"
#define	UNC_PFMT_XSQLUINTEGER		"lX"

/* ODBCINT64 is defined as long long. */
#define	UNC_PFMT_ODBCINT64		"lld"
#define	UNC_PFMT_xODBCINT64		"llX"
#define	UNC_PFMT_XODBCINT64		"llX"

/* UODBCINT64 is defined as unsigned long long. */
#define	UNC_PFMT_UODBCINT64		"llu"
#define	UNC_PFMT_xUODBCINT64		"llX"
#define	UNC_PFMT_XUODBCINT64		"llX"

/* SQLLEN is defined as SQLINTEGER. */
#define	UNC_PFMT_SQLLEN			UNC_PFMT_SQLINTEGER
#define	UNC_PFMT_xSQLLEN		UNC_PFMT_xSQLINTEGER
#define	UNC_PFMT_XSQLLEN		UNC_PFMT_XSQLINTEGER

#endif	/* UNC_LP64 */

#define	UNC_PFMT_SQLBIGINT		UNC_PFMT_ODBCINT64
#define	UNC_PFMT_xSQLBIGINT		UNC_PFMT_xODBCINT64
#define	UNC_PFMT_XSQLBIGINT		UNC_PFMT_XODBCINT64

#define	UNC_PFMT_SQLUBIGINT		UNC_PFMT_UODBCINT64
#define	UNC_PFMT_xSQLUBIGINT		UNC_PFMT_xUODBCINT64
#define	UNC_PFMT_XSQLUBIGINT		UNC_PFMT_XUODBCINT64

UNC_C_END_DECL

#endif	/* !_UNC_BASE_H */
