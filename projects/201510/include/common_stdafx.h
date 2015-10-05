#pragma once

// DX12
//#include <d3d12.h>
//#include <d3d11_3.h>
//#include <dxgi1_4.h>


#include <d2d1_2.h>
#include <dwrite_2.h>
#include <d3d11_2.h>
#include <d2d1_2helper.h>
#include <d2d1effects_1.h>

#include <memory>
#include <crtdbg.h>
#include <functional>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <comutil.h>
#include <random>
#include <DispEx.h> // for IDispatchEx

#include "comptr.h"
#include "d2dmisc.h"
#include "gdi32.h"

#include "d2dcontextnew.h"
#include "fstring.h"
#include "binary.h"

#ifdef _DEBUG
	#define TRACE FString::debug_trace
#else
	#define TRACE
#endif


#define HR(hr)	(hr == S_OK)
#define xassert(exp)	_ASSERT(exp)

#define NONAME L"__NONAME"
#define OUT
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define D2RGBA(r,g,b,a) ColorF(r/255.0f, g/255.0f, b/255.0f, a/255.0f ) //  light(0) <- alpah <- deep(255)
#define D2RGB(r,g,b) ColorF(r/255.0f, g/255.0f, b/255.0f, 1.0f )
#define D2DRGB(dw) ColorF(((dw&0xFF0000)>>16)/255.0f, ((dw&0x00FF00)>>8)/255.0f, (dw&0xFF)/255.0f, 1.0f )

#define D2DRGBA(dw) ColorF(((dw&0xFF000000)>>24)/255.0f, ((dw&0x00FF0000)>>16)/255.0f, ((dw&0xFF00)>>8)/255.0f, (dw&0xFF)/255.0f )

#define D2DRGBADWORD(R,G,B,A) (LONG)((R<<24)+(G<<16)+(B<<8)+A)

#define DEFAULTFONT	L"Segoe UI"
#define DEFAULTFONT_HEIGHT	12

#define _PAI 3.14159f
#define _weak


using namespace std::placeholders;

namespace V4 {

class D2DWindow;
class D2DControl;
class D2DControls;

};





