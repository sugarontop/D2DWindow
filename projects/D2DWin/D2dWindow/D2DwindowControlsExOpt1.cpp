/*
The MIT License (MIT)
Copyright (c) 2015 admin@sugarontop.net
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
#include "D2DWindowControl_easy.h"
#include "D2DWindowControlsEx.h"

namespace V4 {

	#pragma region D2DButtonGruop

#define CLICKED 0x80 // 64個まで

void D2DButtonGruop::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id, int btncount )
{
	_ASSERT( btncount < CLICKED );
	
	D2DControl::CreateWindow( parent, pacontrol,rc,stat, name, id );
	btncnt_ = btncount;
	btnmode_ = 0;
}
LRESULT D2DButtonGruop::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);			
			mat.PushTransform();
			FPointF pt = GetPlace_( rc_.Size()); // ボタン位置を取得
			mat.Offset( pt.x, pt.y );

			mat_ = mat; 
			
			DrawButton( d->cxt_ );


			mat.PopTransform();
		}
		break;
		
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				int md = 0;
				int w = rc_.Height();
				FRectF rc(0,0,w, rc_.Height() );
				for( int i = 0; i < btncnt_; i++ )
				{
					if ( rc.PtInRect( pt ) )
					{
						md = (i+1) | CLICKED;

						d->SetCapture(this);
						r = 1;
						break;
					}				
					rc.Offset( w+1, 0);				
				}					


				btnmode_ = md;	

			}
		}
		break;
		case WM_MOUSEMOVE:
		{			
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			int md = 0;

			if (btnmode_ & CLICKED )
			{
				md = btnmode_;
			}
			else if ( rc_.PtInRect( pt ) )
			{
				int w = rc_.Height();
				FRectF rc(0,0,w, rc_.Height() );
				for( int i = 0; i < btncnt_; i++ )
				{
					if ( rc.PtInRect( pt ) )
					{
						md = (i+1);
						break;
					}				
					rc.Offset( w+1, 0);				
				}				
			}
			
			

			if ( md != btnmode_ )
			{
				d->redraw_ = 1;
				btnmode_ = md;				
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( d->GetCapture()==this)
			{				
				r = 1;
				btnmode_ &= ~CLICKED;				
				d->ReleaseCapture();

				if ( rc_.PtInRect(pt))
				{
					int md=0;
					int w = rc_.Height();
					FRectF rc(0,0,w, rc_.Height() );
					for( int i = 0; i < btncnt_; i++ )
					{
						if ( rc.PtInRect( pt ) )
						{
							md = (i+1);
							break;
						}				
						rc.Offset( w+1, 0);				
					}			


					// click event
					if ( md == btnmode_ )
					{
						int btn_idx = md;
						
						parent_control_->WndProc( d, WM_D2D_BUTTON_CLICK, (WPARAM)this, btn_idx );

						TRACE( L"btn=%d\n", btnmode_ );
					}

				}

				btnmode_ = 0;
			}
		}
		break;
	}
	
	return r;
}
void D2DButtonGruop::DrawButton( D2DContext& cxt )
{
	ComPTR<ID2D1SolidColorBrush> br1,br2;
	cxt.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,100), &br1 );
	cxt.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,255), &br2 );

	int w = rc_.Height();
	FRectF rc(0,0,w, rc_.Height() );
	for( int i = 0; i < btncnt_; i++ )
	{		
		if ( (i+1) == (btnmode_ & (CLICKED-1))  )
		{
			if ( btnmode_ & CLICKED )
			{				
				FillRectangle( cxt, rc, cxt.white );
			}
			else
				FillRectangle( cxt, rc, br2 );
		}
		else
			FillRectangle( cxt, rc, br1 );

		rc.Offset( w+1, 0);

	}
}
#pragma endregion


#pragma region D2DTitleBar
// D2DTitleBar::D2DTitleBar(){}

void DrawTitleBar(D2DContext& cxt, const FRectF& titlebar, bool iscapture, bool dlg)
{
	ComPTR<ID2D1LinearGradientBrush> br;

	if (iscapture )
	{
		if (dlg )
			CreateVerticalGradation(cxt.cxt, FSizeF(1, titlebar.Height()), D2RGBA(180, 120, 60, 100), D2RGBA(120, 180, 60, 255), &br);
		else
			CreateVerticalGradation(cxt.cxt, FSizeF(1, titlebar.Height()), D2RGBA(120, 180, 60, 100), D2RGBA(120, 180, 60, 255), &br);
	}
	else
		CreateVerticalGradation( cxt.cxt, FSizeF(1,titlebar.Height()), D2RGBA(200,100,200,0) , D2RGBA(200,100,200,200) , &br );

	FRectF rc = titlebar; //(1,1,titlebar );
	rc.Offset(1,0);

	FillRectangle( cxt, rc, cxt.white );
	FillRectangle( cxt, rc, br  );
	
	//DrawFillRect(cxt, rc, cxt.black, cxt.transparent, 1.0f);

	DrawFillRectEx2( cxt.cxt, rc, cxt.black, cxt.transparent, 1.0f, (0x1|0x2|0x8|0x10) );
		
//	D2DContextText cxtt;
//	cxtt.Init( cxt, 11,DEFAULTFONT_JP ); //DEFAULTFONT_HEIGHT_JP, DEFAULTFONT_JP);
	FString str = L"Diect2D Window";
	rc.left += 5;
	DrawCenterText(cxt, cxt.black, rc, str.c_str(), str.length(), 0 );
}
void DrawButtons(D2DContext& cxt, const FRectF& titlebar)
{
	FRectF b1,b2,b3;
	b1.SetRect( 0,0,25,20 ); 
	b2.SetRect( 0,0,25,20 ); b2.Offset( 26,  0);
	b3.SetRect( 0,0,25,20 ); b3.Offset( 26+26, 0);

	ComPTR<ID2D1SolidColorBrush> br1,br2;
	cxt.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,100), &br1 );
	cxt.cxt->CreateSolidColorBrush( D2RGB(199,80,80), &br2 );

	D2DMatrix mat(cxt);			
	mat.PushTransform();

	mat.Offset( titlebar.right-(26+26+26+6),0 );

	FillRectangle( cxt, b1, br1 );
	//FillRect( cxt, b2, br1 );
	//FillRect( cxt, b3, br2 );

	FRectF b1a( 8,12, 17, 14 );
	FRectF b2a( 8,5, 17, 7 ); b2a.Offset(26,0);
	
	FillRectangle( cxt, b1a, cxt.black );
	FillRectangle( cxt, b2a, cxt.black );
	FRectF b2b( 8,5, 17, 13 ); b2b.Offset(26,0);

	DrawRect( cxt, b2b, cxt.black,1 );

	mat.PopTransform();

}
void D2DTitleBar::OnCreate()
{
	D2DButtonGruop* p = new D2DButtonGruop();
	FRectF rc( 0,0,78,25 );
	p->CreateWindow( parent_, this, rc, 1,L"gpbtn",-1, 3 );
	//p->GetPlace_ = boost::bind( &D2DTitleBar::GetPlaceRightButton, this, _1 );
	p->GetPlace_ = std::bind( &D2DTitleBar::GetPlaceRightButton, this, std::placeholders::_1 );
	btngrp_ = p;
}
LRESULT D2DTitleBar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);
			
			mat.PushTransform();
			mat_ = mat; // 親が原点
			mat.Offset( rc_.left, rc_.top );

			DrawTitleBar( d->cxt_, rc_, parent_->CaptureIsInclude(parent_control_), false );
			
			
			SendMessageReverseAll(d, message, wParam, lParam);

			 mat.PopTransform();
		}
		break;
			
		case WM_D2D_BUTTON_CLICK:
		{
			if ( btngrp_ == (void*)wParam) 
				parent_control_->WndProc( d, WM_D2D_BUTTON_CLICK, (WPARAM)this, lParam );
		}
		break;
		default:
			r = SendMessageAll(d, message, wParam, lParam);
			break;
	}
	
	return r;
}
FPointF D2DTitleBar::GetPlaceRightButton(FSizeF sz )
{
	FPointF pt;

	FRectF rc = rc_.ZeroRect();

	pt.x = rc.right - sz.width-6;
	pt.y = rc.top;

	return pt;
}

#pragma endregion
};

