#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "MoveTarget.h"

using namespace V4;

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region D2DTitleBar
// D2DTitleBar::D2DTitleBar(){}

void DrawTitleBar(D2DContext& cxt, const FRectF& titlebar, bool iscapture, bool dlg)
{
	CComPtr<ID2D1LinearGradientBrush> br;

	if (iscapture )
	{
		if (dlg )
			CreateVerticalGradation(cxt.cxt, FSizeF(1, titlebar.Height()), D2RGBA(180, 120, 60, 100), D2RGBA(120, 180, 60, 255), &br);
		else
			CreateVerticalGradation(cxt.cxt, FSizeF(1, titlebar.Height()), D2RGBA(120, 180, 60, 100), D2RGBA(120, 180, 60, 255), &br);
	}
	else
		CreateVerticalGradation( cxt.cxt, FSizeF(1,titlebar.Height()), D2RGBA(200,100,200,0) , D2RGBA(200,100,200,200) , &br );

	FRectF rc = titlebar; //(1,1,titlebar );
	rc.Offset(1,0);

	DrawFill( cxt, rc, cxt.white );
	DrawFill( cxt, rc, br  );
	
	//DrawFillRect(cxt, rc, cxt.black, cxt.transparent, 1.0f);

	DrawFillRectEx2( cxt.cxt, rc, cxt.black, cxt.transparent, 1.0f, (0x1|0x2|0x8|0x10) );
		
	D2DContextText cxtt;
	cxtt.Init( cxt, 11,DEFAULTFONT_JP ); //DEFAULTFONT_HEIGHT_JP, DEFAULTFONT_JP);
	FString str = L"Diect2D Window";
	rc.left += 5;
	DrawCenterText(cxtt, cxt.black, rc, str.c_str(), str.length(), 0 );
}
void DrawButtons(D2DContext& cxt, const FRectF& titlebar)
{
	FRectF b1,b2,b3;
	b1.SetRect( 0,0,25,20 ); 
	b2.SetRect( 0,0,25,20 ); b2.Offset( 26,  0);
	b3.SetRect( 0,0,25,20 ); b3.Offset( 26+26, 0);

	CComPtr<ID2D1SolidColorBrush> br1,br2;
	cxt.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,100), &br1 );
	cxt.cxt->CreateSolidColorBrush( D2RGB(199,80,80), &br2 );

	D2DMatrix mat(cxt);			
	mat.PushTransform();

	mat.Offset( titlebar.right-(26+26+26+6),0 );

	DrawFill( cxt, b1, br1 );
	//FillRect( cxt, b2, br1 );
	//FillRect( cxt, b3, br2 );

	FRectF b1a( 8,12, 17, 14 );
	FRectF b2a( 8,5, 17, 7 ); b2a.Offset(26,0);
	
	DrawFill( cxt, b1a, cxt.black );
	DrawFill( cxt, b2a, cxt.black );
	FRectF b2b( 8,5, 17, 13 ); b2b.Offset(26,0);

	DrawRect( cxt, b2b, cxt.black,1 );

	mat.PopTransform();

}
void D2DTitleBar::OnCreate()
{
	D2DButtonGruop* p = new D2DButtonGruop();
	FRectF rc( 0,0,78,25 );
	p->CreateWindow( parent_, this, rc, 1,L"gpbtn",-1, 3 );
	//p->GetPlace_ = boost::bind( &D2DTitleBar::GetPlaceRightButton, this, _1 );
	p->GetPlace_ = std::bind( &D2DTitleBar::GetPlaceRightButton, this, std::placeholders::_1 );
	btngrp_ = p;
}
LRESULT D2DTitleBar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);
			
			mat.PushTransform();
			mat_ = mat; // 親が原点
			mat.Offset( rc_.left, rc_.top );

			DrawTitleBar( d->cxt_, rc_, parent_->CaptureIsInclude(parent_control_), false );
			
			
			SendMessageReverseAll(d, message, wParam, lParam);

			 mat.PopTransform();
		}
		break;
			
		case WM_D2D_BUTTON_CLICK:
		{
			if ( btngrp_ == (void*)wParam) 
				parent_control_->WndProc( d, WM_D2D_BUTTON_CLICK, (WPARAM)this, lParam );
		}
		break;
		default:
			r = SendMessageAll(d, message, wParam, lParam);
			break;
	}
	
	return r;
}
FPointF D2DTitleBar::GetPlaceRightButton(FSizeF sz )
{
	FPointF pt;

	FRectF rc = rc_.ZeroRect();

	pt.x = rc.right - sz.width-6;
	pt.y = rc.top;

	return pt;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region D2DButtonGruop

#define CLICKED 0x80 // 64個まで

void D2DButtonGruop::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id, int btncount )
{
	_ASSERT( btncount < CLICKED );
	
	D2DControl::CreateWindow( parent, pacontrol,rc,stat, name, id );
	btncnt_ = btncount;
	btnmode_ = 0;
}
LRESULT D2DButtonGruop::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);			
			mat.PushTransform();
			FPointF pt = GetPlace_( rc_.Size()); // ボタン位置を取得
			mat.Offset( pt.x, pt.y );

			mat_ = mat; 
			
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
				int w = rc_.Height();
				FRectF rc(0,0,w, rc_.Height() );
				for( int i = 0; i < btncnt_; i++ )
				{
					if ( rc.PtInRect( pt ) )
					{
						md = (i+1) | CLICKED;

						d->SetCapture(this);
						r = 1;
						break;
					}				
					rc.Offset( w+1, 0);				
				}					


				btnmode_ = md;	

			}
		}
		break;
		case WM_MOUSEMOVE:
		{			
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			int md = 0;

			if (btnmode_ & CLICKED )
			{
				md = btnmode_;
			}
			else if ( rc_.PtInRect( pt ) )
			{
				int w = rc_.Height();
				FRectF rc(0,0,w, rc_.Height() );
				for( int i = 0; i < btncnt_; i++ )
				{
					if ( rc.PtInRect( pt ) )
					{
						md = (i+1);
						break;
					}				
					rc.Offset( w+1, 0);				
				}				
			}
			
			

			if ( md != btnmode_ )
			{
				d->redraw_ = 1;
				btnmode_ = md;				
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( d->GetCapture()==this)
			{				
				r = 1;
				btnmode_ &= ~CLICKED;				
				d->ReleaseCapture();

				if ( rc_.PtInRect(pt))
				{
					int md=0;
					int w = rc_.Height();
					FRectF rc(0,0,w, rc_.Height() );
					for( int i = 0; i < btncnt_; i++ )
					{
						if ( rc.PtInRect( pt ) )
						{
							md = (i+1);
							break;
						}				
						rc.Offset( w+1, 0);				
					}			


					// click event
					if ( md == btnmode_ )
					{
						int btn_idx = md;
						
						parent_control_->WndProc( d, WM_D2D_BUTTON_CLICK, (WPARAM)this, btn_idx );

						TRACE( L"btn=%d\n", btnmode_ );
					}

				}

				btnmode_ = 0;
			}
		}
		break;
	}
	
	return r;
}
void D2DButtonGruop::DrawButton( D2DContext& cxt )
{
	
	auto br1 = MakeBrsuh(cxt, D2RGBA(54,101,179,100));
	auto br2 = MakeBrsuh(cxt, D2RGBA(54,101,179,255));

	int w = rc_.Height();
	FRectF rc(0,0,w, rc_.Height() );
	for( int i = 0; i < btncnt_; i++ )
	{		
		if ( (i+1) == (btnmode_ & (CLICKED-1))  )
		{
			if ( btnmode_ & CLICKED )
			{				
				DrawFill( cxt, rc, cxt.white );
			}
			else
				DrawFill( cxt, rc, br2 );
		}
		else
			DrawFill( cxt, rc, br1 );

		rc.Offset( w+1, 0);

	}
}
#pragma endregion
///////////////////////////////////////////////////////////////////////////////////////


