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

	D2DContext& cxt = d->cxt_;
	D2DMatrix mat(cxt);		
	mat.PushTransform();
	

	switch( message )
	{

		case WM_PAINT:
		{			
			
			mat_ = mat; // 親コントロールが原点		
			mat.Offset(rc_.left, rc_.top);

			// 自座標　左上が(0,0)となる
			FRectF rc1 = rc_.GetContentRectZero();
			
			//DrawDebugCross(cxt, cxt.black);

			//D2DRectFilter f(d->cxt_, rc1);	
			
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
				//D2DRectFilter(cxt, rc2);
			
				//cxt.cxt->FillRectangle( rc2, cxt.red);
				cxt.cxt->FillGeometry(pGeometry, d->cxt_.ltgray);

				OnTagDraw(d->cxt_);
			}

			mat.PushTransform();
			{
				mat.Offset(0, ofy);

				

				controls_[active_idx_]->WndProc(d,message,wParam,lParam);	
			}
			mat.PopTransform();
			
				
			ret = -1;				
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
			if ( scale_ & AUTO::HEIGHT || auto_resize_ )
			{
				auto rr = parent_control_->GetRect();
				auto sz = parent_control_->GetRect().GetContentRect().Size();
				auto sz2 = parent_control_->GetContentRect().Size();


				rc_.bottom = rc_.top + sz.height;

				for(auto& it : controls_ )
				{
					auto p = it.get();
					FRectFBoxModel rc = it.get()->GetRect();
					rc.bottom = rc.top + sz.height;
					it.get()->SetRect(rc);
				}
			}

			if ( scale_ & AUTO::WIDTH || auto_resize_ )
			{
				auto sz = parent_control_->GetRect().GetContentRect().Size();
				rc_.right = rc_.left + sz.width;

				for(auto& it : controls_ )
				{
					FRectFBoxModel rc = it.get()->GetRect();
					rc.right = rc.left + sz.width;
					it.get()->SetRect(rc);
				}
			}

	
			for( auto& it : controls_ )
				it->WndProc(d,message,wParam,lParam);


			ret = -1;
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

	mat.PopTransform();
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
