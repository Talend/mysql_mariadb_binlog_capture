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
//////////////////////////////////////////////////////////////////////////////////
//
//																			
//////////////////////////////////////////////////////////////////////////////////
//																			
//	FILE				:Synchronozation.h
//																			
//	PURPOSE				:syncronisation wrapper
//																			
//
//	DATE				:17/05/2013
//
// FEATURES				:
//						:
//////////////////////////////////////////////////////////////////////////////////

#ifndef __SYNCHRONISATION__
#define __SYNCHRONISATION__

#include <windows.h>
#include <winbase.h>

//////////////////////////////////////////////////////////////////////////////////
// Wrapper for CriticalSection Lock
//////////////////////////////////////////////////////////////////////////////////
class Lock
{
	CRITICAL_SECTION h;
	Lock(Lock const &);
	Lock const & operator=(Lock const &);
public:
	Lock()
	{
		InitializeCriticalSection(&h);
	}
	~Lock()
	{
		DeleteCriticalSection(&h);
	}
	void Enter()
	{
		EnterCriticalSection(&h);
	}
//	bool TryEnter()
//	{
//		return 0 != TryEnterCriticalSection(&h);
//	}
	void Exit()
	{
		LeaveCriticalSection(&h);
	}
	CRITICAL_SECTION * Handle()
	{
		return &h;
	}
};


//////////////////////////////////////////////////////////////////////////////////
// Wrapper for Event Signal
//////////////////////////////////////////////////////////////////////////////////
class Event
{
	HANDLE h;
    Event(Event const &);
    Event const & operator=(Event const &);
public:
	explicit Event(bool manual = false)
    {
		h = CreateEvent(NULL, manual, false, NULL);
    }
    ~Event()
    {
		CloseHandle(h);
	}
	bool Set()
	{
		return SetEvent(h)?true:false;
	}
	bool Clear()
	{
		return ResetEvent(h)?true:false;
	}
	DWORD Wait()
	{
		return WaitForSingleObject(h, INFINITE);
	}
	DWORD Wait(DWORD dwMilliseconds)
	{
		return WaitForSingleObject(h, dwMilliseconds);
	}
	HANDLE * Handle()
	{
		return &h;
	}
};


#endif //__SYNCHRONISATION__
