#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "D2DContextNew.h"
//#include "MoveTarget.h"
 

using namespace V4;

#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"winmm.lib")

#define TITLE_BAR_HEIGHT 24


		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D2DControl::D2DControl()
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT( _CrtIsValidHeapPointer(this));
}
int D2DControl::Stat( int new_stat)
{
	if ( new_stat < 0 )
		return stat_;
	
	stat_ = new_stat;
	parent_->redraw_ = 1;

	return stat_;
}
void D2DControl::Enable(bool bEnable)
{
	stat_ = (bEnable ? (stat_ & ~STAT::DISABLE) : (stat_ | STAT::DISABLE) );
}
void D2DControl::Visible(bool bVisible)
{
	stat_ = (bVisible ? (stat_ | STAT::VISIBLE) : (stat_ & ~STAT::VISIBLE) );
}
bool D2DControl::IsEnable() const
{
	return !(stat_ & STAT::DISABLE);
}
bool D2DControl::IsVisible()   const
{
	return (stat_ & STAT::VISIBLE);
}
void D2DControl::StatActive(bool bActive)
{
	if ( bActive )
		stat_ |= FOCUS;
	else
		stat_ &= ~FOCUS;
}

void D2DControl::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow( parent, pacontrol,rc,stat,name,controlid);

	OnCreate();
}
void D2DControl::InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	_ASSERT(parent);

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	parent_control_ = pacontrol;
	name_ = name;
	id_ = controlid;
	target_ = nullptr; // このオブジェクトと関連付けるポインタ、通常はnull

	if (parent_control_)
	{
		parent_control_->controls_.push_back(std::shared_ptr<D2DControl>(this));
	}

	if ( stat_ & DROPACCEPTOR )
	{
		parent->drag_accepters_.push_back( this );

		stat_ &= ~DROPACCEPTOR;
	}

	CHDL id = parent_->chandle_.CreateControlHandle( this );
	chdl_ = id;
}

void D2DControl::DestroyWindow()
{
	stat_ &= ~STAT::VISIBLE;
	stat_ |= STAT::DEAD;

	if ( parent_->GetCapture() == this )
		parent_->ReleaseCapture(); // 1階層目だけはcheck


	if ( parent_control_ )
	{
		auto p = parent_control_->Detach( this );

		parent_->death_objects_.push_back(p);
	}

	parent_->chandle_.DeleteControlHandle( chdl_ );
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DControls::CreateWindow( D2DWindow* parent, D2DControls* paconrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	xassert( parent );
	xassert( paconrol != this );

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	parent_control_ = paconrol;
	name_ = name;
	id_ = id;
	auto_resize_ = false;

	bwindow_size_ = (rc.Width() <= 0);
	if ( bwindow_size_ )
	{
		_ASSERT( rc.left == 0 && rc.top == 0 );
	}

	if ( parent_control_ )
		parent_control_->controls_.push_back( std::shared_ptr<D2DControl>( this ));
	else if ( parent_  )
	{
		_ASSERT( parent_->children_  == NULL );
		parent_->children_ = std::shared_ptr<D2DControls>( this );		
	}


	if ( stat_ & DROPACCEPTOR && parent_ )
	{
		parent->drag_accepters_.push_back( this );
		stat_ &= ~DROPACCEPTOR;
	}


	OnCreate();
}
void D2DControls::Clear()
{
	controls_.clear();
	
}
void D2DControls::UpdateScrollbar(D2DScrollbar* bar)
{
	//if ( bar->info_.bVertical )	
	//	scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	//else
	//	scrollbar_off_.width = bar->info_.position;
}

LRESULT D2DControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;
	
	D2DContext& cxt = d->cxt_;
	D2DMatrix mat(cxt);		
	mat.PushTransform();
	if ( message == WM_PAINT )
	{		
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
mat.Offset( rc_.left, rc_.top );
		mat_ = mat; // 自座標(左上0,0)のmatrix
	}
	else
		mat = mat_;	// mat_と同じ値にする。
	
	

	switch( message )
	{
		case WM_PAINT:
		{						
			if (stat_ & VISIBLE )
			{
//
//#ifdef _DEBUG				
//				if ( (stat_ & BORDER) && (stat_ & DEBUG1) )
//				{
//					FRectF rc1 = rc_.ZeroRect();
//					DrawFillRect( cxt, rc1, d->cxt_.black, d->cxt_.ltgray, 1.0f );
//					DrawLine( cxt, rc1.LeftTop(), rc1.RightBottom(), 1, cxt.bluegray );
//
//				}
//#endif

				SendMessageReverseAll(d,message,wParam,lParam);	

			}

			
		}
		break;		
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:

			ret = SendMessageAll(d, message, wParam, lParam);
		break;
		case WM_MOUSEMOVE:
		{
			if (ENABLE(stat_))
			{
				FPointF pt3 = mat_.DPtoLP( FPointF(lParam));
				pt3.x += rc_.left;
				pt3.y += rc_.top;

				if ( rc_.PtInRect( pt3 ))
				{
					if ( !mouse_enter_ )
					{
						wParam = wParam | MK_F_MOUSE_ENTER;
						mouse_enter_ = true;
					}
					ret = SendMessageAll(d, message, wParam, lParam);
				
				}
				else if ( mouse_enter_ )
				{
					wParam = wParam | MK_F_MOUSE_LEAVE;
					ret = SendMessageAll(d, message, wParam, lParam);
					mouse_enter_ = false;
				}
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

			if (auto_resize_)
			{
				// top,leftがある場合は、parentのサイズにあわせる
				auto sz = parent_control_->GetContentRect().Size();
				rc_.bottom = sz.height;// - rc_.top;
				rc_.right = sz.width;// - rc_.left;
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
		
		case WM_D2D_OBJECT_SUICIDE:
		{
			D2DControl* p = (D2DControl*)lParam;

			if (p->parent_control_ ==  this )
			{
				auto it = controls_.begin();
				
				for( ; it != controls_.end(); it++ )
				{
					D2DControl* d = (*it).get();
					if ( d == p )
						break;
				}

				if ( it!=controls_.end())
				{
					controls_.erase( it ); // PostMessageされたWM_D2D_OBJECT_SUICIDEにより、targetを削除する					
				}
				ret = 1;
			}
			else if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);	
		}
		break;
		default:
			if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);		
	}

	mat.PopTransform();
	return ret;
}
LRESULT D2DControls::SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	for( auto it = controls_.begin(); it!=controls_.end(); ++it )// 順番：表示奥のオブジェクトから前面のオブジェクトへ
	{
		auto obj = (*it).get();
		xassert( obj != this );

		r += obj->WndProc(parent,message,wParam,lParam);

		if (r != 0 && r != LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS)
			break;

	}
	return r;


}
LRESULT D2DControls::SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;

	for( auto it = controls_.rbegin(); it!=controls_.rend(); ++it ) // 順番：表示前面のオブジェクトから奥へ
	{
		auto obj = (*it).get();
		xassert( obj != this );
		
		int size = controls_.size();

		r += obj->WndProc(parent,message,wParam,lParam);

		if (r != 0 && r != LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS)
			break;

		xassert( size == controls_.size() ); // ここで落ちる場合は、r=1を返せ。
	}
	return r;
}


