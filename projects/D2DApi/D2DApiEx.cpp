#include "stdafx.h"
#include "D2DContextEx.h"
#include "D2DApi.h"
#include "D2DMisc.h"
#include "D2DResource.h"
#include "D2DContextNew.h"
#include "StringStream.h"

namespace V4{ 

DLLEXPORT void WINAPI D2DrawCenterText( D2DContextBase& cxt, ID2D1Brush* clr, const FRectF& rc, LPCWSTR str, int len, int align )
{	
	DWRITE_TEXT_METRICS tm;

	CComPtr<IDWriteTextLayout> tl;
	FSizeF sz = rc.GetSize();
	if (HR(cxt.wfactory->CreateTextLayout(str,len, cxt.text,sz.width, sz.height, &tl )))
	{
		tl->GetMetrics(&tm);

		FRectF rcX(rc);
		float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
		rcX.top = center - tm.height/2;
		rcX.bottom = rcX.top + tm.height;

	
		if ( align == 2 )
			rcX.left = rcX.right-tm.width;
		else if ( align == 1 )
		{
			rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
			rcX.right = rcX.left + tm.width;
		}

		cxt.cxt->DrawText( str, len, cxt.text, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	}
}

DLLEXPORT int WINAPI D2GetLineMetrics( D2DContextBase& cxt, const FSizeF& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS* ret )
{
	xassert( ret );
	
	
	DWRITE_TEXT_METRICS tm;
	CComPtr<IDWriteTextLayout> tl;
	if (HR(cxt.wfactory->CreateTextLayout(str,len, cxt.text,sz.width, sz.height, &tl )))
	{	
		tl->GetMetrics(&tm);

		*ret = tm;

		return tm.lineCount;
	}
	return 0;
}
DLLEXPORT void WINAPI DrawSkyBlueGrid( ID2D1RenderTarget* cxt, FSizeF sz, float duration )
{
	cxt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	D2DMatrix mat(cxt);
	mat.PushTransform();
	CComPtr<ID2D1SolidColorBrush> br;
	cxt->CreateSolidColorBrush(D2RGB(226,245,250),&br);

	int fcnt = (int)(sz.width / duration);

	for( int x = 0; x < fcnt; x++ )
	{	
		mat.Offset( duration, 0 );	
		DrawLine( cxt, FPointF(0,0), FPointF(0,sz.height), 1, br );
	}
	mat.PopTransform();
	mat.PushTransform();

	fcnt = (int)(sz.height / duration);
	for( int x = 0; x < fcnt; x++ )
	{	
		mat.Offset( 0, duration );	
		DrawLine( cxt, FPointF(0,0), FPointF(sz.width,0), 1, br );
	}
	mat.PopTransform();
	cxt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}


static CComBSTR Hex( wchar_t ch )
{
	WCHAR cb[12];
	wsprintf(cb,L"%X", ch );
	return cb;
}


DLLEXPORT BSTR WINAPI XmlUtf8Encode( LPCWSTR str )
{
	V4::StringStream encoded_str;	
	int len = lstrlenW( str );
	WCHAR* p = (WCHAR*)str;
	while( *p )
	{
		if ( *str <= 0x2b || *str == 0x3d )
		{
			if ( *p == '\'' ){ encoded_str << L"&apos;";}
			else if ( *p == '"' ){ encoded_str << L"&quot;"; }
			else if ( *p == '&' ){ encoded_str << L"&amp;";  }			  
			else if ( *p == '<' ){ encoded_str << L"&lt;"; }			  
			else if ( *p == '>' ){ encoded_str << L"&gt;"; }			  
			else
				encoded_str << *p;
		}		
		else
			encoded_str << *p;
		p++;
	}

	return encoded_str.ToString();
}

DLLEXPORT BSTR WINAPI InetPercentEncode( LPCWSTR str )
{
	// http://www.platypus.st/home/nosuzuki/sample_code/URLencode.js.txt
	// Unicode to URL encoded UTF-8

	int i;
	wchar_t char_code;

	
	V4::StringStream encoded_str;
	
	int len = lstrlenW( str );
 
	for (i = 0; i < len; i++)
	{
		char_code = str[i];
		if (char_code == 0x20)
		{
			// space -> "+"
			encoded_str << L'+';
		}
		else 
		{ // else 1
			if (((0x30 <= char_code) && (char_code <= 0x39)) || ((0x41 <= char_code) && (char_code <= 0x5a)) || ((0x61 <= char_code) && (char_code <= 0x7a)))
			{
				// [0-9a-z-A-Z]
				// no escape
				encoded_str << str[i];
			}
			else if ((char_code == 0x2a) || (char_code == 0x2e) || (char_code == 0x2d) || (char_code == 0x5f)) 
			{
				// [.-_]
				 // no escape
				 encoded_str << str[i];
			}
			else 
			{ // else 2
				// for internal unicode to UTF-8
				// Ref. http://homepage3.nifty.com/aokura/jscript/utf8.html
				// Ref. http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm

				//if ( char_code > 0xffff ) 
				//{
				//	encoded_str += L"%"; encoded_str+= Hex((char_code >> 18) | 0xf0);
				//	encoded_str += L"%"; encoded_str+= Hex(((char_code >> 12) & 0x3f) | 0x80);
				//	encoded_str += L"%"; encoded_str+= Hex(((char_code >> 6) & 0x3f) | 0x80);
				//	encoded_str += L"%"; encoded_str+= Hex((char_code & 0x3f) | 0x80);
				//}
				//else 
				if ( char_code > 0x7ff ) 
				{
					encoded_str << L'%'; encoded_str << (LPCWSTR)Hex((char_code >> 12) | 0xe0);
					encoded_str << L'%'; encoded_str << (LPCWSTR)Hex(((char_code >> 6) & 0x3f) | 0x80);
					encoded_str << L'%'; encoded_str << (LPCWSTR)Hex((char_code & 0x3f) | 0x80);
				}
				else if ( char_code > 0x7f ) 
				{
					encoded_str << L'%'; encoded_str << (LPCWSTR)Hex(((char_code >> 6) & 0x1f) | 0xc0);
					encoded_str << L'%'; encoded_str << (LPCWSTR)Hex((char_code & 0x3f) | 0x80);
				}
				else 
				{
					// for ascii
					V4::StringStream padded_str;
					padded_str << (LPCWSTR)Hex(char_code);
					
					
					BSTR bs = padded_str.ToString();
					
					auto len = ::SysStringLen(bs);

					WCHAR x[3];
					const WCHAR* cb = (LPCWSTR)bs; //padded_str;
					x[0] = cb[len-2];
					x[1] = cb[len-1];
					x[2] = 0;


					encoded_str << L'%'; 
					encoded_str << x;

					::SysFreeString(bs);

					//encoded_str+= padded_str.Mid(padded_str.GetLength() - 2, 2);
					
				}
			} // else 2
		} // else 1
	} // for
	return encoded_str.ToString();

}


// 1000->1,000
DLLEXPORT BSTR WINAPI StringToMoneyString( LPCWSTR cmoney )
{
	if ( cmoney )
	{
		auto f = []( LPCWSTR money,int len )->std::wstring
		{
			int y[] = {0,0,0,0,1,2,3};			
			WCHAR cb[16];
			memset(cb,0,sizeof(cb));

			int xx = len;

			int j = 0;

			while( len > 3 )
			{
				while ( xx >= sizeof(y)/sizeof(int) )
					xx -= 3;

				int yy = y[xx];

				cb[ yy + j*4 ] = L',';

				j++;

				if ( yy+j*4 > len )
					break;		
			}		
			
			for( int i = 0, j=0; i < len ; i++ )
			{
				if ( cb[j] == ',' )
					j++;

				cb[j++] = money[i];
			}

			return cb;
		};

		LPCWSTR ph = cmoney;
		LPCWSTR ph2 = 0;
		
		int len = 0;

		while( *ph != 0 && *ph != '.' )
		{
			ph++;
			len++;
		}

		if ( *ph == '.' )
			ph2 = ph;
		
		std::wstring ret;
		ph = cmoney;

		if ( *ph == L'-' || *ph == L'+' )		
			ret = *ph + f(cmoney+1, len-1);
		else
			ret = f(cmoney, len );

		if ( ph2 )
			ret += ph2;

		return ::SysAllocString( ret.c_str());


	}		
	
	return ::SysAllocString(L"");
}





}