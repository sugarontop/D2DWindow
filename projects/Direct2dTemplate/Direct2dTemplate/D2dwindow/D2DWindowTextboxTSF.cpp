﻿#include "stdafx.h"
#include "D2DWindowControl.h"
#include "tsf\TextEditor.h"
#include "tsf\TextLayout.h"
#include "D2DCharRect.h"

//#ifdef TEXTBOXTEST
//	#include "s1.h"
//#endif

#define TAB_WIDTH_4CHAR 4

using namespace TSF;
using namespace V4;

#define LEFT_MARGIN 3.0f

LPCWSTR regexpre[] = { 
	/*
		#include <regex>
		std::wregex re(regexpre[0]);	
		bool bl = std::regex_match( L"ABCabc", re );

	*/

	L"[A-Za-z]+",									// 1文字以上 英字 
	L"[A-Za-z0-9]+",								// 1文字以上 英数字 
	L"[0-9]+",										// 1文字以上 数字 
	L"^[-+0-9][0-9]{0,14}" ,						// 1文字以上 数字１５桁まで 
	L"^[-+0-9][0-9]{0,14}[.]{1}[0-9]{1,4}",			// 1文字以上 数字　ドット １文字　小数点４桁まで
	L"[0-9]{2,4}[./ ]{1}[0-9]{1,2}[./ ]{1}[0-9]{1,4}"// 日付用 
};

namespace V4 {

D2DTextbox::D2DTextbox(CTextEditorCtrl* ctrl)
{
	if (!ctrl)
	{
		ctrl = TSF::GetTextEditorCtrl();
	}
	
	clr_border_ = ColorF(ColorF::Black);
	clr_fore_ = ColorF(ColorF::Black);
	clr_back_ = ColorF(ColorF::White);
	clr_active_border_ = ColorF(ColorF::Blue);

	typ_ = TYP::SINGLELINE;
	ct_.bSingleLine_ = true;
	ctrl_ = ctrl;
	bActive_ = false;
	bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.LimitCharCnt_ = 65500;
	font_height_ = 0;	
}
D2DTextbox::D2DTextbox(CTextEditorCtrl* ctrl,TYP typ)
{
	if (!ctrl)
	{
		ctrl = TSF::GetTextEditorCtrl();
	}


	if ( typ & RIGHT || typ & CENTER || typ & VCENTER )
		typ = (D2DTextbox::TYP)(typ | TYP::SINGLELINE);

	clr_border_ = ColorF(ColorF::Black);
	clr_fore_ = ColorF(ColorF::Black);
	clr_back_ = ColorF(ColorF::White);
	clr_active_border_ = ColorF(ColorF::Blue);


	typ_ = typ;
	ctrl_ = ctrl;
	bActive_ = false;
	bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.bSingleLine_ = ( typ_ != TYP::MULTILINE );
	ct_.LimitCharCnt_ = 65500;	

}
void D2DTextbox::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );
	
	if ( fmt_ )
	{
		CComPtr<IDWriteTextLayout> tl;		
		if ( S_OK == parent_->cxt_.wfactory->CreateTextLayout( L"T", 1, fmt_, 1000,1000,  &tl ))
		{
			DWRITE_HIT_TEST_METRICS mt;
			float y = 0, xoff=0;
			tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );
			
			font_height_ = mt.height;			
		}
	}
	else
	{
		font_height_ = parent->cxt_.cxtt.line_height;

	}
	 
	// 右隅のPadding へスクロールバー
	if ( 15 <= rc.Padding_.r )
	{	
		if ( typ_ & TYP::MULTILINE )
		{	
			float h = rc.GetContentRect().Height();	// v or hの判断にしか使われない。wm_sizeでのサイズで決まる。		
			float w = rc.Padding_.r;

			FRectFBoxModel rcscb(rc_.GetContentRect().Width(), 0, FSizeF(w, h));
			scbar_ = std::shared_ptr<D2DScrollbar>(new D2DScrollbar());
			scbar_->CreateWindowEx(parent_, this, rcscb, VISIBLE | ALWAYSDRAW, NONAME);
			scbar_->SetTotalSize(1);
		}
	}


	SetText(L"");
}
IDWriteTextFormat* D2DTextbox::GetFormat()
{
	return fmt_;
}
void D2DTextbox::SetFont( CComPtr<IDWriteTextFormat> fmt )
{
	fmt_ = fmt;

	CalcRender(true);

}

