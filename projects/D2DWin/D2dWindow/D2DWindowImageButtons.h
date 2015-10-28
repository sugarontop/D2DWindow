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

// The one image has some buttons. Button have transparent area.

class D2DImageButtons : public D2DControl
{
	public :
		D2DImageButtons();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id = -1);

		void SetButtons(const D2D1_RECT_F* rcs, DWORD* mode, int cnt);
		void SetBackColor( D2D1_COLOR_F offclr, D2D1_COLOR_F onclr, D2D1_COLOR_F floatclr,D2D1_COLOR_F disableclr);
		void SetButtonImage(void* img, int length);

		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

		std::function<void(D2DImageButtons*, int idx)> onclick_;

	protected :
		void OnPaintType1(D2DWindow* d);
		void OnPaintType2(D2DWindow* d);
	private :
		std::shared_ptr<FRectF> rcs_;
		std::shared_ptr<DWORD> btn_mode_;

		int btn_cnt_;
		SolidColor offbr_,onbr_,floatbr_,disablebr_,back_color_;
		ID2D1Bitmap* img_;
		ComPTR<ID2D1Bitmap> bmp;
		bool bresize_width;

		Binary img_bin_;

		int float_idx_, last_idx_;

		
		
};


#define MODE_ENABLE				0x1
#define MODE_CLICK_ONLY			0x2
#define MODE_ONOFF				0x4
#define STAT_ON					0x8
#define MODE_ONOFF_OTHERALLOFF	0x10

#define IS_DISABLE(md)		((md&MODE_ENABLE) == 0)
#define IS_CLICKONLY(md)	(md&MODE_CLICK_ONLY)
#define IS_ONOFF(md)		(md&MODE_ONOFF)
#define IS_ON(md)			(md&STAT_ON)

};

