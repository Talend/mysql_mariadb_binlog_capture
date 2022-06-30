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
#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#include "targetver.h"	
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <math.h>

#define FD_SETSIZE		1024 
#define NT
#define NOOP_SUPPORTED

#pragma warning (disable : 4189) // local variable is initialized but not referenced
#pragma warning (disable : 4100) // unreferenced formal parameter
#pragma warning (disable : 4127) // conditional expression is constant (cf TRACE)
#pragma warning (disable : 4700) // local variable 'hstmt' used without having been initialized
#pragma warning (disable : 4701) // local variable 'dwProcessNameLenExpected' may be used without having been initialized

#include <sys/types.h>
#include <time.h>
#include <WinSock2.h>
#include <WinBase.h>
#include <crtdbg.h>
#include <sqlext.h>
#include <sqltypes.h>

#define _Packed
#pragma pack (1)

#endif // STDAFX_H
