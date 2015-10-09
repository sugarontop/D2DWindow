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

#pragma once

#include "D2DMisc.h"


namespace V4 
{
struct D2DContextBase
	{
		ComPTR<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* text;
		IDWriteFactory* wfactory;
	};


// SingletonD2DInstanceは独立した存在なので、HWNDに関わるリソースはもたない。
struct SingletonD2DInstance
{
	ComPTR<IDWriteFactory2> wrfactory;
	ComPTR<ID2D1Factory2>  factory;
	ComPTR<IDWriteTextFormat> text; // IDWriteTextFormat1 is from Win8.1.

	static SingletonD2DInstance& Init();

};

#define STOCKSIZE 16
struct D2DContext;



struct D2DContextText
{
	bool Init(D2DContext& inshw,  float height, LPCWSTR fontname );

	UINT GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics );
	UINT GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric );
	UINT GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics );
	UINT GetLineMetric( const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics );

	ComPTR<IDWriteTextFormat> textformat;
	ComPTR<IDWriteFactory2> wfactory;
	
	float xoff;			// １行表示の左端の余幅
	float line_height;	// １行表示の高さ

};
struct D2DContext : public D2DContextBase
{	
	/*
	struct D2DContextBase
	{
		ComPTR<ID2D1RenderTarget>  cxt;
		IDWriteTextFormat* text;
		IDWriteFactory* wfactory;
	};
	*/
	
	

	SingletonD2DInstance* insins;

	
	operator ID2D1RenderTarget*() const{ return cxt.p; } 
#ifdef USE_ID2D1DEVICECONTEXT
	ComPTR<IDXGISwapChain1> dxgiSwapChain;
	D2D1_SIZE_U RenderSize_;
#endif

	ComPTR<ID2D1SolidColorBrush> ltgray;
	ComPTR<ID2D1SolidColorBrush> black;
	ComPTR<ID2D1SolidColorBrush> white;
	ComPTR<ID2D1SolidColorBrush> red;
	ComPTR<ID2D1SolidColorBrush> gray;
	ComPTR<ID2D1SolidColorBrush> bluegray;
	ComPTR<ID2D1SolidColorBrush> transparent;
	ComPTR<ID2D1SolidColorBrush> halftone;
	ComPTR<ID2D1SolidColorBrush> halftoneRed;
	ComPTR<ID2D1SolidColorBrush> tooltip;

	ComPTR<ID2D1StrokeStyle> dot4_;
	ComPTR<ID2D1StrokeStyle> dot2_;

	ComPTR<ID2D1Factory2> factory(){ return insins->factory; }
	D2DContextText cxtt;

	ID2D1SolidColorBrush* forecolor_;

	LPVOID free_space;

	void Init(SingletonD2DInstance& ins, HWND hWnd );
	void Destroy();

	void Init(SingletonD2DInstance& ins);
	
	
	void CreateResourceOpt();

	void CreateDeviceContextRenderTargetTest( HWND hWnd, UINT width );

	
	void DestroyRenderTargetResource();

	void DestroyAll();

	void SetAntiAlias(bool bl){ cxt->SetAntialiasMode( bl ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE:D2D1_ANTIALIAS_MODE_ALIASED);} 


	HRESULT CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret );


	void CreateRenderResource( HWND hWnd );


	protected :
		void CreateHwndRenderTarget( HWND hWnd );
		void CreateDeviceContextRenderTarget( HWND hWnd );
		void CreateRenderTargetResource( ID2D1RenderTarget* t );

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
//struct D2DRectFilterType1
//{
//	D2DRectFilterType1(D2DContext& cxt1, FRectF rc ):cxt(cxt1)
//	{
//		rc.left--; rc.top--;
//		cxt.cxt->PushAxisAlignedClip( rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
//	}
//	~D2DRectFilterType1()
//	{
//		cxt.cxt->PopAxisAlignedClip();
//	}
//
//	private :
//		D2DContext& cxt;
//};
//
//struct SingleLineText
//{
//	FPointF ptLineText;
//	ComPTR<IDWriteTextLayout> textlayout;
//
//	void CreateLayout(D2DContextText& cxt, const FRectF& rc, LPCWSTR str, int len, int align );
//
//	void CreateLayoutEx(D2DContextText& cxt,IDWriteTextFormat* fmt, const FRectF& rc, LPCWSTR str, int len, int align );
//
//	void DrawText(D2DContext& cxt, ID2D1Brush* foreclr );
//};
/////////////////////////////////////////////////////////////////////////////////////////////
class D2DError 
{
	public :
		explicit D2DError( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm):hr_(hr),msg_(msg),line_(line),fnm_(fnm )
		{
			FString msg1 = FString::Format(L"%s :%d行 HR=%x %s\n", (LPCWSTR)fnm_, line_,hr_,(LPCWSTR)msg_);		

			::OutputDebugString( msg1 );
		}

	public :
		FString msg_;
		FString fnm_;
		UINT line_;
		HRESULT hr_;

};
inline void ThrowIfFailed( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm )
{
	if (FAILED(hr))
	{
		throw D2DError( hr, msg, line,fnm );
		//General access denied error 0x80070005 
	}
}
#define THROWIFFAILED(hr,msg) ThrowIfFailed(hr,msg, __LINE__, __FILE__)


DWRITE_TEXT_METRICS DrawCenterText( D2DContext& cxt, ID2D1Brush* clr, const FRectF& rc, LPCWSTR str, int len,int align, float xoff=0  );

void DrawControlRect(D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr, ID2D1Brush* fillclr);
void SetSystemCursor(UINT idx);
ComPTR<ID2D1SolidColorBrush> MakeBrsuh( ID2D1RenderTarget* cxt, D2D1_COLOR_F clr );

typedef std::function<void(HWND,void*)> DispatherDelegate;


//以下 TSFのために追加 /////////////////////////////////////////////////////////////////////
void InvertRect(D2DContext& cxt, FRectF* rc );

bool DrawCaret(D2DContext& cxt, const FRectF& rc );

};
