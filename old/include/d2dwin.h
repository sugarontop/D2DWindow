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
#pragma once

#ifdef _USRDLL
#define DLLEXPORT extern "C" __declspec( dllexport )
#else
#define DLLEXPORT extern "C"
#endif

#include "D2DMisc.h"


struct D2DWin
{
	HANDLE win;
};
struct D2Ctrls
{
	HANDLE ctrls;
};
struct D2Ctrl
{
	HANDLE ctrl;
};

struct DDRowInfo
{
	int row;
	bool is_selected_row;
	
	int float_row;
	
	V4::FSizeF sz_row;
	const float* col_xpos;
};
struct DDColor
{
	byte r,g,b,a;
};

struct DDImage
{
	V4::FPointF pt;
	V4::FSizeF sz;

	ComPTR<IUnknown> p;
	LPVOID LockedResource;
	DWORD Length;
	
	LPVOID parent;
};

using D2DWinOnEntry = void(*)(D2DWin win, LPVOID prm);
using DDContext = LPVOID;

// ----------------------------------------------------------------------------------------------------------------------
// basic

DLLEXPORT D2Ctrl WINAPI Ctrl(D2Ctrls ctrl);
DLLEXPORT HWND WINAPI D2HWND( D2Ctrl ctrl );
DLLEXPORT HWND WINAPI DDHWND(D2DWin win);
DLLEXPORT void WINAPI DDRedraw( D2Ctrl ctrl );


// ----------------------------------------------------------------------------------------------------------------------
// DDWindow

DLLEXPORT D2DWin WINAPI DDMkWindow(HWND hWnd, D2DWinOnEntry entry, LPVOID prm=nullptr );
DLLEXPORT void WINAPI DDResizeWindow(D2DWin win, int cx, int cy );
DLLEXPORT void WINAPI DDDestroyWindow(D2DWin win );


// ----------------------------------------------------------------------------------------------------------------------
// DDConstrols

DLLEXPORT D2Ctrls WINAPI DDMkTopControls( D2DWin win, const FRectFBM& rc, LPCWSTR name );
DLLEXPORT D2Ctrls WINAPI DDMkControls( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR classnm, LPCWSTR name);
DLLEXPORT D2Ctrls WINAPI DDMkControlsWithTitle( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR classnm, LPCWSTR name);
DLLEXPORT D2Ctrls WINAPI DDMkVerticalStackControls(D2Ctrls ctrls, const FRectFBM& rc,bool order, LPCWSTR classnm, LPCWSTR name);


// ----------------------------------------------------------------------------------------------------------------------
// TAB controls
DLLEXPORT D2Ctrls WINAPI DDMkTabControls( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name, int id=-1 );
DLLEXPORT D2Ctrls WINAPI DDGetPage( D2Ctrls ctrl, int page ); // 0 start
DLLEXPORT D2Ctrls WINAPI DDAddPage( D2Ctrls ctrl, int page ); // newpage : page=-1, insert: page > 0, 



// ----------------------------------------------------------------------------------------------------------------------
// control

DLLEXPORT D2Ctrl WINAPI DDMkButton( D2Ctrls parent, const FRectFBM& rc, LPCWSTR name, int id=-1 );
DLLEXPORT D2Ctrl WINAPI DDMkTextbox( D2Ctrls parent, const FRectFBM& rc, int typ, LPCWSTR name, int id=-1 );
DLLEXPORT D2Ctrl WINAPI DDMkListbox( D2Ctrls parent, const FRectFBM& rc, int typ, LPCWSTR name, int id=-1 );
DLLEXPORT D2Ctrl WINAPI DDMkStatic(D2Ctrls parent, const FRectFBM& rc, int , LPCWSTR text, int id=-1);
DLLEXPORT void WINAPI DDSetParameter(D2Ctrl ctrl, const std::map<std::wstring,VARIANT>& prms );

