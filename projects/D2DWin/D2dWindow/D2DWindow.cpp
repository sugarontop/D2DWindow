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
#include "D2DWindow.h"
#include "D2DWindowControl_easy.h"
#include "gdi32.h"
#include <Shellapi.h>
#include "MoveTarget.h"

#define CLASSNAME L"D2DWindow"

using namespace V4;
using namespace GDI32;

#define IDLE_TIME (5*1000)
#define IDLE_TIMER_ID 99




static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D2DWindow* d = (D2DWindow*)::GetWindowLongPtr( hWnd, GWL_USERDATA );

	switch( message )
	{
		case WM_CREATE:
		{									
			CREATESTRUCT* st = (CREATESTRUCT*)lParam;
			
			::SetWindowLongPtr( hWnd, GWL_USERDATA,(LONG) st->lpCreateParams ); // GWL_USERDATA must be set here.

			SetFocus(hWnd);

			
			auto IdleMessage = [](HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
			{
				if (IDLE_TIMER_ID == id)
					SendMessage(hwnd, WM_D2D_IDLE, 0, 0);
			};
			
			::SetTimer(hWnd, IDLE_TIMER_ID, IDLE_TIME, IdleMessage);
						
			return 0; 
		}		
		break;
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);					
			ID2D1RenderTarget* cxt = d->cxt_.cxt;			
			

			cxt->BeginDraw();
			D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();


			cxt->SetTransform(mat);
			cxt->Clear(ColorF(ColorF::White));

			d->WndProc(WM_PAINT, wParam, lParam );



			// captureされたものをもう一度トップで描画			
			if (!d->capture_obj_.empty())
			{
				int cap_cnt = d->capture_obj_.size();
				auto p = d->capture_obj_.head();
				for( int i = 0; i < cap_cnt; i++ )
				{
					p[i]->WndProc( d, WM_D2D_PAINT, wParam, lParam );
				}
			}


			// for anime
			for(auto it = d->mts_.begin(); it!=d->mts_.end(); )
			{					
				bool isend;
				MoveTarget* mtx = (MoveTarget*)it->first;
				mtx->UIFire(d, isend);

				if (isend)
					d->mts_.erase(it++);
				else
					it++;
			}

#ifdef BOXMODELTEST
{
	auto cxttemp = d->cxt_;
	FRectFBoxModel rcx(200,200,FSizeF(100,100));

	rcx.Margin_.Set(10);
	rcx.BorderWidth_ = 10;
	rcx.Padding_.Set(0); 

	{
		auto rc1 = rcx.GetBorderRect();
	
		D2DRectFilter filter(d->cxt_, rc1 );

		auto rcb = rcx.GetBorderRect();

		auto rcc = rcx.GetContentRect();

		d->cxt_.cxt->FillRectangle( rcc, cxttemp.ltgray );	
		d->cxt_.cxt->DrawRectangle( rcb, cxttemp.black, 1 );
	}

	{
		
		auto rc1 = rcx.GetMarginRect();
		D2DRectFilter filter(d->cxt_, rc1 );
	
		d->cxt_.cxt->FillRectangle( rc1, cxttemp.transparent );	
		d->cxt_.cxt->DrawRectangle( rc1, cxttemp.black, 1 );
	}
}
#endif
			auto hr = cxt->EndDraw();

			// ディスプレイの解像度を変更したり、ディスプレイ アダプターを取り外したりすると、デバイスが消失する可能性があります
			if ( D2DERR_RECREATE_TARGET == hr || d->resource_test_ )
			{
				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 0,0 ); //リソースのリンクを切る
				d->ResourceCreate(false); // 実際にリリース
							
				d->cxt_.DestroyRenderTargetResource();
				d->cxt_.CreateRenderResource(hWnd);
								
				d->ResourceCreate(true); // リソースを作成
				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 1,0 );// リソースとリンクする

				d->redraw_ = 1;
				d->resource_test_ = false;
			}
			else if ( hr != S_OK )
			{
				// 88990015 リソースが誤ったレンダターゲットを使用
				
				//D2DError( hr, L"EndDraw error", __LINE__, __FILE__ );
				d->redraw_ = 0;

			}

			EndPaint(hWnd, &ps);

			if (d->redraw_)
			{
				InvalidateRect(hWnd, NULL, FALSE);
				d->redraw_ = 0;
			}

		#ifdef USE_ID2D1DEVICECONTEXT
			d->cxt_.dxgiSwapChain->Present(1, 0);
		#endif

			return 0;
		}
		break;
		case WM_ERASEBKGND:
			return 1;
		break;
		case WM_SIZE :
		{		
			// CComPtrの参照数にきをくばること
			auto bl = ( d->cxt_.cxt != NULL );		
			if ( bl )
			{
				#ifdef USE_ID2D1DEVICECONTEXT
				{					
					UINT cx = d->cxt_.RenderSize_.width;
					UINT cy = d->cxt_.RenderSize_.height;

					UINT ncx = LOWORD(lParam);
					UINT ncy = HIWORD(lParam);

					// 大きい場合と小さすぎる場合に再作成
					if ( cx < ncx || cy < ncy ||  ncx < (cx * 0.5) ||  ncy < (cy * 0.5)  )
					{
						d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 0,0 );
						d->ResourceCreate(false);


						d->cxt_.DestroyRenderTargetResource();
						d->cxt_.CreateRenderResource( hWnd );
						
						d->ResourceCreate(true);
						d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 1,0 );
					}
				}
				#else
				{
					ComPTR<ID2D1HwndRenderTarget> trgt;
					auto hr = d->cxt_.cxt->QueryInterface( &trgt );												
					if ( HR(hr))				
						trgt->Resize( D2D1::SizeU( LOWORD(lParam), HIWORD(lParam) ));
				}
				#endif

				

				d->WndProc( message, wParam,lParam );
			}
					
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		case WM_MOUSEWHEEL:
		{				
			FPoint pt(lParam);			
			ScreenToClient( hWnd, &pt );				
			lParam = MAKELONG(pt.x, pt.y );
		}
		case WM_LBUTTONDOWN:		
		case WM_RBUTTONDOWN:
			SetFocus(hWnd);
		case WM_LBUTTONDBLCLK:
		case WM_CAPTURECHANGED:		
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:		
		case WM_CHAR:
		{

			d->WndProc( message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		//case WM_IME_NOTIFY:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_COMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		{
			d->WndProc(  message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_KILLFOCUS: // 0x8
		case WM_KEYDOWN: // 0x100
		case WM_KEYUP: //0x101
		{
			d->WndProc( message, wParam,lParam );

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}
			
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_DROPFILES:
		{
			//std::vector<D2D_DROPOBJECT> ar;			
			//{
			//	HDROP hd = (HDROP)wParam;
			//	UINT cnt = DragQueryFile(hd,-1,nullptr,0);
			//	ar.resize(cnt);
			//	for( UINT i = 0; i < cnt; i++ )
			//	{
			//		WCHAR cb[MAX_PATH];
			//		POINT pt;
			//		DragQueryFile(hd,i,cb,MAX_PATH);
			//		DragQueryPoint(hd, &pt);

			//		D2D_DROPOBJECT it;
			//		it.filename = cb;
			//		it.pt = pt;
			//		ar[i] = it;
			//	}
			//	DragFinish( hd );
			//}						
			//d->WndProc( WM_D2D_DRAGDROP, (WPARAM)&ar,0 );
			//

			//if ( d->redraw_ )
			//{
			//	InvalidateRect( hWnd, NULL, FALSE );				
			//	d->redraw_ = 0;
			//}

			return 0;
		}
		break;
		
		case WM_DESTROY:
		{			
			d->WndProc( message, wParam,lParam );
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		case WM_D2D_DISPATCHER_DELEGATE:
		{
			// from PostMessage.

			DispatherDelegate& dispatcher = *(DispatherDelegate*)wParam;

			dispatcher(hWnd, (void*)lParam);

			return 0;
		}
		break;
		default :
		{
			if ( message >= WM_USER )
			{
				d->WndProc(  message, wParam,lParam );
				if ( d->redraw_ )
				{
					InvalidateRect( hWnd, NULL, FALSE );				
					d->redraw_ = 0;
				}
			}
			else 
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}
ATOM D2DWindowRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	ZeroMemory( &wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance; 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; 
	wcex.lpszClassName	= CLASSNAME;

	return RegisterClassEx(&wcex);
}

int D2DWindow::SecurityId(bool bNew)
{
	static int sec_id;
	
	std::random_device seed_gen;
    static std::default_random_engine engine(seed_gen());
    static std::uniform_int_distribution<> dist(-999999, 999999);  
	
	if ( bNew )
		sec_id = dist(engine);
				
	return sec_id;
}

D2DWindow::D2DWindow():capture_obj_(256),capture_pt_(256)
{
	
	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(wcx);
	
	if ( !GetClassInfoEx( GetModuleHandle(0), CLASSNAME, &wcx ))	
		D2DWindowRegisterClass( GetModuleHandle(0));
}
void D2DWindow::CallDispatcher( DispatherDelegate* pfunc, LPVOID lp )
{
	_ASSERT( pfunc );

	PostMessage( hWnd_, WM_D2D_DISPATCHER_DELEGATE, (WPARAM)pfunc, (LPARAM)lp );
}
void D2DWindow::Clear()
{
	// All object are cleared.

	ResourceCreate(false);

	while ( !capture_obj_.empty())
		capture_obj_.pop();
	
	children_ = NULL;
	
}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );

	dwWSSTYLE |=WS_CLIPCHILDREN|WS_MAXIMIZE;

	HWND h = parent;
	HWND h2 = parent;

	while( h )
	{
		h2 = h;
		h = ::GetParent(h2);
	}

	hMainFrame_ = h2;

	RECT rc1;
	::GetClientRect( hMainFrame_, &rc1 );

	if ( rc.left == rc.right )
	{
		rc = rc1;
	}
	
	hWnd_ = ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this ); 	

	try 
	{
		cxt_.Init( SingletonD2DInstance::Init() );
				
		cxt_.CreateRenderResource( hWnd_ );
			
		cxt_.CreateResourceOpt();
	}
	catch( D2DError er)
	{
		throw er;

	}
	
	DragAcceptFiles( hWnd_, TRUE );

	redraw_ = 0;
	resource_test_ = false;
	roundpaint_obj_ = nullptr;

	if ( OnCreate )
		OnCreate( this );

	// OnCreateで各子コントロールを作成後にサイズの調整が必要
	SendMessage(hWnd_, WM_SIZE,0,MAKELPARAM(rc.right-rc.left,rc.bottom-rc.top));
	return hWnd_;

}


LRESULT D2DWindow::WndProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	auto children = children_.get();

	// WM_PAINT, WM_SIZE はCAPTUREに関わらず、すべてにSENDすること

	if ( message == WM_PAINT  && children)
	{		
		children->WndProc(this,WM_PAINT,wParam,lParam);

		if (roundpaint_obj_)
			roundpaint_obj_->WndProc(this, message, wParam, lParam);

		return ret;
	}
	else if (message == WM_SIZE && children)
	{
		children->WndProc(this,WM_SIZE,wParam,lParam);
		return ret;
	}
	else if ( message == WM_D2D_RESTRUCT_RENDERTARGET )
	{
		// WM_D2D_RESTRUCT_RENDERTARGETは裏から処理する
		if ( wParam == 0 )
		{
			if ( children )
				children->OnResutructRnderTarget(false); //ResourceRelease
		}					
		else if ( wParam == 1 )
		{
			if ( children )
				children->OnResutructRnderTarget(true); //CreateResource
		}
		return ret;
	}
	else if ( !capture_obj_.empty() && message < WM_D2D_EVENT_FIRST )
	{
		auto obj1 = capture_obj_.top();
	
		auto r = obj1->WndProc( this, message, wParam, lParam ); 

		if ( r == LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS && message == WM_LBUTTONDOWN && children_)
			r = children_->WndProc(this,message,wParam,lParam);

		
		if ( message==WM_MOUSEMOVE  )
		{		
			auto dropobject = (D2DControl*)obj1;

			auto rcxx = dropobject->GetRect();

			int a = WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR;
				
			// dropされるobjectをどこへ落とすか
			for( auto& it : drag_accepters_ )
			{
				int ra = (it)->WndProc( this, WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR, (WPARAM)dropobject, 0 );
				if ( ra )
					break;
			}
		}
	}
	else if (WM_D2D_IDLE == message && capture_obj_.empty() && death_objects_.size())
	{
		//_ASSERT(L"death object removed cnt=%d\n", death_objects_.size());

		for( auto& it : death_objects_ )
		{
			if ( it.is_alive_in_parentControl )
			{
				it.target->parent_control_->Detach( it.target.get() );
			}
		}
		
		death_objects_.clear();
	}	
	else if (children )
	{		
		children->WndProc(this,message,wParam,lParam);
	}
	
	
	switch( message )
	{		
		// 状態変化しやすい命令はデフォルトで必ずリドローさせる。
		// WM_MOUSEMOVEは適宜、リドローさせる。
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:		
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			redraw_ = 1; 
		break;
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			redraw_ = 1; 
		break;
		case WM_DESTROY:
		{
			if ( children_ )
				children_->DestroyControl();
			children_ = NULL;

			if ( OnDestroy )
				OnDestroy( this );

			cxt_.DestroyAll();
			
		}
		break;
		
	}
	return ret;
}

