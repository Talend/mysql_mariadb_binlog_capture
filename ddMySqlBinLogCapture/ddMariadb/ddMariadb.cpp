/* Copyright (C) 2021-2022 Talend Inc. - www.talend.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02111-1301, USA */
//////////////////////////////////////////////////////////////////////////////////
//
//																			
//////////////////////////////////////////////////////////////////////////////////
//																			
//	FILE				:ddMariadb.cpp
//																			
//	PURPOSE				:Defines the exported functions for the DLL application.
//																			
//
//	DATE				:29/12/2011
//
//	2017-05-21 add define _STORE_RESULT and _USE_RESULT used by MyGetResult
//////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////////
// Native access used only with visual studion > VC2010  beacause type long long doesn't exit in VC6
///////////////////////////////////////////////////////////////////////////////////
//MSVC++ 10.0 _MSC_VER = 1600 
//MSVC++ 9.0  _MSC_VER = 1500 
//MSVC++ 8.0  _MSC_VER = 1400 
//MSVC++ 7.1  _MSC_VER = 1310 
//MSVC++ 7.0  _MSC_VER = 1300 
//MSVC++ 6.0  _MSC_VER = 1200 
//MSVC++ 5.0  _MSC_VER = 1100
#if (_MSC_VER && _MSC_VER > 1400)

#include <mysql.h>

#pragma comment(lib, "mariadbclient.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "crypt32")
#pragma comment(lib, "secur32")

#endif

#include "ddMariadb.h"

static long verbosity = 0;

//////////////////////////////////////////////////////////////////////////
// Defines the entry point for the DLL application.
//////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// MySetVerbosity
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MySetVerbosity(const int verbose)
{
	verbosity = verbose;
	if (verbosity) printf("\nDLL:MySetVerbosity to %d", verbosity);
	return verbosity;
}

#if (_MSC_VER && _MSC_VER > 1400)

