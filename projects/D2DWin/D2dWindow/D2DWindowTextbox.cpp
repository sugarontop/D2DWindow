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
#include "D2DWindowMessage.h"
#include "D2DWindowControl_easy.h"
#include "D2DCommon.h"
#include "D2DWindowTextbox.h"

#include "tsf\TextEditor.h"
#include "tsf\TextLayout.h"
#include "tsf\D2DCharRect.h"
using namespace V4;

#ifdef TEXTBOXTEST
	#include "s1.h"
#endif

#define TAB_WIDTH_4CHAR 4
#define MAXLENGTH 65500

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

#pragma region D2DTextbox

D2DTextbox::D2DTextbox(CTextEditorCtrl* ctrl)
{
	if (!ctrl)
	{
		ctrl = TSF::GetTextEditorCtrl();
	}
	
	
	border_color_.color = ColorF(ColorF::Black);
	fore_color_.color = ColorF(ColorF::Black);
	back_color_.color = ColorF(ColorF::White);
	active_border_color_.color = ColorF(ColorF::Red);

	typ_ = TYP::SINGLELINE;
	ctrl_ = ctrl;
	bActive_ = false;
	bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.bSingleLine_ =  true;
	ct_.LimitCharCnt_ = MAXLENGTH;
	temp_font_height_ = 0;	
}
D2DTextbox::D2DTextbox(TYP typ, CTextEditorCtrl* ctrl)
{
	if ( !ctrl )
	{
		ctrl = TSF::GetTextEditorCtrl();
	}


	if ( typ & RIGHT || typ & CENTER || typ & VCENTER )
		typ = (D2DTextbox::TYP)(typ | TYP::SINGLELINE);

	border_color_.color = ColorF(ColorF::Black);
	fore_color_.color = ColorF(ColorF::Black);
	back_color_.color = ColorF(ColorF::White);
	active_border_color_.color = ColorF(ColorF::Red);


	typ_ = typ;
	ctrl_ = ctrl;
	bActive_ = false;
	bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.bSingleLine_ = ( typ_ != TYP::MULTILINE );
	ct_.LimitCharCnt_ = MAXLENGTH;	
	temp_font_height_ = 0;
}
UINT D2DTextbox::MaxLength( UINT nlength )
{
	UINT old = ct_.LimitCharCnt_;

	if ( (int)nlength < 0 )
		return old;

	ct_.LimitCharCnt_ = min( MAXLENGTH, nlength);
	return old;
}
void D2DTextbox::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );

	fore_color_.br = parent_->GetSolidColor(fore_color_.color);
	back_color_.br = parent_->GetSolidColor(back_color_.color);
	border_color_.br = parent_->GetSolidColor(border_color_.color);
	active_border_color_.br = parent_->GetSolidColor(active_border_color_.color);

	if ( fmt_ )
	{
		ComPTR<IDWriteTextLayout> tl;			
		if ( S_OK == parent_->cxt_.wfactory->CreateTextLayout( L"T", 1, fmt_.p, 32000,32000,  &tl ))
		{
			DWRITE_HIT_TEST_METRICS mt;
			float y = 0, xoff=0;
			tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );
			
			temp_font_height_ = mt.height;			
		}
	}
	else
	{
		temp_font_height_ = parent->cxt_.cxtt.line_height;
	}
	 
	bSizing_ = false;
	if ( IsFixedSize())
		OnSizing();
}

// Sizeが決定したとき
void D2DTextbox::OnSizing()
{
	if ( bSizing_ ) 
		return;

	// 右隅のPadding へスクロールバー
	FRectFBM rc = rc_;

	if ( 15 <= rc.Padding_.r )
	{	
		if ( typ_ & TYP::MULTILINE )
		{	
			float h = rc.GetContentRect().Height();	// v or hの判断にしか使われない。wm_sizeでのサイズで決まる。		
			float w = rc.Padding_.r;
			
			FRectFBoxModel rcscb(rc_.GetContentRect().Width(), 0, FSizeF(w, h));
						
			scbar_ = std::shared_ptr<D2DScrollbar>(new D2DScrollbar());
			scbar_->CreateWindowEx( parent_, this, rcscb, VISIBLE|ALWAYSDRAW, NONAME );
			scbar_->SetTotalSize(1);
		}
	}

	SetText(L"");
	bSizing_ = true;


}

