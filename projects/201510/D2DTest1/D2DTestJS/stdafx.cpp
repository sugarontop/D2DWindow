// stdafx.cpp : 標準インクルード D2DTestJS.pch のみを
// 含むソース ファイルは、プリコンパイル済みヘッダーになります。
// stdafx.obj にはプリコンパイル済み型情報が含まれます。

#include "stdafx.h"


#pragma comment (lib,"d2dcommon.lib")
#pragma comment (lib,"d2dwin.lib")


#ifdef USE_EDGEMODE_JSRT
#pragma comment (lib,"chakrart.lib")
#else
#pragma comment(lib, "jsrt.lib")
#endif


#pragma comment( lib, "ws2_32.lib" )