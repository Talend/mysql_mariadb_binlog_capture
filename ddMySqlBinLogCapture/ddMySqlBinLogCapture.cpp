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
/**
* MySQL / MariaDB Binlog capture
  date		10/05/2022
*
*/

#include "StdAfx.h"
#include <stdio.h>
#include <tchar.h>
#include <Psapi.h>
#include "sql.h"
#include "sqlext.h"

#include "CppSQLite3.h"
#include "MeasurementPrecision.h"	
#include "synchronization.h"
#include "MyLogFile.h"
#include "StdOutRedirect.h"
#include "ddMariaDB.h"
#include "ddBinlogLib.h"

/* Helpful conversion constants. */
static const double UNIT_PER_SECOND = 10000000;

#define SIZE_400				400
#define SIZE1024				1024
#define SIZE_2KO				2048
#define SIZE_4KO				4096 

#define NOT_EXIST				-1

#define COMPANY					"Talend"
#define PROJECT					"DataDistribution"
#define PROCESS_IDENTITY		"ddMySqlBinLogCapture"
#define SECTION_DDFOR			"DD_for_MySql"
#define ENTRY_PATH_SOURCE		"PathSource"
#define ENTRY_REPOSITORY		"Repository"
#define ENTRY_LISTEN_PORT		"Port"

#define STATUS_OK				0
#define STATUS_ERROR			1

#define	KEY_TRACE				"dwTrace"
#define	KEY_TRACE_LEVEL			"dwTraceLevel"
#define	KEY_TRACE_MODE			"dwTraceMode"
#define	KEY_TRACE_ARCHIVE		"dwTraceArchive"		// Max number of file trace
#define	KEY_TRACE_LOGREADER		"TraceLogReader"
#define	KEY_BUFFER_LINE			"BufferLine"
#define	KEY_TRACE_DURATION		"TraceDuration"
#define	KEY_BINLOG_LIB_NAME		"BinLogLibName"
#define	KEY_DB_DRIVER_NAME		"DbDriverName"
#define	KEY_TRACE_DEBUG			"TraceDebug"
#define	KEY_CAPTURE_PROCESS		"CaptureAndProcess" 
#define	KEY_FLUSH				"Flush"

#define	DEFAULT_TRACE_DEBUG		0

// Default values
#define	DEFAULT_TRACE			0
#define	DEFAULT_TRACE_LEVEL		0
#define	DEFAULT_TRACE_MODE		0
#define	DEFAULT_TRACE_ARCHIVE	10
#define	DEFAULT_MAX_TXT_LENGTH	1000
#define	DEF_TRACE_LOGREADER		0
#define	DEF_PERF				0
#define	DEFAULT_MAX_QUEUE		3
#define DEFAULT_BUFFER__LINE	33
#define	DEF_CAPTURE_PROCESS		0
#define	DEF_BINLOG_LIB_NAME		"ddBinlogLib"
#define	DEF_DB_DRIVER_NAME		"ddMySql"
#define	DEF_TRACE_DURATION		0
#define	DEF_FLUSH				0

#define TRACE_CREATE			0
#define TRACE_APPEND			1

#define TRACE_INIT			0	//Trace = 1
#define TRACE_FREE1			1	//Trace = 2
#define TRACE_LOOP			2	//Trace = 3
#define TRACE_ROW1			3	//Trace = 4
#define TRACE_ROW2			4	//Trace = 5
#define TRACE_COLUMN1		5	//Trace = 6
#define TRACE_COLUMN2		7	//Trace = 7

#define BUFFER_STDOUT			2*1024
#define LEN_BUF_WORK	512

#define BIN_LOG_HEADER_SIZE		4	// BinLog start position (first call must have this value)
#define BINLOG_SEP				'!'
#define LEN_CHARSET				32
#define LEN_SEQNO				20

#define		WORKING_AREA		"WORKING_AREA"

#define GET_PROC(name, type, var) if (fReturn){ var = (type) GetProcAddress(g_hLib, name);fReturn = var ? true:false;sprintf(szFuncName, name);}

typedef _Packed struct
{
	char			szBinLogName[50];	// Binlog name
	unsigned long	ulSize;				// Size of binlog
	unsigned long	ulPosition;			// Start position of binlog
} BinLogInfo;

typedef _Packed struct
{
	char			szBinLogName[50];		// Binlog name
	unsigned long	ulPosition;				// Start position of binlog.
	unsigned long	ulIndex;				// Index of statement into transaction
} BinLogSequence;

typedef _Packed struct
{
	unsigned long		ulCaptureNum;			// num last capture file processed
	unsigned long		ulPosition;				// position last entry processed

} CAPTURE_SEQUENCE;

struct StructCapture
{
	char szFullFileName[MAX_PATH];
	char szFileName[MAX_PATH];
	unsigned long ulSize;
	unsigned long ulNum;
	char szNum[11];
	struct StructCapture*	pStNext;	// Next pointer of StructCapture
};

MyLogFile g_LogFile;

_MySetVerbosity			hMySetVerbosity = NULL;
_MyInitDB				hMyInitDB = NULL;
_MyFreeDB				hMyFreeDB = NULL;
_MySetOptionDB			hMySetOptionDB = NULL;
_MyOpenConnection		hMyOpenConnection = NULL;
_MyCloseConnection		hMyCloseConnection = NULL;
_MyGetErrormessage		hMyGetErrormessage = NULL;
_MyGetErrorNum			hMyGetErrorNum = NULL;
_MyGetSqlState			hMyGetSqlState = NULL;
_MyFreeStatement		hMyFreeStatement = NULL;
_MyExecStatement		hMyExecStatement = NULL;
_MyGetResult			hMyGetResult = NULL;
_MyGetResultValue		hMyGetResultValue = NULL;
_MyGetFetchRow			hMyGetFetchRow = NULL;
_MyGetAffectedRows		hMyGetAffectedRows = NULL;
_MyGetInfo				hMyGetInfo = NULL;
_MyGetResultCount		hMyGetResultCount = NULL;
_MyGetNumField			hMyGetNumField = NULL;
_MyGetResultLength		hMyGetResultLength = NULL;
_MyGetResultField		hMyGetResultField = NULL;

static char				g_szTmp[2048];

// Threads variable
HANDLE					g_hThread_B;
DWORD					g_dwTID_B;
//Events variable
Event					g_hEvent_B;
Event					g_hEventExit;
Event					g_hEventEndOfBinlog;

StdOutRedirect			g_stdoutRedirect;
char*					g_pszLine;
char*					g_pszBufferStdOut;
FILE *					g_output_file = NULL; // Debug only
char*					g_pszLog = NULL;
char*					g_pszSavedLine;
static int				g_iLenBufferOut = BUFFER_STDOUT;
static int				g_iMaxBufferLine = DEFAULT_BUFFER__LINE * 1024;

// Variables connection Database
static char				g_szServer[SIZE_400];
static char				g_szInstance[SIZE_400];
static char				g_szSchema[SIZE_400];
static char				g_szDBpub[SIZE_400];
static char				g_szUser[100];
static char				g_szPassword[100];
static char				g_szPort[100];
// Global working buffer
static char				g_szbuffer_Tmp[SIZE_2KO * 2];
char*					g_pszbuffer_Tmp = &g_szbuffer_Tmp[0];
static char				g_szMsgError[SIZE1024];
static char				g_szErrorNumAlpha[SIZE_400];
static char				g_szSQL[SIZE1024];
static char				g_szJrnName[SIZE_400];
static char				g_szFormatMsg[1024];

HINSTANCE				g_hLib;

char*					g_hSocket = NULL;
void*					g_hDatabase;
static int				g_iPort = 3306;

static BinLogSequence	g_stBinLogSeq;
static BinLogSequence	g_stBinLogSeqInProgess;

static BinLogInfo*		g_pStBinLogInfo = NULL;
static BinLogInfo		g_StMasterStatus;
static BinLogInfo		g_StBinLogInfoFirst;
static BinLogInfo		g_StBinLogInfoLast;
static int				g_iNbBinLog;
static FILE*			g_pFileJrnTransact;

// Debug
static BinLogSequence	g_StBinLogSequenceXai;
static BinLogSequence	g_StBinLogSequenceStartRead;
static int				g_nSaveInDiskIndex = 0;
static char				g_cSaveInDiskThread = 'X';
static int				g_nBinLogBytesRead = 0;
static int				g_nBinLogStep = 0;
static int				g_nReadJournalStep1 = 0;
static int				g_nReadJournalStep2 = 0;
static int				g_nReadJournalBreak1 = 0;
static int				g_nReadJournalBreak2 = 0;

static char				g_szLastBufferWork[LEN_BUF_WORK + 1];
static char				g_szLastBufferReadJournal[1024 + 1];

static HANDLE			g_hThreadPipe;
static DWORD			g_dwTIDExit;
static HANDLE			g_hPipe;
static char				g_szPipeName[256];
static char				g_szModuleName[256];
bool					g_fExitApp = false;
static char				g_szMsgGlobal[SIZE_2KO];			// Use in main process
static char				g_szMsgThread[SIZE_4KO];	// Use in thread Pipe
static char				g_szMsgThreadB[SIZE_4KO];	// Use in thread B
static bool				g_fSelectRedoLog = false;

static double			g_dblMaxWriteEntry = 0; // Max time to execute WriteEntry function.
static char				g_szPathSource[SIZE_400];

// Variables connection Database SQLite3
static char				g_szFullDataBase[512];
static char				g_szDataBaseName[512];
static CppSQLite3DB		g_dbParam;

// Variables journal
static char				g_szJrnPath[_MAX_PATH];
static char				g_szWorkingArea[_MAX_PATH];
static char				g_szJrn_WithoutExtention[_MAX_PATH];
static long				g_lFlush = 0;
static FILETIME			g_StartFlush;
static FILETIME			g_EndFlush;


// Variables trace
long					g_lActivateTrace = DEFAULT_TRACE;
long					g_lTraceLevel = DEFAULT_TRACE_LEVEL;
static long				g_lTraceMode = DEFAULT_TRACE_MODE;
static long				g_lTraceArchive = DEFAULT_TRACE_ARCHIVE;
static long				g_lTraceDebug = DEFAULT_TRACE_DEBUG;	

static long				g_lTraceBinLog;
static CppSQLite3DB		g_db;
static CppSQLite3Query	g_query;
static CppSQLite3Buffer g_bufSQL;
static int				g_iRc;
static char				g_szDB[SIZE_400];

static char				g_szMySqlVersion[255];
static int				g_iMySQLVersion;

static FILE*			g_pDiskBufferTmpHandle = NULL;
static char				g_szDiskBufferTmpFileName[SIZE_400];
static char				g_sVersionRepository[11];
static int				g_iNumVersion;

static char				g_szBinlogLibName[512];
static char				g_szDbDriverName[512];

static char				g_szPathCapture[_MAX_PATH];
static char				g_szPathCaptureMemo[_MAX_PATH];
static FILE*			g_pFileCapture = NULL;
static long				g_lCaptureAndProcess = 0;
static unsigned long	g_ulCaptureSize = 0;
static unsigned long	g_ulCaptureCount = 0;
static unsigned long	g_ulCaptureNum = 1;
static unsigned long	g_ulCaptureMax = 0;

// structure for MeasurementPrecision
typedef _Packed struct
{
	unsigned long ulCountBinLog;
	unsigned long ulCountAllRow;
	unsigned long ulCountRowtable;
	unsigned long ulCountTransaction;
	unsigned long ulCountInsert;
	unsigned long ulCountUpdate;
	unsigned long ulCountDelete;
} StRowCount;

// structure for MeasurementPrecision
typedef struct
{
	char				StartDateTime[20];
	char				EndDateTime[20];
	BinLogSequence		StartBinLog;
	BinLogSequence		EndBinLog;
	double				DurationReadJournal;
	double				DurationLoadBinlog;
	double				DurationReadBinlog;
	double				DurationlistenBinLog;
	StRowCount			RowCount;
} StAllDuration;

static char					g_szPathDuration[256];
static long					g_lTraceDuration;
static StAllDuration		g_AllDuration;
static MeasurementPrecision g_MeasurementReadJournal;
static MeasurementPrecision g_MeasurementLoadBinLog;
static MeasurementPrecision g_MeasurementReadBinLog;
static MeasurementPrecision g_MeasurementlistenBinLog;

static bool g_fStartBinlog = false;
static bool g_fEndBinlog = false;
static bool g_fFirstBinlog = false;
static bool g_fLastBinlog = false;
static unsigned long g_ulLastBinlogTimeout = 0;
static double g_DurationLastBinlog = 0.0;
static double g_DurationLastBinlogMin = DBL_MAX;
static double g_DurationLastBinlogMax = 0;

static Lock g_LockDD_LOG;
static char g_dateTime[64];

static FILE*	g_hLog = NULL;
static FILE*	g_pFileDuration = NULL;
static FILE*	g_pFileTransaction = NULL;
static int		g_nbfile = 0;
static char		g_szNewFile[255];

const char *pszQUERY_JOURNAL = "SELECT JRNFILE FROM JOURNAL WHERE DB ='%s';";
const char *pszQUERY_BINLOG = "SHOW MASTER LOGS";
const char *pszQUERY_MASTER_STATUS = "SHOW MASTER STATUS";
const char *pszQUERY_VERSION = "SHOW VARIABLES LIKE 'version'";

DWORD GetLastSystemError(void);
bool ChangeJournal(const char *pszThread);

#include "ddBinlogdll.cpp"

/**
*
*/
DWORD GetLastSystemError(void)
{
	return GetLastError();
}

/**
*
*/
char* GetFormatMessage()
{
	DWORD dwError = GetLastSystemError();;
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		g_szFormatMsg,
		sizeof(g_szFormatMsg), 0);
	return g_szFormatMsg;
}

/**
*
*/
int FileExist(char * fullPathName)
{
	FILE	*pFile = fopen(fullPathName, "r");;
	if (pFile != NULL)
	{
		fclose(pFile);
		return 1;
	}
	else
		return -1;
}

/**
 * Gets File Size
 */
unsigned long ddGetFileSize(const char *pszFileName)
{
	WIN32_FIND_DATA FindData; // File information.
	unsigned long ulFileSize = 0;
	HANDLE hFind = FindFirstFile(pszFileName, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		//(FindData.nFileSizeHigh *(MAXDWORD+1)) + FindData.nFileSizeLow;
		ulFileSize = FindData.nFileSizeLow;
		FindClose(hFind);
		return ulFileSize;
	}
	else
		return 0;
}

/**
* Gets current datetime
*/
char* GetDateTime(char *destination)
{
	SYSTEMTIME	time;
	::GetLocalTime(&time);
	sprintf(destination, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	return destination;
}

/**
 * Gets current datetime
 */
char* GetDateTime()
{
	return GetDateTime(g_dateTime);
}


/**
 * Gets current datetime
 * The GetSystemTime function set FILETIME with the current system date and time.
 */
void GetSystemTime(FILETIME	*ftDateTime)
{
	SYSTEMTIME	stDateTime;
	::GetLocalTime(&stDateTime);
	SystemTimeToFileTime(&stDateTime, ftDateTime);
}

/**
 * Gets ElapsedTime
 * The ElapsedTime gets elapsed second between 2 FILETIME reference
 */
double ElapsedTime(FILETIME start, FILETIME end)
{
	LARGE_INTEGER liStart;
	LARGE_INTEGER liEnd;

	liStart.LowPart = start.dwLowDateTime;
	liStart.HighPart = start.dwHighDateTime;

	liEnd.LowPart = end.dwLowDateTime;
	liEnd.HighPart = end.dwHighDateTime;

	return ((liEnd.QuadPart - liStart.QuadPart) / UNIT_PER_SECOND);
}

/**
 *
 */
void GetModuleFilePath(char* out)
{
	char dir[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, dir, sizeof(dir));
	char* end = strrchr(dir, '\\');
	if (end) *end = '\0';
	strcpy(out, dir);
}

/**
 *
 */
void GetModuleFileName(char* out)
{
	char moduleName[MAX_PATH] = { 0 };
	GetModuleBaseName(GetCurrentProcess(), NULL, moduleName, sizeof(moduleName));
	char* end = strrchr(moduleName, '.');
	if (end) *end = '\0';
	strcpy(out, moduleName);
}

/**
*
*/
void init_dd_log()
{
	char logFileName[512];
	char fileName[512];
	char path[512];
	char *dot;
			
	GetModuleFilePath(path);
	GetModuleFileName(fileName);
		
	sprintf(logFileName, "%s\\%s.log", path, fileName);
	g_hLog = fopen(logFileName, "w");
	if (g_hLog)
	{
		fprintf(g_hLog, "%s:start:%s\n", GetDateTime(g_dateTime), logFileName);
		fflush(g_hLog);
	}
}

/**
*
*/
void free_dd_log()
{
	if (g_hLog)
	{
		fprintf(g_hLog, "%s:stop\n", GetDateTime(g_dateTime));
		fclose(g_hLog);
	}
}

/**
 *
 */
void dd_log(char *msg)
{
	g_LockDD_LOG.Enter();
	if (g_hLog)
	{
		fprintf(g_hLog, "%s:%s\n", GetDateTime(g_dateTime), msg);
		fflush(g_hLog);
	}
	g_LockDD_LOG.Exit();
}

/**
 * Copies the first size characters of source to destination.
 * if nullChar is true, a terminating null character is added at the end of destination.
 * @returns Returns � pointer to destination.
 */
char* ddstrncpy(char* destination, const char *source, int size, bool nullChar)
{
	strncpy(destination, source, size);
	if (nullChar) destination[size] = 0;
	return destination;
}

/**
 * Convert string to long integer.
 *
 * @returns Returns the converted number as a long.
 */
long ddatol(char *source, int size)
{
	char buf[128];
	strncpy(buf, source, size);
	buf[size] = 0;
	return atol(buf);
}

/**
 * Copies the first size characters of source to destination with rigth trim.
 * A terminating null character is added at the end of destination.
 */
void cpyrtrim(char *destination, const char* source, unsigned int size)
{
	int i = -1;
	memcpy(destination, source, size);
	if (size > 1)
	{
		for (i = size - 1; (i >= 0) && (destination[i] == ' '); i--);
		destination[i + 1] = 0;
	}
	else
	{
		if (destination[0] == ' ')
			destination[0] = 0;
		else
			destination[1] = 0;
	}
}

/**
 * tolowerStr
 *
 * Copies the string pointed by source into the array pointed by dest
 * Convert uppercase to lowercase
 * source must be � string with terminating null-characters
 * A terminating null character is added at the end of dest
 * if length of source >  iSizeDest only the iSizeDest-1 bytes will be copied
 *
 * @returns Returns pointer to destination.
 */
char *tolowerStr(char *dest, const char *source, int sizeDest)
{
	int i;
	int iLen = strlen(source);
	iLen = iLen < sizeDest ? iLen : sizeDest - 1;
	for (i = 0; *source && i < iLen; i++, source++) dest[i] = tolower(*source);
	dest[i] = 0;
	return dest;
}

/**
 * Locate first occurrence of character in source.
 * source  must be a string with terminating null-characters
 *
 * @returns Position to the first occurrence of character in source. If the character is not found, the function returns -1.
 */
int ddstrchr(const char *source, const char character)
{
	int iPos;
	char *tmp = (char*)strchr(source, character);
	if (tmp != NULL)
		iPos = tmp - source + 1;
	else
		iPos = -1;
	return iPos;
}

/**
 * Gets value from Windows registry
 */
bool GetRegistryValue(const char* pszSection, const char* pszKey, DWORD dwType, LPVOID pData, DWORD dwSize)
{
	bool fOK = false;
	HKEY  hKey;
	long  lResult;
	DWORD dwReadType;
	char  szRegistryEntry[512];

	sprintf(szRegistryEntry, "Software\\%s\\%s\\%s", COMPANY, PROJECT, pszSection);
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryEntry, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		lResult = ::RegQueryValueEx(hKey, pszKey, NULL, &dwReadType, (LPBYTE)pData, &dwSize);
		if (ERROR_SUCCESS == lResult)
		{
			if (dwType == dwReadType)
			{
				fOK = true;
			}
		}
		::RegCloseKey(hKey);
	}
	return fOK;
}

