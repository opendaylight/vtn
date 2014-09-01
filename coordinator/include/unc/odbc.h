/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_ODBC_H
#define	_UNC_ODBC_H

/*
 * Declare wrapper functions for ODBC APIs.
 */

#ifndef	_PFC_IN_CFDEFC

#include <unc/base.h>
#include <sql.h>
#include <sqlext.h>

UNC_C_BEGIN_DECL

/*
 * Prototypes.
 */
extern SQLRETURN	_unc_SQLConnect(SQLHDBC ConnectionHandle,
					SQLCHAR *ServerName,
					SQLSMALLINT NameLength1,
					SQLCHAR *UserName,
					SQLSMALLINT NameLength2,
					SQLCHAR *Authentication,
					SQLSMALLINT NameLength3);
extern SQLRETURN	_unc_SQLConnectW(SQLHDBC hdbc, SQLWCHAR *szDSN,
					 SQLSMALLINT cbDSN, SQLWCHAR *szUID,
					 SQLSMALLINT cbUID,
					 SQLWCHAR *szAuthStr,
					 SQLSMALLINT cbAuthStr);
extern SQLRETURN	_unc_SQLConnectA(SQLHDBC hdbc, SQLCHAR *szDSN,
					 SQLSMALLINT cbDSN, SQLCHAR *szUID,
					 SQLSMALLINT cbUID, SQLCHAR *szAuthStr,
					 SQLSMALLINT cbAuthStr);

extern SQLRETURN	_unc_SQLBrowseConnect(SQLHDBC hdbc,
					      SQLCHAR *szConnStrIn,
					      SQLSMALLINT cbConnStrIn,
					      SQLCHAR *szConnStrOut,
					      SQLSMALLINT cbConnStrOutMax,
					      SQLSMALLINT *pcbConnStrOut);
extern SQLRETURN	_unc_SQLBrowseConnectW(SQLHDBC hdbc,
					       SQLWCHAR *szConnStrIn,
					       SQLSMALLINT cbConnStrIn,
					       SQLWCHAR *szConnStrOut,
					       SQLSMALLINT cbConnStrOutMax,
					       SQLSMALLINT *pcbConnStrOut);
extern SQLRETURN	_unc_SQLBrowseConnectA(SQLHDBC hdbc,
					       SQLCHAR *szConnStrIn,
					       SQLSMALLINT cbConnStrIn,
					       SQLCHAR *szConnStrOut,
					       SQLSMALLINT cbConnStrOutMax,
					       SQLSMALLINT *pcbConnStrOut);

extern SQLRETURN	_unc_SQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd,
					      SQLCHAR *szConnStrIn,
					      SQLSMALLINT cbConnStrIn,
					      SQLCHAR *szConnStrOut,
					      SQLSMALLINT cbConnStrOutMax,
					      SQLSMALLINT *pcbConnStrOut,
					      SQLUSMALLINT fDriverCompletion);
extern SQLRETURN	_unc_SQLDriverConnectW(SQLHDBC hdbc, SQLHWND hwnd,
					       SQLWCHAR *szConnStrIn,
					       SQLSMALLINT cbConnStrIn,
					       SQLWCHAR *szConnStrOut,
					       SQLSMALLINT cbConnStrOutMax,
					       SQLSMALLINT *pcbConnStrOut,
					       SQLUSMALLINT fDriverCompletion);
extern SQLRETURN	_unc_SQLDriverConnectA(SQLHDBC hdbc, SQLHWND hwnd,
					       SQLCHAR *szConnStrIn,
					       SQLSMALLINT cbConnStrIn,
					       SQLCHAR *szConnStrOut,
					       SQLSMALLINT cbConnStrOutMax,
					       SQLSMALLINT *pcbConnStrOut,
					       SQLUSMALLINT fDriverCompletion);

extern SQLRETURN	_unc_SQLDisconnect(SQLHDBC ConnectionHandle);
extern SQLRETURN	_unc_SQLAllocConnect(SQLHENV EnvironmentHandle,
					     SQLHDBC *ConnectionHandle);
extern SQLRETURN	_unc_SQLFreeConnect(SQLHDBC ConnectionHandle);
extern SQLRETURN	_unc_SQLAllocHandle(SQLSMALLINT HandleType,
					    SQLHANDLE InputHandle,
					    SQLHANDLE *OutputHandle);
extern SQLRETURN	_unc_SQLFreeHandle(SQLSMALLINT HandleType,
					   SQLHANDLE Handle);
extern SQLRETURN	_unc_SQLFreeEnv(SQLHENV EnvironmentHandle);
extern SQLRETURN	_unc_SQLTransact(SQLHENV EnvironmentHandle,
					 SQLHDBC ConnectionHandle,
					 SQLUSMALLINT CompletionType);
extern SQLRETURN	_unc_SQLEndTran(SQLSMALLINT HandleType,
					SQLHANDLE Handle,
					SQLSMALLINT CompletionType);

#ifndef	_UNC_ODBC_NOWRAPPER

/*
 * Define mappings to wrapper functions.
 */
#undef	SQLConnect
#undef	SQLBrowseConnect
#undef	SQLDriverConnect

#if	!defined(SQL_NOUNICODEMAP) && defined(UNICODE)
#define	SQLConnect		_unc_SQLConnectW
#define	SQLBrowseConnect	_unc_SQLBrowseConnectW
#define	SQLDriverConnect	_unc_SQLDriverConnectW
#else	/* defined(SQL_NOUNICODEMAP) || !defined(UNICODE) */
#define	SQLConnect		_unc_SQLConnect
#define	SQLBrowseConnect	_unc_SQLBrowseConnect
#define	SQLDriverConnect	_unc_SQLDriverConnect
#endif	/* !defined(SQL_NOUNICODEMAP) && defined(UNICODE) */

#define	SQLConnectA		_unc_SQLConnectA
#define	SQLBrowseConnectA	_unc_SQLBrowseConnectA
#define	SQLDriverConnectA	_unc_SQLDriverConnectA

#define	SQLDisconnect		_unc_SQLDisconnect
#define	SQLAllocConnect		_unc_SQLAllocConnect
#define	SQLFreeConnect		_unc_SQLFreeConnect
#define	SQLFreeEnv		_unc_SQLFreeEnv
#define	SQLTransact		_unc_SQLTransact

#if	(ODBCVER >= 0x0300)
#define	SQLAllocHandle		_unc_SQLAllocHandle
#define	SQLFreeHandle		_unc_SQLFreeHandle
#define	SQLEndTran		_unc_SQLEndTran
#endif	/* (ODBCVER >= 0x0300) */

#endif	/* !_UNC_ODBC_NOWRAPPER */

UNC_C_END_DECL

#endif	/* !_PFC_IN_CFDEFC */

#endif	/* !_UNC_ODBC_H */
