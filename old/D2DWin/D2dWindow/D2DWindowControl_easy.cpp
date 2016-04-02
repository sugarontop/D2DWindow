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
#include "D2DWin.h"
#include "D2DWindowMessage.h"
#include "D2DWindowControl_easy.h"
#include "D2DCommon.h"
#include "D2DWindowMessageStruct.h"
#include "MoveTarget.h"
#include "D2DWindowMenu.h"
using namespace V4;

#define MENU_ITEM_HEIGHT 24

#pragma region D2DControl
D2DControl::D2DControl()
{
	_ASSERT( _CrtIsValidHeapPointer(this)); // You must create this object on Heap.
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
void D2DControl::StatActive(bool bActive)
{
	if ( bActive )
		stat_ |= FOCUS;
	else
		stat_ &= ~FOCUS;
}
bool D2DControl::IsFixedSize() const
{
	DWORD dw = GetStat();
	bool fixw = ISFLAG_ON(dw,STAT::WIDTH_FIX);
	bool fixh = ISFLAG_ON(dw, STAT::HEIGHT_FIX);

	return ( fixw && fixh );
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

	if ( rc_.Width() > 0 )
		stat_ |= STAT::WIDTH_FIX;
	if (rc_.Height() > 0)
		stat_ |= STAT::HEIGHT_FIX;


	if (parent_control_)
	{
		parent_control_->controls_.push_back(std::shared_ptr<D2DControl>(this));
	}

	if ( stat_ & DROPACCEPTOR )
	{
		parent->drag_accepters_.push_back( this );

		stat_ &= ~DROPACCEPTOR;
	}

	if (rc.Width())
		stat_ |= STAT::WIDTH_FIX;
	if (rc.Height())
		stat_ |= STAT::HEIGHT_FIX;

//	CHDL id = parent_->chandle_.CreateControlHandle( this );
//	chdl_ = id;
}

void D2DControl::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		
//SendMessage( WM_D2D_DESTROY_CONTROL, 0, (LPARAM)this);
		
	
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;

		if ( parent_->GetCapture() == this )
			parent_->ReleaseCapture(); // 1階層目だけはcheck


		if ( parent_control_ )
		{
			auto p = parent_control_->Detach( this );

			D2DWindow::DeathObject d;
			d.target = p;
			d.is_alive_in_parentControl = false; // Deatchしたのでfalse;

			parent_->death_objects_.push_back(d);
		}

//		parent_->chandle_.DeleteControlHandle( chdl_ );
	}
}
void D2DControl::SafeDestroyControl()
{
	if (!(stat_ & STAT::DEAD))
	{
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;

		if (parent_control_)
		{	
			std::shared_ptr<D2DControl> me;
			for (auto& it : parent_control_->controls_)
			{
				if (it.get() == this)
				{
					me = it;
					break;
				}
			}


			D2DWindow::DeathObject d;
			d.target = me;
			d.is_alive_in_parentControl = true; // Deatchしていないのでtrue;
			parent_->death_objects_.push_back(d);
		}			
	}
}
void D2DControl::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	_variant_t v,r;
	if ( findParameterMap(prms, L"enable", v, r ))
	{
		r.ChangeType(VT_BOOL);
		Enable(( r.boolVal?true:false));
	}
	if ( findParameterMap(prms, L"visible", v, r ))
	{
		r.ChangeType(VT_BOOL);
		Visible(( r.boolVal?true:false));
	}
}

