
#undef DrawText
#undef DrawRect
#undef FillRect

#include <atlbase.h>
#include <atlcom.h>

#include <d2d1.h>
#include <dwrite.h>
#include <d2d1helper.h>

#include <string>
#include <stack>
#include <sstream>
#include <vector>
#include <map>
#include <dispex.h>
#include <limits.h>
#include <functional>
#include <memory>
#include <type_traits>

using namespace std::placeholders;
//typedef _bstr_t bstring;


#define _TAB 9
#define _CR 13
#define _LF 10
#define _PAI 3.14159f
#define CRLF L"\r\n"

#define HR(hr)	(hr == S_OK)

#define DEFAULTFONT_JP	L"ÉÅÉCÉäÉI"
#define DEFAULTFONT_HEIGHT_JP	32


#define DEFAULTFONT			DEFAULTFONT_JP
#define DEFAULTFONT_HEIGHT	DEFAULTFONT_HEIGHT_JP


#include "D2DMisc.h"







