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



#include "D2DAppExperiment.h"


using namespace V4;
DLLEXPORT void CreateD2DDesktop( D2Ctrls  ctrls)
{
	
	//DDMkFRectFBM(ctrls,FRectF(10,10,100,100),1,NONAME);


	FRectF rcb1( 20,20, FSizeF(200,30));
	FRectF rcb2( 20,60, FSizeF(200,30));

	FRectF rct1( 20,200, FSizeF(200,30));

	FRectF rct2( 300,200, FSizeF(200,400));

	DDMkButton(ctrls, rcb1, NONAME );
	DDMkButton(ctrls, rcb2, NONAME );

	DDMkTextbox(ctrls, rct1, 0, NONAME );
	DDMkTextbox(ctrls, rct2, 1, NONAME );
}

DLLEXPORT void CreateD2DDesktop3(D2Ctrls  ctrls, float cx, float cy, int xcnt, int ycnt, LPCWSTR str)
{
	FRectFBM rc( 0,0,FSizeF(cx,cy));

	rc.Margin_.Set(2);
	
	for (int y = 0; y < ycnt; y++)
	{
		auto rc1 = rc;
		for( int x = 0; x < xcnt; x++ )
		{
			DDMkStatic( ctrls, rc, 0,str );
			rc.Offset( cx, 0 );
		}
		rc = rc1;
		rc.Offset(0, cy);
	}

}
