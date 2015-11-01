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
#include "D2DTest2.h"
#include "D2DWin.h"
#include "fstring.h"
#include "resource.h"
#import <msxml6.dll>	// make msxml6.tlh, msxml6.tli 

using namespace V4;
using namespace std;

const float title_height = 26;

D2Ctrl ListboxTest1(D2Ctrls t0, FRectF rc);
D2Ctrl ListboxTest2(D2Ctrls t0, FRectF rc);
D2Ctrl ListboxTest3(D2Ctrls t0, FRectF rc);
D2Ctrl TextboxTest(D2Ctrls t0, FRectF rc);

void MessageBoxTest(D2Ctrls t0, FRectF rc);
void CreatePageA(D2Ctrls t0);
void CreatePageB(D2Ctrls t0);
void MoveTest(D2Ctrls t0, D2Ctrl target, FRectF rc);
void LoginTest(D2Ctrls t0, FRectF rc);

bool LoadResourceFromRC(UINT id, LPCWSTR Section, DDImage& r)
{
	HMODULE hModule = ::GetModuleHandle(NULL);
	HRSRC hsrc = FindResource(hModule, MAKEINTRESOURCE(id), Section);
	DWORD len = SizeofResource(hModule, hsrc);
	HGLOBAL hg = LoadResource(hModule, hsrc);
	r.LockedResource = LockResource(hg);
	r.Length = len;
	return true;
}


void CreatePage0(D2Ctrls t0)
{
	std::map<std::wstring, VARIANT> m;
	auto t1 = t0; // DDMkControls(t0, FRectF(0,0,9000,1000), L"group", NONAME);
	

	auto tab = DDMkTabControls(t1,FRectF(),NONAME);

	auto page2 = DDAddPage(tab,-1);

	auto page1 = DDGetPage(tab,0);

	_variant_t nm(L"page2");
	m[L"name"] = nm;
	DDSetParameter(Ctrl(page2), m);

	CreatePageA( page1 );



	CreatePageB(page2);
}