// static
LRESULT D2DControl::MouseSelectedMoveProc(D2DControl* p, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	
	D2DWindow* d = p->parent_;

	switch( message )
	{
		case WM_LBUTTONDOWN:
		{			
			auto rc = p->GetRect();

			static D2DMat mat = p->Mat();

			FPointF pt = mat.DPtoLP(lParam);

			if ( rc.PtInRect(pt))
			{					
				d->SetCapture(p, &pt, &mat);
				ret = 1;			
			}


		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( d->GetCapture() == p )
			{
				auto mat = d->CaptureMat();
				FPointF pt = mat.DPtoLP(lParam);
				FPointF ptprv = d->CapturePoint(pt);

				float offx = pt.x - ptprv.x; 
				float offy = pt.y - ptprv.y;


				auto rc = p->GetRect();
				rc.Offset( offx, offy );
				p->SetRect(rc);
				ret =1;
				d->redraw_ = 1;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == p )
			{
				d->ReleaseCapture();				
				ret = 1;
			}
		}
		break;
	}
	return ret;
}
// static
LRESULT D2DControl::MousePropertyMenuProc(D2DControl* p, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	
	D2DWindow* d = p->parent_;

	switch( message )
	{
		case WM_RBUTTONDOWN:
		{			
			auto rc = p->GetRect();

			static D2DMat mat = p->Mat();

			FPointF pt = mat.DPtoLP(lParam);

			if ( rc.PtInRect(pt))
			{					
				d->SetCapture(p, &pt, &mat);
				ret = 1;		
				
				
				// collect FloatMenuItems by WM_D2D_MAINFRAME_GET_FLOAT_MENU message.

				std::vector<FloatMenuItem> ar;

				::SendMessage( d->hMainFrame_, WM_D2D_MAINFRAME_GET_FLOAT_MENU, 0, (LPARAM)&ar );

				if ( ar.empty())
				{
					// sample
					FloatMenuItem x;
					x.enable= true;
					x.imgid = -1;
					x.keyboard = L"xxx";
					x.text = L"here is menu text";

					x.msg.message = WM_COMMAND;
					x.msg.wParam = MAKEWPARAM(99,0);
					x.msg.lParam = 0;
				
					for( int i = 0; i < 6; i++ )
					{
						WCHAR cb[256];
						wsprintf(cb,L"menu %d", i );
						x.text = cb;

						x.enable = ( i != 1 );
						x.msg.lParam = i;


						ar.push_back(x);
					}
				}
				
				if ( !ar.empty())
				{
					D1::D2DMenuItems* test = new D1::D2DMenuItems();
					test->CreateWindow( d,  p->parent_control_, pt, ar );
				}
				ret = 1;
			}

			
		}
		break;		
		case WM_RBUTTONUP:
		{
			if ( d->GetCapture() == p )
			{
				d->ReleaseCapture();				
				ret = 1;
			}
		}
		break;
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DControls::CreateWindow( D2DWindow* parent, D2DControls* paconrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	_ASSERT( parent );
	_ASSERT( paconrol != this );

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
	if (rc_.Width() > 0)
		stat_ |= STAT::WIDTH_FIX;
	if (rc_.Height() > 0)
		stat_ |= STAT::HEIGHT_FIX;

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

//	chdl_ = parent_->chandle_.CreateControlHandle( this );
	
	
	back_color_.color = D2RGBA(0, 0, 0, 0);
	border_color_.color = D2RGBA(0, 0, 0, 0);
	OnResutructRnderTarget(true);

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

void D2DControls::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);
		
	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
	}


	for (auto& it : controls_)
		it->OnResutructRnderTarget(bCreate);
}

LRESULT D2DControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	_ASSERT(message != WM_D2D_RESTRUCT_RENDERTARGET);

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
			if ( stat_ & BORDER )
			{
				FRectF rc1 = rc_.ZeroRect();				
				//DrawFillRect( cxt, rc1, (ID2D1Brush*)border_color_.br, (ID2D1Brush*)back_color_.br, 1.0f );

				DrawControlRect(cxt, rc1, (ID2D1Brush*) border_color_.br, (ID2D1Brush*) back_color_.br);

			}

			SendMessageReverseAll(d,message,wParam,lParam);	
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
			if (dynamic_cast<IUpdatar*>(parent_control_))
			{
				auto p = dynamic_cast<IUpdatar*>(parent_control_);
				p->RequestUpdate( this, WM_SIZE );
			}
			else
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
			}


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

	mat.PopTransform();
	return ret;
}
LRESULT D2DControls::SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	for( auto it = controls_.begin(); it!=controls_.end(); ++it )// 順番：表示奥のオブジェクトから前面のオブジェクトへ
	{
		auto obj = (*it).get();
		_ASSERT( obj != this );

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
		_ASSERT( obj != this );
		
		int size = controls_.size();


		r += obj->WndProc(parent,message,wParam,lParam);

		if (r != 0 && r != LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS)
			break;

		_ASSERT( size == controls_.size() ); // ここで落ちる場合は、r=1を返せ。またはループが間違っている。
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

							

							TRACE( L"%x UNACTIVE %s ", this, (LPCWSTR)nm );
							
							it->StatActive(false);
							bl = true;
						}
						else if ( bl )
						{
							// messageをitへ集中させる

				
							/*if ( !dynamic_cast<D2DStatic*>( it.get()) )
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
							}*/
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
void D2DControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");

	
}

#pragma endregion // D2DControl

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D2DTopControls::CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id )
{
	CreateWindow(d,NULL,FRectFBoxModel(0,0,-1,-1),stat,name,id );

}
void D2DTopControls::CreateWindow(D2DWindow* d, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	_ASSERT( pacontrol == NULL );
	D2DControls::CreateWindow(d, pacontrol, rc, stat, name, id);
	
	zoom_ =1.0f;
	bAutoSize_ = true;
	back_color_.color = D2RGB(255,255,255 );
	back_color_.br = parent_->GetSolidColor(back_color_.color);
			
	
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
	//WndProc(parent_,WM_D2D_APP_ON_CREATE, 0,0);		
}

void D2DTopControls::BackColor(D2D1_COLOR_F clr)
{
	back_color_.color = clr;
	back_color_.br = parent_->GetSolidColor(back_color_.color);
	
}

