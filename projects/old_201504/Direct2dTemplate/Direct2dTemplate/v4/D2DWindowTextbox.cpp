#include "stdafx.h"
#include "D2DWindowControl.h"
#include "tsf\TextEditor.h"
#include "tsf\TextLayout.h"
#include "D2DCharRect.h"

using namespace TSF;
using namespace V4;

#define LEFT_MARGIN 3.0f

D2DTextbox::D2DTextbox(CTextEditorCtrl* ctrl):brush_border_(0),brush_fore_(0),brush_back_(0),brush_active_border_(0)
{
	_ASSERT( ctrl );
	
	typ_ = TYP::SINGLELINE;
	ct_.bSingleLine_ = true;
	ctrl_ = ctrl;
	bActive_ = false;
	//bUpdateScbar_ = false;
	fmt_ = NULL;
	
	ct_.LimitCharCnt_ = 65500;
	temp_font_height_ = 0;	
}
void D2DTextbox::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );
	
	brush_border_ = D2RGB(0,0,0);
	brush_fore_ = D2RGB(0,0,0);
	brush_back_ = D2RGB(255,255,255);
	brush_active_border_ = D2RGB(255,0,0);

	if ( fmt_ )
	{
		CComPtr<IDWriteTextLayout> tl;		
		if ( S_OK == parent_->cxt_.wfactory->CreateTextLayout( L"T", 1, fmt_, 1000,1000,  &tl ))
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
 

	SetText(L"");
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
		text_layout_ = ctrl_->GetLayout()->DWTextLayout_;
}
LRESULT D2DTextbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if ( !VISIBLE(stat_))
		return ret;

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
				auto br1 = CreateBrush( cxt, brush_back_ );
				DrawFill(cxt,rc1,br1 );// 裏面
			}
			
			auto wakucolor = CreateBrush( cxt,brush_border_ );
			auto forecolor = CreateBrush( cxt,brush_fore_ );

			ID2D1Brush* textcolor = ( stat_&READONLY ? cxt.bluegray.p : forecolor.p );
			
			mat.PushTransform(); // 1
			{			
				mat.Offset( rccnt.left + LEFT_MARGIN , rccnt.top );		
				mat.PushTransform(); // 2
				{
					float hh = 0; //( scbar_.get()==nullptr ? 0.0 : RowHeight() *  scbar_->info_.rowno);

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
						
							ret = ctrl_->WndProcX( d, WM_PAINT, wParam, lParam );

							if ( old )
							{
								parent_->cxt_.text = old;
								parent_->cxt_.cxtt.line_height = oldheight;

							}
							wakucolor = cxt.red; // border color
						}					
						mat.PopTransform(); // 3
					}			
				}
				mat.PopTransform(); // 2
			}

			mat.PopTransform(); // 1

			if ( stat_ & BORDER )
			{
				D2DRectFilter f(cxt, rc1);
				DrawFillRect( cxt, rc1, wakucolor, cxt.transparent,1.0f );
			}

			if ( stat_ & DISABLE )
			{
				DrawFill( cxt, rc1, cxt.halftone );
			}

			mat.PopTransform();	//0	

			
			if ( bActive_ )
			{
				d->redraw_=1; // redraw
			}
		}
		break;
		default :
		{	
						
			switch( message )
			{
				case WM_LBUTTONDOWN:
				{
					FPointF pt = mat_.DPtoLP( FPointF(lParam));		
			
					if ( rc_.PtInRect(pt) )
					{						
						d->SetCapture( this, &pt );

						StatActive(true);						
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
				

			}

			if( ctrl_->GetContainer() != &ct_ || d->GetCapture() != this )
				return ret;

			int bAddTabCount = 0;
			
			if ( message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEMOVE  
				|| message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_LBUTTONDBLCLK )			
			{					
				FPointF npt = matEx_.DPtoLP( FPointF(lParam) );
				lParam = MAKELONG( (WORD)npt.x, (WORD)npt.y );				
			}

			ret = ctrl_->WndProcX( d, message, wParam, lParam ); // WM_CHAR,WM_KEYDOWNの処理など

			// add tab
			for( int i = 0; i < bAddTabCount; i++ )
				OnKeyDown(d,WM_KEYDOWN,(WPARAM)L'\t',0);
		
		}
		break;	
	}
	return ret; 
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
	}

	return ret;
}
void D2DTextbox::SetText(LPCWSTR str1)
{
	parent_->redraw_ = 1;
	//bUpdateScbar_ = true;

	{
		FString s1b = FilterInputString( str1, lstrlen(str1) );
	
		UINT nrCnt; 
		ct_.Clear();
		ct_.InsertText( 0, s1b.c_str(), s1b.length(), nrCnt );
		ct_.CaretLast();
	}	


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
		text_layout_ = ctrl_->GetLayout()->DWTextLayout_; // singlineの場合自動で、vcenterになる

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

}
void D2DTextbox::StatActive( bool bActive )
{
	// captureは上位層で操作
	
	if ( bActive )
	{
		xassert( parent_->GetCapture() == this );

		CalcRender(true);

		bActive_ = true;
	}
	else
	{
		if ( ctrl_->GetContainer() == &ct_ )
		{						
			text_layout_ = ctrl_->GetLayout()->DWTextLayout_;
			
			ctrl_->SetContainer( NULL, NULL );
		}
		V4::SetCursor( ::LoadCursor(NULL,IDC_ARROW));
		bActive_ = false;

	}
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
FRectF D2DTextbox::GetClientRect()
{
	return rc_;
}

FRectFBoxModel D2DTextbox::GetClientRectEx()
{
	return rc_;
}
IDWriteTextFormat* D2DTextbox::GetFormat()
{
	return fmt_;
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

			UINT rcnt;
			ct_.InsertText( ct_.nSelEnd_, s1b.c_str(), s1b.length(), rcnt );
			ct_.nSelEnd_ += rcnt;
			ct_.nSelStart_ = ct_.nSelEnd_;

			GlobalUnlock( h );

			CalcRender(true);
		}	
	}	
	::CloseClipboard();
	return TRUE;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CreateTextLayout::Create( int zStartPos, int zCaretPos, IDWriteTextLayout** layout)
{
	//if ( IsSibleLine_ )
		return CreateSingleTextLayout(zStartPos,zCaretPos,layout);
	/*else
		return CreateMultiTextLayout(zStartPos,zCaretPos,layout);*/
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static TSF::CTextEditorCtrl *g_pTextInputCtrl = NULL;
static ITfKeystrokeMgr *g_pKeystrokeMgr	= NULL;

ITfThreadMgr *g_pThreadMgr			= NULL;
TfClientId g_TfClientId				= TF_CLIENTID_NULL;
HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();



namespace TSF {
CTextEditorCtrl* GetTextEditorCtrl()
{
	xassert( g_pTextInputCtrl );

	return g_pTextInputCtrl;
}
};

void D2DTextbox::CreateInputControl(D2DWindow* parent)
{
	xassert( g_pTextInputCtrl );
	g_pTextInputCtrl->Create( parent->hWnd_ );	

}
bool D2DTextbox::AppTSFInit()
{
	g_pTextInputCtrl = new TSF::CTextEditorCtrl();


	if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,IID_ITfThreadMgr, (void**)&g_pThreadMgr))) goto Exit;    
	if (FAILED(g_pThreadMgr->Activate(&g_TfClientId)))  goto Exit;    
	if (FAILED(g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&g_pKeystrokeMgr)))  goto Exit;
	if ( FAILED(InitDisplayAttrbute()))	goto Exit;

	return true;
Exit:

	return false;

}
void D2DTextbox::AppTSFExit()
{
	UninitDisplayAttrbute();
	delete g_pTextInputCtrl;


	if ( g_pThreadMgr )
	{
		g_pThreadMgr->Deactivate();
		g_pThreadMgr->Release();
	}

	if ( g_pKeystrokeMgr )
		g_pKeystrokeMgr->Release();

}
/////////////////////////////////////////////////////////////////////////////////////////////

namespace V4 {
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
}