// ----------------------------------------------------------------------------------------------------------------------
// datagrid
DLLEXPORT D2Ctrl WINAPI DDMkDataGrid(D2Ctrls ctrls, const FRectFBM& rc, int colum_cnt, float* column_width, float row_height, float title_height, LPCWSTR name, int id=-1);
DLLEXPORT void WINAPI DDDataGridAllocbuffer(D2Ctrl ctrl, int total_rowcnt);
DLLEXPORT int  WINAPI DDDGGetSelectedIdx(D2Ctrl ctrl, int* idx);
DLLEXPORT LPCWSTR  WINAPI DDDGGetKey(D2Ctrl ctrl, int idx);
DLLEXPORT LPCWSTR  WINAPI DDDGGetValue(D2Ctrl ctrl, LPCWSTR key);





DLLEXPORT D2Ctrl WINAPI DDMkFRectFBM(D2Ctrls ctrls, const FRectFBM& rc, int typ, LPCWSTR name); // for debug


DLLEXPORT void WINAPI DDDestroyControl( D2Ctrl ctrl );
DLLEXPORT D2Ctrls WINAPI DDGetParent(D2Ctrl ctrl);


DLLEXPORT void WINAPI DDShow(D2Ctrl ctrl, bool bShow);



DLLEXPORT LONG_PTR* WINAPI DDGetListBuffer(D2Ctrl ctrl, int* cnt);

// listbox

DLLEXPORT void WINAPI DDListAddItem( D2Ctrl ctrl, LPCWSTR s );
DLLEXPORT void WINAPI DDListInsertItem( D2Ctrl ctrl, int idx, LPCWSTR s );
DLLEXPORT void WINAPI DDListDelItem( D2Ctrl ctrl, int idx );
DLLEXPORT int WINAPI DDListCurrentIdx( D2Ctrl ctrl );
DLLEXPORT void WINAPI DDListClear( D2Ctrl ctrl );
DLLEXPORT int WINAPI DDListItemCount( D2Ctrl ctrl, int idx );
DLLEXPORT void WINAPI DDListSetCurrentIdx( D2Ctrl ctrl, int idx );
DLLEXPORT LONG_PTR* WINAPI DDListGetBuffer(D2Ctrl ctrl);



// ----------------------------------------------------------------------------------------------------------------------
// Textbox
DLLEXPORT void WINAPI DDSetText( D2Ctrl ctrl, LPCWSTR text );
DLLEXPORT BSTR WINAPI DDGetText( D2Ctrl ctrl );
DLLEXPORT void WINAPI DDInsertText(D2Ctrl ctrl, int pos, LPCWSTR text);




// ----------------------------------------------------------------------------------------------------------------------
// misc
DLLEXPORT D2Ctrl WINAPI DDImageLoadFile( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR filename );
DLLEXPORT D2Ctrl WINAPI DDMkWaiter( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name );




// ----------------------------------------------------------------------------------------------------------------------
// drawing function
DLLEXPORT void WINAPI DDDrawFillRect(DDContext p, float cx, float cy, const DDColor& color );
DLLEXPORT void WINAPI DDDrawStringEx(DDContext p, const V4::FRectF& rc, LPCWSTR str, int center);
DLLEXPORT void WINAPI DDDrawString(DDContext p, float offx, float offy, LPCWSTR str);
DLLEXPORT void WINAPI DDDrawWhiteString(DDContext p, float offx, float offy, LPCWSTR str);
DLLEXPORT void WINAPI DDDrawString2(DDContext p, float offx, float offy, LPCWSTR str, DDColor color);

DLLEXPORT void WINAPI DDDrawString4(DDContext p, float left, float right, float height, LPCWSTR str, int len, int colorIndex );



DLLEXPORT void WINAPI DDDrawLine(DDContext p, V4::FPointF& pt1, V4::FPointF& pt2, int typ );


DLLEXPORT void WINAPI DDSetAlias(DDContext p, bool bOff );

// image -----------------------------------------------------------------------------
DLLEXPORT bool WINAPI DDLoadImage(DDContext p, DDImage& img);
DLLEXPORT void WINAPI DDDrawImage(DDContext p, const V4::FRectF& rc, DDImage& img);
DLLEXPORT bool WINAPI DDCopyToImage(DDContext p, const DDImage& src, const D2D1_RECT_U& rc, DDImage& dst);
DLLEXPORT bool WINAPI DDDivideImage(const DDImage& src, DDImage& dst, const D2D1_RECT_F& dstrc);