bool LoadImagex( LPCWSTR filename, ID2D1RenderTarget* target, ID2D1Bitmap** ret );

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

	mat_ = mat; // 自座標のマット

	FPointF pt21 = mat.DPtoLP( FPointF(lParam));

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;

			cxt.cxt->FillRectangle(rc_,	(ID2D1Brush*) back_color_.br );
			
			SendMessageReverseAll(d,message,wParam,lParam);	

			#ifdef _DEBUG
				IDWriteTextFormat* fm = cxt.text;

				FString s = FString::Format( L"D2DControlsMove w=%6.2f,h=%6.2f", rc_.Size().width, rc_.Size().height);//小数点を入れて 6桁（小数点以下 2桁）
				cxt.cxt->DrawText( s.c_str(), s.length(), fm, FRectF(10,rc_.bottom-50,1000,rc_.bottom), d->cxt_.red);
			#endif
		}
		break;	
		/*case WM_D2D_JS_ERROR:
		{
			error_msg_ = (BSTR)lParam;

			ret = 1;
			

		}
		break;*/
		
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
void D2DTopControls::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);

	}
	else
	{
		back_color_.br = nullptr;
	}


	for (auto& it : controls_)
		it->OnResutructRnderTarget(bCreate);
}
void D2DTopControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);

	for (auto& it : prms)
	{
		auto& key = it.first;
		if (key == L"backcolor")
		{
			back_color_.color = VariantColor(it.second);
			back_color_.br = parent_->GetSolidColor(back_color_.color);


		}
		/*else if (key == L"bordercolor")
		{
			backclr_ = VariantColor(it.second);
		}
		else if (key == L"moveable")
		{
			move_able_ = (VariantBOOL(it.second) == true ? 1 : 0);
		}*/
	}

}


///////////


void D2DButton::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);
	
}
void D2DButton::DrawRadioButton(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool bChecked)
{
	FRectF rcb = rc.GetBorderRect();

	FRectF rcchk = rcb;
	rcchk.right = rcchk.left + 30;

	auto pt = rcchk.CenterPt();
	rcchk.left = pt.x - 8;
	rcchk.right = pt.x + 8;
	rcchk.top = pt.y - 8;
	rcchk.bottom = pt.y + 8;

	if (bChecked)
	{
		D2D1_ELLIPSE el;
		el.point = pt;
		el.radiusX = el.radiusY = 8;
		cxt.cxt->FillEllipse(el, cxt.black);
	}
	else
	{
		D2D1_ELLIPSE el;
		el.point = pt;
		el.radiusX = el.radiusY = 8;
		cxt.cxt->DrawEllipse(el, cxt.black);
	}

	rcb.left += 30;
	DrawCenterText(cxt, cxt.black, rcb, s, s.length(), 0);
}
void D2DButton::DrawCheckButton(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool bChecked)
{
	FRectF rcb = rc.GetBorderRect();

	FRectF rcchk = rcb;
	rcchk.right = rcchk.left+30;

	auto pt = rcchk.CenterPt();
	rcchk.left = pt.x - 8;
	rcchk.right = pt.x + 8;		
	rcchk.top = pt.y - 8;
	rcchk.bottom = pt.y + 8;

	if (bChecked)
		cxt.cxt->FillRectangle(rcchk, cxt.black);
	else
		cxt.cxt->DrawRectangle(rcchk, cxt.black);
	

	rcb.left += 30;
	DrawCenterText( cxt, cxt.black, rcb, s, s.length(), 0 );	
}
void D2DButton::DrawNormalButton( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool)
{	
	FRectF rcb = rc.GetBorderRect();


	//DrawRect( cxt.cxt, rcb, cxt.black, 1);
	cxt.cxt->DrawRectangle( rcb, cxt.black );

	FRectF rcc = rc.GetContentRect();

	D2DTextWrite info;
	info.factory = cxt.wfactory;
	info.textformat =  cxt.text;

#ifdef _DEBUG
	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, info,rcc, s, cxt.gray );
	}
	else if ( (stat & CLICK) && (stat & DEBUG1)  )
		DrawButton( cxt.cxt, info,rcc, L"onclick", cxt.red );
	else if ( (stat & MOUSEMOVE) )
		DrawButton( cxt.cxt, info,rcc, s, cxt.red );
	else if ( (stat & FOCUS) && (stat & DEBUG1)  )
		DrawButton( cxt.cxt, info,rcc, s, cxt.red );
	else
		DrawButton( cxt.cxt, info,rcc, s, cxt.black );

#else
	if ( stat & DISABLE )
	{
		DrawButton( cxt.cxt, info, rcc, s, cxt.gray );
	}
	else
		DrawButton( cxt.cxt, info, rcc, s, cxt.black );

#endif

}
void D2DButton::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent, pacontrol, rc, stat, name, id );

	ondraw_ = DrawNormalButton;
	typ_ = TYP::NORMAL;

	prev_ = nullptr;
	next_ = nullptr;
}

void D2DButton::CreateCheckboxButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id)
{
	D2DControl::CreateWindow(parent, pacontrol, rc, stat, name, id);
	
	ondraw_ = DrawCheckButton;
	typ_ = TYP::CHKCKBOX;
	
	prev_ = nullptr;
	next_ = nullptr;
}
void D2DButton::CreateRadioButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id)
{
	D2DControl::CreateWindow(parent, pacontrol, rc, stat, name, id);
	
	ondraw_ = DrawRadioButton;
	typ_ = TYP::RADIO;

	prev_ = nullptr;
	next_ = nullptr;
}