/**
 * Sets value to Windows registry
 */
bool SetRegistryValue(const char* pszSection, const char* pszKey, DWORD dwType, const void* pData, DWORD dwSize)
{
	bool fResult;
	HKEY  hKey;
	long  lResult;
	char  szRegistryEntry[512];

	sprintf(szRegistryEntry, "Software\\%s\\%s\\%s", COMPANY, PROJECT, pszSection);
	lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		szRegistryEntry,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		NULL);
	if (ERROR_SUCCESS != lResult)
	{
		fResult = false;
	}
	else
	{
		lResult = ::RegSetValueEx(hKey, pszKey, 0, dwType, (LPBYTE)pData, dwSize);
		if (ERROR_SUCCESS != lResult)
		{
			fResult = false;
		}
		else
			fResult = true;
		::RegCloseKey(hKey);
	}
	return fResult;
}

/**
 * Gets string value to Windows registry
 */
bool GetRegistryString(const char* pszSection, const char* pszKey, char* pszVal, unsigned long ulSize)
{
	return GetRegistryValue(pszSection, pszKey, REG_SZ, pszVal, ulSize);
}

/**
 * Sets long value to Windows registry
 */
bool GetRegistryLong(const char* pszSection, const char* pszKey, long *plVal)
{
	return GetRegistryValue(pszSection, pszKey, REG_DWORD, plVal, sizeof(*plVal));
}

/**
 * Read registry and create entry if not exist
 */
void ReadOrCreateRegistryLong(char *pszPath, char *pszKey, long lDefault, long  *plDestValue)
{
	if (GetRegistryValue(pszPath, pszKey, REG_DWORD, plDestValue, sizeof(*plDestValue)) == false)
	{
		bool fResult = SetRegistryValue(pszPath, pszKey, REG_DWORD, &lDefault, sizeof(lDefault));
		*plDestValue = lDefault;
		sprintf(g_szbuffer_Tmp, "Creation Key %s\\%s value:%ld %s", pszPath, pszKey, lDefault, fResult ? "OK" : "KO");
		dd_log(g_szbuffer_Tmp);
	}
}

/**
 * Read registry and create entry if not exist
 */
void ReadOrCreateRegistryString(char *pszPath, char *pszKey, char *pszDefault, char  *pszDestValue, unsigned long ulSize)
{
	memset(pszDestValue, 0, ulSize);
	if (GetRegistryValue(pszPath, pszKey, REG_SZ, pszDestValue, ulSize) == false)
	{
		bool fResult = SetRegistryValue(pszPath, pszKey, REG_SZ, pszDefault, strlen(pszDefault));
		strcpy(pszDestValue, pszDefault);
		sprintf(g_szbuffer_Tmp, "Creation Key %s\\%s value:[%s] %s", pszPath, pszKey, pszDefault, fResult ? "OK" : "KO");
		dd_log(g_szbuffer_Tmp);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// GetRepositoryFullPathName
//
// Return Repository full path name according entry 'Repository'
// If entry = Driver={SQLite3 ODBC Driver};Database=c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3 ==> return c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3
// If entry = Driver={SQLite3 ODBC Driver};Database=SFRDEV04@db2.db3 ==> return pszPathSource + \SFRDEV04@db2.db3
// If entry = c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3 ==> return c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3
// If entry = SFRDEV04@db2.db3 ==> return pszPathSource + \SFRDEV04@db2.db3																				
//////////////////////////////////////////////////////////////////////////////////
/**
 * GetRepositoryFullPathName
 *
 */
char* GetRepositoryFullPathName(const char *pszPathSource, const char *pszRepositoryEntry)
{
	static char szRepositoryFullPathName[1000];

	char szLower[512];
	char szPath[512];

	char *pszRepository = (char*)pszRepositoryEntry;

	int iLen = strlen(pszPathSource);
	strcpy(szPath, pszPathSource);
	if (szPath[iLen - 1] == '\\') szPath[iLen - 1] = 0;

	tolowerStr(szLower, pszRepository, 512);

	char *pszDriver = strstr(szLower, "driver");
	char *pszDataBase = strstr(szLower, "database");

	if (pszDriver && pszDataBase)
	{
		char *pszBackSlash = strrchr(pszDataBase, '\\');
		char *pszName = strrchr(pszDataBase, '=');
		if (pszBackSlash)
		{// case "Driver={SQLite3 ODBC Driver};Database=c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3"
			sprintf(szRepositoryFullPathName, "%s", ++pszName);
		}
		else
		{// case "Driver={SQLite3 ODBC Driver};Database=SFRDEV04@db2.db3"
			sprintf(szRepositoryFullPathName, "%s\\%s", szPath, ++pszName);
		}
	}
	else
	{
		char *pszBackSlash = (char*)strrchr(pszRepositoryEntry, '\\');
		if (pszBackSlash)
		{// case "c:\Data'Distribution\OpenEdge\SFRDEV04@db2.db3"
			sprintf(szRepositoryFullPathName, "%s", pszRepositoryEntry);
		}
		else
		{// case "SFRDEV04@db2.db3"
			sprintf(szRepositoryFullPathName, "%s\\%s", szPath, pszRepositoryEntry);
		}
	}
	return szRepositoryFullPathName;
}

/**
 *
 */
bool SendMessageToLogReader(char *pszProcessSender, int iMessage)
{
	HANDLE	hPipe = INVALID_HANDLE_VALUE;
	DWORD	dwError = 0;
	char	g_szPipeName[256];
	short	iAttempt = 0;

	sprintf(g_szPipeName, "\\\\.\\pipe\\%s", pszProcessSender);

	sprintf(g_szbuffer_Tmp, "+++ send <%d> to %s (%s)", iMessage, pszProcessSender, g_szPipeName);
	printf("%s\n", g_szbuffer_Tmp);

	while (true)
	{
		sprintf(g_szbuffer_Tmp, "+++ send <%d> to %s attempt %d", iMessage, pszProcessSender, iAttempt++);
		printf("%s\n", g_szbuffer_Tmp);

		hPipe = CreateFile((LPSTR)g_szPipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		dwError = GetLastSystemError();
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			sprintf(g_szbuffer_Tmp, "+++ connected to %s ", pszProcessSender);
			printf("%s\n", g_szbuffer_Tmp);
			break;
		}
		// If any error except the ERROR_PIPE_BUSY has occurred,we should return FALSE. 
		if (dwError != ERROR_PIPE_BUSY)
		{
			sprintf(g_szbuffer_Tmp, "+++ ERROR (%ld) send <%d> to %s ", dwError, iMessage, pszProcessSender);
			printf("%s\n", g_szbuffer_Tmp);
			return false;
		}
		// The named pipe is busy. Let�s wait for 2 seconds. 
		if (!WaitNamedPipe((LPSTR)g_szPipeName, 2000))
		{
			dwError = GetLastSystemError();
			sprintf(g_szbuffer_Tmp, "+++ ERROR WaitNamedPipe (%ld) send <%d> to %s ", dwError, iMessage, pszProcessSender);
			printf("%s\n", g_szbuffer_Tmp);
			return false;
		}
	}
	DWORD dwRead = 0;
	if (!(WriteFile(hPipe, (LPVOID)&iMessage, sizeof(int), &dwRead, 0)))
	{
		sprintf(g_szbuffer_Tmp, "+++  ERROR WriteFile  send <%d> to %s ", iMessage, pszProcessSender);
		printf("%s\n", g_szbuffer_Tmp);

		CloseHandle(hPipe);
		return false;
	}
	else
	{
		sprintf(g_szbuffer_Tmp, "+++  WriteFile send <%d> to %s OK", iMessage, pszProcessSender);
		printf("%s\n", g_szbuffer_Tmp);
	}

	CloseHandle(hPipe);
	Sleep(0);
	return true;
}

/**
 *
 */
bool LoadMySQLLib(char *pszLibName, char *pszMsg)
{
	char szFuncName[50];
	bool fReturn = true; // you must initialize this variable to true at first 
	char szMsg[1024];

	if ((g_hLib = LoadLibrary(pszLibName)) != NULL)
	{
		GET_PROC(NAME_MySetVerbosity, _MySetVerbosity, hMySetVerbosity)
			GET_PROC(NAME_MyInitDB, _MyInitDB, hMyInitDB)
			GET_PROC(NAME_MyFreeDB, _MyFreeDB, hMyFreeDB)
			GET_PROC(NAME_MySetOptionDB, _MySetOptionDB, hMySetOptionDB)
			GET_PROC(NAME_MyOpenConnection, _MyOpenConnection, hMyOpenConnection)
			GET_PROC(NAME_MyCloseConnection, _MyCloseConnection, hMyCloseConnection)
			GET_PROC(NAME_MyGetErrormessage, _MyGetErrormessage, hMyGetErrormessage)
			GET_PROC(NAME_MyGetErrorNum, _MyGetErrorNum, hMyGetErrorNum)
			GET_PROC(NAME_MyGetSqlState, _MyGetSqlState, hMyGetSqlState)
			GET_PROC(NAME_MyFreeStatement, _MyFreeStatement, hMyFreeStatement)
			GET_PROC(NAME_MyExecStatement, _MyExecStatement, hMyExecStatement)
			GET_PROC(NAME_MyGetResult, _MyGetResult, hMyGetResult)
			GET_PROC(NAME_MyGetResultValue, _MyGetResultValue, hMyGetResultValue)
			GET_PROC(NAME_MyGetFetchRow, _MyGetFetchRow, hMyGetFetchRow)
			GET_PROC(NAME_MyGetAffectedRows, _MyGetAffectedRows, hMyGetAffectedRows)
			GET_PROC(NAME_MyGetInfo, _MyGetInfo, hMyGetInfo)
			GET_PROC(NAME_MyGetResultCount, _MyGetResultCount, hMyGetResultCount)
			GET_PROC(NAME_MyGetNumField, _MyGetNumField, hMyGetNumField)
			GET_PROC(NAME_MyGetResultLength, _MyGetResultLength, hMyGetResultLength)
			GET_PROC(NAME_MyGetResultField, _MyGetResultField, hMyGetResultField)

			if (!fReturn)
			{
				if (pszMsg)
				{
					sprintf(pszMsg, "Loading function %s of %s failed", szFuncName, pszLibName);
				}
				else
				{
					sprintf(szMsg, "Loading function %s of %s failed", szFuncName, pszLibName);
					if (g_lActivateTrace != 0)	g_LogFile.Write(szMsg);
					dd_log(szMsg);
				}
			}
	}
	else
	{
		fReturn = false;
		DWORD dwError = GetLastSystemError();
		char szErrorMsg[1024];
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,                    // source
			dwError,           // code d'erreur
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			szErrorMsg,           // o� stocker la description  
			sizeof(szErrorMsg),  // taille minimale � retourner
			0);

		if (pszMsg)
		{
			sprintf(pszMsg, "%s (%ld -  %s)", pszLibName, dwError, szErrorMsg);
		}
		else
		{
			if (g_lActivateTrace != 0)	g_LogFile.Write(szMsg);
			dd_log(szMsg);
		}
	}
	return fReturn;
}


/**
 *
 */
void SaveDurationInDisk()
{
	if (g_lTraceDuration == 1)
	{
		if (g_pFileDuration == NULL)
		{
			sprintf(g_szPathDuration, "%s_mes.log", g_szJrnName);
			g_pFileDuration = fopen(g_szPathDuration, "a+b");
		}

		if (g_pFileDuration != NULL)
		{
			fseek(g_pFileDuration, 0, SEEK_CUR);
			fwrite((char*)&g_AllDuration, 1, sizeof(g_AllDuration), g_pFileDuration);
			fflush(g_pFileDuration);
		}
	}
}


/**
 *
 */
bool CheckEvent(Event *pEvent, char* pszName)
{
	if (*pEvent->Handle() == NULL)
	{
		sprintf(g_szMsgGlobal, "Creation Event %s failed", pszName);
		if (g_lActivateTrace != 0)	g_LogFile.Write(g_szMsgGlobal);
		dd_log(g_szMsgGlobal);
		return false;
	}
	else
	{
		if (g_lActivateTrace != 0)
		{
			sprintf(g_szMsgGlobal, "Creation Event %s successful", pszName);
			g_LogFile.Write(g_szMsgGlobal);
		}
		return true;
	}
	return true;
}

/**
 *
 */
bool GetMySQLVersion(const char *pszThread)
{
	bool fReturn = true;
	char szVersion[20];
	long lResult;
	MYSQL_ROW hRow = NULL;
	void* hQuery = NULL;
	char* hMsg = NULL;
	unsigned int uiMySqlError;
	char* hSqlState = NULL;

	*g_szErrorNumAlpha = 0;	// Clear error num
	*g_szMsgError = 0;		// Clear error msg

	memset(g_szMySqlVersion, 0, sizeof(g_szMySqlVersion));
	sprintf(g_szSQL, pszQUERY_VERSION);

	lResult = hMyExecStatement(g_hDatabase, g_szSQL);
	if (lResult != mySUCCESS)
	{
		lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
		lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
		lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
		sprintf(g_szMsgError, "Cannot get VERSION in database %s in server %s error <%s><%d><%d><%s>", g_szDBpub, g_szServer, "hMyExecStatement", uiMySqlError, *hSqlState, hMsg);
		sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
		fReturn = false;
	}
	else
	{
		lResult = hMyGetResult(g_hDatabase, &hQuery);
		if (lResult == mySUCCESS)
		{
			lResult = hMyGetFetchRow(hQuery, &hRow);
			if (lResult == mySUCCESS)
			{
				char *pszValue = NULL;
				lResult = hMyGetResultValue(hRow, 1, &pszValue);
				strcpy(g_szMySqlVersion, pszValue);
				memset(szVersion, 0, 20);
				char *pszLast = strrchr(g_szMySqlVersion, '-');
				if (pszLast)
				{
					for (char *s = g_szMySqlVersion, *d = szVersion; s != pszLast; s++) if (*s != '.' && *s != ' ') *d++ = *s;
					g_iMySQLVersion = atoi(szVersion);
					fReturn = true;
				}
				else
				{
					sprintf(g_szMsgError, "Cannot format VERSION %s", g_szMySqlVersion);
					fReturn = false;
				}
			}
		}
		else
		{
			lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
			lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
			lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
			sprintf(g_szMsgError, "Cannot get VERSION in database %s in server %s error <%s><%d><%d><%s>", g_szDBpub, g_szServer, "hMyGetResult", uiMySqlError, *hSqlState, hMsg);
			sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
			fReturn = false;
		}
		if (hQuery)
		{
			lResult = hMyFreeStatement(hQuery);
		}
	}
	return fReturn;
}


/**
 *
 */
DWORD WINAPI ListenPipeThread(LPVOID param)
{
	bool fContinue = true;

	sprintf(g_szMsgThread, "[T]:ListenPipeThread started");
	dd_log(g_szMsgThread);
	if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);

	while (fContinue)
	{

		sprintf(g_szMsgThread, "[T]:ListenPipeThread wait message");
		dd_log(g_szMsgThread);
		if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);

		BOOL  bResult = ConnectNamedPipe(g_hPipe, 0);
		DWORD dwError = GetLastSystemError();
		if (bResult || dwError == ERROR_PIPE_CONNECTED)
		{
			BYTE  buffer[sizeof(int)];
			DWORD dwRead = 0;
			int iMessage = 0;

			sprintf(g_szMsgThread, "[T]:ListenPipeThread read message");
			dd_log(g_szMsgThread);
			if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);

			if (!(ReadFile(g_hPipe, &buffer, sizeof(int), &dwRead, 0)))
			{
				dwError = GetLastSystemError();

				sprintf(g_szMsgThread, "[T]:ListenPipeThread ReadFile failed (%ld)", dwError);
				dd_log(g_szMsgThread);
				if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);
			}
			else
			{
				iMessage = *((int*)&buffer[0]);

				// The processing of the received data
				sprintf(g_szMsgThread, "[T]:ListenPipeThread ReadFile receive : %d", iMessage);
				dd_log(g_szMsgThread);
				if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);

				if (iMessage == 1 || iMessage == 2 || iMessage == 3)
				{
					g_fExitApp = true;
					fContinue = false;

					if (!g_hEventExit.Set())
					{
						sprintf(g_szMsgThread, "[T]:ListenPipeThread SetEvent Exit failed");
						dd_log(g_szMsgThread);
					}
					if (!g_hEventEndOfBinlog.Set())
					{
						sprintf(g_szMsgThread, "[T]:ListenPipeThread SetEvent EndOfBinlog failed");
						dd_log(g_szMsgThread);
					}
				}
			}
			DisconnectNamedPipe(g_hPipe);
		}
		else
		{
			sprintf(g_szMsgThread, "[T]:ConnectNamedPipe failed (%ld)", dwError);
			dd_log(g_szMsgThread);
			if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);
			break;
		}
		Sleep(0);
	}
	sprintf(g_szMsgThread, "[T]:ListenExitThread terminated");
	dd_log(g_szMsgThread);
	if (g_lActivateTrace != 0) g_LogFile.Write("[T]:%s", g_szMsgThread);

	return 0;
}

