#include "stdafx.h"
#include "D2DContextEx.h"

//#pragma comment(lib,"D2DApi.lib")

static LARGE_INTEGER __s_frequency_;

 
namespace V4{

SingletonD2DInstance& SingletonD2DInstance::Init()
{
	static SingletonD2DInstance st;
		
	if ( st.factory.p == NULL )
	{
		// Exeにつき１回の実行

		HRESULT hr;
		D2D1_FACTORY_OPTIONS options;	
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),&options,(void**)&st.factory );
		_ASSERT(HR(hr));

		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&st.wrfactory));
		_ASSERT(HR(hr));
		
		hr = st.wrfactory->CreateTextFormat(DEFAULTFONT, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DEFAULTFONT_HEIGHT, L"", &st.text);
		_ASSERT( HR(hr));
	}

	return st;
}

void D2DContext::Init(SingletonD2DInstance& ins, HWND hWnd1 ) 
{
	// D2DWindow::CreateD2DWindowのタイミングで実行
	// Windowつき１回の実行

	insins = &ins;
	ID2D1Factory* factory = ins.factory;		
	CComPtr<ID2D1HwndRenderTarget> temp;

	auto hr = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd1, D2D1::SizeU(1, 1), D2D1_PRESENT_OPTIONS_NONE), &temp);
	xassert(HR(hr));
	
	hr= temp.QueryInterface( &cxt );
	xassert(HR(hr));

	FLOAT dpix,dpiy;
	cxt->GetDpi( &dpix, &dpiy );
	_ASSERT( dpix == 96 && dpiy == 96 );// ハードウェアの値、controlパネルでdpiを変えても、この数字は変わらない。

	cxt->CreateSolidColorBrush( D2RGB(0,0,0), &black );
	cxt->CreateSolidColorBrush( D2RGB(255,255,255), &white );
	cxt->CreateSolidColorBrush( D2RGB(192,192,192), &gray );
	cxt->CreateSolidColorBrush( D2RGB(255,0,0), &red );
	cxt->CreateSolidColorBrush( D2RGB(230,230,230), &ltgray);
	cxt->CreateSolidColorBrush( D2RGB(113,113,130), &bluegray);
	cxt->CreateSolidColorBrush( D2RGBA(0, 0, 0, 0), &transparent);

	cxt->CreateSolidColorBrush( D2RGBA(113,113,130,100), &halftone); 
	cxt->CreateSolidColorBrush( D2RGBA(250,113,130,150), &halftoneRed);

	
	

	float dashes[] = {2.0f};

	factory->CreateStrokeStyle(
	D2D1::StrokeStyleProperties(
		D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
		10.0f,
		D2D1_DASH_STYLE_CUSTOM,
		0.0f),
		dashes, ARRAYSIZE(dashes),
		&dot2_
	);

	float dashes2[] = {4.0f};
	factory->CreateStrokeStyle(
	D2D1::StrokeStyleProperties(
		D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
		10.0f,
		D2D1_DASH_STYLE_CUSTOM,
		0.0f),
		dashes2, ARRAYSIZE(dashes2),
		&dot4_
	);






	text = insins->text.p;
	wfactory = insins->wrfactory.p;
	

	ZeroMemory(stock, sizeof(IUnknown*) * STOCKSIZE);

	QueryPerformanceFrequency( &__s_frequency_ );

}
void D2DContext::Destroy()
{
	// OnDestroyのタイミングで実行

	cxt.Release();
	black.Release();
	white.Release();
	gray.Release();
	red.Release();
	ltgray.Release();
	transparent.Release();

	for (int i = 0; i < STOCKSIZE; i++)
	{
		if ( stock[i] )
			stock[i]->Release();
	}	
}


	
void D2DContextText::Init(D2DContext& inshw, float height, LPCWSTR fontname )
{	
	cxt = &inshw;
	line_height = 0;
	xoff = 0;

	if ( HR(inshw.insins->wrfactory->CreateTextFormat(fontname,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"",&textformat)))
	{
		CComPtr<IDWriteTextLayout> tl;
		cxt->wfactory->CreateTextLayout( L"T",1, textformat, 1000, 1000, &tl );

		DWRITE_HIT_TEST_METRICS mt;
	
		float y;
		tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );

		line_height = mt.height;


		/*DWRITE_TEXT_METRICS tm;
		GetLineMetric( FSizeF(1000,1000), L"T", 1, tm );

		temp_font_height_ = tm.height;*/

	}
}
UINT D2DContextText::GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics )
{
	CComPtr<IDWriteTextLayout> textlayout;
	cxt->insins->wrfactory->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 

	
    textlayout->GetMetrics(&textMetrics);

    lineMetrics.resize(textMetrics.lineCount);
    textlayout->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
    
    return textMetrics.lineCount; // 全行数
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric )
{
	std::vector<DWRITE_LINE_METRICS> ar;
	
	UINT r = GetLineMetrics( sz,str,len, textMetrics, ar );
	_ASSERT( r );
	lineMetric = ar[0];
	return r;
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	CComPtr<IDWriteTextLayout> tl;
	cxt->insins->wrfactory->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &tl ); 
    tl->GetMetrics(&textMetrics);

	return textMetrics.lineCount;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void DrawCenterText( D2DContextText& cxt, ID2D1Brush* clr, FRectF& rc, LPCWSTR str, int len, int align )
{	
	DWRITE_TEXT_METRICS tm;
	cxt.GetLineMetric( rc.Size(), str, len, tm );

	FRectF rcX(rc);
	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
	rcX.top = center - tm.height/2;
	rcX.bottom = rcX.top + tm.height;

	
	if ( align == 2 )
		rcX.left = rcX.right-tm.width;
	else if ( align == 1 )
	{
		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
		rcX.right = rcX.left + tm.width;
	}

	cxt.cxt->cxt->DrawText( str, len, cxt.textformat, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
}
void TestDrawFillRectEx( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr )
{	
	ID2D1RenderTarget* cxt_ = cxt.cxt;

	FRectF rcc(rc);
	rcc.InflateRect(-1,-1);

	cxt_->DrawRectangle( rcc, wakuclr, 2.0f );	
				
	cxt_->FillRectangle( rcc, fillclr );	
}
/*
void DrawFillRect( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
{
	_ASSERT( width > 0 );
	// Line is FillRectangle.
	
	DrawFillRectEx( cxt.cxt, rc, wakuclr, fillclr, width );
}
*/

void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr )
{
	FRectF yrc(rc);
	yrc.InflateRect(-1,-1);
	cxt.cxt->FillRectangle( yrc, fillclr );	
}