void CreatePageA(D2Ctrls t0)
{
	std::map<std::wstring, VARIANT> m;

	_variant_t color((UINT) D2DRGBADWORD(87, 106, 148, 0));

	//m[L"moveable"] = variant_t(TRUE); // TRUEで移動できます。
	m[L"bordercolor"] = color;

	DDSetParameter(Ctrl(t0), m);


	m.clear();
	

	_variant_t vf(L"Arial");
	color = _variant_t((UINT) D2DRGBADWORD(87, 106, 148, 255));
	m[L"fontname"] = vf;
	m[L"fontheight"] = _variant_t(14);
	m[L"fontbold"] = _variant_t(800);
	m[L"color"] = color;


	FRectF rc1(620, 70, V4::FSizeF(200, 30));
	FRectF rc2(900, 70, V4::FSizeF(200, 30));
	FRectF rc3(120, 70, V4::FSizeF(450, 30));
	FRectF rc4(120, 470, V4::FSizeF(600, 30));
	FRectF rc5(600, 470, V4::FSizeF(600, 30));
	FRectF rc6(830, 470, V4::FSizeF(600, 30));
	FRectF rc7(600, 570, V4::FSizeF(600, 30));


	auto label1 = DDMkStatic(t0, rc1, 0, L"Listbox");
	auto label2 = DDMkStatic(t0, rc2, 0, L"DropDownList");
	auto label3 = DDMkStatic(t0, rc3, 0, L"Grid");
	auto label4 = DDMkStatic(t0, rc4, 0, L"Textbox");
	auto label5 = DDMkStatic(t0, rc5, 0, L"Messagebox");
	auto label6 = DDMkStatic(t0, rc6, 0, L"遊び(３回で戻る）");
	auto label7 = DDMkStatic(t0, rc7, 0, L"login");

	DDSetParameter(label1, m);
	DDSetParameter(label2, m);
	DDSetParameter(label3, m);
	DDSetParameter(label4, m);
	DDSetParameter(label5, m);
	DDSetParameter(label6, m);
	DDSetParameter(label7, m);


	rc1.Offset(0, 30);
	rc2.Offset(0, 30);

	

	rc3.Offset(0, 30);
	rc4.Offset(0, 30);
	rc5.Offset(0, 30);
	rc6.Offset(0, 30);
	rc7.Offset(0, 30);

	
	auto ls1 = ListboxTest1(t0, rc1);
	auto ls2 = ListboxTest2(t0, rc2);

	
	auto ls3 = ListboxTest3(t0, rc3);
	auto tx = TextboxTest(t0, rc4);

	MessageBoxTest(t0, rc5);

	MoveTest( t0, ls3, rc6 );
	LoginTest( t0, rc7 );

	/////////////////////////////////////////////////////////////////////////

	//LoadResourceFromRC(IDB_PNG1, L"PNG", g4);

	//FRectFBM rc10(1000,20, FSizeF(180, 32));
	//FRectF rc101[4];
	//rc101[0] = FRectF(0, 0, FSizeF(38, 32));
	//rc101[1] = FRectF(43, 0, FSizeF(36, 32));
	//rc101[2] = FRectF(91, 0, FSizeF(33, 32));
	//rc101[3] = FRectF(134, 0, FSizeF(36, 32));

	//DDMkImageButtons(t0, rc10, NONAME, g4, rc101, 4 );


	/////////////////////////////////////////////////////////////////////////

	/*LoadResourceFromRC(IDB_PNG_LOCKED, L"PNG", g5);

	FRectFBM rcBtn1(0, 20, FSizeF(153, 32));
	FRectF rcBtn1s[1];
	rcBtn1s[0] = FRectF(0, 0, FSizeF(153, 32));

	DDMkImageButtons(t0, rcBtn1, NONAME, g5, rcBtn1s, 1);*/

	 
}
void LoginTest(D2Ctrls t0, FRectF rc)
{
	FRectFBM rc1x(rc.left, rc.top, FSizeF(800, 90)); 
	auto t1 = DDMkTableControls(t0, rc1x, 2, 2, NONAME);

	std::map<std::wstring, VARIANT> m;
	m[L"value"] = _variant_t(200);
	m[L"col"] = _variant_t(0);
	DDSetParameter(Ctrl(t1), m);
	
	FRectFBM rc1(0,0, FSizeF(200, 30));
	rc1.Margin_.Set(2);

	auto x1 = DDMkStatic(t1, rc1, 2, L"usercd:");
	auto x1t = DDMkTextbox(t1, rc1, 0, NONAME); DDSetText(x1t, L"000001");

	auto x2 = DDMkStatic(t1, rc1, 2, L"password:");
	auto x2t = DDMkTextbox(t1, rc1, 2, NONAME); 

	DDSetControlPlace(t1, x1, 0, 0);
	DDSetControlPlace(t1, x1t, 0, 1);
	DDSetControlPlace(t1, x2, 1, 0);
	DDSetControlPlace(t1, x2t, 1, 1);
	


}
void LoginTestOld(D2Ctrls t0, FRectF rc)
{
	FRectFBM rc1( rc.left, rc.top, FSizeF(200,30));
	FRectFBM rc1t = rc1;
	rc1t.Offset( 200, 0);
	rc1.Margin_.r = 10;
	rc1t.Margin_.l = 10;
	
	auto x1 = DDMkStatic(t0, rc1, 2, L"usercd:" );
	auto x1t = DDMkTextbox(t0, rc1t, 0, NONAME); DDSetText(x1t, L"000001");

	rc1.Offset(0,50);
	rc1t.Offset(0,50);

	auto x2 = DDMkStatic(t0, rc1, 2, L"password:");
	auto x2t = DDMkTextbox(t0, rc1t, 2, NONAME); //DDSetText(x1t, L"");

}







D2Ctrl ListboxTest1(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(200, 312);

	auto ls = DDMkDGListbox(t0, rc, 26,true, NONAME);


	for( int i = 0; i < 100; i++ )
	{
		FString key = FString::NewGuid();
		FString val = FString::Format( L"MSDN MAGAZINE %d", i );
		DDAddKeyValue( ls, key, val );
	}


	DDDataGridAllocbuffer(ls, 100 );


	return ls;
}
D2Ctrl ListboxTest2(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(200, 312);

	auto ls = DDMkDropdownList(t0, rc, 26, NONAME);


	for (int i = 0; i < 100; i++)
	{
		FString key = FString::NewGuid();
		FString val = FString::Format(L"--MSDN MAGAZINE %d", i);
		DDAddKeyValue(ls, key, val);
	}
	 

	DDDataGridAllocbuffer(ls, 100);



	


	return ls;
}


