// D2DApi.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "D2DContextEx.h"
#include "D2DApi.h"
#include "D2DMisc.h"
#include "D2DResource.h"
#include "D2DContextNew.h"
#include "FString.h"

#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")

namespace V4{ 

struct RenderFilter
{
	RenderFilter(ID2D1RenderTarget* cxt1, const D2D1_RECT_F& rc ):cxt(cxt1)
	{
		cxt->PushAxisAlignedClip( rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );		
	}	

	~RenderFilter()
	{
		cxt->PopAxisAlignedClip();
	}
	private :
		ID2D1RenderTarget* cxt;	
};
DLLEXPORT void WINAPI DrawText(ID2D1RenderTarget* cxt,IDWriteTextFormat* tf, LPCWSTR str, ID2D1Brush* br, const D2D1_RECT_F& rc )
{
	cxt->DrawText( str, lstrlen(str), tf, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
}

// d2dcontext.cppのDRAWFillRectを名前が重なるので大文字にした
DLLEXPORT void WINAPI DRAWFillRect( ID2D1RenderTarget* cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
{
	RenderFilter f(cxt,rc);
	cxt->FillRectangle( rc, fillclr );	
	cxt->DrawRectangle( rc, wakuclr, width );				
	
}

DLLEXPORT bool WINAPI CreateTextFormat(LPCWSTR fontnm, float height, IDWriteTextFormat** ret )
{
	IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;
	return ( S_OK == fc->CreateTextFormat( fontnm,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"", ret ));	
}

DLLEXPORT void WINAPI DrawFillRectEx2( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width, int md )
{
	
	
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



DLLEXPORT void WINAPI DrawRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width )
{		
	RenderFilter f(cxt,rc);
	cxt->DrawRectangle( rc, br, width );			
}


DLLEXPORT void WINAPI DrawDotRect( ID2D1RenderTarget* cxt,const D2D1_RECT_F& rc, ID2D1Brush* br )
{
	RenderFilter f(cxt,rc);
	auto white = D2DResource::s_Active->white_;
	auto pen = D2DResource::s_Active->pen_[2];
	cxt->DrawRectangle( rc, br, 1.0f, pen );		
}
DLLEXPORT void WINAPI DrawSolidRect( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* br )
{		
	cxt_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	cxt_->DrawRectangle( rc, br, 1.0f );		
	cxt_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			
}

DLLEXPORT void WINAPI DrawFill( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* br )
{
	cxt_->FillRectangle( rc, br );
}

DLLEXPORT void WINAPI DrawFillRoundRect( ID2D1RenderTarget* cxt_, ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius,ID2D1Brush* wakuclr,ID2D1Brush* fillclr )
{
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	CComPtr<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->DrawGeometry( pr.p, wakuclr );
	cxt_->FillGeometry( pr.p, fillclr );
}

DLLEXPORT void WINAPI DrawRoundRect( ID2D1RenderTarget* cxt_,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br )
{		
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	CComPtr<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->DrawGeometry( pr.p, br );
}

DLLEXPORT void WINAPI FillRoundRect( ID2D1RenderTarget* cxt_,ID2D1Factory* factory_, const D2D1_RECT_F& rc, float radius, ID2D1Brush* br )
{		
	D2D1_ROUNDED_RECT rectangle = D2D1::RoundedRect( D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom), radius, radius);
	CComPtr<ID2D1RoundedRectangleGeometry> pr;
	factory_->CreateRoundedRectangleGeometry( rectangle, &pr );			
	cxt_->FillGeometry( pr.p, br );
}


DLLEXPORT void WINAPI CreateVerticalGradation(ID2D1RenderTarget* target, const D2D1_SIZE_F& sz, D2D1_COLOR_F& clr1, D2D1_COLOR_F& clr2, ID2D1LinearGradientBrush** br )
 {
	D2D1_GRADIENT_STOP gradientStops[] = {{ 0.0f, clr1},{ 1.0f, clr2 }};

	CComPtr<ID2D1GradientStopCollection> gradientStopsCollection;
	target->CreateGradientStopCollection(gradientStops,_countof(gradientStops),&gradientStopsCollection);
	
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES properties = D2D1::LinearGradientBrushProperties(FPointF(0,0), FPointF( 0, sz.height));
	target->CreateLinearGradientBrush(properties, gradientStopsCollection.p, br);
 }
 DLLEXPORT void WINAPI CreateSolidBrush( ID2D1RenderTarget* cxt, D2D1_COLOR_F& clr, ID2D1SolidColorBrush** br )
 {
	cxt->CreateSolidColorBrush( clr, br );

 }



DLLEXPORT void WINAPI DrawButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, LPCWSTR str, ID2D1Brush* br1 )
{	
	ID2D1LinearGradientBrush* br = D2DResource::s_Active->Silver1_;

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
	
	DrawCenterText( cxt, br1, rc, str, lstrlen(str), 1 );
}




DLLEXPORT void WINAPI DrawWaiterCircle( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rcb, ID2D1Brush* br, int OneTo12, float size )
{
	FPointF center_pt = FPointF( (rcb.left+rcb.right)/2.0f, (rcb.top+rcb.bottom)/2.0f );
	double hankei = (rcb.right-rcb.left )/ 2.0f;
	int pos = OneTo12 - 1;

	_ASSERT ( 0 <=pos && pos < 12 );

	D2D1_ELLIPSE el;
	el.radiusX = size;
	el.radiusY = size;

	for( int i = 0; i < 12; i++ )
	{		
		el.point.y = (FLOAT)(hankei*sin( M_PI/6*i ) + center_pt.y);
		el.point.x = (FLOAT)(hankei*cos( M_PI/6*i ) + center_pt.x);

		if ( i == pos)
			cxt->FillEllipse( &el, br );
		else
			cxt->DrawEllipse( &el, br );
	}
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
	CComPtr<ID2D1GeometrySink> sink;	
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


DLLEXPORT void WINAPI CreateRetenGeometry(ID2D1PathGeometry* pathg)
{
	// 13x13のレ点
	FPointF pt[6];
	pt[0] = FPointF(2,6);
	pt[1] = FPointF(2,8);
	pt[2] = FPointF(6,11);
	pt[3] = FPointF(11,4);
	pt[4] = FPointF(11,1);
	pt[5] = FPointF(6,8);

	CreatePathGeometry( pathg, pt, 6 );
}
DLLEXPORT void WINAPI CreateHexagonGeometry(ID2D1PathGeometry* pathg, float cx, float cy )
{	
	float diff = 0;
	FPointF pt[7];

	pt[0] = FPointF(0,cy/2);
	pt[1] = FPointF(diff,0);
	pt[2] = FPointF(cx+diff,0);
	pt[3] = FPointF(cx+diff+diff,cy/2);
	pt[4] = FPointF(cx+diff,cy);
	pt[5] = FPointF(diff,cy);
	pt[6] = FPointF(0,cy/2);

	
	CreatePathGeometry( pathg, pt, 7 );
}
DLLEXPORT void WINAPI CreateRTRoundGeometry(ID2D1PathGeometry* pathg, float cx, float cy, float roundsize )
{
/*
usage.

	CComPtr<ID2D1Factory> factory;
	CComPtr<ID2D1PathGeometry> pPathGeometry;
	cxt->GetFactory( &factory );
	factory->CreatePathGeometry(&pPathGeometry);

	CreateRTRoundGeometry(pPathGeometry, 50,100,10 );
	cxt->DrawGeometry( pPathGeometry, d->cxt_.black );
*/
	CComPtr<ID2D1GeometrySink> pSink;
	pathg->Open(&pSink);

	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
    pSink->BeginFigure(
        D2D1::Point2F(0, 0),
        D2D1_FIGURE_BEGIN_FILLED
        );
	pSink->AddLine(FPointF(cx,0));
    pSink->AddArc(
        D2D1::ArcSegment(
            D2D1::Point2F(cx+roundsize, roundsize), // end point
            D2D1::SizeF(roundsize, roundsize),
            0, // rotation angle
            D2D1_SWEEP_DIRECTION_CLOCKWISE,
            D2D1_ARC_SIZE_SMALL
            ));            
	pSink->AddLine(FPointF(cx+roundsize,cy));
	pSink->AddLine(FPointF(0,cy));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();
}



DLLEXPORT void WINAPI CreateTraiangle( ID2D1Factory* factory, int type, float cx, float cy, ID2D1PathGeometry** ret )
{
	xassert( 0 <=type && type <= 3 );
	
	int cnt;
	FPointF* pt = Triangle( 2, cx,cy, &cnt );

	CComPtr<ID2D1PathGeometry> pathg;
	if ( HR(factory->CreatePathGeometry( &pathg )))
	{
		CreatePathGeometry( pathg, pt, cnt );

		pathg.p->AddRef();
		*ret = pathg;
	}
	delete [] pt;
}

DLLEXPORT void WINAPI FillArrow( ID2D1RenderTarget* cxt, const D2D1_POINT_2F& center, ID2D1Brush* br, int typ )
{
	CComPtr<ID2D1Factory> factory;
	cxt->GetFactory(&factory);

	CComPtr<ID2D1PathGeometry> pathg;
	factory->CreatePathGeometry( &pathg );

	int cnt;
	FPointF* pt = Triangle( typ, 10, 5 , &cnt );
	CreatePathGeometry( pathg, pt, cnt );
	cxt->FillGeometry( pathg, br );

	delete [] pt;
}   

DLLEXPORT void WINAPI FillPolygon( ID2D1RenderTarget* cxt, const D2D1_POINT_2F* pt, int pt_count, ID2D1Brush* br )
{
	CComPtr<ID2D1Factory> factory; 
	cxt->GetFactory(&factory);

	CComPtr<ID2D1PathGeometry> pathg;
	factory->CreatePathGeometry( &pathg );
	
	CreatePathGeometry( pathg, pt, pt_count );
	cxt->FillGeometry( pathg, br );

	
} 

DLLEXPORT void WINAPI CreateTagButtomGeometry(ID2D1RenderTarget* cxt, const FRectF& rcButton, const FRectF& rc, ID2D1PathGeometry** ret )
{
	xassert( rcButton.top == rc.top );

	// rcは全体
	// rcButtonは飛び出たボタン部分
	/*
	    ____
	  __|   |___
	 |          | 
	 |__________| <-- CreateGeomery
	 
	*/

	CComPtr<ID2D1PathGeometry> pGeometry;
	CComPtr<ID2D1Factory> factory;
	cxt->GetFactory(&factory);		
	factory->CreatePathGeometry(&pGeometry);

	CComPtr<ID2D1GeometrySink> pSink;
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
DLLEXPORT void WINAPI DrawScrollbar( ID2D1RenderTarget* cxt, D2DScrollbarInfo& info )
{
	CComPtr<ID2D1SolidColorBrush> bkcolor,br1b,br2;
	cxt->CreateSolidColorBrush(D2RGBA(230,230,230,255 ), &bkcolor );
	cxt->CreateSolidColorBrush(D2RGBA(200,200,200,255 ), &br1b );
	cxt->CreateSolidColorBrush(D2RGBA(100,100,100,255 ), &br2 );
	
	cxt->FillRectangle( info.rc, bkcolor ); // 全体

	
	if ( info.bVertical )
	{
		// Upper button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.top+5 ) );				
		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 0 );		
		

		cxt->SetTransform( mat.CalcOffset( info.rc.left+3, info.rc.bottom-10 ) );				

		// Lower button
		rc = ScrollbarRect( info, 2 );		
		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 2 );		

		cxt->SetTransform( &mat ); // 戻す



		// Thumb button
		rc = ScrollbarRect( info, 3 );
		//rc.InflateRect( -4 ,0);
		rc.right -= 1;
		

		info.thumb_rc = rc;

		bool bl = (bool)((info.stat & D2DScrollbarInfo::ONTHUMB) || (info.stat & D2DScrollbarInfo::CAPTURED));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	else
	{
		// left side button
		FRectF rc = ScrollbarRect( info, 1 );
				
		D2DMat mat;
		cxt->GetTransform( &mat );				

		cxt->SetTransform( mat.CalcOffset( info.rc.left+5, info.rc.top+5 ) );				
		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN1 ? br2 : br1b ), 3 );		
		
		cxt->SetTransform( mat.CalcOffset( info.rc.right-10, info.rc.top+5 ) );				

		// right side button
		rc = ScrollbarRect( info, 2 );		
		FillArrow( cxt, rc.CenterPt(), ( info.stat & D2DScrollbarInfo::ONBTN2 ? br2 : br1b ), 1 );		

		cxt->SetTransform( &mat ); // 戻す

		// Thumb button
		rc = ScrollbarRect( info, 3 );
		
		rc.InflateRect( 0,-4);
		

		info.thumb_rc = rc;

		bool bl = (bool)(((info.stat & D2DScrollbarInfo::ONTHUMB) > 0) | ((info.stat & D2DScrollbarInfo::CAPTURED) >0));

		cxt->FillRectangle( rc, ( bl ? br2 : br1b ) ); // thumb

	}
	
}