void D2DButton::RadioButtonUpdate(bool bcheck)
{
	bChecked_ = bcheck;

	auto pr = prev_;
	auto x = this;
	while(pr)
	{
		x = pr;
		pr = x->prev_;
	}

	while(x)
	{
		if ( x != this )
			x->bChecked_ = false;
		x = x->next_;
	}
}
void D2DButton::LinkRadioButton(D2DButton* next)
{
	_ASSERT(next);

	next_ = next;
	next->prev_ = this;
}
void D2DButton::OnDraw( D2DContext& cxt )
{
	if ( VISIBLE( stat_ ))
	{
		D2DMatrix mat(cxt);
		mat_ = mat.PushTransform();	
		mat.Offset( rc_.left, rc_.top );


		

#ifdef _DEBUG

		UINT stat = stat_; //( dbg_ > 0 ? (stat_|DEBUG1) : stat_ );
		ondraw_( cxt,rc_.ZeroRect(),  stat, name_, bChecked_ );

#else
		ondraw_( cxt, rc_.ZeroRect(),  stat_, name_, bChecked_);
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
						if ( id_ > 0 && typ_ == TYP::NORMAL )
						{
							WPARAM wp = MAKEWPARAM( id_, 0 );
							HWND hwnd = ::GetAncestor(parent_->hWnd_, GA_ROOTOWNER);
							::SendMessage( hwnd, WM_DD_COMMAND, wp, (LPARAM)this);
						}
												
						
						if ( OnClick_ )
							OnClick_(this);

						if ( typ_ == TYP::CHKCKBOX )
							bChecked_ = !bChecked_;
						else if (typ_ == TYP::RADIO)
						{							
							RadioButtonUpdate(true);
						}

						
					}														
					ret = 1;
				}
			}
		}
		break;
		case WM_KEYDOWN:
		{
			if (ENABLE(stat_))
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
		}
		break;
		case WM_KEYUP:
		{
			if (ENABLE(stat_))
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
		}
		break;
		case WM_D2D_CLOSE_CONTROL:
		{
			DestroyControl();

		}
		break;
		/*case WM_D2D_JS_ENABLE:
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
		break;*/
		case WM_SIZE:
		{
			auto p = dynamic_cast<IUpdatar*>( parent_control_ );
			if ( p )
			{
				bool fixw = GetStat() & STAT::WIDTH_FIX;
				bool fixh = GetStat() & STAT::HEIGHT_FIX;
				
				if ( !fixw && !fixh )
					p->RequestUpdate(this, WM_SIZE );				
				return 0;
			}
			/*else
			{
				FRectF rcc = rc_; 
				auto child_sz = parent_control_->GetChildSize(this);
			
				{
					auto pt = rcc.LeftTop();

					bool fixw = GetStat() & STAT::WIDTH_FIX;
					bool fixh = GetStat() & STAT::HEIGHT_FIX;

					FSizeF sz;
					sz.width = (fixw ? rcc.Width() :  child_sz.width);
					sz.height = (fixh ? rcc.Height() : child_sz.height);

					rc_.SetRect(pt, sz);
				
				}
			}*/

		}
		break;


	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region  D2DScrollbar