D2Ctrl TextboxTest(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(400,300);

	auto txs = DDMkTextbox( t0, rc, 1, NONAME );

	DDSetText( txs, L"D2DWindow controls.\n\nTSF IME対応" );

	return txs;
}
void MessageBoxTest(D2Ctrls t0, FRectF rc)
{	
	rc.SetSize(100, 30);
	auto btn = DDMkButton( t0, rc, L"show msgbox" );

	DDEvent0(D2EVENT0_MODE::CLICK, btn, [t0](D2EVENT0_MODE ev, D2Ctrl c) {
		
		FRectF rcx( 100,100, FSizeF(300, 100) );
		
		//DDMkWaiter(t0, rcx, L"ssss");
		

		DDMkMessageBox(t0, rcx, MB_OK, L"これはテスト", L"ここにタイトル", [](int ret){ 
			int x = ret; // IDOK or IDCANCEL			
		});
	});

	
}

void MoveTest(D2Ctrls t0, D2Ctrl target, FRectF rc)
{
	auto test_mode = [](int reset)->int
	{
		static int test_mode = 0;

		if (reset == 0)
			test_mode = 0;
		else
			test_mode++;

		return test_mode;
	};

	rc.SetSize(200, 30);
	auto btn = DDMkButton(t0, rc, L"move GRID");

	DDEvent0(D2EVENT0_MODE::CLICK, btn, [target, test_mode](D2EVENT0_MODE ev, D2Ctrl c) {

		int md = test_mode(1);

		if (md == 1)
		{
			FRectF xrc(1400, 0, FSizeF(100, 200));
			DDMoveResize(target, 0, xrc, 800.0f);
		}
		else if (md == 2)
		{
			FRectF xrc(1000, 10, FSizeF(500, 900));
			DDMoveResize(target, 0, xrc, 800.0f);

		}
		else if (md == 3)
		{
			FRectF xrc(120, 30 + 70, FSizeF(450, 350));
			DDMoveResize(target, 0, xrc, 800.0f);

			test_mode(0);
		}

		

	});
}



