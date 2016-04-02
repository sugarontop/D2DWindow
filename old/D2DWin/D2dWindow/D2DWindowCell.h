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
#pragma once


namespace V4 {


class TableFRectF
{
	public :
		TableFRectF();
		TableFRectF(int rowcnt, int colcnt);
		~TableFRectF();

		void Set(int rowcnt, int colcnt);
	
		FRectF GetRectF(int row, int col);

		void SetColWidth( int col, float width );
		void SetRowHeight( int row, float height );
		int ColCnt(){ return colcnt_; }
		int RowCnt() { return rowcnt_; }

		UINT magicnumber(int row, int col);
		FRectF GetRectF(UINT magicnumber);
	private :
		float* xpos_;
		float* ypos_;

		int rowcnt_,colcnt_;

};

class D2DTableControls : public D2DControls
{
	public :
		// D2DControl interface///////////////////////////////////////////
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, int rowcnt,int colcnt, LPCWSTR name, int id = -1);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		void SetControl( int row, int col, D2DControl* ctrl );
		void Draw(D2DContext& cxt);
	public :
		
	protected :
		TableFRectF trc_;
		std::map<UINT, D2DControl*> tctrl_;
};

};




