#include "stdafx.h"
#include "D2DContextEx.h"

static LARGE_INTEGER __s_frequency_;

namespace V4{

SingletonD2DInstance& SingletonD2DInstance::Init()
{
	static SingletonD2DInstance st;
		
	if ( st.factory.p == NULL )
	{
		HRESULT hr;
		D2D1_FACTORY_OPTIONS options;	
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory1),&options,(void**)&st.factory );
		_ASSERT(HR(hr));

		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&st.wrfactory));
		_ASSERT(HR(hr));
		
		hr = st.wrfactory->CreateTextFormat(DEFAULTFONT, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DEFAULTFONT_HEIGHT, L"", &st.text);
		_ASSERT( HR(hr));
	}

	return st;
}

void D2DContext::Init(SingletonD2DInstance& ins )
{
	insins = &ins;
}
void D2DContext::CreateHwndRenderTarget( HWND hWnd )
{
	// 未使用
	ID2D1Factory1* factory = insins->factory;		
	CComPtr<ID2D1HwndRenderTarget> temp;

	auto hr = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(1, 1), D2D1_PRESENT_OPTIONS_NONE), &temp);
	xassert(HR(hr));
	
	hr= temp.QueryInterface( &cxt );
	xassert(HR(hr));
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
}
	
void D2DContext::CreateResourceOpt()
{
	// RenderTargetとは関係ないリソース作成
	
	
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
void D2DContext::DestroyRenderTargetResource()
{
	cxt.Release();
	dxgiSwapChain.Release();

	ltgray.Release();
	black.Release();
	white.Release();
	red.Release();
	gray.Release();
	bluegray.Release();
	transparent.Release();
	halftone.Release();
	halftoneRed.Release();
	
}
void D2DContext::DestroyAll()
{
	// OnDestroyのタイミングで実行

	DestroyRenderTargetResource();

}


	
void D2DContextText::Init(D2DContext& inshw, float height, LPCWSTR fontname )
{	
	cxt = &inshw;
	line_height = 0;
	xoff = 0;
	
	textformat.Release();
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
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch Win32 API errors.
        throw L"fail";
    }
}

void D2DContext::CreateDeviceContextRenderTarget( HWND hWnd )
{
	RECT rc;
	GetClientRect(hWnd, &rc);

//	UINT width = ::GetSystemMetrics( SM_CXFULLSCREEN );
//	UINT height = ::GetSystemMetrics( SM_CYFULLSCREEN );

	UINT width = max(800, rc.right);
	UINT height = max(800, rc.bottom);

	CComPtr<ID2D1DeviceContext> d2d1DeviceContext;
	CComPtr<IDXGISwapChain1> dxgiSwapChain1;
	CComPtr<ID2D1Factory1> d2d1Factory = insins->factory;

	CComPtr<ID2D1Device> d2d1Device;
	CComPtr<ID2D1Bitmap1> d2d1Bitmap;

	CComPtr<ID3D11DeviceContext> d3d11DeviceContext;
	CComPtr<ID3D11Device> d3d11Device;
	CComPtr<IDXGISurface> dxgiSurface;
	CComPtr<IDXGIAdapter> dxgiAdapter;
	CComPtr<IDXGIFactory2> dxgiFactory;
	CComPtr<IDXGIDevice1> dxgiDevice;

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


	ThrowIfFailed( D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
					&d3d11Device, &returnedFeatureLevel, &d3d11DeviceContext));


	ThrowIfFailed(d3d11Device->QueryInterface(&dxgiDevice));
	ThrowIfFailed(d2d1Factory->CreateDevice(dxgiDevice.p, &d2d1Device));
	ThrowIfFailed(d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d1DeviceContext));
	ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
	ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

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

	auto hr = dxgiFactory->CreateSwapChainForHwnd(d3d11Device,
													hWnd,
													&swapChainDesc,
													nullptr,
													nullptr,
													&dxgiSwapChain1);
	xassert( hr == S_OK );

	ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));
	ThrowIfFailed(dxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface)));

	auto bmp_property = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
	ThrowIfFailed(d2d1DeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.p, &bmp_property, &d2d1Bitmap));
	d2d1DeviceContext->SetTarget(d2d1Bitmap.p);

	auto sz = d2d1Bitmap->GetSize();

	_ASSERT( sz.width == width  );
	_ASSERT( sz.height == height  );
	//////////////////////////////////////////////////////////////////////////////////////////////

	d2d1DeviceContext->QueryInterface( IID_ID2D1RenderTarget, (void**)&cxt );

	dxgiSwapChain = dxgiSwapChain1;
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

void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr )
{
	FRectF yrc(rc);
	yrc.InflateRect(-1,-1);
	cxt.cxt->FillRectangle( yrc, fillclr );	
}






///TSF////////////////////////////////////////////////////////////////////////////////////////////




};