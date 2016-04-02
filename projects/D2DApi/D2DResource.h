#pragma once

namespace V4 {
class D2DResource
{
	public :
		D2DResource( ID2D1RenderTarget* handle, HWND hWnd ):handle_(handle),hWnd_(hWnd){}

		bool Initialize(IDWriteFactory* wfct);

		ID2D1RenderTarget* handle_;
		IDWriteFactory* textwritefactory_;
		HWND hWnd_;

		CComPtr<ID2D1LinearGradientBrush> Silver1_;
		CComPtr<ID2D1LinearGradientBrush> Silver2_;

		CComPtr<ID2D1SolidColorBrush> black_;
		CComPtr<ID2D1SolidColorBrush> white_;
		CComPtr<ID2D1StrokeStyle> pen_[3];		
		CComPtr<IDWriteTextFormat> TextFormat_;


		

	public :
		static D2DResource* s_Active;
		
		static bool LoadImage( ID2D1RenderTarget* target, LPCWSTR filenm, ID2D1Bitmap** bmp );
		static bool LoadImage( ID2D1RenderTarget* target, IStream* filenm, ID2D1Bitmap** bmp );
		static bool LoadImage2( ID2D1RenderTarget* target, IWICImagingFactory* pWICFactory, IWICStream* pStream, ID2D1Bitmap** bmp );
		static bool LoadImage( ID2D1RenderTarget* target, LPCWSTR resource_section, int resource_id, ID2D1Bitmap** bmp );

};

};