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
#include "D2DWindowControl_Easy.h"
#include "D2DWindowTagTicket.h"
#include "movetarget.h"
using namespace V4;

#define YOFF 26

bool DPtoLP( D2DMat& mat, LPARAM lp, const FRectF& rc, FPointF& ret )
{
	auto pt = mat.DPtoLP(lp, rc);
	
	if ( 0 <= pt.x && 0 <= pt.y && pt.x <= rc.right-rc.left  && pt.y <= rc.bottom-rc.top)
	{
		ret = pt;
		return true;
	}
	return false;
}

void D2DTagTicket::OnCreate()
{
	fore_color_.color = ColorF(ColorF::White);
	back_color_.color = D2RGB(240,240,240);
	OnResutructRnderTarget(true);


	FRectF rc = rc_.ZeroRect();
	rc.bottom -= YOFF;

	D2DFRectFBM* rc1 = new D2DFRectFBM();
	rc1->CreateWindow( parent_, this, rc,VISIBLE,NONAME,0);
	rc1->BackColor( D2RGBA(63,72,204,255));


	rc.Offset( rc.Width(), 0 );

	D2DFRectFBM* rc2 = new D2DFRectFBM();
	rc2->CreateWindow( parent_, this, rc,VISIBLE,NONAME,0);
	rc2->BackColor( D2RGBA(0,128,64,150));

	xoffw_ = 0;
	

	innerbtn1.SetRect(0,0,40,YOFF);
	innerbtn2.SetRect(40,0,rc_.right,YOFF);
	
}
void D2DTagTicket::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow(parent,pacontrol,rc,stat,name,id);
	//typ_ = typ;
	auto_resize_ = !( stat_&STAT::WIDTH_FIX && stat_&STAT::HEIGHT_FIX );
}


LRESULT D2DTagTicket::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return ret;

	switch (message)
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			
			
			D2DMatrix mat(cxt);
			mat.PushTransform();
			mat_ = mat;
			D2DRectFilter fil( cxt, rc_);
			mat.Offset( rc_ );

			

			DrawFill(cxt, rc_.GetContentRectZero(), (ID2D1Brush*) back_color_.br);
			
			DrawTitleButton(cxt);


			mat.PushTransform();
			DrawSlide(mat,cxt);
			mat.PopTransform();



			mat.PopTransform();
		}
		break;
		case WM_SIZE:
		{
			if ( auto_resize_ )
			{
				auto p = dynamic_cast<IUpdatar*>( parent_control_ );
				if ( p )
					p->RequestUpdate(this, WM_SIZE );					
			}
		}
		break;		
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		{
			FPointF pt; // = mat_.DPtoLP(lParam, rc_);

			if ( DPtoLP( mat_,lParam,rc_,pt))
			{

				if ( innerbtn2.PtInRect(pt))
					ret = MouseSelectedMoveProc( this,message,wParam,lParam);
			
				if ( ret == 0 && message == WM_LBUTTONDOWN && innerbtn1.PtInRect(pt) )
				{
					SlideView();
					ret = 1;
				}
			}
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			//if ( typ_ > 0 )
				ret = MousePropertyMenuProc(this,message,wParam,lParam);
		break;
	}
	return ret;
}



void D2DTagTicket::DrawTitleButton(D2DContext& cxt)
{
	FRectF rc = innerbtn1;

	cxt.cxt->DrawText( L"<-->", 4, cxt.cxtt.textformat, rc.Offset(10,5), cxt.black );

}

void D2DTagTicket::DrawSlide(D2DMatrix& mat, D2DContext& cxt)
{
	mat.Offset( -xoffw_, YOFF );
	SendMessageAll(parent_,WM_PAINT,0,0);	
}
void D2DTagTicket::SlideView()
{
	float prv_xoff = xoffw_;
	float xo;

	if ( xoffw_ == 0 )
	{
		xo = prv_xoff + rc_.Width();
		md_ = 0;
	}
	else 
	{
		xo = prv_xoff - rc_.Width();
		md_ = 1;
	}
	

	MoveTarget* m = new MoveTarget();
		
	m->SetParameter(prv_xoff, 0, xo, 0, 300);
		
	m->Fire_ = [this](MoveTarget* p, float offa, float offb)->bool
	{
		xoffw_ = offa;
		parent_->redraw_ = 1;
		return true;
	};

	m->End_ = [this](MoveTarget* p, float offa, float offb)
	{
		
		xoffw_ = ( md_ == 0 ? rc_.Width() : 0 );
		
		parent_->redraw_ = 1;

		delete p;
		
	};
	
	m->Start( parent_ );
	
}


void D2DTagTicket::BackColor( D2D1_COLOR_F clr)
{
	back_color_.color = clr;
	back_color_.br = parent_->GetSolidColor(back_color_.color);

}
void D2DTagTicket::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);

	ParameterColor(parent_, fore_color_,prms, L"forecolor");
	ParameterColor(parent_, back_color_, prms, L"backcolor");
}
void D2DTagTicket::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		fore_color_.br = parent_->GetSolidColor(fore_color_.color);
		back_color_.br = parent_->GetSolidColor(back_color_.color);
	}
	else
	{
		fore_color_.br = nullptr;
		back_color_.br = nullptr;
	}
}