DLLEXPORT D2D1_POINT_2F WINAPI DrawLine(ID2D1RenderTarget* cxt,const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2, float width,ID2D1Brush* br, ID2D1StrokeStyle* st )
{
	cxt->DrawLine( pt1, pt2, br, width, st );
	return pt2;
}


DLLEXPORT void WINAPI DrawDebugCross(ID2D1RenderTarget* cxt,ID2D1Brush* br )
{
	CComPtr<ID2D1Factory> factory;

	cxt->GetFactory(&factory);

	CComPtr<ID2D1StrokeStyle> strokeStyle1;
	D2D1_STROKE_STYLE_PROPERTIES prop = D2D1::StrokeStyleProperties();
		
	
	{
		prop.dashStyle = D2D1_DASH_STYLE_SOLID;
		float dashes[] ={5, 5, 5};
		factory->CreateStrokeStyle(prop,	dashes,	_countof(dashes), &strokeStyle1);	

	}

	{
		DrawLine( cxt, FPointF(-10,0), FPointF(10,0), 5, br, strokeStyle1);
		DrawLine( cxt, FPointF(0,-10), FPointF(0,10), 5, br, strokeStyle1);
	}
	
}
DLLEXPORT void WINAPI DrawDebugCross2(ID2D1RenderTarget* cxt,ID2D1Brush* br, FPointF pt )
{
	CComPtr<ID2D1Factory> factory;

	cxt->GetFactory(&factory);

	CComPtr<ID2D1StrokeStyle> strokeStyle1;
	D2D1_STROKE_STYLE_PROPERTIES prop = D2D1::StrokeStyleProperties();
		
	
	{
		prop.dashStyle = D2D1_DASH_STYLE_SOLID;
		float dashes[] ={5, 5, 5};
		factory->CreateStrokeStyle(prop,	dashes,	_countof(dashes), &strokeStyle1);	

	}

	{
		DrawLine( cxt, FPointF(-10+pt.x,0+pt.y), FPointF(10+pt.x,0+pt.y), 5, br, strokeStyle1);
		DrawLine( cxt, FPointF(0+pt.x,-10+pt.y), FPointF(0+pt.x,10+pt.y), 5, br, strokeStyle1);
	}
	
}