#pragma region D2DDropedPack
//D2DControls* hostage_; // 人質

D2DDropedPack::~D2DDropedPack()
{
	int a = 0;
}
void D2DDropedPack::Pack( D2DControl* pc )
{ 
	xassert( pc );

	hostage_ = pc; 
	hostage_->OnHostage(this,true);
}
LRESULT D2DDropedPack::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	bool bl = true;

	if (!VISIBLE(stat_))
		return 0;
	
	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	if ( message == WM_PAINT )
	{		
		//mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
		mat_ = mat; // 親コントロールが原点			
		mat.Offset( rc_.left, rc_.top );
	}
	else
		mat = mat_;	// mat_と同じ値にする。

	switch( message )
	{
		case WM_PAINT:
		{			
			// 自座標　左上が(0,0)となる
			FRectF rc1 = rc_.ZeroRect();
			D2DRectFilter f(d->cxt_, rc1);	
			
			OnDraw(d->cxt_ );		
			
			
			//SendMessageReverseAll(d,message,wParam,lParam);	

			bl = false;
		}
		
		case WM_LBUTTONDOWN :
		{
			FPointF pt21 = mat_.DPtoLP(lParam); 			
			if ( rc_.PtInRect(pt21))
			{
				d->SetCapture(this);				
				d->CapturePoint(pt21);
				d->CaptureRect(rc_);
				ret = 1;
				bl = false;
			}
		}
		break;
		case WM_MOUSEMOVE :
		{
			if ( d->GetCapture() == this )
			{
				FPointF pt21 = mat_.DPtoLP(lParam);

				FPointF prv = d->CapturePoint(pt21);

				rc_.Offset( pt21.x-prv.x, pt21.y-prv.y );

				
				d->redraw_ = 1;
				ret = 1;
				bl = false;
			}
		}
		break;
		case WM_LBUTTONUP :
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();

				FPointF pt21 = mat_.DPtoLP(lParam);				
				if ( rc_.PtInRect(pt21))
				{
					D2D_OBJECT_UPDATE pm;
					pm.globalpt = FPointF(lParam);
					pm.object = this;
					pm.stat = D2D_OBJECT_UPDATE::TYP::TRY_NEWPARENT;

					::SendMessage(d->hWnd_,WM_D2D_OBJECT_UPDATE,0,(LPARAM)&pm);
					ret = 1;
					bl = false;


					// 同じ状態なので、失敗とみなし元の状態に戻す。
					if ( pm.stat == D2D_OBJECT_UPDATE::TYP::TRY_NEWPARENT )
						rc_ = d->CaptureRect( FRectFBoxModel());
					else if (pm.stat == D2D_OBJECT_UPDATE::TYP::SUCCESS )
					{
						hostage_->OnHostage(this,false);

						// もうD2DDropedPackは役割を終えたのでメモリから消す
						::PostMessage(d->hWnd_,WM_D2D_OBJECT_SUICIDE,0,(LPARAM)this);
				
					}
				}
			}
		}
		break;
		

	}

	
	mat.PopTransform();


	return ret;




}
void D2DDropedPack::OnDraw(D2DContext& cxt )
{
	FRectF rc1 = rc_.ZeroRect();
	DrawFillRect( cxt, rc1, cxt.black, cxt.white ,  1.0f );
}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////////
#pragma region D2DSlideControls