// 文字のRECT情報を計算し、文字レイアウトを取得
void D2DTextbox::CalcRender(bool bLayoutUpdate)
{
	if ( fmt_ )
	{
		IDWriteTextFormat* old = NULL;

		old = parent_->cxt_.text;
		parent_->cxt_.text = fmt_;

		ctrl_->SetContainer( &ct_, this ); 
		ctrl_->mat_ = mat_;	
		ctrl_->CalcRender( parent_->cxt_ );


		parent_->cxt_.text = old;
	}
	else
	{
		ctrl_->SetContainer( &ct_, this ); 
		ctrl_->mat_ = mat_;	
		ctrl_->CalcRender( parent_->cxt_ );
	}

	if ( bLayoutUpdate )
	{
		text_layout_.Release();
		ctrl_->GetLayout()->GetTextLayout( &text_layout_ ); // ->DWTextLayout_;
	}
}


LRESULT D2DTextbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	//if ( !VISIBLE(stat_))
	//	return ret;
	if ( !IsThroughMessage(message))
		return 0;

	switch ( message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;										
			D2DMatrix mat( cxt );		

			FRectF rccnt = rc_.GetContentRect();
		
			mat.PushTransform(); // 0
			mat_ = mat;


			FRectF rc1 = rc_.GetBorderRect(); 
		
			D2DRectFilter f(cxt, rc1);

			{
				auto br1 = MakeBrsuh( cxt, clr_back_ );
				DrawFill(cxt,rc1,br1 );// 裏面
			}
			
			auto wakucolor = MakeBrsuh( cxt,clr_border_ );
			auto forecolor = MakeBrsuh( cxt,clr_fore_ );

			ID2D1Brush* textcolor = ( stat_&READONLY ? cxt.bluegray.p : forecolor.p );
			
			mat.PushTransform(); // 1
			{			
				mat.Offset( rccnt.left + LEFT_MARGIN , rccnt.top );		
				mat.PushTransform(); // 2
				{
					float hh = ( scbar_.get()==nullptr ? 0.0 : RowHeight() *  scbar_->info_.rowno);

					mat.Offset( 0, -hh ); 
					matEx_ = mat;

					{		
						mat.PushTransform(); // 3
						int len = ct_.GetTextLength();
			
						if ( len && !bActive_ )
						{	
							if ( typ_ & MULTILINE )
								cxt.cxt->DrawTextLayout(offpt_, text_layout_, textcolor );
							else
							{
								float yoff = ( rccnt.Height()-font_height_ ) / 2.0f; // vcenter
								mat.Offset(0,yoff );
								cxt.cxt->DrawTextLayout(offpt_, text_layout_, textcolor );
							}
						}
						else if( ctrl_->GetContainer() == &ct_ && bActive_ )
						{
							ctrl_->mat_ = mat_;
							
							// on editting, bActive_ is true.

							IDWriteTextFormat* old = NULL;
							float oldheight;

							if ( fmt_ == NULL )
								fmt_ = parent_->cxt_.text;
							else
							{
								old = parent_->cxt_.text;
								parent_->cxt_.text = fmt_;
								oldheight = parent_->cxt_.cxtt.line_height;
								parent_->cxt_.cxtt.line_height = font_height_;
							}
					
							if ( !(typ_ & MULTILINE) )
							{
								float yoff = ( rccnt.Height()-font_height_ ) / 2.0f; // vcenter
								mat.Offset(0,yoff );	
							}
						
							ret = ctrl_->WndProc( d, WM_PAINT, wParam, lParam );

							if ( old )
							{
								parent_->cxt_.text = old;
								parent_->cxt_.cxtt.line_height = oldheight;

							}
							wakucolor = MakeBrsuh( cxt, clr_active_border_ ); 
						}					
						mat.PopTransform(); // 3
					}			
				}
				mat.PopTransform(); // 2
			}

			if ( scbar_ )
			{
				//FRectF rccc = scbar_->GetRect();				
				mat.Offset( -LEFT_MARGIN , -rc_.Padding_.t );	
				scbar_->WndProc(d,WM_PAINT,wParam,lParam);
			}

			mat.PopTransform(); // 1

			if ( stat_ & BORDER )
			{
				//D2DRectFilter f(cxt, rc1);
				DrawFillRect( cxt, rc1, wakucolor, cxt.transparent,1.0f );
			}

			if ( stat_ & DISABLE )
			{
				DrawFill( cxt, rc1, cxt.halftone );
			}

			mat.PopTransform();	//0	

			if ( bUpdateScbar_ )
			{
				if (TryTrimingScrollbar())
					scbar_->info_.rowno = 0;
				d->redraw_=1;
				bUpdateScbar_  =false;
			}

			if ( bActive_ )
			{
				#ifndef TEXTBOXTEST
				d->redraw_=1;
				#endif
			}
		}
		break;
		default :
		{	
			if ( WM_D2D_APP_USER <= message )
			{
				if ( OnWndProcExt_ )
					ret = OnWndProcExt_( this, message, wParam, lParam );
			}

			if ( (stat_ & READONLY) || (stat_ & DISABLE) ) 
				return 0;
				
			if ( EventMessageHanler_ && EventMessageHanler_(this, message,wParam,lParam))
				return 1;

			
			switch( message )
			{
				case WM_LBUTTONDOWN:
				{
					FPointF pt = mat_.DPtoLP( FPointF(lParam));		
										
					
					if ( rc_.GetContentRect().PtInRect(pt) )
					{						
						d->SetCapture( this, &pt );

						StatActive(true);
						ret = 1;	
									
					}
					else if ( d->GetCapture() == this )
					{
						d->ReleaseCapture();
						StatActive(false);

						ret = LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS;
					}
				}
				break;
				case WM_KEYDOWN:
				{			
					ret = OnKeyDown(d,message,wParam,lParam);

					if ( ret )
						return ret;

					ret = parent_control_->KeyProc(this,WM_KEYDOWN,wParam,lParam );

					if ( ret )
						return ret;
				}
				break;
				case WM_RBUTTONUP:
				{
					ret = parent_control_->WndProc(d,WM_D2D_TEXTBOX_FLOAT_MENU,wParam,lParam ); // parent_controlはbproecssをFALSEにすること
					if ( ret )
						return ret;
				}
				break;


			}




			if ( scbar_ && 0 != scbar_->WndProc(d,message,wParam,lParam ) )
				return 1;

			if (ctrl_->GetContainer() != &ct_ || d->GetCapture() != this)
				return ret;

			int bAddTabCount = 0;
			
			if ( message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEMOVE  
				|| message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_LBUTTONDBLCLK )			
			{					
				FPointF npt = matEx_.DPtoLP( FPointF(lParam) );
				lParam = MAKELONG( (WORD)npt.x, (WORD)npt.y );				
			}
			else if ( message == WM_CHAR && (WCHAR)wParam == L'\r' )
			{
				// top column, tab count.

				bAddTabCount = TabCountCurrentRow();
			}

			ret = ctrl_->WndProc( d, message, wParam, lParam ); // WM_CHAR,WM_KEYDOWNの処理など

			// add tab
			for( int i = 0; i < bAddTabCount; i++ )
				OnKeyDown(d,WM_KEYDOWN,(WPARAM)L'\t',0);

			// scroll
			if ( message == WM_KEYDOWN && (wParam == VK_DOWN || wParam == VK_UP || wParam == VK_RETURN ) )
			{
				bool bl = TryTrimingScrollbar();				

				if ( wParam == VK_RETURN && bl )
					scbar_->info_.rowno++;
			}
			else if (message == WM_MOUSEMOVE && d->GetCapture() == this && ret == 0)
			{
				// mouse auto sctoll.
				bool bl = TryTrimingScrollbar();
			}
		}
		break;	
	}
	return ret; 
}


