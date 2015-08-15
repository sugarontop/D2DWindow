#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "D2DContextNew.h"



using namespace V4; 

namespace TSF {
CTextEditorCtrl* GetTextEditorCtrl();
};


LRESULT D2DFrameWindowControl::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;
			//D2DRectFilter f(cxt, rcx );

			D2DMatrix mat(cxt);
			mat.PushTransform();
			FRectF rc = rc_.GetBorderRect();
			mat_ = mat;
			mat.Offset( rc.left, rc.top );
			
			{
				rc = rc_.GetBorderRectZero();

				

				cxt.cxt->DrawRectangle( rc, cxt.black,1 );
				cxt.cxt->FillRectangle( rc, cxt.white );

				FRectF rctitle = rc;
				rctitle.SetHeight( 30.0f );
				cxt.cxt->FillRectangle( rctitle, cxt.red );

				FString str = L"D2DFrameWindowControl";
				DrawCenterText( cxt.cxtt, cxt.black, rc, str.c_str(), str.length(), 1 );

				
				SendMessageReverseAll( d,message,wParam,lParam);
			}
			mat.PopTransform();
			return 0;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(FPointF(lParam));
			FRectF rctitle = rc_;
			rctitle.SetHeight( 30.0f );

			if ( rctitle.PtInRect(pt))
			{
				MeToLast(); // order‚ð•ÏX

				
				d->redraw_ =1 ;
				FRectF rctitle = rc_.GetBorderRect();
				rctitle.SetHeight( 30.0f );
				if ( rctitle.PtInRect(pt))
				{
					d->SetCapture(this, &pt);	
					ret = 1;				
				}
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( d->GetCapture() == this )
			{
				FPointF pt = mat_.DPtoLP(FPointF(lParam));
				FPointF ptprv = d->CapturePoint(pt);
				rc_.Offset( pt.x- ptprv.x, pt.y-ptprv.y );
				d->redraw_ =1 ;

				ret = 1;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				ret = 1;
			}
		}
		break;
	}

	if ( ret == 0 )
		ret = D2DControls::WndProc( d,message,wParam,lParam);

	return ret;
}


void D2DFrameWindowControl::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{	
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id );	
}

void D2DFrameWindowControl::OnCreate()
{
	FRectF rctxt( 10, 50,FSizeF(200,26));
	D2DTextbox* tx = new D2DTextbox(TSF::GetTextEditorCtrl());
	tx->CreateWindow( parent_, this, rctxt, VISIBLE|BORDER, L"NONAME" );
	tx->SetText( L"hello world" );
}