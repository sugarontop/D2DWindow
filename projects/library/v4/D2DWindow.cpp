#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include <random>
#define CLASSNAME L"D2DWindow"

using namespace V4;
using namespace GDI32;


std::wstring D2DWindow::appinfo_;


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D2DWindow* d = (D2DWindow*)::GetWindowLongPtr( hWnd, GWL_USERDATA );

	switch( message )
	{
		case WM_CREATE:
		{									
			CREATESTRUCT* st = (CREATESTRUCT*)lParam;
			
			::SetWindowLongPtr( hWnd, GWL_USERDATA,(LONG) st->lpCreateParams ); 
			SetFocus(hWnd);
			return 0;
		}		
		break;
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);					
			ID2D1RenderTarget* cxt = d->cxt_.cxt;			
			
			//SelectResource( d->res_ );

			cxt->BeginDraw();
			D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();

			cxt->SetTransform(mat);
			cxt->Clear(ColorF(ColorF::White));

			d->WndProc( message, wParam, lParam ); // All objects is drawned.


			// CAPTURE OBJECTは最後に表示 
			// Comobox内のListboxなど
			int cap_cnt = d->capture_obj_.size();
			if ( cap_cnt )
			{
				auto p = d->capture_obj_.head();
				for( int i = 0; i < cap_cnt; i++ )
				{
					p[i]->WndProc( d, WM_D2D_PAINT, wParam, lParam );
				}
			}
			if (d->redraw_)
			{
				InvalidateRect(hWnd, NULL, FALSE);
				d->redraw_ = 0;
			}

			if ( D2DERR_RECREATE_TARGET == cxt->EndDraw())
			{
				// ディスプレイの解像度を変更したり、ディスプレイ アダプターを取り外したりすると、デバイスが消失する可能性があります
				//D2R.Restruct(hWnd);

				//TRACE( L"D2DERR_RECREATE_TARGET\n" );
				//InvalidateRect(hWnd, NULL, FALSE);
			}
			EndPaint(hWnd, &ps);

			return 0;
		}
		break;
		case WM_ERASEBKGND:
			return 1;
		break;
		case WM_SIZE :
		{		
			auto cxt = d->cxt_.cxt;		
			if ( cxt )
			{
				CComPtr<ID2D1HwndRenderTarget> trgt;
				auto hr = cxt->QueryInterface( &trgt );												
				_ASSERT(HR(hr));

				trgt->Resize( D2D1::SizeU( LOWORD(lParam), HIWORD(lParam) ));

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
		
		case WM_DESTROY:
		{			
			d->WndProc( message, wParam,lParam );
			return DefWindowProc(hWnd, message, wParam, lParam);
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

void D2DWindow::Clear()
{
	// All object are cleared.

	while ( !capture_obj_.empty())
		capture_obj_.pop();
	
	children_ = NULL;
}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );

	dwWSSTYLE |=WS_CLIPCHILDREN;

	HWND h = parent;
	HWND h2 = parent;

	while( h )
	{
		h2 = h;
		h = ::GetParent(h2);
	}

	hMainFrame_ = h2;
	
	hWnd_ =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this ); 	

	cxt_.Init( SingletonD2DInstance::Init(), hWnd_ );

	cxt_.cxtt.Init( cxt_,DEFAULTFONT_HEIGHT_JP, DEFAULTFONT_JP ); // default font, default font size

	//res_ = ResourceAllocate( cxt_.cxt, SingletonD2DInstance::Init().wrfactory, hWnd_ );
	redraw_ = 0;
	
	//SelectResource( res_ );
	
	if ( OnCreate )
		OnCreate( this );

	return hWnd_;

}