void D2DControls::Attach( std::shared_ptr<D2DControl> ctrl )
{
	_ASSERT( ctrl->parent_control_ == NULL );

	controls_.push_back( ctrl );
	ctrl->parent_control_ = this;

}


std::shared_ptr<D2DControl> D2DControls::Detach(D2DControl* target)
{
	for( auto it = controls_.begin(); it != controls_.end(); ++it )
	{
		if ( (*it).get() == target )
		{
			std::shared_ptr<D2DControl> ret = (*it);

			controls_.erase( it );

			ret->parent_control_ = NULL;

			return ret;
		}
	}

	_ASSERT( false );

	return NULL;
}

void D2DControls::MeToLast()
{
	std::vector<std::shared_ptr<D2DControl>>& ls = this->parent_control_->controls_;
	
	std::shared_ptr<D2DControl> me;
	
	int j = 0;
	for(auto it=ls.begin(); it != ls.end(); it++ )
	{
		if ( this == (*it).get() )
		{
			me = (*it);
			ls.erase( it );	 // remove
			vector_idx.first = j;
			break;
		}		
		j++;
	}
	
	ls.push_back( me ); // add to last.


	vector_idx.second = ls.size()-1;
}
LRESULT D2DControls::KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	LRESULT ret = 0;
	switch( message )
	{
		// TABキーでコントロール間を移動
		
		case WM_KEYDOWN:
		{
			if ( wParam == VK_TAB )
			{
				auto& ls = controls_; //  created object order.

				bool bl = false;
				for( int i = 0; i < 2; i++ ) // first->last->first and break.
				{
					
					int j = 0;
					
					for( auto& it : ls )
					{
						auto nm = it->GetName();
						//TRACE( L"%x name=%s \n", this, (LPCWSTR)nm.c_str() );
						
						if ( it.get() == sender )
						{
							
							if ( parent_->GetCapture() == it.get())
							{
								parent_->ReleaseCapture();							
							}

							

							//TRACE( L"%x UNACTIVE %s ", this, (LPCWSTR)nm );
							
							it->StatActive(false);
							bl = true;
						}
						
						j++;
					}
				}
			}
		}
		break;
	}

	return ret;

	return 0; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DButton::DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s )
{	
	FRectF rcb = rc.GetBorderRect();


	DrawRect( cxt.cxt, rcb, cxt.black, 1);
	//DrawBox( cxt.cxt, rc, cxt.black, cxt.white );

	FRectF rcc = rc.GetContentRect();

	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, rcc, s, cxt.gray );
	}
	else if ( stat & CLICK )
		DrawButton( cxt.cxt, rcc, L"onclick", cxt.red );
	else if ( stat & MOUSEMOVE )
		DrawButton( cxt.cxt, rcc, L"onmove", cxt.black );
	else if ( stat & FOCUS )
		DrawButton( cxt.cxt, rcc, s, cxt.red );
	else
		DrawButton( cxt.cxt, rcc, s, cxt.black );


	

}
void D2DButton::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent, pacontrol, rc, stat, name, id );
	ondraw_ = DrawContent;


}
void D2DButton::OnDraw( D2DContext& cxt )
{
	if ( VISIBLE( stat_ ))
	{
		D2DMatrix mat(cxt);
		mat_ = mat.PushTransform();	
		mat.Offset( rc_.left, rc_.top );

		ondraw_( cxt, rc_.ZeroRect(), stat_, name_ );		
		
		if ( parent_->GetCapture() == this )
		{			
			cxt.cxt->FillRectangle( rc_.ZeroRect().GetBorderRect(), cxt.halftone );
		}		

		mat.PopTransform();
	}
}



