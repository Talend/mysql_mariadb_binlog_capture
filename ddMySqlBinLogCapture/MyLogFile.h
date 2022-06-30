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
//	FILE				:MyLogFile.h
//																			
//	PURPOSE				:A multithread debug log file wrapper
//																			
//
//	DATE				:03/08/2011
//
// FEATURES				:not depends on MFC ore ATL.
//						:file name could use absolute path or just the name, in which case the 
//						:file will be created at the same place with the binary module, no concern 
//						:with curret directory, which always bring me truble.
//						:every log line has a time stamp attached.
//						:uses printf like format to write log lines
//						:multi thread safe, finally added
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __MYLOGFILE__
#define __MYLOGFILE__

#ifdef WIN32
#include <windows.h>
typedef CRITICAL_SECTION MutexT;
#else
#include <pthread.h>
typedef pthread_mutex_t MutexT;
#endif

#include <stdio.h>
#include <stdlib.h>


struct StructFile
{		
	char szFileName[MAX_PATH];
	struct StructFile*	pStNext;	// Next pointer of StructFile
};

class MyLogFile
{
private:
	FILE*	m_pLogFile;
	MutexT	m_MutexLog;
	int		iBufferSize; 
	char*	pszBufferLog;
	char*	pszBufferLine;
	char	szFileLogName[MAX_PATH];
	char	szFileExtention[MAX_PATH];
	char	szFilePath[MAX_PATH];
	int		iNumberOfFile;
	bool	bMode;
	bool	bDisplay;

public:
	MyLogFile()
	{
		m_pLogFile = NULL;
		CreateMutex(&m_MutexLog);	
		iBufferSize = 2048; 
		pszBufferLog = (char*)malloc(iBufferSize);
		pszBufferLine = (char*)malloc(iBufferSize);
		iNumberOfFile = 1;
		bMode = true;
		bDisplay = false;
		
	}

	/////////////////////////////////////////////////////////
	//	Destructor, close if logfile if opened
	/////////////////////////////////////////////////////////
	~MyLogFile()
	{
		if (m_pLogFile)
		{
			fputs("\n===============Finish Loging================\n\n", m_pLogFile);
			fclose(m_pLogFile);
		}
		DestroyMutex(m_MutexLog);	
		if (pszBufferLog) free(pszBufferLog);
		if (pszBufferLine)  free(pszBufferLine);
	}

