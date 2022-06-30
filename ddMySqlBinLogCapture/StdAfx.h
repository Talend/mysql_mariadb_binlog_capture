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
