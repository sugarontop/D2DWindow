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
