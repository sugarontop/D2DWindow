#include "stdafx.h"
#include "D2DWindow.h" 
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "D2DContextNew.h"
#include "MoveTarget.h"

#define CLASSNAME L"D2DWindow"
using namespace V4;

#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"winmm.lib")

#define TITLE_BAR_HEIGHT 24


		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DControl
D2DControl::D2DControl()
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT( _CrtIsValidHeapPointer(this));
}
D2DControl::~D2DControl()
{

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
void D2DControl::SetRect( const FRectF& rc )
{ 
	rc_.SetFRectF(rc); 
}
void D2DControl::SetRect( const FPointF& pt, const FSizeF& sz )
{ 
	rc_.SetFRectF( FRectF(pt,sz)); 
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
	guid_ = FString::NewGuid();

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

void D2DControl::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		//TRACE( L"me=%x parent=%x HWND=%x %d\n", this, parent_, parent_->hWnd_, (IsWindow(parent_->hWnd_)?1:0) );
		SendMessage( WM_D2D_DESTROY_CONTROL, 0, (LPARAM)this);
		
	
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
}
bool D2DControl::IsThroughMessage( UINT message )
{
	bool bUi = (WM_MOUSEFIRST <= message && message <= WM_MOUSELAST) || (WM_KEYFIRST <= message && message <= WM_KEYLAST);
	
	
	if ( VISIBLE(stat_))
	{
		if (!ENABLE(stat_) && bUi)
			return false;
	
		return true;
	}

	// 非表示なのでUIメッセージを無視する
	if ( WM_PAINT == message || WM_D2D_PAINT==message || bUi )
		return false;

	// 有効なメッセージ
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DControls::D2DControls()
{ 
	mouse_enter_ = false;
	scrollbar_off_ = FSizeF(0,0);
	clr_ = D2RGB(255,255,255);
}
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
	guid_ = FString::NewGuid();

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

	chdl_ = parent_->chandle_.CreateControlHandle( this );
		
	OnCreate();
}
void D2DControls::Clear()
{
	controls_.clear();
	
}
void D2DControls::UpdateScrollbar(D2DScrollbar* bar)
{
	if ( bar->info_.bVertical )	
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	else
		scrollbar_off_.width = bar->info_.position;
}


D2DControl* D2DControls::GetControl( int id )
{
	for( auto& it : controls_ )
	{
		if ( it->GetID() == id )
			return it.get();
	}
	return nullptr;
}

LRESULT D2DControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_DESTROY:
			SendMessageAll(d,message,wParam,lParam);
			return 0;
		break;
		case WM_D2D_RESTRUCT_RENDERTARGET:
			//CreateResource(wParam == 1 );				
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
	}


	
	
//	D2DContext& cxt = d->cxt_;
//	D2DMatrix mat(cxt);		
//	mat.PushTransform();
//	if ( message == WM_PAINT )
//	{		
//		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
//mat.Offset( rc_.left, rc_.top );
//		mat_ = mat; // 自座標(左上0,0)のmatrix
//	}
//	else
//		mat = mat_;	// mat_と同じ値にする。
	
	

	switch( message )
	{
		case WM_PAINT:
		{						
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);		
			mat_ = mat.PushTransform();
			mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
			mat.Offset( rc_.left, rc_.top );

			if ( (stat_ & BORDER) )
			{				
				FRectF rc1 = rc_.GetContentRectZero(); //.ZeroRect();
				
				CComPtr<ID2D1SolidColorBrush> backbr = MakeBrsuh(cxt, clr_ );		
				DrawFillRect( cxt, rc1, d->cxt_.black, backbr, 1.0f );
				

				rc1.InflateRect(-1,-1);
				D2DRectFilter fil( cxt, rc1 );

				SendMessageReverseAll(d,message,wParam,lParam);					
			}
			else
				SendMessageReverseAll(d,message,wParam,lParam);	


			mat.PopTransform();
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
				FPointF pt3 = mat_.DPtoLP(lParam);

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

			if ( controls_.size() == 1 )
			{
				auto it = controls_[0];
				if ( it->auto_resize_ )
				{
					auto rc = rc_.ZeroRect();
					it->SetRect( rc );
				}
			}
			else
			{
				




			}

			//if (auto_resize_)
			//{
			//	// top,leftがある場合は、parentのサイズにあわせる
			//	auto sz = parent_control_->GetContentRect().Size();
			//	rc_.bottom = sz.height;// - rc_.top;
			//	rc_.right = sz.width;// - rc_.left;
			//}


			SendMessageAll(d,message,wParam,lParam);		
			ret = 0;
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

	//mat.PopTransform();
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

		xassert( size == controls_.size() ); // ここで落ちる場合は、r=1を返せ。またはループが間違っている。
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
bool D2DControls::IsLast( D2DControl* obj ) const
{
	return ( controls_.rbegin()->get() == obj);

}
void D2DControls::MeToLast()
{
	std::vector<std::shared_ptr<D2DControl>>& ls = this->parent_control_->controls_;
	std::shared_ptr<D2DControl> me;
	

	for(auto it=ls.begin(); it != ls.end(); it++ )
	{
		if ( this == (*it).get() )
		{
			me = (*it);
			ls.erase( it );	 // remove
			break;
		}		
	}
	
	ls.push_back( me ); // add to last.


	//vector_idx.second = ls.size()-1;
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

							

							TRACE( L"%x UNACTIVE %s ", this, (LPCWSTR)nm );
							
							it->StatActive(false);
							bl = true;
						}
						else if ( bl )
						{
							// messageをitへ集中させる

				
							if ( !dynamic_cast<D2DStatic*>( it.get()) )
							{
								parent_->SetCapture(it.get());	
								
								

								TRACE( L"ACTIVE %s \n", (LPCWSTR)nm );

								if ( nm == L"btn" )
								{
										int a = 0;
								}

																			
								it->StatActive(true);
								ret = 1;
								return ret;						
							}
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

void D2DControls::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		std::list<D2DControl*> ls;
	
		for(auto& it : controls_ )
			ls.push_back(it.get());
		
		for(auto& it : ls )
		{
			int st = it->GetStat();

			if ( !(st & STAT::DEAD ) )
				it->DestroyControl();
		}


		
		D2DControl::DestroyControl();
	}
}
#pragma endregion // D2DControl

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DButton::DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s )
{	
	FRectF rcb = rc.GetBorderRect();


	DrawRect( cxt.cxt, rcb, cxt.black, 1);

	FRectF rcc = rc.GetContentRect();

#ifdef _DEBUG
	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, rcc, s, cxt.gray );
	}
	else if ( (stat & CLICK) && (stat & DEBUG1)  )
		DrawButton( cxt.cxt, rcc, L"onclick", cxt.red );
	else if ( (stat & MOUSEMOVE) && (stat & DEBUG1)  )
		DrawButton( cxt.cxt, rcc, L"onmove", cxt.black );
	else if ( (stat & FOCUS) && (stat & DEBUG1)  )
		DrawButton( cxt.cxt, rcc, s, cxt.red );
	else
		DrawButton( cxt.cxt, rcc, s, cxt.black );

#else
	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, rcc, s, cxt.gray );
	}
	else
		DrawButton( cxt.cxt, rcc, s, cxt.black );

#endif

	

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

#ifdef _DEBUG
		
		UINT stat = ( dbg_ > 0 ? (stat_|DEBUG1) : stat_ );
		ondraw_( cxt, rc_.ZeroRect(), stat, name_ );		

#else
		ondraw_( cxt, rc_.ZeroRect(), stat_, name_ );		
#endif		
		if ( parent_->GetCapture() == this )
		{
			//FillRectF(cxt, rc_.ZeroRect().GetBorderRect(), cxt.halftone );
			cxt.cxt->FillRectangle( rc_.ZeroRect().GetBorderRect(), cxt.halftone );
		}		

		mat.PopTransform();
	}
}



