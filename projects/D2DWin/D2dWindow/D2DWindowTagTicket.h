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


class InnerButton : public FRectF 
{
	public :
		InnerButton(){};


};

class D2DTagTicket : public D2DControls
{
	public :
		D2DTagTicket(){};
		
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id=-1 );
		
		
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void OnCreate();
		virtual void BackColor(D2D1_COLOR_F clr);
		
		void DrawTitleButton(D2DContext& cxt);
		void SlideView();
		void DrawSlide(D2DMatrix& mat,D2DContext& cxt);
	private :
		float xoffw_;
		SolidColor fore_color_;
		int md_, vidx_;

		InnerButton innerbtn1;
		InnerButton innerbtn2;

};
};