/**
 *
 */
void InitThreadPipe()
{
	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (!InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
	{
		DWORD err = GetLastSystemError();
		sprintf(g_szMsgGlobal, "InitializeSecurityDescriptor failed (%ld)", err);
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace != 0) g_LogFile.Write(g_szMsgGlobal);
	}
	if (!SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, (PACL)0, FALSE))
	{
		DWORD err = GetLastSystemError();
		sprintf(g_szMsgGlobal, "SetSecurityDescriptorDacl failed (%ld)", err);
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace != 0) g_LogFile.Write(g_szMsgGlobal);
	}
	sa.nLength = sizeof sa;
	sa.bInheritHandle = TRUE;

	// Build pipe name
	GetModuleBaseName(GetCurrentProcess(), NULL, g_szModuleName, sizeof(g_szModuleName));
	sprintf(g_szPipeName, "\\\\.\\pipe\\%s", g_szModuleName);

	g_hPipe = ::CreateNamedPipe((LPSTR)g_szPipeName, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
		sizeof(int), sizeof(int), NMPWAIT_USE_DEFAULT_WAIT, &sa);
	if (g_hPipe == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastSystemError();
		sprintf(g_szMsgGlobal, "[M]:CreateNamedPipe <%s> failed (%ld)", g_szPipeName, dwError);
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace != 0) g_LogFile.Write(g_szMsgGlobal);
	}
	else
	{
		sprintf(g_szMsgGlobal, "[M]:CreateNamedPipe <%s> successful", g_szPipeName);
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace != 0) g_LogFile.Write(g_szMsgGlobal);

		g_hThreadPipe = CreateThread(NULL, 0, ListenPipeThread, NULL, 0, &g_dwTIDExit);

		sprintf(g_szMsgGlobal, "[M]:CreateThread <%s> %s ", "ListenPipeThread", (g_hThreadPipe == NULL) ? "failed" : "successful");
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace != 0) g_LogFile.Write(g_szMsgGlobal);
	}
}

/**
 * SearchArg
 * Extract Instance, Schema, user and password from argument aplication
 */
bool SearchArg(char *pszUser, char * pszPassword, char* pszArgument)
{
	bool fReturn = true;
	int	i;

	*g_szMsgError = 0;		// Clear error msg

	g_pszbuffer_Tmp = g_szbuffer_Tmp;

	memset(g_pszbuffer_Tmp, 0, sizeof(g_szbuffer_Tmp));
	cpyrtrim(g_szbuffer_Tmp, pszArgument, SIZE1024);

	i = ddstrchr(g_pszbuffer_Tmp, '/');
	if (i > 1)
	{
		memcpy(g_szInstance, g_pszbuffer_Tmp, i - 1);
		if ((i + 1) < (int)strlen(g_pszbuffer_Tmp))
		{
			g_pszbuffer_Tmp += (i);
			i = ddstrchr(g_pszbuffer_Tmp, '/');
			if (i > 1)
			{
				memcpy(g_szSchema, g_pszbuffer_Tmp, i - 1);
				if ((i + 1) < (int)strlen(g_pszbuffer_Tmp))
				{
					g_pszbuffer_Tmp += (i);
					i = ddstrchr(g_pszbuffer_Tmp, '/');
					if (i > 1)
					{
						memcpy(pszUser, g_pszbuffer_Tmp, i - 1);
						if ((i + 1) < (int)strlen(g_pszbuffer_Tmp))
						{
							g_pszbuffer_Tmp += (i);
							sprintf(pszPassword, g_pszbuffer_Tmp);
						}
						else
						{
							*pszPassword = 0;
						}
						fReturn = true;
					}
					else
					{
						*pszUser = 0;
						*pszPassword = 0;
						fReturn = true;
					}
				}
				else
				{
					*pszUser = 0;
					*pszPassword = 0;
					fReturn = true;
				}
			}
			else
			{
				sprintf(g_szMsgError, "schema name not found in command line");
				fReturn = false;
			}
		}
		else
		{
			sprintf(g_szMsgError, "Instance name not found in command line");
			fReturn = false;
		}
	}
	else
	{
		sprintf(g_szMsgError, "Instance name not found in command line");
		fReturn = false;
	}
	return (fReturn);
}

/**
 *
 */
void CloseConnection()
{
	hMyCloseConnection(g_hDatabase);
}

/**
 *
 */
bool OpenConnection(void **hDb, char** hSoc, char *pszUser, char * pszPassword)
{
	bool fReturn = true;
	long lResult;
	char* hMsg = NULL;
	unsigned int uiMySqlError;
	char* hSqlState = NULL;

	lResult = hMyInitDB(hDb);

	while (true)
	{
		if (lResult != mySUCCESS)
		{
			sprintf(g_szMsgError, "Failed on hMyInitDB.");
			fReturn = false;
			break;
		}

		lResult = hMyOpenConnection(*hDb, g_szServer, pszUser, pszPassword, g_iPort, true, "TCP", hSoc);
		if (lResult != mySUCCESS)
		{
			lResult = hMyGetErrormessage(*hDb, &hMsg);
			lResult = hMyGetErrorNum(*hDb, &uiMySqlError);
			lResult = hMyGetSqlState(*hDb, &hSqlState);
			sprintf(g_szMsgError, "Failed on hMyOpenConnection [%d][%d][%s]", uiMySqlError, *hSqlState, hMsg);
			sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);

			fReturn = false;
		}
		break;
	}
	if (!fReturn)
	{
		hMyCloseConnection(g_hDatabase);
	}
	return fReturn;
}

/**
 *
 */
bool SearchJrnPath()
{
	bool fReturn = true;
	const char* pszJournal;
	int iRc;

	*g_szErrorNumAlpha = 0;	// Clear error num
	*g_szMsgError = 0;		// Clear error msg
	*g_szJrnName = 0;

	try
	{
		printf("SearchJrnPath\n");

		strcpy(g_szFullDataBase, GetRepositoryFullPathName(g_szPathSource, g_szDataBaseName));

		g_dbParam.open(g_szFullDataBase);
		iRc = g_dbParam.execDML("PRAGMA journag_mode = 'OFF'");
		iRc = g_dbParam.execDML("PRAGMA synchronous = 'OFF'");
		iRc = g_dbParam.execDML("PRAGMA cache_size = 10000;");
		iRc = g_dbParam.execDML("PRAGMA legacy_file_format = 0;");


		sprintf(g_szSQL, pszQUERY_JOURNAL, g_szSchema);
		printf("SearchJrnPath [%s]\n", g_szSQL);

		CppSQLite3Statement stmt = g_dbParam.compileStatement(g_szSQL);
		CppSQLite3Query query = stmt.execQuery();
		if (query.eof())
		{
			sprintf(g_szMsgError, "The table DataDistribution_Journal was not found into the database %s ", g_szFullDataBase);
			printf("SearchJrnPath [%s]\n", g_szMsgError);
			fReturn = false;
		}
		else
		{
			pszJournal = query.fieldValue(0);
			printf("SearchJrnPath [%s]\n", pszJournal);
			cpyrtrim(g_szJrnName, pszJournal, strlen(pszJournal));
			sprintf(g_szDiskBufferTmpFileName, "%s.TMP", g_szJrnName);
		}
		printf("SearchJrnPath fReturn:%d\n", fReturn);
	}
	catch (CppSQLite3Exception& e)
	{
		printf("SearchJrnPath exception <%s>\n", e.errorMessage());
		sprintf(g_szMsgError, "Error %s : %s", g_szFullDataBase, e.errorMessage());
		sprintf(g_szErrorNumAlpha, "%d", e.errorCode());
		fReturn = false;
	}
	printf("SearchJrnPath fReturn:%d ENDS\n", fReturn);
	return fReturn;
}


/**
 *
 */
bool LoadParam()
{
	bool fReturn = true;
	char szGlobalSection[255];
	long lport = 0;
	long lVar = -1;

	*g_szErrorNumAlpha = 0;// Clear error num
	*g_szMsgError = 0; // Clear error msg

	sprintf(szGlobalSection, "%s\\", SECTION_DDFOR);
	// Search full database path
	if (GetRegistryString(szGlobalSection, ENTRY_PATH_SOURCE, g_szPathSource, sizeof(g_szPathSource)) == false)
	{
		fReturn = false;
		sprintf(g_szMsgError, "The entry HKEY_LOCAL_MACHINE\\SOFTWARE\\%s\\%s\\%s\\%s is missing", COMPANY, PROJECT, szGlobalSection, ENTRY_PATH_SOURCE);
		dd_log(g_szMsgError);
	}
	sprintf(szGlobalSection, "%s\\%s\\", SECTION_DDFOR, g_szInstance);

	// Search database name
	if (GetRegistryString(szGlobalSection, ENTRY_REPOSITORY, g_szDataBaseName, sizeof(g_szDataBaseName)) == false)
	{
		fReturn = false;
		sprintf(g_szMsgError, "The entry HKEY_LOCAL_MACHINE\\SOFTWARE\\%s\\%s\\%s\\%s is missing", COMPANY, PROJECT, szGlobalSection, ENTRY_REPOSITORY);
		dd_log(g_szMsgError);
	}

	if (GetRegistryString(&szGlobalSection[0], ENTRY_LISTEN_PORT, g_szPort, sizeof(g_szPort)) == false)
	{
		fReturn = false;
		sprintf(g_szMsgError, "[M]:The entry HKEY_LOCAL_MACHINE\\SOFTWARE\\%s\\%s\\%s\\%s is missing", COMPANY, PROJECT, szGlobalSection, ENTRY_LISTEN_PORT);
		dd_log(g_szMsgError);
		g_iPort = 0;
	}
	else
		g_iPort = atoi(g_szPort);

	char *pszServerName = strchr(g_szInstance, '@');
	memset(g_szServer, 0, sizeof(g_szServer));
	memcpy(g_szServer, g_szInstance, pszServerName - g_szInstance);
	strcpy(g_szDBpub, g_szSchema);

	sprintf(g_szMsgError, "[M]:Instance:<%s> Server:<%s> Schema:<%s> DB:<%s> PathSource:<%s> DataBaseName:<%s> User:<%s>", g_szInstance, g_szServer, g_szSchema, g_szDBpub, g_szPathSource, g_szDataBaseName, g_szUser);
	dd_log(g_szMsgError);

	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE, DEFAULT_TRACE, &g_lActivateTrace);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_LEVEL, DEFAULT_TRACE_LEVEL, &g_lTraceLevel);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_MODE, DEFAULT_TRACE_MODE, &g_lTraceMode);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_ARCHIVE, DEFAULT_TRACE_ARCHIVE, &g_lTraceArchive);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_LOGREADER, 0, &g_lTraceBinLog);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_DURATION, 0, &g_lTraceDuration);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_TRACE_DEBUG, DEFAULT_TRACE_DEBUG, &g_lTraceDebug);
	ReadOrCreateRegistryString(szGlobalSection, KEY_BINLOG_LIB_NAME, DEF_BINLOG_LIB_NAME, g_szBinlogLibName, sizeof(g_szBinlogLibName));
	ReadOrCreateRegistryString(szGlobalSection, KEY_DB_DRIVER_NAME, DEF_DB_DRIVER_NAME, g_szDbDriverName, sizeof(g_szDbDriverName));
	ReadOrCreateRegistryLong(szGlobalSection, KEY_BUFFER_LINE, DEFAULT_BUFFER__LINE, &lVar);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_CAPTURE_PROCESS, DEF_CAPTURE_PROCESS, &g_lCaptureAndProcess);
	ReadOrCreateRegistryLong(szGlobalSection, KEY_FLUSH, DEF_FLUSH, &g_lFlush);

	g_iMaxBufferLine = (int)lVar * 1024;

	sprintf(g_szTmp, "[M]:TraceLogReader:%d MaxBufferLine:%d", g_lTraceBinLog, g_iMaxBufferLine);
	dd_log(g_szTmp);

	g_pszBufferStdOut = (char*)malloc(g_iMaxBufferLine);
	if (!g_pszBufferStdOut)
	{
		sprintf(g_szTmp, "[M]:ERROR allocate BufferLine:%d", g_iMaxBufferLine);
		dd_log(g_szTmp);
		fReturn = false;
	}
	g_pszLine = (char*)malloc(g_iMaxBufferLine);
	if (!g_pszLine)
	{
		sprintf(g_szTmp, "[M]:ERROR allocate BufferLine Tmp:%d", g_iMaxBufferLine);
		dd_log(g_szTmp);
		fReturn = false;
	}

	g_pszSavedLine = (char*)malloc(g_iLenBufferOut);
	if (!g_pszSavedLine)
	{
		sprintf(g_szTmp, "[M]:ERROR allocate SavedLine Tmp:%d", g_iLenBufferOut);
		dd_log(g_szTmp);
		fReturn = false;
	}

	return fReturn;
}

/**
 *
 */
bool InitParam()
{
	bool fReturn = true;
	char szJrnTransact[SIZE_400];

	//init  la structure
	memset((char*)&g_stBinLogSeq, 0, sizeof(g_stBinLogSeq));

	sprintf(szJrnTransact, "%s.XAI", g_szJrnName);
	// si le fichier n'existe pas, alors cree (r+b)
	if (FileExist(szJrnTransact) == NOT_EXIST)
		g_pFileJrnTransact = fopen(szJrnTransact, "w+b"); // ouverture pour ecriture avec suppresssion de contenu ( creation auto si n'existe pas)
	else
	{
		g_pFileJrnTransact = fopen(szJrnTransact, "r+b"); // ouverture
		if (g_pFileJrnTransact != NULL)
		{
			fseek(g_pFileJrnTransact, 0, SEEK_SET);
			int nRead = fread(&g_stBinLogSeq, 1, sizeof(g_stBinLogSeq), g_pFileJrnTransact);
			if (g_lActivateTrace && g_lTraceLevel > TRACE_INIT) g_LogFile.Write("[M]:InitParam read:%ld", nRead);
		}
	}

	sprintf(g_szbuffer_Tmp, "[M]:InitParam XAI BinLog:<%s> Pos:%ld Idx:%ld", g_stBinLogSeq.szBinLogName, g_stBinLogSeq.ulPosition, g_stBinLogSeq.ulIndex);
	dd_log(g_szbuffer_Tmp);
	if (g_lActivateTrace && g_lTraceLevel > TRACE_INIT) g_LogFile.Write(g_szbuffer_Tmp);
	if (!g_pFileJrnTransact)	fReturn = false;

	return fReturn;
}

/**
 *
 */
