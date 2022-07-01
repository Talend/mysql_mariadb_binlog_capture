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

#include <windows.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

#define READ_FD		0
#define WRITE_FD	1

#define CHECK(a) if ((a)!= 0) return -1;

class StdOutRedirect
{
    public:
        int Start(int bufferSize);
        int Stop();
        int GetBuffer(char *buffer, int size);
		int GetError();

    private:
        int fdStdOutPipe[2];
        int fdStdOut;
		int error;
};

int StdOutRedirect::Start(int bufferSize)
{
	error = 0;
    if (_pipe(fdStdOutPipe, bufferSize, O_TEXT)!=0)
		return -1;
    fdStdOut = _dup(_fileno(stdout));

    fflush( stdout );
    CHECK(_dup2(fdStdOutPipe[WRITE_FD], _fileno(stdout)));
    ios::sync_with_stdio();
    setvbuf( stdout, NULL, _IONBF, 0 ); // absolutely needed
    return 0;
}

int StdOutRedirect::Stop()
{
    CHECK(_dup2(fdStdOut, _fileno(stdout)));
    ios::sync_with_stdio();
    _close(fdStdOut);
    _close(fdStdOutPipe[WRITE_FD]);
    _close(fdStdOutPipe[READ_FD]);
    return 0;
}

int StdOutRedirect::GetBuffer(char *buffer, int size)
{
    int nOutRead = _read(fdStdOutPipe[READ_FD], buffer, size);
    if (nOutRead>0) 
	{
		buffer[nOutRead] = '\0';
		error = 0;
	}
	else if (nOutRead==0) 
	{
		error = 0;
	}
	else
		error = errno;

    return nOutRead;
}

int StdOutRedirect::GetError()
{
    return error;
}

