#include "stdafx.h"
#include <d2d1helper.h>

using namespace D2D1;
struct SingletonD2DInstance
{
	CComPtr<IDWriteFactory> wrfactory;
	CComPtr<ID2D1Factory>  factory;
	CComPtr<IDWriteTextFormat> text;
	CComPtr<ID2D1HwndRenderTarget> target;
};


static SingletonD2DInstance st;




void Init( HWND hWnd )
{
	if ( st.factory.p == NULL )
	{
		HRESULT hr;
		D2D1_FACTORY_OPTIONS options;	
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),&options,(void**)&st.factory );
		_ASSERT(HR(hr));

		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&st.wrfactory));
		_ASSERT(HR(hr));
		
		hr = st.wrfactory->CreateTextFormat(DEFAULTFONT_JP, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DEFAULTFONT_HEIGHT_JP, L"ja-jp", &st.text);
		_ASSERT( HR(hr));

		hr = st.factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(1, 1), D2D1_PRESENT_OPTIONS_NONE), &st.target);
		_ASSERT( HR(hr));
	}	
}

void WindowSize(UINT cx, UINT cy )
{
	if( st.target )
		st.target->Resize( D2D1::SizeU( cx,cy ));
}
void HelloWorld()
{	
	HRESULT hr;

	st.target->BeginDraw();
	D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();

	st.target->SetTransform(mat);
	st.target->Clear( ColorF(ColorF::White));

	std::wstring str = L"Hello world";

	
	RECT rect;
	GetClientRect( st.target->GetHwnd(), &rect );
	
	
	D2D1_RECT_F rc;
	rc.left = (rect.left+rect.right)/2.0f;
	rc.top = (rect.top+rect.bottom)/2.0f;
	rc.right = rc.left+1000;
	rc.bottom = rc.top+1000;

	CComPtr<ID2D1SolidColorBrush> br;
	st.target->CreateSolidColorBrush(D2RGB(0,0,0), &br );
	
	st.target->DrawText( str.c_str(), str.length(), st.text, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_NONE );
	
	hr = st.target->EndDraw();

	if ( hr == D2DERR_RECREATE_TARGET )
	{
		HWND hWnd = st.target->GetHwnd();
		
		st.target.Release();
		st.factory.Release();
		st.text.Release();
		st.wrfactory.Release();

		Init( hWnd );
	}

}