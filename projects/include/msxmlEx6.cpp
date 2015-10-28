/*
The MIT License (MIT)
Copyright (c) 2015 admin@sugarontop.net
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
#include "msxmlex6.h"

#define __MSXML	L"MSXML2.DOMDocument.6.0"	// CLSID clsid; hr = CLSIDFromProgID( MSXML, &clsid );CComPtr<IXMLDOMDocument> xml;	hr = CoCreateInstance( clsid, NULL,CLSCTX_ALL,IID_IXMLDOMDocument,(void**)&xml );
//#define MSXMLHTTP L"MSXML2.XMLHTTP.6.0"	// 自動キャッシュが効く　CComPtr<IXMLHTTPRequest> req; CLSIDFromProgID( MSXMLHTTP, &clsid ); hr = CoCreateInstance( clsid,NULL,CLSCTX_INPROC_SERVER,IID_IXMLHTTPRequest,(LPVOID*)&req );

namespace V4 {

bool XMLDOMDocument6::Create()
{
	if ( doc == NULL )
	{
		if ( S_OK != doc.CreateInstance( __uuidof(MSXML2::DOMDocument60) ))
			return false;
	}
	return true;
}
bool XMLDOMDocument6::Load(LPCWSTR url )
{
	if ( !Create() )
		return false;
	
	doc->put_async(VARIANT_FALSE);

	VARIANT_BOOL bl = doc->load(_variant_t(url));

	return ( bl == VARIANT_TRUE );
}
bool XMLDOMDocument6::LoadXML(BSTR xml )
{
	if ( !Create() )
		return false;
	
	VARIANT_BOOL bl = VARIANT_FALSE;

	doc->put_async(bl);
	bl = doc->loadXML(xml);

	return ( bl == VARIANT_TRUE );
}

long XMLDOMDocument6::ParsePropertys( MSXML2::IXMLDOMNode* nd, std::map<std::wstring,std::wstring>& ret )
{	
	MSXML2::IXMLDOMNamedNodeMapPtr m;
	long len = 0;
		
	if ( S_OK == nd->get_attributes(&m) )
	{
		m->get_length(&len);

		for( long i = 0; i <len; i++ )
		{
			MSXML2::IXMLDOMNodePtr x;
			m->get_item( i, &x );
			
			_bstr_t key = x->GetnodeName();
			_variant_t val = x->GetnodeValue();		
			
			_ASSERT( val.vt == VT_BSTR );	
						
			ret[ (LPCWSTR)key ] = (LPCWSTR)val.bstrVal;
		}
	}
	return len;
}

long XMLDOMDocument6::ParseList( MSXML2::IXMLDOMNodeList* ndls, std::vector<MSXML2::IXMLDOMNodePtr>& ls )
{
	long len = 0;
	ndls->get_length(&len);
	for( long i = 0; i <len; i++ )
	{
		MSXML2::IXMLDOMNodePtr nd;
		ndls->get_item(i, &nd);
		ls.push_back(nd);
	}
	return len;
}

//static void ParseList( IXMLDOMNode* nd, std::vector<IXMLDOMNode*>& ls );


std::vector<_bstr_t> parse_candidate( _bstr_t s )
{
	std::vector<_bstr_t> ar;

	WCHAR buf[256];
	LPWSTR b = buf;

	LPCWSTR p = s;

	if ( *p++ == '[' )
	{		
		while( *p )
		{
			if ( *p == ',' || *p == ']')
			{
				*b = 0;
				ar.push_back( buf );
				b = buf;
				p++;
			}
			else if ( *p == '\'' )
			{
				p++;
				while( *p != '\'' ){*b++ = *p++;}
				p++;
			}
			else
			{
				*b++ = *p++;
			}
		}
	}
	return ar;
}

_bstr_t GetCurrentDirectory()
{
	WCHAR buf[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buf);
	return buf;
}

};