IDWriteTextFormat* D2DTextbox::GetFormat()
{
	return fmt_;
}
void D2DTextbox::SetFont( ComPTR<IDWriteTextFormat> fmt )
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
		ctrl_->GetLayout()->GetTextLayout( &text_layout_ ); 
	}
}
void D2DTextbox::OnPaint(D2DWindow* d)
{
	D2DContext& cxt = d->cxt_;										
	D2DMatrix mat( cxt );		

	FRectF rccnt = rc_.GetContentRect();
		
	mat.PushTransform(); // 0
	mat_ = mat;


	FRectF rc1 = rc_.GetBorderRect(); 
		
	D2DRectFilter f(cxt, rc1);						
	DrawFill(cxt,rc1, (ID2D1Brush*)back_color_.br );	// fill back color
			
			
	ID2D1Brush* textcolor = ( stat_&READONLY ? cxt.bluegray.p : (ID2D1Brush*) fore_color_.br );
			
			
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
						float yoff = ( rccnt.Height()-temp_font_height_ ) / 2.0f; // vcenter
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
						parent_->cxt_.cxtt.line_height = temp_font_height_;
					}
					
					if ( !(typ_ & MULTILINE) )
					{
						float yoff = ( rccnt.Height()-temp_font_height_ ) / 2.0f; // vcenter
						mat.Offset(0,yoff );	
					}
							

					cxt.forecolor_ = (ID2D1SolidColorBrush*)fore_color_.br;							
					ctrl_->WndProc( d, WM_PAINT, 0, 0 );
					cxt.forecolor_ = nullptr;

					if ( old )
					{
						parent_->cxt_.text = old;
						parent_->cxt_.cxtt.line_height = oldheight;

					}
							
				}					
				mat.PopTransform(); // 3
			}			
		}
		mat.PopTransform(); // 2
	}

	if ( scbar_ )
	{
		FRectF rccc = scbar_->GetRect();
		mat.Offset( -LEFT_MARGIN , -rc_.Padding_.t );		
		scbar_->WndProc(d,WM_PAINT,0,0);
	}

	mat.PopTransform(); // 1

	if ( stat_ & BORDER )
	{
		D2DRectFilter f(cxt, rc1);
		//DrawFillRect( cxt, rc1, (ID2D1Brush*)border_color_.br, cxt.transparent,1.0f );	
				
		DrawControlRect(cxt, rc1, (ID2D1Brush*) border_color_.br, (ID2D1Brush*) cxt.transparent);
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
}
		

