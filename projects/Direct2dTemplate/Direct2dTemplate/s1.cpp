#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "MoveTarget.h"
#include "D2DAPPMessage.h"
#include "s1.h"
#define CLASSNAME L"D2DWindow"
using namespace V4;

//void DrawText3( D2DContextText& cxtt, ID2D1SolidColorBrush* br, FRectF rc, FString& nm )
//{
//	rc.left += 2;
//
//	DrawText2(cxtt, br, rc, nm, nm.length());
//}
void DrawText3( D2DContextText& cxtt, ID2D1SolidColorBrush* br, FRectF rc, FString& nm )
{	
	float yoff = (rc.Height()- cxtt.line_height)/2.0f;
	rc.Offset( cxtt.xoff ,yoff);

	//DrawText2(cxtt, br, rc, nm, nm.length());

	cxtt.cxt->cxt->DrawText( nm, nm.length(), cxtt.textformat, rc, br, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_NONE );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void DrawRectLine( D2DContext& cxt, FRectF rc, ID2D1Brush* br )
//{
//	FPointF p1 = rc.LeftTop();
//	FPointF p2 = rc.RightBottom();
//	FPointF p3 = FPointF( p2.x, p1.y );
//	FPointF p4 = FPointF( p2.x, p1.y );
//	
//	
//	DrawLine( cxt.cxt, FPointF(0,0),FPointF(rc1.Width(),0),1,br );
//	DrawLine( cxt.cxt, FPointF(0,0),FPointF(0,height),1,br );
//	DrawLine( cxt.cxt, FPointF(w1,0),FPointF(w1,height),1,br );
//	DrawLine( cxt.cxt, FPointF(rc1.Width(),0),FPointF(rc1.Width(),height),1,br );
//}

D2DControlsWithHScrollbar::D2DControlsWithHScrollbar()
{

}
LRESULT D2DControlsWithHScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;

	D2DContext& cxt = d->cxt_;

	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	
	switch( message )
	{
		case WM_PAINT:
		{			
			FRectF rcborder = rc_.GetBorderRect();
			
			DrawFillRect( cxt, rcborder, cxt.black, cxt.white,1);
			
			FRectF rc = rc_.GetContentRect();
			D2DRectFilter f(cxt, rc );
			
			mat.Offset( rcborder.left, rcborder.top );		
			mat_ = mat; // Ž©À•W(¶ã0,0)‚Ìmatrix
			
			mat.PushTransform();
			{
				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
						
				LRESULT r = 0;
				auto it = controls_.begin();
				
				_ASSERT((*it).get() == scbar_ );			
					
				it++;
				for(; it!= controls_.end(); ++it )
				{				
					(*it)->WndProc(d,message,wParam,lParam);
				}
			}
			mat.PopTransform();
						
			scbar_->WndProc(d,message,wParam,lParam);




		}
		break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_CAPTURECHANGED:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			pt.x += rc_.left;
			pt.y += rc_.top;

			if ( rc_.PtInRect( pt ))
				ret = D2DControls::WndProc(d,message,wParam,lParam);
		

		}
		break;

		default :
			ret = D2DControls::WndProc(d,message,wParam,lParam);
		break;		

	}
	

	mat.PopTransform();

	return ret;
}
void D2DControlsWithHScrollbar::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	FRectFBoxModel rca1(rc);
	
	rca1.BoderWidth_ = 1;
	
	D2DControls::CreateWindow( parent, pacontrol,rca1, stat, name, id );

	FRectFBoxModel xrc = rca1.GetContentRectZero();

	xrc.left = xrc.right - 18;

	scbar_ = new D2DScrollbar();
	scbar_->CreateWindow(parent,this,xrc,VISIBLE,NONAME );
//	scbar_->TotalScrollSize( xrc.Height()*1.3 );
	scbar_->target_control_ = this;



	//D2DPropertyControl* pc = new D2DPropertyControl();
	//FRectF xrc1(0,0,FSizeF(300,1500));
	//pc->CreateWindow(parent, this, xrc1, VISIBLE, L"XXX", 5005);



}

void D2DControlsWithHScrollbar::UpdateScrollbar(D2DScrollbar* bar)
{
	scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	scrollbar_off_.width = 0;

}