///----------------


DLLEXPORT D2D1_SIZE_F WINAPI CalcText( IDWriteTextFormat* fmt, LPCWSTR str, int len )
 {
	IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;

	DWRITE_TEXT_METRICS tm;

	CComPtr<IDWriteTextLayout> textlayout;
	auto hr = fc->CreateTextLayout(str,len,fmt,(FLOAT)32000, (FLOAT)32000, &textlayout ); 
	xassert(hr == 0);
    textlayout->GetMetrics(&tm);
	
	return FSizeF( tm.width, tm.height );

 }

DLLEXPORT D2D1_POINT_2F WINAPI DrawCenterText2( const D2D1_RECT_F& rc, LPCWSTR str, int len, int align,IDWriteTextLayout** ret )
{	
	// cxt.cxt->DrawTextLayout( pt, textlayout, cxt.black, D2D1_DRAW_TEXT_OPTIONS_CLIP );

	return DrawCenterTextEx2(D2DResource::s_Active->TextFormat_, rc,str,len,align, ret );
}
DLLEXPORT D2D1_POINT_2F WINAPI DrawCenterTextEx2( IDWriteTextFormat* fmt, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align, IDWriteTextLayout** ret )
{	
	IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;
	
	DWRITE_TEXT_METRICS tm;
	FSizeF sz( rc.right-rc.left, rc.bottom-rc.top );
	CComPtr<IDWriteTextLayout> textlayout;

	fc->CreateTextLayout(str,len,fmt, 32000, sz.height, &textlayout );  // SINGLE LINE ONLY
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
		

	*ret = textlayout;
	(*ret)->AddRef();
	
	return rcX.LeftTop();
}

