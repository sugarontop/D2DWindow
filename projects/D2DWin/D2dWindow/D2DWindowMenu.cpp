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
#include "D2DWindowMenu.h"
#include "D2DWindowTitle.h"
#include "gdi32.h"

using namespace V4;



#define FLOATING_COLOR D2RGBA(169,202,239,180)

#pragma region D2DMainFrameMenu
////////////////////////////////////////////////////////////////////////////////////
LRESULT D2DMainFrameMenu::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch( message )
	{
		case WM_D2D_PAINT:
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);

			if ( message == WM_D2D_PAINT )
				mat.PushTransformInCaputre( mat_ );
			else				
				mat_ = mat.PushTransform();				

			mat.Offset( rc_.left, rc_.top );
			
			

			FRectF rc(0,0, rc_.Width(), rc_.Height());

			//DrawFillRect( cxt, rc,cxt.ltgray,cxt.ltgray,1);
			FillRectangle( cxt, rc, cxt.ltgray );

			int k = 0;
			
			for(auto& it : ar_ )
			{											
				rc.right=it.width;
				
				if ( active_ == k )
					//DrawFillRect( cxt, rc,cxt.gray,cxt.gray,1);
					FillRectangle( cxt, rc, cxt.gray );

				DrawCenterText( cxt, cxt.black, rc, (LPCWSTR)it.title, it.title.length(), 1 );

				 if ( active_ == k && d->CaptureIsInclude(this) )
				 {
					FRectF rc2 = rc;
					rc2.top += rc_.Height();
					rc2.SetHeight(100);					
					//DrawFillRect( cxt, rc2,cxt.gray,cxt.gray,1);

					mat.PushTransform();
					mat.Offset( 0, rc_.Height());

					submenu_->WndProc(d,message,wParam,lParam);
					mat.PopTransform();
				 }


				
				mat.Offset(it.width, 0 );
				k++;
			}			
			
			mat.PopTransform();
			
		}
		break;
		case WM_SIZE:
		{
			auto sz = parent_control_->GetRect().GetContentRect().Size();
			rc_.right = rc_.left + sz.width;
		}
		break;

		case WM_MOUSEMOVE:
		{			
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			
			if ( rc_.PtInRect( pt ) )
			{
				active_ = -1;
				pt.Offset( -rc_.left, -rc_.top );

				FRectF rc(0,0,0,rc_.Height());
				int k = 0;
				
				for(auto& it : ar_ )
				{											
					rc.right=rc.left+it.width;
					
					if ( rc.PtInRect(pt))
					{
						 if ( active_ != k )
						 {
							active_ = k;
							d->redraw_ = 1;

							CreateSubMenu();
							return 1;
						}
					}

					rc.Offset(it.width,0);

					k++;
				}			
			}
			else if ( d->GetCapture() == this )
			{
				if ( submenu_ )
					submenu_->WndProc(d,message,wParam,lParam);
			}			
			else
				active_ = -1;
		}
		break;
		case WM_LBUTTONDOWN:
			if ( active_ > -1 && d->GetCapture() == NULL )
			{
				d->SetCapture(this);
				return 1;
			}
			else if ( d->GetCapture() == this )
			{
				LRESULT x = 0;
				if ( submenu_ )
				{
					x = submenu_->WndProc(d,message,wParam,lParam);


					if ( x == 9100 )
					{
						d->ReleaseCapture();

					}
				}
				
			
				return 1;
			}
		break;
		case WM_LBUTTONUP:
		{
			LRESULT x = 0;
			if ( d->GetCapture() == this )
			{
				if ( submenu_ )
					x = submenu_->WndProc(d,message,wParam,lParam);


				
			}

		}
		break;


	}


	//if ( ret == 0 )
	//	ret = D2DControls::WndProc(d,message,wParam, lParam );

	return ret;
}



void D2DMainFrameMenu::LoadMenu()
{	
	bool bl = menuxml_->load( L"menu.xml" );

	if ( !bl )
	{
		_bstr_t xml = L"<?xml version='1.0' encoding='utf-8' ?><root><c typ='item_head' title='ファイル(F)'><c typ='click' id='105' title='終了'/></c></root>";
		bl = menuxml_->load( xml );
	}
}

int WINAPI D2GetLineMetrics( D2DContextBase& cxt, const FSizeF& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS* ret )
{
	xassert( ret );
		
	DWRITE_TEXT_METRICS tm;
	ComPTR<IDWriteTextLayout> tl;
	if (HR(cxt.wfactory->CreateTextLayout(str,len, cxt.text,sz.width, sz.height, &tl )))
	{	
		tl->GetMetrics(&tm);

		*ret = tm;

		return tm.lineCount;
	}
	return 0;
}


