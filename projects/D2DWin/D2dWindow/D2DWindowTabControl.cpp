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
#include "D2DWindowTabControl.h"

using namespace V4;

//////////////////////////////////////////////////////////////////////////////////////////////////

#define TAGHEIGHT 20
#define TAGWIDTH 70
D2DTabControls::D2DTabControls()
{

}
D2DTabControls::~D2DTabControls()
{
	int a = 0;
}


//FSizeF D2DTabControls::GetChildSize(D2DControl* child) const
//{ 
//	auto sz = rc_.Size(); 
//
//	sz.height = sz.height - TAGHEIGHT;
//
//	return sz;
//}

void D2DTabControls::RequestUpdate(D2DControl* client, int typ)
{
	if (typ == WM_SIZE )
	{
		int j = 0;
		for( auto& it : controls_ )
		{
			if ( it.get() == client )
			{
				
				//auto nsz = rcc_[j].Size();
				FRectF rc1 = rc_.GetContentRectZero();
				rc1.top += TAGHEIGHT + 6;

				client->SetRect(FPointF(0,0), rc1.GetSize());


				break;
			}
			j++;
		}
	}

}

LRESULT D2DTabControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ( WM_D2D_BROADCAST_UPDATE == message )
	{
		for( auto& it : controls_ )
			it->WndProc( d, WM_D2D_BROADCAST_UPDATE, wParam,lParam);
		return ret;
	}

	if ( !VISIBLE(stat_))
		return ret;

	switch( message )
	{

		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);		
			mat.PushTransform();

			mat_ = mat; // 親コントロールが原点		
			mat.Offset(rc_.left, rc_.top);

			// 自座標　左上が(0,0)となる
			FRectF rc1 = rc_.GetContentRectZero();
		
			FRectF rc = OnTagDraw(d->cxt_);
			FRectF rc2 = rc_; //.GetContentRect();
			rc2.bottom = rc.bottom + 3;
			float ofy = rc2.bottom;
			
			{
				ComPTR<ID2D1PathGeometry> pGeometry;
			
				rc.top += 2;
				rc.left += 2;
				rc2.top += 2;
				rc2.left += 2;

				CreateTagButtomGeometry(cxt.cxt, rc, rc2, &pGeometry);

				cxt.cxt->FillGeometry(pGeometry, d->cxt_.ltgray);

				OnTagDraw(d->cxt_);
			}

			mat.PushTransform();
			{
				mat.Offset(0, ofy);

				

				controls_[active_idx_]->WndProc(d,message,wParam,lParam);	
			}
			mat.PopTransform();
			
				

			mat.PopTransform();	

			return 0;
		}
		break;
		
		case WM_LBUTTONDOWN:
		{
			FPointF pt21 = mat_.DPtoLP(lParam); 	
			FRectF rc = rc_; rc.SetHeight( TAGHEIGHT );

			if ( rc.PtInRect(pt21))
			{
				
				d->SetCapture(this);
				
				ret = 1;
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				FPointF pt21 = mat_.DPtoLP(lParam); 
				FRectF rc = rc_; rc.SetHeight( TAGHEIGHT );			
				if ( rc.PtInRect(pt21))
				{
					d->ReleaseCapture();

					pt21.x -= rc.left;
					pt21.y -= rc.top;

					UINT idx = HitTagPoint( pt21 );

					if ( idx != -1 )
						SetActivePage(idx);						

					
					ret = 1;

					d->redraw_ = 1;
				}
			}
		}
		break;
		case WM_SIZE :
		{
			auto p = dynamic_cast<IUpdatar*>( parent_control_ );
			if ( p )
			{
				p->RequestUpdate(this, WM_SIZE );
			}
			else
			{
				auto sz = parent_control_->GetChildSize(this);
				rc_.SetSize(sz);
			}

			SendMessageAll(d,message,wParam,lParam);		
			return 0;

		}
		break;
		/*case WM_D2D_RESTRUCT_RENDERTARGET :
			for( auto& it : controls_ )
				it->WndProc(d,message,wParam,lParam);*/


		break;
		case WM_D2D_TAB_ACTIVE:
			SetActivePage( (int)lParam );
		break;
		case WM_DESTROY:
			for( auto& it : controls_ )
				it->WndProc(d,message,wParam,lParam);
			ret = -1;
		break;
		
	}


	
	if ( active_idx_ < controls_.size() && ret == 0 )
		ret = controls_[active_idx_]->WndProc(d,message,wParam,lParam);

	
	return (LRESULT)( ret <= 0 ? 0 : ret );
}
void D2DTabControls::SetActivePage( int idx )
{
	int stat = controls_[active_idx_]->GetStat();
	stat &= ~FOCUS;
	controls_[active_idx_]->SetStat( stat );

	active_idx_ = idx;
	stat = FOCUS | controls_[active_idx_]->GetStat();
	controls_[active_idx_]->SetStat( stat );
}