void D2DWindow::SetCapture(D2DCaptureObject* p,  FPointF* pt, D2DMat* mat )
{
	if ( capture_obj_.empty())
	{
		::SetCapture( hWnd_ );		
	}

	::SetFocus( hWnd_ );
	
	if (capture_obj_.empty() )
		capture_obj_.push(p);
	else if ( p != capture_obj_.top())
		capture_obj_.push(p);

	if ( pt )
		capture_pt_ = *pt; 
	else
		capture_pt_ = (FPointF(0,0));

	if ( mat )
		capture_matrix_ = *mat;
}
D2DCaptureObject* D2DWindow::ReleaseCapture( bool all_layer )
{
	auto p = capture_obj_.top();
	HWND hWnd = ::GetCapture();

	while(p)
	{
		capture_obj_.pop();		
		p = ( all_layer ? capture_obj_.top() : nullptr );
	}
		
	if ( capture_obj_.empty())			
		::ReleaseCapture();
	
	::SetFocus( hWnd );
	redraw_ = 1;

	return p;
}

void V4::SetCursor( HCURSOR h )
{
	if ( h != ::GetCursor() )
	{	
		::SetCursor( h );
	}
}


DWORD D2DRGBADWORD_CONV(D2D1_COLOR_F clr)
{
	DWORD r = ROUND(clr.r * 255);
	DWORD g = ROUND(clr.g * 255);
	DWORD b = ROUND(clr.b * 255);
	DWORD a = ROUND(clr.a * 255);

	return D2DRGBADWORD(r, g, b, a);
}
ID2D1SolidColorBrush* D2DWindow::GetSolidColor(D2D1_COLOR_F clr)
{
	DWORD d = D2DRGBADWORD_CONV(clr);

	auto& it = SolidColorBank_.find(d);
	if (it != SolidColorBank_.end())
		return it->second;

	auto r = MakeBrsuh(cxt_, clr).Detach();
	SolidColorBank_[d] = r;
	return r;
}

