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
#include "JsrtWrapper.h"


/*
usage.
int main()
{
	auto r = JSRuntime::CreateRuntime(JsRuntimeVersion11);

	auto cxt = r.CreateContext();

	{
		JSContext::Scope ct(cxt);

		JsValueRef gl = cxt.Global();

		DefineHostCallback(gl, L"echo", Echo, nullptr);

		try
		{
			cxt.Run(L"\n\nvar a = 1+100w; echo(a); ");
		}
		catch (ExceptionScriptRun ex1)
		{

		}
		catch (ExceptionScriptCompile ex1)
		{
			std::wstring a = ex1.message_.bstrVal;
		}
	}
	r.GavageCollect();
	r.Dispose();
	return 0;
}
*/




namespace V4 
{

//JSRuntime /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void JSRuntime::Dispose()
{
	if (IsValid())
		ERRORCHECK( JsDisposeRuntime(handle_));

	handle_ = JS_INVALID_RUNTIME_HANDLE;
}

void JSRuntime::GavageCollect()
{
	if (IsValid())
		ERRORCHECK(JsCollectGarbage(handle_));
}

void JSRuntime::ErrorCode(JsErrorCode ec)
{
	switch (ec)
	{
		case JsNoError:
			return;
		case JsErrorScriptException:
		{
			JsValueRef exception;
			JsGetAndClearException(&exception);
			throw ExceptionScriptRun(exception);
		}
		break;
		case JsErrorScriptCompile:
		{
			JsValueRef exception;
			JsGetAndClearException(&exception);
			throw ExceptionScriptCompile(exception);
		}
		break;
		case JsErrorFatal:
		{
			JsValueRef exception;
			JsGetAndClearException(&exception);
			throw ExceptionScriptRun(exception);

		}

		default:
			throw -1; // fatal error


	}


	
}

JSRuntime JSRuntime::CreateRuntime()
{
	JsRuntimeHandle r;

#ifdef USE_EDGEMODE_JSRT
	ERRORCHECK(JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &r)); 
#else
	ERRORCHECK(JsCreateRuntime(JsRuntimeAttributeNone, JsRuntimeVersion11, nullptr, &r));
#endif

	
		
	return JSRuntime(r);
}
JSContext JSRuntime::CreateContext()
{
	JsContextRef nc = nullptr;
	
#ifdef USE_EDGEMODE_JSRT
	ERRORCHECK(JsCreateContext(handle_, &nc));
#else
	ERRORCHECK(JsCreateContext(handle_, nullptr, &nc));
#endif
	return JSContext(nc);
}

//JSReference/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT JSReference::AddRef()
{
	UINT cnt;
	ERRORCHECK( JsAddRef( ref_, &cnt));
	return cnt;
}
UINT JSReference::Releaes()
{
	UINT cnt;
	ERRORCHECK(JsRelease(ref_, &cnt));
	return cnt;
}



JsValueRef JSContext::GetAndClearException()
{
	JsValueRef r;
	ERRORCHECK(JsGetAndClearException(&r) );
	
	return r;
}

JSContext JSContext::Current()
{
	JsContextRef ch;
	JsGetCurrentContext(&ch);
	return JSContext(ch);
}

JsValueRef JSContext::Global()
{
	JsValueRef gl;
	ERRORCHECK(JsGetGlobalObject(&gl));
	return gl;
}

void JSContext::Run(LPCWSTR src)
{	
	ERRORCHECK(JsRunScript( src, JS_SOURCE_CONTEXT_NONE, L"", nullptr));

}
//////////////////////////////////////////////////////////////
#pragma comment(lib, "comsuppwd.lib")
_variant_t ConvertRef2String( LPCWSTR nm, JsValueRef ref )
{
	if ( ref == nullptr )
		return variant();

	object obj(ref);
	property_id id = property_id::create(nm);
	wstring n = obj.get_property_native<wstring>(id);

	return variant(n.c_str());
}
_variant_t ConvertRef2Double(LPCWSTR nm, JsValueRef ref)
{
	object obj(ref);
	property_id id = property_id::create(nm);
	double n = obj.get_property_native<double>(id);

	return variant( (int)n );
}


ExceptionScriptRun::ExceptionScriptRun(JsValueRef ref)
{
	message_ = ConvertRef2String( L"message", ref );


}
ExceptionScriptCompile::ExceptionScriptCompile(JsValueRef ref)
{
	message_ = ConvertRef2String(L"message", ref);
	line_ = ConvertRef2Double(L"line", ref);
	column_ = ConvertRef2Double(L"column", ref);
	length_ = ConvertRef2Double(L"length", ref);
	source_  = ConvertRef2String(L"source", ref);


}



};
