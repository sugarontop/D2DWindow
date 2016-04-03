#include "stdafx.h"
#include "CJsValueRef.h"


CJsValueRef::CJsValueRef(int val)
{
	JsIntToNumber(val,&ref_);
	//AddRef();
}
CJsValueRef::CJsValueRef(double val)
{
	JsDoubleToNumber(val,&ref_);
	//AddRef();
}
CJsValueRef::CJsValueRef(LPCWSTR str)
{
	JsPointerToString(str,lstrlen(str), &ref_);
	AddRef();
}
CJsValueRef::CJsValueRef(bool bl)
{
	JsBoolToBoolean( bl, &ref_ );
	//AddRef();
}
CJsValueRef::CJsValueRef(const std::vector<CJsValueRef>& ar)
{
	JsCreateArray( ar.size(), &ref_ );
	AddRef();
	
	int i = 0;
	for( auto& it : ar )
	{
		JsValueRef  index;
		JsIntToNumber(i++, &index );
		JsSetIndexedProperty( ref_, index, it.ref_ );
	}
	
}
CJsValueRef::CJsValueRef( const std::map<std::wstring,CJsValueRef>& m)
{
	JsCreateArray( m.size(), &ref_ );
	AddRef();
	
	JsValueRef jsobj;
	JsConvertValueToObject(ref_, &jsobj );
	JsValueRef xnm;
	JsGetOwnPropertyNames( jsobj, &xnm );
		
	int i = 0;	
	for( auto& it : m )
	{
		auto& key = it.first;

		JsValueRef  index;
		JsIntToNumber(i++, &index);
		
		JsValueRef ck;
		JsPointerToString( key.c_str(),key.length(), &ck );
					
		auto er = JsSetIndexedProperty(xnm, index, ck); // set xnm.
		_ASSERT( er == JsNoError);	

		JsValueRef IndexedProperty;
		er = JsGetIndexedProperty(xnm, index, &IndexedProperty);
		_ASSERT( er == JsNoError);	

		er = JsSetIndexedProperty( jsobj, IndexedProperty, it.second.ref_ ); // set jsobj.
		_ASSERT( er == JsNoError);	
	}

}
/*
CJsValueRef::CJsValueRef( const std::map<std::wstring,CJsValueRef>& m)
{
	JsCreateArray( m.size(), &ref_ );
	AddRef();
	
	JsValueRef jsobj;
	JsConvertValueToObject(ref_, &jsobj );
	JsValueRef xnm;
	JsGetOwnPropertyNames( jsobj, &xnm );
	JsValueRef lengthName,lengthValue;
	auto ret = JsGetPropertyIdFromName(L"length", &lengthName);

	int count = (int)m.size();
	JsIntToNumber( count,&lengthValue);

	ret = JsSetProperty( xnm, lengthName, lengthValue, true);
	
	int i = 0;	
	for( auto& it : m )
	{
		auto& key = it.first;

		JsValueRef  index;
		JsIntToNumber(i++, &index);
		
		JsValueRef ck;
		JsPointerToString( key.c_str(),key.length(), &ck );
					
		ret = JsSetIndexedProperty(xnm, index, ck); // set xnm.
	}

	for (i = 0; i < count; i++)
	{		
		JsValueRef  index;
		JsIntToNumber(i, &index);

		JsValueRef IndexedProperty;
		ret = JsGetIndexedProperty(xnm, index, &IndexedProperty);

		JsValueRef itemref;
		JsConvertValueToString(IndexedProperty, &itemref);

		WCHAR* cb; UINT len;
		JsStringToPointer(itemref, (const wchar_t**) &cb, &len);
				
		auto& key = std::wstring(cb,len);
		CJsValueRef val = m.find(key)->second;
		
		auto er = JsSetIndexedProperty( jsobj, IndexedProperty, val.ref_ ); // set jsobj.
		_ASSERT( er == JsNoError);	
	}
	
}
*/

CJsValueRef::CJsValueRef( JsValueRef ref )
{
	ref_ = ref;
	AddRef();
}
CJsValueRef::CJsValueRef( const CJsValueRef& r )
{
	ref_ = r.ref_;
	AddRef();
}

UINT CJsValueRef::AddRef()
{ 
	UINT refcnt = 0; 
	JsAddRef( ref_, &refcnt ); 


	//TRACE( L"%x cnt=%d\n", this, refcnt );

	return refcnt; 
}

UINT CJsValueRef::Release()
{
	// Heap上にある場合、
	// 遅くてもWM_DESTRORYのタイミングが望ましい。
	// アプリ終了のDestructorでの実行は別のエラーになる。


	UINT refcnt = 0;
	if ( ref_ )
		JsRelease( ref_, &refcnt );

	ref_ = nullptr;
	return refcnt;
}
CJsValueRef& CJsValueRef::operator=( const CJsValueRef& x )
{
	if ( this == &x ) return *this;

	Release();
	ref_ = x.ref_;
	AddRef();

	return *this;
}

CJsValueRef::JSTYPE CJsValueRef::Type()
{
	JsValueType ty;
	auto t = JsGetValueType(ref_, &ty ); // 8: JsArray

	switch( t )
	{
		case JsArray:
			return JSTYPE_ARRAY;
		break;
		case JsObject:
			return JSTYPE_OBJ;
		break;			
		case JsNumber:
			return JSTYPE_NUMBER;
		break;
		case JsString:
			return JSTYPE_STRING;
		break;
		case JsFunction:
			return JSTYPE_FUNCTION;
		break;
		case JsUndefined:
			return JSTYPE_UNDEF;
		break;
		case JsNull:
			return JSTYPE_NULL;
		break;
		case JsBoolean:
			return JSTYPE_BOOL;
		break;
	}
	return JSTYPE_NULL;
}