void SaveInDisk(const char *pszThread, int idx, BinLogSequence* pBinLogSeq)
{
	if (g_pFileJrnTransact != NULL)
	{
		memcpy(&g_StBinLogSequenceXai, pBinLogSeq, sizeof(BinLogSequence));
		g_nSaveInDiskIndex = idx;
		g_cSaveInDiskThread = *pszThread;
		fseek(g_pFileJrnTransact, 0, SEEK_SET);
		fwrite((char*)pBinLogSeq, 1, sizeof(BinLogSequence), g_pFileJrnTransact);
		fflush(g_pFileJrnTransact);
		if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1)  g_LogFile.Write("[%s]:SaveInDisk(%d) Binlog:<%s> Pos:<%lu> Idx:<%lu>", pszThread, idx, pBinLogSeq->szBinLogName, pBinLogSeq->ulPosition, pBinLogSeq->ulIndex);
	}
}

/**
 *
 */
void WriteOutput(char* pszFormat, ...)
{
	if (!g_output_file) return;
	if (!g_pszLog) return;
	va_list argList;
	va_start(argList, pszFormat);
	vsprintf(g_pszLog, pszFormat, argList);
	va_end(argList);
	fputs(g_pszLog, g_output_file);
	fflush(g_output_file);
}

 /**
 *
*/
void TraceBinLogDataContents(const char *pszThread, unsigned ulLine, char *pszData)
{
	g_db.execDML("begin transaction;");
	try
	{
		g_bufSQL.format("insert or ignore into CONTENTS (Line, Info) values(%d, '%q');", ulLine, pszData);
		g_iRc = g_db.execDML(g_bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		sprintf(g_szbuffer_Tmp, "Error insert db(CONTENTS) : error:%i %s", e.errorCode(), e.errorMessage());
		if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[%s]:TraceBinLogData:%s", pszThread, g_szbuffer_Tmp);
	}
	g_db.execDML("commit transaction;");
}

/**
 *
 */
void TraceBinLogDataEvents(const char *pszThread, char *pszLogName, unsigned ulLine, char*pszEventType, unsigned long ulPosition, unsigned long ulEndPosition, unsigned long ulIndex, char *pszXid, char *pszTableName, char *pszCommand)
{
	g_db.execDML("begin transaction;");
	try
	{
		g_bufSQL.format("insert or ignore into EVENTS (LogName, Line, EventType, Position, EndPosition, Idx, Xid, TableName, Command) values('%s', %d, '%s', %d, %d, %d, '%s', '%s', '%q');",
			pszLogName, ulLine, pszEventType, ulPosition, ulEndPosition, ulIndex, pszXid, pszTableName, pszCommand);
		g_iRc = g_db.execDML(g_bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		sprintf(g_szbuffer_Tmp, "Error insert db(EVENTS) : error:%i %s", e.errorCode(), e.errorMessage());
		//dd_log(g_szbuffer_Tmp);
		if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[%s]:TraceBinLogData:%s", pszThread, g_szbuffer_Tmp);
	}
	g_db.execDML("commit transaction;");
}


/**
 *
 */
void CloseDiskBufferTmp()
{
	if (g_pDiskBufferTmpHandle)
	{
		fclose(g_pDiskBufferTmpHandle);
		DeleteFile(g_szDiskBufferTmpFileName);
		g_pDiskBufferTmpHandle = NULL;
	}
}

/**
 *
 */
bool OpenDiskBufferTmp()
{
	CloseDiskBufferTmp();
	// si le fichier existe , alors delete
	if (FileExist(g_szDiskBufferTmpFileName) != NOT_EXIST) DeleteFile(g_szDiskBufferTmpFileName);
	g_pDiskBufferTmpHandle = fopen(g_szDiskBufferTmpFileName, "w+b");
	return g_pDiskBufferTmpHandle ? true : false;
}


/**
 *
 */
DWORD WINAPI listenBinLogThread(LPVOID param)
{
	MeasurementPrecision Measurement;
	char cZero = 0;
	int nBytesRead = 0;
	char *pszBufferWork = NULL;
	char *pszBufferStart = g_pszBufferStdOut;
	char *pszStartLine = g_pszBufferStdOut;
	char *pszEndLine;
	bool bStartOfStmt = false;
	bool bQueryInProgress = false;
	bool bDataColumnInProgress = false;
	bool bQueryFound = false;
	bool bLOBpresent = false;
	bool bWaitingLOB = false;
	char *pszEnd;
	char *pszDot;

	char szJournalEntry[4];
	char szBinLogName[500];
	char szTableName[500];
	char szSchemaName[500];
	char szXid[50];
	unsigned long ulPosition = 0;
	unsigned long ulEndPosition = 0;
	unsigned long ulIndex = 0;
	short iNumField = 0;
	short iCountField = 0;
	short iOffsetCmd = 0;
	long lLenLob = 0;
	long lLenTmpFile = 0;
	long lLenTmpRead = 0;
	long lLenTmp = 0;

	unsigned long ulCountBinLog = 0;
	unsigned long ulCountStatement = 0;
	unsigned long ulCountTransaction = 0;
	unsigned long ulCountTotalStatement = 0;
	unsigned long ulCountTotalTransaction = 0;
	unsigned long ulCountLine = 0;
	unsigned long ulCountInsert = 0;
	unsigned long ulCountUpdate = 0;
	unsigned long ulCountDelete = 0;

	char szSeqNo[LEN_SEQNO + 1];
	int iLenEntry = 0;
	int nWritten;
	bool fLob = false;
			
	try
	{
		g_LogFile.Write("[B]:listenBinLogThread[01] Binlog:%d Line:%d started...", ulCountBinLog, ulCountLine);
		strcpy(szSeqNo, "0");

		GetSystemTime(&g_StartFlush);

		while (1)
		{
			DWORD dwEvent = g_hEvent_B.Wait(100);

			if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[02] Binlog:%d Line:%d stdoutRedirect.GetBuffer(a) : Len:%d...",
				ulCountBinLog, ulCountLine, g_iLenBufferOut);

			//-----------------------------------------------------------------------
			nBytesRead = g_stdoutRedirect.GetBuffer(pszBufferStart, g_iLenBufferOut);
			//-----------------------------------------------------------------------

			// Debug
			g_nBinLogBytesRead = nBytesRead;
			g_nBinLogStep++;

			if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[03] Binlog:%d Line:%d stdoutRedirect.GetBuffer(a) nBytesRead:%d Error:%d",
				ulCountBinLog, ulCountLine, nBytesRead, nBytesRead < 0 ? g_stdoutRedirect.GetError() : 0);

			while (nBytesRead > 0)
			{
				if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2) 
					g_LogFile.Write("[B]:listenBinLogThread[04] Binlog:%d Line:%d nBytesRead:%d IdxBuf:%d LenBufferStart:<%d> LenStartLine:<%d>",
							ulCountBinLog, ulCountLine, nBytesRead, pszBufferStart - g_pszBufferStdOut, strlen(pszBufferStart), strlen(pszStartLine));				

				if (g_lTraceBinLog) WriteOutput("%s", pszBufferStart);

				pszBufferWork = pszBufferStart;
				while (*pszBufferWork)
				{
					if (*pszBufferWork == '\n')
					{
						iLenEntry = (pszBufferWork - pszStartLine) + 1;
						if ((nWritten = fwrite(pszStartLine, sizeof(char), iLenEntry, g_pFileCapture)) == iLenEntry)
						{
							if (g_lFlush != 0) fflush(g_pFileCapture);
							g_ulCaptureSize += iLenEntry;

							GetSystemTime(&g_EndFlush);
							if (ElapsedTime(g_StartFlush, g_EndFlush) >= 10.0)
							{
								fflush(g_pFileCapture);
								GetSystemTime(&g_StartFlush);
								if (g_lActivateTrace && g_lTraceLevel > 5)
									g_LogFile.Write("[B]:listenBinLogThread Flush [%s]", g_szPathCaptureMemo);
							}
						}
						else
						{
							sprintf(g_szMsgGlobal, "listenBinLogThread error write capture [%s] [%d] [%s]", g_szPathCaptureMemo, ferror(g_pFileCapture), GetFormatMessage());
							if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", "B", g_szMsgGlobal);
							dd_log(g_szMsgGlobal);
							exit(0);
						}
						
						if (g_ulCaptureSize > g_ulCaptureMax)
						{
							if (!ChangeJournal("B"))
							{
								sprintf(g_szMsgGlobal, "listenBinLogThread error change journal capture [%s]", g_szPathCaptureMemo);
								if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", "B", g_szMsgGlobal);
								dd_log(g_szMsgGlobal);
								exit(0);
							}
						}

						if (fLob)
						{// LOB in progress 

							// Save data after '\n' to buffer tmp
							strcpy(g_pszSavedLine, pszBufferWork + 1);
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
								g_LogFile.Write("[B]:listenBinLogThread[06] Binlog:%d Line:%d SAVE LINE <%d><%s>", ulCountBinLog, ulCountLine, strlen(g_pszSavedLine), g_pszSavedLine);

							// Reload start line from backup
							strcpy(g_pszBufferStdOut, g_pszLine);							
							lLenTmp = strlen(g_pszLine);
							pszBufferWork = &g_pszBufferStdOut[lLenTmp];
							
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
								g_LogFile.Write("[B]:listenBinLogThread[07] Binlog:%d Line:%d RELOAD (%d, %d) LenLob:%ld LenTmpFile:%ld LenTmpRead:%ld <%s>", ulCountBinLog, ulCountLine, strlen(pszStartLine), (pszBufferWork - pszStartLine), lLenLob, lLenTmpFile, lLenTmpRead, pszStartLine);
						}

						*pszBufferWork = 0;
						
						pszEndLine = pszBufferWork;	//Mark end of line
						if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
						{
							g_LogFile.Write("[B]:listenBinLogThread[08] Binlog:%d Line:%d LenStartLine:%d %.100s", ulCountBinLog, ulCountLine, strlen(pszStartLine), pszStartLine);
						}
						if (g_lTraceBinLog) TraceBinLogDataContents("B", ulCountLine, pszStartLine);

						char *pszLineWork = pszStartLine;
						int iLenLine = strlen(pszLineWork);
						strncpy(g_szLastBufferWork, pszLineWork, iLenLine < LEN_BUF_WORK ? iLenLine : LEN_BUF_WORK);
						g_szLastBufferWork[iLenLine < 1024 ? iLenLine : LEN_BUF_WORK] = 0;

						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						// End of DATA_COLUMN for query 
						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						if (bDataColumnInProgress && strncmp(pszStartLine, "###   @", 7) != 0)
						{// DATA_COLUMN completed ==> process row

							// If RU(B) leave flag query in progress 
							if (szJournalEntry[2] != 'B') bQueryInProgress = false;
							bDataColumnInProgress = false;

							SaveInDisk("B", 1, &g_stBinLogSeqInProgess);

						}// End of if (bDataColumnInProgress && strncmp(pszStartLine, "###   @", 7) != 0)

						////////////////////////////////////////////////////////////////////////////////////////////
						// BEGIN and STMT tag previously detected ==> START OF QUERY 
						////////////////////////////////////////////////////////////////////////////////////////////
						if (bStartOfStmt && strncmp(pszStartLine, "###", 3) == 0)
						{// BEGIN and STMT tag detected ==> scan for query
							bQueryFound = false;

							// Query in progress ==> process DATA_COLUMN  
							if (bQueryInProgress && strncmp(pszStartLine, "###   @", 7) == 0)
							{	// ###   @1=14
								//... 
								//###   @10='string:tynytext'
								pszLineWork += 7;//skip command
								pszEnd = strchr(pszLineWork, '=');
								iNumField = (short)ddatol(pszLineWork, pszEnd - pszLineWork);
								iCountField++;
								pszLineWork = pszEnd + 1; //skip '='

								if (g_lActivateTrace && g_lTraceLevel > TRACE_COLUMN1)
									g_LogFile.Write("[B]:listenBinLogThread[34] Binlog:%d Line:%d entry:<%s> Table:<%s> NumField:%03d CountField:%03d",
										ulCountBinLog, ulCountLine, szJournalEntry, szTableName, iNumField, iCountField);

								if (fLob)
								{
									strcpy(g_pszBufferStdOut, g_pszSavedLine);
									pszBufferWork = g_pszBufferStdOut - 1; // buffer will be incremented at the end of process line 
								}
								fLob = false;

							}
							else if (bQueryInProgress && strncmp(pszStartLine, "### WHERE", 9) == 0)
							{// start of data for update(RUB) and delete
								bDataColumnInProgress = true;
								iCountField = 0;
								if (szJournalEntry[1] == 'U') szJournalEntry[2] = 'B';
							}
							else if (bQueryInProgress && strncmp(pszStartLine, "### SET", 7) == 0)
							{// start of data for insert and update(RUP) 
								bDataColumnInProgress = true;
								if (szJournalEntry[1] == 'U') szJournalEntry[2] = 'P';

							}
							else if (strncmp(pszStartLine, "### INSERT", 10) == 0)
							{//'### INSERT INTO dd_test.mysqg_alg_types'
								strcpy(szJournalEntry, "RPT");
								iOffsetCmd = 16;
								bQueryFound = true;
								ulCountInsert = 1;
							}
							else if (strncmp(pszStartLine, "### UPDATE", 10) == 0)
							{//'### UPDATE dd_test.mysqg_alg_types'
								strcpy(szJournalEntry, "RUX");
								iOffsetCmd = 11;
								bQueryFound = true;
								ulCountUpdate = 1;
							}
							else if (strncmp(pszStartLine, "### DELETE", 10) == 0)
							{//'### DELETE FROM dd_test.mysqg_alg_types'
								strcpy(szJournalEntry, "RDL");
								iOffsetCmd = 16;
								bQueryFound = true;
								ulCountDelete = 1;
							}

							if (bQueryFound)
							{
								g_AllDuration.RowCount.ulCountAllRow++;

								bQueryInProgress = bDataColumnInProgress = false;
								pszLineWork += iOffsetCmd;			//skip command
								pszDot = strchr(pszLineWork, '.');	// remove database name
								strcpy(szTableName, pszDot ? pszDot + 1 : pszLineWork);

								if (pszDot)
								{
									strncpy(szSchemaName, pszLineWork, pszDot - pszLineWork);
									szSchemaName[pszDot - pszLineWork] = 0;
								}
								else
								{
									strcpy(szSchemaName, "");
								}

								if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
									g_LogFile.Write("[B]:listenBinLogThread[37] Binlog:%d Line:%d n:%d db:<%s> gdb:<%s> cmp:%d tbl:<%s> line:<%s> dot:<%s>",
										ulCountBinLog, ulCountLine, pszDot - pszLineWork, szSchemaName, g_szSchema, strcmp(szSchemaName, g_szSchema), szTableName, pszLineWork, pszDot);

								// Incremente Index 
								ulIndex++;
								if (
									(
										ulCountBinLog > 0 ||
										(ulCountBinLog == 0 && ulPosition != g_stBinLogSeq.ulPosition) ||
										(ulCountBinLog == 0 && ulPosition == g_stBinLogSeq.ulPosition &&  ulIndex > g_stBinLogSeq.ulIndex)
										)
									)
								{
									g_AllDuration.RowCount.ulCountRowtable++;
									g_AllDuration.RowCount.ulCountInsert += ulCountInsert;
									g_AllDuration.RowCount.ulCountUpdate += ulCountUpdate;
									g_AllDuration.RowCount.ulCountDelete += ulCountDelete;

									ulCountStatement++;
									ulCountTotalStatement++;
									iCountField = 0;
									bQueryInProgress = true;
									if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, szJournalEntry, ulPosition, 0, ulIndex, "", szTableName, pszStartLine);

								}
							}
						} // End of if (bStartOfStmt && strncmp(pszStartLine, "###", 3) == 0)
						else if (strncmp(pszStartLine, "*!BEGIN!", 8) == 0)
						{
							// Start transaction ==> *!BEGIN!107!186!*
							// or 
							// Start transaction ==> *!BEGIN!107!186!124!*
							ulCountTransaction++;
							ulCountTotalTransaction++;
							g_AllDuration.RowCount.ulCountTransaction++;
							ulIndex = 0;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2) g_LogFile.Write("[B]:listenBinLogThread[44] Binlog:%d Line:%d BEGIN : <%s>", ulCountBinLog, ulCountLine, pszStartLine);
							pszLineWork += 8;//skip command
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							// Memorize the position of start transaction  ==>  BinLogSequence.ulPosition 
							ulPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);				// end pos
							ulEndPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							if (pszEnd)
							{
								strncpy(szSeqNo, pszLineWork, pszEnd - pszLineWork); // seq_no
								szSeqNo[pszEnd - pszLineWork] = 0;
							}
							else
							{
								strcpy(szSeqNo, "0");
							}

							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "BEGIN", ulPosition, ulEndPosition, 0, szSeqNo, "", "");

							strcpy(g_stBinLogSeqInProgess.szBinLogName, szBinLogName);
							g_stBinLogSeqInProgess.ulPosition = ulPosition;
							g_stBinLogSeqInProgess.ulIndex = 0;

						}
						else if (strncmp(pszStartLine, "*!STMT!", 7) == 0)
						{// Start statement ==> *!STMT!522!825!*
							bStartOfStmt = true;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2) g_LogFile.Write("[B]:listenBinLogThread[45] Binlog:%d Line:%d STMT: <%s>", ulCountBinLog, ulCountLine, pszStartLine);
							pszLineWork += 7;//skip command
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							ulPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);				// end pos
							ulEndPosition = ddatol(pszLineWork, pszEnd - pszLineWork);

							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "STMT", ulPosition, ulEndPosition, 0, "", "", "");

							ulCountInsert = ulCountUpdate = ulCountDelete = 0;

						}
						else if (strncmp(pszStartLine, "*!COMMIT!", 9) == 0)
						{// commit transaction ==> *!COMMIT!18338!18365!745!*
							bQueryInProgress = bStartOfStmt = bDataColumnInProgress = false;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2) g_LogFile.Write("[B]:listenBinLogThread[46] Binlog:%d Line:%d COMMIT : <%s>",
								ulCountBinLog, ulCountLine, pszStartLine);
							pszLineWork += 9;//skip command
							pszEnd = strchr(pszLineWork, BINLOG_SEP);				// start pos
							ulPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							ulIndex = 0;
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);				// end pos
							ulEndPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							strncpy(szXid, pszLineWork, pszEnd - pszLineWork);		// Xid
							szXid[pszEnd - pszLineWork] = 0;
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "COMMIT", ulPosition, ulEndPosition, 0, szXid, "", "");

							// transaction terminated
							strcpy(g_stBinLogSeqInProgess.szBinLogName, szBinLogName);
							g_stBinLogSeqInProgess.ulPosition = ulEndPosition;
							g_stBinLogSeqInProgess.ulIndex = ulIndex;

							SaveInDisk("B", 2, &g_stBinLogSeqInProgess);

						}
						else if (strncmp(pszStartLine, "*!QUERY!", 8) == 0)
						{// Query ==> *!QUERY!19968!20238!DROP TABLE `dd_journal`
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[50] Binlog:%d Line:%d QUERY :<%s>", ulCountBinLog, ulCountLine, pszStartLine);
							pszLineWork += 8;//skip command
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							// Memorize the position of start transaction  ==>  BinLogSequence.ulPosition 
							ulPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							ulIndex = 0;
							pszLineWork = pszEnd + 1;
							pszEnd = strchr(pszLineWork, BINLOG_SEP);				// end pos
							ulEndPosition = ddatol(pszLineWork, pszEnd - pszLineWork);
							pszLineWork = pszEnd + 1;
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "QUERY", ulPosition, ulEndPosition, 0, "", "", pszLineWork);

							strcpy(g_stBinLogSeqInProgess.szBinLogName, szBinLogName);
							g_stBinLogSeqInProgess.ulPosition = ulEndPosition;
							g_stBinLogSeqInProgess.ulIndex = 0;

							SaveInDisk("B", 3, &g_stBinLogSeqInProgess);

						}
						else if (strncmp(pszStartLine, "*!START_BINLOG!", 15) == 0)
						{// Start Binary log ==> *!START_BINLOG!BinaryLog.000058!*

							g_fStartBinlog = true;
							g_fEndBinlog = false;

							Measurement.Start();
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[53] Binlog:%d Line:%d:<%s>", ulCountBinLog, ulCountLine, pszStartLine);
							ulCountStatement = ulCountTransaction = ulCountLine = 0;
							ulCountInsert = ulCountUpdate = ulCountDelete = 0;

							ulCountBinLog++;
							g_AllDuration.RowCount.ulCountBinLog++;

							pszLineWork += 15;//skip command
							pszEnd = strchr(pszLineWork, BINLOG_SEP);
							strncpy(szBinLogName, pszLineWork, pszEnd - pszLineWork);
							szBinLogName[pszEnd - pszLineWork] = 0;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[54] Binlog:%d Line:%d : <%s>", ulCountBinLog, ulCountLine, szBinLogName);
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "START_BINLOG", 0, 0, 0, "", "", "");

							if (strcmp(szBinLogName, g_StBinLogInfoFirst.szBinLogName) != 0)
							{// If not the first reset position
								strcpy(g_stBinLogSeqInProgess.szBinLogName, szBinLogName);
								g_stBinLogSeqInProgess.ulPosition = BIN_LOG_HEADER_SIZE;
								g_stBinLogSeqInProgess.ulIndex = 0;
							}

						}
						else if (strncmp(pszStartLine, "*!END_BINLOG!", 13) == 0)
						{// end of binary log ==> *!END_BINLOG!BinaryLog.000058!*

							g_fStartBinlog = false;
							g_fEndBinlog = true;

							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[55] Binlog:%d Line:%d (%f s) NbTrans:%lu NbStmt:%lu :<%s>",
								ulCountBinLog, ulCountLine, Measurement.GetTimeFromStart(), ulCountTransaction, ulCountStatement, pszStartLine);
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "END_BINLOG", 0, 0, 0, "", "", "");
						}
						else if (strncmp(pszStartLine, "*!FIRST_BINLOG!", 15) == 0)
						{// first binary log ==> *!FIRST_BINLOG!BinaryLog.000058!XXXX!*

							g_fFirstBinlog = true;
							g_fLastBinlog = false;

							g_MeasurementlistenBinLog.Start();

							ulCountTotalStatement = ulCountTotalTransaction = 0;
							pszLineWork += 15;//skip command
							ulCountBinLog = 0;
							pszEnd = strchr(pszLineWork, BINLOG_SEP); // enf of start binlog
							strncpy(szBinLogName, pszLineWork, pszEnd - pszLineWork);
							szBinLogName[pszEnd - pszLineWork] = 0;

							strcpy(g_AllDuration.StartBinLog.szBinLogName, szBinLogName);
							g_AllDuration.StartBinLog.ulPosition = g_stBinLogSeqInProgess.ulPosition;
							g_AllDuration.StartBinLog.ulIndex = g_stBinLogSeqInProgess.ulIndex;

							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[56] Binlog:%d Line:%d : <%s>", ulCountBinLog, ulCountLine, pszStartLine);
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "FIRST_BINLOG", 0, 0, 0, "", "", "");
						}
						else if (strncmp(pszStartLine, "*!LAST_BINLOG!", 14) == 0)
						{// end of binary log ==> *!LAST_BINLOG!BinaryLog.000058!XXXX!*

							g_fFirstBinlog = false;
							g_fLastBinlog = true;

							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[57] Binlog:%d Line:%d (%f s) NbTrans:%lu NbStmt:%lu :<%s>",
								ulCountBinLog, ulCountLine, Measurement.GetTimeFromStart(), ulCountTotalTransaction, ulCountTotalStatement, pszStartLine);
							if (g_lTraceBinLog) TraceBinLogDataEvents("B", szBinLogName, ulCountLine, "LAST_BINLOG", 0, 0, 0, "", "", "");

							if (strcmp(szBinLogName, g_StBinLogInfoLast.szBinLogName) == 0)
							{
								g_AllDuration.DurationlistenBinLog = g_MeasurementlistenBinLog.GetTimeFromStart();
								strcpy(g_AllDuration.EndBinLog.szBinLogName, szBinLogName);
								g_AllDuration.EndBinLog.ulPosition = g_stBinLogSeqInProgess.ulPosition;
								g_AllDuration.EndBinLog.ulIndex = g_stBinLogSeqInProgess.ulIndex;

								//if (g_lActivateTrace && g_lTraceLevel>TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread Last binlog : <%s> Pos:%ld Idx:%ld SLEEP...", szBinLogName, g_stBinLogSeq.ulPosition, g_stBinLogSeq.ulIndex);
								//Sleep(3000);
								//if (g_lActivateTrace && g_lTraceLevel>TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread Last binlog : <%s> Pos:%ld Idx:%ld SLEEP END", szBinLogName, g_stBinLogSeq.ulPosition, g_stBinLogSeq.ulIndex);

								memcpy(&g_stBinLogSeq, &g_stBinLogSeqInProgess, sizeof(BinLogSequence));

								if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[60] Binlog:%d Line:%d Last binlog : <%s> Pos:%ld Idx:%ld EventEndOfBinlog",
									ulCountBinLog, ulCountLine, szBinLogName, g_stBinLogSeq.ulPosition, g_stBinLogSeq.ulIndex);

								if (!g_hEventEndOfBinlog.Set())
								{
									sprintf(g_szMsgError, "StopThread error SetEvent EndOfBinlog");
									if (g_lActivateTrace) g_LogFile.Write("[B]:listenBinLogThread %s", g_szMsgError);
								}
							}
						}
						else
						{
							//if (g_lActivateTrace && g_lTraceLevel>TRACE_ROW1) 
							//{
							//	int len =  strlen(pszStartLine);
							//	g_LogFile.Write("[B]:listenBinLogThread Binlog:%d Line:%d: LenStartLine<%d><%s>", 
							//						ulCountBinLog, ulCountLine, strlen(pszStartLine), szBUFFER_FORMAT_XZ(pszStartLine, len>100?100:len));
							//}
						}

						ulCountLine++;
						pszStartLine = pszBufferWork + 1;
						//if (g_lActivateTrace && g_lTraceLevel>TRACE_ROW1)
						//{
						//	int len =  strlen(pszStartLine);
						//	//g_LogFile.Write("[B]:listenBinLogThread Binlog:%d Line:%d: NEXT Line LenStartLine<%d><%s>", 
						//	//													ulCountBinLog, ulCountLine, len, szBUFFER_FORMAT_XZ(pszStartLine, len>10?10:len));
						//	g_LogFile.Write("[B]:listenBinLogThread Binlog:%d Line:%d: NEXT Line LenStartLine<%d><%s>", 
						//														ulCountBinLog, ulCountLine, len, pszStartLine);
						//}
					} // End of if (*pszBufferWork == '\n')

					pszBufferWork++;
				}// End while (*pszBufferWork) 

				//if (g_lActivateTrace && g_lTraceLevel>TRACE_ROW2) g_LogFile.Write("[B]:listenBinLogThread bDataColumnInProgress:%d iCountField:%d g_pStArticleInprogress:%p", bDataColumnInProgress, iCountField, g_pStArticleInprogress);

				if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
					g_LogFile.Write("[B]:listenBinLogThread[61] Binlog:%d Line:%d UsedBuf:%ld StartOfStmt:%d QueryInProgress:%d DataColumnInProgress:%d  CountField:%d",
						ulCountBinLog, ulCountLine, (pszBufferWork - g_pszBufferStdOut), bStartOfStmt, bQueryInProgress,
						bDataColumnInProgress, iCountField);

				if ((pszBufferWork - g_pszBufferStdOut) + g_iLenBufferOut > g_iMaxBufferLine)
				{// buffer full
					if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
						g_LogFile.Write("[B]:listenBinLogThread[62] Binlog:%d Line:%d Max buffer(%d) reached %d <%s>",
							ulCountBinLog, ulCountLine, g_iMaxBufferLine, (pszBufferWork - g_pszBufferStdOut) + g_iLenBufferOut, pszStartLine);

					// if max g_pszBufferStdOut reached, and startline = g_pszBufferStdOut and not LOB ==> ERROR not enough memory.
					// if max g_pszBufferStdOut reached, and startline = g_pszBufferStdOut and LOB ==> use temporary file to store line.
					if (g_pszBufferStdOut == pszStartLine || (pszStartLine - g_pszBufferStdOut) < g_iLenBufferOut)
					{
						if (bDataColumnInProgress)
						{// We are waiting LOB ==> store into file  
							fLob = true;
							iLenEntry = pszBufferWork - pszStartLine;
							if ((nWritten = fwrite(pszStartLine, sizeof(char), iLenEntry, g_pFileCapture)) == iLenEntry)
							{
								if (g_lFlush != 0) fflush(g_pFileCapture);
								g_ulCaptureSize += iLenEntry;
							}
							else
							{
								sprintf(g_szMsgGlobal, "listenBinLogThread error write capture [%s] [%d] [%s]", g_szPathCaptureMemo, ferror(g_pFileCapture), GetFormatMessage());
								if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", "B", g_szMsgGlobal);
								dd_log(g_szMsgGlobal);
								exit(0);
							}

							// TODO save line here 
							strncpy(g_pszLine, pszStartLine, iLenEntry);
							g_pszLine[iLenEntry] = 0;

							pszBufferStart = pszStartLine = g_pszBufferStdOut;
						}
						else
						{// No LOB and no data ==> ignore data
							pszBufferStart = pszStartLine = g_pszBufferStdOut;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
								g_LogFile.Write("[B]:listenBinLogThread[65] Binlog:%d Line:%d Ignore data", ulCountBinLog, ulCountLine);
						}
					}
					else
					{// move line at the beginning of the buffer 
						strcpy(g_pszBufferStdOut, pszStartLine);
						pszBufferStart = g_pszBufferStdOut + (pszBufferWork - pszStartLine);
						if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW2)
						{
							g_LogFile.Write("[B]:listenBinLogThread[66] Binlog:%d Line:%d MOVE BUFFER lenLine:%d free:%d <%d><%s>",
								ulCountBinLog, ulCountLine, (pszBufferWork - pszStartLine), pszStartLine - g_pszBufferStdOut, strlen(g_pszBufferStdOut), g_pszBufferStdOut);
						}
					}
					pszStartLine = g_pszBufferStdOut;
				}
				else
				{// continue to get data from pipe
					pszBufferStart = pszBufferWork;
				}

				if (g_fExitApp)
				{
					sprintf(g_szMsgThreadB, "[B]:Exit app : listenBinLogThread[1]");
					dd_log(g_szMsgThreadB);
					if (g_lActivateTrace) g_LogFile.Write(g_szMsgThreadB);
					break;
				}

				if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[67] Binlog:%d Line:%d stdoutRedirect.GetBuffer(b) : Len:%d...",
					ulCountBinLog, ulCountLine, g_iLenBufferOut);
				nBytesRead = g_stdoutRedirect.GetBuffer(pszBufferStart, g_iLenBufferOut);

				g_nBinLogBytesRead = nBytesRead;
				g_nBinLogStep++;

				if (g_lActivateTrace && g_lTraceLevel > TRACE_ROW1) g_LogFile.Write("[B]:listenBinLogThread[68] Binlog:%d Line:%d stdoutRedirect.GetBuffer(b) nBytesRead:%d Error:%d",
					ulCountBinLog, ulCountLine, nBytesRead, nBytesRead < 0 ? g_stdoutRedirect.GetError() : 0);

			} // End of while ( nBytesRead  > 0 )

			if (g_fExitApp)
			{
				sprintf(g_szMsgGlobal, "[B]:Exit app : listenBinLogThread[2]");
				dd_log(g_szMsgGlobal);
				if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
				break;
			}

			if (dwEvent == WAIT_OBJECT_0) break;

		} // End while (1)

		sprintf(g_szMsgThreadB, "[B]:listenBinLogThread terminated last : binlog:%s position:%lu ", g_stBinLogSeqInProgess.szBinLogName, g_stBinLogSeqInProgess.ulPosition);
		dd_log(g_szMsgThreadB);
		if (g_lActivateTrace) g_LogFile.Write(g_szMsgThreadB);

		return 0;
	}
	catch (...)
	{
		sprintf(g_szMsgThreadB, "[B]:listenBinLogThread ERROR(catch)");
		dd_log(g_szMsgThreadB);
		if (g_lActivateTrace) g_LogFile.Write(g_szMsgThreadB);
		exit(0);
	}
}

