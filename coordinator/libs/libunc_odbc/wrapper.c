/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * wrapper.c - Declare unixODBC wrapper functions.
 */

#include <pfc/synch.h>
#include <pfc/debug.h>
#include <unc/odbc.h>

/*
 * The global lock that serializes unixODBC function call.
 */
static pfc_mutex_t	odbc_mutex = PFC_MUTEX_INITIALIZER;

#define	ODBC_API_LOCK()		pfc_mutex_lock(&odbc_mutex)
#define	ODBC_API_UNLOCK()	pfc_mutex_unlock(&odbc_mutex)

/*
 * Macros to declare wrapper function.
 */
#define	ODBC_API_WRAPPER(name, ret_type, args_decl, args)		\
	ret_type							\
	_unc_##name args_decl						\
	{								\
		ret_type	ret;					\
									\
		ODBC_API_LOCK();					\
		ret = name args;					\
		ODBC_API_UNLOCK();					\
		return ret;						\
	}

/*
 * Declare unixODBC wrapper functions.
 */
ODBC_API_WRAPPER(SQLConnect, SQLRETURN,
		 (SQLHDBC ConnectionHandle, SQLCHAR *ServerName,
		  SQLSMALLINT NameLength1, SQLCHAR *UserName,
		  SQLSMALLINT NameLength2, SQLCHAR *Authentication,
		  SQLSMALLINT NameLength3),
		 (ConnectionHandle, ServerName, NameLength1, UserName,
		  NameLength2, Authentication, NameLength3));
ODBC_API_WRAPPER(SQLConnectW, SQLRETURN,
		 (SQLHDBC hdbc, SQLWCHAR *szDSN, SQLSMALLINT cbDSN,
		  SQLWCHAR *szUID, SQLSMALLINT cbUID, SQLWCHAR *szAuthStr,
		  SQLSMALLINT cbAuthStr),
		 (hdbc, szDSN, cbDSN, szUID, cbUID, szAuthStr, cbAuthStr));
ODBC_API_WRAPPER(SQLConnectA, SQLRETURN,
		 (SQLHDBC hdbc, SQLCHAR *szDSN, SQLSMALLINT cbDSN,
		  SQLCHAR *szUID, SQLSMALLINT cbUID, SQLCHAR *szAuthStr,
		  SQLSMALLINT cbAuthStr),
		 (hdbc, szDSN, cbDSN, szUID, cbUID, szAuthStr, cbAuthStr));

ODBC_API_WRAPPER(SQLBrowseConnect, SQLRETURN,
		 (SQLHDBC hdbc, SQLCHAR *szConnStrIn, SQLSMALLINT cbConnStrIn,
		  SQLCHAR *szConnStrOut, SQLSMALLINT cbConnStrOutMax,
		  SQLSMALLINT *pcbConnStrOut),
		 (hdbc, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut));
ODBC_API_WRAPPER(SQLBrowseConnectW, SQLRETURN,
		 (SQLHDBC hdbc, SQLWCHAR *szConnStrIn, SQLSMALLINT cbConnStrIn,
		  SQLWCHAR *szConnStrOut, SQLSMALLINT cbConnStrOutMax,
		  SQLSMALLINT *pcbConnStrOut),
		 (hdbc, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut));
ODBC_API_WRAPPER(SQLBrowseConnectA, SQLRETURN,
		 (SQLHDBC hdbc, SQLCHAR *szConnStrIn, SQLSMALLINT cbConnStrIn,
		  SQLCHAR *szConnStrOut, SQLSMALLINT cbConnStrOutMax,
		  SQLSMALLINT *pcbConnStrOut),
		 (hdbc, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut));

ODBC_API_WRAPPER(SQLDriverConnect, SQLRETURN,
		 (SQLHDBC hdbc, SQLHWND hwnd, SQLCHAR *szConnStrIn,
		  SQLSMALLINT cbConnStrIn, SQLCHAR *szConnStrOut,
		  SQLSMALLINT cbConnStrOutMax, SQLSMALLINT *pcbConnStrOut,
		  SQLUSMALLINT fDriverCompletion),
		 (hdbc, hwnd, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut, fDriverCompletion));
ODBC_API_WRAPPER(SQLDriverConnectW, SQLRETURN,
		 (SQLHDBC hdbc, SQLHWND hwnd, SQLWCHAR *szConnStrIn,
		  SQLSMALLINT cbConnStrIn, SQLWCHAR *szConnStrOut,
		  SQLSMALLINT cbConnStrOutMax, SQLSMALLINT *pcbConnStrOut,
		  SQLUSMALLINT fDriverCompletion),
		 (hdbc, hwnd, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut, fDriverCompletion));
ODBC_API_WRAPPER(SQLDriverConnectA, SQLRETURN,
		 (SQLHDBC hdbc, SQLHWND hwnd, SQLCHAR *szConnStrIn,
		  SQLSMALLINT cbConnStrIn, SQLCHAR *szConnStrOut,
		  SQLSMALLINT cbConnStrOutMax, SQLSMALLINT *pcbConnStrOut,
		  SQLUSMALLINT fDriverCompletion),
		 (hdbc, hwnd, szConnStrIn, cbConnStrIn, szConnStrOut,
		  cbConnStrOutMax, pcbConnStrOut, fDriverCompletion));

ODBC_API_WRAPPER(SQLDisconnect, SQLRETURN, (SQLHDBC ConnectionHandle),
		 (ConnectionHandle));

ODBC_API_WRAPPER(SQLAllocConnect, SQLRETURN,
		 (SQLHENV EnvironmentHandle, SQLHDBC *ConnectionHandle),
		 (EnvironmentHandle, ConnectionHandle));

ODBC_API_WRAPPER(SQLFreeConnect, SQLRETURN, (SQLHDBC ConnectionHandle),
		 (ConnectionHandle));

ODBC_API_WRAPPER(SQLAllocHandle, SQLRETURN,
		 (SQLSMALLINT HandleType, SQLHANDLE InputHandle,
		  SQLHANDLE *OutputHandle),
		 (HandleType, InputHandle, OutputHandle));

ODBC_API_WRAPPER(SQLFreeHandle, SQLRETURN,
		 (SQLSMALLINT HandleType, SQLHANDLE Handle),
		 (HandleType, Handle));

ODBC_API_WRAPPER(SQLFreeEnv, SQLRETURN, (SQLHENV EnvironmentHandle),
		 (EnvironmentHandle));

ODBC_API_WRAPPER(SQLTransact, SQLRETURN,
		 (SQLHENV EnvironmentHandle, SQLHDBC ConnectionHandle,
		  SQLUSMALLINT CompletionType),
		 (EnvironmentHandle, ConnectionHandle, CompletionType));

ODBC_API_WRAPPER(SQLEndTran, SQLRETURN,
		 (SQLSMALLINT HandleType, SQLHANDLE Handle,
		  SQLSMALLINT CompletionType),
		 (HandleType, Handle, CompletionType));
