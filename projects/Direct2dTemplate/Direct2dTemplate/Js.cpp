#include "stdafx.h"
#include "ChakraCore.h"
#include "CJsValueRef.h"
#include "js.h"

#pragma comment( lib, "chakracore")

#define INITIAL_FILE L"js/initial.js"

static unsigned currentSourceContext = 0;
static JsRuntimeHandle runtime = nullptr;
static JsContextRef jscontext = nullptr;
static JsValueRef ghostObject;
static JsContextRef AppInit();
static HWND s_hMainWnd;

JsValueRef CALLBACK msgbox(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState);
JsValueRef CALLBACK CreateCard(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState);

static bool ReadScriptFile_ANSI( LPCWSTR fnm, BSTR* ret );

CJsValueRef JsRun(LPCWSTR script)
{
	JsValueRef result;

	JsSetCurrentContext(jscontext);
	
	JsErrorCode errorCode = JsRunScript(script, currentSourceContext++, L"", &result);


	if ( JsNoError != errorCode )
	{
		throw errorCode;
	}

	return CJsValueRef(result);
}

bool JsAppInit( HWND hMainWnd )
{
	s_hMainWnd = hMainWnd;
	bool bl = false;
	jscontext = AppInit();
	 
	BSTR bs;

	if ( ReadScriptFile_ANSI(INITIAL_FILE, &bs )) // *.js
	{

		JsRun( bs );
			
		::SysFreeString(bs);
		bl = true;
	}

	return bl;
}
void JsAppClose()
{
	if ( runtime )
	{		
		JsCollectGarbage(runtime);		
		JsSetCurrentContext(JS_INVALID_REFERENCE);
		JsDisposeRuntime(runtime);		
		runtime = nullptr;
	}
}

#define IfFailRet
#define IfFailError( func,msg)

JsErrorCode DefineHostCallback(JsValueRef globalObject, const wchar_t *callbackName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	IfFailRet(JsGetPropertyIdFromName(callbackName, &propertyId));

	JsValueRef function;
	IfFailRet(JsCreateFunction(callback, callbackState, &function));

	IfFailRet(JsSetProperty(globalObject, propertyId, function, true));

	return JsNoError;
}
JsErrorCode CreateHostContext(JsRuntimeHandle runtime, int argc, wchar_t *argv [], int argumentsStart, JsContextRef *context)
{	
	JsCreateContext(runtime, context);
	
	JsSetCurrentContext(*context);

	
	JsValueRef hostObject;
	JsCreateObject(&hostObject);
	ghostObject = hostObject;

	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);
	
	JsPropertyIdRef hostPropertyId;
	JsGetPropertyIdFromName(L"app", &hostPropertyId); // app.XXXXX 

	JsSetProperty(globalObject, hostPropertyId, hostObject, true);

	//IfFailRet(DefineHostCallback(hostObject, L"msgbox", msgbox, nullptr));

	JsSetCurrentContext(JS_INVALID_REFERENCE);

	return JsNoError;
}

void RegistFunction( LPCWSTR function_name, JSFUNCTION func )
{
	JsSetCurrentContext(jscontext);

	IfFailRet(DefineHostCallback(ghostObject, function_name, func, nullptr));

	JsSetCurrentContext(JS_INVALID_REFERENCE);
}



static JsContextRef AppInit()
{
	JsContextRef context;
	int argc = 1;
	WCHAR* argv[1];
	argv[0] = L"dumy";
	
	JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);
	CreateHostContext(runtime, argc,argv,1,&context);

	JsSetCurrentContext(context);


	return context;
}

//JsValueRef CALLBACK msgbox(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState)
//{
//	for (unsigned int index = 1; index < argumentCount; index++)
//	{
//		CJsValueRef v = CJsValueRef(arguments[index]);
//		auto s = v.ToString();
//	}
//
//	return JS_INVALID_REFERENCE;
//}



LPWSTR ANSIToUNICODE(LPCSTR str, int strlen )		
{
	LPCSTR vs = str;
	int len = strlen;
	int cchMultiByte = ::MultiByteToWideChar(CP_ACP, 0, vs,len, NULL, 0);
	LPWSTR lpa = new WCHAR [cchMultiByte+1];
	int nMultiCount = ::MultiByteToWideChar(CP_ACP, 0, vs, len, lpa, cchMultiByte);
	lpa[cchMultiByte] = 0;
	return lpa;
}

bool ReadScriptFile_ANSI( LPCWSTR fnm, BSTR* ret )
{
	HANDLE pf = CreateFile( fnm, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL );
	
	if ( INVALID_HANDLE_VALUE == pf ) return false;
	
	DWORD len = GetFileSize( pf, NULL );

	BYTE* cb = new BYTE[len];

	DWORD lx=0; 
	ReadFile( pf, cb, len, &lx,0);
	
	LPCWSTR xs = (LPCWSTR)ANSIToUNICODE( (LPCSTR)cb, len );
	
	*ret = ::SysAllocString(xs);
	
	delete []xs;
	delete [] cb;
	
	::CloseHandle( pf );
	
	return true;
}


