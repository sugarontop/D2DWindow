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

//#include "D2DWindow.h"

namespace V4 {

	class D2DListboxItemEx : public D2DListboxItem
	{
	public:
		D2DListboxItemEx(D2DControls* ctrls) { ctrls_ = ctrls; height_ = ctrls->GetRect().Size().height; }

		// Datagridで使用
		//virtual void DrawBegin( D2DContext& cxt, FSizeF& sz ){};
		//virtual void DrawEnd( D2DContext& cxt, FSizeF& sz ){};

		virtual void Draw(D2DContext& cxt, FSizeF& sz, int stat, void* etc)
		{
			FRectF rc(10, 0, sz.width - 10, sz.height);

			/*FString* s = (FString*)data_.data;

			if ( stat & STAT::MOUSEMOVE )
			{
			FillRoundRect(cxt,cxt.factory(), rc, 3, data_.color_mouse_move_back );
			V4::DrawCenterText( cxt.cxt, cxt.black, rc,s->c_str(), s->length(), 0 );
			}
			else if ( (stat & STAT::SELECTED) && (stat & STAT::CAPTURED) )
			{
			FillRoundRect(cxt, cxt.factory(), rc, 3, data_.color_selected_back );
			V4::DrawCenterText( cxt.cxt, data_.color_selected_text, rc,s->c_str(), s->length(), 0 );

			}
			else if ( stat & STAT::SELECTED )
			{
			FillRoundRect(cxt, cxt.factory(), rc, 3, data_.color_selected_back );
			V4::DrawCenterText( cxt.cxt, cxt.black, rc,s->c_str(), s->length(), 0 );
			}
			else
			V4::DrawCenterText( cxt.cxt, cxt.black, rc,s->c_str(), s->length(), 0 );*/


			if (data_.data)
			{
				LPCWSTR x = (LPCWSTR) data_.data;
				//V4::DrawCenterText( cxt.cxt, cxt.black, rc,x, lstrlen(x), 0 );
			}


			ctrls_->WndProc(ctrls_->parent_, WM_PAINT, 0, 0);



		}
		virtual float TitleHeight() { return height_; }

		virtual D2DListboxItem* Clone()
		{
			return mkclone_();
		}

		virtual void BindImage(D2DContextText& cxt, FSizeF& sz) {}

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
		{

			return ctrls_->WndProc(parent, message, wParam, lParam);

			return 0;
		} // OnSelectedItem時, captureされる。動作

		void* GetData() { return data_.data; }
	public:
		virtual void Bind(D2DWindow* win, BindData& data)
		{
			data_ = data;

			//FString s = *(FString*)data.data;

			ctrls_->Bind(data.data);
		}

		D2DControls* ctrls_;

		std::function<D2DListboxItem*()> mkclone_;

	protected:
		float height_;
	};


	class D2DControlsTest : public D2DControls
	{
	public:
		D2DControlsTest() {}
		virtual LRESULT WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		
		virtual void Bind(void* p)
		{
			if (bind_)
				bind_(p);
		}
		std::function<void(void*p)> bind_;

	};
};
