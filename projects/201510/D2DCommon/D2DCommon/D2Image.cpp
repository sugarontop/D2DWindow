#include "stdafx.h"
#include <wincodec.h>
#include "D2Image.h"



bool LoadImage2( ID2D1DeviceContext * target,IWICImagingFactory* pWICFactory,IWICStream* pStream, ID2D1Bitmap1** bmp )
{
	_ASSERT( pWICFactory );

	HRESULT hr;

	ComPTR<IWICBitmapDecoder> pDecoder;
	ComPTR<IWICBitmapFrameDecode> pSource;	
	ComPTR<IWICFormatConverter> pConverter;

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
//bool LoadImage( ID2D1RenderTarget* target, IStream* sm, ID2D1Bitmap** bmp )
//{
//	HRESULT hr;
//	ComPTR<IWICStream> pStream;
//	ComPTR<IWICImagingFactory> pWICFactory;	
//
//	hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory );
//	if ( hr != S_OK ) return false;
//
//	hr = pWICFactory->CreateStream( &pStream );
//	if ( hr != S_OK ) return false;
//	
//	hr = pStream->InitializeFromIStream( sm ); // bmp,gif,jpg,png OK
//	if ( hr != S_OK ) return false;
//	
//	return LoadImage2( target,pWICFactory, pStream, bmp );
//}
bool LoadImage( ID2D1DeviceContext * target, LPCWSTR filenm, ID2D1Bitmap1** bmp )
{
	HRESULT hr;
	ComPTR<IWICStream> pStream;
	ComPTR<IWICImagingFactory> pWICFactory;	

	hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)&pWICFactory );
	if ( hr != S_OK ) return false;

	hr = pWICFactory->CreateStream( &pStream );
	if ( hr != S_OK ) return false;
	
	hr = pStream->InitializeFromFilename( filenm, GENERIC_READ); // bmp,gif,jpg,png OK
	if ( hr != S_OK ) return false;
	
	return LoadImage2( target,pWICFactory, pStream, bmp );
}