LRESULT D2DWindow::WndProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	auto children = children_.get();

	if ( message == WM_PAINT  && children)
	{
		children->WndProc(this,WM_PAINT,wParam,lParam);
	}
	else if (message == WM_SIZE && children)
	{
		children->WndProc(this,message,wParam,lParam);
	}
	else if ( !capture_obj_.empty() && message < WM_D2D_EV_FIRST )
	{
		auto obj1 = capture_obj_.top();
	
		auto r = obj1->WndProc( this, message, wParam, lParam ); 

		if ( r == LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS && message == WM_LBUTTONDOWN && children_)
			children_->WndProc(this,message,wParam,lParam);

		
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
		ATLTRACE(L"death object removed cnt=%d\n", death_objects_.size());
		death_objects_.clear();
	}
	else if (children &&  message != WM_MOUSEMOVE)
	{		
		children->WndProc(this,message,wParam,lParam);
	}
	else if ( children &&  message==WM_MOUSEMOVE )
	{		
		children->WndProc(this,message,wParam,lParam);
	}
	
	
	switch( message )
	{		
		// 状態変化しやすい命令はデフォルトで必ずリドローさせる。WM_MOUSEMOVEは適宜、リドローさせる。
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
			children_ = NULL;
			cxt_.Destroy();
			//ResourceRelease( res_ );
		}
		break;
	}
	return ret;
}

void D2DWindow::SetCapture(D2DCaptureObject* p,  FPointF* pt )
{
	if ( capture_obj_.empty())
	{
		::SetCapture( hWnd_ );
		
	}
	::SetFocus( hWnd_ );
	

	#ifdef _DEBUG
	{
		/*V2::File cf;
		int a = cf.OpenWrite( L"capture_reki.txt", false, V2::File::TYP::ASCII );
		cf.SeekToEnd();
	

		if ( dynamic_cast<D2DControl*>(p) )
		{
			FString s = FString::Format(L"SetCapture %x %s", p, ((D2DControl*) p)->name_.c_str());
			cf.WriteString( s, true );
			ATLTRACE( L"%s\n", s.c_str() );
		}
		else 	
		{
			FString s = FString::Format(L"SetCapture %x %s", p, L"---" );
			cf.WriteString(s, true);
			ATLTRACE(L"%s\n", s.c_str());
		}

		if (!capture_obj_.empty() && p == capture_obj_.top())
		{
			cf.WriteString(L"却下", true);
			ATLTRACE(L"%s\n", L"却下");
		}


		cf.Close();*/
	}
	#endif

	if (capture_obj_.empty() )
		capture_obj_.push(p);
	else if ( p != capture_obj_.top())
		capture_obj_.push(p);

	if ( pt )
		capture_pt_ = *pt; //.push(*pt);
	else
		capture_pt_ = (FPointF(0,0));



	//auto pw = dynamic_cast<D2DControls*>(p);// dynamic_cast<D2DCaptureWindow*>(p);
	//if ( pw )
	//{
	//	// vectorの順番を変更する
	//	pw->MeToLast();
	//}
}
D2DCaptureObject* D2DWindow::ReleaseCapture()
{
	auto p = capture_obj_.top();


	capture_obj_.pop();		

	HWND hWnd = ::GetCapture();

	if ( capture_obj_.empty())
	{		
		::ReleaseCapture();
	}

	::SetFocus( hWnd );
	redraw_ = 1;
	

	return p;
}
////////////////////////////////////////////////////////////////////////////////////////////
CHDL ControlHandle::handle_ = 800; // initial value

CHDL ControlHandle::CreateControlHandle(D2DControl* ctrl)
{	
	handle_++;

	m1_[handle_] = ctrl;
	m2_[ctrl->GetName()] = handle_;
	return handle_;
}
void ControlHandle::DeleteControlHandle( CHDL id )
{
	if ( id == 0 ) return;

	xassert( m1_.find(id) != m1_.end());

	D2DControl* ctrl = m1_[id];
		
	m1_[id] = NULL;
	m2_[ctrl->GetName()] = 0;

}
D2DControl* ControlHandle::GetFromControlHandle(CHDL id )
{
	if ( m1_.find(id) != m1_.end())
		return m1_[id];

	return NULL;
}
D2DControl* ControlHandle::GetFromControlName(LPCWSTR nm )
{
	if ( m2_.find(nm) != m2_.end())
	{
		return GetFromControlHandle(m2_[nm]);
	}
	return NULL;
}

