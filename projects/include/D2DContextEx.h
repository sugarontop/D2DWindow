#pragma once

#include "D2DMisc.h"
#include "D2DApi.h"

namespace V4 
{



// SingletonD2DInstanceは独立した存在なので、HWNDに関わるリソースはもたない。
struct SingletonD2DInstance
{
	CComPtr<IDWriteFactory1> wrfactory;
	CComPtr<ID2D1Factory1>  factory;
	CComPtr<IDWriteTextFormat> text; // IDWriteTextFormat1 is from Win8.1.

	static SingletonD2DInstance& Init();

};

#define STOCKSIZE 16
struct D2DContext;

struct D2DContextText
{
	void Init(D2DContext& inshw,  float height, LPCWSTR fontname );
	D2DContext* cxt;

	
	UINT GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics );
	UINT GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric );
	UINT GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics );


	UINT GetLineMetric( const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics );

	CComPtr<IDWriteTextFormat> textformat;
	CComPtr<IDWriteFactory1> wrfactory;
	
	float xoff;			// １行表示の左端の余幅
	float line_height;	// １行表示の高さ

};
struct D2DContext : public D2DContextBase
{	
	/*
	struct D2DContextBase
	{
		CComPtr<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* text;
		IDWriteFactory* wfactory;
	};
	*/
	
	

	SingletonD2DInstance* insins;

	
	operator ID2D1RenderTarget*() const{ return cxt.p; } 
#ifdef USE_ID2D1DEVICECONTEXT
	CComPtr<IDXGISwapChain1> dxgiSwapChain;
	D2D1_SIZE_U RenderSize_;
#endif

	CComPtr<ID2D1SolidColorBrush> ltgray;
	CComPtr<ID2D1SolidColorBrush> black;
	CComPtr<ID2D1SolidColorBrush> white;
	CComPtr<ID2D1SolidColorBrush> red;
	CComPtr<ID2D1SolidColorBrush> gray;
	CComPtr<ID2D1SolidColorBrush> bluegray;
	CComPtr<ID2D1SolidColorBrush> transparent;
	CComPtr<ID2D1SolidColorBrush> halftone;
	CComPtr<ID2D1SolidColorBrush> halftoneRed;
	CComPtr<ID2D1SolidColorBrush> tooltip;

	CComPtr<ID2D1SolidColorBrush> basegray;
	CComPtr<ID2D1SolidColorBrush> basegray_line;
	CComPtr<ID2D1SolidColorBrush> basetext;

	CComPtr<ID2D1StrokeStyle> dot4_;
	CComPtr<ID2D1StrokeStyle> dot2_;

	CComPtr<ID2D1Factory1> factory(){ return insins->factory; }
	D2DContextText cxtt;

	LPVOID free_space;

	void Init(SingletonD2DInstance& ins, HWND hWnd );
	void Destroy();

	void Init(SingletonD2DInstance& ins);
	void CreateHwndRenderTarget( HWND hWnd );
	void CreateDeviceContextRenderTarget( HWND hWnd );
	void CreateResourceOpt();

	void CreateDeviceContextRenderTargetTest( HWND hWnd, UINT width );

	void CreateRenderTargetResource( ID2D1RenderTarget* t );
	void DestroyRenderTargetResource();

	void DestroyAll();

	void SetAntiAlias(bool bl){ cxt->SetAntialiasMode( bl ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE:D2D1_ANTIALIAS_MODE_ALIASED);} 


	HRESULT CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret );
};

///////////////////////////////////////////////////////
// application初期設定後に自由に設定できる。
// 各クラスのコンストラクタ内の設定で参照される。
// 変更すれば、その後作成時に変更された色になる。
///////////////////////////////////////////////////////
//struct ColorBank
//{
//	static CComPtr<ID2D1SolidColorBrush> SolidColorBrush( D2DContext& cxt, LPCWSTR name, D2D1_COLOR_F defaultColor ); // 同じnameが存在しない場合のみ、defaultColorが適用される。
//
//	private :
//		static std::map<std::wstring, CComPtr<ID2D1SolidColorBrush>> bank_;
//};

//struct D2DColor
//{
//	D2DColor( D2DContext& cxt, ColorF& clr )
//	{
//		cxt.cxt->CreateSolidColorBrush( clr, &Brush );
//	}
//	D2DColor( D2DContext& cxt, float r, float g, float b, float a )
//	{
//		cxt.cxt->CreateSolidColorBrush( D2RGBA(r,g,b,a), &Brush );
//	}	
//	operator ID2D1Brush*()
//	{		
//		return Brush;
//	}
//
//	CComPtr<ID2D1SolidColorBrush> Brush;
//};