DLLEXPORT float WINAPI DrawCenterText( ID2D1RenderTarget* cxt, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align )
{	
	return DrawCenterTextEx(cxt,D2DResource::s_Active->TextFormat_, clr,rc,str,len,align );
}

DLLEXPORT float WINAPI DrawCenterTextEx( ID2D1RenderTarget* cxt,IDWriteTextFormat* fmt, ID2D1Brush* clr, const D2D1_RECT_F& rc, LPCWSTR str, int len, int align )
{	
	IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;
	
	DWRITE_TEXT_METRICS tm;
	FSizeF sz( rc.right-rc.left, rc.bottom-rc.top );
	CComPtr<IDWriteTextLayout> textlayout;

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



DLLEXPORT void WINAPI DrawCheckbox( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, bool bCheck, bool bEnable, LPCWSTR str )
{
	auto black = D2DResource::s_Active->black_;
	auto white = D2DResource::s_Active->white_;

	FRectF rcc = rc; //.GetContentRect();
	float size = 13; //(rc.bottom-rc.top)/2;
	

	FRectF rcd = rcc;
	rcd.right = rcd.left + size;
	rcd.bottom = rcd.top + size;

	
	CComPtr<ID2D1SolidColorBrush> br1,br2,br3;
	cxt->CreateSolidColorBrush( D2RGB(70,70,70), &br1 );
	cxt->CreateSolidColorBrush( D2RGB(200,200,200), &br2 );
	cxt->CreateSolidColorBrush( D2RGB(230,230,230), &br3 );
	
	
	cxt->FillRectangle( rcd, br1 );

	rcd.left+=1.0f;rcd.top+=1.0f;
	cxt->FillRectangle( rcd, br2 );

	rcd.left+=1.0f;rcd.top+=1.0f;
	rcd.right-=1.0f;rcd.bottom-=1.0f;
	cxt->FillRectangle( rcd, br3 );

	
	if ( bCheck )
	{
		rcd.left+=2.0f;rcd.top+=2.0f;
		rcd.right-=2.0f;rcd.bottom-=2.0f;
		cxt->FillRectangle( rcd, black );// まだみかん	
	}	
	

	rcc.left += size+3;
	rcc.bottom = rcc.top + size;

	DrawCenterText( cxt, black, rcc, str, lstrlen(str), 0 );



}
DLLEXPORT void WINAPI DrawRadioButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, bool bCheck, bool bEnable, LPCWSTR str )
{
	// まだみかん
	
	
	auto black = D2DResource::s_Active->black_;
	auto white = D2DResource::s_Active->white_;

	FRectF rcc = rc; //.GetContentRect();
	float size = 13; //(rc.bottom-rc.top)/2;
	float xy = (rc.top+rc.bottom)/2;

	D2D1_ELLIPSE el;
	el.radiusX = size;
	el.radiusY = size;
	el.point = FPointF( rc.left+size, rc.top+size );
	cxt->DrawEllipse( el, black );
	cxt->FillEllipse( el, white );
}