/**
 *
 */
bool StartThread(const char *pszThread)
{
	bool fReturn = true;
	MeasurementPrecision Measurement;

	Measurement.Start();

	if (!CheckEvent(&g_hEventExit, "Exit"))
	{
		sprintf(g_szMsgError, "StartThread error CreateEvent Exit");
		fReturn = false;
	}

	// Redirection du Stdout
	g_stdoutRedirect.Start(32768);

	if (!CheckEvent(&g_hEvent_B, "B"))
	{
		sprintf(g_szMsgError, "StartThread error CreateEvent");
		fReturn = false;
	}

	if (!CheckEvent(&g_hEventEndOfBinlog, "EndBinLog"))
	{
		sprintf(g_szMsgError, "StartThread error CreateEvent EndOfBinlog");
		fReturn = false;
	}

	g_hThread_B = CreateThread(NULL, 0, listenBinLogThread, NULL, 0, &g_dwTID_B);
	if (g_hThread_B == NULL)
	{
		sprintf(g_szMsgError, "StartThread listenBinLogThread error CreateThread");
		fReturn = false;
	}

	g_LogFile.Write("[%s]:StartThread processed in %f s", pszThread, Measurement.GetTimeFromStart());

	return fReturn;
}

/**
 *
 */
bool IsAliveThread(HANDLE hThread)
{
	bool fReturn = true;
	DWORD dwExitCode;

	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		sprintf(g_szMsgError, "AliveThread ERROR Unable to get exit code thread");
		dd_log(g_szMsgError);
		fReturn = false;
	}
	if (dwExitCode == STILL_ACTIVE)
	{
		fReturn = true;
	}
	else
	{
		fReturn = false;
	}
	return fReturn;
}


/**
 *
 */
bool StopThread(const char* pszThread, const char* pszThreadToStop, DWORD timeOut, HANDLE	hThread)
{
	bool fReturn = true;
	DWORD dwRep = WaitForSingleObject(hThread, timeOut);
	if (dwRep == WAIT_TIMEOUT)
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s WAIT_TIMEOUT", pszThread, pszThreadToStop);
		fReturn = false;
	}
	else if (dwRep == WAIT_FAILED)
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s WAIT_FAILED", pszThread, pszThreadToStop);
		fReturn = false;
	}
	else if (dwRep == WAIT_ABANDONED)
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s WAIT_ABANDONED", pszThread, pszThreadToStop);
		fReturn = false;
	}
	else if (dwRep != WAIT_OBJECT_0)
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s not WAIT_OBJECT_0", pszThread, pszThreadToStop);
		fReturn = false;
	}
	DWORD dwExitCode;
	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s ERROR Unable to get exit code thread", pszThread, pszThreadToStop);
		fReturn = false;
	}
	if (dwExitCode == STILL_ACTIVE)
	{
		sprintf(g_szMsgError, "[%s]:StopThread %s ERROR exit code (%d)", pszThread, pszThreadToStop, dwExitCode);
		fReturn = false;
	}
	return fReturn;
}


