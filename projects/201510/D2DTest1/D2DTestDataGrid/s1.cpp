#include "stdafx.h"
#include "D2DTest2.h"
#include "D2DWin.h"


void CreatePage1(D2Ctrls t1)
{

	D2Ctrl xt1;
	xt1.ctrl = t1.ctrls;

	DDSetText( xt1, L"page2" );

	auto t1_1 = DDMkControlsWithTitle( t1, V4::FRectF(10,100,V4::FSizeF(600,750)), nullptr, NONAME );



	FRectFBM rc(0,0,590,650);
	//rc.Margin_.Set(1);

	rc.Margin_.l = 1;

	auto tab = DDMkControls(t1_1, rc, L"tab", NONAME);

	

	auto pg = DDGetPage(tab, 0);
	auto tx = DDMkTextbox(pg, V4::FRectF(220,100,V4::FSizeF(200,550)), 1, NONAME );
	DDSetText( tx, L"Hello world" );

	
	pg = DDAddPage(tab,-1);

	auto tx1 = DDMkTextbox(pg, V4::FRectF(120, 20, V4::FSizeF(400, 30)), 0, NONAME);
	DDSetText(tx1, L"Hello world");
}