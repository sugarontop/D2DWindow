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
#include "D2DWindowControl_easy.h"
#include "D2DWindowControlsEx.h"
#include "MoveTarget.h"

#define TITLE_BAR_HEIGHT 30

namespace V4 {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DControlsEx::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 )
{
	FRectFBoxModel nrc = rc;
	if ( nrc.Margin_.l == 0 )
		nrc.Margin_.Set(1);

	nrc.BoderWidth_ = 1;
	
	D2DControls::CreateWindow(parent,pacontrol,nrc,stat,name,id);

	FSizeF titlebar( nrc.GetBorderRect().Size().width, TITLE_BAR_HEIGHT );
	bDialog_ = false;

	wndstat_ = 0;
	tbar_ = new D2DTitleBar();
	FRectFBoxModel rc1( 0,0, titlebar );
	tbar_->CreateWindow( parent, this, rc1, 1, L"title", -1 );
	titlebar_height_ = titlebar.height;
}
D2DControlsEx::~D2DControlsEx()
{
	int a = 0;
}
void D2DControlsEx::MeToLast()
{
	std::vector<std::shared_ptr<D2DControl>>& ls = this->parent_control_->controls_;
	
	std::shared_ptr<D2DControl> me;

	for( auto it=ls.begin(); it !=ls.end(); it++ )
	{
		if ( this == (*it).get() )
		{
			me = *it;
			ls.erase( it );	 // remove
			break;
		}		
	}
	
	ls.push_back( me ); // add to last.

}
LRESULT D2DControlsEx::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( wndstat_ == 0 )
		return WndProc0( d, message, wParam, lParam );
	else
		return WndProc1( d, message, wParam, lParam );
}
//D2DControls* test;


