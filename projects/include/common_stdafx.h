#pragma once



#include "targetver.h"
//#define _WIN32_WINNT	_WIN32_WINNT_WIN7


#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>

#undef DrawText
#undef DrawRect
#undef FillRect

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlcom.h>



#ifdef USE_ID2D1DEVICECONTEXT

	#ifdef USE_WIN8_METRO_CONTEXT
		#include <d3d11_2.h>
		#include <d2d1_2.h>		
		#include <dwrite_2.h>
		#include <d2d1_2helper.h>
		#include <d2d1effects_1.h>
	#else
		#include <d3d11_1.h>
		#include <d2d1_1.h>
		#include <dwrite_1.h>
		#include <d2d1_1helper.h>
		
	#endif
#else
	#include <d2d1_1.h>
	#include <dwrite_1.h>
	#include <d2d1helper.h>
#endif







// ---windows共通----------------------
#include <string>
#include <stack>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include <fcntl.h>
#include <io.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <sstream>
#include <codecvt>
#include <dispex.h>
#include <limits.h>
#include <functional>
#include <memory>
#include <type_traits>
//#include <fstream>

#include <imm.h>
#include <msctf.h>

#include <msxml6.h>
#define MSXML	L"MSXML2.DOMDocument.6.0"	// CLSID clsid; hr = CLSIDFromProgID( MSXML, &clsid );CComPtr<IXMLDOMDocument> xml;	hr = CoCreateInstance( clsid, NULL,CLSCTX_ALL,IID_IXMLDOMDocument,(void**)&xml );
#define MSXMLHTTP L"MSXML2.XMLHTTP.6.0"	// 自動キャッシュが効く　CComPtr<IXMLHTTPRequest> req; CLSIDFromProgID( MSXMLHTTP, &clsid ); hr = CoCreateInstance( clsid,NULL,CLSCTX_INPROC_SERVER,IID_IXMLHTTPRequest,(LPVOID*)&req );

#include <comutil.h>

//metro非対応////////////////////////////////////////
//#include <atlenc.h>
//#include <WinCrypt.h>
#include <mmsystem.h>

//#include "xcom.h"


//#include "D2DMisc.h"
//#include "D2DMisc2.h" // "FString.h"

#include "FString.h"

#define DEFAULTFONT_US	L"Verdana"
#define DEFAULTFONT_HEIGHT_US	12

#ifndef DEFAULTFONT_JP
#define DEFAULTFONT_JP	L"メイリオ"
#define DEFAULTFONT_HEIGHT_JP	12
#endif



#define DEFAULTFONT DEFAULTFONT_JP
#define DEFAULTFONT_HEIGHT DEFAULTFONT_HEIGHT_JP

#define DEFAULTFONT_HEIGHT_WITH_SPACE	18
#define _TAB 9
#define _CR 13
#define _LF 10
#define _PAI 3.14159f
#define CRLF L"\r\n"

#define MAJAR_VERSION 1
#define MINOR_VERSION 0


#define NONAME L"__NONAME"
#define OUT




#ifdef _DEBUG
	#define TRACE FString::debug_trace
#else
	#define TRACE //
#endif

#define HR(hr)	(hr == S_OK)



#pragma warning(disable:4482) // C4482 enumのms拡張
#pragma warning (disable:4250)
#pragma warning (disable:4244)


#define ENABLE_HWND_TEXTBOX

#define TEMP_INVALID_VAL 100000




#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define xassert(x)	_ASSERT(x)
// VARIANT は plValを使用すること
#define VT_F_LPVOID	(VT_BYREF|VT_I4)


using namespace std::placeholders;
typedef _bstr_t bstring;