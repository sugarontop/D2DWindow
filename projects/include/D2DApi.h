#pragma once

#ifdef _USRDLL
#define DLLEXPORT extern "C" __declspec( dllexport )
#else
#define DLLEXPORT extern "C"
#endif

#include <D2D1.h>
#include <dwrite.h>

using namespace D2D1;
namespace V4 
{

class D2DResource;

struct D2DRES
{
	int id;
	D2DResource* res;
};



DLLEXPORT D2DRES WINAPI ResourceAllocate( ID2D1RenderTarget* cxt,IDWriteFactory* wfct, HWND hWnd );
DLLEXPORT void WINAPI ResourceRelease( D2DRES& s );
DLLEXPORT void WINAPI SelectResource( const D2DRES& s );

DLLEXPORT HWND WINAPI GetActiveHwnd();
DLLEXPORT ID2D1RenderTarget* WINAPI GetActiveRenderTarget();


////////////////////////////////////////////////////////////////////////////////

DLLEXPORT bool WINAPI LoadResourceImage( ID2D1RenderTarget* cxt, LPCWSTR resource_section, int resource_id, ID2D1Bitmap** ret );
DLLEXPORT bool WINAPI LoadImageFromFile( ID2D1RenderTarget* cxt, LPCWSTR filenm, ID2D1Bitmap** ret );
DLLEXPORT bool WINAPI LoadImageFromIStream( ID2D1RenderTarget* cxt, IStream* sm, ID2D1Bitmap** ret );


DLLEXPORT bool WINAPI CreateTextFormat(LPCWSTR fontnm, float height, IDWriteTextFormat** ret );

DLLEXPORT void WINAPI DrawText(ID2D1RenderTarget* cxt,IDWriteTextFormat* tf, LPCWSTR str, ID2D1Brush* clr, const D2D1_RECT_F& rc ); 
DLLEXPORT float WINAPI DrawCenterText( ID2D1RenderTarget* cxt, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int strlen, int align );
DLLEXPORT float WINAPI DrawCenterTextEx( ID2D1RenderTarget* cxt,IDWriteTextFormat* fmt, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int strlen, int align );

DLLEXPORT D2D1_SIZE_F WINAPI CalcText( IDWriteTextFormat* fmt, LPCWSTR str, int strlen );
DLLEXPORT D2D1_POINT_2F WINAPI DrawCenterText2( const D2D1_RECT_F& rc, LPCWSTR str, int len, int align, IDWriteTextLayout** ret );
DLLEXPORT D2D1_POINT_2F WINAPI DrawCenterTextEx2( IDWriteTextFormat* fmt, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align, IDWriteTextLayout** ret );


DLLEXPORT void WINAPI DRAWFillRect( ID2D1RenderTarget* cxt,const D2D1_RECT_F& rc, ID2D1Brush* frameclr,ID2D1Brush* fillclr, float width );

DLLEXPORT void WINAPI DrawFill( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br );
DLLEXPORT void WINAPI DrawRect( ID2D1RenderTarget* cxt,const D2D1_RECT_F& rc, ID2D1Brush* br, float width );


DLLEXPORT void WINAPI DrawFillRectEx2( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width, int md );



DLLEXPORT void WINAPI DrawDotRect( ID2D1RenderTarget* cxt,const D2D1_RECT_F& rc, ID2D1Brush* br );

DLLEXPORT void WINAPI DrawSolidRect( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* br ); // GDIモードで表示

DLLEXPORT void WINAPI DrawFillRoundRect( ID2D1RenderTarget* cxt, ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius,ID2D1Brush* wakuclr,ID2D1Brush* fillclr );
DLLEXPORT void WINAPI DrawRoundRect( ID2D1RenderTarget* cxt,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br );

DLLEXPORT void WINAPI FillRoundRect( ID2D1RenderTarget* cxt,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br );


DLLEXPORT void WINAPI CreateVerticalGradation(ID2D1RenderTarget* target, const D2D1_SIZE_F& sz, D2D1_COLOR_F& clr1, D2D1_COLOR_F& clr2, ID2D1LinearGradientBrush** br );
DLLEXPORT void WINAPI CreateSolidBrush( ID2D1RenderTarget* target, D2D1_COLOR_F& clr, ID2D1SolidColorBrush** br );

DLLEXPORT void WINAPI DrawButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, LPCWSTR str, ID2D1Brush* br1 );


