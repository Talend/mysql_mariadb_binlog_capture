//////////////////////////////////////////////////////////////////////////////////
//
//																			
//////////////////////////////////////////////////////////////////////////////////
//																			
//	FILE				:Synchronozation.h
//																			
//	PURPOSE				:syncronisation wrapper
//																			
//	MANUFACTURER		:Gamma Soft
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