/*
DLLEXPORT void WINAPI DrawImage( ID2D1RenderTarget* cxt, D2DImage image_idx, const D2D1_RECT_F& rc )
{
	_ASSERT( image_idx.id <= (int)D2DResource::s_Active->images_.size() );

	auto bmp = D2DResource::s_Active->images_[image_idx.id-1];
	_ASSERT( bmp != nullptr ); // 動作がとまるので
	
	cxt->DrawBitmap( bmp, rc );
}
DLLEXPORT bool WINAPI RegistorImageFile( ID2D1RenderTarget* cxt, LPCWSTR filename, D2DImage& img )
{
	
	ID2D1Bitmap* bmp;
	if ( D2DResource::LoadImage( cxt, filename, &bmp ) )
	{
		RegistorImage( bmp, img );

		img.prop.size = bmp->GetPixelSize();
		return true;
	}

	return false; // error
}
DLLEXPORT bool WINAPI RegistorResourceImage( ID2D1RenderTarget* cxt, LPCWSTR resource_secnm, int resource_id, D2DImage& img )
{

	/////////////////////////////////////////////////////////////////////////////
	//
	// PNG
	//

	IDB_PNG2                PNG                     "icons\\icon_briefcase.png"
	IDB_PNG3                PNG                     "icons\\icon_building.png"
	IDB_PNG4                PNG                     "icons\\icon_calendar.png"


	
	
	ID2D1Bitmap* bmp;
	if ( D2DResource::LoadImage( cxt, resource_secnm,resource_id, &bmp ) )
	{
		RegistorImage( bmp, img );

		img.prop.size = bmp->GetPixelSize();
		img.prop.resource_id = resource_id;
		
		return true;
	}

	return false; // error


}



DLLEXPORT void WINAPI RegistorImage( ID2D1Bitmap* bmp, D2DImage& img )
{
	D2DResource::s_Active->images_.push_back( bmp );
	img.id = D2DResource::s_Active->images_.size(); // 1 start;
}
DLLEXPORT void WINAPI ClearImage( D2DImage img )
{
	auto p = D2DResource::s_Active->images_[ img.id-1 ];
	if ( p )
		p->Release();

	D2DResource::s_Active->images_[ img.id-1 ] = NULL;
}
*/