D2DScrollbar::D2DScrollbar()
{
	info_.button_height = 16;
	info_.stat = D2DScrollbarInfo::STAT::NONE;
	info_.position = 0;
	info_.total_height = 0; 
	info_.bVertical = true;
	info_.auto_resize = true;
	target_control_ = NULL;

}
void D2DScrollbar::CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	parent_ = parent;
	target_control_ = target;
	rc_ = rc;
	stat_ = stat;
	name_ = name;
	id_ = id;

	parent_control_ = nullptr;

	OnResutructRnderTarget(true);
	OnCreate();

}
void D2DScrollbar::OnResutructRnderTarget(bool bCreate)
{
	if ( bCreate )
	{
		info_.back_color = parent_->cxt_.ltgray;
		info_.thumb_color = parent_->cxt_.gray;
		info_.thumb_on_color = parent_->cxt_.bluegray;
	}
	else
	{
		info_.back_color = nullptr;
		info_.thumb_color = nullptr;
		info_.thumb_on_color = nullptr;

	}
}
void D2DScrollbar::Clear()
{
	info_.position = 0;
}
void D2DScrollbar::OnCreate()
{
	info_.bVertical = (rc_.Width() < rc_.Height());		
	info_.total_height = ( info_.bVertical ? rc_.Height() : rc_.Width()); 

}
void D2DScrollbar::Show( bool visible )
{
	stat_ = ( visible ? stat_|VISIBLE : stat_&~VISIBLE );
}
LRESULT D2DScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ( (stat_ & VISIBLE) == false )
		return ret;

	bool bVisible = true;
	
	if ( info_.bVertical )
		bVisible = ((info_.total_height > rc_.Height() ) || (stat_ & ALWAYSDRAW));
	else
		bVisible = ((info_.total_height > rc_.Width() ) || (stat_ & ALWAYSDRAW));


	if ( !bVisible ) return ret;

	
	switch( message )
	{
		case WM_PAINT:
		{
			auto cxt = d->cxt_;
			info_.rc = rc_.GetContentRect();
									
			cxt.cxt->GetTransform( &mat_ );
			DrawScrollbar( cxt.cxt, info_ );
		}
		break;
			
		case WM_LBUTTONDOWN:
		{
			auto cxt = d->cxt_;

			FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); // 親座標で変換
			info_.ptprv = pt2;
			int stat = D2DScrollbarInfo::STAT::NONE;
			if ( rc_.PtInRect(pt2))
			{
				d->SetCapture( this, &pt2 );

				FRectF rc1 = ScrollbarRect( info_,1 );
				FRectF rc2 = ScrollbarRect( info_,2 );
				FRectF rc = ScrollbarRect( info_,3 );
				if ( rc.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONTHUMB;
				else if ( rc1.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN1;				
				else if ( rc2.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN2;
				else
				{
					float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
					
					FRectF rcc1( rc1.left,rc1.bottom, rc1.right,  rc1.bottom + info_.position );
					FRectF rcc2( rc1.left,rc1.bottom+info_.position+thumb_height_c, rc1.right, rc2.top );
					if ( rcc1.PtInRect(pt2 ) ) 
						stat = D2DScrollbarInfo::STAT::ONSPC1;	
					else if ( rcc2.PtInRect(pt2 ) ) 
						stat = D2DScrollbarInfo::STAT::ONSPC2;	
				}
				stat |= D2DScrollbarInfo::STAT::CAPTURED;

				ret = 1;
			}

			if ( info_.stat != stat )
			{
				info_.stat = stat;
			}
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); 
			
			int stat = D2DScrollbarInfo::STAT::NONE;
			if ( rc_.PtInRect(pt2) || d->GetCapture() == this )
			{
				stat = D2DScrollbarInfo::STAT::ONTHUMB;

				FRectF rc1 = ScrollbarRect( info_,1 );
				FRectF rc2 = ScrollbarRect( info_,2 );
				FRectF rc = ScrollbarRect( info_,3 );
				//if ( rc.PtInRect(pt2 ) ) 
				//	stat = D2DScrollbarInfo::STAT::ONTHUMB;
				if ( rc1.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN1;
				else if ( rc2.PtInRect(pt2 ) ) 
					stat = D2DScrollbarInfo::STAT::ONBTN2;		
			}
			
			float off = 0;
			// キャプチャーしていたらスクロールは有効とみなす
			if ( d->GetCapture() == this && stat & D2DScrollbarInfo::STAT::ONTHUMB )
			{
				FPointF prev = info_.ptprv;// d->CapturePoint( pt2 );
					
				if ( info_.bVertical )
				{					
					off = pt2.y-prev.y;
					if ( off )
					{
						float move_area = rc_.GetContentRect().Height()-info_.button_height*2;
						float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;

						if ( info_.position + off + thumb_height_c <= move_area )
						{
							info_.position = max(0,info_.position + off );									
						}						
					}
				}
				else
				{
					off = pt2.x-prev.x;
					if ( off )
					{
						float move_area = rc_.Width()-info_.button_height*2;
						float thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;

						if ( info_.position + off + thumb_height_c < move_area )
							info_.position = max(0,info_.position + off );							
					}
				}
				
				if ( off )
				{
					if (target_control_)
						target_control_->UpdateScrollbar( this );										

					d->redraw_ = 1;
				}
				
				stat |= D2DScrollbarInfo::STAT::CAPTURED;
				ret = 1;
			}
			else
			{
				stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
			}

			if (info_.stat != stat)
			{
				info_.stat = stat;
				d->redraw_ = 1;
			}
			info_.ptprv = pt2;
		}
		break;		
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;

				float off = 0;
				if ( info_.stat == D2DScrollbarInfo::STAT::ONBTN1 )
					off = -OffsetOnBtn(1);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONBTN2 )
					off = OffsetOnBtn(1);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONSPC1 )
					off = -OffsetOnBtn(2);
				else if ( info_.stat == D2DScrollbarInfo::STAT::ONSPC2 )
					off = OffsetOnBtn(2);

				if ( off )
				{
					float move_area = 0, thumb_height_c = 0;
					if ( info_.bVertical )
					{
						move_area = rc_.GetContentRect().Height()-info_.button_height*2;
						thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
					}
					else
					{
						move_area = rc_.Width()-info_.button_height*2;
						thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;
					}
						
					info_.position = min( move_area -thumb_height_c, max(0,info_.position + off ));
					if (target_control_)
						target_control_->UpdateScrollbar( this );										
				}
				ret = 1;
			}
			else
			{
				info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
				info_.stat = D2DScrollbarInfo::STAT::NONE;
			}

			
		}
		break;
		case WM_SIZE:
		{
			if ( info_.auto_resize && parent_control_ )
			{
				FRectF rc = parent_control_->GetContentRect();
				FSizeF sz = rc_.Size();

				if ( info_.bVertical )
				{					
					rc_.SetPoint( rc.right-sz.width, rc.top-2 );
					rc_.SetSize( sz.width, rc.bottom-rc.top ); 
				}
				else
				{
					rc_.SetPoint( rc.left, rc.bottom-sz.height );
					rc_.SetSize( rc.Width(), sz.height ); 
				}
			}
			else if ( info_.auto_resize )
			{
				FRectF rc = target_control_->GetRect().GetBorderRect().ZeroRect();  //GetContentRect();
				FSizeF sz = rc_.Size();

				if ( info_.bVertical )
				{
					rc_.SetPoint( rc.right-sz.width,rc.top );
					rc_.SetSize( sz.width, rc.bottom-rc.top ); 
				}
				else
				{
					rc_.SetPoint( rc.left, rc.bottom-sz.height );
					rc_.SetSize( rc.Width(), sz.height ); 
				}
			}
		}
		break;
	}	
	return ret;
}