//////////////////////////////////////////////////////////////////////////////////////////////////////
// InitDB
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyInitDB(void **handle)
{
	*handle = (void*)mysql_init(NULL);
	return (*handle) ? mySUCCESS : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// FreeDB
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyFreeDB()
{
	mariadb_cancel(0);
	return mySUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// SetOptionDB
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MySetOptionDB(void *handle, mysql_option option, const void *pArg)
{
	if (handle) mysql_optionsv((MYSQL*)handle, option, pArg);
	return (handle) ? mySUCCESS : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenConnection
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyOpenConnection(void *handle, const char *pszHost, const char *pszUser, const char *pszPassword, int iPort, bool fAutoReconnect, char *pszProtocol, char ** socket)
{
	MYSQL* res = NULL;

	if (pszHost == NULL || pszUser == NULL || pszPassword == NULL)	return myERROR_PARAM;

	if (handle)
	{
		unsigned int uiOptProtocol = pszProtocol == "SOCKET" ? MYSQL_PROTOCOL_SOCKET : MYSQL_PROTOCOL_TCP;

		mysql_optionsv((MYSQL*)handle, MYSQL_OPT_PROTOCOL, (void*)&uiOptProtocol);
		res = mysql_real_connect((MYSQL*)handle, pszHost, pszUser, pszPassword, 0, iPort, *socket, 0);
		if (verbosity && !res) printf("\nDLL:MyOpenConnection Error:<%s>", mysql_error((MYSQL*)handle));
		// set to 1 if automatic reconnect
		if (res && fAutoReconnect)  ((MYSQL*)handle)->options.reconnect = 1;
		if (verbosity && !res) printf("\nDLL:MyOpenConnection OK");
	}
	return (handle) ? res ? mySUCCESS : myERROR_HANDLE_CONNECTION : myERROR_HANDLE_INIT;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// CloseConnection
// close the current connection and free the PGconn data structure
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyCloseConnection(void *handle)
{
	if (handle) mysql_close((MYSQL*)handle);
	return (handle) ? mySUCCESS : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetErrormessage
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetErrormessage(void *handle, char **hMsg)
{
	*hMsg = handle ? (char*)mysql_error((MYSQL*)handle) : NULL;
	return handle ? (*hMsg) ? mySUCCESS : myERROR_HANDLE_MSG : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetErrorNum
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetErrorNum(void *handle, unsigned int *hMsg)
{
	*hMsg = handle ? mysql_errno((MYSQL*)handle) : 0;
	return handle ? (hMsg) ? mySUCCESS : myERROR_HANDLE_MSG : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetSqlState
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetSqlState(void *handle, char **hMsg)
{
	*hMsg = handle ? (char*)mysql_sqlstate((MYSQL*)handle) : NULL;
	return handle ? (*hMsg) ? mySUCCESS : myERROR_HANDLE_MSG : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ExecStatement
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyExecStatement(void *handle, const char *query)
{
	if (query == NULL)	return myERROR_PARAM;

	int res = mysql_query((MYSQL*)handle, query);
	if (verbosity && !res) printf("\nDLL:MyExecStatement Error:<%s>", mysql_error((MYSQL*)handle));
	return handle ? res ? myERROR_HANDLE_QUERY : mySUCCESS : myERROR_HANDLE_INIT;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// FreeStatement
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyFreeStatement(void *hQuery)
{
	if (hQuery) mysql_free_result((MYSQL_RES*)hQuery);
	return hQuery ? mySUCCESS : myERROR_HANDLE_QUERY;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetResult
// Error:
// CR_COMMANDS_OUT_OF_SYNC
// CR_OUT_OF_MEMORY
// CR_SERVER_GONE_ERROR
// CR_SERVER_LOST
// CR_UNKNOWN_ERROR
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetResult(void *handle, void **hQuery)
{
#ifdef _STORE_RESULT
	*hQuery = handle ? (void*)mysql_store_result((MYSQL*)handle) : NULL;
#else
	*hQuery = handle ? (void*)mysql_use_result((MYSQL*)handle) : NULL;
#endif
	return handle ? (*hQuery) ? mySUCCESS : myERROR_HANDLE_QUERY : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetResultValue
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetResultValue(MYSQL_ROW hRow, int nColumn, char **pszResult)
{
	*pszResult = hRow ? hRow[nColumn] : NULL;
	return hRow ? (*pszResult) ? mySUCCESS : myERROR_HANDLE_RESULT : myERROR_HANDLE_ROW;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// GetFetchRow
//
// Error:
// CR_SERVER_LOST
// CR_UNKNOW_ERROR
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetFetchRow(void *hQuery, MYSQL_ROW *hRow)
{
	if (hRow == NULL) return myERROR_HANDLE_ROW;
	*hRow = hQuery ? mysql_fetch_row((MYSQL_RES*)hQuery) : NULL;
	if (verbosity) printf("\nDLL:MyGetFetchRow %p ", *hRow);
	return hQuery ? (*hRow) ? mySUCCESS : myERROR_NO_DATA : myERROR_HANDLE_QUERY;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetAffectedRows
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetAffectedRows(void *handle)
{
	int res = mysql_affected_rows((MYSQL*)handle);
	if (verbosity) printf("\nDLL:MyGetAffectedRows Count:<%d>", res);
	return handle ? res : myERROR_HANDLE_INIT;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetInfo
// Retrieve � string providing information about the most recently executed statement.
// Only for the statement listed here:
// INSERT INTO ... SELECT ...						string format = Records: 100 Duplicates: 0 Warnings: 0
// INSERT INTO ... VALUES (...), ((...),(...) ...	string format = Records: 3 Duplicates: 0 Warnings: 0
// UPDATE ...										string format = Rows matched: 40 Changed: 0 Warnings: 0
// LOAD DATA INFILE...								string format = Records: 3 Deleted: 0 Skipped: 0 Warnings: 0
// ALTER YABLE ...									string format = Records: 3 Duplicates: 0 Warnings: 0
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetInfo(void *handle, char **hMsg)
{
	*hMsg = handle ? (char*)mysql_info((MYSQL*)handle) : NULL;
	return handle ? (*hMsg) ? mySUCCESS : myERROR_HANDLE_MSG : myERROR_HANDLE_INIT;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetResultCount
// Return the number of rows in the query result
// If return < 0 ==> return = num error
// If return >=0 return = count row
//
// No error
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetResultCount(void *hQuery)
{
	return hQuery ? (long)mysql_num_rows((MYSQL_RES*)hQuery) : myERROR_HANDLE_QUERY;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetNumField
// Returns the number of columns (fields) in the query result.
// If return < 0 ==> return = num error
// If return >=0 return = count row
//
// No error
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetNumField(void *hQuery)
{
	return hQuery ? (long)mysql_num_fields((MYSQL_RES*)hQuery) : myERROR_HANDLE_QUERY;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetGetResultLength
//
// No error
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetResultLength(void *hQuery, unsigned long **hLength)
{
	if (hLength == NULL) return myERROR_HANDLE_INIT;
	*hLength = hQuery ? mysql_fetch_lengths((MYSQL_RES*)hQuery) : NULL;
	if (verbosity) printf("\nDLL:MyGetResultLength %p ", *hLength);
	return hQuery ? (*hLength) ? mySUCCESS : myERROR_HANDLE_RESULT : myERROR_HANDLE_QUERY;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// MyGetResultField
// No error
/////////////////////////////////////////////////////////////////////////////////////////////////////
DDMYSQL_API long PASCAL MyGetResultField(void *hQuery, MYSQL_FIELD **hField)
{
	if (hField == NULL) return myERROR_HANDLE_INIT;
	*hField = hQuery ? mysql_fetch_fields((MYSQL_RES*)hQuery) : NULL;
	if (verbosity) printf("\nDLL:MyGetResultField %p ", *hField);
	return hQuery ? (*hField) ? mySUCCESS : myERROR_HANDLE_RESULT : myERROR_HANDLE_QUERY;
}


#else

DDMYSQL_API long PASCAL MyInitDB(void **handle)
{
	if (verbosity) printf("\nDLL:MyInitDB not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyFreeDB()
{
	if (verbosity) printf("\nDLL:MyFreeDB not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MySetOptionDB(void *handle, mysql_option option, const void *pArg)
{
	if (verbosity) printf("\nDLL:MySetOptionDB not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyOpenConnection(void *handle, const char *pszHost, const char *pszUser, const char *pszPassword, int iPort, bool fAutoReconnect, char *pszProtocol, char ** socket)
{
	if (verbosity) printf("\nDLL:MyOpenConnection not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyCloseConnection(void *handle)
{
	if (verbosity) printf("\nDLL:MyCloseConnection not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetErrormessage(void *handle, char **hMsg)
{
	if (verbosity) printf("\nDLL:MyGetErrormessage not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetErrorNum(void *handle, unsigned int *hMsg)
{
	if (verbosity) printf("\nDLL:MyGetErrorNum not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetSqlState(void *handle, char **hMsg)
{
	if (verbosity) printf("\nDLL:MyGetSqlState not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyExecStatement(void *handle, const char *query)
{
	if (verbosity) printf("\nDLL:MyExecStatement not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyFreeStatement(void *hQuery)
{
	if (verbosity) printf("\nDLL:MyFreeStatement not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetResult(void *handle, void **hQuery)
{
	if (verbosity) printf("\nDLL:MyGetResult not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetResultValue(MYSQL_ROW hRow, int nColumn, char **pszResult)
{
	if (verbosity) printf("\nDLL:MyGetResultValue not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetFetchRow(void *hQuery, MYSQL_ROW *hRow)
{
	if (verbosity) printf("\nDLL:MyGetFetchRow not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetAffectedRows(void *handle)
{
	if (verbosity) printf("\nDLL:MyGetAffectedRows not implemented");
	return myERROR_HANDLE_INIT;
}

DDMYSQL_API long PASCAL MyGetInfo(void *handle, char **hMsg)
{
	if (verbosity) printf("\nDLL:MyGetInfo not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetNumField(void *hQuery)
{
	if (verbosity) printf("\nDLL:MyGetNumField not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetResultLength(void *hQuery, unsigned long **hLength)
{
	if (verbosity) printf("\nDLL:MyGetResultLength not implemented");
	return myERROR_HANDLE_INIT;
}
DDMYSQL_API long PASCAL MyGetResultField(void *hQuery, MYSQL_FIELD **hField)
{
	if (verbosity) printf("\nDLL:MyGetResultField not implemented");
	return myERROR_HANDLE_INIT;
}

DDMYSQL_API long PASCAL MyGetResultCount(void *hQuery)
{
	if (verbosity) printf("\nDLL:MyGetResultCount not implemented");
	return myERROR_HANDLE_INIT;
}

#endif