void D2DMainFrameMenu::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id  )
{
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id);

	LoadMenu();

	MenuItem item;

	auto list = menuxml_->documentElement->selectNodes(_bstr_t(L"c/@title"));
	long len;
	list->get_length(&len);
	for( long i = 0; i < len; i++ )
	{
		MSXML2::IXMLDOMNode* nd;
		list->get_item(i, &nd);

		item.title = nd->Gettext();
		ar_.push_back(item);

	}

	FSizeF sz = rc_.Size();
	DWRITE_TEXT_METRICS t;
	
	for( auto& it: ar_ )
	{				
		D2GetLineMetrics( parent->cxt_, sz, it.title,it.title.length(), &t );
		it.width = t.width + 20;
	}
	
	active_ = -1;
	submenu_ = NULL;
	
}

void D2DMainFrameMenu::CreateSubMenu()
{
	xassert( active_ > -1 );

	if ( submenu_ )
		submenu_->DestroyControl();

	auto list = menuxml_->documentElement->selectNodes(_bstr_t(L"c"));
	MSXML2::IXMLDOMNode* nd;
	list->get_item(active_, &nd);

	auto list1 = nd->selectNodes( _bstr_t(L"c") );
	long len = list1->Getlength();

	submenu_ = new D2DMenuItems();

	FRectF rc(0,0,200,MENU_ITEM_HEIGHT*len);
	submenu_->CreateWindow(parent_,this,rc,VISIBLE|BORDER,NONAME,true);

	CreateSubMenuItem(nd);
}
void D2DMainFrameMenu::CreateSubMenuItem(MSXML2::IXMLDOMNode* nds)
{
	xassert( submenu_ );
	xassert( active_ > -1 );

	//auto df = [](D2DContext& cxt, const FRectFBoxModel& rc, int x, const FString& s){
	//	
	//	D2DrawCenterText(cxt, cxt.black,rc,s,s.length(),1);

	//};

	float w = submenu_->GetRect().GetSize().width;


	auto list = nds->selectNodes(_bstr_t(L"c") );
	long len = list->Getlength();

	_bstr_t typ_click = L"click", typ_itemh = L"item_head";
	for( long i = 0; i < len; i++ )
	{
		D2DMenuItemEx* btn = new D2DMenuItemEx();
		FRectFBoxModel rc(0,0,w,MENU_ITEM_HEIGHT);
		 
		rc.Margin_.Set(1);

		auto nd = list->Getitem(i);
		auto title = nd->selectSingleNode( _bstr_t(L"@title"))->Gettext();
		auto typ = nd->selectSingleNode( _bstr_t(L"@typ"))->Gettext();
		auto id = nd->selectSingleNode( _bstr_t(L"@id"))->Gettext();

		

		btn->CreateWindow(parent_, submenu_, rc, VISIBLE, (LPCWSTR)title, _wtoi(id) );
		if ( typ == typ_itemh )
		{
			btn->nd_ = nd;
			auto xxx = nd->selectNodes(_bstr_t(L"c") );



			int a = 0;
		}
		


		auto border = nd->selectSingleNode( _bstr_t(L"@border"));
		if ( border )
			btn->bBottomLine_ = true;
		
	}
}

#pragma endregion //D2DMainFrameMenu

