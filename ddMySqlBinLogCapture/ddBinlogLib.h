/* Copyright (C) 2022 Talend Inc. - www.talend.com

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
/*
* ddBinlogLib.h - DataDistribution MariadDB C/C++ library
*
*
*/
#ifndef DDBINLOG_H
#define DDBINLOG_H

#pragma once

#ifdef _MSC_VER
#if _MSC_VER >1400
#include <stdint.h>
#endif
#else
#include <stdint.h>
#endif

#ifdef _MSC_VER
#undef DD_EXPORT
#ifdef BINLOG_EXPORTS
#define DD_EXPORT extern "C" __declspec(dllexport)
#else
#define DD_EXPORT extern "C" __declspec(dllimport)
#endif
#else
#define DD_EXPORT
#endif

#include "MyLogFile.h"

#define _BINLOG_PTR_DECL(NAME) extern  _##NAME h##NAME

#define DD_DLL 

#ifndef BINLOG_EXPORTS

/**
Exit status for functions in this file.
*/
enum Exit_status {
	/** No error occurred and execution should continue. */
	OK_CONTINUE = 0,
	/** An error occurred and execution should stop. */
	ERROR_STOP,
	/** No error occurred but execution should stop. */
	OK_STOP,
	/** No error occurred - end of file reached. */
	OK_EOF,
};

#endif

DD_EXPORT int main_binlog(int argc, char** argv);
typedef int (DD_DLL* _main_binlog)(int argc, char** argv);
_BINLOG_PTR_DECL(main_binlog);

DD_EXPORT void InitMySql(const char *pszAppName);
typedef void (DD_DLL* _InitMySql)(const char *pszAppName);
_BINLOG_PTR_DECL(InitMySql);

DD_EXPORT void MySetVerbose(int v);
typedef void (DD_DLL* _MySetVerbose)(int v);
_BINLOG_PTR_DECL(MySetVerbose);

DD_EXPORT void MySetTrace(int trace, int level);
typedef void (DD_DLL* _MySetTrace)(int trace, int level);
_BINLOG_PTR_DECL(MySetTrace);

DD_EXPORT Exit_status InitRemoteReadBinLog(const char *pszAppName, const char *pszHost, const char *pszUser, const char *pszPassword, const char *pszPort);
typedef Exit_status (DD_DLL* _InitRemoteReadBinLog)(const char *pszAppName, const char *pszHost, const char *pszUser, const char *pszPassword, const char *pszPort);
_BINLOG_PTR_DECL(InitRemoteReadBinLog);

DD_EXPORT Exit_status FreeReadBinLog(void);
typedef Exit_status (DD_DLL* _FreeReadBinLog)(void);
_BINLOG_PTR_DECL(FreeReadBinLog);

DD_EXPORT Exit_status StartReadBinLog(const char* pszLogName, unsigned long ulStartPosition, unsigned long ulIndex);
typedef Exit_status (DD_DLL* _StartReadBinLog)(const char* pszLogName, unsigned long ulStartPosition, unsigned long ulIndex);
_BINLOG_PTR_DECL(StartReadBinLog);

DD_EXPORT void PrintfResult(char *msg);
typedef void (DD_DLL* _PrintfResult)(char *msg);
_BINLOG_PTR_DECL(PrintfResult);

#endif