bool D2DTextbox::TryTrimingScrollbar()
{
	if ( scbar_ == nullptr ) return false;
	if ( ctrl_->ct_ == nullptr ) return false;

	FRectF rc;
	int pos = ctrl_->CurrentCaretPos();				
	int rowno = scbar_->info_.rowno;
	float cy;

	if ( rowno )
	{
		while(1)
		{
			cy = RowHeight() *  scbar_->info_.rowno;
			ctrl_->GetLayout()->RectFromCharPos( pos, &rc );	
					
			if ( rc.top < cy )
				scbar_->info_.rowno--;
			else
				break;
		}
	}

	while(1)
	{
		cy = RowHeight() *  scbar_->info_.rowno + rc_.GetContentRect().Height();
		if ( ctrl_->GetLayout()->RectFromCharPos( pos, &rc ))
		{		
			if ( rc.bottom > (cy-RowHeight()) )
				scbar_->info_.rowno++;
			else
				break;
		}
		else
			break;
	}
	
	bool ret = (rowno != scbar_->info_.rowno);

	if ( ret )
	{
		// Thumbボタンの位置調整

		scbar_->SetTotalSize(RowHeight()*RowCount(true));

		FRectF rc = scbar_->info_.rc;
		FRectF rcb = scbar_->info_.thumb_rc; // 真ん中のボタン

		float h2 = scbar_->info_.button_height; // 上下のボタン
		float h = rc.Height() - h2*2 - rcb.Height(); // thumbの移動範囲

		int cnt = RowCount(true)-RowCount(false);
		if ( cnt )
			scbar_->info_.position = scbar_->info_.rowno * h / cnt;

	}


	return ret;
}