void D2DWindow::ResourceCreate(bool bCreate)
{
	if (!bCreate)
	{
		for (auto& it : SolidColorBank_)
		{
			ULONG x = it.second->Release();
			
			_ASSERT( x == 0 );
		}
		// この後でRenderTargetは再作成される
		// it.firstは色情報。
	}
	else
	{
		for (auto& it : SolidColorBank_)
		{
			DWORD dw = it.first;
			auto clr = D2DRGBA(dw);
			SolidColorBank_[dw] = MakeBrsuh(cxt_, clr).Detach();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
//CHDL ControlHandle::handle_ = 800; // initial value
//
//CHDL ControlHandle::CreateControlHandle(D2DControl* ctrl)
//{	
//	handle_++;
//
//	m1_[handle_] = ctrl;
//	m2_[ctrl->GetName()] = handle_;
//	return handle_;
//}
//void ControlHandle::DeleteControlHandle( CHDL id )
//{
//	if ( id == 0 || m1_.empty() ) return;
//
//	xassert( m1_.find(id) != m1_.end());
//
//	D2DControl* ctrl = m1_[id];
//		
//	m1_[id] = NULL;
//	m2_[ctrl->GetName()] = 0;
//
//}
//D2DControl* ControlHandle::GetFromControlHandle(CHDL id )
//{
//	if ( m1_.find(id) != m1_.end())
//		return m1_[id];
//
//	return NULL;
//}
//D2DControl* ControlHandle::GetFromControlName(LPCWSTR nm )
//{
//	if ( m2_.find(nm) != m2_.end())
//	{
//		return GetFromControlHandle(m2_[nm]);
//	}
//	return NULL;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
