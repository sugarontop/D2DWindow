///////////////////////////////////////////////////////////////////////
//
// (C) Copyright 2014, Yukihiro Furuya.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#pragma once


#include "D2DMisc.h"



namespace V4 
{
struct RowCol
{
	int row;
	int col;

	RowCol():row(0),col(0){}

};
class Grid
{
	public :
		Grid():rows_(1),cols_(1){}

		void Set( int rows, int cols )
		{
			rows_ = rows;
			cols_ = cols;
			_ASSERT( rows_ > 0 && cols_ > 0 );
		}

		void SetRect( FRectF rc )
		{ 
			rc_ = rc; 
		}

		FRectF RectF( int row, int col ) const
		{
			FSizeF sz = rc_.Size();			
			float w = sz.width / cols_;
			float h = sz.height / rows_;
			FRectF r(rc_.left, rc_.top, rc_.left+w, rc_.top+h );
			r.Offset( w * col, h * row);
			return r;
		}
		FSizeF SizeF() const
		{
			FSizeF sz = rc_.Size();			
			float w = sz.width / cols_;
			float h = sz.height / rows_;
			return FSizeF( w, h );
		}
		int Rows() const { return rows_; }
		int Cols() const { return cols_; }
	
	private :
		int rows_,cols_;
		FRectF rc_;
			
};





class DateTime
{
	public :
		DateTime(){ GetLocalTime(&localtm_); }
		DateTime( const SYSTEMTIME& time ):localtm_(time){};
		DateTime( double value ){ VariantTimeToSystemTime( value, &localtm_ );	}
		static DateTime Now()
		{ 
			SYSTEMTIME lt;	
			GetLocalTime(&lt);
			return DateTime(lt);
		}
		static DateTime FarDate()
		{ 
			DateTime d;
			d.Parse( L"2200/01/01" );
			return d;
		}
		static DateTime UtcNow()
		{
			SYSTEMTIME lt;	
			GetSystemTime(&lt);
			return DateTime(lt);
		}
		DateTime ToUtc() const
		{
			TIME_ZONE_INFORMATION tinfo;
			GetTimeZoneInformation( &tinfo );
			
			SYSTEMTIME utc;	
			TzSpecificLocalTimeToSystemTime( &tinfo, &localtm_, &utc );
			
			return DateTime( utc );
		}
		DateTime ToLocal() const
		{
			TIME_ZONE_INFORMATION tinfo;
			GetTimeZoneInformation( &tinfo );
			
			SYSTEMTIME lctm;	
			SystemTimeToTzSpecificLocalTime( &tinfo, &localtm_, &lctm );
			
			return DateTime( lctm );
		}
		FString ToStringTime( int typ=0 ) const
		{
			FString rval;

			if ( typ == 1 )
				rval = FString::Format( L"%02d:%02d", Hour(),Min());
			else
				rval = FString::Format( L"%02d:%02d:%02d", Hour(),Min(),Sec());			
				
			return rval;
		}

		enum TYP{ YYYYMMDDHHSSMM=0, YYYYMMDD,HHSSMM  };
		
		FString ToString(TYP ty) const
		{
			FString r;
			if ( ty == TYP::YYYYMMDD )
				r = FString::Format( L"%04d/%02d/%02d", localtm_.wYear,localtm_.wMonth,localtm_.wDay );
			else if ( ty == TYP::HHSSMM )
				r = FString::Format( L"%02d:%02d:%02d", localtm_.wHour,localtm_.wMinute,localtm_.wSecond );
			else 				
				r = FString::Format( L"%04d/%02d/%02d %02d:%02d:%02d", localtm_.wYear,localtm_.wMonth,localtm_.wDay, localtm_.wHour,localtm_.wMinute,localtm_.wSecond );

			return r;
		}
		FString ToString() const
		{
			return ToString(DateTime::TYP::YYYYMMDDHHSSMM);
		}
		
		// ŽžŠÔ‚Ì”äŠr‚ÅŽg—p‚·‚é ,ƒ~ƒŠ•b‚Í–³Ž‹‚³‚ê‚é
		double Value() const 
		{
			double ret;
			SystemTimeToVariantTime((LPSYSTEMTIME)&localtm_, &ret ); 
			return ret;
		}

		WORD YYYY()  const{ return localtm_.wYear; }
		WORD MM()  const { return localtm_.wMonth; }
		WORD DD()  const{ return localtm_.wDay; }
		WORD Hour()  const{ return localtm_.wHour; }
		WORD Min()  const{ return localtm_.wMinute; }
		WORD Sec()  const{ return localtm_.wSecond; }
		WORD Msec()  const{ return localtm_.wMilliseconds; }

		void Parse(LPCWSTR yyyymmddetc )
		{
			WCHAR buf[256];
			WCHAR* p = buf;
			WCHAR* ps = (WCHAR*)yyyymmddetc;

			int md = 0;

			int val[10];
			memset(val,0,sizeof(int)*10);

			while(1)
			{
				if ( *ps == '/' || *ps == ' ' || *ps == 0  )
				{
					*p = 0;
					val[md++] = _wtoi(buf);
					p = buf;
					ps++;
				}
				else if ( *ps == ':' || *ps == ' ' || *ps == 0 )
				{
					*p = 0;
					val[md++] = _wtoi(buf);
					p = buf;
					ps++;
				}
				
				if ( *ps == 0 )
					break;
				
				*p++ = *ps++;
			}
			localtm_.wYear = val[0];
			localtm_.wMonth = val[1];
			localtm_.wDay = val[2];
			localtm_.wHour = val[3];
			localtm_.wMinute = val[4];
			localtm_.wSecond = val[5];
			localtm_.wMilliseconds = val[6];

		}


		struct Timer
		{
			LARGE_INTEGER freq, ntime;
		};

		// Œo‰ßŽžŠÔ‚ð•b’PˆÊ‚ÅŒv‘ª
		static Timer Start();
		static double Stop( Timer& timer ); 

	private :
		SYSTEMTIME localtm_;
};

class Exception
{
	public :
		Exception( LPCWSTR errmesg ){ msg_ = errmesg; now_ = DateTime::Now(); }

		FString Message(){ return msg_; }
		DateTime Date(){ return now_; }
	private :
		FString msg_;
		DateTime now_;

};


};

