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
#include "s1.h"
#include "JsrtWrapper.h"

//using namespace V4;
//int main()
//{
//	auto r = JSRuntime::CreateRuntime();
//
//	auto cxt = r.CreateContext();
//
//	{
//		JSContext::Scope ct(cxt);
//
//		JsValueRef gl = cxt.Global();
//
//		DefineHostCallback(gl, L"echo", Echo, nullptr);
//
//		try
//		{
//			cxt.Run(L"\n\nvar a = 1+100w; echo(a); ");
//		}
//		catch (ExceptionScriptRun ex1)
//		{
//
//		}
//		catch (ExceptionScriptCompile ex1)
//		{
//			std::wstring a = ex1.message_.bstrVal;
//		}
//	}
//	r.GavageCollect();
//	r.Dispose();
//	return 0;
//}

using namespace V4;

namespace CHAKRA2 {
	void Initialize(JSRuntime& grun, JSContext& rcxt);
	bool JavaScriptRun(JSContext& r, LPCWSTR src);
	void Destroy(JSRuntime);

	wstring ErrorMsg;
};



namespace CHAKRA2 {
void Initialize(JSRuntime& grun, JSContext& rcxt)
{
	auto r = JSRuntime::CreateRuntime();
	
	JSContext cxt = r.CreateContext();
	
	
	JSContext::Scope ct(cxt);

	JsValueRef gl = cxt.Global();

	rcxt = cxt;
	
	grun = r;

	



}
	
bool JavaScriptRun(JSContext& cxt, LPCWSTR src)
{

	try
	{
		//auto cxt = gcxt; // r.CreateContext();
		//JSContext::Scope ct(cxt);

		JsSetCurrentContext(cxt.Handle());

		cxt.Run( src );


		return true;
	}
	catch (ExceptionScriptRun ex1)
	{
		if ( ex1.message_.vt == VT_NULL || ex1.message_.vt == VT_EMPTY )
			ErrorMsg = L"エラー発生";
		else
			ErrorMsg = ex1.message_.bstrVal;
	}
	catch (ExceptionScriptCompile ex1)
	{
		ErrorMsg = ex1.message_.bstrVal;
	}

	return false;
}

void Destroy(JSRuntime r)
{
	JsSetCurrentContext(0);

	r.GavageCollect();
	r.Dispose();
}

}