/**
 *
 */
bool StopThread(const char *pszThread, DWORD timeOut)
{
	bool fReturn = true;
	MeasurementPrecision Measurement;

	Measurement.Start();


	// Stop redirection
	g_stdoutRedirect.Stop();

	// Stop Thread de traitement
	if (!g_hEvent_B.Set())
	{
		sprintf(g_szMsgError, "StopThread error SetEvent");
		fReturn = false;
	}

	fReturn = StopThread(pszThread, "B", timeOut, g_hThread_B);
	g_LogFile.Write("[%s]:StopThread %s processed in %f s", "B", pszThread, Measurement.GetTimeFromStart());

	return fReturn;
}

/**
 *
 */
bool LoadMasterStatus(const char *pszThread)
{
	bool fReturn = true;

	long lResult;
	MYSQL_ROW hRow = NULL;
	void* hQuery = NULL;
	//MYSQg_FIELD* hField;
	char* hMsg = NULL;
	unsigned int uiMySqlError;
	char* hSqlState = NULL;

	*g_szErrorNumAlpha = 0;// Clear error num
	*g_szMsgError = 0; // Clear error msg
	g_iNbBinLog = 0;

	memset(&g_StMasterStatus, 0, sizeof(BinLogInfo));
	sprintf(g_szSQL, pszQUERY_MASTER_STATUS);
	if (g_lActivateTrace && g_lTraceLevel > 2)  g_LogFile.Write("[%s]:LoadBinLogStatus %s", pszThread, g_szSQL);

	lResult = hMyExecStatement(g_hDatabase, g_szSQL);
	if (lResult != mySUCCESS)
	{
		lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
		lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
		lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
		sprintf(g_szMsgError, "Cannot list MASTER STATUS in database %s in server %s error <%s><%d><%d><%s>",
			g_szDBpub, g_szServer, "hMyExecStatement", uiMySqlError, *hSqlState, hMsg);
		sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
		fReturn = false;
	}
	lResult = hMyGetResult(g_hDatabase, &hQuery);
	if (lResult != mySUCCESS)
	{
		lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
		lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
		lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
		sprintf(g_szMsgError, "Cannot list MASTER STATUS in database %s in server %s error <%s><%d><%d><%s>",
			g_szDBpub, g_szServer, "hMyGetResult", uiMySqlError, *hSqlState, hMsg);
		sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
		fReturn = false;
	}
	if (fReturn)
	{
		char *pszBinLogName = NULL;
		char *pszPosition = NULL;

		lResult = hMyGetFetchRow(hQuery, &hRow);
		if (lResult == mySUCCESS)
		{
			lResult = hMyGetResultValue(hRow, 0, &pszBinLogName);
			lResult = hMyGetResultValue(hRow, 1, &pszPosition);

			strcpy(g_StMasterStatus.szBinLogName, pszBinLogName);
			g_StMasterStatus.ulPosition = strtoul(pszPosition, (char**)0, 10);
			g_StMasterStatus.ulSize = 0;

			if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP)
				g_LogFile.Write("[%s]:LoadMasterStatus<%s> Pos:%d", pszThread, g_StMasterStatus.szBinLogName, g_StMasterStatus.ulPosition);
		}
		else
		{
			fReturn = false;
		}
	}
	lResult = hMyFreeStatement(hQuery);
	return fReturn;
}


/**
 *
 */
bool LoadBinLog(const char *pszThread)
{
	bool fReturn = true;
	BinLogInfo* pStBinLogInfo;
	MeasurementPrecision Measurement;

	long lResult;
	MYSQL_ROW hRow = NULL;
	void* hQuery = NULL;
	//MYSQg_FIELD* hField;
	char* hMsg = NULL;
	unsigned int uiMySqlError;
	char* hSqlState = NULL;
	char *pszColumn1 = NULL;
	char *pszColumn2 = NULL;

	Measurement.Start();

	*g_szErrorNumAlpha = 0;// Clear error num
	*g_szMsgError = 0; // Clear error msg
	g_iNbBinLog = 0;
	sprintf(g_szSQL, pszQUERY_BINLOG);
	if (g_lActivateTrace && g_lTraceLevel > 2)  g_LogFile.Write("[%s]:LoadBinLog %s", pszThread, g_szSQL);

	lResult = hMyExecStatement(g_hDatabase, g_szSQL);
	if (lResult != mySUCCESS)
	{
		lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
		lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
		lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
		sprintf(g_szMsgError, "Cannot list BINLOG in database %s in server %s error <%s><%d><%d><%s>",
			g_szDBpub, g_szServer, "hMyExecStatement", uiMySqlError, *hSqlState, hMsg);
		sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
		fReturn = false;
	}
	lResult = hMyGetResult(g_hDatabase, &hQuery);
	if (lResult != mySUCCESS)
	{
		lResult = hMyGetErrormessage(g_hDatabase, &hMsg);
		lResult = hMyGetErrorNum(g_hDatabase, &uiMySqlError);
		lResult = hMyGetSqlState(g_hDatabase, &hSqlState);
		sprintf(g_szMsgError, "Cannot list BINLOG in database %s in server %s error <%s><%d><%d><%s>",
			g_szDBpub, g_szServer, "hMyGetResult", uiMySqlError, *hSqlState, hMsg);
		sprintf(g_szErrorNumAlpha, "%d", uiMySqlError);
		fReturn = false;
	}

	if (fReturn)
	{
		while ((hMyGetFetchRow(hQuery, &hRow) == mySUCCESS))
		{
			lResult = hMyGetResultValue(hRow, 0, &pszColumn1);
			lResult = hMyGetResultValue(hRow, 1, &pszColumn2);
			// At start g_pStBinLogInfo = NULL and realloc behaves like malloc
			g_pStBinLogInfo = (BinLogInfo*)realloc(g_pStBinLogInfo, (g_iNbBinLog + 1) * sizeof(BinLogInfo));
			if (!g_pStBinLogInfo)
			{
				sprintf(g_szMsgError, "Memory allocation error binlog %s in database %s in server %s", pszColumn1, g_szDBpub, g_szServer);
				fReturn = false;
				break;
			}
			pStBinLogInfo = &g_pStBinLogInfo[g_iNbBinLog];
			strcpy(pStBinLogInfo->szBinLogName, pszColumn1);
			pStBinLogInfo->ulSize = strtoul(pszColumn2, (char**)0, 10);
			pStBinLogInfo->ulPosition = BIN_LOG_HEADER_SIZE;

			//if (g_lActivateTrace && g_lTraceLevel>2)
			//	g_LogFile.Write("LoadBinLog %d : <%s> Size:%d", g_iNbBinLog, pStBinLogInfo->szBinLogName, pStBinLogInfo->ulSize);				

			g_iNbBinLog++;
		}
	}
	lResult = hMyFreeStatement(hQuery);

	if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP)
		g_LogFile.Write("[%s]:LoadBinLog %d file(s) processed in %f s", pszThread, g_iNbBinLog, Measurement.GetTimeFromStart());

	return fReturn;
}


/**
 *
 */
void ReadJournal(const char *pszThread)
{
	Exit_status retval;
	MeasurementPrecision Measurement;
	BinLogInfo* pStBinLogInfo = NULL;
	int iStartIdx;
	bool fError;
	bool fGoAhead;
	// For debug 
	unsigned long ulIndex = 0;
	bool fhasTimeout = false;
	DWORD dwEventEndOfBinlog;

	// Cleanup  BinLog variable 
	//*g_szBinLogName = 0;
	//g_ulLastPosition = 0;

	iStartIdx = 0;
	fGoAhead = false;
	fError = false;

	try
	{
		while (1)
		{
			if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal....", pszThread);
			DWORD dwEvent = g_hEventExit.Wait(1000);
			if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal....LoadBinLog", pszThread);

			memset(&g_AllDuration, 0, sizeof(g_AllDuration));
			g_MeasurementReadJournal.Start();

			if (g_fExitApp)
			{
				sprintf(g_szMsgGlobal, "[%s]:Exit app : ReadJournal(1)", pszThread);
				dd_log(g_szMsgGlobal);
				if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
				break;
			}

			g_MeasurementLoadBinLog.Start();

			g_nReadJournalStep1++;

			fGoAhead = false;
			if (LoadBinLog("M"))
			{
				g_fStartBinlog = false;
				g_fEndBinlog = false;
				g_fFirstBinlog = false;
				g_fLastBinlog = false;

				fError = false;
				pStBinLogInfo = g_pStBinLogInfo;
				// If XAI exist then look up for binlog
				if (*g_stBinLogSeq.szBinLogName)
				{
					for (int i = 0; i < g_iNbBinLog; i++, pStBinLogInfo++)
					{
						//if (g_lActivateTrace && g_lTraceLevel>TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal %s %lu", pszThread, pStBinLogInfo->szBinLogName, pStBinLogInfo->ulSize);
						if (strcmp(g_stBinLogSeq.szBinLogName, pStBinLogInfo->szBinLogName) == 0)
						{
							// Save last position and index of first file to work
							iStartIdx = i;
							pStBinLogInfo->ulPosition = g_stBinLogSeq.ulPosition;
							fGoAhead = true;
							if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal %s Size:%lu found Pos:%lu", pszThread, pStBinLogInfo->szBinLogName, pStBinLogInfo->ulSize, pStBinLogInfo->ulPosition);
						}
					} // End for (int i=0;i<g_iNbBinLog;i++, pStBinLogInfo++)

					if (!fGoAhead)
					{// File not found after loop for ==> error
						sprintf(g_szbuffer_Tmp, "[%s]:ReadJournal : Error : file %s not found in list", pszThread, g_stBinLogSeq.szBinLogName);
						dd_log(g_szbuffer_Tmp);
						if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
						fError = true;
					}
				}
				else
				{// No file already processed ==> start on last file in list

					if (LoadMasterStatus("M"))
					{
						strcpy(g_stBinLogSeq.szBinLogName, g_StMasterStatus.szBinLogName);
						g_stBinLogSeq.ulPosition = g_StMasterStatus.ulPosition;
						g_stBinLogSeq.ulIndex = 0;
						SaveInDisk("M", 4, &g_stBinLogSeq);

						sprintf(g_szbuffer_Tmp, "[%s]:ReadJournal start on %s Pos : %d", pszThread, g_StMasterStatus.szBinLogName, g_StMasterStatus.ulPosition);
						dd_log(g_szbuffer_Tmp);
						if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write(g_szbuffer_Tmp);
					}
				} // End if (*g_stBinLogSeq.szBinLogName && g_iNbBinLog)
			}
			else
			{// error get list binlog ==> log event and do nothing
				if (!fError)
				{
					sprintf(g_szbuffer_Tmp, "[%s]:ReadJournal:LoadBinLog: Error: %s", pszThread, g_szMsgError);
					dd_log(g_szbuffer_Tmp);
					fError = true;
				}
			}
			if (fGoAhead)
			{
				g_nReadJournalStep2++;

				GetDateTime(g_AllDuration.StartDateTime);

				g_MeasurementReadBinLog.Start();
				g_AllDuration.DurationLoadBinlog = g_MeasurementLoadBinLog.GetTimeFromStart();

				memset(&g_StBinLogInfoFirst, 0, sizeof(BinLogInfo));
				memset(&g_StBinLogInfoLast, 0, sizeof(BinLogInfo));
				pStBinLogInfo = &g_pStBinLogInfo[iStartIdx];

				memcpy(&g_StBinLogInfoFirst, pStBinLogInfo, sizeof(BinLogInfo));

				ulIndex++;

				sprintf(g_szbuffer_Tmp, "*!FIRST_BINLOG!%s!%lu!%s!*\n", pStBinLogInfo->szBinLogName, ulIndex, GetDateTime());
				strcpy(g_szLastBufferReadJournal, g_szbuffer_Tmp);
				hPrintfResult(g_szbuffer_Tmp);

				strcpy(g_stBinLogSeqInProgess.szBinLogName, pStBinLogInfo->szBinLogName);
				g_stBinLogSeqInProgess.ulPosition = pStBinLogInfo->ulPosition;
				g_stBinLogSeqInProgess.ulIndex = 0;

				for (int i = iStartIdx; i < g_iNbBinLog; i++, pStBinLogInfo++)
				{
					if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal FileName:%s Position:%lu", pszThread, pStBinLogInfo->szBinLogName, pStBinLogInfo->ulPosition);
					//printf("ReadJournal... ReadJournal FileName:%s Position:%lu\n", pStBinLogInfo->szBinLogName, pStBinLogInfo->ulPosition);
					memcpy(&g_StBinLogSequenceStartRead, pStBinLogInfo, sizeof(BinLogSequence));

					// For debug
					if (fhasTimeout)
					{
						if (g_lTraceDebug > 1)
						{
							sprintf(g_szbuffer_Tmp, "ReadJournal:StartRemoteReadBinLog <%s><%lu> (previous timeout) ", pStBinLogInfo->szBinLogName, pStBinLogInfo->ulPosition);
							dd_log(g_szbuffer_Tmp);
							if (g_lActivateTrace) g_LogFile.Write(g_szbuffer_Tmp);
						}
					}
					fhasTimeout = false;
					if ((retval = hStartReadBinLog(pStBinLogInfo->szBinLogName, pStBinLogInfo->ulPosition, ulIndex)) != OK_CONTINUE)
					{
						sprintf(g_szbuffer_Tmp, "ReadJournal:StartRemoteReadBinLog <%s> ERROR", pStBinLogInfo->szBinLogName);
						dd_log(g_szbuffer_Tmp);
						if (g_lActivateTrace) g_LogFile.Write(g_szbuffer_Tmp);
						fError = true;
					}
					else
					{
						if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal:StartRemoteReadBinLog <%s> processed", pszThread, pStBinLogInfo->szBinLogName);
					}

					if (g_fExitApp)
					{
						sprintf(g_szMsgGlobal, "Exit app : ReadJournal(2)");
						dd_log(g_szMsgGlobal);
						if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
						break;
					}
				}// end of for (int i=iStartIdx;i<g_iNbBinLog;i++, pStBinLogInfo++)

				pStBinLogInfo--;
				memcpy(&g_StBinLogInfoLast, pStBinLogInfo, sizeof(BinLogInfo));

				sprintf(g_szbuffer_Tmp, "*!LAST_BINLOG!%s!%lu!%s!*\n", pStBinLogInfo->szBinLogName, ulIndex, GetDateTime());
				strcpy(g_szLastBufferReadJournal, g_szbuffer_Tmp);
				hPrintfResult(g_szbuffer_Tmp);

				g_AllDuration.DurationReadBinlog = g_MeasurementReadBinLog.GetTimeFromStart();

				if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal:Wait end of <%s> ...", pszThread, pStBinLogInfo->szBinLogName);

				Measurement.Start();

				for (int i = 0; i < 5000 / 100; i++)
				{
					dwEventEndOfBinlog = g_hEventEndOfBinlog.Wait(100);
					if (dwEventEndOfBinlog == WAIT_OBJECT_0)
					{
						g_nReadJournalBreak1++;
						break;
					}
					else if (dwEventEndOfBinlog == WAIT_TIMEOUT && g_fEndBinlog)
					{
						g_nReadJournalBreak2++;
						break;
					}
					else if (dwEventEndOfBinlog == WAIT_FAILED || dwEventEndOfBinlog == WAIT_ABANDONED)
					{
						break;
					}
				}

				g_DurationLastBinlog = Measurement.GetTimeFromStart();

				if (g_DurationLastBinlog > g_DurationLastBinlogMax)
				{
					g_DurationLastBinlogMax = g_DurationLastBinlog;
				}
				if (g_DurationLastBinlog < g_DurationLastBinlogMin)
				{
					g_DurationLastBinlogMin = g_DurationLastBinlog;
				}

				if (g_lActivateTrace && g_lTraceLevel > TRACE_LOOP) g_LogFile.Write("[%s]:ReadJournal:Wait end of <%s> event:%ld", pszThread, pStBinLogInfo->szBinLogName, dwEventEndOfBinlog);

				if (dwEventEndOfBinlog != WAIT_OBJECT_0)
				{
					if (dwEventEndOfBinlog == WAIT_FAILED)
					{
						::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL, GetLastSystemError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							g_szbuffer_Tmp, sizeof(g_szbuffer_Tmp), 0);
						sprintf(g_szMsgGlobal, "[%s]:ReadJournal:Wait end of <%s> WAIT_FAILED [%s]", pszThread, pStBinLogInfo->szBinLogName, g_szbuffer_Tmp);
						dd_log(g_szMsgGlobal);
						if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
					}
					else if (dwEventEndOfBinlog == WAIT_ABANDONED)
					{
						sprintf(g_szMsgGlobal, "[%s]:ReadJournal:Wait end of <%s> WAIT_ABANDONED", pszThread, pStBinLogInfo->szBinLogName);
						dd_log(g_szMsgGlobal);
						if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
					}
					else if (dwEventEndOfBinlog == WAIT_TIMEOUT)
					{
						fhasTimeout = true;
						g_ulLastBinlogTimeout++;

						if (g_lTraceDebug > 0)
						{
							sprintf(g_szMsgGlobal, "[%s]:ReadJournal:Wait end of <%s> timeout event:%ld  LastBinlogTimeout:%d break1:%d break2:%d",
								pszThread, pStBinLogInfo->szBinLogName, dwEventEndOfBinlog, g_ulLastBinlogTimeout,
								g_nReadJournalBreak1, g_nReadJournalBreak2);
							dd_log(g_szMsgGlobal);
							if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
						}

						if (g_lTraceDebug > 1)
						{
							//                                1                2     3  4    5     6    7     12   13  1415 16   17   18   19  20  21  22  23  24   25
							int len = sprintf(g_szMsgGlobal, "[%s]:ReadJournal:%20s %15s %20s %15s %20s %15s %20s %15s %7s %10s %10s %20s %15s %5s %5s %5s %5s %12s %12s\n",
								pszThread, "First_binlog", "First_pos", "Last_binlog", "Last_pos", "Start_binlog", "Start_pos", "XAI_binlog", "XAI_pos", "XAI_SRC", "BIN_read", "BIN_step", "CUR_binlog", "CUR_pos", "First", "Last", "Start", "End", "break1", "break2");
							dd_log(g_szMsgGlobal);
							if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);

							//                              1                2     3  4    5     6    7     12   13    14 15   16   17   18   19   20  21  22  23  24   25
							len = sprintf(g_szMsgGlobal, "[%s]:ReadJournal:%20s %15lu %20s %15lu %20s %15lu %20s %15lu %6c%1d %10d %10d %20s %15lu %5d %5d %5d %5d %12d %12d",
								pszThread,																			// 1
								g_StBinLogInfoFirst.szBinLogName, g_StBinLogInfoFirst.ulPosition,					// 2, 3
								g_StBinLogInfoLast.szBinLogName, g_StBinLogInfoLast.ulPosition,						// 4, 5
								g_StBinLogSequenceStartRead.szBinLogName, g_StBinLogSequenceStartRead.ulPosition,	// 6, 7
								g_StBinLogSequenceXai.szBinLogName, g_StBinLogSequenceXai.ulPosition,				// 12, 13 
								g_cSaveInDiskThread, g_nSaveInDiskIndex,											// 14, 15 
								g_nBinLogBytesRead, g_nBinLogStep,													// 16, 17
								g_stBinLogSeqInProgess.szBinLogName, g_stBinLogSeqInProgess.ulPosition,				// 18, 19
								g_fFirstBinlog, g_fLastBinlog, g_fStartBinlog, g_fEndBinlog,						// 20, 21, 22, 23
								g_nReadJournalBreak1, g_nReadJournalBreak2											// 24, 25
							);
							dd_log(g_szMsgGlobal);
							if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
						}

					}
					//This operation shlould be done in thread read console but we must do this action here 
					memcpy(&g_stBinLogSeq, &g_stBinLogSeqInProgess, sizeof(BinLogSequence));
				}// End of if (dwEventEndOfBinlog != WAIT_OBJECT_0)

			}// End of if (fGoAhead )

			if (dwEvent == WAIT_OBJECT_0 || fError || g_fExitApp)
			{
				sprintf(g_szMsgGlobal, "Exit app : ReadJournal(3) error:%d exit:%d", fError, g_fExitApp);
				dd_log(g_szMsgGlobal);
				if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
				break;
			}

			// Exit loop for test
			//break;

			g_AllDuration.DurationReadJournal = g_MeasurementReadJournal.GetTimeFromStart();
			GetDateTime(g_AllDuration.EndDateTime);

			//if (g_lActivateTrace) g_LogFile.Write("[%s]:ReadJournal position:%lu", pszThread, g_AllDuration.EndBinLog.ulPosition);

			if (strcmp(g_AllDuration.StartBinLog.szBinLogName, g_AllDuration.EndBinLog.szBinLogName) == 0 &&
				g_AllDuration.StartBinLog.ulPosition == g_AllDuration.EndBinLog.ulPosition)
			{
			}
			else
			{
				SaveDurationInDisk();
			}

			if (!IsAliveThread(g_hThread_B))
			{
				sprintf(g_szMsgGlobal, "[%s]:ReadJournal Thead B not alive", pszThread);
				dd_log(g_szMsgGlobal);
				exit(0);
			}

		} // End of while(1)

		if (g_lActivateTrace) g_LogFile.Write("[%s]:ReadJournal terminated", pszThread);

		if (pStBinLogInfo)
		{
			sprintf(g_szMsgGlobal, "[%s]:ReadJournal Binlog:%s Position:%lu", pszThread, pStBinLogInfo->szBinLogName, pStBinLogInfo->ulPosition);
			dd_log(g_szMsgGlobal);
		}

	}
	catch (...)
	{
		sprintf(g_szMsgGlobal, "[%s]:ReadJournal ERROR(catch)", pszThread);
		dd_log(g_szMsgGlobal);
		if (g_lActivateTrace) g_LogFile.Write(g_szMsgGlobal);
		printf(g_szMsgGlobal); printf("\n");
		exit(0);
	}
}