float D2DScrollbar::OffsetOnBtn( int typ )
{	
	if ( typ == 1  )
		return  10; 
	else if ( typ == 2 )
		return 20; 

	return 0;
}
void D2DScrollbar::SetTotalSize( float cy )
{
	_ASSERT( 0 <= cy  );
	info_.total_height = cy;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region  D2DImage

D2DImage::D2DImage()
{

}
void D2DImage::OnResutructRnderTarget(bool bCreate)
{
	if (!bCreate)
		bmp_.Release();
	else
	{
		ComPTR<ID2D1DeviceContext> dcxt;
		if (HR(parent_->cxt_.cxt->QueryInterface(&dcxt)))
		{
			LoadImage(dcxt, filename_);
		}
	}

}
LRESULT D2DImage::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;

	switch( message )
	{
		case WM_PAINT:
		{
	
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);		
			mat.PushTransform();
			
			mat.Offset( rc_.left, rc_.top );
			mat_ = mat; // 自座標(左上0,0)のmatrix
			
			auto sz = bmp_->GetSize();

			FRectF rc(0,0,sz);
			cxt.cxt->DrawBitmap( bmp_, rc );
		}
		break;
	}
	return 0;
}
void D2DImage::OnCreate()
{

}
bool D2DImage::LoadImage( ID2D1DeviceContext* cxt, LPCWSTR fnm )
{
	filename_ = fnm;
	if ( V4::ImageLoad( cxt, fnm, &bmp_ ))
	{
		auto sz = bmp_->GetSize();
		rc_.SetSize(sz);
		return true;
	}
	return false;
}

#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region  D2DWaiter

D2DWaiter::~D2DWaiter()
{
	int a = 0;
}

void D2DWaiter::DrawContent2(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, int pos)
{
	FRectF drc = rc;
	DrawRect(cxt, drc, cxt.black,1);
	FillRectangle(cxt, drc, cxt.white);

	drc.Offset(10,10);
	drc.SetSize(4,13);

	pos = pos % 20;
	for( int i = 0; i < 20; i++ )
	{
		FillRectangle(cxt, drc, (i == pos ? cxt.white: cxt.bluegray));

		drc.Offset( 6, 0 );
	}

}
void D2DWaiter::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent, pacontrol, rc, stat, name, id );
	ondraw_ = DrawContent2;
	pos_ = -1;
	mt_ = NULL;
}
void D2DWaiter::OnDraw( D2DContext& cxt )
{
	FRectF rcz = rc_.GetContentRectZero();

	DWORD elapsed_time_ms = MoveTarget::TimeGetTime(freq_) - start_time_;

	cxt.cxt->GetTransform( &mat_ );
	ondraw_( cxt, rcz, stat_, name_, pos_ );			

		
	FString s = FString::Format( L"elapsed time: %d[ms]", elapsed_time_ms);
		
	DrawCenterText( cxt, cxt.black, rcz, s,s.length(),0 ,0);
}
void D2DWaiter::Cancel()
{
	if (mt_ )
		mt_->Cancel();
}
void D2DWaiter::Start()
{
	parent_->SetCapture(this);

	MoveTarget* mt = new MoveTarget();

	freq_ = mt->__s_frequency_;
	start_time_ = MoveTarget::TimeGetTime(freq_);
	
		
	mt->SetParameter( 0, 0, 200, 0, 1000*3 );
			
	mt->Fire_ = [this](MoveTarget* mt, float x1, float y1)->bool
	{
		pos_ = (int)x1;		
		return true;
	};
		
	mt->End_ = [this](MoveTarget* mt, float x, float y)
	{		
		Visible(false);

		if ( parent_->GetCapture() == this )
			parent_->ReleaseCapture();
				

		SafeDestroyControl();
		
		mt_ = NULL;
		delete mt;
	};
			
	mt->Start(parent_);


	mt_ = mt;
}

LRESULT D2DWaiter::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DMatrix mat( d->cxt_ );

			mat.PushTransform();
			mat.Offset( rc_ );
			
			
			OnDraw( d->cxt_ );

			if ( pos_ < 0 )
			{
				pos_ = 0;
				Start();
			}

			mat.PopTransform();
					
		}
		break;
		case WM_DESTROY:
		{
		   delete mt_;

		}

		break;
	}
	return 0;
}





#pragma endregion

////////////////////////////////////////////////////////////////////////////
#pragma region  D2DFRectFBM

void D2DFRectFBM::OnCreate()
{
	fore_color_.color = ColorF(ColorF::White);
	back_color_.color = D2RGB(104,143,149);
	

	OnResutructRnderTarget(true);
}
void D2DFRectFBM::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int typ, int id )
{
	D2DControl::CreateWindow(parent,pacontrol,rc,stat,name,id);
	typ_ = typ;

	auto_resize_ = !( stat_&STAT::WIDTH_FIX && stat_&STAT::HEIGHT_FIX );
}

