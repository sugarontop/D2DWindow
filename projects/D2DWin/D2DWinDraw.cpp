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
#include "D2DWin.h"
#include "D2DWindowControl_easy.h"
#include <Wincodec.h>

using namespace V4; 

DLLEXPORT void WINAPI DDDrawStringEx(DDContext p, const V4::FRectF& rc, LPCWSTR str, int center)
{
	D2DContext& cxt = *(D2DContext*) p;

	V4::DrawCenterText(cxt, cxt.black, rc, str, lstrlen(str), center);
}

DLLEXPORT void WINAPI DDDrawString(DDContext p, float offx, float offy, LPCWSTR str)
{
	D2DContext& cxt = *(D2DContext*) p;

	FRectF rc(offx, offy, FSizeF(10000, 26));

	V4::DrawCenterText(cxt, cxt.black, rc, str, lstrlen(str), 0);
}
DLLEXPORT void WINAPI DDDrawWhiteString(DDContext p, float offx, float offy, LPCWSTR str)
{
	D2DContext& cxt = *(D2DContext*) p;

	FRectF rc(offx, offy, FSizeF(10000, 26));

	V4::DrawCenterText(cxt, cxt.white, rc, str, lstrlen(str), 0);
}
DLLEXPORT void WINAPI DDDrawString2(DDContext p, float offx, float offy, LPCWSTR str, DDColor c)
{
	D2DContext& cxt = *(D2DContext*) p;

	FRectF rc(offx, offy, FSizeF(10000, 26));

	auto br = MakeBrsuh(cxt, D2RGBA(c.r, c.g, c.b, c.a));

	V4::DrawCenterText(cxt, br, rc, str, lstrlen(str), 0);
}





DLLEXPORT void WINAPI DDDrawFillRect(DDContext p, float cx, float cy, const DDColor& c)
{
	D2DContext& cxt = *(D2DContext*) p;
	auto br = MakeBrsuh(cxt, D2RGBA(c.r, c.g, c.b, c.a));

	DrawFill(cxt, FRectF(0, 0, cx, cy), br);
}