int D2DTextbox::TabCountCurrentRow()
{
	int bAddTabCount = 0;

	LPCWSTR s = ct_.GetTextBuffer();
	int pos = max(0, ct_.nSelEnd_ - 1 );

	while(pos)
	{
		if ( s[pos] == L'\n' || s[pos] == L'\r')
			break;

		if ( s[pos] == L'\t' )
			bAddTabCount++;
		else
			bAddTabCount = 0;

		pos--;
	}				
	return bAddTabCount;

}
int D2DTextbox::OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	bool heldShift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
    bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
	switch( wParam )
	{
		case 'C':		
			ret =  (heldControl && Clipboard( d->hWnd_, L'C' ) ? 1 : 0 );
		break;
		case 'X':		
			ret =  (heldControl && Clipboard( d->hWnd_, L'C' ) ? 1 : 0 );
			if ( ct_.nSelEnd_ > ct_.nSelStart_ )
			{
				ct_.RemoveText( ct_.nSelStart_, ct_.nSelEnd_-ct_.nSelStart_ );
				ct_.nSelEnd_ = ct_.nSelStart_;
			}
		break;
		case 'V':
			ret =  (heldControl && Clipboard( d->hWnd_, L'V' ) ? 1 : 0 );			 
		break;
		case 'Z':
		break;
		case VK_INSERT:				
			ret =  (heldShift && Clipboard( d->hWnd_, L'V' ) ? 1 : 0 );
		break;
		case VK_RETURN:
		case VK_TAB:			
			if ( OnPushKey_ )
				ret = OnPushKey_( this, (UINT)wParam );			
		break;

		default :
			if ( OnPushKey_ )			
				ret = OnPushKey_( this, (UINT)wParam );			
		break;

	}

	return ret;
}
BOOL D2DTextbox::Clipboard( HWND hwnd, TCHAR ch )
{
	if ( !OpenClipboard( hwnd ) ) return FALSE;
	
	if ( ch == L'C' ) // copy
	{
		int SelLen = ct_.nSelEnd_-ct_.nSelStart_;
		if ( ctrl_->GetContainer() == &ct_ &&  SelLen > 0 )
		{
			std::unique_ptr<WCHAR[]> xcb( new WCHAR[SelLen+1] );
			WCHAR* cb = xcb.get();

			ct_.GetText(ct_.nSelStart_, cb, SelLen );
			cb[SelLen]=0;

			int cch = SelLen;
			
			HANDLE hglbCopy = GlobalAlloc(GMEM_DDESHARE, (cch + 1) * sizeof(WCHAR)); 
			if (hglbCopy == NULL) 
			{ 
				CloseClipboard(); 
				return FALSE; 
			} 
	 
			// Lock the handle and copy the text to the buffer. 
	 
			WCHAR* lptstrCopy = (WCHAR*)GlobalLock(hglbCopy); 
			memcpy(lptstrCopy, cb, cch * sizeof(WCHAR)); 
			lptstrCopy[cch] = 0; 
			GlobalUnlock(hglbCopy); 
	        
			// Place the handle on the clipboard. 
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hglbCopy); 

		}
		else
		{
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, NULL);		
		}
	}
	else if ( ch == L'V' ) // paste
	{
		HANDLE h = GetClipboardData( CF_UNICODETEXT );
		if ( h )
		{
			LPWSTR s1a = (LPWSTR)GlobalLock( h );
			auto s1b = FilterInputString( s1a, lstrlen(s1a) );

			UINT s1 = ct_.nSelStart_;
			UINT e1 = ct_.nSelEnd_;


			UINT rcnt;
			ct_.InsertText( ct_.nSelEnd_, s1b.c_str(), s1b.length(), rcnt );
			ct_.nSelEnd_ += rcnt;
			ct_.nSelStart_ = ct_.nSelEnd_;

			GlobalUnlock( h );


			// 上書された分を削除
			if ( e1 - s1 > 0 )
				ct_.RemoveText(s1, e1-s1);


			bUpdateScbar_ = true;
			CalcRender(true);
		}	
	}	
	::CloseClipboard();
	return TRUE;

}

