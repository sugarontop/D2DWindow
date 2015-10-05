#include "stdafx.h"
#include "D2DTest2.h"
#include "D2DWin.h"
#include "fstring.h"
#include "resource.h"

using namespace V4;
using namespace std;


static DDImage g1, g2,g3,g4,g5;
const float title_height = 26;

D2Ctrl ListboxTest1(D2Ctrls t0, FRectF rc);
D2Ctrl ListboxTest2(D2Ctrls t0, FRectF rc);
D2Ctrl ListboxTest3(D2Ctrls t0, FRectF rc);

D2Ctrl TextboxTest(D2Ctrls t0, FRectF rc);

void MessageBoxTest(D2Ctrls t0, FRectF rc);
void CreatePageA(D2Ctrls t0);
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
	
	auto t1 = DDMkControls(t0, FRectF(0,0,9000,1000), L"group", NONAME);


	std::map<std::wstring, VARIANT> m;

	_variant_t color((UINT) D2DRGBADWORD(87, 106, 148, 0));

	m[L"moveable"] = variant_t(TRUE); // TRUEで移動できます。
	m[L"bordercolor"] = color;

	DDSetParameter(Ctrl(t1), m);


	CreatePageA( t1 );
	//CreatePageB(t1);
}

void CreatePageA(D2Ctrls t0)
{
	std::map<std::wstring, VARIANT> m;

	_variant_t vf(L"Arial");
	_variant_t color((UINT) D2DRGBADWORD(87, 106, 148, 255));
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





D2Ctrl ListboxTest3(D2Ctrls t0, FRectF rc)
{

	struct RowStruct
	{
		int img;
		wstring status;
		wstring name;
		wstring comment;
	};


	rc.SetSize( rc.GetSize().width,350 );

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


	LoadResourceFromRC(IDB_PNG3, L"PNG", g3);



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


			bool bl = DDLoadImage(p, g3);

			
			FRectF rc1(0,0,16,16);
			
			
			DDDivideImage( g3, g1, rc1 );
					
			rc1.Offset(16,0);
			DDDivideImage(g3, g2, rc1);
			
			
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
			g3.p.Release();
			return;
		}

		

		std::vector<RowStruct>& ar = *(std::vector<RowStruct>*)data;

		FString x = ar[r.row].status;
		FString n = ar[r.row].name;
		FString w = ar[r.row].comment;
		int color = 0;

		auto gbmp = (r.row % 6 == 0 ? g2 : g1);
		LPCWSTR status = (r.row % 6 == 0 ? L"fail" : x.c_str());
			
		if (r.is_selected_row)
		{
			DDColor c;
			c.r = 87;
			c.g = 106;
			c.b = 148;
			c.a = 255;
			DDDrawFillRect(p, r.sz_row.width, r.sz_row.height, c);
			
			DDDrawImage(p, FRectF(2, 3, 18, 19), gbmp);
			DDDrawWhiteString(p, r.col_xpos[1], 0, status );
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

		DDDrawString(p, r.col_xpos[1], 0, status );
		DDDrawString(p, r.col_xpos[2], 0, n.c_str());
		DDDrawString(p, r.col_xpos[3], 0, w.c_str());
	});


	DDEvent0(D2EVENT0_MODE::DESTROY, ls, [data](D2EVENT0_MODE ev, D2Ctrl c){

		delete data;
		
	});
	 

	return ls;
	
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
		FString val = FString::Format(L"MSDN MAGAZINE %d", i);
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