void D2DSlideControls::OnCreate()
{
	initial_rc_size_ = rc_.Size();
}

LRESULT D2DSlideControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_) || !IsOpen())
		return ret;

	D2DContext& cxt = d->cxt_;

	D2DMatrix mat(d->cxt_);		
	mat.PushTransform();
	
	switch( message )
	{
		case WM_PAINT:
		{
			D2DRectFilter filter(cxt, rc_ );

			DrawRect(cxt, rc_.GetBorderRect(), cxt.black, 1 );
			
			ret = D2DControls::WndProc(d,message,wParam,lParam);
		}
		break;

		default :
			ret = D2DControls::WndProc(d,message,wParam,lParam);
		break;		

	}
	

	mat.PopTransform();

	return ret;
}

void D2DSlideControls::CloseView(bool bVertical, int duration_time_ms)
{ 
	MoveTargetEx* mt = new MoveTargetEx();
	auto sz = initial_rc_size_;

	if ( bVertical )
		mt->SetParameter( sz.width, sz.height,sz.width, 0, duration_time_ms );		
	else
		mt->SetParameter( sz.width, sz.height,0, sz.height, duration_time_ms );		

	mt->Fire_ = [this](MoveTargetEx* mt, float w, float h)->bool
	{
		rc_.SetSize( FSizeF(w,h));
		return true;

	};
	mt->End_ = [this](MoveTargetEx* mt, float w, float h)
	{
		rc_.SetSize( FSizeF(w,h));
		delete mt;
	};
		
	mt->Start( this->parent_ );
}
void D2DSlideControls::OpenView(int duration_time_ms)
{
	MoveTargetEx* mt = new MoveTargetEx();
	
	auto sz = initial_rc_size_;
	auto sz1 = rc_.Size();
	mt->SetParameter( sz1.width, sz1.height, sz.width, sz.height, duration_time_ms );		

	mt->Fire_ = [this](MoveTargetEx* mt, float w, float h)->bool
	{
		rc_.SetSize( FSizeF(w,h));
		return true;

	};
	mt->End_ = [this](MoveTargetEx* mt, float w, float h)
	{
		rc_.SetSize( FSizeF(w,h));
		delete mt;
	};
		
	mt->Start( this->parent_ );
}
bool D2DSlideControls::IsOpen()
{
	auto sz = rc_.Size();
	return (sz.height > 0 && sz.width > 0);
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DStackPanel

void D2DStackPanel::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id  )
{
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id);
	

}
void D2DStackPanel::SetRect( const FRectF& rc )
{ 
	rc_.SetFRectF(rc); 
}