std::wstring D2DTextbox::FilterInputString( LPCWSTR s, UINT len )
{
	if ( typ_ != TYP::MULTILINE )
	{
		for(UINT i = 0; i < len; i++ )
		{
			if ( s[i] == '\r' || s[i] == '\n' )
			{
				len = i;
				break;
			}
		}

		std::wstring r( s, len );
		return r;
	}
	else
	{
		// \n->\r or \r\n->\r
		WCHAR* cb = new WCHAR[len+1];
		WCHAR* p = cb;
		for(UINT i = 0; i < len; i++ )
		{
			if ( s[i] == '\r' && s[i+1] == '\n' )
			{				
				*p++ = '\r';
				i++;
			}
			else if ( s[i] == '\n' )
				*p++ = '\r';
			else
				*p++ = s[i];
		}
		*p = 0;

		std::wstring r = cb;
		delete [] cb;

		return r;
	}
}

void D2DTextbox::SetViewText(LPCWSTR str)
{
	FString org = ct_.GetTextBuffer();

	SetText(str);
	

	UINT nrCnt; 
	ct_.Clear();
	ct_.InsertText( 0, org, org.length(), nrCnt );
	ct_.CaretLast();

	CalcRender(true);

}
void D2DTextbox::SetReadOnly(bool bReadOnly )
{
	if ( bReadOnly )
		stat_ |=  STAT::READONLY ;
	else
		stat_ &=  ~STAT::READONLY;
}
void D2DTextbox::SetText(VARIANT v)
{
	if ( v.vt == VT_BSTR )
		SetText( v.bstrVal );
	else
	{
		_variant_t dst;
		if ( HR(VariantChangeType( &dst, &v, 0, VT_BSTR )))
			SetText( dst.bstrVal );
	}
}
int D2DTextbox::InsertText( LPCWSTR str, int pos, int strlen)
{
	// pos < 0  is current postion.
	parent_->redraw_ = 1;

	if ( pos < 0 )
	{
		int zCaretPos = (ct_.bSelTrail_ ? ct_.nSelEnd_ : ct_.nSelStart_ );
		pos = zCaretPos;

	}
	
	if ( strlen < 0 )
		strlen = lstrlen(str);

	UINT nrCnt; 
	if ( ct_.InsertText( pos, str, strlen, nrCnt ))
	{
		ct_.nSelEnd_ += nrCnt;
		ct_.nSelStart_ += nrCnt;
	}
	bUpdateScbar_ = true;



	CalcRender(true);
	return 0;

}


void D2DTextbox::SetText(LPCWSTR str1)
{
	parent_->redraw_ = 1;
	bUpdateScbar_ = true;

	{
		FString s1b = FilterInputString( str1, lstrlen(str1) );
	
		UINT nrCnt; 
		ct_.Clear();
		ct_.InsertText( 0, s1b, s1b.length(), nrCnt );
		ct_.CaretLast();
	}	
	auto ct = ctrl_->GetContainer();
	auto bri = ctrl_->bri_;

	ActiveSw();

	ctrl_->SetContainer( ct, bri ); 
}

