/*
The MIT License (MIT)
Copyright (c) 2015 sugarontop@icloud.com
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "stdafx.h"
#include "datetime.h"
#include <time.h>

using namespace V4;

static LPCWSTR wdaynm[] = { L"Sun",L"Mon",L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
static LPCWSTR monthnm[] = { L"Jan",L"Feb",L"Mar",L"Apr", L"May",L"Jun",L"Jul",L"Aug",L"Sep",L"Oct",L"Nov",L"Dec" };


DateTime::DateTime()
{
	year = 0;
	mon =  0;
	mday = 0;
}
DateTime::DateTime(const FILETIME& ft)
{
	FILETIME localFileTime;
	SYSTEMTIME systemTime;

	FileTimeToLocalFileTime(&ft, &localFileTime);
	FileTimeToSystemTime(&localFileTime, &systemTime);

	year = systemTime.wYear;
	mon = 	systemTime.wMonth+1;
	mday = 	systemTime.wDay;
	hour = 	systemTime.wHour;
	min = 	systemTime.wMinute;
	sec = 	systemTime.wSecond;
	//year = 	systemTime.wMilliseconds;
	wday = 	systemTime.wDayOfWeek;
}
FString DateTime::Format( TYP typ )
{
	FString r = L"DateTime::Format typ error";

	if ( typ == YMD )
		r = r.Format( L"%d/%02d/%02d", year,mon, mday );
	else if ( typ == YMDHMS )
		r = r.Format( L"%d/%02d/%02d %02d:%02d:%02d", year,mon, mday, hour, min, sec );
	else if ( typ == RFC1123 )
	{
		// RFC1123
		// Thu, 16 May 2013 03:36:09 GMT
		r = r.Format( L"%s, %02d %s %d %02d:%02d:%02d GMT", wdaynm[wday],mday, monthnm[mon-1], year, hour, min, sec );
	}
	else if ( typ == ISO8601 )
	{
		// 2008-01-01T13:00:00Z
		// 拡張表記
		r = r.Format( L"%d-%02d-%02dT%02d:%02d:%02dZ", year, mon,mday,hour,min,sec ); // UTC
	}

	return r;
}

DateTime DateTime::Now()
{
	DateTime dt;

	struct tm newtime;
	__time64_t long_time;
	// Get time as 64-bit integer.
	_time64( &long_time ); 
	// Convert to local time.
	_localtime64_s( &newtime, &long_time ); 

	dt.sec = newtime.tm_sec;
	dt.min = newtime.tm_min;
	dt.hour = newtime.tm_hour;
	dt.mday = newtime.tm_mday;	
	dt.mon = newtime.tm_mon + 1;
	dt.year = newtime.tm_year + 1900;
	dt.wday = newtime.tm_wday;

	return dt;

}
DateTime DateTime::GNow()
{
	DateTime dt;

	struct tm newtime;
	__time64_t long_time;
	// Get time as 64-bit integer.
	_time64( &long_time ); 
	// Convert to local time.
	_gmtime64_s( &newtime, &long_time ); 

	dt.sec = newtime.tm_sec;
	dt.min = newtime.tm_min;
	dt.hour = newtime.tm_hour;
	dt.mday = newtime.tm_mday;	
	dt.mon = newtime.tm_mon + 1;
	dt.year = newtime.tm_year + 1900;
	dt.wday = newtime.tm_wday;

	return dt;

}
