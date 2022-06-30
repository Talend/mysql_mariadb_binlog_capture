/*
* ddbinlogdll - DataDistribution binlog C/C++ library
*
* 2021 GammaSoft
*
*/

/**
* @file ddbinlogdll.cpp
* @brief DataDistribution Binlog C/C++ library for dll call.
*
*/

#include <iostream>
#include <windows.h>

#define _BINLOG_PTR_DECL_NULL(NAME) _##NAME h##NAME = NULL  
#define _BINLOG_PTR_DECL_INIT(NAME) h##NAME = ##NAME
#define _BINLOG_PTR_PROC_DECL(NAME)														\
		if (result)																		\
		{																				\
			h##NAME = (_##NAME) GetProcAddress(gBinlogLib, #NAME);	\
			result = h##NAME ? true:false;										\
			sprintf(funcName, #NAME);													\
			if (!result)																\
			{																			\
				if (msg) {sprintf(msg, "%s:%s", funcName, GetDllErrorBinlog(errorMsg));}	\
			}																			\
		}										

static HINSTANCE gBinlogLib;

_BINLOG_PTR_DECL_NULL(main_binlog);
_BINLOG_PTR_DECL_NULL(InitMySql);
_BINLOG_PTR_DECL_NULL(MySetVerbose);
_BINLOG_PTR_DECL_NULL(MySetTrace);
_BINLOG_PTR_DECL_NULL(InitRemoteReadBinLog);
_BINLOG_PTR_DECL_NULL(FreeReadBinLog);
_BINLOG_PTR_DECL_NULL(StartReadBinLog);
_BINLOG_PTR_DECL_NULL(PrintfResult);

char* GetDllErrorBinlog(/* out */char *msg)
{
	DWORD error = GetLastSystemError();

	char errorMsg[1024];
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,                    // source
		error,				// code d'erreur
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errorMsg,				// où stocker la description  
		sizeof(errorMsg),		// taille minimale à retourner
		0);
	sprintf(msg, "%ld - %s", error, errorMsg);
	return msg;
}

bool LoaddbinlogDll(char* libName,  /* out */char *msg)
{
	char funcName[100];
	char errorMsg[1000];
	bool result = true; // you must initialize this variable to true at first 
	
	if ((gBinlogLib = LoadLibrary(libName)) != NULL)
	{
		_BINLOG_PTR_PROC_DECL(main_binlog)
		_BINLOG_PTR_PROC_DECL(InitMySql)
		_BINLOG_PTR_PROC_DECL(MySetVerbose)
		_BINLOG_PTR_PROC_DECL(MySetTrace)
		_BINLOG_PTR_PROC_DECL(InitRemoteReadBinLog)
		_BINLOG_PTR_PROC_DECL(FreeReadBinLog)
		_BINLOG_PTR_PROC_DECL(StartReadBinLog)
		_BINLOG_PTR_PROC_DECL(PrintfResult)

		if (!result)
		{
			if (msg)  sprintf(msg, "LoaddbinlogDll:GetProcAddress '%s' error ", funcName);
		}
	}
	else
	{
		result = false;
		if (msg) GetDllErrorBinlog(msg);
	}
	if (result)
	{
		if (msg)  sprintf(msg, "LoaddbinlogDll success");
	}
	return result;	
}