FString DebugFRectFBM( const FRectFBM& rc, bool bCRLF )
{
	
	return FString::Format( L"left:%4.2f\ntop:%4.2f\nwidth:%4.2f\nheight:%4.2f\nborder:%4.2f\npadding-l:%4.2f", rc.left,rc.top,rc.Width(),rc.Height(),rc.BorderWidth_,rc.Padding_.l);

	

}

LRESULT D2DFRectFBM::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return ret;

	if ( typ_ == 0 && message == WM_PAINT )
	{
		int a = 0;
	}

	switch (message)
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			
			
			D2DMatrix mat(cxt);
			mat.PushTransform();
			mat_ = mat;

			mat.Offset( rc_ );

			


			DrawFill(cxt, rc_.GetContentRectZero(), (ID2D1Brush*) back_color_.br);

			if (typ_ == 1)
			{
				FRectF rc = rc_.GetContentRectZero();
				D2DRectFilter fil( cxt, rc );

				FString s = L"name:";
				s += name_;
				s += L"\n";
				s += DebugFRectFBM(rc_, true);
				
				mat.Offset( 5,2);
				auto br = (ID2D1Brush*) fore_color_.br;
				cxt.cxt->DrawTextW( s, s.length(), cxt.cxtt.textformat, rc, br );
			}

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
			if ( typ_ > 0 )
				ret = MouseSelectedMoveProc( this,message,wParam,lParam);
		break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if ( typ_ > 0 )
				ret = MousePropertyMenuProc(this,message,wParam,lParam);
		break;
	}
	return ret;
}



void V4::ParameterColor(D2DWindow* parent, SolidColor& clr, const std::map<std::wstring, VARIANT>& prms, LPCWSTR key )
{
	auto it = prms.find(key);
	if ( it != prms.end())
	{		
		clr.color = VariantColor(it->second);
		clr.br = parent->GetSolidColor(clr.color);
	}
}

void D2DFRectFBM::BackColor( D2D1_COLOR_F clr)
{
	back_color_.color = clr;
	back_color_.br = parent_->GetSolidColor(back_color_.color);

}
void D2DFRectFBM::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);

	ParameterColor(parent_, fore_color_,prms, L"forecolor");
	ParameterColor(parent_, back_color_, prms, L"backcolor");
}
void D2DFRectFBM::OnResutructRnderTarget(bool bCreate)
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


#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////
#pragma region  D2DMessageBox
LRESULT D2DMessageBox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	bool bprocess = true;

	switch (message)
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_; 
			D2DMatrix mat(cxt);

			const FRectF rc = rc_.GetBorderRect();
			D2DRectFilter filter(cxt, rc);

			FillRectangle(cxt, rc, cxt.white);

			mat.PushTransform();
			mat.Offset(rc_.left, rc_.top);
			mat_ = mat;
			Draw(cxt);
			mat.PopTransform();


			cxt.cxt->DrawRectangle(rc, cxt.black);
							

			
		}
		break;
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				Close(IDCANCEL);
				ret = 1;
			}
			else if (wParam == VK_RETURN)
			{
				Close(IDOK);
				ret = 1;
			}
		}
		break;
	}

	if (bprocess)
		ret = D2DControls::WndProc(d, message, wParam, lParam);

	return ret;
}