LRESULT D2DButton::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;

	switch( message )
	{
		
		case WM_PAINT:
		{			
			OnDraw( d->cxt_  );						
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if (ENABLE(stat_))
			{
				FRectF rcb = rc_.GetBorderRect();
				FPointF pt = mat_.DPtoLP( lParam );
				if ( rcb.PtInRect( pt ) )
				{
					Stat( stat_ | CLICK | FOCUS );

					
					if ( parent_control_ )
					{
						//parent_control_->WndProc( parent_, WM_D2D_BUTTON_CLICK, id_,(LPARAM)this);
						::SendMessage( d->hWnd_, WM_D2D_BUTTON_CLICK, id_,(LPARAM)this);

					}
					// after sending message.
					d->SetCapture( this );
					Stat( stat_ | CLICK | FOCUS|CAPTURED );
					ret = 1;
				}
			}
		}
		break;
		case WM_MOUSEMOVE:
		{		
			//if ( d->GetCapture() == this )
			{
				if (ENABLE(stat_))
				{
					FPointF pt = mat_.DPtoLP(lParam); // 親の座標へ変換, rc_は親座標

					bool bl = rc_.PtInRect2( pt );
													
					int stat =( bl ?  (stat_ | MOUSEMOVE) :  (stat_ & ~MOUSEMOVE) );

					if ( stat != stat_ )
					{
						Stat( stat );				
						ret = 1;
					}
				}				
			}
		}
		break;		
		
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if (ENABLE(stat_))
			{
				if ( d->GetCapture() == this )
				{
					d->ReleaseCapture();
					Stat( stat_ & ~CLICK );	
					Stat( stat_ & ~FOCUS );	
					Stat( stat_ & ~CAPTURED );	

					FRectF rcb = rc_.GetBorderRect();
					FPointF pt = mat_.DPtoLP( lParam);
					if ( rcb.PtInRect( pt ) )
					{
						if ( OnClick_ )
							OnClick_(this);
					}														
					ret = 1;
				}
			}
		}
		break;
		case WM_KEYDOWN:
		{
			if ( wParam == VK_RETURN )
			{
				Stat( stat_ | CLICK |FOCUS|CAPTURED );
				d->SetCapture(this);
				if ( parent_control_ )
				{
					//parent_control_->WndProc( parent_, WM_D2D_BUTTON_CLICK, id_,(LPARAM)this);
					::SendMessage( d->hWnd_, WM_D2D_BUTTON_CLICK, id_,(LPARAM)this);
					ret = 1;
				}
			}
			else if ( wParam == VK_TAB )
			{
				ret = parent_control_->KeyProc(this, message,wParam,lParam );
			}
		}
		break;
		case WM_KEYUP:
		{
			if ( wParam == VK_RETURN )
			{
				bool bl = ( d->GetCapture() == this );

				if ( bl )
					d->ReleaseCapture();


				if ( bl && (stat_ & CLICK) )
				{
				
					Stat( stat_ & ~CLICK );	
					Stat( stat_ & ~FOCUS );	
					Stat( stat_ & ~CAPTURED );	
					if ( OnClick_ )
						OnClick_(this);
					ret = 1;
				}
			}
			else
			{
				ret = parent_control_->KeyProc(this, message,wParam,lParam );
			}
		}
		break;


		case WM_D2D_JS_ENABLE:
		if ( id_ == wParam )
		{
			VARIANT* pv = (VARIANT*)lParam;
			
			if ( pv->vt == VT_I4 )
			{
				if( pv->intVal == 0 )
					stat_ |= STAT::DISABLE;
				else
					stat_ &= ~STAT::DISABLE;
			}
			
			ret = 1;
			
		}
		break;
		case WM_D2D_JS_SETTEXT:
		if ( id_ == wParam )
		{
			VARIANT* pv = (VARIANT*)lParam;
			if ( pv->vt == VT_BSTR )
			{
				name_ = pv->bstrVal;				
				ret = 1;
			}
		}
		break;


	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTopControls::CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id )
{
	CreateWindow(d,NULL,FRectFBoxModel(0,0,-1,-1),stat,name,id );

}
void D2DTopControls::CreateWindow(D2DWindow* d, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	xassert( pacontrol == NULL );
	
	background_ = d->cxt_.white;
	zoom_ =1.0f;
	bAutoSize_ = true;
			
	D2DControls::CreateWindow(d, pacontrol, rc, stat,name, id );
	
}
FRectF D2DTopControls::CalcAutoSize( const GDI32::FSize& sz )
{
	FRectF rc = rc_;

	if ( calc_auto_size_ )
		rc = calc_auto_size_( rc, sz );
	else
	{
		rc.SetSize(sz);
	}
	return rc;
}

