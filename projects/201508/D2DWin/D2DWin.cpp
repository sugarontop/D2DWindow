/*
The MIT License (MIT)
Copyright (c) 2015 sugarontop@icloud.com
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
#include "D2DWindowControl_easy.h"
#include "D2dWindow\D2DWindowTextbox.h"
#include "D2dWindow\D2DWindowListbox.h"
#include "D2dWindow\D2DWindowControlsEx.h"
#include "D2dWindow\D2DWindowDataGrid.h"
#include "D2dWindow\\D2DWindowTabControl.h"
#include "D2dWindow\MoveTarget.h"
#include "D2DWindowCell.h"
using namespace V4;

void AppInit()
{
	
}
void AppExit()
{
	D2DTextbox::AppTSFExit();
}
DLLEXPORT D2Ctrl Ctrl(D2Ctrls ctrl)
{
	D2Ctrl x;
	x.ctrl = ctrl.ctrls;
	return x;
}

DLLEXPORT D2DWin WINAPI DDMkWindow(HWND hWnd, D2DWinOnEntry entry )
{
	D2DWin w;

	GDI32::FRect rc(0,0,0,0);
	D2DWindow* win = new D2DWindow();
	win->CreateD2DWindow( 0, hWnd, WS_CHILD|WS_VISIBLE, rc );

	w.win = (HANDLE)win;

	auto bl = D2DTextbox::AppTSFInit();
	_ASSERT(bl);
	D2DTextbox::CreateInputControl(win);	

	if ( entry )
		entry( w );
	return w;
}	
DLLEXPORT void WINAPI DDDestroyWindow(D2DWin win )
{
	D2DWindow* p = (D2DWindow*)win.win;
	if ( p )
		delete p;
}
DLLEXPORT void WINAPI DDResizeWindow(D2DWin win, int cx, int cy )
{
	if ( cx !=0 && cy !=0 )
	{
		D2DWindow* p = (D2DWindow*)win.win;

		::MoveWindow( p->hWnd_, 0,0, cx, cy, TRUE );

		//SendMessageAll(, message, wParam, lParam);

		
	}

}



DLLEXPORT D2Ctrls WINAPI DDMkTopControls( D2DWin win, const FRectFBM& rc, LPCWSTR name )
{
	D2DWindow* pw = (D2DWindow*)win.win;
	RECT rc1;
	GetClientRect(pw->hWnd_,&rc1);
	bool bl = false;
	
	FRectFBM rcf = rc;
	if ( rcf.right -  rcf.left <= 0 )
	{
		rcf.left = rc1.left;
		rcf.right = rc1.right;
		rcf.top = rc1.top;
		rcf.bottom = rc1.bottom;
		bl = true;
	}
	
	D2DTopControls* p = new D2DTopControls();
	p->CreateWindow( (D2DWindow*)win.win,nullptr, rcf, VISIBLE,name );

	p->auto_resize_ = bl;


	D2Ctrls r;
	r.ctrls = p;
	return r;
}


DLLEXPORT D2Ctrl WINAPI DDMkButton( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name )
{
	D2DButton* btn = new D2DButton();
	auto parent = (D2DControls*)ctrls.ctrls;
	btn->CreateWindow( parent->parent_, parent, rc, VISIBLE|BORDER,name);

	D2Ctrl r;
	r.ctrl = btn;
	return r;
}

DLLEXPORT D2Ctrl WINAPI DDMkTextbox( D2Ctrls ctrls, const FRectFBM& rc, int typ, LPCWSTR name )
{
	auto parent = (D2DControls*)ctrls.ctrls;

	int ty = D2DTextbox::TYP::SINGLELINE;
	switch( typ )
	{
		case 1:
			ty = D2DTextbox::TYP::MULTILINE;
		break;
		case 2:
			ty = D2DTextbox::TYP::PASSWORD;
		break;
		case 3:
			ty = D2DTextbox::TYP::RIGHT|D2DTextbox::TYP::SINGLELINE;
		break;
	}


	D2DTextbox* tx = new D2DTextbox( (D2DTextbox::TYP)ty, NULL );
	
	tx->CreateWindow( parent->parent_, parent, rc, VISIBLE|BORDER,name);

	if ( ty == D2DTextbox::TYP::MULTILINE )
		tx->TabEnable();


	D2Ctrl r;
	r.ctrl = tx;
	return r;
}
DLLEXPORT D2Ctrl WINAPI DDMkFRectFBM(D2Ctrls ctrls, const FRectFBM& rc )
{
	auto parent = (D2DControls*) ctrls.ctrls;

	D2DFRectFBM* tx = new D2DFRectFBM();

	tx->CreateWindow(parent->parent_, parent, rc, VISIBLE | BORDER, L"noname");
	D2Ctrl r;
	r.ctrl = tx;
	return r;
}


DLLEXPORT D2Ctrl WINAPI DDMkListbox( D2Ctrls ctrls, const FRectFBM& rc, int typ, LPCWSTR name )
{
	auto parent = (D2DControls*)ctrls.ctrls;
	D2DListbox* ls = new D2DListbox();
	ls->CreateWindow( parent->parent_, parent, rc, VISIBLE|BORDER,26,NULL,name );
	
	D2Ctrl r;
	r.ctrl = ls;
	return r;
}

DLLEXPORT D2Ctrl WINAPI DDMkDataGrid(D2Ctrls ctrls, const FRectFBM& rc, int columm_cnt, float row_height, float title_height, LPCWSTR name)
{
	auto parent = (D2DControls*) ctrls.ctrls;

	D2DDataGrid* ls = new D2DDataGrid();
	ls->CreateWindow(parent->parent_, parent, rc, VISIBLE | BORDER, row_height,title_height,columm_cnt, name, 0);


	D2Ctrl r;
	r.ctrl = ls;
	return r;

}

DLLEXPORT LONG_PTR* WINAPI DDGetListBuffer(D2Ctrl ctrl, int* cnt)
{
	D2DDataGrid* tab = dynamic_cast<D2DDataGrid*>((D2DControls*) ctrl.ctrl);
	if (!tab) return nullptr;
	
	return tab->GetDatabox(cnt).get();
}


DLLEXPORT D2Ctrl WINAPI DDMkStatic(D2Ctrls ctrls, const FRectFBM& rc, int alignment, LPCWSTR text)
{
	auto parent = (D2DControls*) ctrls.ctrls;

	D2DStatic* t = new D2DStatic();
	t->CreateWindow(parent->parent_, parent, rc, VISIBLE, alignment, NONAME);

	D2Ctrl r;
	r.ctrl = t;

	std::map<std::wstring, VARIANT> m;
	_variant_t tt(text), fnm(DEFAULTFONT);
	m[L"text"] = tt;
	m[L"fontname"] = fnm;


	DDSetParameter( r, m );
	return r;
}
//DLLEXPORT D2Ctrl WINAPI DDMkListbox2( D2Ctrls ctrls, const FRectFBM& rc, D2Ctrls tmpl, LPCWSTR name )
//{
//	auto parent = (D2DControls*)ctrls.ctrls;
//	auto tm = (D2DControls*)tmpl.ctrls;
//
//	D2DListboxItemEx* item = new D2DListboxItemEx( tm );
//
//
//	D2DListbox* ls = new D2DListbox();
//	ls->CreateWindow( parent->parent_, parent, rc, VISIBLE|BORDER,26,item,name );
//
//	delete item;
//
//	
//	D2Ctrl r;
//	r.ctrl = ls;
//	return r;
//
//}

static D2Ctrls RetCtrls(void* p )
{
	D2Ctrls r;
	r.ctrls = p;
	return r;
}


DLLEXPORT D2Ctrls WINAPI DDMkControls( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR classnm, LPCWSTR name )
{
	auto parent = (D2DControls*)ctrls.ctrls;

	FString clsnm = classnm;
	clsnm.ToLower();
	
	D2DControls* p = nullptr;

	if ( clsnm == L"" )
	{	
		p = new D2DControls();
		p->CreateWindow( parent->parent_, parent, (rc), VISIBLE|BORDER,name );
	}
	else if ( clsnm == L"tab" )
	{	
		D2DTabControls* p1 = new D2DTabControls();
		p1->CreateWindow( parent->parent_, parent, (rc), VISIBLE|BORDER,name );		
		p = p1;

		p1->auto_resize_ = true;
	}
	else if (clsnm == L"group")
	{
		// 枠ありなし、移動可、タイトルなし
		p = new D2DGroupControls();
		p->CreateWindow(parent->parent_, parent, (rc), VISIBLE | BORDER, name);
	}

		
	
	
	return RetCtrls(p);
}
DLLEXPORT D2Ctrls WINAPI DDGetPage( D2Ctrls ctrl, int page )
{
	D2DTabControls* tab = dynamic_cast<D2DTabControls*>( (D2DControls*)ctrl.ctrls );
	if ( tab )
	{
		if ( page < (int)tab->CountChild())
		{
			return RetCtrls( tab->Child(page));
		}
	}

	return RetCtrls(0);
}

DLLEXPORT D2Ctrls WINAPI DDAddPage( D2Ctrls ctrl, int page )
{
	D2DTabControls* tab = dynamic_cast<D2DTabControls*>( (D2DControls*)ctrl.ctrls );
	if ( tab )
	{
		if ( page < 0 )
		{			
			// new page
			UINT page = tab->AddChild( L"noname");
			return RetCtrls( tab->Child(page) );
		}
		else if ( page < (int)tab->CountChild())
		{
			// insert page

		}
	}
	return RetCtrls(0);
}


DLLEXPORT void WINAPI DDDestroyControl( D2Ctrl ctrl )
{
	if ( ctrl.ctrl )
	{
		D2DControl* p = (D2DControl*)ctrl.ctrl;

		p->DestroyControl();

	}	
}
DLLEXPORT void WINAPI DDShow(D2Ctrl ctrl, bool bShow)
{
	if (ctrl.ctrl)
	{
		D2DControl* p = (D2DControl*) ctrl.ctrl;

		p->Visible(bShow);

	}

}


DLLEXPORT D2Ctrls WINAPI DDGetParent(D2Ctrl ctrl)
{
	D2Ctrls r;
	if (ctrl.ctrl)
	{
		D2DControl* p = (D2DControl*) ctrl.ctrl;
		p = p->parent_control_;		
		r.ctrls = p;
		
	}
	return r;

}
DLLEXPORT void WINAPI DDEvent0( D2EVENT0_MODE ev, D2Ctrl ctrl, D2Event0Delegate func )
{
	D2DButton* btn = dynamic_cast<D2DButton*>( (D2DControl*)ctrl.ctrl );

	if ( btn && ev == D2EVENT0_MODE::CLICK )
	{
		btn->OnClick_ = [func,ev](D2DButton* p){
	
			D2Ctrl r;
			r.ctrl = p;
			func( ev, r );
		};
		return;
	}


	D2DDataGrid* dg = dynamic_cast<D2DDataGrid*>((D2DControl*) ctrl.ctrl);
	if (dg && ev == D2EVENT0_MODE::DESTROY)
	{
		dg->Destroy_ = [func, ev](D2DControl* p) {

			D2Ctrl r;
			r.ctrl = p;
			func(ev, r);
		};
		return;
		
	}



}
DLLEXPORT void WINAPI DDEvent1( D2EVENT1_MODE ev, D2Ctrl ctrl, D2Event1Delegate func )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );

	if ( ls && ev == D2EVENT1_MODE::SELECTED )
	{
		ls->onselected_changed_ = [func,ev](D2DListbox* ls,int idx, D2DListboxItem* x )->bool{
						
			D2Ctrl r;
			r.ctrl = ls;
			func( ev, r, idx );

			return false; // Comboboxの場合だけtrueを返す。
		};
		
		return;		
	}

	D2DMessageBox* mb = dynamic_cast<D2DMessageBox*>((D2DControl*) ctrl.ctrl);
	
	if (mb && ev == D2EVENT1_MODE::RESULT)
	{
		mb->onclose_ = [func,ev,mb](D2DMessageBox* p, int result){
			
			D2Ctrl r;
			r.ctrl = mb;
			func(ev, r, result);
			
		};

		return;


	}
	
}
DLLEXPORT void WINAPI DDEvent2( D2EVENT2_MODE ev, D2Ctrl ctrl, D2Event2Delegate func )
{

}
DLLEXPORT void WINAPI DDEvent2Draw(D2EVENT2_MODE ev, D2Ctrl ctrl, D2Event2DrawDelegate func)
{
	D2DDataGrid* dg = dynamic_cast<D2DDataGrid*>((D2DControl*) ctrl.ctrl);
	if (dg && ev == D2EVENT2_MODE::DRAW)
	{
		dg->DrawRow_ = [func,ev](D2DDataGrid* x, D2DContext& cxt, D2DDataGrid::RowInfo& rinfo ){


		DDRowInfo d;
		d.row = rinfo.row;
		d.float_row = rinfo.float_row;
		d.is_selected_row = (rinfo.selected_rows.end() != rinfo.selected_rows.find(rinfo.row));
		d.sz_row = rinfo.sz_row;
		d.col_xpos = rinfo.col_xpos;

		D2Ctrl r;
		r.ctrl = x;
		func(ev, r, &cxt, d);
		};
	}


}

DLLEXPORT void WINAPI DDDTestResourceDestroy( D2DWin w, int layer )
{
	D2DWindow* pw = (D2DWindow*)w.win;
	pw->resource_test_ = true;

}
#pragma region D2List

DLLEXPORT void WINAPI DDListAddItem( D2Ctrl ctrl, LPCWSTR s )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );

	if ( ls )
		ls->AddString( s );
}
DLLEXPORT void WINAPI DDListInsertItem( D2Ctrl ctrl, int idx, LPCWSTR s )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );
}
DLLEXPORT void WINAPI DDListDelItem( D2Ctrl ctrl, int idx )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );
}
DLLEXPORT int WINAPI DDListCurrentIdx( D2Ctrl ctrl )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );
	if ( ls )
		return ls->SelectedIdx();

	return -1;
}
DLLEXPORT void WINAPI DDListClear( D2Ctrl ctrl )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );
}
DLLEXPORT int WINAPI DDListItemCount( D2Ctrl ctrl, int idx )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );

	if ( ls )
		return 0; //ls->->SelectedIdx();

	return 0;


}
DLLEXPORT void WINAPI DDListSetCurrentIdx( D2Ctrl ctrl, int idx )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );
	//if ( ls )
	//	return ls-->SelectedIdx();


}
DLLEXPORT LONG_PTR* WINAPI DDListGetBuffer( D2Ctrl ctrl )
{
	D2DListbox* ls = dynamic_cast<D2DListbox*>( (D2DControl*)ctrl.ctrl );

	if ( ls )
		return ls->GetBuffer();
	return nullptr;
}
#pragma endregion


#pragma region D2Text

DLLEXPORT void WINAPI DDSetText( D2Ctrl ctrl, LPCWSTR text )
{
	D2DTextbox* txt = dynamic_cast<D2DTextbox*>( (D2DControl*)ctrl.ctrl );
	if ( txt )
	{
		txt->SetText( text );
		return;
	}
	D2DControls* ct = dynamic_cast<D2DControls*>( (D2DControl*)ctrl.ctrl );
	if ( ct )
	{
		ct->SetName(text);
		return;
	}
	D2DButton* bct = dynamic_cast<D2DButton*>((D2DControl*) ctrl.ctrl);
	if (bct)
	{
		bct->SetName(text);
		return;
	}
}
DLLEXPORT BSTR WINAPI DDGetText( D2Ctrl ctrl )
{
	D2DTextbox* txt = dynamic_cast<D2DTextbox*>( (D2DControl*)ctrl.ctrl );
	BSTR r = nullptr;
	if ( txt )
		r = ::SysAllocString( txt->GetText().c_str());
	return r;
}

DLLEXPORT void WINAPI DDInsertText(D2Ctrl ctrl, int pos, LPCWSTR text)
{
	D2DTextbox* txt = dynamic_cast<D2DTextbox*>((D2DControl*) ctrl.ctrl);
	BSTR r = nullptr;
	if (txt)
	{
		txt->InsertText(text,pos, lstrlen(text));		
	}
}
#pragma endregion

DLLEXPORT void WINAPI DDSetParameter(D2Ctrl ctrl, const std::map<std::wstring, VARIANT>& prms)
{
	D2DControl* txt = (D2DControl*) ctrl.ctrl;
	if (txt)
	{
		txt->SetParameters( prms );
	}

}

DLLEXPORT D2Ctrls WINAPI DDMkControlsWithTitle( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR classnm, LPCWSTR name)
{
	auto parent = (D2DControls*)ctrls.ctrls;
	
	D2DControlsEx* p = new D2DControlsEx();
	p->CreateWindow( parent->parent_, parent, (rc), VISIBLE|BORDER, name, 0 );

	D2Ctrls r;
	r.ctrls = p;
	return r;

}

DLLEXPORT D2Ctrl WINAPI DDImageLoadFile( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR filename )
{
	auto parent = (D2DControls*)ctrls.ctrls;
	D2DImage* img = new D2DImage();

	img->CreateWindow( parent->parent_, parent, rc,VISIBLE,NONAME );

	D2Ctrl r;
	r.ctrl = nullptr;
	ComPTR<ID2D1DeviceContext> dcxt;
	if ( HR(parent->parent_->cxt_.cxt->QueryInterface( &dcxt )))
	{
		if ( img->LoadImage( dcxt, filename ))
			r.ctrl = img;
	}
	return r;
}




DLLEXPORT D2Ctrl WINAPI DDMkWaiter( D2Ctrls ctrls, const FRectFBM& rc, LPCWSTR name )
{
	auto parent = (D2DControls*)ctrls.ctrls;

	D2DWaiter* ls = new D2DWaiter();
	ls->CreateWindow( parent->parent_, parent, rc, VISIBLE|BORDER,name );
		
	ls->Start();
	D2Ctrl r;
	r.ctrl = ls;
	return r;
}

DLLEXPORT D2Ctrl WINAPI DDMkDGListbox(D2Ctrls ctrls, const FRectFBM& rc, float row_height, bool multiselect, LPCWSTR name)
{
	auto parent = (D2DControls*) ctrls.ctrls;

	D2DDataGridListbox* ls = new D2DDataGridListbox();
	int stat = (multiselect? (VISIBLE | BORDER|MULTISELECT) : (VISIBLE | BORDER));

	ls->CreateWindow(parent->parent_, parent, rc, stat, row_height, name, -1);
	
	D2Ctrl r;
	r.ctrl = ls;
	return r;

}
DLLEXPORT void WINAPI DDAddKeyValue(D2Ctrl ctrl, LPCWSTR key, LPCWSTR value)
{	
	D2DDataGridListbox* ls = dynamic_cast<D2DDataGridListbox*>((D2DControl*) ctrl.ctrl);

	if (ls)
	{
		ls->AddString( key, value );
		return;
	}
	
	D2DDropdownList* ds = dynamic_cast<D2DDropdownList*>((D2DControl*) ctrl.ctrl);
	if (ds)
	{
		ds->AddString(key, value);
		return;
	}
}


DLLEXPORT int  WINAPI DDDGGetSelectedIdx(D2Ctrl ctrl, int* idx )
{
	D2DDataGridListbox* ls = dynamic_cast<D2DDataGridListbox*>((D2DControl*) ctrl.ctrl);

	if (ls)
	{
		auto set = ls->GetSelectIdxes();
		if ( idx != nullptr )
		{
			int i = 0;
			for( auto it = set.begin(); it != set.end(); it++ )
			{
				idx[i++] = *it;
			}

		}
		return set.size();
	}

	return -1;
}
DLLEXPORT LPCWSTR  WINAPI DDDGGetKey(D2Ctrl ctrl, int idx )
{
	D2DDataGridListbox* ls = dynamic_cast<D2DDataGridListbox*>((D2DControl*) ctrl.ctrl);

	if (ls)
	{
		return ls->GetKey(idx);
	}
	return nullptr;
}
DLLEXPORT LPCWSTR  WINAPI DDDGGetValue(D2Ctrl ctrl, LPCWSTR key )
{
	D2DDataGridListbox* ls = dynamic_cast<D2DDataGridListbox*>((D2DControl*) ctrl.ctrl);

	if (ls)
	{
		return ls->GetValue(key);
	}
	return nullptr;
}



DLLEXPORT void DebugTrace(LPCWSTR msg, D2D1_RECT_F& rc)
{
	FString s = FString::Format(L"%s, (%f,%f %f,%f), width=%f, height=%f %x \n", msg, rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, &rc);
	::OutputDebugString(s);

}

DLLEXPORT bool DDOpenTextFile(LPCWSTR fnm, BSTR* ret)
{
	HANDLE pf = NULL;
	BY_HANDLE_FILE_INFORMATION fileinfo_;

	

	long len = 0;
	pf = CreateFile(fnm, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pf != INVALID_HANDLE_VALUE)
	{
		
		auto ReadFile = [pf](LPVOID pbuf, DWORD len)->BOOL {
			DWORD lx = 0;
			return (::ReadFile(pf, pbuf, len, &lx, 0) && lx > 0);
		};

		auto SetSeekPos = [pf](long pos, DWORD method)->DWORD {
			return SetFilePointer(pf, pos, 0, method);
		};
		
		len = GetFileSize(pf, NULL);
		GetFileInformationByHandle(pf, &fileinfo_);
		WCHAR bom = 0, bom2 = 0;

		// read BOM
		{
			ReadFile(&bom, 2);
			ReadFile(&bom2, 2);
		}
		
		enum TYP {ASCII, UTF16,UTF8};

		TYP typ = TYP::ASCII;

		if (bom == 0xFEFF ) // UNICODE
		{
			typ = TYP::UTF16;

			if (bom == 0xFEFF)
				SetSeekPos(2, SEEK_SET);
		}
		else if ((bom == 0XBBEF && ((bom2 & 0x00FF) == 0xBF))) // UTF8 (EF BB BF)
		{
			typ = TYP::UTF8;

			if ((bom == 0XBBEF && ((bom2 & 0x00FF) == 0xBF)))
				SetSeekPos(3, SEEK_SET);
		}
		else
		{
			typ = TYP::ASCII;
			SetSeekPos(0,SEEK_SET);
		}
	

		WCHAR* wcb = nullptr;
		char* cb = new char[len];

		ReadFile(cb, len);
		
		if ( typ == TYP::ASCII )
		{
			len = ::MultiByteToWideChar(CP_ACP, 0, cb, len, 0, 0);

			wcb = new WCHAR[len+1];
			len = ::MultiByteToWideChar(CP_ACP, 0, cb, len, wcb, len);
			wcb[len]= 0;
		}
		else if (typ == TYP::UTF16)
		{			
			len -= 2; 
			DWORD wlen = len / 2;

			wcb = new WCHAR[wlen + 1];
			memcpy( wcb, cb, sizeof(WCHAR)*wlen);
			wcb[len] = 0;
		}
		else if (typ == TYP::UTF8)
		{
			len -= 3;

			DWORD wlen = ::MultiByteToWideChar(CP_UTF8, 0, cb, len, 0, 0);

			wcb = new WCHAR[wlen + 1];
			len = ::MultiByteToWideChar(CP_UTF8, 0, cb, len, wcb, wlen);
			wcb[wlen] = 0;
		}
		
		if ( ret )
			*ret = ::SysAllocString( wcb );


		delete [] cb;
		delete [] wcb;
	}
	return (pf != INVALID_HANDLE_VALUE);



}

DLLEXPORT void WINAPI DDDataGridAllocbuffer(D2Ctrl ctrl, int rowcnt)
{
	D2DDataGrid* tab = dynamic_cast<D2DDataGrid*>((D2DControls*) ctrl.ctrl);
	if (tab)
	{
		tab->AllocateBuffer(rowcnt);
		return;
	}

	D2DDropdownList* ds = dynamic_cast<D2DDropdownList*>((D2DControls*) ctrl.ctrl);
	if ( ds )
	{
		ds->GetListbox()->AllocateBuffer(rowcnt);
		return;
	}


}


DLLEXPORT D2Ctrl WINAPI DDMkDropdownList(D2Ctrls ctrls, const FRectFBM& rc, float row_height, LPCWSTR name)
{
	auto parent = (D2DControls*) ctrls.ctrls;
	D2DDropdownList* ls = new D2DDropdownList();
	ls->CreateWindow(parent->parent_, parent, rc, VISIBLE | BORDER, row_height,name,-1);

	D2Ctrl r;
	r.ctrl = ls;
	return r;

}


DLLEXPORT D2Ctrl WINAPI DDMkMessageBox(D2Ctrls ctrls, const FRectFBM& rc, int typ, LPCWSTR msg, LPCWSTR title, D2MessageBoxDelegate ret)
{
	auto parent = (D2DControls*) ctrls.ctrls;
	D2DMessageBox* mb = new D2DMessageBox();

	mb->onclose_ = [ret](D2DMessageBox* p, int result){

		ret(result);		
	};
	
	mb->DoModal( parent, msg, title, typ);

	D2Ctrl r;
	r.ctrl = mb;
	return r;
}


DLLEXPORT void  WINAPI DDMoveResize(D2Ctrl ctrl, int typ, FRectF dstrc, float millisecond)
{
	if (millisecond <= 0) return;

	D2DControl* pctrl = (D2DControl*) ctrl.ctrl;

	FRectF rc = pctrl->GetRect();

	float offx = dstrc.left - rc.left;


	{
		MoveTarget* m = new MoveTarget();
		

		D2DControl* target = pctrl;

		m->SetParameter(0, 0, offx, 0, millisecond);

		m->Fire_ = [target, rc, dstrc](MoveTarget* p, float offa, float offb)->bool
		{
			float rto = p->GetRto(); // 0-1.0
			

			float left = rc.left + (dstrc.left - rc.left)*rto;
			float right = rc.right + (dstrc.right - rc.right)*rto;
			float top = rc.top + (dstrc.top - rc.top)*rto;
			float bottom = rc.bottom + (dstrc.bottom - rc.bottom)*rto;

			FRectF rcx(left,top,right,bottom);			
			target->SetRect(rcx);

			return true;
		};


		m->End_ = [target, dstrc](MoveTarget* p, float offa, float offb)
		{			
			target->SetRect(dstrc);

			delete p;
		};

		m->Start(pctrl->parent_);
	}
}
DLLEXPORT void  WINAPI DDMove(D2Ctrl ctrl, int typ, float offx, float offy, float millisecond)
{
	if (millisecond <= 0 ) return;

	D2DControl* pctrl = (D2DControl*)ctrl.ctrl;

	FRectF rc = pctrl->GetRect();
	
	
	{
		//MoveTargetEx* m = new MoveTargetEx();
		MoveTarget* m = new MoveTarget();
				
		D2DControl* target = pctrl;

		m->SetParameter(0,0, offx,offy, millisecond);

		m->Fire_ = [target, rc](MoveTarget* p, float offa, float offb)->bool
		{					
			FRectF rcx = rc;
			rcx.Offset( offa, offb );			
			target->SetRect(rcx);

			return true;
		};


		m->End_ = [](MoveTarget* p, float offa, float offb)
		{
			delete p;
		};

		m->Start(pctrl->parent_);
	}
}

DLLEXPORT D2Ctrls WINAPI DDMkTableControls(D2Ctrls ctrls, const FRectFBM& rc, int rowcnt, int colcnt, LPCWSTR name)
{
	auto parent = (D2DControls*) ctrls.ctrls;
	D2DTableControls* pc = new D2DTableControls();
	pc->CreateWindow( parent->parent_, parent, rc, VISIBLE,rowcnt, colcnt, name );


	D2Ctrls r;
	r.ctrls = pc;
	return r;
}
DLLEXPORT void WINAPI DDSetControlPlace(D2Ctrls ctrls, D2Ctrl ctrl, int row, int col)
{
	D2DTableControls* pc = dynamic_cast<D2DTableControls*>( (D2DControls*) ctrls.ctrls );
	if ( pc )
	{
		D2DControl* pcc = (D2DControl*)ctrl.ctrl;

		pc->SetControl( row,col, pcc );
	}	
}