LRESULT D2DStackPanel::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	switch( message )
	{
		case WM_D2D_RESTRUCT_RENDERTARGET:
			//CreateResource(wParam == 1 );				
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
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

			DrawFill(cxt, rc_, cxt.basegray);

			mat.Offset( rc_ );
			
				SendMessageReverseAll(d,message,wParam,lParam);

			mat.PopTransform();
			return 0;
		}
		break;
		case WM_D2D_SIZE_FITTING:
		{
			if ( typ_ == TYP::VERTICAL || typ_ == TYP::CHILD_VERTICAL )
				OnSizeV();
			else
				OnSizeH();

		}
		break;
		case WM_SIZE :
		{
			FSizeF sz(lParam);
			if ( sz.width > 0 && sz.height > 0 )
			{
				if ( typ_ == TYP::VERTICAL || typ_ == TYP::CHILD_VERTICAL )
					OnSizeV();
				else
					OnSizeH();
			}
		}
		break;
	}

	if ( ret == 0 )
		ret = SendMessageAll(d,message,wParam,lParam);		

	return ret;
}

void D2DStackPanel::OnSizeV()
{
	auto selfrc = parent_control_->GetRect();
	if ( selfrc.bottom < 0 ) 
		return ; // 最小化すると、ここに来る
			
	float h = rc_.Height();

	if ( typ_ == TYP::VERTICAL )
		h = selfrc.GetContentRect().Height();


	std::vector<std::shared_ptr<D2DControl>> ar;
	for(auto it=controls_.begin(); it != controls_.end(); it++ )
	{
		if ( (*it)->IsVisible())
			ar.push_back( (*it) );
	}
			
	// 一番高いchildの高さを加減する
	// 横幅も合わせる

	float hc = 0, ht = 0;
	auto itt=ar.end();
	for(auto it=ar.begin(); it != ar.end(); it++ )
	{
		float a = (*it)->GetRect().Height();

		if ( a > hc )
		{
			hc = a;
			itt = it; // maxheight
		}
				
		ht += a;
	}

	if ( itt!=ar.end())
	{
		auto rc = (*itt)->GetRect();
				
		float off = ht - h;

		rc.bottom -= off;

		(*itt)->SetRect( rc );
	}

	// 位置を縦に整列
	float ypos = 0;
	for(auto it=ar.begin(); it != ar.end(); it++ )
	{
		auto rc = (*it)->GetRect();

		float h = rc.Height();
		rc.top = ypos;
		rc.bottom = ypos + h;

		if ( typ_ == TYP::VERTICAL )
		{
			rc.left = 0;
			rc.right = selfrc.right;
		}
		else if ( typ_ == TYP::CHILD_VERTICAL )
		{
			rc.left = 0;
			rc.right = rc_.Width();
		}

		(*it)->SetRect(rc);

		ypos = rc.bottom; //rc.Height();				
	}			
}
void D2DStackPanel::OnSizeH()
{
	auto selfrc = parent_control_->GetRect();

	if ( selfrc.bottom < 0 ) 
		return ; // 最小化すると、ここに来る

	if ( !auto_resize_ )
		selfrc = rc_; // rc_に関しては既に設定済み
			
	float wmax = selfrc.GetContentRect().Width();

	// 表示objectを集める
	std::vector<std::shared_ptr<D2DControl>> ar;
	for(auto it=controls_.begin(); it != controls_.end(); it++ )
	{
		if ( (*it)->IsVisible())
			ar.push_back( (*it) );
	}
			
	// 一番高いchildの高さを加減する
	// 横幅も合わせる

	float hc = 0, ht = 0;
	auto itt=ar.end();
	for(auto it=ar.begin(); it != ar.end(); it++ )
	{
		float a = (*it)->GetRect().Width();

		if ( a > hc )
		{
			hc = a;
			itt = it; // maxwidth
		}
				
		ht += a;
	}

	if ( itt!=ar.end())
	{
		auto rc = (*itt)->GetRect();
				
		float off = ht - wmax;

		rc.right -= off;

		(*itt)->SetRect( rc );
	}

	// 位置を横に整列
	float xpos = 0;
	for(auto it=ar.begin(); it != ar.end(); it++ )
	{
		auto rc = (*it)->GetRect();

		float w = rc.Width();
		rc.left = xpos;
		rc.right = xpos + w;
		rc.top = 0;
		rc.bottom = rc_.Height();// selfrc.bottom;//selfrc.top;

		(*it)->SetRect(rc);

		xpos = rc.right; //rc.Width();				
	}			
}

#pragma endregion