void D2DTextbox::ActiveSw()
{	
	IDWriteTextFormat* old = NULL;
	if ( fmt_ == NULL )
		fmt_ = parent_->cxt_.text;
	else
	{
		old = parent_->cxt_.text;
		parent_->cxt_.text = fmt_;
	}

	ctrl_->SetContainer( &ct_, this ); 
	ctrl_->mat_ = mat_;	
	ctrl_->CalcRender( parent_->cxt_ );

	{
		text_layout_.Release();
		ctrl_->GetLayout()->GetTextLayout( &text_layout_ );  // singlineの場合自動で、vcenterになる

		offpt_.y = offpt_.x = 0;

		if (typ_ & SINGLELINE) 
		{
			float topline_width = ctrl_->GetLayout()->GetLineWidth();	

			if (typ_ & RIGHT )
			{			
				offpt_.x = rc_.GetContentRect().Width() - topline_width;
			}
			else if ( typ_ & CENTER )
			{			
				offpt_.x = (rc_.GetContentRect().Width() - topline_width)/2.0f;
			}
		}

		ct_.offpt_ = offpt_;
	}

	if ( old )
		parent_->cxt_.text = old;


	if ( scbar_ )
	{
		UpdateScrollbar( scbar_.get() );

		float hh = ( ct_.bSingleLine_? 0.0 : RowHeight() *  scbar_->info_.rowno);
		ctrl_->mat_._32 -= hh;
	}
}

void CaretActive(); // D2DContextEx.cpp
void D2DTextbox::StatActive( bool bActive )
{
	// captureは上位層で操作
	
	if ( bActive )
	{
		xassert( parent_->GetCapture() == this );

		ctrl_->Password((typ_ & PASSWORD) != 0);

		CalcRender(true);
		bActive_ = true;

		if ( scbar_ )
		{
			float hh = ( ct_.bSingleLine_? 0.0 : RowHeight() *  scbar_->info_.rowno);
			ctrl_->mat_._32 -= hh;
		}

		ctrl_->SetFocus();
		V4::CaretActive(); // D2DContextEx.cpp

		if ( OnEnter_ )
			OnEnter_(this);
	}
	else
	{
		if ( ctrl_->GetContainer() == &ct_ )
		{						
			text_layout_.Release();
			ctrl_->GetLayout()->GetTextLayout( &text_layout_ );

			ctrl_->SetContainer( NULL, NULL );
		}
		V4::SetCursor( ::LoadCursor(NULL,IDC_ARROW));
		bActive_ = false;

		if ( OnLeave_ )
			OnLeave_(this);
	}
}

FRectF D2DTextbox::GetClientRect()
{
	return rc_;
}

FRectFBoxModel D2DTextbox::GetClientRectEx()
{
	// 変換ダイアログの表示位置 -> CTextStore::GetTextExt
	FRectFBoxModel x = rc_;
	

	if ( this->typ_ == TYP::SINGLELINE || this->typ_ == TYP::PASSWORD )
	{
		// 26:標準フォント高で微調整
		x.Offset(0, (rc_.Height()- 26)/2+2 ); // ???
	}

	return x;
}
FString D2DTextbox::GetText()
{	
	return FString( ct_.GetTextBuffer());  // null terminate	
}

D2DTextbox* D2DTextbox::Clone(D2DControls* pacontrol)
{
	D2DTextbox* ret = new D2DTextbox( ctrl_ );

	ret->typ_ = typ_;
	ret->bActive_ = bActive_;
	
	ret->text_layout_ = text_layout_;
	ret->clr_fore_ = clr_fore_;
	ret->clr_back_ = clr_back_;
	ret->bUpdateScbar_ = bUpdateScbar_;
	ret->ct_.bSingleLine_ = ct_.bSingleLine_;
	ret->ct_.LimitCharCnt_ = ct_.LimitCharCnt_;

	if ( pacontrol )
		ret->CreateWindow( parent_, pacontrol, rc_, stat_, name_ );

	
	return ret;
}

void D2DTextbox::SetSize( const FSizeF& sz )
{
	FRectF rc( rc_.left, rc_.top, rc_.left+sz.width, rc_.top+sz.height);
	SetRect(rc);
}
void D2DTextbox::SetRect( const FRectF& rc )
{
	rc_ = rc;

	if ( scbar_ )
		scbar_->WndProc( parent_,WM_SIZE,0,0);
}