void D2DTabControls::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id  )
{
	FRectFBoxModel rcx(rc);
	if ( rcx.Width() == 0 )
	{
		auto rc2 = pacontrol->GetRect().GetContentRectZero();
		rcx.SetRect( rc2 );		
	}
	
	
	D2DControls::CreateWindow(parent,pacontrol,rcx,stat,name,id );
	
	FRectF rc1 = rcx.GetContentRectZero();

	D2DControlsWithScrollbar* b1 = new D2DControlsWithScrollbar();
	b1->CreateWindow( parent, this, rc1, VISIBLE, L"page1", 0 );
	active_idx_ = 0;
	SetActivePage(active_idx_);
	scale_ = 0;
}
FRectF D2DTabControls::OnTagDraw( D2DContext& cxt )
{
	FSizeF tag(TAGWIDTH,TAGHEIGHT);
	
	FRectF rc(0,0,tag);

	FRectF ret;

	UINT k = 0;


	rc.bottom += 3;
	for( auto&it : controls_ )
	{		
		if ( k == active_idx_ )
		{		
			ret = rc;
		}
				
		rc.Offset( tag.width, 0 );
		k++;
	}

	rc.SetRect(0,0,tag);

	for( auto&it : controls_ )
	{		
		auto br = cxt.gray;
		
		if ( it->IsEnable())
		{
			if ( it->GetStat()&FOCUS )
				br = cxt.red;
			else
				br = cxt.black;
		}		
		
		
		D2DTextWrite info;
		info.factory = cxt.wfactory;
		info.textformat =  cxt.text;
		DrawCenterText( cxt, info, br, rc, it->GetName(), it->GetName().length(),1 );
		
		rc.Offset( tag.width, 0 );
	}

	return ret;
}
FRectF D2DTabControls::GetContentRect()
{
	FRectF rc(rc_); 

	rc.bottom -= TAGHEIGHT;	


	return rc;
}
D2DControls* D2DTabControls::Child( UINT idx )
{
	xassert( idx < controls_.size() );
	
	D2DControl* pc = controls_[idx].get();
	
	auto ret = dynamic_cast<D2DControls*>(pc);
	
	return ret;

}
UINT D2DTabControls::HitTagPoint( FPointF pt )
{
	FSizeF tag(TAGWIDTH,TAGHEIGHT);
	FRectF rc(0,0,tag);

	UINT r = 0;

	UINT cnt = controls_.size();

	while( r < cnt )
	{
		if ( rc.PtInRect(pt ))
		{
			if ( controls_[r]->IsEnable())			
				return r;
		}

		rc.Offset(tag.width, 0 );
		r++;
	}


	return -1;

}
UINT D2DTabControls::AddChild( LPCWSTR name )
{	
	auto rc = controls_[0]->GetRect();
	
	D2DControlsWithScrollbar* b2 = new D2DControlsWithScrollbar();
	b2->CreateWindow( parent_, this, rc, VISIBLE, name, 0 );
	b2->auto_resize_ = true;

	return CountChild()-1;
}
void D2DTabControls::TagEnable( UINT idx, bool bEnable )
{
	if ( idx < controls_.size())
	{
		controls_[idx]->Enable(bEnable);
	}

}
void D2DTabControls::AutoScale( bool bWidth, bool bHeight )
{
	if ( bWidth ) 
		scale_ |= AUTO::WIDTH;
	else
		scale_ &= ~AUTO::WIDTH;

	if ( bHeight ) 
		scale_ |= AUTO::HEIGHT;
	else
		scale_ &= ~AUTO::HEIGHT;

}