LRESULT D2DButton::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

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
		case WM_D2D_CLOSE_CONTROL:
		{
			DestroyControl();

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DControlWindow::OnDraw( D2DContext& cxt, FRectF& zeroRc )
{
	FRectF rc2( zeroRc.right-10, zeroRc.top, zeroRc.right, zeroRc.top+10 );
	DrawFillRect( cxt, rc2, cxt.red,cxt.red,1);

	rc2.Offset( 0, 20 );
	rc2.bottom += 160;

	CComPtr<ID2D1SolidColorBrush> br;
	CreateSolidBrush( cxt.cxt, D2RGB(181,230,29), &br );
	DrawFillRect( cxt, rc2, br,br,1);
	

	CComPtr<ID2D1SolidColorBrush> br2;
	CreateSolidBrush( cxt.cxt, D2RGB(123,158,18), &br2 );
	rc2.Offset( 0, rc2.Size().height+ 20 );
	DrawFillRect( cxt, rc2, br2,br2,1);

	
}
LRESULT D2DControlWindow::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;
	
	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	if ( message == WM_PAINT || message == WM_D2D_PAINT )
	{		
		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
		mat.Offset( rc_.left, rc_.top );

		if (md_ != MD::MOVE)
			mat_ = mat; // 自座標(左上0,0)のmatrix
	}

	switch( message )
	{
		case WM_D2D_PAINT:
		case WM_PAINT:
		{						
			if ( iscapture_ && message == WM_PAINT )
			{
				
			}			
			else if (stat_ & VISIBLE )
			{
				D2DContext& cxt = d->cxt_;
				FRectF rc1 = rc_.ZeroRect();
				
				D2DRectFilter f(cxt, rc1);

				if ( stat_ & BORDER )
					DrawFillRect( cxt, rc1, d->cxt_.black, MakeBrsuh(cxt, brush_back_),   1.0f );
				else
					DrawFillRect( cxt, rc1, d->cxt_.transparent, MakeBrsuh(cxt,brush_back_),   1.0f );

				

				OnDraw( d->cxt_, rc1 );

				/*mat.PushTransform();
				mat.Offset( rc1.right-10, 400 );
				btn1_->OnDraw( d->cxt_ );
				mat.PopTransform();*/

				SendMessageAll(d,WM_PAINT,wParam,lParam);	
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FRectF rc1 = rc_.ZeroRect();
			FRectF rc2( rc1.right-10, rc1.top, rc1.right, rc1.top+10 );

			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			if ( rc2.PtInRect( pt ))
			{
				md_ = MD::MOVE;
				d->SetCapture( this, &pt );
				ret = 1;
				iscapture_ = true;

				MeToLast();
			}
			else if ( rc1.PtInRect(pt))
			{
				SendMessageAll(d,message,wParam,lParam);	
				MeToLast();
				ret = 1;
			}
		}
		break;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{			
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				md_= MD::NONE;
				ret = 1;
				iscapture_ = false;
			}
			
			FRectF rc1 = rc_.ZeroRect();
			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			if ( rc1.PtInRect(pt))
			{
				SendMessageAll(d,message,wParam,lParam);	
				ret = 1;
			}
				
		}
		break; 
		case WM_MOUSEMOVE:
		{
			//btn1_->WndProc( d, message, wParam, lParam );

			if (md_ == MD::MOVE )
			{
				FPointF pt3 = mat_.DPtoLP( FPointF(lParam));
				FPointF ptprev = d->CapturePoint( pt3 );
				rc_.Offset( pt3.x-ptprev.x, pt3.y-ptprev.y );

				d->redraw_ = 1;
				ret = 1;

			}
			else if (md_ == MD::NONE )
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
					SendMessageAll(d,message,wParam,lParam);
				
				}
				else if ( mouse_enter_ )
				{
					wParam = wParam | MK_F_MOUSE_LEAVE; // once only.
					SendMessageAll(d,message,wParam,lParam);				
					mouse_enter_ = false;
				}
			}
		}
		break;
		case WM_SIZE:
		{
			FSizeF sz(lParam);
			if ( rc_.Height() < 0 ) rc_.bottom = rc_.top + sz.height;
			if ( rc_.Width() < 0 ) rc_.right = rc_.left + sz.width;

			SendMessageAll(d,message,wParam,lParam);		
		}
		break;
		case WM_KEYDOWN:
		{
			

			if ( onkeydown_ )
				onkeydown_( wParam, false );

		}
		break;
		
		
		default:
			SendMessageAll(d,message,wParam,lParam);		
	}

	mat.PopTransform();
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DControlWindow::~D2DControlWindow()
{
	//delete btn1_;
}
void D2DControlWindow::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControls::CreateWindow( parent, pacontrol, rc, stat, name, id );

	md_ = MD::NONE;
	iscapture_ = false;

	brush_fore_ = D2RGB(0,0,0);
	brush_back_ = D2RGB(255,255,255 );
	

	/*btn1_ = new D2DButton();
	FRectF rc1( 0,0,10,200);
	btn1_->CreateWindow( parent, NULL, rc1, 1, L"b1" );

	btn1_->ondraw_ = [](D2DContext& cxt, const FRectFBoxModel& rc, int stat )
	{

		if ( stat & D2DButton::STAT::CLICK )
			DrawFillRect( cxt, rc, cxt.red, cxt.red ,1); 
		else
			DrawFillRect( cxt, rc, cxt.black, cxt.black ,1); 

	};*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );
 
	brush_fore_ = D2RGB(0,0,0);
	brush_back_ = D2RGB(255,255,255 );
	

	SetText( name );

}

void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, LPCWSTR name )
{
	CreateWindow( parent, pacontrol, rc, STAT::VISIBLE, name, -1 );
}
void D2DStatic::SetText( LPCWSTR str, int align )
{
	name_ = str;

	if ( align > -1 )
		alignment_ = align;

	SetFont( parent_->cxt_.cxtt.textformat );
}
void D2DStatic::SetFont( IDWriteTextFormat* fmt )
{
	FRectF rc = rc_.GetContentRect(); //Zero(); 
	rc = rc.ZeroRect();
	rc.left += 5;
	xassert( alignment_ == 0 ||alignment_ == 1 ||alignment_ == 2 );
	stext_.CreateLayoutEx( parent_->cxt_.cxtt,fmt, rc, name_, lstrlen(name_), alignment_ ); // 単行のみ
}

LRESULT D2DStatic::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( !IsThroughMessage(message))
		return 0;

	LRESULT ret = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			auto cxt = d->cxt_;
			D2DMatrix mat( cxt );				
			mat.PushTransform();
			mat_ = mat;

			//FRectF rc1 = rc_.GetBorderRect(); //.ZeroRect();
			//D2DRectFilter f(d->cxt_, rc1);

			//if ( stat_ & BORDER )
			//{				
			//	DrawFillRect( d->cxt_, rc1, d->cxt_.black, d->cxt_.transparent,   1.0f );
			//}

			FRectF rccnt = rc_.GetContentRect();
			mat.Offset( rccnt.left, rccnt.top );	

			auto br = MakeBrsuh(cxt,brush_fore_);
			stext_.DrawText( cxt, br );

			mat.PopTransform();
			
		}
		break;
		case WM_D2D_JS_SETTEXT:
			if ( id_ == wParam )
			{
				VARIANT* pv = (VARIANT*)lParam;
				if ( pv->vt == VT_BSTR )
				{
					SetText(pv->bstrVal);				
					ret = 1;
				}
			}
		break;
	}


	return ret;
}

///////////////////////
void D2DWaiter::DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, int pos )
{	
	
	FPointF pt((rc.left+rc.right)/2, (rc.top+rc.bottom)/2);
	float w = 100;
	int j = pos % 8;

	for( int i = 0; i < 8; i++ )
	{
		FPointF pt2;		
		pt2.y = pt.y + w*sin( 2*_PAI/8*i );
		pt2.x = pt.x + w*cos( 2*_PAI/8*i );		

		if ( i == j )
			FillCircle( cxt.cxt, pt2, 35, cxt.red );
		else
			DrawCircle( cxt.cxt, pt2, 35, cxt.red );
	}
	/*
	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, rc, s, cxt.gray );
	}
	else if ( stat & CLICK )
		DrawButton( cxt.cxt, rc, L"onclick", cxt.red );
	else if ( stat & MOUSEMOVE )
		DrawButton( cxt.cxt, rc, L"onmove", cxt.black );
	else
		DrawButton( cxt.cxt, rc, s, cxt.black );
	*/
}
void D2DWaiter::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent, pacontrol, rc, stat, name, id );
	ondraw_ = DrawContent;
	pos_ = -1;
	mt_ = NULL;
	

}
void D2DWaiter::OnDraw( D2DContext& cxt )
{
	if ( VISIBLE( stat_ ))
	{
		cxt.cxt->GetTransform( &mat_ );

		ondraw_( cxt, rc_.GetContentRect(), stat_, name_, pos_ );			
		
		
		
	}
}

void D2DWaiter::Start()
{
	MoveTargetEx* mt = new MoveTargetEx();
		
	mt->SetParameter( 0, 0, 7*50, 0, 10000 );
	
	DWORD st = ::timeGetTime();
	
		
	mt->Fire_ = [this](MoveTargetEx* mt, float x1, float y1)->bool
	{
		/*if ( pos_ == (int)x1 )
		{
			MSG msg;
			while(::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}

			return false;
		}*/

		pos_ = (int)x1;		
		return true;
	};
		
	mt->End_ = [this](MoveTargetEx* mt, float x, float y)
	{		
		
		/*
						
		*/

		mt_ = NULL;
		delete mt; 
		// 自ら削除されるのでこれ以降記述は無効。		
	};		
	mt->Start(parent_);


	mt_ = mt;
}

LRESULT D2DWaiter::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{			
			OnDraw( d->cxt_  );		

			if ( pos_ < 0 )
			{
				pos_ = 0;
				Start();
			}
					
		}
		break;
		case WM_DESTROY:
		{
		   delete (MoveTargetEx*) mt_;

		}

		break;
	}
	return 0;
}
////////////////////////////////////////////////////////