// datagridを拡張したリストボックス
DLLEXPORT D2Ctrl WINAPI DDMkDGListbox(D2Ctrls parent, const FRectFBM& rc, float row_height,bool multiselect, LPCWSTR name, int id=-1);
DLLEXPORT void  WINAPI DDAddKeyValue(D2Ctrl parent, LPCWSTR key, LPCWSTR value );


DLLEXPORT D2Ctrl WINAPI DDMkDropdownList(D2Ctrls parent, const FRectFBM& rc, float row_height, LPCWSTR name, int id=-1);

// table controls-------------------------------------------------------------------------
DLLEXPORT D2Ctrls WINAPI DDMkTableControls(D2Ctrls ctrls, const FRectFBM& rc, int rowcnt, int colcnt, LPCWSTR name);
DLLEXPORT void WINAPI DDSetControlPlace(D2Ctrls ctrls, D2Ctrl ctrl, int row, int col);


#define MODE_ENABLE				0x1
#define MODE_CLICK_ONLY			0x2
#define MODE_ONOFF				0x4
#define STAT_ON					0x8
#define MODE_ONOFF_OTHERALLOFF	0x10

DLLEXPORT D2Ctrl WINAPI DDMkImageButtons(D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name, DDImage btnImage, const D2D1_RECT_F* btnrc,DWORD* btnmode, int btnrc_cnt, int id=-1);

DLLEXPORT D2Ctrl WINAPI DDMkTopTitleBar(D2Ctrls ctrls, float height, LPCWSTR name);
DLLEXPORT D2Ctrls WINAPI DDMkSomeblocks(D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name);

/////////////////////////////////////////////////////////////////////////////////////////
class V4::FRectFBoxModel;

typedef std::function<void(D2Ctrl)> D2OnClick;

enum D2EVENT0_MODE { CLICK,DESTROY };
enum D2EVENT1_MODE { SELECTED,BIND,RESULT, ON };
enum D2EVENT2_MODE { MOUSE_LBUTTONDOWN,MOUSE_LBUTTONUP,MOUSEMOVE,DRAW };

typedef std::function<void(D2EVENT0_MODE ev, D2Ctrl)> D2Event0Delegate;
typedef std::function<void(D2EVENT1_MODE ev, D2Ctrl, int idx)> D2Event1Delegate;
typedef std::function<void(D2EVENT2_MODE ev, D2Ctrl, float x, float y)> D2Event2Delegate;

typedef std::function<void(D2EVENT2_MODE , D2Ctrl, DDContext, DDRowInfo& row)> D2Event2DrawDelegate;


DLLEXPORT void WINAPI DDEvent0( D2EVENT0_MODE ev, D2Ctrl ctrl, D2Event0Delegate func );
DLLEXPORT void WINAPI DDEvent1( D2EVENT1_MODE ev, D2Ctrl ctrl, D2Event1Delegate func );
DLLEXPORT void WINAPI DDEvent2( D2EVENT2_MODE ev, D2Ctrl ctrl, D2Event2Delegate func );

DLLEXPORT void WINAPI DDEvent2Draw(D2EVENT2_MODE ev, D2Ctrl ctrl, D2Event2DrawDelegate func);


// listbox


DLLEXPORT void WINAPI DDDTestResourceDestroy( D2DWin w, int layer );
DLLEXPORT void DebugTrace(LPCWSTR msg, D2D1_RECT_F& rc);
DLLEXPORT bool DDOpenTextFile(LPCWSTR msg, BSTR* ret);

typedef std::function<void(int result)> D2MessageBoxDelegate;
DLLEXPORT D2Ctrl WINAPI DDMkMessageBox(D2Ctrls parent, const FRectFBM& rc, int typ, LPCWSTR text, LPCWSTR title, D2MessageBoxDelegate ret);


DLLEXPORT void WINAPI DDMove(D2Ctrl ctrl, int typ, float offx, float offy, float millisecond );
DLLEXPORT void WINAPI DDMoveResize(D2Ctrl ctrl, int typ, V4::FRectF dstrc, float millisecond);

