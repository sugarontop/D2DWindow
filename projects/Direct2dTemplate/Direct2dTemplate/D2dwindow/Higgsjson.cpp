#include "stdafx.h"
#include <sstream>
#include "higgsjson.h"

namespace HiggsJson
{
bool IsNULL( Higgs x )
{
	return ( x.len == 0 );
}

WCHAR* ParseList( LPCWSTR str, std::vector<Higgs>& ar )
{
	WCHAR* p = (WCHAR*)str;
	WCHAR last = 0;
	if ( *p == '[' )
		last = ']';	

	if ( last )
	{
		p++;
		while( *p != last )
		{
			Higgs x;
			x.head = p;

			int acnt = 0, bcnt=0;
			int char_mode = 0;
			bool bext = false;
			do
			{
				 bext = false;

				if ( *p == '"') 
				{
					char_mode += 1;
					if ( char_mode == 2 )
						char_mode = 0;
				}
												
				if ( char_mode == 0 )
				{
					if ( *p == '[' ) acnt++;
					else if ( *p == ']') acnt--;
					else if ( *p == '{' ) bcnt++;
					else if ( *p == '}') bcnt--;
				}
				else
				{
					if ( *p == '\\' && *(p+1)=='"' )
					{
						char_mode += -1;						
						bext = true;
					}
				}

				p++;
			}
			while( (*p != ',' && *p != last) || acnt!=0 || bcnt!=0 || char_mode || bext );

			x.len = p - x.head;
			ar.push_back( x );

			if ( *p == ',' )
				p++;

		}
		

		p++;
	}
	return p;
}
WCHAR* ParseMap( LPCWSTR str, std::map<std::wstring,Higgs>& map )
{
	WCHAR last = 0;

	WCHAR* p = (WCHAR*)str;
	while( *p <=' ' ) p++;	
	if ( *p == '{' )
		last = '}';

	if ( last )
	{
		p++;
		while( *p <=' ' ) p++;	
		while( *p != last )
		{
			// string parse
			while( *p <=' ' ) p++;	
			WCHAR key[256];
			WCHAR* kh = key;

			_ASSERT( *p == '"' );
			p++;
			do
			{
				*kh = *p++;	
				kh++;
			}
			while( *p != '"' );
			*kh = 0;

			p++;
			while( *p <=' ' ) p++;		
			_ASSERT( *p == ':' );
			p++;
			while( *p <=' ' ) p++;		

			Higgs x;
			x.head = p;

			int acnt = 0, bcnt=0;
			int char_mode = 0;
			bool bext = false;
			do
			{
				 bext = false;

				if ( *p == '"') 
				{
					char_mode += 1;
					if ( char_mode == 2 )
						char_mode = 0;
				}
												
				if ( char_mode == 0 )
				{
					if ( *p == '[' ) acnt++;
					else if ( *p == ']') acnt--;
					else if ( *p == '{' ) bcnt++;
					else if ( *p == '}') bcnt--;
				}
				else
				{
					if ( *p == '\\' && *(p+1)=='"' )
					{
						char_mode += -1;						
						bext = true;
					}
				}

				p++;
			}
			while( (*p != ',' && *p != last) || acnt!=0 || bcnt!=0 || char_mode || bext );
			

			x.len = p - x.head;
			map[key] = x;

			if ( *p == ',' )
				p++;

		}
		

		p++;
	}
	return p;
}



std::wstring stringbuild( LPCWSTR s, int len )
{
	WCHAR* p = (WCHAR*)s;
	std::wstringstream sm;

	int pos = 0;
	while( *p && pos < len )
	{
		if ( *p == '\\' )
		{
			p++;
			pos++;
			if ( *p == 'u' )
			{
				p++;
				pos++;
				WCHAR cb[5];
				for( int i = 0; i < 4; i++ )
					cb[i] = *p++;
				cb[4] = 0;

				wchar_t*	pszEnd;
				sm << (WCHAR)::wcstol(cb,&pszEnd,16); // 16bitHex->String								

				pos+=4;
			}
			else if ( *p == 't' ){ sm << '\t'; p++; pos++;}
			else if ( *p == 'r' ){ sm << '\r'; p++; pos++; }
			else if ( *p == 'n' ){ sm << '\n'; p++; pos++;}
			else if ( *p == '"' ){ sm << '\"'; p++; pos++;}
			else if ( *p == '\\' ){ sm << '\\'; p++; pos++;}
		}
		else
		{
			sm << *p++;
			pos++;
		}
	}

	return sm.rdbuf()->str();


}
bool ToBool(Higgs x)
{
	WCHAR* pch = x.head;
	while( *pch <= ' ' ) pch++;
	
	if ( *pch == 'f' && x.len >= 5 )
	{
		WCHAR* p = pch;
		if ( p[0] == 'f' &&p[1]=='a'&&p[2]=='l'&& p[3]=='s' && p[4]=='e' )
			return false;
	}
	else if ( *pch == 't' && x.len >= 4 )
	{
		WCHAR* p = pch;
		if ( p[0] == 't' &&p[1]=='r'&&p[2]=='u'&& p[3]=='e' )		
			return true;
	}
	return false;
}

int ToInt(Higgs x)
{
	WCHAR cb[64];
	memcpy( cb, x.head, x.len*2 );
	cb[x.len]=0;
	return _wtoi(cb);
}
double ToDouble(Higgs x)
{
	WCHAR cb[64];
	memcpy( cb, x.head, x.len*2 );
	cb[x.len]=0;
	return _wtof(cb);
}
std::wstring ToStr(Higgs x)
{
	int alen = x.len;
	auto p = x.head;
	while( *p <= ' ' ){ p++; alen--; }
		


	WCHAR ch = *p;
	if (ch == '"' )
	{
		WCHAR* head = p;
		WCHAR* p = head+1;
	

		for( int i = 1; i < alen; i++)
		{
			if ( p[i] == L'"' && p[i-1] != L'\\' )
			{
				p += i;
				break;
			}
		}

		int len = p - (head);

		_ASSERT( len <= x.len );

		_ASSERT( *(head+len) == L'"' );

		std::vector<WCHAR> cb( len );
		memcpy( &cb.at(0), head+1, (len-1)*sizeof(WCHAR) );
		cb[len-1]=0;

		return stringbuild(&cb.at(0),(len-1));
	}
	else if ( ch == L'n' )
	{
		// maybe null
	}
	else if (ch == '{')
	{
		int cnt = 1;
		WCHAR* head = p;
		WCHAR* p = head + 1;

		for (int i = 1; i < alen; i++)
		{
			if (p[i] == L'}')
			{
				if ( cnt == 1 )
				{
					i++;
					p += i;					
					break;
				}

				cnt--;
			}
			else if ( p[i] == L'{' )
				cnt++;
		}

		int len = p - (head);

		_ASSERT(alen <= x.len);

		_ASSERT(*(head + len) == L'}');

		std::vector<WCHAR> cb(len+1);
		memcpy(&cb.at(0), head, (len)*sizeof(WCHAR));
		cb[len] = 0;

		return stringbuild(&cb.at(0), len);
	}
	return L"";
}

std::vector<Higgs> ToList(Higgs x)
{
	std::vector<Higgs> ret;
	ParseList( x.head, ret );
	return ret;
}

std::map<std::wstring,Higgs> ToMap(Higgs x)
{
	std::map<std::wstring,Higgs> ret;
	ParseMap( x.head, ret );
	return ret;
}



std::wstring StringEncode( LPCWSTR s )
{
	int len = lstrlen(s);
	std::wstringstream sm;
	sm << L'"';

	for( int i = 0; i < len; i++ )
	{
		WCHAR c = s[i];
		if ( c < 255 )
		{
			switch( c )
			{
				case '\\':
				case '"':
				case '\t':
				case '\r':
				case '\n':
				case '\f':
				case '\b':
				case '/' :
				{
					sm << '\\';
					sm << c;
				}
				break;
				default:
					sm << c;
			}
		}
		else
		{
			// unicode encoding			
			WCHAR buf[10];			
			wsprintf( buf, L"\\u%04x", (long)c );
			sm << buf;
		}
	}
	sm << L'"';
	return sm.rdbuf()->str();
}
std::wstring ValueEncode( const XVariant& s )
{
	if ( s.vt == XVariant::XVT::XVT_STRING )
	{
		return StringEncode( s.val.str );
	}
	else if ( s.vt == XVariant::XVT::XVT_INT )
	{
		std::wstringstream sm;
		sm << s.val.ival;
		return sm.rdbuf()->str();
	}
	else if ( s.vt == XVariant::XVT::XVT_FLOAT )
	{
		std::wstringstream sm;
		sm << s.val.fval;
		return sm.rdbuf()->str();
	}
	else if ( s.vt == XVariant::XVT::XVT_STR_LIST )
	{
		std::wstringstream sm;
		sm << ToObject( s.val.str );		
		return sm.rdbuf()->str();
	}

	else if ( s.vt == XVariant::XVT::XVT_LIST )
	{
		bool bfirst = true;
		std::wstringstream sm;
		sm << L"[";

		auto ar = s.val.ar;
		for( auto& it : *ar )
		{
			if ( !bfirst )				
				sm << L",";

			sm << ValueEncode( it );
			bfirst = false;	
		}
		sm << L"]";
		return sm.rdbuf()->str();
	}
	else if ( s.vt == XVariant::XVT::XVT_STR_MAP )
	{
		std::wstringstream sm;
		sm << ToObject( s.val.str );		
		return sm.rdbuf()->str();
	}

	else if ( s.vt == XVariant::XVT::XVT_MAP )
	{
		bool bfirst = true;
		std::wstringstream sm;
		sm << L"{";

		auto ar = s.val.map;
		for( auto& it : *s.val.map )
		{
			if ( !bfirst )				
				sm << L",";
			

			sm << StringEncode( it.first.c_str() );
			sm << L':';
			sm << ValueEncode( it.second );		
			bfirst = false;	
		}
		sm << L"}";
		return sm.rdbuf()->str();
	}
	

	std::wstringstream sm;
	if ( s.vt == XVariant::XVT::XVT_NULL )
		sm << L"null";
	else if ( s.vt == XVariant::XVT::XVT_BOOL )
		sm << (s.val.bl ? L"true" : L"false");

	return sm.rdbuf()->str();
}



XVariant::XVT whatType(Higgs x )
{
	if ( x.len == 0 ) return XVariant::XVT_NULL;

	WCHAR* pch = x.head;

	while ( *pch <= L' ' ) 
		pch++;

	WCHAR ch = *pch;

	if ( ch == L'[' ) return XVariant::XVT_LIST;
	if ( ch == L'{' ) return XVariant::XVT_MAP;

	

	if ( ch == L'"' ) return XVariant::XVT_STRING;
	
	if ( ('0' <= ch && ch <='9' ) || '-' == ch || '+' == ch )
	{
		for( int i = 0; i < x.len; i++ )
		{
			if ( '.' == x.head[i] )
				return XVariant::XVT_FLOAT;
		}
	
		return XVariant::XVT_INT;
	}	
	else if ( ch == 'n' && x.len >= 4 )
	{
		WCHAR* p = pch;
		if ( p[0] == 'n' &&p[1]=='u'&&p[2]=='l'&& p[3]=='l' )
			return XVariant::XVT_NULL; 
	}
	else if ( ch == 'f' && x.len >= 5 )
	{
		WCHAR* p = pch;
		if ( p[0] == 'f' &&p[1]=='a'&&p[2]=='l'&& p[3]=='s' && p[4]=='e' )
			return XVariant::XVT_BOOL; 
	}
	else if ( ch == 't' && x.len >= 4 )
	{
		WCHAR* p = pch;
		if ( p[0] == 't' &&p[1]=='r'&&p[2]=='u'&& p[3]=='e' )		
			return XVariant::XVT_BOOL; 
	}
	

	return XVariant::XVT_NAN; // 不明
}


std::wstring ToObject( LPCWSTR json_str )
{
	WCHAR* p = (WCHAR*)json_str;

	if ( *p == L'"' && ( *(p+1) == L'[' || *(p+1) == L'{' ) )
	{
		p++;

		WCHAR *s, *e;
		s = p;
		e = s;

		WCHAR last = ((*p == L'[') ? L']' : L'}' );

		int acnt = 0;
		int bcnt = 0;
		bool ccnt = false;

		while(*p++)
		{
			if ( (acnt == 0) && (bcnt == 0) &&(ccnt == false) && ( *p == last && *(p+1) == L'"') )
			{
				e = p;
				break;
			}
			
			if ( *p == '[' ) acnt++;
			else if ( *p == ']') acnt--;
			else if ( *p == '{' ) bcnt++;
			else if ( *p == '}') bcnt--;
			else if ( *p == '"') ccnt=!ccnt;
						
		}
		_ASSERT( (acnt == 0) && (bcnt == 0) &&(ccnt == false));

		return std::wstring( (LPCWSTR)s, (int)(e-s+1));
	}
	return p;
}

};