void D2DButtonThreeState::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DButton::CreateWindow( parent, pacontrol,rc,stat,name,id );
	
	//ondraw_ = DrawContent;
	bpushed_ = false;
	rad_ = _PAI/2;

}
LRESULT  D2DButtonThreeState::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{
			if ( VISIBLE( stat_ ))
			{
				//D2DMat mat2;
				//d->cxt_.cxt->GetTransform( &mat2 );

				D2DMatrix mat(d->cxt_);
				mat.PushTransform();
				mat_ = mat;


				
				FPointF pt = rc_.CenterPt();
				mat.Offset( pt.x, pt.y );

				D2DMat mr,mr2;
				mr.Rotate( rad_ );		

				D2DMat::Multi( mr2, mr, mat );
							
				//mr2.SetTransform();
				d->cxt_.cxt->SetTransform( &mr2 );
				
				FRectF rc(-5,-5,5,5);
				
				FPointF ptx[3];
				ptx[0] = FPointF( -5, 5 );
				ptx[1] = FPointF( -5, -5 );
				ptx[2] = FPointF( 5, 0 );

				FillPolygon( d->cxt_.cxt,  ptx, 3, d->cxt_.black );
				DrawRect( d->cxt_.cxt, rc, 	d->cxt_.black, 1 );
				mat.PopTransform();

				
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if (ENABLE(stat_))
			{
				FPointF pt = mat_.DPtoLP( FPointF(lParam));
				if ( rc_.PtInRect( pt ) )
				{
					bpushed_ = !bpushed_; 

					
					MoveTargetEx* mt = new MoveTargetEx();

					if ( !bpushed_ )
						mt->SetParameter( 0,0,_PAI/2.0f,0, 600 );
					else
						mt->SetParameter( _PAI/2.0f,0,0,0, 600 );

					mt->Fire_ = [this](MoveTargetEx* m, float rad, float y)->bool
					{
						rad_ = rad;
						
						return true;

					};
					mt->End_ = [this](MoveTargetEx* m, float rad, float y)
					{
						rad_ = rad;
						delete m;
					};


					mt->Start( d );

					if ( OnClick_ )
						OnClick_( this );

				}
			}
		}
		break;
	}



	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DSlider::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControls::CreateWindow( parent,pacontrol,rc,stat,name,id);

	max_= 100;
	min_=  0; 
	pos_ = 50;	
}
LRESULT  D2DSlider::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret=0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);
			mat.PushTransform();
			auto cxt = d->cxt_;

			DrawFillRect( d->cxt_, rc_.GetContentRect(), cxt.black, cxt.white, 1 );

			const FRectF rc1 = rc_.GetContentRectZero();

			mat.Offset( rc_.left, rc_.top );
			mat_ = mat; // rc_の左上が原点

			

			FRectF rc = rc1; 
			FPointF pt = rc.CenterPt();
			rc.top = pt.y - 2;
			rc.bottom = pt.y + 2;

			DrawRect( cxt, rc, cxt.black, 1 ); 

			rc = rc1;

			float rto = pos_ / (max_ - min_);
			
			pt.x = (rc.right - rc.left )*rto + rc.left;
			rc.left = pt.x-3;
			rc.right = pt.x+3;

			DrawFill( cxt, rc, cxt.black );

			bar_ = rc;

			SendMessageAll( d, message, wParam,lParam );

			mat.PopTransform();
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( bar_.PtInRect( pt ) ) // if ( rc_.GetZeroRect().PtInRect( pt1 ) )
			{
				d->SetCapture(this, &pt );

				ATLTRACE( L"slilder id=%d\n", id_ );

				ret = 1;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( d->GetCapture() == this )
			{				
				FPointF pt = mat_.DPtoLP( FPointF(lParam));
				FPointF prev = d->CapturePoint(pt);
							
				float off2 = (pt.x - prev.x) / rc_.GetContentRect().Width()*100.0f;
				pos_ = min( 100, max( 0, pos_ + off2));
				
				PostMessage( parent_->hWnd_, WM_D2D_EVSLIDER_CHEANGED, (WPARAM)this, (LPARAM)&pos_ );

				d->redraw_ = 1;
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


	return ret;


}




void DrawResizMark( D2DContext& cxt, const FRectF& rc )
{
	//D2DRectFilter f(cxt, rc); 

	D2DMatrix mat(cxt);
	mat.PushTransform();
	mat.Offset( rc.right,rc.bottom);

	CComPtr<ID2D1PathGeometry> pathg;
	CComPtr<ID2D1PathGeometry> pathg2;
		
	{

		CComPtr<ID2D1Factory> factory;
		cxt.cxt->GetFactory(&factory);
		
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

	cxt.cxt->FillGeometry( pathg, cxt.ltgray );
	cxt.cxt->FillGeometry(pathg2, cxt.white);

	mat.PopTransform();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DControlsEx::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 )
{
	FRectFBoxModel nrc = rc;
	if ( nrc.Margin_.l == 0 )
		nrc.Margin_.Set(1);
	
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

	if ( !IsThroughMessage(message))
		return 0;
	
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
				DrawFill( d->cxt_, rc1, d->cxt_.red );
			else
				DrawFill( d->cxt_, rc1, d->cxt_.white );

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


	//if ( message == WM_PAINT || message == WM_MOUSEMOVE )
	//	dp_rc_ = mat.LPtoDP(rc_);

	mat.PopTransform();
	return ret;

}
LRESULT D2DControlsEx::WndProc0(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;
	
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
			
					
			FRectF rc1 = rc_.ZeroRect();

			rc1 = rc_.GetBorderRectZero();
						
			rc1.Offset( 0, titlebar_height_  );					
			rc1.bottom -= titlebar_height_;
				
			FRectF rc11 = rc1;
			rc11.top -= rc_.BoderWidth_;
			
			// Draw window frame
			DrawFillRect( d->cxt_, rc11, d->cxt_.gray, d->cxt_.white, rc_.BoderWidth_ ); // 1.0f );

							
			// タイトルバー			
			auto it = controls_.begin();
			{
				(*it)->WndProc( d, message, wParam,lParam); // draw title bar
				it++;
			}


			D2DRectFilter f2(d->cxt_, rc_.GetContentRectZero());
			mat.Offset( 0, titlebar_height_ );
			{
				for( ;it != controls_.end(); ++it )
				{
					auto obj = (*it).get();
					obj->WndProc(d,message,wParam,lParam);
				}
			}

			// Draw resize mark. right-bottom.
			mat.Offset( 0, -titlebar_height_ );
			FRectF rc2( rc1.right-30,rc1.bottom-30,rc1.right,rc1.bottom );
			DrawResizMark( d->cxt_, rc2 );
	
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
			//if ( d->GetCapture() == this )
			//{
			//	d->ReleaseCapture();
			//}
			//else
			md_ = 0;
				SendMessageAll(d,message,wParam,lParam);	
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

				rc_.right += (pt.x - ptprv.x)*mat_._11 ; //, 
				rc_.bottom += (pt.y - ptprv.y)*mat_._22; // 


				D2DTitleBar* bar = (D2DTitleBar*)controls_[0].get();
				
				FRectFBoxModel rc1 = bar->GetRect();

				rc1.right = rc1.left + rc_.GetBorderRect().Width();

				bar->SetRect( rc1 );
				
				
				//titlebar_.width = rc_.GetBorderRect().Width();


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
/*
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

				TRACE(L"----------------------------------\n" );
			}
*/
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

		MoveTargetEx* mt = new MoveTargetEx();

		mt->SetParameter( mat2_._11, mat2_._22, 0.3f, 0.3f, 500 ); // 100 / sz.width,100 / sz.width, 500 );

		mt->Fire_ = [this](MoveTargetEx* p, float a, float b)->bool{

			mat2_._11 = a;
			mat2_._22 = b;
			
			return true;

		};

		mt->End_ = [this](MoveTargetEx* p, float a, float b){
			
			delete p;
		};
		
		mt->Start(parent_);

		
		

		bExpand_ = bExpand;
	}
	else
	{
		
		MoveTargetEx* mt = new MoveTargetEx();

		mt->SetParameter( mat2_._11, mat2_._22, 1,1 , 500 );

		mt->Fire_ = [this](MoveTargetEx* p, float a, float b)->bool{

			mat2_._11 = a;
			mat2_._22 = b;
			
			return true;

		};

		mt->End_ = [this](MoveTargetEx* p, float a, float b){
			
			mat2_._11 = 1;
			mat2_._22 = 1;
			delete p;
		};
		
		mt->Start(parent_);

		bExpand_ = bExpand;

	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D2DPlacementGrid::D2DPlacementGrid():scale_(1)
{

}

void D2DPlacementGrid::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );

	brush_border_ = D2RGB(0,0,0);
	clr_ = D2RGB(255,255,255);
	scale_ = 1;
	recalc_ = false;

}


LRESULT D2DPlacementGrid::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{
			if ( recalc_ )
			{
				PlaceEnd();
				recalc_ = false;
			}

			D2DContext& cxt = d->cxt_;				
			D2DMatrix mat( cxt );			
				
			mat.PushTransform(); 
			mat.Offset( rc_.left, rc_.top );


			mat.Scale( scale_, scale_ );

			mat_ = mat;
		
			FRectF rc1 = rc_.GetBorderRect().ZeroRect();

			if ( stat_ & BORDER )
				DrawFillRect( cxt, rc1, MakeBrsuh(cxt, brush_border_) , d->cxt_.ltgray,   1.0f );
			else
				DrawFillRect( cxt, rc1, cxt.transparent , MakeBrsuh(cxt, clr_),   1.0f );

			if ( stat_ & BORDER )
			{
				cxt.SetAntiAlias(false);
		
				float prv = 0;
				for( int i = 0; i < widths_.count(); i++ )
				{
					float x = prv + widths_[i];
					DrawLine( d->cxt_.cxt, FPointF(x,rc1.top), FPointF(x,rc1.bottom),1,d->cxt_.gray, NULL );
					prv += widths_[i];;
				}
				cxt.SetAntiAlias(true);
			}
		
			// drawing child controls.
			for( auto& it : ar_ )
			{
				//DrawRect(cxt, it.rc,cxt.black,1);
				
				
				mat.PushTransform();
				mat.Offset( it.rc.left,it.rc.top ); // 原点移動
				it.ctrl->WndProc(d,message,wParam,lParam);

				mat.PopTransform();
			}

//DrawDebugCross(cxt,cxt.black);
			mat.PopTransform();
	}
	break;

	case WM_D2D_SIZE_FITTING:
	case WM_SIZE:
	{
		if ( auto_resize_ )
		{
			recalc_ = true;

			rc_ = parent_control_->GetContentRect().ZeroRect();

		}
		for( auto& it : ar_ )
		{
			ret = (it.ctrl->WndProc(d,message,wParam,lParam));
				
		}
		recalc_ = true;
	}
	break;

	default :
		for( auto& it : ar_ )
		{
			ret = (it.ctrl->WndProc(d,message,wParam,lParam));
				
		}
	}

	return ret;
}

void D2DPlacementGrid::PlaceBegin()
{
	TRACE( L"D2DPlacementGrid::PlaceBegin()xxxxxxx\n");
}