#pragma region D2DMenuItems
//////////////////////////////////////////////////////////////
void D2DMenuItems::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, bool bVertical, int id  )
{
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id);
	bVertical_ = bVertical;
	active_ = -1;

}
D2DControl* D2DMenuItems::GetActiveControl()
{
	if ( active_ < 0 )
		return NULL;
	return controls_[active_].get();
}
LRESULT D2DMenuItems::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	int md = 0;
	switch( message )
	{
		case WM_D2D_PAINT:
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(d->cxt_);		
			mat.PushTransform();


			if ( message == WM_D2D_PAINT )
				mat.PushTransformInCaputre( mat_ );
			else				
				mat_ = mat.PushTransform();	

	

			mat.Offset( rc_.left, rc_.top );
			
			if ( stat_ & BORDER )
				//DrawFillRect( d->cxt_, rc_.ZeroRect(), d->cxt_.gray, d->cxt_.ltgray, 1.0f );
				FillRectangle( cxt, rc_.ZeroRect(), cxt.ltgray );

			int k = 0;
			if ( bVertical_ )
			{
				for(auto& it : controls_ )
				{												
					it->WndProc(d,message,wParam,lParam);
					float offy = it->GetRect().Size().height;			
					mat.Offset( 0, offy );
					k++;
				}			
			}
			else
			{
				for(auto& it : controls_ )
				{					
					it->WndProc(d,message,wParam,lParam);
					float offw = it->GetRect().Size().width;
					mat.Offset( offw, 0 );
					k++;
				}		
			}
			mat.PopTransform();
			md = 1;
		}
		break;
		case WM_SIZE:
		{
			auto sz = parent_control_->GetRect().GetContentRect().Size();

			rc_.right = rc_.left + sz.width;

		}
		break;
		case WM_MOUSEMOVE:
		{
			active_ = -1;
			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			parent_->SetCapture(this); 


			if ( rc_.PtInRect( pt ) )
			{
				pt.Offset( -rc_.left, -rc_.top );

				int k = 0;
				FSizeF off(0,0);
				for(auto& it : controls_ )
				{											
					FRectF rc = it->GetRect();
					
					rc.Offset(off);
					
					if ( rc.PtInRect(pt))
					{
						 active_ = k;
						 d->redraw_ = 1;						 
						 ret = 1;
						 break;
					}

					if ( bVertical_ )
						off.height += rc.Height();
					else
						off.width += rc.Width();

					k++;
				}		
				//ret = D2DControls::WndProc(d,message,wParam, lParam );	
				ret = SendMessageAll(d, message, wParam, lParam);
			}
			else 
			{
				FRectF rc(rc_);
				if ( (rc.right < pt.x || !(rc.top-MENU_ITEM_HEIGHT/4.0 < pt.y && rc.top+MENU_ITEM_HEIGHT > pt.y)  )  &&  this == parent_->GetCapture())
				{					
					parent_->ReleaseCapture();
					parent_->redraw_ = 1;
				}
			}
			ret = 1;

			//ret = SendMessageAll(d, message, wParam, lParam);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			
			if ( rc_.PtInRect( pt ) )
			{
				D2DControls::WndProc(d,message,wParam, lParam );
				return 1; //親のcaptureは保持
			}
			else
			{
				while( parent_->GetCapture() )
					parent_->ReleaseCapture();

			}


			return 0;
		}
		break;
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				D2DControls::WndProc(d,message,wParam, lParam );	
					
				while( parent_->GetCapture() )
					parent_->ReleaseCapture();

												
				return 0;
			}
			return 1;
		}
		break;
	}


	if ( ret == 0 && md == 0)
		ret = D2DControls::WndProc(d,message,wParam, lParam );
		

	return ret;

}
#pragma endregion

#pragma region D2DMenuItemEx
//////////////////////////////////////////////////////////////
void D2DMenuItemEx::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBM& rc, int stat, LPCWSTR name, int id )
{
	
	D2DControl::CreateWindow(parent,pacontrol,rc,stat,name,id);

	strL = name;
	strR = L"ctrl-x";
	md_ = 0;
	bBottomLine_ = false;
	submenu_ = NULL;
	nd_ = NULL;
}