// Tab文字　可
void D2DTextbox::TabEnable()
{
	OnPushKey_ = [](D2DTextbox* tx,UINT key)->LRESULT
	{
		if ( key == '\t' )
		{			
			tx->InsertText( L"\t", -1, 1 );
			return 1;
		}
		return 0;
	};
}

//-------------------------------------------------------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------------------------------------------------------
CharsRectF::CharsRectF(IDWriteTextLayout* tx, bool bSingle  )
{
	layout_ = tx;
	cnt_ = 0;
	rects_ = NULL;
	line_height_ = 0;
	bSingleLine_ = bSingle;
}
void CharsRectF::Clear()
{
	delete [] rects_;
	cnt_ = 0;
	line_height_ = 0;
}
FRectF* CharsRectF::calc(const FSizeF&  sz, LPCWSTR str, int slen, int* plen)
{
	float x, y;
	{				
		DWRITE_HIT_TEST_METRICS tm;
		layout_->HitTestTextPosition(0, false, &x, &y, &tm);
		line_height_ = tm.height;		
	}
	
	cnt_ = slen;
	rects_ = new FRectF[cnt_];
	ZeroMemory( rects_, sizeof(FRectF)*cnt_ );

	int r = 0;
	for (int i = 0; i < slen; i++)
	{
		float x1,y1;
		DWRITE_HIT_TEST_METRICS tm;

		layout_->HitTestTextPosition(i, false, &x1, &y1, &tm);


		if ( bSingleLine_ && y1 != y )
			break;

		rects_[i].SetRect(tm.left, tm.top, FSizeF(tm.width, tm.height));

		r++;
	}

	*plen = r;
	
	return rects_;

}

////////////////////////////////////////////////////////////////////////////////////////////
int CreateTextLayout::Create( int zStartPos, int zCaretPos, IDWriteTextLayout** layout)
{
	if ( IsSibleLine_ )
		return CreateSingleTextLayout(zStartPos,zCaretPos,layout);
	else
		return CreateMultiTextLayout(zStartPos,zCaretPos,layout);
}
int CreateTextLayout::CreateMultiTextLayout( int zStartPos, int zCaretPos, IDWriteTextLayout** layout)
{
	if ( HR(fc_->CreateTextLayout( psz_+zStartPos, len_-zStartPos, fmt_, sz_.width, sz_.height, layout )))
	{
		
		LPCWSTR x = L"TABWIDTH";
		CComPtr<IDWriteTextLayout> tl;
		if ( HR(fc_->CreateTextLayout( x, TAB_WIDTH_4CHAR, fmt_, 1000, sz_.height,  &tl )))
		{
			DWRITE_TEXT_METRICS t;
			tl->GetMetrics(&t);
			
			(*layout)->SetIncrementalTabStop( t.width ); //タブの距離設定 tab is 4 chars
		}
	}

	return 0;
}

int CreateTextLayout::CreateSingleTextLayout( int zStartPos, int zCaretPos, IDWriteTextLayout** layout)
{
	int viewlen = 0;
	float width9999 = 9999;
	int off = 0;

	// 表示可能文字数を算出する。
	if ( len_ && (int)len_ - zStartPos >= 0)
	{
		CComPtr<IDWriteTextLayout> temp;
		auto hr = fc_->CreateTextLayout( psz_+zStartPos, len_-zStartPos, fmt_, width9999, sz_.height, &temp );

		float wd = 0;
		const float triming = 21.0f;	// ???
		for (UINT i = 0; i < len_; i++)
		{
			float x1=0,y1=0;
			DWRITE_HIT_TEST_METRICS tm;
			hr = temp->HitTestTextPosition(i, false, &x1, &y1, &tm);
		
			if ( (wd+triming) > sz_.width )
				break;

			wd += tm.width;

			viewlen = i+1;
		}

		// キャレットから先頭文字の位置を算出
	}

	if ( len_ > 0 && viewlen == 0 )
		viewlen = 1;

	
	{
		off = 0;
		if ( len_ && zStartPos<=(int)len_)
		{
			SingleLineCaret sc( zStartPos,len_, viewlen, zCaretPos-zStartPos );
				
			sc.CaretOff(-1);
			sc.CaretOff(1);

			off = sc.zS();
		}
		int len = max(0, (int)len_-off );

		auto hr = fc_->CreateTextLayout( psz_+off, len, fmt_, width9999, sz_.height, layout );

		_ASSERT(hr == S_OK);
	}
	
	return off;

}

