#pragma once 


//#include "D2DApi.h"
namespace V4 
{

struct D2DContextBase
{	
	CComPtr<ID2D1RenderTarget>  cxt;
	IDWriteTextFormat* text;
	IDWriteFactory* wfactory;
};


// SingletonD2DInstanceは独立した存在なので、HWNDに関わるリソースはもたない。
struct SingletonD2DInstance
{
	CComPtr<IDWriteFactory> wrfactory;
	CComPtr<ID2D1Factory>  factory;
	CComPtr<IDWriteTextFormat> text;

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

	CComPtr<IDWriteTextFormat> textformat;
	
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

	CComPtr<ID2D1SolidColorBrush> ltgray;
	CComPtr<ID2D1SolidColorBrush> black;
	CComPtr<ID2D1SolidColorBrush> white;
	CComPtr<ID2D1SolidColorBrush> red;
	CComPtr<ID2D1SolidColorBrush> gray;
	CComPtr<ID2D1SolidColorBrush> bluegray;
	CComPtr<ID2D1SolidColorBrush> transparent;
	CComPtr<ID2D1SolidColorBrush> halftone;
	CComPtr<ID2D1SolidColorBrush> halftoneRed;

	CComPtr<ID2D1StrokeStyle> dot4_;
	CComPtr<ID2D1StrokeStyle> dot2_;

	CComPtr<ID2D1Factory> factory(){ return insins->factory; }
	D2DContextText cxtt;

	IUnknown* stock[STOCKSIZE];

	LPVOID free_space;

	void Init(SingletonD2DInstance& ins, HWND hWnd );
	void Destroy();

	void SetAntiAlias(bool bl){ cxt->SetAntialiasMode( bl ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE:D2D1_ANTIALIAS_MODE_ALIASED);} 

};

struct ColorBank
{
	static CComPtr<ID2D1SolidColorBrush> SolidColorBrush( D2DContext& cxt, LPCWSTR name, D2D1_COLOR_F defaultColor ); 

	private :
		static std::map<std::wstring, CComPtr<ID2D1SolidColorBrush>> bank_;
};

struct D2DColor
{
	D2DColor( D2DContext& cxt, ColorF& clr )
	{
		cxt.cxt->CreateSolidColorBrush( clr, &Brush );
	}
	D2DColor( D2DContext& cxt, float r, float g, float b, float a )
	{
		cxt.cxt->CreateSolidColorBrush( D2RGBA(r,g,b,a), &Brush );
	}	
	operator ID2D1Brush*()
	{		
		return Brush;
	}

	CComPtr<ID2D1SolidColorBrush> Brush;
};

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

void DrawCenterText( D2DContextText& cxt, ID2D1Brush* clr, FRectF& rc, LPCWSTR str, int len,int align  );


/*
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

	void DrawText(D2DContext& cxt, ID2D1Brush* foreclr );
};

//////////////////////////////////////////////////////////////////////////////////////
// Variant
FRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy );
FSizeF FSizeFV( _variant_t& cx,_variant_t& cy );
FPointF FPointFV( _variant_t& cx,_variant_t& cy );
FString FStringV( _variant_t& s );



void DrawFillRect( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width );
void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr );

void TestDrawFillRectEx( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr );


// bitmap　使ってない ///////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::function<void (D2DContext& cxt)> DrawFunction;
bool CreateBitmapPartBrush( D2DContext& cxt, const FSizeF& size, DrawFunction drawfunc, OUT ID2D1BitmapBrush **ppBitmapBrush );


//以下 TSFのために追加 /////////////////////////////////////////////////////////////////////
void InvertRect(D2DContext& cxt, FRectF* rc );

bool DrawCaret(D2DContext& cxt, const FRectF& rc );

////////////////////////////////////////////////////////////////////////////////////////
 
 CComPtr<ID2D1SolidColorBrush> CreateBrush( D2DContext& cxt, D2D1_COLOR_F color );
            

*/

};