void D2DPlacementGrid::Place( UINT row, UINT col, D2DControl* child )
{
	xassert( child->parent_control_ == this );

	Item m;
	m.row = row;
	m.col = col;
	m.ctrl = child;

	ar_.push_back(m);
}
void D2DPlacementGrid::PlaceEnd()
{
	UINT mrow=0,mcol=0;
	
	for( auto& it : ar_ )
	{
		mrow = max(mrow, it.row );
		mcol = max(mcol, it.col );
	}

	// 単体セルの大きさ
	float cell_width1 = rc_.Width() / (mcol+1);
	float cell_height = rc_.Height() / (mrow+1);


	int mc = max((int)mcol+1, widths_.count());


	std::unique_ptr<FRectF> frc( new FRectF[ mc ] );
	FRectF* pcell_width = frc.get(); // rc_.Width() / (mcol+1);

	widths_.clear();
	widths_.uniqeVaule( mc, cell_width1 );
	

	int i;
	float prv = 0;
	for( i = 0; i < widths_.count(); i++ )
	{
		pcell_width[i].left = prv;
		pcell_width[i].right = prv + widths_.get()[i];
		pcell_width[i].top = 0;
		pcell_width[i].bottom = 0;

		prv += widths_.get()[i];
	}
	
	for( auto& it : ar_ )
	{
		// center配置
		 
		float h = it.ctrl->GetRect().Height();
		float offy = (cell_height - h )/2.0f;


		float w = pcell_width[ it.col ].Width();
		float x = it.ctrl->GetRect().Width();
		float offx = (w - x )/2.0f;
		
		x = pcell_width[ it.col ].left;

		// it.rcは表示時に参照するだけ
		FRectF rck( x +offx, cell_height * it.row+offy,FSizeF(w,cell_height));
		it.rc = rck;
		
	}	
}
void D2DPlacementGrid::SetWidth( std::vector<float>& ar )
{
	widths_ = ar;

}
void D2DPlacementGrid::SetScale( float rto )
{
	scale_ = rto;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DSheetWindow::D2DSheetWindow():brush_border_(0),brush_back_(0)
{
	bSmall_ = true;
	small_rc_.SetRect(0,0,100,100);
}
LRESULT D2DSheetWindow::WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( bSmall_ )
		return WndProcSmall(parent,message,wParam,lParam );
	else
		return WndProcNormal(parent,message,wParam,lParam );
}
void D2DSheetWindow::CreateWindow( D2DWindow* d, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControls::CreateWindow(d,pacontrol,rc,stat,name,id);

	xassert( dynamic_cast<D2DBookWindow*>( pacontrol ));


	brush_border_ = D2RGB(0,0,0);
	brush_back_ = D2RGB(255,255,255 );
	
	md_ = MD::NONE;

	small_rc_.MoveCenter( rc );

}

void DrawControlBorder( D2DContext& cxt, FRectF& rc, int stat, ID2D1Brush* border, ID2D1Brush* back )
{
	// DrawControlBorder(d->cxt_, rc1, stat_, brush_border_, brush_back_ );

	if ( stat & BORDER )
		DrawFillRect( cxt, rc, border, back,   1.0f );
	else
		DrawFillRect( cxt, rc, cxt.transparent, back,   1.0f );

}
LRESULT D2DSheetWindow::WndProcNormal(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( !IsThroughMessage(message))
		return 0;

	if ( message == WM_PAINT )
	{		
		//FRectF rc1 = rc_;

		D2DContext& cxt = d->cxt_;
		//D2DMatrix mat(cxt);
		//mat.PushTransform();
		//mat_ = mat; // 親が原点

		//
		//FRectF rcc = rc1;
		//rcc.InflateRect(-1,0);
		//
		//DrawControlBorder(cxt, rc1, stat_, brush_border_, brush_back_ );
		//D2DRectFilter f(cxt, rcc);
		//
		//FRectF rc11 = Reactive();
		
		
		RECT rc;
		::GetClientRect(d->hWnd_,&rc);

		FRectF rc11(0,0,rc.right,rc.bottom);
		DrawFill( cxt, rc11, cxt.white );
		
		rc_ = rc11;

		//mat.PopTransform();
		
		
		D2DControls::WndProc(d, message, wParam, lParam);
	}
	
	else if ( message == WM_LBUTTONDBLCLK )
	{
		//FPointF pt = mat_.DPtoLP( FPointF(lParam));
		//FRectF rc1 = rc_.ZeroRect();
		//if ( rc1.PtInRect(pt))
		{
			bSmall_ = true;
			d->redraw_ = 1;

			D2DBookWindow* p = dynamic_cast<D2DBookWindow*>(parent_control_);
			p->SetActive(NULL);

			ret = 1;
		}
	}
	else if ( message == WM_MOUSEMOVE )
	{
		ret = SendMessageReverseAll(d, message, wParam, lParam);		
	}
	else
		return D2DControls::WndProc(d, message, wParam, lParam);
	
	return ret;
}

LRESULT D2DSheetWindow::WndProcSmall(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( (stat_ & DISABLE) && message != WM_PAINT )
		return ret;

	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	if ( message == WM_PAINT || message == WM_D2D_PAINT )
	{		
		//mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
		mat.Offset( small_rc_.left, small_rc_.top );

		if (md_ != MD::MOVE)
			mat_ = mat; // 自座標(左上0,0)のmatrix
	}



	switch( message )
	{
		case WM_PAINT:
		{
			if (stat_ & VISIBLE )
			{
				FRectF rc1 = small_rc_.ZeroRect();
				
				auto& cxt = d->cxt_;

				D2DRectFilter f(d->cxt_, rc1);

				
				DrawControlBorder(d->cxt_, rc1, stat_, MakeBrsuh(cxt,brush_border_), MakeBrsuh(cxt,brush_back_) );

				
				V4::DrawCenterText( d->cxt_.cxt,d->cxt_.black,rc1, name_, name_.length(), 1);
			}
		}
		break;
		case WM_LBUTTONDBLCLK :
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			FRectF rc1 = small_rc_.ZeroRect();
			if ( rc1.PtInRect(pt))
			{
				bSmall_ = false;
				d->redraw_ = 1;

				D2DBookWindow* p = dynamic_cast<D2DBookWindow*>(this->parent_control_);
				p->SetActive(this);
				
			}
			else if ( !bSmall_ )
				ret = 1;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			FRectF rc1 = small_rc_.ZeroRect();
			if ( rc1.PtInRect(pt))
			{
				md_ = MD::MOVE;
				d->SetCapture( this, &pt );
				ret = 1;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (md_ == MD::MOVE )
			{
				FPointF pt3 = mat_.DPtoLP( FPointF(lParam));
				FPointF ptprev = d->CapturePoint( pt3 );
				small_rc_.Offset( pt3.x-ptprev.x, pt3.y-ptprev.y );

				d->redraw_ = 1;
				ret = 1;

			}
		}
		break;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				md_= MD::NONE;
				ret = 1;				

				

				rc_.MoveCenter( small_rc_ );
			}
		}
		break;
		
	}
	mat.PopTransform();
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////
D2DBookWindow::D2DBookWindow():brush_border_(0),brush_back_(0)
{
	active_ = NULL;
}
LRESULT D2DBookWindow::WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( active_ )
		ret = active_->WndProc(parent,message,wParam,lParam);
	else
		ret = D2DControls::WndProc(parent,message,wParam,lParam);

	return ret;
}
void D2DBookWindow::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id);

}