//void DrawRect( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width )
//{		
//	ID2D1RenderTarget* cxt_ = cxt.cxt;
//	cxt_->DrawRectangle( rc, br, width );			
//}
//void FillRect( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* br )
//{
//	ID2D1RenderTarget* cxt_ = cxt.cxt;
//	cxt_->FillRectangle( rc, br );
//}


///TSF////////////////////////////////////////////////////////////////////////////////////////////

void InvertRect(D2DContext& cxt, FRectF* rc )
{
	//static bool bl = false;
	FRectF rcf(*rc);

	CComPtr<ID2D1SolidColorBrush> br;
	cxt.cxt->CreateSolidColorBrush( D2RGBA(255,200,0,100), &br );

	cxt.cxt->FillRectangle( rcf, br );
}

bool DrawCaret(D2DContext& cxt, const FRectF& rc )
{
	static bool bl = false;
	static LARGE_INTEGER gtm,pregtm;
	
	QueryPerformanceCounter(&gtm);
						
	float zfps =((float)__s_frequency_.QuadPart)/ ((float)(gtm.QuadPart-pregtm.QuadPart));

	#ifdef TEXTBOXTEST
	//if  ( 1.0/zfps  > 0.4f )
	//{
	//	pregtm = gtm;
	//	bl = !bl;
	//}
	//else

	{
	bl = true;
		cxt.cxt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		cxt.cxt->FillRectangle( rc, ( bl ? cxt.black : cxt.white ));
		cxt.cxt->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
	#else
	if  ( 1.0/zfps  > 0.4f )
	{
		pregtm = gtm;
		bl = !bl;
	}
	else
	{
	
		cxt.cxt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		cxt.cxt->FillRectangle( rc, ( bl ? cxt.black : cxt.white ));
		cxt.cxt->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
	#endif

	return true;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////////////

CComPtr<ID2D1SolidColorBrush> CreateBrush( D2DContext& cxt, D2D1_COLOR_F color )
{
	CComPtr<ID2D1SolidColorBrush> br;
	cxt.cxt->CreateSolidColorBrush( color, &br);
	return br;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SingleLineText::CreateLayout(D2DContextText& cxt, const FRectF& rc, LPCWSTR str, int len, int align )
{
	xassert( rc.top == 0 ); // for vertical line calc.
	
	DWRITE_TEXT_METRICS tm;

	auto prv = cxt.textformat->GetWordWrapping();
	cxt.textformat->SetWordWrapping( DWRITE_WORD_WRAPPING_NO_WRAP ); //改行なしへ、

	cxt.GetLineMetric( rc.Size(), str, len, tm );

	FRectF rcX(rc);
	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
	rcX.top = center - tm.height/2;
	rcX.bottom = rcX.top + tm.height;

	
	if ( align == 2 )
		rcX.left = rcX.right-tm.width;
	else if ( align == 1 )
	{
		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
		rcX.right = rcX.left + tm.width;
	}

	if ( textlayout.p )
		textlayout.Release();

	ptLineText = rcX.LeftTop();
	FSizeF sz = rcX.Size();
	cxt.cxt->insins->wrfactory->CreateTextLayout(str,len, cxt.textformat,sz.width, sz.height, &textlayout ); 

	cxt.textformat->SetWordWrapping( prv );
   
}
void SingleLineText::DrawText(D2DContext& cxt, ID2D1Brush* foreclr )
{
	xassert( textlayout.p );

	cxt.cxt->DrawTextLayout( ptLineText,textlayout,foreclr );	
}

////////////////////////////////////////////////////////////////////////////////////////
FRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy )
{
	float fx; x.ChangeType( VT_R4 ); fx = x.fltVal;
	float fy; y.ChangeType( VT_R4 ); fy = y.fltVal;
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal+fx;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal+fy;

	return FRectF(fx,fy,fcx,fcy );
}
FSizeF FSizeFV( _variant_t& cx,_variant_t& cy )
{
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;

	return FSizeF( fcx,fcy );

}
FPointF FPointFV( _variant_t& cx,_variant_t& cy )
{
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;
	return FPointF( fcx,fcy );
}
FString FStringV( _variant_t& s )
{
	s.ChangeType( VT_BSTR );

	return s.bstrVal;
}

bool CreateBitmapPartBrush( D2DContext& cxtbase, const FSizeF& size, DrawFunction drawfunc, OUT ID2D1BitmapBrush **ppBitmapBrush )
{
	// 画面のbitmapを作成、画面のハードコピー, FillRectで表示(0,0から引きつめられる)
	// cxtのリソースをすべて再作成する必要あり。
	
	D2DContext cxt;
	ID2D1RenderTarget* pRenderTarget = cxtbase.cxt;
    CComPtr<ID2D1BitmapRenderTarget> pCompatibleRenderTarget;
    
	// create cmpati render target.
	HRESULT hr = pRenderTarget->CreateCompatibleRenderTarget( size, &pCompatibleRenderTarget );

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
            pCompatibleRenderTarget->BeginDraw();
			{
				// change rendertarget.

				D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();
				pCompatibleRenderTarget->SetTransform(mat);

				cxt.cxt = pCompatibleRenderTarget;

				drawfunc( cxt );
			}
            pCompatibleRenderTarget->EndDraw();

			// Create bitmap
            CComPtr<ID2D1Bitmap> pGridBitmap;
            hr = pCompatibleRenderTarget->GetBitmap(&pGridBitmap);
            if (SUCCEEDED(hr))
            {
                // Choose the tiling mode for the bitmap brush.
                D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
                    D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

                // Create the bitmap brush.
                hr = pRenderTarget->CreateBitmapBrush(pGridBitmap, brushProperties, ppBitmapBrush);
            }
        }
    }

    return ( hr == S_OK );
}









/////////////////////////////////////////////////////////////////////////////////
std::map<std::wstring, CComPtr<ID2D1SolidColorBrush>> ColorBank::bank_;

CComPtr<ID2D1SolidColorBrush> ColorBank::SolidColorBrush( D2DContext& cxt, LPCWSTR name, D2D1_COLOR_F defaultColor )
{	
	auto it = bank_.find( name );
	
	if ( it == bank_.end())
	{
		CComPtr<ID2D1SolidColorBrush> br;
		cxt.cxt->CreateSolidColorBrush( defaultColor, &br );
		
		bank_[name] = br;

		it = bank_.find( name );
	}
	
	return it->second;	
}
*/




};