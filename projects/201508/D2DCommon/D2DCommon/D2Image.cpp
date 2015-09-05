/*
The MIT License (MIT)
Copyright (c) 2015 sugarontop@icloud.com
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