void D2DBookWindow::SetActive(D2DSheetWindow* p)
{
	if ( p )
	{
		xassert(p->parent_control_ == this );
	}

	active_ = p;
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

D2DControlsWithScrollbar::D2DControlsWithScrollbar()
{

}
D2DControlsWithScrollbar::~D2DControlsWithScrollbar()
{
	int a = 0;
}

void D2DControlsWithScrollbar::ShowScrollbar( TYP typ, bool visible )
{
	if ( typ == TYP::VSCROLLBAR )		
		Vscbar_->Show( visible );
	else if ( typ == TYP::HSCROLLBAR )		
		Hscbar_->Show( visible );
	
}
LRESULT D2DControlsWithScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	bool bProcess = true;

	if ( !IsThroughMessage(message))
		return 0;

	D2DContext& cxt = d->cxt_;

	D2DMatrix mat(cxt);	
		
	mat.PushTransform();
	mat.Scale(scale_,scale_);
	
	switch( message )
	{
		case WM_PAINT:
		{			

			//cxt.SetAntiAlias(false);

			FRectF rcborder = rc_.GetBorderRect();
			
			auto br = ( stat_ & BORDER ? cxt.black : cxt.transparent  );

			DrawFillRect( cxt, rcborder, br, cxt.white,1);
			
			FRectF rc = rc_.GetContentRect();
			D2DRectFilter f(cxt, rc );
			
			mat.Offset( rcborder.left, rcborder.top );		
			mat_ = mat; // 自座標(左上0,0)のmatrix
			
			mat.PushTransform();
			{
				mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
						
				LRESULT r = 0;
				auto it = controls_.begin();
				
				_ASSERT((*it).get() == Vscbar_ );			
					
				it++; // pass Vscbar_
				it++; // pass Hscbar_

				for(; it!= controls_.end(); ++it ){ 
					(*it)->WndProc(d,message,wParam,lParam); 
				}
				
			}
			mat.PopTransform();

			bProcess = false;

			//cxt.SetAntiAlias(true);

			Vscbar_->WndProc(d,message,wParam,lParam);
			Hscbar_->WndProc(d,message,wParam,lParam);
		}
		break;

		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			pt.x += rc_.left;
			pt.y += rc_.top;

			if ( rc_.PtInRect( pt ))				
				ret = D2DControls::WndProc(d,message,wParam,lParam);
		
			bProcess = false;

		}
		break;				
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			pt.x += rc_.left;
			pt.y += rc_.top;

			if ( rc_.PtInRect( pt ))				
				ret = D2DControls::WndProc(d,message,wParam,lParam);
		
			bProcess = false;
		}
		break;

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
			{				
				ret = D2DControls::WndProc(d,message,wParam,lParam);
				bProcess = false;
			}
		}
		break;
		case WM_D2D_OBJECT_UPDATE :
		{
			// Dropされたobjectを受け入れるか。
			if ( bDropped_object_accept_ )
			{
				D2D_OBJECT_UPDATE* pm = (D2D_OBJECT_UPDATE*)lParam;
				
				FPointF pt = mat_.DPtoLP(pm->globalpt);
				pt.x += rc_.left;
				pt.y += rc_.top;

				D2DControl* pc = (D2DControl*)pm->object;

				// 自分の領域に落とされた場合
				if ( rc_.PtInRect( pt ))
				{
					if ( this != pc->parent_control_ )
					{
						OnDropObject( pc );

						if ( pm->stat == D2D_OBJECT_UPDATE::TYP::TRY_NEWPARENT )
							pm->stat = D2D_OBJECT_UPDATE::TYP::SUCCESS;						
					}
					else
						pm->stat = D2D_OBJECT_UPDATE::TYP::MOVE;

					// return 1; 
					bProcess = true;

				}				
			}
		}
		break;
		case WM_SIZE:
		{			
			if ( auto_resize_ )
			{
				FRectF rc = parent_control_->GetContentRect();
				rc_ = rc.ZeroRect();

				Vscbar_->WndProc(d,message,wParam,lParam);
				Hscbar_->WndProc(d,message,wParam,lParam);
			}
		}
		break;
		case WM_D2D_EVSLIDER_CHEANGED:
		{
			D2DSlider* sl = (D2DSlider*)wParam;
			float pos = (float)lParam;

			scale_ = 2*(sl->pos_ / (sl->max_ - sl->min_));

			


			
			d->redraw_ = 1;
		}
		break;
	}



	if (bProcess && ret == 0 )
		ret = D2DControls::WndProc(d,message,wParam,lParam);
	

	mat.PopTransform();

	return ret;
}
void D2DControlsWithScrollbar::OnDropObject( D2DControl* xpc )
{	
	auto xpc2 = xpc->UnPack();

	if ( xpc2->parent_control_ != this )
	{
		int stat = xpc2->GetStat();
		stat &= ~VISIBLE; // WndProcをスルーするためにVISIBLEをとる
		xpc2->SetStat(stat);

		D2DDropedPack* pc = new D2DDropedPack();
		pc->CreateWindow( parent_, this, FRectF(0,0,50,50), VISIBLE, L"suicide" );
		pc->Pack( xpc2 );
	}
	else
	{
		// 元のparent_controlに戻ってきたので、元に戻す。
		
		int stat = (xpc2->GetStat() | VISIBLE);
		xpc2->SetStat(stat);
		FRectFBoxModel rc = xpc2->GetRect();


		rc.SetPoint(0,0);

		xpc2->SetRect(rc);

	}

}


void D2DControlsWithScrollbar::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name,int typ, int id )
{
	FRectFBoxModel rca1(rc);
	
	bDropped_object_accept_ = false;
	rca1.BoderWidth_ = 1;
	
	D2DControls::CreateWindow( parent, pacontrol,rca1, stat, name, id );

	int Vtyp = ( typ == 0 || typ == 1 ? VISIBLE : 0 );
	int Htyp = ( typ == 0 || typ == 2 ? VISIBLE : 0 );
	
	
	FRectFBoxModel xrc = rca1.GetContentRectZero();
	xrc.left = xrc.right - 18;


	if ( typ == 0 ) 
		xrc.bottom -= 18;
	Vscbar_ = new D2DScrollbar();
	Vscbar_->CreateWindow(parent,this,xrc,Vtyp,NONAME );
	Vscbar_->target_control_ = this;
	//Vscbar_->auto_resize_ = true;


	xrc = rca1.GetContentRectZero();
	xrc.top = xrc.bottom - 18;

	if ( typ == 0 ) 
		xrc.right -= 18;
	Hscbar_ = new D2DScrollbar();
	Hscbar_->CreateWindow(parent,this,xrc,Htyp,NONAME );
	Hscbar_->target_control_ = this;
	//Hscbar_->auto_resize_ = true;

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;
	scale_ = 1;

}
void D2DControlsWithScrollbar::SetTotalSize( float cx, float cy )
{
	if (cy > 0 )
	{
		cy = max(rc_.Height(), cy );	
		Vscbar_->SetTotalSize( cy );
	}


	if ( cx > 0 )
	{
		cx = max(rc_.Width(), cx );	
		Hscbar_->SetTotalSize( cx );
	}
}


void D2DControlsWithScrollbar::UpdateScrollbar(D2DScrollbar* bar)
{
	if ( bar->info_.bVertical )
	{
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	}
	else
	{
		scrollbar_off_.width = bar->info_.position / bar->info_.thumb_step_c;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////

LRESULT D2DMessageBox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	bool bprocess = true;

	switch( message )
	{
//		case WM_D2D_PAINT:
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);		
			mat.PushTransform();

			D2DContext& cxt = d->cxt_;
			V4::DrawFill(cxt,rc_, cxt.white);
			mat.Offset( rc_.left, rc_.top );
			mat_ = mat;
			Draw( cxt );
			mat.PopTransform();

			
		}
		break;
		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE )
			{				
				Close(IDCANCEL);
				ret = 1;
			}
			else if ( wParam == VK_RETURN )
			{
				Close(IDOK);
				ret = 1;
			}

		}
		break;
	}

	if ( bprocess )
		ret = D2DControls::WndProc(d,message,wParam,lParam);


	return ret;
}

void D2DMessageBox::DoModal( D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype )
{
	xassert( parent );
	
	
	HWND hWnd = parent->parent_->hWnd_;

	RECT rc;
	GetClientRect( hWnd, &rc );
	
	FRectF rcf( 0,0,320,200 );
	FRectF rcbtn(0,0,100,30);

	rcbtn.SetCenter( rcf );

	rcf.Offset( (rc.left+rc.right)/2, (rc.bottom+rc.top )/2 );

	D2DControls* pcs = parent->parent_control_;

	if ( pcs == NULL )
		pcs = dynamic_cast<D2DControls*>(parent);
	else
	{	
		while( pcs->parent_control_ )
		{
			pcs = pcs->parent_control_; // search top controls.
		}
	}
			
	CreateWindow( parent->parent_, pcs, rcf, VISIBLE,NONAME ); 

	
	rcbtn.SetRect( 45, 200-45, FSizeF(100,30));
	D2DButton* closebtn_ = new D2DButton();
	closebtn_->CreateWindow( parent_, this, rcbtn, VISIBLE|BORDER, L"close" );
	closebtn_->OnClick_ = [this](D2DButton* btn)
	{
		Close(IDOK); // 内部でReleaseCaptureされる
	};


	rcbtn.Offset( 130, 0 );
	D2DButton* cancelbtn_ = new D2DButton();
	cancelbtn_->CreateWindow( parent_, this, rcbtn, VISIBLE|BORDER, L"cancel" );
	cancelbtn_->OnClick_ = [this](D2DButton* btn)
	{
		Close(IDCANCEL); // 内部でReleaseCaptureされる
	};


	parent->parent_->SetCapture(this); // Capture!!

	title_ = title;
	message_ = msg;

	

	Result_ = 0;
}

// static function
D2DMessageBox* D2DMessageBox::ShowDialog( D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype )
{	
	D2DMessageBox* mb = new D2DMessageBox();
	mb->DoModal( parent , msg, title, btntype );
	parent->parent_->redraw_ = 1;
	return mb;
}
void D2DMessageBox::Close(int result)
{
	Result_ = result;

	if ( onclose_ )
	{
		onclose_(this, Result_);
	}
	
	DestroyControl(); 
}
void D2DMessageBox::Draw( D2DContext& cxt )
{
	FRectF rc = rc_.ZeroRect();

	V4::DrawFill(cxt, rc, cxt.gray);
	rc.InflateRect( -5,-5);
	V4::DrawFill(cxt, rc, cxt.white);


	FRectF rc2 = rc; //rc_.ZeroRect();
	rc2.bottom -= 60;
	rc2.left += 20;
	DrawCenterText( cxt.cxtt,cxt.black, rc2, message_, message_.length(), 0 );


	FRectF rc1 = rc;//rc_.ZeroRect();
	rc1.top = rc1.bottom-60;
	V4::DrawFill(cxt, rc1, cxt.ltgray);
}
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

	if ( !IsThroughMessage(message))
		return 0;

	if ( WM_D2D_BROADCAST_UPDATE == message )
	{
		for( auto& it : controls_ )
			it->WndProc( d, WM_D2D_BROADCAST_UPDATE, wParam,lParam);
		return ret;
	}

	
	switch( message )
	{
		case WM_D2D_RESTRUCT_RENDERTARGET:
			//CreateResource( wParam == 1);
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
		case WM_DESTROY:			
			SendMessageAll(d, message, wParam, lParam);
			Clear();
			return 0;
		break;						
	}

	

	bool bAllRound = false;

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			D2DRectFilter f(d->cxt_, rc_);	

			FRectF rc1 = rc_.GetPaddingRect();
			if ( BORDER(stat_))
				DrawFillRect(cxt, rc1, cxt.basegray, cxt.white, 1 );
			else			
				DrawFill(cxt, rc1, cxt.white );
			
			
			mat.Offset( rc_.left, rc_.top );

			rc1 = rc_.ZeroRect();
			
			CComPtr<ID2D1PathGeometry> pGeometry;

			FRectF tagrc = TagDrawRect();			
			rc1.bottom = tagrc.bottom + 3;
			CreateTagButtomGeometry( cxt.cxt, tagrc, rc1, &pGeometry );
		
			cxt.cxt->FillGeometry( pGeometry, d->cxt_.ltgray );
			OnTagDraw(d->cxt_ );		
				
			// draw active child.
			{
				mat.Offset(0,rc1.bottom);
				controls_[active_idx_]->WndProc(d,message,wParam,lParam);	
			}

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
			if ( scale_ & AUTO::HEIGHT || auto_resize_ )
			{
				//auto rr = parent_control_->GetRect();
				auto sz = parent_control_->GetRect().GetContentRect().Size();
				//auto sz2 = parent_control_->GetContentRect().Size();

				//sz.height = 200;


				rc_.bottom = rc_.top + sz.height;

				for(auto& it : controls_ )
				{
					auto p = it.get();
					FRectF rc = it.get()->GetRect();
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
					FRectF rc = it.get()->GetRect();
					rc.right = rc.left + sz.width;
					it.get()->SetRect(rc);
				}
			}

			bAllRound = true;
		}
		break;
		case WM_D2D_TAB_ACTIVE:
			SetActivePage( (int)lParam );
			bAllRound = true;
		break;				
	}


	if ( bAllRound )
		ret = SendMessageAll(d, message, wParam, lParam);
	else 
	{
		//if ( active_idx_ < controls_.size() && ret == 0 )
		_ASSERT( active_idx_ < controls_.size() );
		ret = controls_[active_idx_]->WndProc(d,message,wParam,lParam);
	}

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
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id );
	FRectFBoxModel rc1 = rc.GetContentRect();
	rc1 = rc1.GetContentRectZero();

	//rc1.top += TAGHEIGHT;

	D2DControlsWithScrollbar* b1 = new D2DControlsWithScrollbar();
	b1->CreateWindow( parent, this, rc1, VISIBLE, L"page1", 0 );
	b1->auto_resize_ = true;
	active_idx_ = 0;
	SetActivePage(active_idx_);
	scale_ = 0;
}
FRectF D2DTabControls::TagDrawRect()
{
	FSizeF tag(TAGWIDTH,TAGHEIGHT);
	FRectF rc(0,0,tag);
	FRectF ret;
	UINT k = 0;
	rc.bottom += 3;
	for( auto&it : controls_ )
	{		
		if ( k == active_idx_ )
			ret = rc;
			
		rc.Offset( tag.width, 0 );
		k++;
	}
	return ret;
}