LRESULT D2DTopControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return 0;
	
	D2DMatrix mat(d->cxt_);
	mat.PushTransform();
	mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
	mat.Offset( rc_.left, rc_.top );

	mat._11 = zoom_ ;
	mat._22 = zoom_ ;
	
	mat.SetTransform();

	mat_ = mat; 

	FPointF pt21 = mat.DPtoLP( FPointF(lParam));

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;

			auto rcx = rc_.ZeroRect();
			
			D2DRectFilter f(cxt, rcx );

//FillRect(cxt,rcx,background_);

#ifdef _SKEYBLUE_GRID
			RECT rc;
			GetClientRect(d->hWnd_,&rc);
			DrawSkyBlueGrid( cxt, FSizeF(rc.right,rc.bottom),50);
#endif
			
			// DrawDebugCross(d->cxt_, d->cxt_.black );
			SendMessageReverseAll(d,message,wParam,lParam);	

		}
		break;	
		case WM_D2D_JS_ERROR:
		{
			error_msg_ = (BSTR)lParam;

			ret = 1;
			

		}
		break;
		
		case WM_LBUTTONDOWN:		
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21))
					SendMessageAll(d,message,wParam,lParam);
				else
					SendMessageAll(d,WM_D2D_NCLBUTTONDOWN,wParam,lParam);
			}
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21))
					SendMessageAll(d,message,wParam,lParam);
			}
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:		
		case WM_LBUTTONUP:
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21))
					SendMessageAll(d,message,wParam,lParam);
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21))
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
		}
		break;
		case WM_SIZE:
		{
			if ( bAutoSize_ )
			{
				rc_ = CalcAutoSize(GDI32::FSize(lParam));
			}
			
			SendMessageAll(d,message,wParam,lParam);		
		}
		break;
		case WM_DESTROY:
			SendMessageAll(d,message,wParam,lParam);		
		break;

	
		case WM_D2D_GET_CONTROLS:
		{
			if ( id_ == wParam )
			{
				D2DControls** p = (D2DControls**)lParam;
				*p = this;
				ret = 1;
			}
			else if (ENABLE(stat_))
				SendMessageAll(d,message,wParam,lParam);
		}
		break;
		case WM_D2D_TEST:
			//V4::MessageBox(this,L"hoi",L"hoi", MB_OK ); 
			ret = 1;
		break;
		default:
			if (ENABLE(stat_))
				SendMessageAll(d,message,wParam,lParam);		
	}

	mat.PopTransform();
	return ret;
}



namespace V4{ 
void DrawFillRect( ID2D1RenderTarget* cxt_, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
{
	
	
	_ASSERT( width > 0 );

	// Line is FillRectangle.

	FRectF yrc( rc.left + width,rc.top + width,rc.right - width,rc.bottom - width );

	// Draw fill
	cxt_->FillRectangle( yrc, fillclr );	


	// Draw side
	FRectF trc( rc.left, rc.top, rc.right, yrc.top );
	cxt_->FillRectangle( trc, wakuclr );				
	trc.top = yrc.bottom;
	trc.bottom = rc.bottom;
	cxt_->FillRectangle( trc, wakuclr );				
	trc.right = yrc.left;
	trc.top = rc.top;
	cxt_->FillRectangle( trc, wakuclr );				
	trc.left = yrc.right;
	trc.right = rc.right;
	cxt_->FillRectangle( trc, wakuclr );				
	
	

//  この関数はDrawFillWakuと同じになる
} 


}