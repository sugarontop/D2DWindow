#include "stdafx.h"
#include "D2DApi.h"
#include "D2DResource.h"

#pragma comment(lib, "Windowscodecs.lib")

namespace V4{ 

D2DResource* D2DResource::s_Active = NULL;

DLLEXPORT D2DRES WINAPI ResourceAllocate( ID2D1RenderTarget* cxt, IDWriteFactory* wfct, HWND hWnd )
{	
	D2DRES r;

	D2DResource* p = new D2DResource(cxt, hWnd);
	if ( p->Initialize(wfct) )
	{
		r.id = 0; // not use
		r.res = p;
	}
	else
	{
		r.id = -1;
		r.res = nullptr;
		delete p;
	}

	return r;
}
DLLEXPORT void WINAPI ResourceRelease( D2DRES& s )
{	
	delete s.res;
	s.res = nullptr;

}
DLLEXPORT void WINAPI SelectResource( const D2DRES& s )
{
	D2DResource::s_Active = s.res;
}
DLLEXPORT HWND WINAPI GetActiveHwnd()
{
	return D2DResource::s_Active->hWnd_;
}
DLLEXPORT ID2D1RenderTarget* WINAPI GetActiveRenderTarget()
{
	return D2DResource::s_Active->handle_;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool D2DResource::Initialize(IDWriteFactory* wfct)
{
	textwritefactory_ = wfct;

	textwritefactory_->CreateTextFormat( 
		DEFAULTFONT,
		0,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		12,
		L"",
		&TextFormat_);

	CreateVerticalGradation( handle_, FSizeF(1,100), D2RGBA(200,200,200,0) , D2RGBA(200,200,200,200) , &Silver1_ );
	CreateVerticalGradation( handle_, FSizeF(1,100), D2RGB(255,255,255) , D2RGB(200,200,200) , &Silver2_ );


	handle_->CreateSolidColorBrush( D2RGB(0,0,0), &black_ );
	handle_->CreateSolidColorBrush( D2RGB(255,255,255), &white_ );

	CComPtr<ID2D1Factory> factory;
	handle_->GetFactory(&factory);
	
	D2D1_STROKE_STYLE_PROPERTIES prop = D2D1::StrokeStyleProperties();
		
	prop.dashStyle = D2D1_DASH_STYLE_SOLID;
	float dashes[] ={5, 5, 5};
	factory->CreateStrokeStyle(prop, dashes,_countof(dashes), &pen_[0]);

	prop.dashStyle = D2D1_DASH_STYLE_CUSTOM;
	float dashes1[] ={5, 5, 5};
	factory->CreateStrokeStyle(prop, dashes1,_countof(dashes1), &pen_[1]); 

	prop.dashStyle = D2D1_DASH_STYLE_CUSTOM;
	float dashes2[] ={2, 2, 2};
	factory->CreateStrokeStyle(prop, dashes2,_countof(dashes2), &pen_[2]); 

	return true;	
}

bool D2DResource::LoadImage( ID2D1RenderTarget* target, LPCWSTR filenm, ID2D1Bitmap** bmp )
{
	HRESULT hr;
	CComPtr<IWICStream> pStream;
	CComPtr<IWICImagingFactory> pWICFactory;	

	hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory );
	if ( hr != S_OK ) return false;

	hr = pWICFactory->CreateStream( &pStream );
	if ( hr != S_OK ) return false;
	
	hr = pStream->InitializeFromFilename( filenm, GENERIC_READ); // bmp,gif,jpg,png OK
	if ( hr != S_OK ) return false;
	
	return LoadImage2( target,pWICFactory, pStream, bmp );
}

bool D2DResource::LoadImage( ID2D1RenderTarget* target, IStream* sm, ID2D1Bitmap** bmp )
{
	HRESULT hr;
	CComPtr<IWICStream> pStream;
	CComPtr<IWICImagingFactory> pWICFactory;	

	hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory );
	if ( hr != S_OK ) return false;

	hr = pWICFactory->CreateStream( &pStream );
	if ( hr != S_OK ) return false;
	
	hr = pStream->InitializeFromIStream( sm ); // bmp,gif,jpg,png OK
	if ( hr != S_OK ) return false;
	
	return LoadImage2( target,pWICFactory, pStream, bmp );
}

bool D2DResource::LoadImage( ID2D1RenderTarget* target, LPCWSTR resource_section, int resource_id, ID2D1Bitmap** bmp )
{
	HRESULT hr;
	CComPtr<IWICImagingFactory> pWICFactory;	
	CComPtr<IWICStream> pStream;

	hr= CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory );
	if ( hr != S_OK ) return false;

	// load png from .rc
	HMODULE hmodule = ::GetModuleHandle(NULL);
	HRSRC hsrc = FindResource( hmodule,  MAKEINTRESOURCE(resource_id), resource_section ); // resource_section:png
	if ( hsrc == NULL ) return false;

	DWORD len = SizeofResource(hmodule,hsrc);
	HGLOBAL hg = LoadResource( hmodule, hsrc );
	byte* d = (byte*)LockResource( hg );

	hr = pWICFactory->CreateStream( &pStream );
	if ( hr != S_OK ) return false;	
	hr = pStream->InitializeFromMemory( d, len ); // bmp,gif,jpg,png OK
	if ( hr != S_OK ) return false;

	return LoadImage2( target,pWICFactory, pStream, bmp );
}


bool D2DResource::LoadImage2( ID2D1RenderTarget* target,IWICImagingFactory* pWICFactory,IWICStream* pStream, ID2D1Bitmap** bmp )
{
	_ASSERT( pWICFactory );

	HRESULT hr;

	CComPtr<IWICBitmapDecoder> pDecoder;
	CComPtr<IWICBitmapFrameDecode> pSource;	
	CComPtr<IWICFormatConverter> pConverter;

	hr = pWICFactory->CreateDecoderFromStream( pStream, 0, WICDecodeMetadataCacheOnLoad, &pDecoder ); // jpeg,png:OK, bmp:88982f50のエラーになる, iconもエラー
	if ( hr != S_OK ) return false;
	hr = pDecoder->GetFrame(0, &pSource);
	if ( hr != S_OK ) return false;

	// Convert the image format to 32bppPBGRA
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = pWICFactory->CreateFormatConverter(&pConverter);
	if ( hr != S_OK ) return false;
	hr = pConverter->Initialize( pSource, GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);
	if ( hr != S_OK ) return false;
	hr = target->CreateBitmapFromWicBitmap(pConverter,NULL,bmp );

	return ( hr == S_OK );
}


};




