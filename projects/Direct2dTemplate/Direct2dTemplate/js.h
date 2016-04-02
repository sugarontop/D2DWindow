#pragma once
#include "CJsValueRef.h"

CJsValueRef JsRun(LPCWSTR script);
double JsRunInitial( HWND hMainWnd );
void JsClose();



typedef JsValueRef (CALLBACK *JSFUNCTION)(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, _In_opt_ void *callbackState);

// 関数名は大文字小文字あり
void RegistFunction( LPCWSTR function_name, JSFUNCTION func );