DLLEXPORT void WINAPI DDDrawLine(DDContext p, FPointF& pt1, FPointF& pt2, int typ)
{
	D2DContext& cxt = *(D2DContext*) p;
	cxt.cxt->DrawLine(pt1, pt2, cxt.ltgray);
}
DLLEXPORT void WINAPI DDSetAlias(DDContext p, bool bOff)
{
	D2DContext& cxt = *(D2DContext*) p;
	cxt.cxt->SetAntialiasMode(bOff ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

DLLEXPORT bool WINAPI DDDivideImage(const DDImage& src, DDImage& dst, const D2D1_RECT_F& dstrc )
{
	dst = src;
	FRectF rc = dstrc;
	dst.pt = rc.LeftTop();	
	dst.sz = rc.Size();

	return true;
}


DLLEXPORT bool WINAPI DDCopyToImage(DDContext p, const DDImage& src, const D2D1_RECT_U& rc, DDImage& dst)
{
	D2DContext& cxt = *(D2DContext*) p;
	HRESULT hr;

	ComPTR<ID2D1Bitmap> bmp;
	ComPTR<ID2D1Bitmap> bdst;

	
	D2D1_BITMAP_PROPERTIES prop;
	{
		ZeroMemory(&prop,sizeof(prop));
		prop.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		prop.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		cxt.cxt->GetDpi(&prop.dpiX, &prop.dpiY);
	}

	D2D1_SIZE_U sz;
	sz.width = rc.right-rc.left;
	sz.height = rc.bottom - rc.top;
	
	hr = cxt.cxt->CreateBitmap( sz, prop, &bdst );
	if (hr != S_OK) return false;
	hr = src.p->QueryInterface(&bmp);
	if (hr != S_OK) return false;

	D2D1_POINT_2U pt;
	pt.x = 0;
	pt.y = 0;
	hr = bdst->CopyFromBitmap( &pt, bmp, &rc );
	if (hr != S_OK) return false;
	dst.p = bdst;
	dst.parent = p;

	return (hr == S_OK);
}


DLLEXPORT bool WINAPI DDLoadImage(DDContext p, DDImage& img)
{
	if (img.parent == p && img.p)
		return true;
	if (img.p)
	{
		img.p->Release();
		img.parent = nullptr;
	}

	HRESULT hr;
	ComPTR<IWICImagingFactory> pWICFactory;
	ComPTR<IWICStream> pStream;

	_ASSERT(img.Length);

	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**) &pWICFactory);
	if (hr != S_OK) return false;

	byte* d = (byte*) img.LockedResource;

	hr = pWICFactory->CreateStream(&pStream);
	if (hr != S_OK) return false;
	hr = pStream->InitializeFromMemory(d, img.Length); // bmp,gif,jpg,png OK
	if (hr != S_OK) return false;

	ComPTR<IWICBitmapDecoder> pDecoder;
	ComPTR<IWICBitmapFrameDecode> pSource;
	ComPTR<IWICFormatConverter> pConverter;

	hr = pWICFactory->CreateDecoderFromStream(pStream, 0, WICDecodeMetadataCacheOnLoad, &pDecoder); // jpeg,png:OK, bmp:88982f50のエラーになる, iconもエラー
	if (hr != S_OK) return false;
	hr = pDecoder->GetFrame(0, &pSource);
	if (hr != S_OK) return false;

	// Convert the image format to 32bppPBGRA(for D2D)
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = pWICFactory->CreateFormatConverter(&pConverter);
	if (hr != S_OK) return false;
	hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
	if (hr != S_OK) return false;

	D2DContext& cxt = *(D2DContext*) p;

	ComPTR<ID2D1Bitmap> bmp;
	hr = cxt.cxt->CreateBitmapFromWicBitmap(pConverter, NULL, &bmp);
	if (hr != S_OK) return false;
	img.p = bmp;
	img.parent = p;

	return (hr == S_OK);
}
//DLLEXPORT void WINAPI DDDrawImage(DDContext p, const FRectF& rc, DDImage& img)
//{
//	D2DContext& cxt = *(D2DContext*) p;
//	cxt.cxt->DrawBitmap((ID2D1Bitmap*) img.p.p, rc,1.0f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &rcx);
//}


DLLEXPORT void WINAPI DDDrawImage(DDContext p, const FRectF& rc, DDImage& img)
{
	D2DContext& cxt = *(D2DContext*) p;
	FRectF imgrc( img.pt, img.sz); 
	cxt.cxt->DrawBitmap((ID2D1Bitmap*) img.p.p, rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &imgrc);
}









namespace V4 
{

	bool CreateD2D1Bitmap(D2DContext& cxt, ID2D1Bitmap** ppBitmap, Binary& bin )
	{
		HRESULT hr;
		ComPTR<IWICImagingFactory> pWICFactory;
		ComPTR<IWICStream> pStream;

		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**) &pWICFactory);
		if (hr != S_OK) return false;

		byte* d = (byte*)bin.get();  // img.LockedResource;

		hr = pWICFactory->CreateStream(&pStream);
		if (hr != S_OK) return false;
		hr = pStream->InitializeFromMemory(d, bin.length()); // bmp,gif,jpg,png OK
		if (hr != S_OK) return false;

		ComPTR<IWICBitmapDecoder> pDecoder;
		ComPTR<IWICBitmapFrameDecode> pSource;
		ComPTR<IWICFormatConverter> pConverter;

		hr = pWICFactory->CreateDecoderFromStream(pStream, 0, WICDecodeMetadataCacheOnLoad, &pDecoder); // jpeg,png:OK, bmp:88982f50のエラーになる, iconもエラー
		if (hr != S_OK) return false;
		hr = pDecoder->GetFrame(0, &pSource);
		if (hr != S_OK) return false;

		// Convert the image format to 32bppPBGRA(for D2D)
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pWICFactory->CreateFormatConverter(&pConverter);
		if (hr != S_OK) return false;
		hr = pConverter->Initialize(pSource.p, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if (hr != S_OK) return false;


		hr = cxt.cxt->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
		if (hr != S_OK) return false;

		return (hr == S_OK);
	}
}