struct D2DRectFilter
{
	D2DRectFilter(D2DContext& cxt1, const FRectF& rc ):cxt(cxt1)
	{
		cxt.cxt->PushAxisAlignedClip( rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );		
		cnt = 1;
	}

	D2DRectFilter(D2DContext& cxt1, FRectF&& rc ):cxt(cxt1)
	{
		cxt.cxt->PushAxisAlignedClip( std::move(rc), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );		
		cnt = 1;
	}

	~D2DRectFilter()
	{
		if ( cnt == 1 )
			cxt.cxt->PopAxisAlignedClip();
	}
	void Off()
	{		
		if ( cnt == 1 )
		{
			cxt.cxt->PopAxisAlignedClip();
			cnt = 0;
		}
	}

	private :
		D2DContext& cxt;
		int cnt;
};
struct D2DRectFilterType1
{
	D2DRectFilterType1(D2DContext& cxt1, FRectF rc ):cxt(cxt1)
	{
		rc.left--; rc.top--;
		cxt.cxt->PushAxisAlignedClip( rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
	}
	~D2DRectFilterType1()
	{
		cxt.cxt->PopAxisAlignedClip();
	}

	private :
		D2DContext& cxt;
};

struct SingleLineText
{
	FPointF ptLineText;
	CComPtr<IDWriteTextLayout> textlayout;

	void CreateLayout(D2DContextText& cxt, const FRectF& rc, LPCWSTR str, int len, int align );

	void CreateLayoutEx(D2DContextText& cxt,IDWriteTextFormat* fmt, const FRectF& rc, LPCWSTR str, int len, int align );

	void DrawText(D2DContext& cxt, ID2D1Brush* foreclr );
};
///////////////////////////////////////////////////////////////////////////////////////////
class D2DError 
{
	public :
		explicit D2DError( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm):hr_(hr),msg_(msg),line_(line),fnm_(fnm )
		{
			TRACE(L"%s :%d行 HR=%x %s\n", (LPCWSTR)fnm_, line_,hr_,(LPCWSTR)msg_);		
		}

	public :
		CComBSTR msg_;
		CComBSTR fnm_;
		UINT line_;
		HRESULT hr_;

};
inline void ThrowIfFailed( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm )
{
	if (FAILED(hr))
	{
		throw D2DError( hr, msg, line,fnm );
		//General access denied error 0x80070005 
	}
}
#define THROWIFFAILED(hr,msg) ThrowIfFailed(hr,msg, __LINE__, __FILE__)

inline HRESULT NoThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch Win32 API errors.
		TRACE( L"NoThrowIfFailed %x\n", hr );        
    }

	return hr;
}



//////////////////////////////////////////////////////////////////////////////////////
// Variant
FRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy );
FSizeF FSizeFV( _variant_t& cx,_variant_t& cy );
FPointF FPointFV( _variant_t& cx,_variant_t& cy );
FString FStringV( _variant_t& s );


void DrawCenterText( D2DContextText& cxt, ID2D1Brush* clr, FRectF& rc, LPCWSTR str, int len,int align  );
void DrawFillRect( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width );
void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr );

void TestDrawFillRectEx( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr );

CComPtr<ID2D1SolidColorBrush> MakeBrsuh( D2DContext& cxt, D2D1_COLOR_F clr );


// bitmap　使ってない ///////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::function<void (D2DContext& cxt)> DrawFunction;
bool CreateBitmapPartBrush( D2DContext& cxt, const FSizeF& size, DrawFunction drawfunc, OUT ID2D1BitmapBrush **ppBitmapBrush );


//以下 TSFのために追加 /////////////////////////////////////////////////////////////////////
void InvertRect(D2DContext& cxt, FRectF* rc );

bool DrawCaret(D2DContext& cxt, const FRectF& rc );

////////////////////////////////////////////////////////////////////////////////////////
 
 
            



};

namespace V4{

///////////////////////////////////////////////////////////////////////////////////
// センターテキスト表示、フォント作成、layoutから表示するので早い
////////////////////////////////////////////////////////////////////////////////////
class D2DStringLayout
{
	public :
		D2DStringLayout(){}

	public :
		CComPtr<IDWriteTextLayout> tl_;
		FPointF pt_;

	public :
		bool CreateCenterText( D2DContextBase& cxt, const FRectF& rc, LPCWSTR str, int len, int align );
		DWRITE_TEXT_METRICS GetMetrics();
		void Draw( D2DContext& cxt, ID2D1Brush* br );

		bool IsFirst(){ return ( tl_ == nullptr ); }
		void Clear(){ tl_.Release(); }

	public :
		static D2DContextBase CreateFont( D2DContext& cxt, LPCWSTR fontname, float height, DWRITE_FONT_WEIGHT bold=DWRITE_FONT_WEIGHT_NORMAL );

};

};
