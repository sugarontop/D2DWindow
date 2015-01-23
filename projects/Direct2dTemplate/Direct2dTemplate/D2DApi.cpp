#include "stdafx.h"
#include "D2DWindowControl.h"



namespace V4 {

void CreateVerticalGradation(ID2D1RenderTarget* target, const D2D1_SIZE_F& sz, D2D1_COLOR_F& clr1, D2D1_COLOR_F& clr2, ID2D1LinearGradientBrush** br )
{
	D2D1_GRADIENT_STOP gradientStops[] = {{ 0.0f, clr1},{ 1.0f, clr2 }};

	CComPtr<ID2D1GradientStopCollection> gradientStopsCollection;
	target->CreateGradientStopCollection(gradientStops,_countof(gradientStops),&gradientStopsCollection);
	
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties = D2D1::LinearGradientBrushProperties(FPointF(0,0), FPointF( 0, sz.height));
	target->CreateLinearGradientBrush(properties, gradientStopsCollection.p, br);
}

void DrawCenterText( ID2D1RenderTarget* cxt, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align )
{	
	//IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;
	//IDWriteTextFormat* fmt = D2DResource::s_Active->TextFormat_;

	
	CComPtr<IDWriteFactory> fc;
	auto hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&fc));

	CComPtr<IDWriteTextFormat> fmt;
	hr = fc->CreateTextFormat( DEFAULTFONT ,0,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,14,L"",&fmt);
	

	
	DWRITE_TEXT_METRICS tm;
	FSizeF sz( rc.right-rc.left, rc.bottom-rc.top );
	CComPtr<IDWriteTextLayout> textlayout;
	fc->CreateTextLayout(str,len,fmt,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 
    textlayout->GetMetrics(&tm);

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

	cxt->DrawText( str, len, fmt, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
}


void DrawRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width )
{
	cxt->DrawRectangle( rc, br, width );	
}
void DrawButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, FString str, ID2D1Brush* br1 )
{
	CComPtr<ID2D1LinearGradientBrush> br;
	CreateVerticalGradation( cxt, FSizeF(1,100), D2RGBA(200,200,200,0) , D2RGBA(200,200,200,200) , &br );

	br->SetStartPoint( FPointF(0,rc.top));
	br->SetEndPoint( FPointF(0,rc.bottom));

	{
		CComPtr<ID2D1SolidColorBrush> br2;
		cxt->CreateSolidColorBrush(D2RGBA(230,230,230,200 ), &br2 );
		cxt->FillRectangle( rc, br2 );

		FRectF rc2(rc);
		rc2.InflateRect( -2,-2);
		cxt->FillRectangle( rc2, br );
	}
	
	DrawCenterText( cxt, br1, rc, str.c_str(), str.length(), 1 );

}


};