void D2DTabControls::OnTagDraw( D2DContext& cxt )
{
	FSizeF tag(TAGWIDTH,TAGHEIGHT);
	
	FRectF rc(0,0,tag);

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
		
		V4::D2DrawCenterText( cxt, br, rc, it->GetName(), it->GetName().length(),1 );
		
		rc.Offset( tag.width, 0 );
	}
}
FRectF D2DTabControls::GetContentRect()
{
	FRectF rc(rc_); 

	rc.bottom -= TAGHEIGHT;	


	return rc;
}
D2DControls* D2DTabControls::Replace( UINT idx, LPCWSTR typ_name )
{
	//xassert( idx < controls_.size() );
	//
	//D2DControl* pc = controls_[idx].get();
	//pc->DestroyControl();


	//if ( wcscmp( typ_name,L"StackPanel"))
	//{
	//	auto rc = controls_[0]->GetRect();
	//
	//	D2DStackPanel* b2 = new D2DStackPanel();

	//	D2DPlacementGrid

	//	b2->CreateWindow( parent_, this, rc, VISIBLE, L"name", 0 );
	//	b2->auto_resize_ = true;
	//	
	//}

	return nullptr; // not implement

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace TSF { CTextEditorCtrl* GetTextEditorCtrl(); };

namespace V4 {

D2DStatic* CreateStatic( D2DControls* pacontrol, FSizeF sz, LPCWSTR text )
{
	D2DStatic* sc = new D2DStatic();
	FRectF rc(0,0,sz.width,sz.height);
	sc->SetAlignment(2);
	sc->CreateWindow(pacontrol->parent_,pacontrol,rc, text );
	return sc;
}
#ifndef TEXTBOXTEST
D2DTextbox* CreateTextbox( D2DControls* pacontrol, FSizeF sz, LPCWSTR text )
{
	D2DTextbox* tx = new D2DTextbox( TSF::GetTextEditorCtrl() );
	FRectF rc(0,0,sz.width,sz.height);
	tx->CreateWindow(pacontrol->parent_,pacontrol,rc,VISIBLE|BORDER,NONAME);	
	if ( text )
		tx->SetText(text);
	return tx;			
}
#endif
D2DPlacementGrid* CreateGrid(D2DControls* pacontrol, FSizeF sz )
{
	D2DPlacementGrid* grid = new D2DPlacementGrid();
	FRectF rc(0,0,sz.width,sz.height);
	grid->CreateWindow(pacontrol->parent_,pacontrol,rc,VISIBLE, NONAME );
	return grid;			
}
D2DButton* CreateButton( D2DControls* pacontrol, FSizeF sz, LPCWSTR text )
{
	D2DButton* sc = new D2DButton();
	FRectF rc(0,0,sz.width,sz.height);
	sc->CreateWindow(pacontrol->parent_,pacontrol,rc, VISIBLE|BORDER, text );
	return sc;
}
const D2DMessageBox* MessageBox( D2DControl* p, LPCWSTR msg, LPCWSTR title, int btn_type )
{
	D2DMessageBox* mb = new D2DMessageBox();		
	
	std::function<void(D2DMessageBox*,int)>  f = [](D2DMessageBox* p,int result)
	{
		::SendMessage( p->parent_->hWnd_, WM_D2D_MESSAGEBOX_CLOSED, (WPARAM)result, (LPARAM)p );
	};
	
	mb->onclose_ = f;
	mb->DoModal( p, msg, title, btn_type );
	return mb;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DDebugControl::OnCreate()
{
	filter_on_ = true;
	scale_on_ = false;

	CreateResource(true);
}
void D2DDebugControl::SetRect( const FRectF& rc )
{ 
	rc_.SetFRectF(rc); 
}
LRESULT D2DDebugControl::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_D2D_RESTRUCT_RENDERTARGET:
			CreateResource(wParam == 1);
		break;
		case WM_D2D_DESTROY_CONTROL:
		{
			if (this == (D2DDebugControl*)lParam)
			{
				CreateResource(false);
			}
		}
		break;
	}

	switch (message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat( d->cxt_ );				
			mat.PushTransform();
			D2DRectFilter f(d->cxt_, rc_);
			{
				if ( !filter_on_ ) 
					f.Off();
			
			
				//FRectF rc = rc_.GetContentRect();
				mat.Offset( rc_ );

				float sc = ( scale_on_ ? 0.7f : 1 );
				//mat.Offset( rc.left, rc.top );
				mat.Scale(sc,sc);
			
				mat_ = mat;

				OnDraw( d->cxt_  );			
			}
			mat.PopTransform();
			return 0;
		}
		break;		
		case WM_MOUSEMOVE:
		{
			gpt_ = FPointF(lParam);
			mpt_ = mat_.DPtoLP(lParam);
			mpt_.Offset( rc_.left, rc_.top );
			d->redraw_=1;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			mpt_ = mat_.DPtoLP(FPointF(lParam));
			mpt_.Offset( rc_.left, rc_.top );

			FRectF rc(rc_);
			rc.SetSize(10,10);
			FRectF rc2(rc); rc2.Offset(10,0);
			if ( rc.PtInRect(mpt_) )
			{
				filter_on_ = !filter_on_;
				ret = 1;
			}				
			else if ( rc2.PtInRect(mpt_) )
			{
				scale_on_ = !scale_on_;
				ret = 1;
			}

		}
		break;
		case WM_SIZE:
		{
			FSizeF sz(lParam);

			if ( auto_resize_ )
			{
				auto rc = parent_control_->GetRect();

				rc_.SetSize( rc.Size());
			}
		}
		break;
	}
	
	return ret;

}