void D2DMessageBox::DoModal(D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype)
{
	xassert(parent);


	HWND hWnd = parent->parent_->hWnd_;

	RECT rc;
	GetClientRect(hWnd, &rc);

	FRectF rcf(0, 0, 320, 200);
	FRectF rcbtn(0, 0, 100, 30);

	rcbtn.CenterRect(rcf);

	rcf.Offset((rc.left + rc.right) / 2, (rc.bottom + rc.top) / 2);

	D2DControls* pcs = parent->parent_control_;

	if (pcs == NULL)
		pcs = dynamic_cast<D2DControls*>(parent);
	else
	{
		while (pcs->parent_control_)
		{
			pcs = pcs->parent_control_; // search top controls.
		}
	}

	CreateWindow(parent->parent_, pcs, rcf, VISIBLE, NONAME);


	rcbtn.SetRect(45, 200 - 45, FSizeF(100, 30));
	D2DButton* closebtn_ = new D2DButton();
	closebtn_->CreateWindow(parent_, this, rcbtn, VISIBLE | BORDER, L"close");
	closebtn_->OnClick_ = [this](D2DButton* btn)
	{
		Close(IDOK); // 内部でReleaseCaptureされる
	};


	rcbtn.Offset(130, 0);
	D2DButton* cancelbtn_ = new D2DButton();
	cancelbtn_->CreateWindow(parent_, this, rcbtn, VISIBLE | BORDER, L"cancel");
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
void D2DMessageBox::Show(D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype)
{
	D2DMessageBox* mb = new D2DMessageBox();
	mb->DoModal(parent, msg, title, btntype);
	parent->parent_->redraw_ = 1;
}
void D2DMessageBox::Close(int result)
{
	Result_ = result;

	if (onclose_)
	{
		onclose_(this, Result_);
	}

	DestroyControl();
}
void D2DMessageBox::Draw(D2DContext& cxt)
{
	FRectF rc = rc_.ZeroRect();
	{
		V4::FillRectangle(cxt, rc, cxt.gray);
		rc.InflateRect(-5, -5);
		V4::FillRectangle(cxt, rc, cxt.white);


		FRectF rc2 = rc;
		rc2.bottom -= 60;
		rc2.left += 20;

		// messsage
		DrawCenterText(cxt, cxt.black, rc2, message_.c_str(), message_.length(), 0, 0);


		FRectF rc1 = rc;
		rc1.top = rc1.bottom - 60;
		V4::FillRectangle(cxt, rc1, cxt.ltgray);
	}
	
	// title
	rc = rc_.ZeroRect();
	rc.left += 10;
	rc.bottom = rc.top + 30;
	
	D2DTextWrite info;
	info.factory = cxt.wfactory;	
		
	CreateTextFormatL2(info.factory, L"メイリオ", 13,800, &info.textformat);

	DrawCenterText(cxt, info, cxt.black, rc, title_.c_str(), title_.length(), 0 );

	info.textformat->Release();
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region  D2DGroupControls
D2DGroupControls::D2DGroupControls()
{
	move_able_ = 0;
	

}


LRESULT D2DGroupControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return ret;

	D2DContext& cxt = d->cxt_;
	D2DMatrix mat(cxt);
		
	if (message == WM_PAINT)
	{
		mat.PushTransform();
		mat.Offset(-scrollbar_off_.width, -scrollbar_off_.height);
		mat.Offset(rc_.left, rc_.top);
		mat_ = mat; // 自座標(左上0,0)のmatrix


		if (stat_ & BORDER)
		{
			FRectF rc1 = rc_.ZeroRect();

			D2DRectFilter x(cxt, rc1);
			
			
			FillRectangle( cxt, rc1, (ID2D1Brush*)back_color_.br );

			SendMessageReverseAll(d, message, wParam, lParam);

			
			cxt.cxt->DrawRectangle(rc1, (ID2D1Brush*) back_color_.br);
		}
		else
			SendMessageReverseAll(d, message, wParam, lParam);

		mat.PopTransform();
		return 0;
	}
	else if ( !ENABLE(stat_))
		return 0;
	
	mat = mat_;
	
	if ( message == WM_LBUTTONDOWN )
	{
		ret = SendMessageAll(d, message, wParam, lParam);
		
		// ret != 0の場合、下位のオブジェクト（キャプチャーされている）へメッセージが流れるためこれ以上処理しない。
		if ( ret )
			return ret;
	}


	switch (message)
	{		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			if ( move_able_ == 0 )
				ret = SendMessageAll(d, message, wParam, lParam);
			else if (move_able_ > 0 )
			{
				FPointF pt3 = mat_.DPtoLP(lParam);
				pt3.x += rc_.left;
				pt3.y += rc_.top;

				
				if ( d->GetCapture() == this )
				{
					d->ReleaseCapture();
					ret = 1;
					if (move_able_ == 3)
						move_able_ = 1;
				}
				else if (rc_.PtInRect(pt3))
				{
					d->SetCapture( this, &pt3 );
					move_able_ = 2;
					ret = 1;
				}
			}
		}
		break;
		case WM_MOUSEMOVE:
		{			
			if (d->GetCapture() == this)
			{
				FPointF pt3 = mat_.DPtoLP(lParam);
				pt3.x += rc_.left;
				pt3.y += rc_.top;

				move_able_ = 3;


				FPointF ptprev = d->CapturePoint(pt3);

				rc_.Offset( pt3.x-ptprev.x, pt3.y - ptprev.y );

				ret = 1;
				d->redraw_ = 1;
			}
		}
		break;
		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE )
			{
				if (move_able_>0)
					move_able_ = 1;

				if (d->GetCapture() == this)
				{
					d->ReleaseCapture();
					ret = -1;
				}
			}			
		}
		break;
		case WM_SIZE:
		{
			



		}
		break;

	}




	if (ret == 0)
		ret = SendMessageAll(d, message, wParam, lParam);
	else if ( ret < 0 )
		ret = 0;

	return ret;
}



void D2DGroupControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);
	ParameterColor(parent_, back_color_, prms, L"backcolor");

	
	for (auto& it : prms)
	{
		auto& key = it.first;
		if (key == L"moveable")
		{
			move_able_ = (VariantBOOL(it.second) == true ? 1 : 0);
		}
	}

}
#pragma endregion
//////////////////////////////////////////////////////////////////////////////////////////////////

D2D1_COLOR_F V4::VariantColor(const VARIANT& src)
{
	_variant_t s(src), v;
	v.ChangeType(VT_UI4, &s);
	D2D1_COLOR_F r = D2RGB(0,0,0);

	if (v.vt == VT_UI4)
		r = D2DRGBA((DWORD) v.ulVal);

	return r;
}

bool V4::VariantBOOL(const VARIANT& src)
{
	_variant_t s(src), v;
	v.ChangeType(VT_BOOL, &s);
	bool r = false;

	if (v.vt == VT_BOOL)
		r = (v.boolVal == VARIANT_TRUE);

	return r;
}