DLLEXPORT bool WINAPI LoadResourceImage( ID2D1RenderTarget* cxt, LPCWSTR resource_section, int resource_id, ID2D1Bitmap** bmp )
{
	return D2DResource::LoadImage( cxt, resource_section,resource_id, bmp );
}

DLLEXPORT bool WINAPI LoadImageFromFile( ID2D1RenderTarget* target, LPCWSTR filenm, ID2D1Bitmap** ret )
{
	return D2DResource::LoadImage( target, filenm, ret );
}
DLLEXPORT bool WINAPI LoadImageFromIStream( ID2D1RenderTarget* target, IStream* sm, ID2D1Bitmap** ret )
{
	return D2DResource::LoadImage( target, sm, ret );
}

DLLEXPORT void WINAPI DrawLineRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width )
{
	FPointF pt1(rc.left, rc.top);
	FPointF pt2(rc.right, rc.top);
	FPointF pt3(rc.right, rc.bottom);
	FPointF pt4(rc.left, rc.bottom);
	
	
	cxt->DrawLine( pt1, pt2, br, width );
	cxt->DrawLine( pt3, pt2, br, width );
	cxt->DrawLine( pt3, pt4, br, width );
	cxt->DrawLine( pt1, pt4, br, width );
}

DLLEXPORT void WINAPI DrawCheckMark(ID2D1RenderTarget* cxt, ID2D1Brush* br )    
{
	CComPtr<ID2D1Factory> factory;
	CComPtr<ID2D1PathGeometry> pathg;
	cxt->GetFactory( &factory );
	factory->CreatePathGeometry(&pathg);


	CreateRetenGeometry( pathg ); // 13x13のレ点
	cxt->FillGeometry( pathg, br ); 




}
DLLEXPORT float WINAPI DrawFullText( ID2D1RenderTarget* cxt, ID2D1Brush* clr, const D2D1_SIZE_F& sz, LPCWSTR str, int len, int align )
{	
	IDWriteTextFormat* fmt = D2DResource::s_Active->TextFormat_;
	IDWriteFactory* fc = D2DResource::s_Active->textwritefactory_;
		
	FRectF rc(0,0,sz);
	
	cxt->DrawText( str, len, fmt, rc, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
	return 0;
}

};


