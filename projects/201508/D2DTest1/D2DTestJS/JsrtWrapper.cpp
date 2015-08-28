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