LRESULT D2DMenuItemEx::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch ( message )
	{
		case WM_D2D_PAINT:
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);

			if ( message == WM_D2D_PAINT )
				mat.PushTransformInCaputre( mat_ );
			else				
				mat_ = mat.PushTransform();	

			mat.Offset( rc_.left, rc_.top );
			{				
				FRectF rc1 = rc_.GetContentRectZero();
				D2DMenuItems* p = (D2DMenuItems*)parent_control_;
				
				// アクティブな場合
				if ( p->GetActiveControl() == this )
				{
					FRectF rc2(rc1);
					rc2.left++;
					ComPTR<ID2D1SolidColorBrush> br;
					cxt.cxt->CreateSolidColorBrush( FLOATING_COLOR, &br);
					FillRectangle(cxt, rc2, br );
				}

				rc1.left += MENU_ITEM_HEIGHT;
				rc1.right -= 5;

				auto clr = cxt.black; 
				
				V4::DrawCenterText(cxt, clr,rc1,strL,strL.length(),0); // メニューの項目名
				V4::DrawCenterText(cxt, clr,rc1,strR,strR.length(),2);

				if ( bBottomLine_ )
				{
					cxt.SetAntiAlias(false);					
					DrawLine(cxt,rc1.LeftBottom(), rc1.RightBottom(),1,cxt.gray, nullptr );
					cxt.SetAntiAlias(true);
				}	
				
				if ( submenu_ )
					submenu_->WndProc(d,message,wParam,lParam);
					
			}
			mat.PopTransform();

			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( submenu_ )
				ret = submenu_->WndProc(d,message,wParam,lParam);


			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			if ( rc_.PtInRect( pt ))
			{
				if ( nd_ && md_ != 0x1 && submenu_ == NULL)
				{										
					CreateSubMenu();					
					ret = 1;																		
				}
				parent_->redraw_ = 1;
				md_ = 0x1;
			}
			else
			{
				md_ = 0;

				if ( submenu_ )
				{
					submenu_->DestroyControl();
					submenu_ = NULL;
				}
				parent_->redraw_ = 1;
			}

			
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			md_ = ( rc_.PtInRect( pt ) ? 0x2 : md_ );
			
			if ( submenu_ )
				ret = submenu_->WndProc(d,message,wParam,lParam);

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( md_ & 0x2 )
			{
				FPointF pt = mat_.DPtoLP( FPointF(lParam));						
				md_ = ( rc_.PtInRect( pt ) ? md_ : 0 );
			}

			if ( md_ & 0x2 || md_ & 0x1 )
			{
				md_ = 0;
				DWORD x = MAKEWPARAM(id_,0);
				//auto hr = ::SendMessage( parent_->hMainFrame_,WM_D2D_COMMAND, x, D2DWindow::SecurityId(true) ); // 実際送る WM_COMMANDの変形
				auto hr = ::PostMessage( parent_->hMainFrame_,WM_D2D_COMMAND, x, D2DWindow::SecurityId(true) ); // 実際送る WM_COMMANDの変形
			}

			//if ( submenu_ )
			//	ret = submenu_->WndProc(d,message,wParam,lParam);
		}
		break;
	}

	return ret;
}
void D2DMenuItemEx::CreateSubMenu()
{
	// wparam < 0 then just title.
	if ( id_ < 0 )
	{		
		FRectF rc( rc_.Width()-2, 0, FSizeF(200,300));

		D2DMenuItems* x = new D2DMenuItems();
		x->CreateWindow( parent_, this, rc, VISIBLE|BORDER, NONAME, true );

		submenu_ = x;

		float w = 200;

		auto list = nd_->selectNodes(_bstr_t(L"c") );
		long len = list->Getlength();

		_bstr_t typ_click = L"click", typ_itemh = L"item_head";
		for( long i = 0; i < len; i++ )
		{
			D2DMenuItemEx* btn = new D2DMenuItemEx();
			FRectFBoxModel rc(0,0,w,MENU_ITEM_HEIGHT);
		 
			rc.Margin_.Set(1);

			auto nd = list->Getitem(i);
			auto title = nd->selectSingleNode( _bstr_t(L"@title"))->Gettext();
			auto typ = nd->selectSingleNode( _bstr_t(L"@typ"))->Gettext();

			int id = -1;

			if ( typ == typ_click )
				id = _wtoi( nd->selectSingleNode( _bstr_t(L"@id"))->Gettext());

			btn->CreateWindow(parent_, submenu_, rc, VISIBLE, (LPCWSTR)title, id );
		
			if ( typ == typ_itemh )
				btn->nd_ = nd;


			auto border = nd->selectSingleNode( _bstr_t(L"@border"));
			if ( border )
				btn->bBottomLine_ = true;
		
		}
	}
}

#pragma endregion
////////////////////////////////////////

