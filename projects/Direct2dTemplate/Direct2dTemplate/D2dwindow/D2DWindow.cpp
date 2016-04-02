#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "d2dapi.h"

#include "gdi32.h"
#include <Shellapi.h>

#include "MoveTarget.h"
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
			
			::SetWindowLongPtr( hWnd, GWL_USERDATA,(LONG) st->lpCreateParams ); // GWL_USERDATA must be set here.


			SetFocus(hWnd);

			#if _DEBUG
			::remove("capture_reki.txt");
			#endif

			return 0; // DefWindowProc(hWnd, message, wParam, lParam);
		}		
		break;
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);					
			ID2D1RenderTarget* cxt = d->cxt_.cxt;			
			
			SelectResource( d->res_ );

			cxt->BeginDraw();
			D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();

			//mat._11 = mat._11*2.3; 
			//mat._22 = mat._22*2.3;

			cxt->SetTransform(mat);
			cxt->Clear(ColorF(ColorF::White));

			d->WndProc( message, wParam, lParam ); // All objects is drawned.

					
			//D2DImage img;
			//img.id = 0;
			//DrawBitmap( d->cxt_.cxt, img, FRectF(100+0,100+0,100+48,100+48));

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


			// ここがアニメーションの計算場所
			for(auto it = d->mts_.begin(); it!=d->mts_.end(); )
			{					
				bool isend;
				MoveTargetEx* mtx = (MoveTargetEx*)it->first;
				mtx->UIFire(d, isend);

				if (isend)
					d->mts_.erase(it++); // set map流のerase方法らしい			
				else
					it++;
			}