void D2DTextbox::UpdateScrollbar(D2DScrollbar* bar)
{
	// 行数のカウントする
	scbar_->SetTotalSize(RowHeight()*RowCount(true));
	
	FRectF rc = bar->info_.rc;
	float h2 = bar->info_.button_height; // 上下のボタン
	FRectF rcb = bar->info_.thumb_rc; // 真ん中のボタン

	float rowheight = RowHeight();

	float h = rc.Height() - h2*2 - rcb.Height(); // thumbの移動範囲
	
	UINT cnt = RowCount(true);
	if ( cnt )
	{
		cnt = cnt -RowCount(false);
		if ( cnt )
			bar->info_.rowno = (UINT)((bar->info_.position / h)*cnt); 
	}

		
}
float D2DTextbox::RowHeight()
{
	return font_height_;
}
UINT D2DTextbox::RowCount( bool all )
{
	UINT cnt = 0;
	if ( all )
	{
		// 折り返しも含めた行数, 表示されない部分を含めた行数		
		auto x = ctrl_->GetLayout()->GetViewLineCount();

		return x;
	}
	else
	{
		// 表示される行数
		
		FRectF rc = rc_.GetContentRect();
		auto x = (UINT)(rc.Height() / RowHeight()); 	
		return x;
	}
}
}; // namespace V4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// UNIVERSAL APP do not have HWND. so, POPUPWINDOW is not exists.

std::map<HWND,TSF::CTextEditorCtrl*> D2DTextbox::s_text_inputctrl;


static ITfKeystrokeMgr *g_pKeystrokeMgr	= NULL;
TfClientId g_TfClientId	= TF_CLIENTID_NULL;

HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();

#if ( _WIN32_WINNT < _WIN32_WINNT_WIN8 )
ITfThreadMgr* g_pThreadMgr = NULL;
#else
ITfThreadMgr2* g_pThreadMgr = NULL;
#endif


namespace TSF {
CTextEditorCtrl* GetTextEditorCtrl()
{
	auto it = D2DTextbox::s_text_inputctrl.begin();
	return it->second; 
}

CTextEditorCtrl* GetTextEditorCtrl2(HWND hWnd)
{
	auto it = D2DTextbox::s_text_inputctrl.find( hWnd );
	if ( it != D2DTextbox::s_text_inputctrl.end())
	{		
		return it->second;
	}
	return nullptr;
}

};

void D2DTextbox::CreateInputControl(D2DWindow* parent)
{	
	HWND hWnd = parent->hWnd_;
	
	if ( s_text_inputctrl.find( hWnd ) == s_text_inputctrl.end())
	{
		auto ctrl = new TSF::CTextEditorCtrl();
		s_text_inputctrl[hWnd] = ctrl;

		ctrl->Create( hWnd );	
	}
		

}
// STATIC
bool D2DTextbox::AppTSFInit()
{
	#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr2, (void**)&g_pThreadMgr))) goto Exit;    
	#else
	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr, (void**)&g_pThreadMgr))) goto Exit;    
	#endif
	
	if (FAILED(g_pThreadMgr->Activate(&g_TfClientId)))  goto Exit;    
	if (FAILED(g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&g_pKeystrokeMgr)))  goto Exit;
	if ( FAILED(InitDisplayAttrbute()))	goto Exit;

	return true;
Exit:

	return false;

}
// STATIC
void D2DTextbox::AppTSFExit()
{
	UninitDisplayAttrbute();

	{
		for( auto& it : s_text_inputctrl )
			delete it.second;
		s_text_inputctrl.clear();
	}


	if ( g_pThreadMgr )
	{
		g_pThreadMgr->Deactivate();
		g_pThreadMgr->Release();
	}

	if ( g_pKeystrokeMgr )
		g_pKeystrokeMgr->Release();
}
