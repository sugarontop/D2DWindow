// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 一部の CString コンストラクターは明示的です。

#include <atlbase.h>
#include <atlstr.h>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#undef DrawText
#undef FillRect
#undef FillRoundRect


#include "targetver.h"

#include "common_stdafx.h"

#define JSRUN_CLIENT
#define JSCRIPT_ENGINE_VERSION 9


#include "D2DMisc.h"


#define BS(a)	CComBSTR(L##a)

/*
#include <string>
#include <stack>
#include <sstream>
#include <vector>
#include <map>

#include <fcntl.h>
#include <io.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <sstream>
#include <codecvt>


#include <limits.h>
#include <boost\shared_ptr.hpp>
#include <boost\bind.hpp>
#include <boost\function.hpp>
#include <boost\scoped_array.hpp>
#include <boost\shared_array.hpp>
#include "FDirect2d_Misc.h"
#include<type_traits>

#include<atlcom.h>*/

#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN7




#define D2RGBA(r,g,b,a) ColorF(r/255.0f, g/255.0f, b/255.0f, a/255.0f ) //  薄い(0) <- A <- 濃い(255)
#define D2RGB(r,g,b) ColorF(r/255.0f, g/255.0f, b/255.0f, 1.0f )
#define D2DRGB(dw) ColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )

#define _USE_MATH_DEFINES
#include <math.h>

#include <Wincodec.h>