#ifdef BOXMODELTEST
{
	auto cxttemp = d->cxt_;
	FRectFBoxModel rcx(200,200,FSizeF(100,100));

	rcx.Margin_.Set(10);
	rcx.BoderWidth_ = 10;
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

			if (d->redraw_)
			{
				InvalidateRect(hWnd, NULL, FALSE);
				d->redraw_ = 0;
			}


			auto hr = NoThrowIfFailed(cxt->EndDraw());

			if ( D2DERR_RECREATE_TARGET == hr )
			{
				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 0,0 ); // RenderTargetにリンクしている物はすべてリリース

				d->ResourceCreate(false);

				// ディスプレイの解像度を変更したり、ディスプレイ アダプターを取り外したりすると、デバイスが消失する可能性があります
				d->cxt_.DestroyRenderTargetResource();

				#ifdef USE_ID2D1DEVICECONTEXT
					d->cxt_.CreateDeviceContextRenderTarget( hWnd );
				#else
					d->cxt_.CreateHwndRenderTarget( hWnd );
				#endif
				d->cxt_.CreateRenderTargetResource( d->cxt_.cxt );

				d->ResourceCreate(true);
				d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 1,0 );// RenderTargetに再リンク

				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if ( hr != S_OK )
			{
				// 88990015 リソースが誤ったレンダターゲットを使用
				
				D2DError( hr, L"EndDraw error", __LINE__, __FILE__ );

			}



			EndPaint(hWnd, &ps);

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

						// capture時めーっセージが回らないかもしれない
			TRACE( L"%d %d %d %d %x\n" , cx, ncx, cy, ncy, d->GetCapture());

						d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 0,0 ); // RenderTargetにリンクしている物はすべてリリース
						d->ResourceCreate(false);

						d->cxt_.DestroyRenderTargetResource();
						d->cxt_.CreateDeviceContextRenderTarget( hWnd );
						d->cxt_.CreateRenderTargetResource( d->cxt_.cxt );
						
						d->ResourceCreate(true);
						d->WndProc( WM_D2D_RESTRUCT_RENDERTARGET, 1,0 );// RenderTargetに再リンク
					}
				}
				#else
				{
					CComPtr<ID2D1HwndRenderTarget> trgt;
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

		case WM_MOUSEWHEEL:// マウスホイール	
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
		//case WM_MOUSEHWHEEL:
		case WM_CHAR:
		{
			if ( message == WM_CHAR )
			{
				int a = 0;
			}

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
			std::vector<D2D_DROPOBJECT> ar;			
			{
				HDROP hd = (HDROP)wParam;
				UINT cnt = DragQueryFile(hd,-1,nullptr,0);
				ar.resize(cnt);
				for( UINT i = 0; i < cnt; i++ )
				{
					WCHAR cb[MAX_PATH];
					POINT pt;
					DragQueryFile(hd,i,cb,MAX_PATH);
					DragQueryPoint(hd, &pt);

					D2D_DROPOBJECT it;
					it.filename = cb;
					it.pt = pt;
					ar[i] = it;
				}
				DragFinish( hd );
			}						
			d->WndProc( WM_D2D_DRAGDROP, (WPARAM)&ar,0 );
			

			if ( d->redraw_ )
			{
				InvalidateRect( hWnd, NULL, FALSE );				
				d->redraw_ = 0;
			}

			return 0;
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
	wcex.hInstance		= hInstance; //::GetModuleHandle(0);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
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

	ResourceCreate(false); 

	while ( !capture_obj_.empty())
		capture_obj_.pop();
	
	children_ = NULL;
}

HWND D2DWindow::CreateD2DWindow( DWORD dwWSEXSTYLE, HWND parent, DWORD dwWSSTYLE, RECT rc, UINT* img_resource_id, int img_cnt )
{
	_ASSERT( dwWSSTYLE & WS_CHILD );
	_ASSERT( parent );


	if ( img_cnt > 0 )
	{		
		for( int i = 0; i < img_cnt; i++ )
		{
			UINT id = img_resource_id[i];
			BitmapBank_[id] = nullptr;
		}
	}

	dwWSSTYLE |=WS_CLIPCHILDREN;

	HWND h = parent;
	HWND h2 = parent;

	while( h )
	{
		h2 = h;
		h = ::GetParent(h2);
	}

	hMainFrame_ = h2;

	if( rc.top == rc.bottom )
		rc.bottom = rc.top+1;
	if (rc.right == rc.left)
		rc.right = rc.left+ 1;
	
	hWnd_ =  ::CreateWindowExW( dwWSEXSTYLE, CLASSNAME, L"", dwWSSTYLE, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, parent, NULL, ::GetModuleHandle(0), this ); 	

	try 
	{
		cxt_.Init( SingletonD2DInstance::Init() );

		#ifdef USE_ID2D1DEVICECONTEXT
			cxt_.CreateDeviceContextRenderTarget( hWnd_ );
		#else
			cxt_.CreateHwndRenderTarget( hWnd_ );
		#endif

		cxt_.CreateRenderTargetResource( cxt_.cxt );
		cxt_.CreateResourceOpt();


		ResourceCreate(true);
	}
	catch( D2DError er)
	{
		throw er;

	}

	cxt_.cxtt.Init( cxt_,DEFAULTFONT_HEIGHT_JP, DEFAULTFONT_JP ); // default font, default font size

	res_ = ResourceAllocate( cxt_.cxt, SingletonD2DInstance::Init().wrfactory, hWnd_ );
	SelectResource( res_ );
	
	DragAcceptFiles( hWnd_, TRUE );

	redraw_ = 0;
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

	// WM_PAINT, WM_SIZE, WM_D2D_RESTRUCT_RENDERTARGETはCAPTUREに関わらず、すべてにSENDすること

	if ( message == WM_PAINT  && children)
	{
		children->WndProc(this,WM_PAINT,wParam,lParam);

		if ( tooltip_.IsShow())
			tooltip_.Draw();
		if (roundpaint_obj_ )
			roundpaint_obj_->WndProc(this, message, wParam, lParam);
	}
	else if (message == WM_SIZE && children)
	{
		children->WndProc(this,message,wParam,lParam);
	}
	else if ( message == WM_D2D_RESTRUCT_RENDERTARGET )
	{
		if ( wParam == 0 )
		{
			if ( children )
				children->WndProc(this,message,wParam,lParam);

			ResourceRelease( res_ );
		}					
		else if ( wParam == 1 )
		{
			res_ = ResourceAllocate( cxt_.cxt, SingletonD2DInstance::Init().wrfactory, hWnd_ );
			SelectResource( res_ );

			if ( children )
				children->WndProc(this,message,wParam,lParam);
		}
	}
	else if ( !capture_obj_.empty() && message < WM_D2D_EV_FIRST )
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
			children_->DestroyControl();
			children_ = NULL;

			Clear();

			if ( OnDestroy )
				OnDestroy( this );

			cxt_.DestroyAll();
			ResourceRelease( res_ );
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

	if ( mat )
		capture_matrix_ = *mat;

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
	roundpaint_obj_ = nullptr;

	
	if ( dynamic_cast<D2DButton*>(p) )
	{
		int a = 0;

	}


#ifdef _DEBUG
	{
		/*V2::File cf;
		cf.OpenWrite(L"capture_reki.txt", false, V2::File::TYP::ASCII);
		cf.SeekToEnd();

		if (dynamic_cast<D2DControl*>(p))
		{
			FString s = FString::Format(L"ReleaseCapture %x %s", p, ((D2DControl*) p)->name_.c_str());
			cf.WriteString(s, true);
			ATLTRACE(L"%s\n", s.c_str());
		}
		else
		{
			FString s = FString::Format(L"ReleaseCapture %x %s", p, L"---");
			cf.WriteString(s, true);
			ATLTRACE(L"%s\n", s.c_str());
		}
		cf.Close();*/
	}
#endif

	capture_obj_.pop();		

	HWND hWnd = ::GetCapture();

	if ( capture_obj_.empty())
	{		
		::ReleaseCapture();
	}

	::SetFocus( hWnd );
	redraw_ = 1;
	

//auto pw = dynamic_cast<D2DControls*>(p);// dynamic_cast<D2DCaptureWindow*>(p);
//if ( pw )
//{
//	// vectorの順番を変更する
//	pw->MeToOrg();
//}

	return p;
}

void V4::SetCursor( HCURSOR h )
{
	if ( h != ::GetCursor() )
	{	
		::SetCursor( h );
	}
}
void Inner::ToolTip::Set( D2DControl*p, LPCWSTR str )
{ 
	str_ = str; ctrl_ = p; 
	
	isShow_ = true;
	
	::GetCursorPos(&pt_); // マウスの位置

	ScreenToClient(ctrl_->parent_->hWnd_, &pt_ );


	rc_.SetRect(0,0,200,30);

	rc_.Offset( pt_.x, pt_.y+20 ); // 20 cursorの高さサイズ

}

void Inner::ToolTip::Draw()
{
	FPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(ctrl_->parent_->hWnd_, &pt );

	if ( pt.x == pt_.x && pt.y == pt_.y )
	{	
		auto cxt = ctrl_->parent_->cxt_;
		D2DMatrix mat(cxt);
		mat.PushTransform();

		mat._31 = 0;
		mat._32 = 0;

		mat.SetTransform();

		cxt.SetAntiAlias(false );


		FSizeF sz = CalcText( cxt.cxtt.textformat, str_, str_.length() );

		FRectF rc;
		rc.SetPoint( rc_.LeftTop());
		rc.SetSize(sz);		
		rc.InflateRect(3,3);		
		
		FRectFBoxModel rcm = rc;
		
		rcm.Padding_.Set(3);

		xassert( cxt.tooltip );
		DrawFillRect(cxt,rcm.GetBorderRect(),cxt.black, cxt.tooltip,1);
	
		DrawCenterText( cxt, cxt.black,rcm.GetContentRect(), str_, str_.length(), 0 );


		cxt.SetAntiAlias(true );

		mat.PopTransform();
	}
	else
		Show(false);

}
void D2DWindow::ShowToolTip( D2DControl* p, LPCWSTR message )
{
	tooltip_.Set( p, message );	

}

DWORD D2DRGBADWORD_CONV(D2D1_COLOR_F clr)
{
	DWORD r = ROUND(clr.r * 255);
	DWORD g = ROUND(clr.g * 255);
	DWORD b = ROUND(clr.b * 255);
	DWORD a = ROUND(clr.a * 255);

	return D2DRGBADWORD(r,g,b,a );
}

ID2D1SolidColorBrush* D2DWindow::GetSolidColor(D2D1_COLOR_F clr)
{
	DWORD d = D2DRGBADWORD_CONV(clr );

	auto& it = SolidColorBank_.find(d);
	if (it != SolidColorBank_.end())
		return it->second;

	auto r = MakeBrsuh(cxt_, clr).Detach();
	SolidColorBank_[d] = r;
	return r;
}
bool D2DWindow::GetResourceImage( UINT id, ID2D1Bitmap** bmp )
{
	auto it = BitmapBank_.find(id);
	if ( it != BitmapBank_.end())
	{
		*bmp = it->second;
		(*bmp)->AddRef();
		return true;
	}
	*bmp = nullptr;
	return false;
}

void D2DWindow::ResourceCreate(bool bCreate)
{
	if ( !bCreate )
	{
		ULONG cn;
		for( auto& it : SolidColorBank_ )
		{
			cn = it.second->Release();
			_ASSERT( cn == 0 );


		}
		for( auto& it : BitmapBank_ )
		{			
			cn = it.second->Release();
			_ASSERT(cn == 0);
		}
		
		// after this, recreate RenderTarget.	
	}
	else
	{
		for (auto& it : SolidColorBank_)
		{
			DWORD dw = it.first;
			auto clr = D2DRGBA(dw);
			SolidColorBank_[ dw ] = MakeBrsuh( cxt_, clr ).Detach();
		}

		for (auto& it : BitmapBank_)
		{
			DWORD id = it.first;
			
			CComPtr<ID2D1Bitmap> bmp;
			if ( V4::LoadResourceImage(cxt_, L"PNG", id, &bmp))
			{
				BitmapBank_[ id ] =  bmp.Detach();
			}
		}
	}
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
	if ( id == 0 || m1_.empty() ) return;

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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