void D2DDebugControl::DrawText( D2DContextText& cxt, const D2D1_RECT_F& rc, LPCWSTR str )
{
	
	cxt.cxt->cxt->DrawText( str, lstrlen(str), cxt.textformat, rc, cxt.cxt->black, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
}
void D2DDebugControl::OnDraw(D2DContext& cxt )
{
	if ( debug_draw_ )
	{
		debug_draw_(cxt);
		return;
	}
	
	CComPtr<ID2D1SolidColorBrush> waku, fill;
	cxt.cxt->CreateSolidColorBrush(wakuclr_, &waku);
	cxt.cxt->CreateSolidColorBrush(fillclr_, &fill);

	D2DMatrix mat( cxt );				
	mat.PushTransform();
	
	mat.Offset( rc_.Padding_.l+rc_.Margin_.l, rc_.Padding_.t+rc_.Margin_.t );
	{
		auto rc2 = rc_.GetContentRect().ZeroRect();


		bool enable = true;


		if ( stat_ & V4::STAT::BORDER )
		{
			if ( stat_ & V4::STAT::DISABLE )
			{	
				V4::DrawFillRect(cxt, rc2, waku, cxt.ltgray, 1);
				enable = false;
			}
			else
				V4::DrawFillRect(cxt, rc2, waku, fill, 1);
		}
		else
		{
			if ( stat_ & V4::STAT::DISABLE )
			{	
				DrawFill(cxt, rc2, cxt.ltgray);
				enable = false;
			}
			else
				DrawFill(cxt, rc2, fill);
		}
				

		FRectF rc(0,10,500,36);
		FString s = FString::Format( L"D2DDebugControl, pt=(%d,%d) sz=(%d,%d), mouse global=(%d,%d)", (int)rc_.left, (int)rc_.top, (int)rc_.Width(), (int)rc_.Height(),(int)gpt_.x, (int)gpt_.y );
		DrawText(cxt.cxtt,rc, s);
		
		rc.Offset(0,30);
		s = FString::Format( L"mouse local=(%d,%d) ", (int)(mpt_.x-rc_.left), (int)(mpt_.y-rc_.top) );
		DrawText(cxt.cxtt,rc, s);

		rc.Offset(0,30);
		s = FString::Format( L"scale=(%f,%f) ", mat_._11, mat_._22 );
		DrawText(cxt.cxtt,rc, s);

		rc.Offset(0,30);
		s = FString::Format( L"enable=%s ", (enable ? L"true": L"false"));
		DrawText(cxt.cxtt,rc, s);


		// 小さいボタン
		FRectF btn(0,0,10,10);
		DrawFill(cxt.cxt,btn,cxt.red);

		btn.Offset(10,0);
		DrawFill(cxt.cxt,btn,cxt.gray);
	}

	mat.PopTransform();
}
void D2DDebugControl::CreateResource(bool bCreate)
{
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DFloatMenu::D2DFloatMenu(FString* title, int titlecnt )
{
	for( int i = 0;  i < titlecnt; i++ )
	{
		Pack pc;
		pc.title = title[i];
		items_.push_back( pc );
	}
}
void D2DFloatMenu::OnCreate()
{
	float width = max(190,rc_.Width());
	
	FRectF rc(0,0,width, 30 );

	float h = 0;
	for( auto&it : items_ )
	{				
		h += rc.Height();
		it.rc = rc;
		rc.Offset(0,30);
	}
	
	rc_.bottom = rc_.top + h;
	rc_.right = rc_.left + width;

	selected_idx_ = -1;
	parent_->SetCapture(this);
}

LRESULT D2DFloatMenu::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ( !IsThroughMessage(message))
		return 0;

	switch (message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);
			mat.PushTransform();
			mat_ = mat; // 親コントロールが原点
			mat.Offset( rc_.left, rc_.top );
			{
				// 自座標　左上が(0,0)となる
				FRectF rc1 = rc_.ZeroRect();
				//D2DRectFilter f(cxt, rc1);		

				DrawFillRect(cxt, rc1,cxt.black,cxt.ltgray,1);
				
				int j = 0;
				for( auto&it : items_ )
				{										
					it.Draw(cxt);

					if ( selected_idx_ == j++ )
					{
						DrawFill(cxt, it.rc, cxt.halftone );
					}
				}

		
			}
			mat.PopTransform();
		}
		break;

		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				d->ReleaseCapture();

				if ( OnSelected_ && selected_idx_ != -1 )
					OnSelected_( this, selected_idx_ );

				DestroyControl();
				ret =1;

			}
			else if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				DestroyControl();

				ret = 1;
			}
		}
		break;
		case WM_MOUSEMOVE :
		{
			if ( d->GetCapture() == this )
			{
				FPointF pt = mat_.DPtoLP( FPointF(lParam));

				pt.x -= rc_.left;
				pt.y -= rc_.top;

				int j = 0;
				int selected_idx = selected_idx_;
				for( auto&it : items_ )
				{	
					if ( it.rc.PtInRect( pt ) )
					{
						selected_idx = j;
						break;
					}
					j++;
				}
				
				if ( selected_idx != selected_idx_ )
				{
					selected_idx_ = selected_idx;
					d->redraw_ = 1;
				}
				ret = 1;
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
				ret = 1;
		}

	}

	return ret;
}

void D2DFloatMenu::Pack::Draw(D2DContext& cxt )
{
	FRectF drc(rc);

	drc.left+= 30;
	
	DrawCenterText(cxt, cxt.black,drc, title, title.length(), 0 );

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTopControls::CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id )
{
	CreateWindow(d,NULL,FRectFBoxModel(0,0,-1,-1),stat,name,id );

}
void D2DTopControls::CreateWindow(D2DWindow* d, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	xassert( pacontrol == NULL );
	D2DControls::CreateWindow(d, pacontrol, rc, stat, name, id);
	
	zoom_ =1.0f;
	bAutoSize_ = true;

	back_.br = nullptr;
	back_.color = D2RGB(255,255,255 );
	guid_ = FString::NewGuid();
	
	CreateResource(true);
	
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
void D2DTopControls::OnCreate()
{
	WndProc(parent_,WM_D2D_APP_ON_CREATE, 0,0);		
}

void D2DTopControls::BackColor(D2D1_COLOR_F clr)
{
	back_.color = clr;
	back_.br = MakeBrsuh(parent_->cxt_, back_.color);

}

bool LoadImagex( LPCWSTR filename, ID2D1RenderTarget* target, ID2D1Bitmap** ret );

LRESULT D2DTopControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_D2D_RESTRUCT_RENDERTARGET:
			CreateResource( wParam == 1);
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
		case WM_DESTROY:			
			SendMessageAll(d, message, wParam, lParam);
			Clear();
			return 0;
		break;						
	}
	


	D2DMatrix mat(d->cxt_);
	mat.PushTransform();
	mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
	mat.Offset( rc_.left, rc_.top );

	mat._11 = zoom_ ;
	mat._22 = zoom_ ;
	
	mat.SetTransform();

	mat_ = mat; // 自座標のマット

	FPointF pt21 = mat.DPtoLP( FPointF(lParam));

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;

			auto rcx = rc_.ZeroRect();
			
			D2DRectFilter f(cxt, rcx );

			
			DrawFill(cxt,rcx, (ID2D1Brush*)back_.br);

#ifdef _SKEYBLUE_GRID
			RECT rc;
			GetClientRect(d->hWnd_,&rc);
			DrawSkyBlueGrid( cxt, FSizeF(rc.right,rc.bottom),50);
#endif
			
			// DrawDebugCross(d->cxt_, d->cxt_.black );

			


			SendMessageReverseAll(d,message,wParam,lParam);	

			#ifdef _DEBUG
				IDWriteTextFormat* fm = cxt.text;
				V4::DrawText( cxt, fm, L"D2DControlsMove", d->cxt_.red, FRectF(10,rc_.bottom-50,1000,rc_.bottom));
				
				V4::DrawText( cxt, fm, d->appinfo_.c_str(), d->cxt_.halftone, FRectF(10,rc_.bottom-20,1000,rc_.bottom));

				
				V4::DrawText( cxt, fm, error_msg_.c_str(), d->cxt_.halftoneRed, FRectF(0,0,1000,rc_.bottom));
			#endif
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
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
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
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
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
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
					SendMessageAll(d,message,wParam,lParam);
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (ENABLE(stat_))
			{
				//if ( rc_.ZeroRect().PtInRect( pt21 ))

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
			V4::MessageBox(this,L"hoi",L"hoi", MB_OK ); 
			ret = 1;
		break;
		case WM_D2D_RESTRUCT_RENDERTARGET:
		{
			CreateResource(wParam == 1);

			SendMessageAll(d, message, wParam, lParam);
		}
		break;



		default:
			if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);	

			if ( message <= WM_D2D_APP_USER )
			{
				if ( OnWndProcExt_ )
					ret = OnWndProcExt_(this,message,wParam,lParam);
			}

				
	}

	mat.PopTransform();
	return ret;
}
void D2DTopControls::CreateResource(bool bCreate)
{
	back_.br.Release();

	if ( bCreate )
	{
		back_.br = parent_->GetSolidColor( back_.color );
	}		
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DCheckbox::OnCreate()
{
	clr_fore_ = COLOR_DEF_FORE;
}
LRESULT D2DCheckbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ( !IsThroughMessage(message))
		return 0;

	switch (message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);
			mat.PushTransform();
			mat_ = mat; // 親コントロールが原点
			mat.Offset( rc_.left, rc_.top );
			{
				// 自座標　左上が(0,0)となる
				FRectF rc1 = rc_.ZeroRect();
				//D2DRectFilter f(cxt, rc1);		

				if (BORDER(stat_))
					DrawFillRect(cxt, rc1,cxt.black,cxt.ltgray,1);
				

				rc1.left += rc1.Height() + 2;
				V4::DrawCenterText( cxt.cxtt, cxt.black, rc1, name_, name_.length(), 0 );
				
				

				{
					FRectF rcb(0,0,rc1.Height(),rc1.Height());
					if ( bl_ )
					{					
						
						float off = ((rcb.Width() - 13) / 2);
						DrawRect(cxt,rcb,cxt.black,1);
						
						auto br2 = MakeBrsuh(cxt, clr_fore_);
						mat.Offset( off, off );
						V4::DrawCheckMark( cxt, br2 );
					}
					else
					{
						DrawRect(cxt,rcb,cxt.black,1);
					}
			}
			}
			mat.PopTransform();
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				FRectF rc1 = rc_;
				rc1.right = rc1.left + rc1.Height();
				
				if ( rc1.PtInRect(pt))
				{
					bl_ = !bl_;
					d->SetCapture(this);
					ret = 1;
				}
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				

				if ( onclick_ )
					onclick_(this );
			}
		}
		break;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
