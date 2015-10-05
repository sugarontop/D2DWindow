// D2DCommon.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "D2DCommon.h"
#include "D2Image.h"



using namespace D2D1;
using namespace V4;


#define HR(hr) (hr == S_OK)

namespace V4 {

FPointF* WINAPI Triangle( int typ, float cx, float cy, int* ptcount );


DLLEXPORT void WINAPI DrawRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width )
{		
	//RenderFilter f(cxt,rc);
	cxt->DrawRectangle( rc, br, width );			
}
DLLEXPORT float WINAPI DrawCenterText( ID2D1RenderTarget* cxt,D2DTextWrite& info, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align )
{	
	IDWriteFactory* fc = info.factory; 
	auto fmt = info.textformat;
	
	DWRITE_TEXT_METRICS tm;
	FSizeF sz( rc.right-rc.left, rc.bottom-rc.top );
	ComPTR<IDWriteTextLayout> textlayout;

	fc->CreateTextLayout(str,len,fmt, 32000, sz.height, &textlayout ); // single line only
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
	return tm.width;
}

DLLEXPORT void WINAPI DrawButton( ID2D1RenderTarget* cxt,D2DTextWrite& info, const D2D1_RECT_F& rc, LPCWSTR str, ID2D1Brush* br1 )
{	
	ComPTR<ID2D1LinearGradientBrush> br;

	V4::CreateVerticalGradation( cxt, FSizeF(1,100), D2RGBA(200,200,200,0) , D2RGBA(200,200,200,200) , &br );

	br->SetStartPoint( FPointF(0,rc.top));
	br->SetEndPoint( FPointF(0,rc.bottom));

	{
		ComPTR<ID2D1SolidColorBrush> br2;
		cxt->CreateSolidColorBrush(D2RGBA(230,230,230,200 ), &br2 );
		cxt->FillRectangle( rc, br2 );

		FRectF rc2(rc);
		rc2.InflateRect( -2,-2);
		cxt->FillRectangle( rc2, br );
	}
	
	V4::DrawCenterText( cxt, info, br1, rc, str, lstrlen(str), 1 );
}
DLLEXPORT void WINAPI CreateVerticalGradation(ID2D1RenderTarget* target, const D2D1_SIZE_F& sz, D2D1_COLOR_F& clr1, D2D1_COLOR_F& clr2, ID2D1LinearGradientBrush** br )
 {
	D2D1_GRADIENT_STOP gradientStops[] = {{ 0.0f, clr1},{ 1.0f, clr2 }};

	ComPTR<ID2D1GradientStopCollection> gradientStopsCollection;
	target->CreateGradientStopCollection(gradientStops,_countof(gradientStops),&gradientStopsCollection);
	
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties = D2D1::LinearGradientBrushProperties(FPointF(0,0), FPointF( 0, sz.height));
	target->CreateLinearGradientBrush(properties, gradientStopsCollection.p, br);
 }
 DLLEXPORT void WINAPI DrawScrollbar( ID2D1RenderTarget* cxt, D2DScrollbarInfo& info )
{
	ComPTR<ID2D1SolidColorBrush> bkcolor,br1b,br2;
	cxt->CreateSolidColorBrush(D2RGBA(230,230,230,255 ), &bkcolor );
	cxt->CreateSolidColorBrush(D2RGBA(200,200,200,255 ), &br1b );
	cxt->CreateSolidColorBrush(D2RGBA(100,100,100,255 ), &br2 );
	
	cxt->FillRectangle( info.rc, bkcolor ); // 全体

	

	if ( info.bVertical )
	{
		// Upper button
		FRectF rc = V4::ScrollbarRect( info, 1 );
		FPointF cpt = rc.CenterPt();


		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3+2, info.rc.top+5 ) );				
		V4::FillArrow( cxt, cpt, ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 0 );
		

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3+2, info.rc.bottom-10 ) );				

		// Lower button
		rc = V4::ScrollbarRect( info, 2 );		
		V4::FillArrow( cxt, cpt, ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 2 );

		cxt->SetTransform( &mat ); // 戻す



		// Thumb button
		rc = V4::ScrollbarRect( info, 3 );
		rc.InflateRect( -4 ,0);
		

		info.thumb_rc = rc;

		bool bl = (bool)((info.stat & D2DScrollbarInfo::ONTHUMB) || (info.stat & D2DScrollbarInfo::CAPTURED));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	else
	{
		// left side button
		FRectF rc = V4::ScrollbarRect( info, 1 );
		FPointF cpt = rc.CenterPt();


		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+5, info.rc.top+5 ) );				
		V4::FillArrow( cxt, cpt, ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 3 );		
		
		cxt->SetTransform( mat.CalcOffset( info.rc.right-10, info.rc.top+5 ) );				

		// right side button
		rc = V4::ScrollbarRect( info, 2 );		
		V4::FillArrow( cxt, cpt, ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 1 );		

		cxt->SetTransform( &mat ); // 戻す

		// Thumb button
		rc = V4::ScrollbarRect( info, 3 );
		
		rc.InflateRect( 0,-4);
		

		info.thumb_rc = rc;

		bool bl = (0!=(info.stat & D2DScrollbarInfo::ONTHUMB | info.stat & D2DScrollbarInfo::CAPTURED));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	
}
DLLEXPORT D2D1_RECT_F WINAPI ScrollbarRect( D2DScrollbarInfo& info, int typ )
{
	if ( typ == 0 )
		return info.rc;
	
	if ( info.bVertical )
	{
	
		if ( typ == 1 )
		{
			FRectF rc = info.rc;
			rc.bottom = rc.top + info.button_height;
			return rc;
		}
		else if ( typ == 2 )
		{
			FRectF rc = info.rc;
			rc.bottom = info.rc.bottom;
			rc.top = info.rc.bottom - info.button_height;		
			return rc;
		}
		else if ( typ == 3 )
		{
			float min_thumb = 17; // 最少thumb
			float y1 = info.rc.bottom-info.rc.top;
			float y2 = info.total_height - y1;

			float thumb_height = y1 - info.button_height*2;
			float rto = 1.0f;

			if ( y2 <= 0 )
			{
				// scrollbar 不要
			}
			else if ( y1-y2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = y1-y2-info.button_height*2;
			}
			else
			{
				thumb_height = min_thumb;
				float y2a = y1-thumb_height-info.button_height*2;

				rto = y2a / y2;

			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.top = info.position +  info.rc.top + info.button_height;
			thumbrc.bottom = thumbrc.top + thumb_height;

			if ( thumbrc.bottom > info.rc.bottom-info.button_height )
			{
				thumbrc.bottom = info.rc.bottom-info.button_height;
				thumbrc.top = thumbrc.bottom - thumb_height;

			}

			return thumbrc;
		}
	}
	else
	{
		float button_width = info.button_height;
		if ( typ == 1 )
		{
			FRectF rc = info.rc;
			rc.right = rc.left + button_width;
			return rc;
		}
		else if ( typ == 2 )
		{
			FRectF rc = info.rc;
			rc.right = info.rc.right;
			rc.left = rc.right - button_width;		
			return rc;
		}
		else if ( typ == 3 )
		{
			float min_thumb = 5; // 最少thumb
			float w1 = info.rc.right-info.rc.left;
			float w2 = info.total_height - w1;

			float thumb_height = w1 - info.button_height*2;
			float rto = 1.0f;

			if ( w2 <= 0 )
			{
				// scrollbar 不要
			}
			else if ( w1-w2-info.button_height*2-min_thumb >0 )
			{
				thumb_height = w1-w2-info.button_height*2;
			}
			else
			{
				thumb_height = min_thumb;
				float ywa = w1-thumb_height-info.button_height*2;

				rto = ywa / w2;

			}
		
			info.thumb_step_c = rto;
		
			FRectF thumbrc = info.rc;		
			thumbrc.left = info.position +  info.rc.left + info.button_height;
			thumbrc.right = thumbrc.left + thumb_height;

			if ( thumbrc.right > info.rc.right-info.button_height )
			{
				thumbrc.right = info.rc.right-info.button_height;
				thumbrc.left = thumbrc.left - thumb_height;

			}

			return thumbrc;
		}


	}
	return FRectF(0,0,0,0);
}
DLLEXPORT void WINAPI FillArrow( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, ID2D1Brush* br, int typ )
{
	ComPTR<ID2D1Factory> factory;
	cxt->GetFactory(&factory);

	ComPTR<ID2D1PathGeometry> pathg;
	factory->CreatePathGeometry( &pathg );

	int cnt;
	FPointF* pt = Triangle( typ, 10, 5 , &cnt );
	CreatePathGeometry( pathg, pt, cnt );
	cxt->FillGeometry( pathg, br );

	delete [] pt;
}
FPointF* WINAPI Triangle( int typ, float cx, float cy, int* ptcount )
{
	_ASSERT( typ < 4 && -1 < typ );
	*ptcount = 3;
	FPointF* pt = new FPointF[*ptcount];

	if ( typ == 0 )
	{
		// ▲ 図形
		pt[0] = FPointF(cx/2,0);
		pt[1] = FPointF(cx,cy);
		pt[2] = FPointF(0,cy);
	}
	else if ( typ == 1 )
	{
		pt[0] = FPointF(0,0);
		pt[1] = FPointF(cx,cy/2);
		pt[2] = FPointF(0,cy);
	}
	else if ( typ == 2 )
	{
		// ▼ 図形
		pt[0] = FPointF(cx/2,cy);
		pt[1] = FPointF(cx,0);
		pt[2] = FPointF(0,0);
	}
	else if ( typ == 3 )
	{
		pt[0] = FPointF(cx,0);
		pt[1] = FPointF(0,cy/2);
		pt[2] = FPointF(cx,cy);
	}

	return pt;
}
DLLEXPORT bool WINAPI CreatePathGeometry(ID2D1PathGeometry* pathg, const D2D1_POINT_2F* pt, int ptcount)
{
	_ASSERT( ptcount );
	_ASSERT( pathg );
	ComPTR<ID2D1GeometrySink> sink;	
	if ( HR(pathg->Open( &sink )))
	{
		sink->BeginFigure( pt[0], D2D1_FIGURE_BEGIN_FILLED );		
		for( int i = 1; i < ptcount; i++ )
			sink->AddLine( pt[i] );
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();
		return true;
	}
	return false;
}
DLLEXPORT void WINAPI CreateTagButtomGeometry(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rcbutton, const D2D1_RECT_F& rect, ID2D1PathGeometry** ret )
{
	_ASSERT( rcbutton.top == rcbutton.top );

	// rcは全体
	// rcButtonは飛び出たボタン部分
	/*
	    ____
	  __|   |___
	 |          | 
	 |__________| <-- CreateGeomery
	 
	*/

	FRectF rcButton = rcbutton;
	FRectF rc = rect;

	ComPTR<ID2D1PathGeometry> pGeometry;
	ComPTR<ID2D1Factory> factory;
	cxt->GetFactory(&factory);		
	factory->CreatePathGeometry(&pGeometry);

	ComPTR<ID2D1GeometrySink> pSink;
	pGeometry->Open(&pSink);
	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
	float y = rcButton.Height();

	FPointF pt[8];

	pt[0] = rcButton.LeftTop();
	pt[1] = rcButton.RightTop();
	pt[2] = rcButton.RightBottom();
	pt[7] = rcButton.LeftBottom();

	pt[3] = rc.RightTop(); pt[3].y += y;
	pt[4] = rc.RightBottom();
	pt[5] = rc.LeftBottom();
	pt[6] = rc.LeftTop(); pt[6].y += y;
			
	pSink->BeginFigure( pt[0],D2D1_FIGURE_BEGIN_FILLED);				
	for( int i = 1; i < 8; i++ )
		pSink->AddLine( pt[i] );
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();

	*ret = pGeometry;
	(*ret)->AddRef();
}
void  DrawFill(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc ,ID2D1Brush* br )
{
	cxt->FillRectangle( rc, br );
}
//DLLEXPORT int WINAPI FillRect(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc ,ID2D1Brush* br )
//{
//	cxt->FillRectangle( rc, br );
//	return 0;
//}

DLLEXPORT void WINAPI FillRectangle(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br)
{
	cxt->FillRectangle(rc, br);
}



DLLEXPORT void WINAPI FillRoundRect( ID2D1RenderTarget* cxt_,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br )
{		
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	ComPTR<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->FillGeometry( pr.p, br );
}
DLLEXPORT void WINAPI DrawFillRoundRect( ID2D1RenderTarget* cxt_, ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius,ID2D1Brush* wakuclr,ID2D1Brush* fillclr )
{
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	ComPTR<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->DrawGeometry( pr.p, wakuclr );
	cxt_->FillGeometry( pr.p, fillclr );
}

DLLEXPORT void WINAPI DrawRoundRect( ID2D1RenderTarget* cxt_,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br )
{		
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	ComPTR<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->DrawGeometry( pr.p, br );
}


DLLEXPORT void WINAPI CreateTraiangle( ID2D1Factory* factory, int type, float cx, float cy, ID2D1PathGeometry** ret )
{
	xassert( 0 <=type && type <= 3 );
	
	int cnt;
	FPointF* pt = Triangle( 2, cx,cy, &cnt );

	ComPTR<ID2D1PathGeometry> pathg;
	if ( HR(factory->CreatePathGeometry( &pathg )))
	{
		CreatePathGeometry( pathg, pt, cnt );

		pathg.p->AddRef();
		*ret = pathg;
	}
	delete [] pt;
}

DLLEXPORT void WINAPI DrawResizMark( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc )
{
	//D2DRectFilter f(cxt, rc); 

	D2DMatrix mat(cxt);
	mat.PushTransform();
	mat.Offset( rc.right,rc.bottom);

	ComPTR<ID2D1PathGeometry> pathg;
	ComPTR<ID2D1PathGeometry> pathg2;
		
	{
		ComPTR<ID2D1Factory> factory;
		cxt->GetFactory(&factory);
		
		factory->CreatePathGeometry(&pathg);
		factory->CreatePathGeometry(&pathg2);

		FPointF pt(0, 0); //rc.right, rc.bottom);
		FPointF pt3[3];
		pt3[0] = pt3[1] = pt3[2] = pt;
		pt3[1].y -= 20;
		pt3[2].x -= 20;

		CreatePathGeometry(pathg, pt3, 3);

		pt3[1].y += 10;
		pt3[2].x += 10;

		CreatePathGeometry(pathg2, pt3, 3);
	}

	ComPTR<ID2D1SolidColorBrush> ltgray,white;
	cxt->CreateSolidColorBrush(D2RGB(200,200,200 ), &ltgray );
	cxt->CreateSolidColorBrush(D2RGB(255,255,255 ), &white );
		
	cxt->FillGeometry( pathg, ltgray );
	cxt->FillGeometry(pathg2, white);

	mat.PopTransform();
}
//DLLEXPORT void WINAPI DrawFillRect(ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, D2D1_COLOR_F frame, D2D1_COLOR_F fill)
//{
//	ComPTR<ID2D1SolidColorBrush> bframe, bfill;
//	cxt->CreateSolidColorBrush(frame, &bframe);
//	cxt->CreateSolidColorBrush(fill, &bfill);
//	cxt->FillRectangle(rc, bfill);
//	cxt->DrawRectangle(rc, bframe,1);
//}
DLLEXPORT void WINAPI DrawFillRectEx2( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width, int md )
{
	// たぶん、必要ない TitleBarで使っていた
	
	_ASSERT( width > 0 );

	// Line is FillRectangle.

	FRectF yrc( rc.left + width,rc.top + width,rc.right - width,rc.bottom - width );

	// Draw side
	FRectF trc( rc.left, rc.top, rc.right, yrc.top );

	if ( md & 0x2 )
		cxt_->FillRectangle( trc, wakuclr );					
	trc.top = yrc.bottom;
	trc.bottom = rc.bottom;
	if ( md & 0x4 )
		cxt_->FillRectangle( trc, wakuclr );				
	trc.right = yrc.left;
	trc.top = rc.top;
	if ( md & 0x8 )
		cxt_->FillRectangle( trc, wakuclr );				
	trc.left = yrc.right;
	trc.right = rc.right;
	if ( md & 0x10 )
		cxt_->FillRectangle( trc, wakuclr );				
	
	// Draw fill
	if ( md & 0x1 )
		cxt_->FillRectangle( yrc, fillclr );	
}


DLLEXPORT bool WINAPI ImageLoad( ID2D1DeviceContext* cxt, LPCWSTR filename, ID2D1Bitmap1** ret )
{
	return LoadImage( cxt, filename, ret );

}

DLLEXPORT void WINAPI DrawCircle( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, float sz,ID2D1Brush* br )
{
	D2D1_ELLIPSE el;
	el.radiusX = sz; 
	el.radiusY = sz;
	el.point = center;
	
	cxt->DrawEllipse( &el, br );
}
DLLEXPORT void WINAPI FillCircle( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, float sz,ID2D1Brush* br )
{
	D2D1_ELLIPSE el;
	el.radiusX = sz; 
	el.radiusY = sz;
	el.point = center;
	
	cxt->FillEllipse( &el, br );
}

DLLEXPORT bool WINAPI CreateTextFormatL1(IDWriteFactory* wfac, LPCWSTR fontname, float height, IDWriteTextFormat** ret)
{
	return ( S_OK == wfac->CreateTextFormat(fontname, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, height, L"", ret));
}
DLLEXPORT bool WINAPI CreateTextFormatL2(IDWriteFactory* wfac, LPCWSTR fontname, float height, int bold, IDWriteTextFormat** ret)
{
	return (S_OK == wfac->CreateTextFormat(fontname, 0, (DWRITE_FONT_WEIGHT)bold, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, height, L"", ret));
}

};