LRESULT D2DTextbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( !VISIBLE(stat_))
		return ret;

	switch ( message )
	{
		case WM_PAINT:
			OnPaint(d);
		break;
		
		default :
		{	
			if ( WM_SIZE == message )
			{
				// D2DFRectFBMと同じ
				
				if (!IsFixedSize())
				{
				
					FRectF rcc = rc_; 
					auto child_sz = parent_control_->GetChildSize(this);
			
					{
						auto pt = rcc.LeftTop();

						bool fixw =ISFLAG_ON( GetStat(), STAT::WIDTH_FIX);
						bool fixh =ISFLAG_ON(GetStat(), STAT::HEIGHT_FIX);

						FSizeF sz;
						sz.width = (fixw ? rcc.Width() :  child_sz.width);
						sz.height = (fixh ? rcc.Height() : child_sz.height);

						rc_.SetRect(pt, sz);
				
					}
			
					OnSizing();
				}
			}
			
			
			if ( WM_D2D_APP_USER <= message )
			{
				if ( OnWndProcExt_ && OnWndProcExt_( this, message, wParam, lParam ))
					return 1;
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
			
					if ( rc_.PtInRect(pt) )
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
					
					if (ret != 0)
						return ret;
				}
				break;
				//case WM_RBUTTONUP:
				//{
				//	ret = parent_control_->WndProc(d,WM_D2D_TEXTBOX_FLOAT_MENU,wParam,lParam ); // parent_controlはbproecssをFALSEにすること
				//	if (ret != 0)
				//		return ret;
				//}
				//break;
			}
		

			if ( scbar_ && scbar_->WndProc(d,message,wParam,lParam ) )
				return 1;


			if ( ctrl_->GetContainer() == &ct_ && d->GetCapture() == this )
			{	
				int bAddTabCount = 0;
				d->redraw_ = 1;

				//if (message == WM_KEYDOWN)
				//{
				//	debug = 999;
				//}


			
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
				for (int i = 0; i < bAddTabCount; i++)
					OnKeyDown(d, WM_KEYDOWN, (WPARAM) L'\t', 0);
				
				// key scroll. ↓,↑, return
				
				{
					if ( message == WM_KEYDOWN && (wParam == VK_DOWN || wParam == VK_UP || wParam == VK_RETURN ) )
					{
						ret = 1;
						bool bl = TryTrimingScrollbar();				

						if ( wParam == VK_RETURN && bl )
							scbar_->info_.rowno++;
					}
					else if (message == WM_MOUSEMOVE && wParam == MK_LBUTTON )
					{
						// mouse auto sctoll.
						bool bl = TryTrimingScrollbar();
						ret = 1;
					}
										
				}							
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

// CTRL-C, CTRL-V, CTRL-X
//
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
	wstring org = ct_.GetTextBuffer();

	SetText(str);

	UINT nrCnt; 
	ct_.Clear();
	ct_.InsertText( 0, org.c_str(), org.length(), nrCnt );
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
		SetText( v.bstrVal, ::SysStringLen(v.bstrVal));
	else
	{
		_variant_t dst;
		if ( HR(VariantChangeType( &dst, &v, 0, VT_BSTR )))
			SetText( dst.bstrVal, ::SysStringLen(dst.bstrVal));
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

void D2DTextbox::SetText(LPCWSTR str1, int len)
{
	parent_->redraw_ = 1;

	len = ( len < 0 ? lstrlen(str1) : len );

	bUpdateScbar_ = true;

	{
		wstring s1b = FilterInputString( str1, len );
	
		UINT nrCnt; 
		ct_.Clear();
		ct_.InsertText( 0, s1b.c_str(), s1b.length(), nrCnt );
		ct_.CaretLast();
	}	

	auto ct = ctrl_->GetContainer();
	auto bri = ctrl_->bri_;

	ActiveSw();

	ctrl_->SetContainer(ct, bri);

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
	
ctrl_->Password((typ_ & PASSWORD) != 0);
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
void D2DTextbox::StatActive( bool bActive )
{
	// captureは上位層で操作
	
	if ( bActive )
	{
		_ASSERT( parent_->GetCapture() == this );

		ctrl_->Password((typ_ & PASSWORD) != 0);

		CalcRender(true);

		bActive_ = true;

		if ( scbar_ )
		{
			float hh = ( ct_.bSingleLine_? 0.0 : RowHeight() *  scbar_->info_.rowno);
			ctrl_->mat_._32 -= hh;
		}

		ctrl_->SetFocus();

		if ( OnEnter_ )
			OnEnter_(this);
	}
	else
	{
		if ( ctrl_->GetContainer() == &ct_ )
		{						
			text_layout_.Release();
			ctrl_->GetLayout()->GetTextLayout( &text_layout_ );

			ctrl_->SetContainer( NULL, NULL ); // pTextStore_がリリースされない問題あり
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


// 変換ダイアログの表示位置
FRectFBoxModel D2DTextbox::GetClientRectEx()
{
	FRectFBoxModel rc = rc_;
	float trim = 2.0f;

	if ( this->typ_ == TYP::SINGLELINE || this->typ_ == TYP::PASSWORD )
	{
		float height = rc_.GetContentRect().Height();
		float font_height = fmt_->GetFontSize();
		rc.Offset(0, (height - font_height)/2+trim );
	}
	else
		rc.Offset(0,trim);

	return rc; // -> CTextStore::GetTextExt
}


std::wstring D2DTextbox::GetText()
{	
	wstring x = ct_.GetTextBuffer(); // 0で終端しているはず

	return x;
}

D2DTextbox* D2DTextbox::Clone(D2DControls* pacontrol)
{
	D2DTextbox* ret = new D2DTextbox( ctrl_ );

	ret->typ_ = typ_;
	ret->bActive_ = bActive_;
	
	ret->text_layout_ = text_layout_;
	ret->fore_color_ = fore_color_;
	ret->border_color_ = border_color_;
	ret->back_color_ = back_color_;
	ret->active_border_color_ = active_border_color_;

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
void D2DTextbox::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl::SetParameters(prms);

	for( auto& it : prms )
	{
		auto& key = it.first;
		if ( key == L"settext" )
			SetText( it.second );	
		else if ( key == L"maxlength" )
		{
			_variant_t v = it.second; v.ChangeType( VT_INT);
			MaxLength( v.intVal );
		}
	}



	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");
	ParameterColor(parent_, fore_color_, prms, L"forecolor");
	
}
void D2DTextbox::OnResutructRnderTarget(bool bCreate)
{
	if ( bCreate )
	{
		fore_color_.br = parent_->GetSolidColor(fore_color_.color);
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);
		active_border_color_.br = parent_->GetSolidColor(active_border_color_.color);

	}
	else
	{
		fore_color_.br = nullptr;
		back_color_.br = nullptr;
		border_color_.br = nullptr;
		active_border_color_.br = nullptr;
	}

	if (scbar_ )
		scbar_->OnResutructRnderTarget(bCreate);

}
void D2DTextbox::UpdateScrollbar(D2DScrollbar* bar)
{
	// 行数のカウントする
	scbar_->SetTotalSize(RowHeight()*RowCount(true));

	FRectF rc = bar->info_.rc;
	float h2 = bar->info_.button_height; // 上下のボタン
	FRectF rcb = bar->info_.thumb_rc; // 真ん中のボタン

	float rowheight = RowHeight();

	float h = rc.Height() - h2 * 2 - rcb.Height(); // thumbの移動範囲

	UINT cnt = RowCount(true);
	if (cnt)
	{
		cnt = cnt - RowCount(false);
		if (cnt)
			bar->info_.rowno = (UINT) ((bar->info_.position / h)*cnt);
	}


}
float D2DTextbox::RowHeight()
{
	return temp_font_height_;
}
UINT D2DTextbox::RowCount(bool all)
{
	UINT cnt = 0;
	if (all)
	{
		// 折り返しも含めた行数, 表示されない部分を含めた行数		
		auto x = ctrl_->GetLayout()->GetViewLineCount();

		return x;
	}
	else
	{
		// 表示される行数

		FRectF rc = rc_.GetContentRect();
		auto x = (UINT) (rc.Height() / RowHeight());
		return x;
	}
}
#pragma endregion // D2DTextbox

////////////////////////////////////////////////////////////////////////////////////////////
#pragma region CharsRectF

CharsRectF::CharsRectF(IDWriteTextLayout* tx, bool bSingle  )
{
	layout_ = tx;
	cnt_ = 0;
	rects_ = nullptr;
	line_height_ = 0;
	bSingleLine_ = bSingle;
}
void CharsRectF::Clear()
{
	delete [] rects_;
	cnt_ = 0;
	line_height_ = 0;
}
FRectF* CharsRectF::calc(const FSizeF&  sz, LPCWSTR str, int strlen, int* plen)
{
	float x, y;
	{				
		DWRITE_HIT_TEST_METRICS tm;
		layout_->HitTestTextPosition(0, false, &x, &y, &tm);
		line_height_ = tm.height;		
	}
	
	cnt_ = strlen;
	rects_ = new FRectF[cnt_];
	ZeroMemory( rects_, sizeof(FRectF)*cnt_ );

	int r = 0;
	for (int i = 0; i < strlen; i++)
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
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////
#pragma region CreateTextLayout
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
		ComPTR<IDWriteTextLayout> tl;
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
		ComPTR<IDWriteTextLayout> temp;
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

#pragma endregion

}; // namespace V4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// staticな宣言。

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
	if ( g_pThreadMgr == nullptr )
	{
		#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
		if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr2, (void**)&g_pThreadMgr))) goto Exit;    
		#else
		if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr, (void**)&g_pThreadMgr))) goto Exit;    
		#endif
	
		if (FAILED(g_pThreadMgr->Activate(&g_TfClientId)))  goto Exit;    
		if (FAILED(g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&g_pKeystrokeMgr)))  goto Exit;
		if ( FAILED(InitDisplayAttrbute()))	goto Exit;
	}
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