/**
 *
 */
void Cleanup()
{
	if (g_pFileJrnTransact) fclose(g_pFileJrnTransact);
	if (g_output_file) fclose(g_output_file);
	if (g_pszLog) free(g_pszLog);
	if (g_pszBufferStdOut) free(g_pszBufferStdOut);
	if (g_pszLine) free(g_pszLine);
	if (g_pszSavedLine) free(g_pszSavedLine);
	if (g_pFileCapture != NULL)	fclose(g_pFileCapture);
}

/**
 *
 */
void FreeCaptureList(const char *pszThread, StructCapture* pStList)
{
	StructCapture* pStCurrent = pStList;
	StructCapture* pStNext = pStList;
	while (pStNext)
	{
		pStCurrent = pStNext;
		pStNext = pStCurrent->pStNext;
		free(pStCurrent);
	}
}

/**
 *
 */
StructCapture* GetLastCaptureList(const char *pszThread, StructCapture* pStList)
{
	StructCapture* pStCurrent = pStList;
	StructCapture* pStNext = pStList;
	while (pStNext)
	{
		pStCurrent = pStNext;
		pStNext = pStCurrent->pStNext;
	}
	if (pStCurrent)
	{
		if (g_lActivateTrace)
			g_LogFile.Write("[%s]:GetLastCaptureList[%s]", pszThread, pStCurrent->szFileName);
	}
	return pStCurrent;
}

/**
 *
 */
StructCapture* BuildCaptureList(const char *pszThread, char *pszJrn_WithoutExtention)
{
	HANDLE hFind;
	WIN32_FIND_DATA FindData; // File information.
	bool bOk;
	char szSearchName[MAX_PATH];
	char szPath[MAX_PATH];
	short iNbFile = 0;
	StructCapture* pStList = NULL;
	StructCapture* pStCurrent = NULL;

	strcpy(szPath, pszJrn_WithoutExtention);
	char *pszFile = strrchr(szPath, '\\');
	*pszFile = 0;

	sprintf(szSearchName, "%s_capture_*.jrn", pszJrn_WithoutExtention);
	hFind = FindFirstFile(szSearchName, &FindData);

	bOk = (hFind != INVALID_HANDLE_VALUE);
	while (bOk)
	{
		if (pStList == NULL)
		{
			pStList = (struct StructCapture*)malloc(sizeof(StructCapture));
			pStCurrent = pStList;
		}
		else
		{
			pStCurrent->pStNext = (struct StructCapture*) malloc(sizeof(StructCapture));
			pStCurrent = pStCurrent->pStNext;
		}
		strcpy(pStCurrent->szFileName, FindData.cFileName);
		sprintf(pStCurrent->szFullFileName, "%s\\%s", szPath, pStCurrent->szFileName);
		pStCurrent->ulSize = ddGetFileSize(pStCurrent->szFullFileName);
		pStCurrent->pStNext = NULL;

		char *pCapture = strstr(pStCurrent->szFileName, "_capture_");
		char *pDot = strchr(pCapture, '.');
		pCapture += 9;
		memset(pStCurrent->szNum, 0, sizeof(pStCurrent->szNum));
		memcpy(pStCurrent->szNum, pCapture, pDot - pCapture);
		pStCurrent->ulNum = atol(pStCurrent->szNum);

		iNbFile++;

		if (g_lActivateTrace && g_lTraceLevel > 4) 
			g_LogFile.Write("[%s]:BuildCaptureList:%d:<%s> size:%lu Num:%010lu", pszThread, iNbFile, pStCurrent->szFileName, pStCurrent->ulSize, pStCurrent->ulNum);

		bOk = FindNextFile(hFind, &FindData) ? true : false;
	}

	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if (g_lActivateTrace)
		g_LogFile.Write("[%s]:BuildCaptureList  %d file(s)", pszThread, iNbFile);		

	return pStList;
}

/**
 *
 */
bool OpenCapture(const char *pszThread, bool fChange)
{
	bool fReturn = true;
	bool bOk;
	StructCapture* pStList = NULL;
	StructCapture* pStLast = NULL;
	int iLenEntry = 0;
	int nWritten;

	if (g_lCaptureAndProcess == 1)
		g_ulCaptureMax = 1024 * 1000 * 1000;
	else if (g_lCaptureAndProcess>1)
		g_ulCaptureMax = 1024 * 1000 * g_lCaptureAndProcess;
	else
		g_ulCaptureMax = 1024 * 1000 * 1000;
	
	if (g_lActivateTrace)
		g_LogFile.Write("[%s]:OpenCapture Max:%ld change:%d", pszThread, g_ulCaptureMax, fChange);

	if (!fChange)
	{
		pStList = BuildCaptureList(pszThread, g_szJrn_WithoutExtention);
		if (pStList)
		{
			pStLast = GetLastCaptureList(pszThread, pStList);

			if (g_lActivateTrace) g_LogFile.Write("[%s]:OpenCapture Last:%s", pszThread, pStLast ? "SUCCESS" : "ERROR");
		
			if (pStLast)
			{
				g_ulCaptureNum = pStLast->ulNum + 1;
				g_ulCaptureSize = pStLast->ulSize;

				g_pFileCapture = fopen(pStLast->szFullFileName, "r+b");

				// Memo capture name
				strcpy(g_szPathCaptureMemo, pStLast->szFullFileName);
				
				if (g_pFileCapture)
				{
					// Mark EOF in capture file
					fseek(g_pFileCapture, 0, SEEK_END);
					iLenEntry = sprintf(g_szbuffer_Tmp, "*!EOF!%s!%lu!%s!*\n", pStLast->szFullFileName, g_ulCaptureNum, GetDateTime());
					bOk = (nWritten = fwrite(g_szbuffer_Tmp, sizeof(char), iLenEntry, g_pFileCapture)) == iLenEntry;
					if (bOk)
					{
						if (g_lActivateTrace)
							g_LogFile.Write("[%s]:OpenCapture write EOF (%d-%d) success in [%s]", pszThread, iLenEntry, nWritten, pStLast->szFullFileName);
					}
					else
					{
						sprintf(g_szbuffer_Tmp, "OpenCapture Error write EOF (%d-%d-%d) [%s] in [%s]", iLenEntry, nWritten, ferror(g_pFileCapture), GetFormatMessage(), pStLast->szFullFileName);
						if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
						dd_log(g_szbuffer_Tmp);
						fReturn = false;
					}
					fclose(g_pFileCapture);
				}
				else
				{
					sprintf(g_szbuffer_Tmp, "OpenCapture Error open file  [%s] %010lu", pStLast->szFullFileName, g_ulCaptureNum);
					sprintf(g_szMsgError, g_szbuffer_Tmp);
					if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
					dd_log(g_szbuffer_Tmp);
					fReturn = false;
				}
			}
			else
			{
				sprintf(g_szbuffer_Tmp, "OpenCapture ERROR last capture not found");
				sprintf(g_szMsgError, g_szbuffer_Tmp);
				if (g_lActivateTrace)
					g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
				dd_log(g_szbuffer_Tmp);
				fReturn = false;
			}
			FreeCaptureList(pszThread, pStList);
		}
		else
		{
			g_ulCaptureNum = 1;
			g_ulCaptureSize = 0;
		}
	}
	else
	{// fChange ==> add mark EOF in capture file and close 
		g_ulCaptureNum++;

		fseek(g_pFileCapture, 0, SEEK_END);
		iLenEntry = sprintf(g_szbuffer_Tmp, "*!EOF!%s!%lu!%s!*\n", g_szPathCaptureMemo, g_ulCaptureNum, GetDateTime());
		bOk = (nWritten = fwrite(g_szbuffer_Tmp, sizeof(char), iLenEntry, g_pFileCapture)) == iLenEntry;
		if (bOk)
		{
			sprintf(g_szbuffer_Tmp, "OpenCapture write EOF (%d-%d) success in [%s]", iLenEntry, nWritten, g_szPathCaptureMemo);
			if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
			//dd_log(g_szbuffer_Tmp);
		}
		else
		{
			sprintf(g_szbuffer_Tmp, "OpenCapture Error write EOF (%d-%d-%d) [%s] in [%s]", iLenEntry, nWritten, ferror(g_pFileCapture), GetFormatMessage(), g_szPathCaptureMemo);
			sprintf(g_szMsgError, g_szbuffer_Tmp);
			if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
			dd_log(g_szbuffer_Tmp);
			fReturn = false;
		}
		fflush(g_pFileCapture);
		fclose(g_pFileCapture);

	}// End of if (fChange)

	sprintf(g_szbuffer_Tmp, "OpenCapture new file:%010lu  max:%lu (%ld)", g_ulCaptureNum, g_ulCaptureMax, g_lCaptureAndProcess);
	if (g_lActivateTrace)
		g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
	//dd_log(g_szbuffer_Tmp);
		
	g_ulCaptureSize = 0;
	g_ulCaptureCount = 0;

	sprintf(g_szPathCapture, "%s_capture_%010lu.jrn", g_szJrn_WithoutExtention, g_ulCaptureNum);
	g_pFileCapture = fopen(g_szPathCapture, "a+b");

	// Memo capture name
	strcpy(g_szPathCaptureMemo, g_szPathCapture);

	if (g_pFileCapture == NULL)
	{
		sprintf(g_szbuffer_Tmp, "Error open %s", g_szPathCapture);
		sprintf(g_szMsgError, g_szbuffer_Tmp);
		if (g_lActivateTrace)
			g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
		dd_log(g_szbuffer_Tmp);
		fReturn = false;
	}
	else
	{
		// New file created
		g_ulCaptureCount++;
			
		iLenEntry = sprintf(g_szbuffer_Tmp, "*!SOF!%s!%lu!%s!*\n", g_szPathCapture, g_ulCaptureNum, GetDateTime());
		bOk = (nWritten = fwrite(g_szbuffer_Tmp, sizeof(char), iLenEntry, g_pFileCapture)) == iLenEntry;
		if (bOk)
		{
			g_ulCaptureSize += iLenEntry;
		}
		else
		{
			sprintf(g_szbuffer_Tmp, "OpenCapture Error write SOF (%d-%d-%d) [%s]", iLenEntry, nWritten, ferror(g_pFileCapture), GetFormatMessage());
			sprintf(g_szMsgError, g_szbuffer_Tmp);
			if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
			dd_log(g_szbuffer_Tmp);
			fReturn = false;
		}
		fflush(g_pFileCapture);
		fclose(g_pFileCapture);

		g_pFileCapture = fopen(g_szPathCapture, "a+b");

		// Memo capture name
		strcpy(g_szPathCaptureMemo, g_szPathCapture);

		if (g_pFileCapture == NULL)
		{
			sprintf(g_szbuffer_Tmp, "Error reopen %s", g_szPathCapture);
			sprintf(g_szMsgError, g_szbuffer_Tmp);
			if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
			dd_log(g_szbuffer_Tmp);
			fReturn = false;
		}

		sprintf(g_szbuffer_Tmp, "OpenCapture file:%s %s", g_szPathCapture, fReturn ? "OK" : "KO");
		sprintf(g_szMsgError, g_szbuffer_Tmp);
		if (g_lActivateTrace) g_LogFile.Write("[%s]:%s", pszThread, g_szbuffer_Tmp);
		//dd_log(g_szbuffer_Tmp);
	}
	return fReturn;
}

/**
 *
 */
bool ChangeJournal(const char *pszThread)
{
	if (g_lActivateTrace) g_LogFile.Write("[%s]:ChangeJournal [%s]", pszThread, g_szPathCaptureMemo);
	fflush(g_pFileCapture);
	return OpenCapture(pszThread, true);
}

/**
 *
 */
void InitGlobal(void)
{
	memset(&g_StMasterStatus, 0, sizeof(g_StMasterStatus));
	memset(&g_StBinLogInfoFirst, 0, sizeof(g_StBinLogInfoFirst));
	memset(&g_StBinLogInfoLast, 0, sizeof(g_StBinLogInfoLast));
	memset(&g_stBinLogSeqInProgess, 0, sizeof(g_stBinLogSeqInProgess));

	memset(&g_StBinLogSequenceXai, 0, sizeof(g_StBinLogSequenceXai));
	memset(&g_StBinLogSequenceStartRead, 0, sizeof(g_StBinLogSequenceStartRead));
	memset(g_szLastBufferWork, 0, sizeof(g_szLastBufferWork));
	memset(g_szLastBufferReadJournal, 0, sizeof(g_szLastBufferReadJournal));

}


