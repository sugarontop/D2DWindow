#pragma once


#include "ChakraCore.h"

class CJsValueRef
{	
	public :
		CJsValueRef():ref_(nullptr){}
		explicit CJsValueRef( JsValueRef ref ) ;
		CJsValueRef( const CJsValueRef& r );
		~CJsValueRef(){	Release(); }

		CJsValueRef(int val);
		CJsValueRef(double val);
		CJsValueRef(LPCWSTR str);
		CJsValueRef(bool bl);
		CJsValueRef(const std::vector<CJsValueRef>& ar);
		CJsValueRef(const std::map<std::wstring,CJsValueRef>& m);


		enum JSTYPE{ JSTYPE_NULL,JSTYPE_ARRAY,JSTYPE_STRING,JSTYPE_NUMBER,JSTYPE_OBJ,JSTYPE_UNDEF,JSTYPE_BOOL,JSTYPE_FUNCTION,JSTYPE_MAP };
		JSTYPE Type();

		UINT AddRef();
		UINT Release();

		std::vector<CJsValueRef> ToArray();
		int ToInt();
		double ToDouble();
		std::wstring ToString();
		std::map<std::wstring,CJsValueRef> ToMap();
		std::vector<std::wstring>ToMapKeys();

		bool empty(){ return (ref_ == nullptr); }

				
		CJsValueRef& operator=( const CJsValueRef& x );
		
		operator JsValueRef(){ return ref_; }
	private :
		JsValueRef ref_;

};