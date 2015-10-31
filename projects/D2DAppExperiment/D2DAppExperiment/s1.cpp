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

#include "D2DWindowControl_easy.h"

using namespace V4;


class MyBUtton : public D2DControl
{
	public :
		MyBUtton()
		{

		}
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
		{
			D2DControl::CreateWindow(parent,pacontrol,rc,stat,name,id );

		}
		virtual LRESULT WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch( message )
			{
				case WM_PAINT:
				{
					auto& cxt = d->cxt_;
					D2DMatrix mat(cxt);

					mat.PushTransform();
					mat.Offset( rc_ );


					

					FRectF rc(0,0,300,30);
					cxt.cxt->DrawRectangle( rc, cxt.black );
					
					DrawCenterText( cxt, cxt.red, rc, L"これはテスト", 6, 0,0 );



					mat.PopTransform();
				}
				break;
			
			
			
			
			}	
			return 0;

		}



};





DLLEXPORT void CreateD2DDesktop2( D2Ctrls  ctrls)
{
	D2DControls* pc = (D2DControls*)ctrls.ctrls;
	
	
	MyBUtton* m = new MyBUtton();

	FRectF rc(100,100,FSizeF(400,60));
	m->CreateWindow( pc->parent_, pc, rc, VISIBLE, NONAME, 0);

}