int HexNumer( LPCWSTR hexstr )
{
	int ret = 0;
	
	// '0xFF' --> 255
	if ( hexstr[0] == '0' && (hexstr[1] == 'X' || hexstr[1] == 'x' ))
	{
		int x = 16;
		for( int i = 2; i < 4; i++ )
		{
			WCHAR w = hexstr[i];
			if ( '0' <= w && w <= '9' )
			{
				ret += x * (w - '0' );
			}
			else if ( 'a' <= w && w <= 'z' )
			{
				ret += x * (10+(w - 'a' ));
			}
			else if ( 'A' <= w && w <= 'Z' )
			{
				ret += x * (10+(w - 'A' ));
			}
			x= 1;
		}
	}
	return ret;
}

DLLEXPORT D2D1_COLOR_F WINAPI ColorParse( LPCWSTR color )
{
	D2D1_COLOR_F ret;

	V4::FString clr = color;
	clr.ToLower();
	

	WCHAR* p = (WCHAR*)(LPCWSTR)clr;

	WCHAR buf[10];

	int v[4];
		
	v[0] = 255;
	v[1] = 255;
	v[2] = 255;
	v[3] = 255;

	int i = 0,j=0;
	bool bc = false;
	if ( *p == L'#' )
	{
		// "#FF00FF00"
		p++;
		WCHAR* p1 = buf+2;
		bc = true;
		
		
		buf[0] = L'0';
		buf[1] = L'x';				
		buf[4]=0;
		while( *p && j < 4  )
		{
			if ( i == 0 )
				buf[2] = *p++;
			else if ( i == 1 )
			{
				buf[3] = *p++;

				v[j++] = HexNumer( buf );
				i = -1;				
			}
			i++;
		}
		
	}
	else if ( *p == L'(' )
	{
		// "(255,0,255,0)"
		p++;
		WCHAR* p1 = buf;
		bc = true;

		while( *p && j < 4 )
		{
			if ( '0' <= *p && *p <= '9' )
			{
				*p1 = *p;
				p1++;
			}
			else if ( *p == ',' )
			{
				*p1 = 0;

				v[j++] = _wtoi( buf );
				p1 = buf;
			}
			else if ( *p == ')' || *p == 0 )
			{
				*p1 = 0;
				v[j++] = _wtoi( buf );
				p1 = buf;
			}

			p++;			
		}
	}
	else if ( L'a' <= *p && *p <= L'z' )
	{
		switch( *p )
		{
			case 'w' :
				if ( clr == L"white" )
					return ColorF( ColorF::White );
			break;
			case 'b' :
				if ( clr == L"black" )
					return ColorF( ColorF::Black );
				if ( clr == L"blue" )
					return ColorF( ColorF::Blue );
			break;
			case 'g' :
				if ( clr == L"gray" )
					return ColorF( ColorF::Gray );
				else if ( clr == L"green" )
					return ColorF( ColorF::Green );
			break;
			case 'r' :
				if ( clr == L"red" )
					return ColorF( ColorF::Red );
			break;
			case 'l' :
				if ( clr == L"ltgray" )
					return ColorF( ColorF::LightGray );
			break;
		}
	}


	if ( bc )
	{
		ret.r = (float)v[0] / 255.0f;
		ret.g = (float)v[1] / 255.0f;
		ret.b = (float)v[2] / 255.0f;
		ret.a = (float)v[3] / 255.0f;
	}

	return ret;

}

