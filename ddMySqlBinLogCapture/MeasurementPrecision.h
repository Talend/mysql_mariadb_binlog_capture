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
//	FILE				:MeasurementPrecision.h
//																			
//	PURPOSE				:A  wrapper for measurement precision
//																			
//
//	DATE				:01/07/2011
//
// USAGE				:source code below
//
//#include "MeasurementPrecision.h"
//
//MeasurementPrecision Measurement;
//if (!Measurement.Start()) {process error}
//......
//double duration = Measurement.GetTimeFromStart();
//if (duration == 0) {process error}
//prinf("duration:%f", duration);
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef _MESURE_PRECISION_H
#define _MESURE_PRECISION_H

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
	#include <stdint.h>
	#include <stdbool.h>
	#include <stddef.h>
	#include <assert.h>
#endif

#ifndef WIN32

#define LARGE_INTEGER int64_t

/* Helpful conversion constants. */
static const unsigned usec_per_sec = 1000000;
static const unsigned usec_per_msec = 1000;

/* These functions are written to match the win32
   signatures and behavior as closely as possible.
*/
bool QueryPerformanceFrequency(LARGE_INTEGER *frequency)
{
    /* Sanity check. */
    assert(frequency != NULL);

    /* gettimeofday reports to microsecond accuracy. */
    *frequency = usec_per_sec;

    return true;
}

bool QueryPerformanceCounter(LARGE_INTEGER *performance_count)
{
    struct timeval time;

    /* Sanity check. */
    assert(performance_count != NULL);

    /* Grab the current time. */
    gettimeofday(&time, NULL);
    *performance_count = time.tv_usec + /* Microseconds. */
                         time.tv_sec * usec_per_sec; /* Seconds. */

    return true;
}
#endif

class MeasurementPrecision 
{
private:
	LARGE_INTEGER frequency, begin, end;
    char lastError[264];
public:

	MeasurementPrecision()
	{
		strcpy(lastError,"Pas d'erreur");
	}
	bool Start()
	{
		if (!QueryPerformanceFrequency(&frequency))
		{
			strcpy(lastError,"QueryPerformanceFrequency error");
			return false;
		}
		if(!QueryPerformanceCounter (&begin))
		{
			strcpy(lastError,"QueryPerformanceCounter error");
			return false;
		}
		strcpy(lastError,"Pas d'erreur");
		return true;
	}
	double GetTimeFromStart()
	{
		if (!QueryPerformanceCounter (&end))
		{
			strcpy(lastError,"QueryPerformanceCounter error");
			return 0;
		}
		strcpy(lastError,"No error");
#ifdef WIN32
		return ((double)((__int64)end.QuadPart)-((__int64)begin.QuadPart)) / (double)frequency.QuadPart;
#else
		return ((double)((__int64)end)-((__int64)begin)) / (double)frequency;
#endif
	}
	char * GetLastError()
	{
		return lastError;
	}

};
#endif
