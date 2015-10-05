#include "stdafx.h"
#include "D2DWindowControl_easy.h"

static LARGE_INTEGER __s_frequency_;

namespace V4{

SingletonD2DInstance& SingletonD2DInstance::Init()
{
	static SingletonD2DInstance st;
		
	if ( st.factory.p == NULL )
	{
		D2D1_FACTORY_OPTIONS options;	
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		THROWIFFAILED( D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory1),&options,(void**)&st.factory ), L"SingletonD2DInstance::Init()");
		THROWIFFAILED( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&st.wrfactory)), L"SingletonD2DInstance::Init()");
		THROWIFFAILED( st.wrfactory->CreateTextFormat(DEFAULTFONT, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DEFAULTFONT_HEIGHT, L"", &st.text), L"SingletonD2DInstance::Init()");
	}
	return st;
}

void D2DContext::Init(SingletonD2DInstance& ins )
{
	insins = &ins;
}
void D2DContext::CreateHwndRenderTarget( HWND hWnd )
{
	ID2D1Factory1* factory = insins->factory;		
	ComPTR<ID2D1HwndRenderTarget> temp;

	factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(1, 1), D2D1_PRESENT_OPTIONS_NONE), &temp); //, L"D2DContext::CreateHwndRenderTarget");
		
	auto hr= temp.p->QueryInterface( &cxt );
	_ASSERT(HR(hr));
}
void D2DContext::CreateDeviceContextRenderTarget( HWND hWnd )
{
#ifdef USE_ID2D1DEVICECONTEXT
	RECT rc;
	GetClientRect(hWnd, &rc);

	if ( rc.top >= rc.bottom )
		rc.bottom = rc.top + 1;

	_ASSERT( rc.left != rc.right );
	_ASSERT( rc.top != rc.bottom );


//	UINT width = ::GetSystemMetrics( SM_CXFULLSCREEN );
//	UINT height = ::GetSystemMetrics( SM_CYFULLSCREEN );

	UINT width = max(100, (UINT)(rc.right*1.5));
	UINT height = max(100, (UINT)(rc.bottom*1.5));

//	UINT width = max(800, (UINT)(rc.right));
//	UINT height = max(800, (UINT)(rc.bottom));

	ComPTR<ID2D1DeviceContext> d2d1DeviceContext;
	ComPTR<IDXGISwapChain1> dxgiSwapChain1;
	ComPTR<ID2D1Factory2> d2d1Factory = insins->factory;

	ComPTR<ID2D1Device> d2d1Device;
	ComPTR<ID2D1Bitmap1> d2d1Bitmap;

	ComPTR<ID3D11DeviceContext> d3d11DeviceContext;
	ComPTR<ID3D11Device> d3d11Device;
	ComPTR<IDXGISurface> dxgiSurface;
	ComPTR<IDXGIAdapter> dxgiAdapter;
	ComPTR<IDXGIFactory2> dxgiFactory;
	ComPTR<IDXGIDevice1> dxgiDevice;

	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 
	
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
 
	D3D_FEATURE_LEVEL returnedFeatureLevel;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};


	THROWIFFAILED( D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
					&d3d11Device, &returnedFeatureLevel, &d3d11DeviceContext),L"CreateDeviceContextRenderTarget");


	THROWIFFAILED(d3d11Device->QueryInterface(&dxgiDevice),L"CreateDeviceContextRenderTarget");
	THROWIFFAILED(d2d1Factory->CreateDevice(dxgiDevice.p, &d2d1Device),L"CreateDeviceContextRenderTarget");
	THROWIFFAILED(d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d1DeviceContext),L"CreateDeviceContextRenderTarget");
	THROWIFFAILED(dxgiDevice->GetAdapter(&dxgiAdapter),L"CreateDeviceContextRenderTarget");
	THROWIFFAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)),L"CreateDeviceContextRenderTarget");

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = 0;

	THROWIFFAILED(dxgiFactory->CreateSwapChainForHwnd(d3d11Device,
													hWnd,
													&swapChainDesc,
													nullptr,
													nullptr,
													&dxgiSwapChain1),L"!!CreateSwapChainForHwnd");
	
	_ASSERT( dxgiSwapChain1 );

	THROWIFFAILED(dxgiDevice->SetMaximumFrameLatency(1),L"CreateDeviceContextRenderTarget");
	THROWIFFAILED(dxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface)),L"CreateDeviceContextRenderTarget");

	auto bmp_property = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
	THROWIFFAILED(d2d1DeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.p, &bmp_property, &d2d1Bitmap),L"CreateDeviceContextRenderTarget");
	d2d1DeviceContext->SetTarget(d2d1Bitmap.p);

	auto sz = d2d1Bitmap->GetSize();

	_ASSERT( sz.width == width  );
	_ASSERT( sz.height == height  );
	//////////////////////////////////////////////////////////////////////////////////////////////

	RenderSize_.width = width;
	RenderSize_.height = height;

	d2d1DeviceContext->QueryInterface(&cxt);

	dxgiSwapChain = dxgiSwapChain1;