LRESULT D2DControlsEx::WndProc1(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;
	
	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	if ( message == WM_PAINT )
	{		
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
		mat.Offset( rc_.left, rc_.top );
		
		{
			mat._11 = mat2_._11;
			mat._22 = mat2_._22;

			mat.SetTransform();
		}

		if ( md_ != 2 )
			mat_ = mat; // 自座標(左上0,0)のmatrix
	}
//dp_rc_ = mat.LPtoDP(rc_);
	switch( message )
	{
		case WM_PAINT:
		{								
			
						
			FRectF rc1 = rc_.ZeroRect();

			if ( stat_ & DROPPING )
				FillRectangle( d->cxt_, rc1, d->cxt_.red );
			else
				FillRectangle( d->cxt_, rc1, d->cxt_.white );

		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));


			if ( rc_.ZeroRect().PtInRect( pt ))
			{
				wndstat_ = 0;

				FRectF rc(rc_.left, rc_.top, FSizeF(800,400) );
				SetRect( rc );
				ret = 1;
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.ZeroRect().PtInRect( pt ))
			{
				d->SetCapture(this, &pt);
				ret = 1;
				md_ = 2;
			}

		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( d->GetCapture() == this && md_ == 2 )
			{
				//if ( rc_.ZeroRect().PtInRect( pt ))
				{
					FPointF ptprv = d->CapturePoint( pt );								
					rc_.Offset( (pt.x - ptprv.x)*mat_._11, (pt.y - ptprv.y)*mat_._22 );
					d->redraw_ = 1;

					
					
				}
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			//if ( rc_.ZeroRect().PtInRect( pt ))
			{
				if ( d->GetCapture() == this )
				{
					d->ReleaseCapture();
					ret = 1;
					md_ = 0;



					if ( stat_ & DROPPING )
					{
						
						auto t = parent_control_->Detach( this );

						//test->controls_.push_back( t );

						//t->parent_control_ = test;

						stat_ &= ~DROPPING;
						wndstat_ = 0;
					}
				}
			}
		}
		break;
		case WM_D2D_DROPOBJECT_FOUND_ACCEPTOR:
		{
			if ( lParam == 1 )
				stat_ |= DROPPING;
			else
				stat_ &= ~DROPPING;


			//test = (D2DControls*)wParam;
		}
		break;


	}


	if ( message == WM_PAINT || message == WM_MOUSEMOVE )
		dp_rc_ = mat.LPtoDP(rc_);

	mat.PopTransform();
	return ret;

}
LRESULT D2DControlsEx::WndProc0(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;
	
	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	if ( message == WM_PAINT )
	{		
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
		mat.Offset( rc_.left, rc_.top );
		
		{
			mat._11 = mat2_._11;
			mat._22 = mat2_._22;

			mat.SetTransform();
		}

		if ( md_ != 2 )
			mat_ = mat; // 自座標(左上0,0)のmatrix
	}
	//else
	//	mat = mat_;	// mat_と同じ値にする。

	//if ( message == WM_PAINT || message == WM_MOUSEMOVE || message == WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR )
	//	dp_rc_ = mat.LPtoDP(rc_);


	switch( message )
	{	
		case WM_PAINT:
		{									
			auto& cxt = d->cxt_;
			_ASSERT(rc_.BoderWidth_ > 0 );
					
			FRectF rc1 = rc_.GetBorderRectZero();
			// Draw window frame
			DrawFill(cxt, rc1, cxt.black);
			FRectF rccon = rc_.GetContentRectZero();
			D2DRectFilter f2(d->cxt_, rccon);
			DrawFill(cxt, rccon, cxt.white);
						
			rc1.Offset( 0, titlebar_height_  );					
			rc1.bottom -= titlebar_height_;
				
			FRectF rc11 = rc1;
			rc11.top -= rc_.BoderWidth_;
			
			// タイトルバー			
			auto it = controls_.begin();
			{
				(*it)->WndProc( d, message, wParam,lParam); // draw title bar
				it++;
			}

			

			mat.Offset( 0, titlebar_height_ );
			{
				for( ;it != controls_.end(); ++it )
				{
					auto obj = (*it).get();
					obj->WndProc(d,message,wParam,lParam);
				}
			}			
			mat.Offset( 0, -titlebar_height_ );

			// Draw resize mark. right-bottom.
			FRectF rc2( rc1.right-30,rc1.bottom-30,rc1.right,rc1.bottom );
			DrawResizMark( cxt, rc2 );
	
		}
		break;		

		case WM_LBUTTONDOWN:
		{
			FRectF rc( rc_.right-30, rc_.top, rc_.right, rc_.top+30 );
			rc.Offset( -rc_.left, -rc_.top );

			auto it = controls_.begin();
			FSizeF sz = (*it)->GetRect().Size();

			FRectF rc2( rc_.left, rc_.top, sz ); //titlebar_ ); //rc_.left+30, rc_.top+30 );
			rc2.Offset( -rc_.left, -rc_.top );

			
			FPointF pt = mat_.DPtoLP( FPointF(lParam));


			if ( rc_.ZeroRect().PtInRect( pt ))
			{
				ret = 1;
				
				if ( d->GetCapture() != this )
					d->SetCapture( this, &pt );
				else
					d->CapturePoint( pt );

				md_ = 0;
				if ( rc.PtInRect( pt ) )
				{
					//d->SetCapture( this );
					bExpand_ = !bExpand_;
					md_ = 1;
					Expand( bExpand_ );
				}
				else if ( rc2.PtInRect( pt ) )
				{
					//d->SetCapture( this );
					auto lr = (*it)->WndProc( d, message, wParam, lParam );

					if ( lr == 0 )
						md_ = 2;
				}
				else
				{					
					FRectF rc(rc_.right - 30, rc_.bottom-30, rc_.right, rc_.bottom); // 右下
					rc.Offset(-rc_.left, -rc_.top);
					if (rc.PtInRect(pt))
					{
						md_ = 3; // resize
					}
					else
					{
						SendMessageAll(d,message,wParam,lParam);	
					}
				}
			}
			else if (this == d->GetCapture() && bDialog_ == false )
			{
				d->ReleaseCapture();

				ret = LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS;
			}			
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( md_ == 3 )
			{
				lParam = MAKELPARAM(rc_.Width(), rc_.Height());
				SendMessageAll(d, WM_SIZE, 0, lParam);
			}

			md_ = 0;
			SendMessageAll(d,message,wParam,lParam);	
		}
		break;
		case WM_MOUSEMOVE:
		{
			 FPointF pt = mat_.DPtoLP(FPointF(lParam));
			if ( md_ == 2 && d->GetCapture() == this )
			{								
				FPointF ptprv = d->CapturePoint( pt );
								
				rc_.Offset( (pt.x - ptprv.x)*mat_._11, (pt.y - ptprv.y)*mat_._22 );

				d->redraw_ = 1;
				
			}		
			else if (md_ == 3 && d->GetCapture() == this)
			{
				FPointF ptprv = d->CapturePoint(pt);

				rc_.right += (pt.x - ptprv.x)*mat_._11 ;
				rc_.bottom += (pt.y - ptprv.y)*mat_._22;


				D2DTitleBar* bar = (D2DTitleBar*)controls_[0].get();
				
				FRectFBoxModel rc1 = bar->GetRect();

				rc1.right = rc1.left + rc_.GetBorderRect().Width();

				bar->SetRect( rc1 );

				d->redraw_ = 1;
			}
			else if (ENABLE(stat_))
			{
				FPointF pt3 = pt; 
				pt3.x += rc_.left;
				pt3.y += rc_.top;

				if ( rc_.PtInRect( pt3 ))
				{
					if ( !mouse_enter_ )
					{
						wParam = wParam | MK_F_MOUSE_ENTER;
						mouse_enter_ = true;
					}
					SendMessageAll(d,message,wParam,lParam);
				
				}
				else if ( mouse_enter_ )
				{
					wParam = wParam | MK_F_MOUSE_LEAVE;
					SendMessageAll(d,message,wParam,lParam);				
					mouse_enter_ = false;
				}
			}
			
			if (d->GetCapture() == this)
			{
				FRectF rc(rc_.right - 30, rc_.bottom - 30, rc_.right, rc_.bottom); // 右下
				rc.Offset(-rc_.left, -rc_.top);
				if (rc.PtInRect(pt))
				{
					::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
				}
				else
				{
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				}
				d->redraw_ = 1;
			}

		}
		break;
		case WM_SIZE:
		{			
			if ( bwindow_size_ ) 
			{
				FSizeF sz(lParam);
				sz = mat_.DPtoLP(sz);
				rc_.bottom = rc_.top + sz.height;
				rc_.right = rc_.left + sz.width;
			}

			SendMessageAll(d,message,wParam,lParam);		
		}
		break;
		case WM_D2D_GET_CONTROLS:
		{
			if ( id_ == wParam )
			{
				D2DControls** p = (D2DControls**)lParam;

				*p = this;

				ret = 1;
			}
			else
				ret=SendMessageAll(d,message,wParam,lParam);
		}
		break;
		case WM_D2D_CREATE_CONTROLS:
		{
			int id = 0;

		}
		break;		
		case WM_KEYDOWN:
		{
			if ( bDialog_ && VK_ESCAPE == wParam )
			{
				DestroyControl();
				ret = 1;
			}
			
		}
		break;
		case WM_D2D_BUTTON_CLICK:
		{
			if ( (void*)wParam == this->tbar_ )
			{
				int id = lParam;

				if ( id == 1 )
				{
					d->ReleaseCapture();
					wndstat_ = 1;

					FRectF rc = rc_;
					rc.SetSize(100,100);

					SetRect( rc );
				}
			}


		}
		break;
		case WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR:
		{

			D2DControl* target = (D2DControl*)wParam;

			if ( this != target )
			{
				auto rc = target->GetDPRect();

				FRectF rc2 = mat.LPtoDP(rc_);


				TRACE( L"%f %f %f %f\n", rc2.left,rc2.top,rc2.right,rc2.bottom );
				TRACE( L"%f %f %f %f\n", rc.left,rc.top,rc.right,rc.bottom );

				ret = 0;
				if ( rc2.RectInRect( rc ) )
				{
					target->WndProc( d,WM_D2D_DROPOBJECT_FOUND_ACCEPTOR, (WPARAM)this, 1 );
					ret  = 1;
				}

				if ( ret == 0 )
					target->WndProc( d,WM_D2D_DROPOBJECT_FOUND_ACCEPTOR, (WPARAM)this, 0 );

			}
		}
		break;
		


		default:
			if (ENABLE(stat_))
				SendMessageAll(d,message,wParam,lParam);		
	}


	

	mat.PopTransform();


	
	return ret;
}
void D2DControlsEx::SetRect( const FRectF& rc )
{ 
	auto rc1 = tbar_->GetRect();
	rc1.right = rc1.left + rc.Width();

	tbar_->SetRect( rc1 );
	
	rc_ = rc; 
}	

