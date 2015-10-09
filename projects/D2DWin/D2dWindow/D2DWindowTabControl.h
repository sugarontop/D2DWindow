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

#pragma once

namespace V4 {

/////////////////////////////////////////////////////////////
//
// Tab付きControls page1,page2,page3,,
//
/////////////////////////////////////////////////////////////

class D2DTabControls : public D2DControls
{
	public :
		D2DTabControls();
		virtual ~D2DTabControls();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );

		FRectF OnTagDraw( D2DContext& cxt );

		virtual FRectF GetContentRect();
		D2DControls* Child( UINT idx );
		UINT AddChild( LPCWSTR name );
		UINT CountChild(){ return controls_.size(); }
		
		void TagEnable( UINT idx, bool bEnable );
		
		void AutoScale( bool bWidth, bool bHeight );
		void SetActivePage( int idx );
	
	protected :
		UINT HitTagPoint( FPointF pt );

	private :
		UINT active_idx_;
		UINT scale_;

		enum AUTO{ WIDTH=0x1, HEIGHT=0x2 };

};

};