LRESULT D2DPlacementGridEx::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{
			if ( recalc_ )
			{
				PlaceEnd();
				recalc_ = false;
			}

			D2DContext& cxt = d->cxt_;				
			D2DMatrix mat( cxt );			
				
			mat.PushTransform(); 
			mat.Offset( rc_.left, rc_.top );


			mat.Scale( scale_, scale_ );

			mat_ = mat;
		
			FRectF rc1 = rc_.GetBorderRect().ZeroRect();

			if ( stat_ & BORDER )
			{
				DrawFillRect( d->cxt_, rc1, MakeBrsuh(d->cxt_,brush_border_) , d->cxt_.ltgray,   1.0f );
			}
			else
			{
				DrawFillRect( d->cxt_, rc1, cxt.transparent , MakeBrsuh(d->cxt_,clr_),   1.0f );
			}

			if ( stat_ & BORDER )
			{
				cxt.SetAntiAlias(false);
		
				float prv = 0;
				for( int i = 0; i < widths_.count(); i++ )
				{
					float x = prv + widths_[i];
					DrawLine( d->cxt_.cxt, FPointF(x,rc1.top), FPointF(x,rc1.bottom),1,d->cxt_.gray, NULL );
					prv += widths_[i];;
				}
				cxt.SetAntiAlias(true);
			}
		
			// drawing child controls.
			for( auto& it : arEx_ )
			{
				if ( stat_ & BORDER )
					DrawRect(cxt, it.rc,cxt.black,1);
								
				mat.PushTransform();
				mat.Offset( it.offpt.x,it.offpt.y ); // 原点移動

				it.ctrl->WndProc(d,message,wParam,lParam);

				mat.PopTransform();
			}

//DrawDebugCross(cxt,cxt.black);
			mat.PopTransform();
	}
	break;
	case WM_SIZE:
	{
		if ( auto_resize_ )
		{
			recalc_ = true;
			auto xx = parent_control_->GetContentRect();
			rc_ = parent_control_->GetContentRect().ZeroRect();

		}
		for( auto& it : arEx_ )
		{
			ret = (it.ctrl->WndProc(d,message,wParam,lParam));
				
		}

	}
	break;

	default :
		for( auto& it : arEx_ )
		{
			ret = (it.ctrl->WndProc(d,message,wParam,lParam));
				
		}
	}

	return ret;
}

void D2DPlacementGridEx::Place( D2DControl* child, PlaceFunc2 f )
{
	ItemEx item;
	item.ctrl = child;
	item.rc.SetEmpty();
	item.typ = CENTER_VCENTER;
	
	arEx_.push_back( item );

	m_[child]= f;
}
void D2DPlacementGridEx::PlaceEnd()
{
	for( auto& it : arEx_ )
	{
		auto x = it.ctrl;
		auto func = m_[x];		
		FRectF rc = func( rc_.GetContentRect(), it.typ );
		it.rc = rc;

		if ( it.ctrl-> IsAutoResize() )
			it.ctrl->SetRect( rc );	// parentのサイズと異なる場合がある。
		else
		{
			auto rc2 = it.ctrl->GetRect();
			rc2.SetPoint( rc.LeftTop());

			it.ctrl->SetRect(rc2);

		}
	}

	auto xxx = rc_.GetContentRect();

	for( auto& it : arEx_ )
	{
		// 配置

		FPointF p1 = it.rc.CenterPt();
		FPointF p2 = it.ctrl->GetRect().CenterPt();

		float offx = p1.x-p2.x;
		float offy = p1.y-p2.y;


		if ( it.typ != TYP::CENTER_VCENTER )
		{
			if ( it.typ & TYP::LEFT ) 
				offx = 0;
			else if ( it.typ & TYP::RIGHT ) 
				offx = it.rc.Width() - it.ctrl->GetRect().Size().width;

			if ( it.typ & TYP::TOP ) 
				offy = 0;
			else if ( it.typ & TYP::BOTTOM ) 
				offy = it.rc.Height() - it.ctrl->GetRect().Size().height;
		}
		
		it.offpt = FPointF( offx, offy );

		
	}	
}

void D2DControls::Lineup(bool vertical )
{
	FPointF pt(0,0);

	if (vertical)
	{
		// 垂直にならべる、stackpanel
		for(auto& it: controls_ )
		{
			FRectFBoxModel rc = it->GetRect();

			rc.SetPoint(pt);
			it->SetRect( rc );

			pt.y += rc.Height();
		}
	}
	else
	{
		// 水平にならべる、stackpanel
		for(auto& it: controls_ )
		{
			FRectFBoxModel rc = it->GetRect();

			rc.SetPoint(pt);
			it->SetRect( rc );

			pt.x += rc.Width();
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DMessageDialogbox
D2DMessageDialogbox::D2DMessageDialogbox(LPCWSTR msg) :msg_(msg), back_(D2RGB(0, 0, 0))
{
}
void D2DMessageDialogbox::ShowDialog(D2DWindow* parent, const FRectFBoxModel& rc, LPCWSTR name, int id)
{	
	CreateWindow(parent, parent->children_.get(), rc, STAT::VISIBLE, name, id);

}
void D2DMessageDialogbox::OnCreate()
{
	// Centering this in Window.
	RECT grc;
	::GetClientRect(parent_->hWnd_, &grc);
	int x = (grc.left + grc.right) / 2;
	int y = (grc.top + grc.bottom) / 2;

	float left = x - rc_.Width() / 2;
	float top = y - rc_.Height() / 2;

	rc_.SetRect(left, top, rc_.Size());


	//D2DButton* btn = new D2DButton();
	//FRectF rc(0, 10, FSizeF(80, 20));
	//btn->CreateWindow(parent_, this, rc, 1, L"close");

	//btn->onclick_ = [this](D2DButton*)
	//{
	//	this->Close();
	//};

	parent_->SetCapture(this);

	InitDraw();
		
}
LRESULT D2DMessageDialogbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;

	switch (message)
	{
		case WM_PAINT:
		{
			if (VISIBLE(stat_))
			{
				D2DMatrix mat(d->cxt_);
				mat.PushTransform();

				FRectF rc = rc_.ZeroRect();
				mat._31 = 0;
				mat._32 = 0;

				mat.SetTransform();
				mat.Offset(rc_.left, rc_.top);
				{
					D2DContextText tt;
					tt.Init(d->cxt_, DEFAULTFONT_HEIGHT_JP, DEFAULTFONT_JP);

					CComPtr<ID2D1SolidColorBrush> br;
					d->cxt_.cxt->CreateSolidColorBrush(back_, &br);

					DrawFillRect(d->cxt_, rc, br, br, 1);
					DrawCenterText(tt, d->cxt_.white, rc, msg_.c_str(), msg_.length(), 1);


					SendMessageReverseAll(d, message, wParam, lParam);
				}

				mat.PopTransform();
			}
		}
		break;
		
		case WM_KEYDOWN:
		{
			if (VK_ESCAPE == wParam)
			{

				Close();

			}

		}
		break;

		default:
			SendMessageAll(d, message, wParam, lParam);
			break;

		}

	return r;
}

void D2DMessageDialogbox::Close()
{
	_ASSERT(this == parent_->GetCapture());

	parent_->ReleaseCapture();

	DestroyControl();
}
void D2DMessageDialogbox::InitDraw()
{
	MoveTargetEx* m = new MoveTargetEx();

	m->SetParameter(0, 0, 255, 0, 1000);

	m->Fire_ = [this](MoveTargetEx* p, float a, float b)->bool {

		back_ = D2RGBA(a, 0, 0, a);

		parent_->redraw_ = 1;
		return true;
	};

	m->End_ = [this](MoveTargetEx* p, float a, float b){

		back_ = D2RGBA(255, 0, 0, a);

		delete p;
	};


	m->Start(parent_);
}

#pragma endregion


#pragma region D2DGroupButton
void D2DGroupButton::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id, int btncount )
{
	_ASSERT( 0 < btncount );
	
	D2DControl::CreateWindow( parent, pacontrol,rc,stat, name, id );
	btncnt_ = btncount;
	clickced_btn_idx_ = -1;
	titles_ = std::shared_ptr< std::wstring>( new std::wstring[btncnt_], std::default_delete<std::wstring[]>() );
	for( int i = 0; i < btncnt_; i++ )
		titles_.get()[i] = name;
	
	clr_fore_ = COLOR_DEF_FORE;

}
LRESULT D2DGroupButton::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);			
			mat_ = mat.PushTransform();			
			mat.Offset( rc_ );
			
			
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
				pt.x -= rc_.left;
				pt.y -= rc_.top;

				float base = rc_.Height() / btncnt_;				
				FRectF rc(0,0,base, base );
				for( int i = 0; i < btncnt_; i++ )
				{
					if ( rc.PtInRect( pt ) )
					{
						clickced_btn_idx_ = i;

						d->SetCapture(this);
						r = 1;
						break;
					}				
					rc.Offset( 0, base );				
				}					
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( d->GetCapture()==this)
			{				
				r = 1;
				
				d->ReleaseCapture();

				if ( rc_.PtInRect(pt))
				{
					pt.x -= rc_.left;
					pt.y -= rc_.top;

					float base = rc_.Height() / btncnt_;				
					FRectF rc(0,0,base, base );
					for( int i = 0; i < btncnt_; i++ )
					{
						if ( rc.PtInRect( pt ) )
						{
							clickced_btn_idx_ = i;
							break;
						}				
						rc.Offset( 0, base );				
					}					

					// click event
					
						
					parent_control_->WndProc( d, WM_D2D_BUTTON_CLICK, (WPARAM)this, clickced_btn_idx_ );

					
				}

				
			}
		}
		break;
	}
	
	return r;
}
void D2DGroupButton::SetTitle( int idx, LPCWSTR title )
{
	if ( idx < btncnt_	)
		titles_.get()[idx] = title;

}
void D2DGroupButton::DrawButton( D2DContext& cxt )
{	
	auto br1 = MakeBrsuh(cxt, D2RGBA(54,101,179,100));
	auto br2 = MakeBrsuh(cxt, clr_fore_ );

	float base = rc_.Height() / btncnt_;				


	FRectF rc(0,0,base, base );
	
	float dbase = base*0.8;

	for( int i = 0; i < btncnt_; i++ )
	{		
		FPointF pt = rc.CenterPt();
		
		if (clickced_btn_idx_ == i  )
		{			
			FillCircle( cxt, pt, dbase/2*0.5f , br2 );
			DrawCircle( cxt, pt, dbase/2 , br1);
		}
		else
			DrawCircle( cxt, pt, dbase/2 , br1);

		FRectF rct = rc;

		rct.left = rc.right + 5;
		rct.right = rct.left + rc_.Width();
		auto str = titles_.get()[i];

		D2DrawCenterText( cxt, cxt.black, rct, str.c_str(), str.length(), 0 );
		
		
		rc.Offset( 0, base );

	}
}

#pragma endregion
};






 