/**
 *
 */
void StopProcess(char *pszProcessName)
{
	SendMessageToLogReader(pszProcessName, 1);
}


/**
 *
 */
void DspXAI(char *pszPathName)
{
	BinLogSequence	stSequence;

	g_pFileTransaction = fopen(pszPathName, "rb");
	if (g_pFileTransaction != NULL)
	{
		fseek(g_pFileTransaction, 0, SEEK_SET);
		int read = fread(&stSequence, sizeof(char), sizeof(BinLogSequence), g_pFileTransaction);
		bool fOk = read == sizeof(BinLogSequence) ? true : false;
		fclose(g_pFileTransaction);
		if (fOk)
		{
			printf("BinLogName:<%s> Position:%lu Index:%lu\n", stSequence.szBinLogName, stSequence.ulPosition, stSequence.ulIndex);
		}
		else
		{
			if (read == 0)
				printf("read %s no data available\n", pszPathName);
			else
				printf("Error read %s read:%d expected:%d\n", pszPathName, read, sizeof(BinLogSequence));
		}
	}
	else
	{
		printf("Error open %s\n", pszPathName);
	}
}

/**
 *
 */
void CrtXAI(char *pszPathName, char	*pszBinLogName, char *pszPosition, char *pszIndex)
{
	BinLogSequence	stSequence;
	char szBackupName[512];

	strcpy(stSequence.szBinLogName, pszBinLogName);
	stSequence.ulPosition = atol(pszPosition);
	stSequence.ulIndex = atol(pszIndex);
	
	if (FileExist(pszPathName) == NOT_EXIST)
		g_pFileJrnTransact = fopen(pszPathName, "w+b"); 
	else
	{
		sprintf(szBackupName, "%s.bak", pszPathName);
		bool res = ::CopyFile(pszPathName, szBackupName, false);
		printf("Copy %s to %s result:%d\n", pszPathName, szBackupName, res);

		g_pFileJrnTransact = fopen(pszPathName, "r+b"); // ouverture
	}
	if (g_pFileJrnTransact != NULL)
	{
		fseek(g_pFileJrnTransact, 0, SEEK_SET);
		fwrite((char*)&stSequence, 1, sizeof(BinLogSequence), g_pFileJrnTransact);
		fclose(g_pFileJrnTransact);
		printf("%s created with [%s][%lu][%lu]\n", pszPathName, stSequence.szBinLogName, stSequence.ulPosition, stSequence.ulIndex);
	}
	else
	{
		printf("Error create %s\n", pszPathName);
	}
}


/**
* Main
*/
int main(int argc, char** argv)
{
	char szDriverName[512];
	int	iReturn = STATUS_OK;

	init_dd_log();

	try
	{
		if (argc <= 1)
		{
			sprintf(g_szbuffer_Tmp, "%s missing argument\n", PROCESS_IDENTITY);
			printf("%s\n", g_szbuffer_Tmp);
			dd_log(g_szbuffer_Tmp);
			return 0;
		}
		if (strcmp(argv[1], "DSPXAI") == 0 && argc == 3)	// Display XAI file  : param = full path file name   
		{
			DspXAI(argv[2]);								// ddMySqlBinLogCapture.exe DSPXAI D:\DD_JOURNAL\MariaDB\MDB_DD_TEST\mdb_dd_test.XAI
			return 0;
		}
		if (strcmp(argv[1], "STOP") == 0 && argc == 3)		// Stop the specified logreader
		{
			StopProcess(argv[2]);							// ddMySqlBinLogCapture.exe STOP DDMyBinLogReaderCapture_mdb_dd_test@195.195.95.60@mdb_dd_test.exe
			return 0;
		}	
		if (strcmp(argv[1], "CRTXAI") == 0 && argc == 6 )
		{
			CrtXAI(argv[2], argv[3], argv[4], argv[5]);		// ddMySqlBinLogCapture.exe CRTXAI D:\DD_JOURNAL\MariaDB\MDB_DD_TEST\mdb_dd_test.XAI dd_test_bin.000033 0 0
			return 0;
		}

		MeasurementPrecision Measurement;
		double duration;

		InitGlobal();

		memset(g_szUser, 0, sizeof(g_szUser));
		memset(g_szPassword, 0, sizeof(g_szUser));

		if ((iReturn == STATUS_OK) && !SearchArg(g_szUser, g_szPassword, argv[1]))
		{
			sprintf(g_szbuffer_Tmp, "[M]:SearchArg: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
			return iReturn;
		}
		
		sprintf(g_szbuffer_Tmp, "[M]:Start %s for %s", PROCESS_IDENTITY, g_szInstance);
		dd_log(g_szbuffer_Tmp);

		// Load param register and others
		if ((iReturn == STATUS_OK) && !LoadParam())
		{
			sprintf(g_szbuffer_Tmp, "[M]:LoadParam: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("LoadParam %s trace:%d\n", iReturn == STATUS_ERROR ? "failed" : "success", g_lActivateTrace);

		sprintf(szDriverName, "%s.dll", g_szDbDriverName);
		if (!LoadMySQLLib(szDriverName, g_szMsgError))
		{
			sprintf(g_szbuffer_Tmp, "[M]:LoadDll: Error %s : %s", g_szDbDriverName, g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("LoadMySQLLib %s\n", iReturn == STATUS_ERROR ? "failed" : "success");

		sprintf(szDriverName, "%s.dll", g_szBinlogLibName);
		if (!LoaddbinlogDll(szDriverName, g_szMsgError))
		{
			sprintf(g_szbuffer_Tmp, "[M]:LoadDll : Error %s : %s", g_szBinlogLibName, g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("LoaddbinlogDll %s\n", iReturn == STATUS_ERROR ? "failed" : "success");

		hInitMySql(argv[0]);
		hMySetTrace(g_lActivateTrace, g_lTraceLevel);
		printf("hInitMySql success\n");

		Measurement.Start();

		// Open connection
		if ((iReturn == STATUS_OK) && !OpenConnection(&g_hDatabase, &g_hSocket, g_szUser, g_szPassword))
		{
			sprintf(g_szbuffer_Tmp, "[M]:OpenConnection: Error:[%s] - [%s]", g_szErrorNumAlpha, g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;

			sprintf(g_szbuffer_Tmp, "[M]:OpenConnection: Error:[%s] - [%s]", g_szUser, g_szPassword);
			dd_log(g_szbuffer_Tmp);

		}
		printf("OpenConnection %s\n", iReturn == STATUS_ERROR ? "failed" : "success");

		if (iReturn == STATUS_OK)
		{
			duration = Measurement.GetTimeFromStart();
			sprintf(g_szbuffer_Tmp, "[M]:OpenConnection to %s in %f s", g_szInstance, duration);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
		}

		if ((iReturn == STATUS_OK) && !GetMySQLVersion("M"))
		{
			sprintf(g_szbuffer_Tmp, "[M]:GetMySQLVersion: Error:%s - %s", g_szErrorNumAlpha, g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		else
		{
			sprintf(g_szbuffer_Tmp, "[M]:MySQLVersion for %s  [%s]", g_szServer, g_szMySqlVersion);
			dd_log(g_szbuffer_Tmp);
		}
		printf("GetMySQLVersion %s [%s][%d]\n", iReturn == STATUS_ERROR ? "failed" : "success", g_szMySqlVersion, g_iMySQLVersion);

		bool hasJrn = SearchJrnPath();
		printf("SearchJrnPath %s\n", !hasJrn ? "failed" : "success");
		// Recuperer le chemin du journal
		if (iReturn == STATUS_OK && !hasJrn)
		{
			sprintf(g_szbuffer_Tmp, "[M]:SearchJrnPath: Error:%s - %s ", g_szErrorNumAlpha, g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}

		sprintf(g_szbuffer_Tmp, "[M]:%s: JRN:%s", PROCESS_IDENTITY, g_szJrnName);
		dd_log(g_szbuffer_Tmp);

		printf("SearchJrnPath %s [%s]\n", iReturn == STATUS_ERROR ? "failed" : "success", g_szJrnName);
				
		if (g_lActivateTrace)
		{
			char *pszFileName = strrchr(g_szJrnName, '\\');
			g_LogFile.SetBufferSize(40000);
			g_LogFile.SetArchive((int)g_lTraceArchive);
			g_LogFile.Open(ddstrncpy(g_szbuffer_Tmp, g_szJrnName, pszFileName - g_szJrnName, true), ++pszFileName, "log", (g_lTraceMode == TRACE_APPEND) ? true : false);
			g_LogFile.Write("**************************************************************");
			g_LogFile.Write("**************************************************************");
			g_LogFile.Write("**************************************************************");
			g_LogFile.Write("OpenConnection to %s in %f", g_szInstance, duration);
			g_LogFile.Write("TraceMode:%ld TraceArchive:%ld TraceLevel:%ld", g_lTraceMode, g_lTraceArchive, g_lTraceLevel);
			//*g_szPreviousTime = 0;
		}

		if ((iReturn == STATUS_OK))
		{
			strcpy(g_szJrnPath, g_szJrnName);
			char *p = strrchr(g_szJrnPath, '\\');
			char *jrn = p + 1;
			*p = 0;
			sprintf(g_szWorkingArea, "%s\\%s", g_szJrnPath, WORKING_AREA);
			sprintf(g_szJrn_WithoutExtention, "%s\\%s\\%s", g_szJrnPath, WORKING_AREA, jrn);
			sprintf(g_szbuffer_Tmp, "+++ WORKING:<%s> JRN:<%s>", g_szWorkingArea, g_szJrn_WithoutExtention);
			dd_log(g_szbuffer_Tmp);
		}

		if (g_lActivateTrace && g_lTraceLevel > TRACE_INIT) g_LogFile.Write("[M]:GetMySqlVersion : <%s><%d>", g_szMySqlVersion, g_iMySQLVersion);

		// Initialization param BinLog
		if ((iReturn == STATUS_OK) && !InitParam())
		{
			sprintf(g_szbuffer_Tmp, "[M]:InitParam: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("InitParam %s\n", iReturn == STATUS_ERROR ? "failed" : "success");

		// Initialization capture
		if ( iReturn == STATUS_OK && !OpenCapture("M", false) )
		{
			sprintf(g_szbuffer_Tmp, "[M]:OpenCapture: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("OpenCapture %s\n", iReturn == STATUS_ERROR ? "failed" : "success");

		// Initialization ReadBinLog
		if ((iReturn == STATUS_OK) && !(hInitRemoteReadBinLog(argv[0], g_szServer, g_szUser, g_szPassword, g_szPort) == OK_CONTINUE))
		{
			sprintf(g_szbuffer_Tmp, "[M]:InitReadBinLog: Error");
			dd_log(g_szbuffer_Tmp);
			printf("%s\n", g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}
		printf("hInitRemoteReadBinLog %s\n", iReturn == STATUS_ERROR ? "failed" : "success");
				
		// For debug ==> duplicate result statement in output_file
		if (g_lActivateTrace && g_lTraceLevel > TRACE_INIT)
		{
			sprintf(g_szbuffer_Tmp, "%s_result.log", g_szJrnName);
			g_pszLog = (char*)malloc(g_iMaxBufferLine + 100);
			g_LogFile.Write("[M]:MaxBufferLine:%d", g_iMaxBufferLine);
			g_output_file = fopen(g_szbuffer_Tmp, "w");
		}

		// Start listener thread 
		if ((iReturn == STATUS_OK) && !StartThread("M"))
		{
			sprintf(g_szbuffer_Tmp, "StartThread: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}

		InitThreadPipe();

		if (g_lTraceBinLog > 0)
		{
			try
			{
				sprintf(g_szDB, "%s.DB", g_szJrnName);
				g_db.open(g_szDB);
				g_iRc = g_db.execDML("PRAGMA count_changes = 0;");		// insert, update, delete ne retourne plus le nombre de lignes impactees
				g_iRc = g_db.execDML("PRAGMA foreign_keys = 0;");
				g_iRc = g_db.execDML("PRAGMA legacy_file_format = 0;");
				g_iRc = g_db.execDML("PRAGMA journag_mode = 'OFF';");
				//g_iRc = g_db.execDML("PRAGMA locking_mode = 'EXCLUSIVE';");
				g_iRc = g_db.execDML("PRAGMA synchronous = 'OFF';");
				g_iRc = g_db.execDML("PRAGMA cache_size = 10000;");
				g_iRc = g_db.execDML("PRAGMA auto_vaccum=INCREMENTAL;");
				g_iRc = g_db.execDML("PRAGMA incrementag_vaccum;");
				g_iRc = g_db.execDML("VACUUM;");	// To Shrink the db file
				if (!g_db.tableExists("EVENTS"))
					g_iRc = g_db.execDML("create table EVENTS (ID INTEGER PRIMARY KEY AUTOINCREMENT, CREATED TIMESTAMP DEFAULT CURRENT_TIMESTAMP, LogName TEXT, Line INTEGER, EventType TEXT, Position INTEGER, EndPosition INTEGER, Idx INTEGER, Xid TEXT, TableName Text, Command TEXT);");
				if (!g_db.tableExists("CONTENTS"))
					g_iRc = g_db.execDML("create table CONTENTS (ID INTEGER PRIMARY KEY AUTOINCREMENT, CREATED TIMESTAMP DEFAULT CURRENT_TIMESTAMP, Line INTEGER, Info TEXT);");
				sprintf(g_szbuffer_Tmp, "[M]:Open: %s ", g_szDB);
				dd_log(g_szbuffer_Tmp);
				//printf("%s\n", g_szbuffer_Tmp);
			}
			catch (CppSQLite3Exception& e)
			{
				iReturn = STATUS_ERROR;
				sprintf(g_szbuffer_Tmp, "[M]:Error opening %s: error:%i %s", g_szDB, e.errorCode(), e.errorMessage());
				dd_log(g_szbuffer_Tmp);
				printf("%s\n", g_szbuffer_Tmp);
			}
		}

		// Read journal
		//printf("ReadJournal...\n");
		if (iReturn == STATUS_OK) ReadJournal("M");

		sprintf(g_szbuffer_Tmp, "[M]:ReadJournal terminated");
		dd_log(g_szbuffer_Tmp);
		//printf("%s\n", g_szbuffer_Tmp);

		if ((iReturn == STATUS_OK) && !StopThread("M", 2000))
		{
			sprintf(g_szbuffer_Tmp, "[M]:StopThread: Error: %s", g_szMsgError);
			dd_log(g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}

		// Cleanup param BinLog
		if ((iReturn == STATUS_OK) && !(hFreeReadBinLog() == OK_CONTINUE))
		{
			sprintf(g_szbuffer_Tmp, "[M]:FreeReadBinLog: Error");
			dd_log(g_szbuffer_Tmp);
			iReturn = STATUS_ERROR;
		}

		sprintf(g_szbuffer_Tmp, "[M]:%s First:binlog:[%s] position:%lu", PROCESS_IDENTITY, g_StBinLogInfoFirst.szBinLogName, g_StBinLogInfoFirst.ulPosition);
		dd_log(g_szbuffer_Tmp);

		sprintf(g_szbuffer_Tmp, "[M]:%s Last:binlog:[%s] position:%lu", PROCESS_IDENTITY, g_StBinLogInfoLast.szBinLogName, g_StBinLogInfoLast.ulPosition);
		dd_log(g_szbuffer_Tmp);

		sprintf(g_szbuffer_Tmp, "[M]:%s start:binlog:[%s] position:%lu", PROCESS_IDENTITY, g_StBinLogSequenceStartRead.szBinLogName, g_StBinLogSequenceStartRead.ulPosition);
		dd_log(g_szbuffer_Tmp);

		sprintf(g_szbuffer_Tmp, "[M]:%s XAI(%c, %d):binlog:[%s] position:%lu", PROCESS_IDENTITY, g_cSaveInDiskThread, g_nSaveInDiskIndex, g_StBinLogSequenceXai.szBinLogName, g_StBinLogSequenceXai.ulPosition);
		dd_log(g_szbuffer_Tmp);

		sprintf(g_szbuffer_Tmp, "[M]:%s Last line : [%s]", PROCESS_IDENTITY, g_szLastBufferWork);
		dd_log(g_szbuffer_Tmp);

		sprintf(g_szbuffer_Tmp, "[M]:Stop %s for schema:%s", PROCESS_IDENTITY, g_szDBpub);
		dd_log(g_szbuffer_Tmp);

		// Crash on CloseConnection(); ==> exit 
		//CloseConnection();	

		Cleanup();

		if (g_lActivateTrace) g_LogFile.Write("[M]:%s terminated", PROCESS_IDENTITY);
		g_LogFile.Close();

		hMyFreeDB();

	}
	catch (std::exception &e)
	{
		sprintf(g_szbuffer_Tmp, "[M]:ERROR %s ==> %s", PROCESS_IDENTITY, e.what());
		dd_log(g_szbuffer_Tmp);
	}
	catch (...)
	{
		sprintf(g_szbuffer_Tmp, "[M]:ERROR %s ==> %s", PROCESS_IDENTITY, "non-standard exception");
		dd_log(g_szbuffer_Tmp);
	}

	free_dd_log();

	return iReturn;
}