	/////////////////////////////////////////////////////////////////////////
	// Close 
	/////////////////////////////////////////////////////////////////////////
	void Close(void)
	{
		if (m_pLogFile)	
		{
			WriteWithoutMutex("Close Log...<%s><%s><%s><%s><%d>", szFilePath, szFileLogName, szFileExtention, bMode?"true":"false", iNumberOfFile);
			fclose(m_pLogFile);
		}
		m_pLogFile = NULL;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// Open
	//
	// PARAMETER:
	// bAppend = true	==> Append to a file
	// bAppend = false	==> Create a new file. If (the file exists its contents is erased.
	/////////////////////////////////////////////////////////////////////////////////////////
	void Open(char* pszFile, bool bAppend = true)
	{
		strcpy(szFileLogName, pszFile);
		strcpy(szFileExtention, "");
		strcpy(szFilePath, "");
		bMode = bAppend;
		m_pLogFile = fopen( pszFile, bAppend ? "a" : "w");
		WriteWithoutMutex("Open Log...<%s><%s><%s><%s><%d>", szFilePath, szFileLogName, szFileExtention, bMode?"true":"false", iNumberOfFile);

	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// Open
	//
	// SUMMARY:
	// A date YYYY-MM-DD will be added before extension
	// Ex : Filename: dummy ==> return dummy2011-08-15
	//		Filename: dummy.log ==> return dummy2011-08-15.log
	//
	// PARAMETER:
	// bAppend = true	==> Append to a file
	// bAppend = false	==> Create a new file. If (the file exists its contents is erased.
	/////////////////////////////////////////////////////////////////////////////////////////
	void Open(const char* pszPath, const char* pszNameWithoutExtention, const char* szExtention, bool bAppend = true)
	{
		strcpy(szFileLogName, pszNameWithoutExtention);
		strcpy(szFileExtention, szExtention);
		strcpy(szFilePath, pszPath);
		bMode = bAppend;

		char szBuffer[256];
		time_t tTime;
		struct tm *tmTime;
		time(&tTime);
		tmTime = localtime(&tTime);

		sprintf(szBuffer, "%s\\%s%04d-%02d-%02d.%s", pszPath, pszNameWithoutExtention, tmTime->tm_year+1900, tmTime->tm_mon+1, tmTime->tm_mday, szExtention);
		m_pLogFile = fopen( szBuffer, bAppend ? "a" : "w");
		WriteWithoutMutex("Open Log...<%s><%s><%s><%s><%d>", szFilePath, szFileLogName, szFileExtention, bMode?"true":"false", iNumberOfFile);
		if (bDisplay) printf("Open Log...<%s><%s><%s><%s><%d>", szFilePath, szFileLogName, szFileExtention, bMode?"true":"false", iNumberOfFile);
	}


	/////////////////////////////////////////////////////////////////////////
	//	Write log info into the logfile, with printf like parameters support
	//  Multi thread safe
	/////////////////////////////////////////////////////////////////////////
	void Write(char* pszFormat, ...)
	{
		if (!m_pLogFile) return;

		LockMutex(m_MutexLog);		
		
		//write the formated log string to szLog
		va_list argList;
		va_start( argList, pszFormat );
		vsprintf( pszBufferLog, pszFormat, argList );
		va_end( argList );

		//Get current time
		SYSTEMTIME	time;
		::GetLocalTime(&time);
		
		sprintf(pszBufferLine, "%02d:%02d:%02d:%03d \t%s\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, pszBufferLog);
		fputs(pszBufferLine, m_pLogFile);
		fflush(m_pLogFile);
		if (bDisplay) printf(pszBufferLine);

		UnlockMutex(m_MutexLog);		
	}

	/////////////////////////////////////////////////////////////////////////
	//	Write log info into the logfile, with printf like parameters support
	//  Multi thread safe
	/////////////////////////////////////////////////////////////////////////
	void WriteWithoutCR(char* pszFormat, ...)
	{
		if (!m_pLogFile) return;

		LockMutex(m_MutexLog);		
		
		//write the formated log string to szLog
		va_list argList;
		va_start( argList, pszFormat );
		vsprintf( pszBufferLog, pszFormat, argList );
		va_end( argList );

		//Get current time
		SYSTEMTIME	time;
		::GetLocalTime(&time);
		
		sprintf(pszBufferLine, "%02d:%02d:%02d:%03d \t%s", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, pszBufferLog);
		fputs(pszBufferLine, m_pLogFile);
		fflush(m_pLogFile);
		if (bDisplay) printf(pszBufferLine);

		UnlockMutex(m_MutexLog);		
	}

	/////////////////////////////////////////////////////////////////////////
	//	Write log info into the logfile, with printf like parameters support
	//  No Multi thread safe
	//  No Mutex  ==> use with Enter et Leave function
	/////////////////////////////////////////////////////////////////////////
	void WriteWithoutMutex(char* pszFormat, ...)
	{
		if (!m_pLogFile) return;

		//write the formated log string to szLog
		va_list argList;
		va_start( argList, pszFormat );
		vsprintf( pszBufferLog, pszFormat, argList );
		va_end( argList );

		//Get current time
		SYSTEMTIME	time;
		::GetLocalTime(&time);
		
		sprintf(pszBufferLine, "%02d:%02d:%02d:%03d \t%s\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, pszBufferLog);
		fputs(pszBufferLine, m_pLogFile);
		fflush(m_pLogFile);
		if (bDisplay) printf(pszBufferLine);

	}

	/////////////////////////////////////////////////////////////////////////
	//	Write char log info into the logfile
	//  Multi thread safe
	/////////////////////////////////////////////////////////////////////////
	void Write(char c)
	{
		if (!m_pLogFile) return;
		LockMutex(m_MutexLog);		
		fputc(c, m_pLogFile);
		if (bDisplay) printf("%c", c);
		UnlockMutex(m_MutexLog);		
	}

	/////////////////////////////////////////////////////////////////////////
	//	Write char log info into the logfile
	//  No Multi thread safe
	//  No Mutex  ==> use with Enter et Leave function
	/////////////////////////////////////////////////////////////////////////
	void WriteWithoutMutex(char c)
	{
		if (!m_pLogFile) return;
		fputc(c, m_pLogFile);
		if (bDisplay) printf("%c", c);
	}

	/////////////////////////////////////////////////////////////////////////
	//	Use with  WriteWithoutMuxtex functions
	/////////////////////////////////////////////////////////////////////////
	void Enter()
	{
		if (!m_pLogFile) return;
		LockMutex(m_MutexLog);		
	}

	/////////////////////////////////////////////////////////////////////////
	//	Use with  WriteWithoutMuxtex functions
	/////////////////////////////////////////////////////////////////////////
	void Leave()
	{
		if (!m_pLogFile) return;
		UnlockMutex(m_MutexLog);		
	}

	/////////////////////////////////////////////////////////////////////////
	//	
	/////////////////////////////////////////////////////////////////////////
	void SetBufferSize(int iSize)
	{
		iBufferSize = iSize; 
		pszBufferLog = (char*)realloc(pszBufferLog, iBufferSize);
		pszBufferLine = (char*)realloc(pszBufferLine, iBufferSize);
	}

	/////////////////////////////////////////////////////////////////////////////
	// SetArchive
	/////////////////////////////////////////////////////////////////////////////
	void SetArchive(const short iNumberOfArchive)
	{
		iNumberOfFile=iNumberOfArchive;
	}

	/////////////////////////////////////////////////////////////////////////////
	// SetDisplay
	/////////////////////////////////////////////////////////////////////////////
	void SetDisplay(const bool bDisplayScreen)
	{
		bDisplay = bDisplayScreen;
	}


	/////////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////
	bool DeleteLog(void)
	{
		HANDLE hFind;
		WIN32_FIND_DATA FindData; // File information.
		bool bOk;
		char szSearchName[MAX_PATH];
		short iNbFile=0;
		StructFile* pStList = NULL;
		StructFile* pStCurrent = NULL;

		sprintf(szSearchName, "%s\\%s*.%s", szFilePath, szFileLogName, szFileExtention);
		if (bDisplay) printf("DeleteLog : <%s>\n", szSearchName);

		hFind = FindFirstFile(szSearchName, &FindData);
		bOk = (hFind != INVALID_HANDLE_VALUE);
		while (bOk) 
		{
			if (pStList==NULL)
			{
				pStList=(struct StructFile*) malloc(sizeof(StructFile));
				pStCurrent = pStList;
			}
			else
			{
				pStCurrent->pStNext = (struct StructFile*) malloc(sizeof(StructFile));
				pStCurrent = pStCurrent->pStNext;
			}
			strcpy(pStCurrent->szFileName, FindData.cFileName);
			pStCurrent->pStNext = NULL;
			iNbFile++;
			bOk = FindNextFile(hFind, &FindData)?true:false;
		}

		if (hFind != INVALID_HANDLE_VALUE)
			FindClose(hFind);
		
		short iDelete = (iNbFile-iNumberOfFile)>0?(iNbFile-iNumberOfFile):0;
		StructFile* pStToDeleted=NULL;

		bool bRep = true;
		if  (pStList)
		{
			if (iDelete)
			{
				sprintf(szSearchName, "%s\\%s", szFilePath, pStList->szFileName);
				bRep &= remove(szSearchName)?false:true;
				iDelete--;
			}
			pStCurrent = pStList->pStNext;
			free(pStList);
			while (pStCurrent)
			{
				if (iDelete)
				{
					sprintf(szSearchName, "%s\\%s", szFilePath, pStCurrent->szFileName);
					bRep &= remove(szSearchName)?false:true;
					iDelete--;
				}
				pStToDeleted = pStCurrent;
				pStCurrent = pStCurrent->pStNext;
				free(pStToDeleted);
			}
		}
		return bRep;
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////
	bool ArchiveLog(void)
	{
		bool bResult = true;

		LockMutex(m_MutexLog);		
		WriteWithoutMutex("ArchiveLog in progress...");
		Close();
		if (bDisplay) printf("ArchiveLog in progress...\n");
		bResult = DeleteLog();
		if (bDisplay) printf("ArchiveLog terminated\n");
		Open(szFilePath, szFileLogName, szFileExtention, bMode);
		UnlockMutex(m_MutexLog);

		Write("ArchiveLog terminated");

		return bResult;
	}

#ifdef WIN32
/*********************** WINDOWS OS SPECIFIC FUNCTIONALITY ********************************/
	bool CreateMutex(MutexT* pMutex)	
	{
		InitializeCriticalSection(pMutex);return (pMutex!=NULL);
	}
	void LockMutex(MutexT& mutex)		
	{
		EnterCriticalSection(&mutex);
	}
	void UnlockMutex(MutexT& mutex)		
	{
		LeaveCriticalSection(&mutex);
	}
	void DestroyMutex(MutexT& mutex)	
	{
		DeleteCriticalSection(&mutex);
	}

#else
/************************* POSIX OS SPECIFIC FUNCTIONALITY ********************************/
	bool CreateMutex(MutexT* pMutex) 
	{
		return ( pthread_mutex_init(pMutex, NULL)==0 );
	}
	void LockMutex(MutexT& mutex) 
	{
		pthread_mutex_lock(&mutex);
	}
	void UnlockMutex(MutexT& mutex) 
	{
		pthread_mutex_unlock(&mutex);
	}
	void DestroyMutex(MutexT& mutex) 
	{
		pthread_mutex_destroy(&mutex);
	}
#endif

};

#endif //__MYLOGFILE__
