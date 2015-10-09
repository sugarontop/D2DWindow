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

JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState)
{
	//
	// Get property ID.
	//

	JsPropertyIdRef propertyId;
	(JsGetPropertyIdFromName(callbackName, &propertyId));

	//
	// Create a function
	//

	JsValueRef function;
	(JsCreateFunction(callback, callbackState, &function));

	//
	// Set the property
	//

	(JsSetProperty(globalObject, propertyId, function, true));

	return JsNoError;
}




HWND ghWnd;
JsValueRef CALLBACK Echo(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	for (unsigned int index = 1; index < argumentCount; index++)
	{
		if (index > 1)
		{
			wprintf(L" ");
		}

		JsValueRef stringValue;
		(JsConvertValueToString(arguments[index], &stringValue)); //, L"invalid argument");

		const wchar_t *string;
		size_t length;
		(JsStringToPointer(stringValue, &string, &length)); //, L"invalid argument");

		//wprintf(L"%s", string);


		SendMessage(ghWnd, WM_USER,0,(LPARAM)string );
	}

	wprintf(L"\n");

	return JS_INVALID_REFERENCE;
}
//
//void test()
//{
//	ComPTR<IDispatch> factory;
//
//	JsRuntimeHandle runtime;
//	JsCreateRuntime(JsRuntimeAttributeNone, JsRuntimeVersion11, nullptr, &runtime);
//	JsContextRef cxt;
//	JsCreateContext(runtime, nullptr, &cxt);
//	JsSetCurrentContext(cxt);
//
//
//	JsValueRef global;
//	JsGetGlobalObject(&global);
//
//	/*JsPropertyIdRef fc;
//	JsGetPropertyIdFromName(L"factory", &fc);
//	JsValueRef vdisp;
//	JsVariantToValue(&vfactory, &vdisp);
//	JsSetProperty(global, fc, vdisp, true); // C++ IDispatch -> Javascript側
//	*/
//
//	DefineHostCallback(global, L"echo", Echo, nullptr);
//
//
//	JsValueRef result;
//	JsErrorCode errorCode = JsRunScript( L"echo( 'hello world');", JS_SOURCE_CONTEXT_NONE, L"", &result);
//
//
//	JsCollectGarbage(runtime);
//	JsSetCurrentContext(JS_INVALID_REFERENCE);
//	JsDisposeRuntime(runtime);
//
//}


#define ERRORCHECK

namespace CHAKRA {

JsRuntimeHandle gruntime = nullptr;

bool Initialize( std::vector<JSFuntions>& ar )
{
	JsRuntimeHandle runtime;
	//JsCreateRuntime(JsRuntimeAttributeNone, JsRuntimeVersion11, nullptr, &runtime);

#ifdef USE_EDGEMODE_JSRT
	ERRORCHECK(JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime));
#else
	ERRORCHECK(JsCreateRuntime(JsRuntimeAttributeNone, JsRuntimeVersion11, nullptr, &runtime));
#endif


	JsContextRef cxt;
	//JsCreateContext(runtime, nullptr, &cxt);
#ifdef USE_EDGEMODE_JSRT
	JsCreateContext(runtime, &cxt);
#else
	JsCreateContext(runtime, nullptr, &cxt);
#endif
	JsSetCurrentContext(cxt);


	JsValueRef global;
	JsGetGlobalObject(&global);
	
	
	//DefineHostCallback(global, L"echo", Echo, nullptr);

	for( auto& it : ar )
	{
		DefineHostCallback(global, it.name, it.func, nullptr);

	}
	

	gruntime = runtime;
	return true;
}

void JavaScriptRun(BSTR src)
{
	JsValueRef result;
	JsErrorCode res = JsRunScript(src, JS_SOURCE_CONTEXT_NONE, L"", &result);

	if (JsNoError == res) return;

	{
		V4::FString msg;
		switch( res )
		{
			case JsErrorScriptCompile:
				msg = "コンパイルエラー";
			break;
			default :
				msg = V4::FString::Format( L"なんかエラー: %d", res );
		}


		JsValueRef exception;
		JsGetAndClearException(&exception);
			

		throw msg;

	}
}

void Destroy()
{
	if (gruntime )
	{
		JsCollectGarbage(gruntime);
		JsSetCurrentContext(JS_INVALID_REFERENCE);
		JsDisposeRuntime(gruntime);
	}

	gruntime = nullptr;


}

};