std::vector<CJsValueRef> CJsValueRef::ToArray()
{
	//_ASSERT( Type() == JSTYPE::JSTYPE_ARRAY );
	std::vector<CJsValueRef> ar;
	JsValueRef jsobj;
	if ( JsNoError == JsConvertValueToObject(ref_, &jsobj ))
	{
		JsValueRef xnm;
		JsGetOwnPropertyNames( jsobj, &xnm );
		JsValueRef lengthName,lengthValue;
		auto ret = JsGetPropertyIdFromName(L"length", &lengthName);
		ret = JsGetProperty(xnm, lengthName, &lengthValue);
		int count;
		JsNumberToInt(lengthValue, &count);
	
		if ( count )
		{		
			ar.resize( count-1);

			for( int i = 0; i < count-1; i++ )
			{
				JsValueRef  index;
				JsIntToNumber(i, &index );

				JsValueRef IndexedProperty;
				JsGetIndexedProperty(jsobj, index, &IndexedProperty );

				ar[i] = CJsValueRef(IndexedProperty);		
			}
		}
	}
	return ar;

}
int CJsValueRef::ToInt()
{
	int ret=0;
	JsValueRef a;
	JsConvertValueToNumber( ref_, &a );
	JsNumberToInt( a, &ret );

	return ret;
}
double CJsValueRef::ToDouble()
{
	double ret=0;
	JsValueRef a;
	JsConvertValueToNumber( ref_, &a );
	JsNumberToDouble( a, &ret );

	return ret;
}
std::wstring CJsValueRef::ToString()
{
	JsValueRef a;
	JsConvertValueToString( ref_, &a );

	const wchar_t* cb; size_t len;
	JsStringToPointer(a, &cb,&len );

	return std::wstring( cb,len);
}
std::map<std::wstring,CJsValueRef> CJsValueRef::ToMap()
{
	auto keys = ToMapKeys();

	int count = (int)keys.size();
	std::map<std::wstring,CJsValueRef> map;
	JsValueRef jsobj;
	auto ret = JsConvertValueToObject(ref_, &jsobj );

	for (int i = 0; i < count; i++)
	{
		JsValueRef  index;
		JsPointerToString(keys[i].c_str(), keys[i].length(), &index);

		JsValueRef IndexedProperty;
		JsGetIndexedProperty(jsobj, index, &IndexedProperty );

		map[ keys[i] ] = CJsValueRef(IndexedProperty);
	}

	return map;
}
std::vector<std::wstring> CJsValueRef::ToMapKeys()
{
	std::vector<std::wstring> keys;
	JsValueRef jsobj;
	if ( JsNoError == JsConvertValueToObject(ref_, &jsobj ))
	{
		JsValueRef xnm;
		JsGetOwnPropertyNames( jsobj, &xnm );
		JsValueRef lengthName,lengthValue;
		auto ret = JsGetPropertyIdFromName(L"length", &lengthName);
		ret = JsGetProperty(xnm, lengthName, &lengthValue);
		int count;
		JsNumberToInt(lengthValue, &count);	

	

		for (int i = 0; i < count; i++)
		{
			JsValueRef  index;
			JsIntToNumber(i, &index);
			JsValueRef IndexedProperty;
			ret = JsGetIndexedProperty(xnm, index, &IndexedProperty);

			JsValueRef itemref;
			JsConvertValueToString(IndexedProperty, &itemref);

			WCHAR* cb; UINT len;
			JsStringToPointer(itemref, (const wchar_t**) &cb, &len);

			keys.push_back(cb);
		}
	}
	return keys;
}
//std::map<std::wstring,CJsValueRef> CJsValueRef::ToMap()
//{
//	std::map<std::wstring,CJsValueRef> map;
//	JsValueRef jsobj;
//	auto ret = JsConvertValueToObject(ref_, &jsobj );
//	
//	JsValueRef xnm;
//	JsGetOwnPropertyNames( jsobj, &xnm );
//	JsValueRef lengthName,lengthValue;
//	ret = JsGetPropertyIdFromName(L"length", &lengthName);
//	ret = JsGetProperty(xnm, lengthName, &lengthValue);
//	int count;
//	JsNumberToInt(lengthValue, &count);	
//
//	std::vector<std::wstring> si;
//
//	for (int i = 0; i < count; i++)
//	{
//		JsValueRef  index;
//		JsIntToNumber(i, &index);
//		JsValueRef IndexedProperty;
//		ret = JsGetIndexedProperty(xnm, index, &IndexedProperty);
//
//		JsValueRef itemref;
//		JsConvertValueToString(IndexedProperty, &itemref);
//
//		WCHAR* cb; UINT len;
//		JsStringToPointer(itemref, (const wchar_t**) &cb, &len);
//
//		si.push_back(cb);
//	}
//
//	for (int i = 0; i < count; i++)
//	{
//		JsValueRef  index;
//		JsPointerToString(si[i].c_str(), si[i].length(), &index);
//
//		JsValueRef IndexedProperty;
//		JsGetIndexedProperty(jsobj, index, &IndexedProperty );
//
//		map[ si[i] ] = IndexedProperty;
//	}
//
//	return map;
//}