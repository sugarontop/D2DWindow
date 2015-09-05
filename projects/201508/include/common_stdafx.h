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

#define DEFAULTFONT	L"メイリオ"
#define DEFAULTFONT_HEIGHT	12

#define _PAI 3.14159f

using namespace std::placeholders;

namespace V4 {

class D2DWindow;
class D2DControl;
class D2DControls;

};