DLLEXPORT void WINAPI DrawWaiterCircle( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rcb, ID2D1Brush* br, int OneTo12, float size );
DLLEXPORT void WINAPI DrawCircle( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, float sz,ID2D1Brush* br );
DLLEXPORT void WINAPI FillCircle( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, float sz,ID2D1Brush* br );


DLLEXPORT bool WINAPI CreatePathGeometry(ID2D1PathGeometry* pathg, const D2D1_POINT_2F* pt, int ptcount);
DLLEXPORT void WINAPI CreateRetenGeometry(ID2D1PathGeometry* pathg);
DLLEXPORT void WINAPI CreateHexagonGeometry(ID2D1PathGeometry* pathg, float cx, float cy );
DLLEXPORT void WINAPI CreateRTRoundGeometry(ID2D1PathGeometry* pathg, float cx, float cy, float roundsize );

struct D2DScrollbarInfo
{
	D2D1_RECT_F rc;	
	bool bVertical;
	float total_height;		// 全表示領域	
	float position;			// スクロールバーの位置

	float button_height;	// UP DOWNのボタン
	float thumb_step_c;
	bool auto_resize;
	int rowno;				// 行番号方式の場合の現在の先頭行番号

	enum STAT { NONE,ONTHUMB=0x1,ONBTN1=0x2,ONBTN2=0x4,ONSPC1=0x8,ONSPC2=0x10,CAPTURED=0x20 };
	int stat;

	D2D1_POINT_2F ptprv; // テンポラリのポインタ
	D2D1_RECT_F thumb_rc;	// 計算される
};

DLLEXPORT void WINAPI DrawScrollbar( ID2D1RenderTarget* cxt, D2DScrollbarInfo& info );
DLLEXPORT D2D1_RECT_F WINAPI ScrollbarRect( D2DScrollbarInfo& info, int typ );

DLLEXPORT void WINAPI FillArrow( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, ID2D1Brush* br, int typ );
DLLEXPORT void WINAPI FillPolygon( ID2D1RenderTarget* cxt, const D2D1_POINT_2F* pt, int pt_count, ID2D1Brush* br );

DLLEXPORT void WINAPI DrawDebugCross(ID2D1RenderTarget* cxt,ID2D1Brush* br );
DLLEXPORT void WINAPI DrawDebugCross2(ID2D1RenderTarget* cxt,ID2D1Brush* br, FPointF pt );
DLLEXPORT D2D1_POINT_2F WINAPI DrawLine(ID2D1RenderTarget* cxt, const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2, float width,ID2D1Brush* br, ID2D1StrokeStyle* st=NULL );
DLLEXPORT void WINAPI DrawLineRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width=1.0f );
DLLEXPORT void WINAPI DrawCheckMark(ID2D1RenderTarget* cxt, ID2D1Brush* br );
DLLEXPORT void WINAPI DrawCheckbox( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, bool bCheck, bool bEnable, LPCWSTR str );
DLLEXPORT void WINAPI DrawRadioButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, bool bCheck, bool bEnable, LPCWSTR str );






DLLEXPORT D2D1_COLOR_F WINAPI ColorParse( LPCWSTR hex_string );



DLLEXPORT void WINAPI CreateTagButtomGeometry(ID2D1RenderTarget* cxt, const FRectF& rcButton, const FRectF& rc, ID2D1PathGeometry** ret );
DLLEXPORT void WINAPI CreateTraiangle( ID2D1Factory* factory, int type, float cx, float cy, ID2D1PathGeometry** ret );


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct D2DContextBase
{	
	CComPtr<ID2D1RenderTarget>  cxt;
	IDWriteTextFormat* text;
	IDWriteFactory* wfactory;
};

DLLEXPORT void WINAPI D2DrawCenterText( D2DContextBase& cxt, ID2D1Brush* clr, const FRectF& rc, LPCWSTR str, int len, int align );
DLLEXPORT int WINAPI D2GetLineMetrics( D2DContextBase& cxt, const FSizeF& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS* ret );
DLLEXPORT void WINAPI DrawSkyBlueGrid( ID2D1RenderTarget* cxt, FSizeF sz, float duration );


DLLEXPORT float WINAPI DrawFullText( ID2D1RenderTarget* cxt, ID2D1Brush* clr, const D2D1_SIZE_F& sz, LPCWSTR str, int len, int align );
DLLEXPORT BSTR WINAPI StringToMoneyString( LPCWSTR cmoney );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internet関連 direct2dは関係しない
DLLEXPORT BSTR WINAPI InetPercentEncode( LPCWSTR src );
DLLEXPORT BSTR WINAPI XmlUtf8Encode( LPCWSTR str );

};