#endif

}
void D2DContext::DestroyRenderTargetResource()
{
	UINT a = cxt.p->AddRef()-1;
	cxt.p->Release();
	_ASSERT( a == 1 );

	cxt.Release();
#ifdef USE_ID2D1DEVICECONTEXT
	a = dxgiSwapChain.p->AddRef()-1;
	dxgiSwapChain.p->Release();
	_ASSERT( a == 1 );
	
	dxgiSwapChain.Release();
#endif
	ltgray.Release();
	black.Release();
	white.Release();
	red.Release();
	gray.Release();
	bluegray.Release();
	transparent.Release();
	halftone.Release();
	halftoneRed.Release();
	tooltip.Release();
	
}
void D2DContext::DestroyAll()
{
	// OnDestroyのタイミングで実行

	DestroyRenderTargetResource();

	dot2_.Release();
	dot4_.Release();
}

void D2DContext::CreateRenderTargetResource( ID2D1RenderTarget* rt )
{
	// D2DWindow::CreateD2DWindowのタイミングで実行
	// D2DWindow::WM_SIZEのタイミングで実行

	rt->CreateSolidColorBrush( D2RGB(0,0,0), &black );
	rt->CreateSolidColorBrush( D2RGB(255,255,255), &white );
	rt->CreateSolidColorBrush( D2RGB(192,192,192), &gray );
	rt->CreateSolidColorBrush( D2RGB(255,0,0), &red );
	rt->CreateSolidColorBrush( D2RGB(230,230,230), &ltgray);
	rt->CreateSolidColorBrush( D2RGB(113,113,130), &bluegray);
	rt->CreateSolidColorBrush( D2RGBA(0, 0, 0, 0), &transparent);

	rt->CreateSolidColorBrush( D2RGBA(113,113,130,100), &halftone); 
	rt->CreateSolidColorBrush( D2RGBA(250,113,130,150), &halftoneRed);

	rt->CreateSolidColorBrush( D2RGBA(255,242,0,255), &tooltip);

	// DestroyRenderTargetResourceを忘れないこと
}

void D2DContext::CreateResourceOpt()
{
	// RenderTargetとは関係ないリソース作成

	cxtt.Init( *this, DEFAULTFONT_HEIGHT, DEFAULTFONT );
	
	
	ID2D1Factory* factory = insins->factory;		
	
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

	QueryPerformanceFrequency( &__s_frequency_ );

}
void D2DContext::CreateRenderResource( HWND hWnd )
{
	
	#ifdef USE_ID2D1DEVICECONTEXT
		CreateDeviceContextRenderTarget( hWnd );
	#else
		CreateHwndRenderTarget( hWnd );
	#endif

	CreateRenderTargetResource( cxt );
	
}


ComPTR<ID2D1SolidColorBrush> MakeBrsuh( ID2D1RenderTarget* cxt, D2D1_COLOR_F clr )
{
	ComPTR<ID2D1SolidColorBrush> br;
	cxt->CreateSolidColorBrush( clr, &br);
	return br;
}

//void DrawFillRect( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
//{	
//	D2DRectFilter f(cxt,rc);
//	cxt.cxt->FillRectangle( rc, fillclr );	
//	cxt.cxt->DrawRectangle( rc, wakuclr, width );					
//}

void DrawControlRect(D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr, ID2D1Brush* fillclr )
{
	cxt.cxt->DrawRectangle( rc, wakuclr );
	cxt.cxt->FillRectangle( rc, fillclr );

	// この後にD2DRectFilterをかけて、childrenをdrawする
}

void SetSystemCursor( UINT idx )
{
	::SetCursor((HCURSOR)::LoadImage(NULL, MAKEINTRESOURCE(idx), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
}


DWRITE_TEXT_METRICS DrawCenterText( D2DContext& cxt, ID2D1Brush* clr, const FRectF& rc, LPCWSTR str, int len, int align, float xoff )
{	
	DWRITE_TEXT_METRICS tm;
	cxt.cxtt.GetLineMetric( rc.Size(), str, len, tm );

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

	rcX.left+=xoff;

	cxt.cxt->DrawText( str, len, cxt.cxtt.textformat, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );

	return tm;
}


///TSF////////////////////////////////////////////////////////////////////////////////////////////

void InvertRect(D2DContext& cxt, FRectF* rc )
{
	//static bool bl = false;
	FRectF rcf(*rc);

	ComPTR<ID2D1SolidColorBrush> br;
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

//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DContextText
#define MAX_SIZE 10000.0f

bool D2DContextText::Init(D2DContext& cxt, float height, LPCWSTR fontname )
{	
	line_height = 0;
	xoff = 0;
	wfactory = cxt.insins->wrfactory;

	textformat.Release();
	if ( HR(wfactory->CreateTextFormat(fontname,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"",&textformat)))
	{
		ComPTR<IDWriteTextLayout> tl;
		wfactory->CreateTextLayout( L"T",1, textformat, MAX_SIZE, MAX_SIZE, &tl );
		
		DWRITE_HIT_TEST_METRICS mt;
	
		float y;
		tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );

		line_height = mt.height;

		return true;
	}
	return false;
}
UINT D2DContextText::GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics )
{
	ComPTR<IDWriteTextLayout> textlayout;
	wfactory->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 

	
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
	return GetLineMetric( sz, textformat, str,len, textMetrics ); 
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	ComPTR<IDWriteTextLayout> tl;
	wfactory->CreateTextLayout(str,len, fmt,(FLOAT)sz.width, (FLOAT)sz.height, &tl ); 
    tl->GetMetrics(&textMetrics);
	return textMetrics.lineCount;
}


#pragma endregion

};