void D2DControlsEx::Dialog(bool bDialog)
{
	bDialog_ = bDialog;

	if (bDialog)
	{
		parent_->SetCapture( this );
	}
	else if ( this == parent_->GetCapture())
	{
		parent_->ReleaseCapture();
	}
}
void D2DControlsEx::TopDialog()
{
	bDialog_ = true;	
	parent_->SetCapture( this );
		
	auto mat = parent_control_->Matrix();
	
	auto me = parent_control_->Detach(this);
	
	// position adjust.
	FRectF rc = mat.LPtoDP( rc_ );
		
	parent_->children_->controls_.push_back( me );
	parent_control_ = parent_->children_.get();

	mat = parent_control_->Matrix();
	
	rc_ = mat.DPtoLP( rc );
	
}
void D2DControlsEx::Expand( bool bExpand )
{
	if ( !bExpand )
	{		
		FSizeF sz = rc_.GetSize();
			
		mat2_ = mat_;

		MoveTarget* mt = new MoveTarget();

		mt->SetParameter( mat2_._11, mat2_._22, 0.3f, 0.3f, 500 ); // 100 / sz.width,100 / sz.width, 500 );

		mt->Fire_ = [this](MoveTarget* p, float a, float b)->bool{

			mat2_._11 = a;
			mat2_._22 = b;
			
			return true;

		};

		mt->End_ = [this](MoveTarget* p, float a, float b){
			
			delete p;
		};
		
		mt->Start(parent_);

		
		

		bExpand_ = bExpand;
	}
	else
	{
		
		MoveTarget* mt = new MoveTarget();

		mt->SetParameter( mat2_._11, mat2_._22, 1,1 , 500 );

		mt->Fire_ = [this](MoveTarget* p, float a, float b)->bool{

			mat2_._11 = a;
			mat2_._22 = b;
			
			return true;

		};

		mt->End_ = [this](MoveTarget* p, float a, float b){
			
			mat2_._11 = 1;
			mat2_._22 = 1;
			delete p;
		};
		
		mt->Start(parent_);

		bExpand_ = bExpand;

	}
}

};