D2Ctrl ListboxTest3(D2Ctrls t0, FRectF rc)
{
	FRectF rcbtn( 0,0,100,26);
	FRectF rcDG( 0,50,300,400);

	rcbtn.Offset( 100, 50);
	rcDG.Offset( 100, 50);

	auto ctBtn = DDMkButton( t0, rcbtn, L"load" );

	FString url = L"out.xml"; 

	////////////////////////////////////////////////////////

	
	
	struct RowStruct
	{
		_bstr_t cd;
		_bstr_t status;
		_bstr_t name;
		_bstr_t comment;
	};

	

	std::vector<RowStruct>* data = new std::vector<RowStruct>();

	float w[4];
	w[0] = 100;
	w[1] = 100;
	w[2] = 100;
	w[3] = 100;

	auto ls = DDMkDataGrid(t0, rcDG, 4, w, 26, 26, NONAME );
		

	

	DDEvent2Draw(D2EVENT2_MODE::DRAW, ls,[data](D2EVENT2_MODE ev, D2Ctrl c, DDContext p, DDRowInfo& r )
	{
		if ( r.row == -1 )
		{
			// タイトル

			int title_height = 26;
			if (title_height )
			{
				FRectF rc(r.col_xpos[1],0, r.col_xpos[2], title_height);
				DDDrawStringEx(p, rc, L"status",1 );

				rc.SetRect(r.col_xpos[2], 0, r.col_xpos[3], title_height);
				DDDrawStringEx(p, rc, L"name",1 );

				rc.SetRect(r.col_xpos[3], 0, 999, title_height);
				DDDrawStringEx(p, rc, L"comment",1);

				DDColor c;
				c.r = 170;
				c.g = 170;
				c.b = 170;
				c.a = 50;
				DDDrawFillRect(p, r.sz_row.width, title_height, c);
			}


			return;
		}
		else if (r.row == -2)
		{
			// 境界線

			DDSetAlias(p, true);

			for( int i = 1; i < 4; i++ )
			{
				FPointF pt1(r.col_xpos[i],0);
				FPointF pt2(r.col_xpos[i], 600);
				DDDrawLine( p, pt1, pt2,0 );
			}

			DDSetAlias(p, false );


			return;
		}

		

		std::vector<RowStruct>& ar = *(std::vector<RowStruct>*)data;

		if ( ar.empty()) return;


		_bstr_t cd = ar[r.row].cd;
		_bstr_t x = ar[r.row].status;
		_bstr_t n = ar[r.row].name;
		_bstr_t w = ar[r.row].comment;
		int color = 0;

		
		
			
		if (r.is_selected_row)
		{
			DDColor c;
			c.r = 87;
			c.g = 106;
			c.b = 148;
			c.a = 255;
			DDDrawFillRect(p, r.sz_row.width, r.sz_row.height, c);			
			

			DDDrawString4(p, r.col_xpos[0], r.col_xpos[1],26.0f, cd, cd.length(), 1 );
			DDDrawString4(p, r.col_xpos[1], r.col_xpos[2],26.0f, x, x.length(), 1 );
			DDDrawString4(p, r.col_xpos[2], r.col_xpos[3],26.0f, n, n.length(), 1 );
			DDDrawString4(p, r.col_xpos[3], r.sz_row.width,26.0f, w, w.length(), 1 );

			return;
		}
		else if ( r.row == r.float_row )
		{
			DDColor c;
			c.r=255;
			c.g=0;
			c.b=0;
			c.a=50;
			DDDrawFillRect(p, r.sz_row.width, r.sz_row.height, c);
		}
		
		DDDrawString4(p, r.col_xpos[0], r.col_xpos[1],26.0f, cd, cd.length(), 0 );
		DDDrawString4(p, r.col_xpos[1], r.col_xpos[2],26.0f, x, x.length(), 0 );
		DDDrawString4(p, r.col_xpos[2], r.col_xpos[3],26.0f, n, n.length(), 0 );
		DDDrawString4(p, r.col_xpos[3], r.sz_row.width,26.0f, w, w.length(), 0 );
	});


	DDEvent0(D2EVENT0_MODE::DESTROY, ls, [data](D2EVENT0_MODE ev, D2Ctrl c){

		delete data;
		
	});


	data->clear();

	///////////////////////////////////////////////////////////////////////
	DDEvent0(D2EVENT0_MODE::CLICK, ctBtn, [url,data,ls](D2EVENT0_MODE ev, D2Ctrl)
	{
		MSXML2::IXMLDOMDocument3Ptr doc;
		doc.CreateInstance( __uuidof(MSXML2::DOMDocument60) );

		
		if ( doc->load( _variant_t(url.c_str())) )
		{
			auto root = doc->GetdocumentElement();
			auto lss = root->selectNodes( _bstr_t(L"row"));

			for( int i = 0; i <lss->length; i++ )
			{
				auto row = lss->Getitem(i);
				
				RowStruct s;

				auto ch = row->GetfirstChild();				
				s.cd = ch->Gettext();
				ch = ch->nextSibling;

				s.status = ch->Gettext();

				ch = ch->nextSibling;
				s.name = ch->Gettext();
				ch = ch->nextSibling;
				s.comment = ch->Gettext();

				data->push_back(s);
			}

			DDDataGridAllocbuffer( ls, data->size() );

		}
	});

	return ls;
} 
//////////////////////////////////////////////////////////////////////////////////
void CreatePageB(D2Ctrls t0)
{
	FRectF rc(10,10,FSizeF(200,200));
	for( int i = 0; i < 5; i++ )
	{
		auto prvrc = rc;
		for( int j = 0; j < 5; j++ )
		{
			DDMkFRectFBM( t0, rc, 1, NONAME );

			rc.Offset( 0, 220);
		}
		rc = prvrc;

		rc.Offset( 220, 0 );
	}
}
