#pragma once

#include <vector>
#include <map>
#include <string>
// 2015/04/03

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Json Decoding,  late parsing
namespace HiggsJson {

struct Higgs
{
	WCHAR* head;
	int len;
};

WCHAR* ParseList( LPCWSTR str, std::vector<Higgs>& ar );
WCHAR* ParseMap( LPCWSTR str, std::map<std::wstring,Higgs>& map );


std::wstring stringbuild( LPCWSTR s, int len );
int ToInt(Higgs x);
double ToDouble(Higgs x);
std::wstring ToStr(Higgs x);
std::vector<Higgs> ToList(Higgs x);
std::map<std::wstring,Higgs> ToMap(Higgs x);
bool ToBool(Higgs x);


std::wstring ToObject( LPCWSTR json_str ); // 頭とお尻の"を取るだけ

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Json Encoding
struct XVariant
{
	enum XVT{XVT_NULL=0,XVT_MAP,XVT_LIST,XVT_VALUE,XVT_STRING,XVT_INT,XVT_FLOAT,XVT_BOOL,XVT_NAN,XVT_STR_MAP,XVT_STR_LIST, };

	XVT vt;
	union v
	{
		int ival;
		LPCWSTR str;
		bool bl;
		float fval;

		std::vector<XVariant>* ar;
		std::map<std::wstring,XVariant>* map;
	};

	v val;

	private :
		std::wstring buffer_;

	public :
	XVariant& operator=(const XVariant& x )
	{
		if (&x != this )
		{
			vt = x.vt;
			val = x.val;

			if ( vt == XVT_STRING || vt == XVT_STR_MAP || vt == XVT_STR_LIST )
			{
				buffer_ = x.buffer_;
				val.str = buffer_.c_str();
			}
		}
		return *this;
	}
	XVariant():vt(XVT_NULL){}

	XVariant( LPCWSTR s, XVT v):vt(v){ buffer_ = s; val.str = buffer_.c_str(); } // XVT_STRING_MAP, XVT_STRING_LIST
	
	XVariant( LPCWSTR s):vt(XVT_STRING){ buffer_ = s; val.str = buffer_.c_str(); }
	XVariant( int n ):vt(XVT_INT){ val.ival = n; }
	XVariant( bool bl ):vt(XVT_BOOL){ val.bl = bl; }
	XVariant( float fval ):vt(XVT_FLOAT){ val.fval = fval; }	
	XVariant( std::vector<XVariant>* s):vt(XVT_LIST){ val.ar = s; }
	XVariant( std::map<std::wstring,XVariant>* s):vt(XVT_MAP){ val.map = s; }


	
};


std::wstring StringEncode( LPCWSTR s );

/*
howto encode:
	std::map<std::wstring,XVariant> m2;
	XVariant v1(100);
	XVariant v2( L"furuya");
	m2[L"val"] = v1;
	m2[L"nm"] = v2;
	XVariant kk(&m2);
	
	auto json = ValueEncode(kk);
*/

std::wstring ValueEncode( const XVariant& s );


XVariant::XVT whatType(Higgs x );
bool IsNULL( Higgs x );

};
