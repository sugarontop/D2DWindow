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

using namespace V4;
using namespace std;


static DDImage g1, g2;
const float title_height = 26;

void ListboxTest1(D2Ctrls t0, FRectF rc);
void ListboxTest2(D2Ctrls t0, FRectF rc); 
void ListboxTest3(D2Ctrls t0, FRectF rc);

void TextboxTest(D2Ctrls t0, FRectF rc);


void CreatePage0(D2Ctrls t0)
{
	std::map<std::wstring, VARIANT> m;

	_variant_t vf(L"Arial");
	_variant_t color((UINT) D2DRGBADWORD(87, 106, 148, 255));
	m[L"fontname"] = vf;
	m[L"fontheight"] = _variant_t(14);
	m[L"fontbold"] = _variant_t(800);
	m[L"color"] = color;


	FRectF rc1(820, 70, V4::FSizeF(200, 30));
	FRectF rc2(1150, 70, V4::FSizeF(200, 30));
	FRectF rc3(120, 70, V4::FSizeF(600, 30));
	FRectF rc4(120, 470, V4::FSizeF(600, 30));


	auto label1 = DDMkStatic(t0, rc1, 1, L"DropDownList");
	auto label2 = DDMkStatic(t0, rc2, 1, L"Listbox");
	auto label3 = DDMkStatic(t0, rc3, 1, L"Grid");
	auto label4 = DDMkStatic(t0, rc4, 1, L"Textbox");


	DDSetParameter(label1, m);
	DDSetParameter(label2, m);
	DDSetParameter(label3, m);
	DDSetParameter(label4, m);


	rc1.Offset(0, 30);
	rc2.Offset(0, 30);
	rc3.Offset(0, 30);
	rc4.Offset(0, 30);

	
	ListboxTest1(t0, rc1);
	ListboxTest2(t0, rc2);
	ListboxTest3(t0, rc3);
	TextboxTest(t0, rc4);

}




bool LoadResourceFromRC( UINT id, LPCWSTR Section, DDImage& r)
{
	HMODULE hModule = ::GetModuleHandle(NULL);
	HRSRC hsrc = FindResource(hModule, MAKEINTRESOURCE(id), Section);
	DWORD len = SizeofResource(hModule, hsrc);
	HGLOBAL hg = LoadResource(hModule, hsrc);
	r.LockedResource =  LockResource(hg);
	r.Length = len;
	return true;
}


void ListboxTest3(D2Ctrls t0, FRectF rc)
{

	struct RowStruct
	{
		int img;
		wstring status;
		wstring name;
		wstring comment;
	};


	rc.SetSize( 600,350 );

	std::vector<RowStruct>* data = new std::vector<RowStruct>();

	auto ls = DDMkDataGrid(t0, rc ,4, 26, title_height, NONAME);
		

	for( int i = 0; i < 100; i++ )
	{
		RowStruct s;

		s.img = 0;
		s.status = L"completed";
		s.name = FString::Format(L"%d, MSDN Magazine july", i);
		s.comment = L"unkonwn";

		data->push_back(s);

	}

	DDDataGridAllocbuffer( ls, data->size() );


	LoadResourceFromRC(IDB_PNG1, L"PNG", g1);
	LoadResourceFromRC(IDB_PNG2, L"PNG", g2);

	DDEvent2Draw(D2EVENT2_MODE::DRAW, ls,[data](D2EVENT2_MODE ev, D2Ctrl c, DDContext p, DDRowInfo& r )
	{
		if ( r.row == -1 )
		{
			// タイトル

			if (title_height )
			{
				FRectF rc(r.col_xpos[1],0, r.col_xpos[2], title_height);
				DDDrawStringEx(p, rc, L"status",1 );

				rc.SetRect(r.col_xpos[2], 0, r.col_xpos[3], title_height);
				DDDrawStringEx(p, rc, L"name",1 );

				rc.SetRect(r.col_xpos[3], 0, r.col_xpos[4], title_height);
				DDDrawStringEx(p, rc, L"comment",1);

				DDColor c;
				c.r = 170;
				c.g = 170;
				c.b = 170;
				c.a = 50;
				DDDrawFillRect(p, r.sz_row.width, title_height, c);
			}


			bool bl = DDLoadImage(p, g1 );

			xassert(bl);

			

			bl = DDLoadImage(p, g2 );
			
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

			g1.p.Release();
			g2.p.Release();
			return;
		}

		auto gbmp = (r.row % 6 == 0 ? g2 : g1 );

		std::vector<RowStruct>& ar = *(std::vector<RowStruct>*)data;

		FString x = ar[r.row].status;
		FString n = ar[r.row].name;
		FString w = ar[r.row].comment;
		int color = 0;
			
		if (r.is_selected_row)
		{
			DDColor c;
			c.r = 87;
			c.g = 106;
			c.b = 148;
			c.a = 255;
			DDDrawFillRect(p, r.sz_row.width, r.sz_row.height, c);
			
			DDDrawImage(p, FRectF(2, 3, 18, 19), gbmp);
			DDDrawWhiteString(p, r.col_xpos[1], 0, x.c_str());
			DDDrawWhiteString(p, r.col_xpos[2], 0, n.c_str());
			DDDrawWhiteString(p, r.col_xpos[3], 0, w.c_str());
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
		
		


		DDDrawImage(p, FRectF(2, 3, 18, 19), gbmp);

		DDDrawString(p, r.col_xpos[1], 0, x.c_str());
		DDDrawString(p, r.col_xpos[2], 0, n.c_str());
		DDDrawString(p, r.col_xpos[3], 0, w.c_str());
	});


	DDEvent0(D2EVENT0_MODE::DESTROY, ls, [data](D2EVENT0_MODE ev, D2Ctrl c){

		delete data;
		
	});
	 
	
} 

void ListboxTest1(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(200, 350);

	auto ls = DDMkDGListbox(t0, rc, 26,true, NONAME);


	for( int i = 0; i < 100; i++ )
	{
		FString key = FString::NewGuid();
		FString val = FString::Format( L"MSDN MAGAZINE %d", i );
		DDAddKeyValue( ls, key, val );
	}


	DDDataGridAllocbuffer(ls, 100 );


	
}
void ListboxTest2(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(200, 350);

	auto ls = DDMkDropdownList(t0, rc, 26, NONAME);


	for (int i = 0; i < 100; i++)
	{
		FString key = FString::NewGuid();
		FString val = FString::Format(L"MSDN MAGAZINE %d", i);
		DDAddKeyValue(ls, key, val);
	}


	DDDataGridAllocbuffer(ls, 100);

}


void TextboxTest(D2Ctrls t0, FRectF rc)
{
	rc.SetSize(400,300);

	auto txs = DDMkTextbox( t0, rc, 1, NONAME );

	DDSetText( txs, L"D2DWindow controls.\n\n日本語対応" );


}