namespace V4 {
namespace D1 {
void D2DMenuItems::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FPointF pt, const std::vector<FloatMenuItem>& items ) 
{
	

	int maxlen = 0;

	for(auto& it : items )
	{
		maxlen = max(it.text.length(), maxlen );
	}


	float width;

	if ( maxlen < 20 )
		width = 200;
	else if ( maxlen < 40 )
		width = 300;
	else 
		width = 400;


	
	
	FRectF rc(pt,FSizeF(width,MENU_ITEM_HEIGHT* items.size()));

	D2DControls::CreateWindow(parent,pacontrol,rc,VISIBLE,NONAME,-1);

	D2DVerticalStackControls* stack = new D2DVerticalStackControls();
	stack->CreateWindow(parent,this,FRectF(0,0,0,0),VISIBLE,NONAME );

	

	
	for(auto it : items )
	{		
		D2DMenuItem* item = new D2DMenuItem(it);
		item->CreateWindow( parent, stack );
	}



	stack->WndProc(parent, WM_SIZE, 0, 0);

	parent->SetCapture(this);
}
LRESULT D2DMenuItems::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	
	if ( !VISIBLE(stat_))
		return ret;

	
	int md = 0;
	switch( message )
	{
		//case WM_D2D_PAINT:
		//case WM_PAINT:
		//{
		//	D2DContext& cxt = d->cxt_;
		//	D2DMatrix mat(d->cxt_);		
		//	mat.PushTransform();


		//	if ( message == WM_D2D_PAINT )
		//		mat.PushTransformInCaputre( mat_ );
		//	else				
		//		mat_ = mat.PushTransform();	

	

		//	mat.Offset( rc_.left, rc_.top );
		//	
		//	if ( stat_ & BORDER )
		//		//DrawFillRect( d->cxt_, rc_.ZeroRect(), d->cxt_.gray, d->cxt_.ltgray, 1.0f );
		//		FillRectangle( cxt, rc_.ZeroRect(), cxt.ltgray );

		//	int k = 0;
		//	/*if ( bVertical_ )
		//	{
		//		for(auto& it : controls_ )
		//		{												
		//			it->WndProc(d,message,wParam,lParam);
		//			float offy = it->GetRect().Size().height;			
		//			mat.Offset( 0, offy );
		//			k++;
		//		}			
		//	}
		//	else
		//	{
		//		for(auto& it : controls_ )
		//		{					
		//			it->WndProc(d,message,wParam,lParam);
		//			float offw = it->GetRect().Size().width;
		//			mat.Offset( offw, 0 );
		//			k++;
		//		}		
		//	}*/
		//	mat.PopTransform();
		//	md = 1;
		//}
		//break;
		case WM_MOUSEMOVE:
		{
			if ( d->GetCapture() == this )
			{




			}


		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				

			}
			else
			{
				SendMessageAll(d,message,wParam,lParam);


				if ( d->GetCapture() == this )
				{
					d->ReleaseCapture(true);
					DestroyControl();
					ret = 1;
				}
			}			
		}
		break;
	
	}

	if ( message == WM_PAINT )
	{
		int a = 0;
	}


	if ( ret == 0 && md == 0)
		ret = D2DControls::WndProc(d,message,wParam, lParam );
		

	return ret;

}

/////////////////////////////////////////////
void D2DMenuItem::CreateWindow( D2DWindow* parent, D2DControls* pacontrol )
{
	D2DControl::CreateWindow(parent,pacontrol,FRectF(0,MENU_ITEM_HEIGHT),VISIBLE,NONAME,-1);

	md_ = 0;

}
LRESULT D2DMenuItem::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(d->cxt_);		
			mat_ = mat.PushTransform();
			mat.Offset( rc_.left, rc_.top );

			DrawMenuItem(cxt);

			mat.PopTransform();

		}
		break;
		case WM_MOUSEMOVE:
		{
			int md = md_;
			FPointF pt = mat_.DPtoLP(lParam);
			if ( rc_.PtInRect( pt ) )
			{
				md = 1;
			}
			else
			{
				md = 0;
			}

			if ( md != md_ )
			{
				md_ = md;
				d->redraw_ = 1;
				ret = 1;
			}
		}
		break;
		case WM_SIZE:
		{
			auto p = dynamic_cast<IUpdatar*>( parent_control_ );
			if ( p )
				p->RequestUpdate(this, WM_SIZE );	
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lParam);
			if ( rc_.PtInRect( pt ) && info_.enable )
			{
				
				::SendMessage( d->hMainFrame_, info_.msg.message, info_.msg.wParam, info_.msg.lParam );

			}
		}
		break;
	}



	return ret;
}

void D2DMenuItem::DrawMenuItem(D2DContext& cxt)
{	
	auto rc = rc_.ZeroRect();
	const float iconw = 36;

	FillRectangle( cxt, rc, cxt.ltgray );

	bool enable = info_.enable;

	auto clr = ( enable ? cxt.black : cxt.gray );

	{
		auto rc2 = rc; rc2.top--; rc2.bottom = rc2.top+1;
		rc2.left += iconw;
		FillRectangle( cxt, rc2, cxt.gray );
	}
	
	if ( md_ > 0 && enable )
	{
		auto rc2 = rc;
		rc2.InflateRect(0,-1);
		FillRectangle( cxt, rc2, cxt.halftoneBlue );
	}

	
	auto& text = info_.text;

	// DrawIcon(cxt,... );


	
	rc.left += iconw;
	
	DrawCenterText( cxt, clr, rc, text.c_str(), text.length(), 0 );

	// auto& key = info_.key;
	// rc.Offset(xxx,0);
	// DrawCenterText( cxt, cxt.black, rc, key.c_str(), key.length(), 0 );


}

}
}


