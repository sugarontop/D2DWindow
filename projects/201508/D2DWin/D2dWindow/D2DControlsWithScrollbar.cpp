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

#include "stdafx.h"
#include "D2DWin.h"
#include "D2DWindowMessage.h"
#include "D2DWindowControl_easy.h"
#include "D2DCommon.h"
#include "D2DWindowMessageStruct.h"
using namespace V4;

#pragma region  D2DControlsWithScrollbar
D2DControlsWithScrollbar::D2DControlsWithScrollbar()
{

}
D2DControlsWithScrollbar::~D2DControlsWithScrollbar()
{
	int a = 0;
}

void D2DControlsWithScrollbar::ShowScrollbar(TYP typ, bool visible)
{
	if (typ == TYP::VSCROLLBAR)
		Vscbar_->Show(visible);
	else if (typ == TYP::HSCROLLBAR)
		Hscbar_->Show(visible);

}



LRESULT D2DControlsWithScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	bool bProcess = true;

	if (!VISIBLE(stat_))
		return ret;

	D2DContext& cxt = d->cxt_;

	D2DMatrix mat(cxt);

	mat.PushTransform();
	mat.Scale(scale_, scale_);

	switch (message)
	{
	case WM_PAINT:
	{

		mat.Offset(rc_.left, rc_.top);
		mat_ = mat;


		FRectF rcborder = rc_.GetContentRectZero();
		//FRectF rcborder2 = rc_.GetContentRect().ZeroRect();
		rcborder.right--;

		auto br = (stat_ & BORDER ? cxt.black : cxt.transparent);

		cxt.cxt->FillRectangle(rcborder, cxt.white);//

		

		FRectF rc = rc_.GetContentRectZero();
		D2DRectFilter f(cxt, rc);

		//mat.Offset( rcborder.left, rcborder.top );		
		//mat_ = mat; // 自座標(左上0,0)のmatrix

		mat.PushTransform();
		{
			mat.Offset(-scrollbar_off_.width, -scrollbar_off_.height);

			LRESULT r = 0;
			auto it = controls_.begin();

			_ASSERT((*it).get() == Vscbar_);

			it++; // pass Vscbar_
			it++; // pass Hscbar_

			for (; it != controls_.end(); ++it) {
				(*it)->WndProc(d, message, wParam, lParam);
			}

		}
		mat.PopTransform();

		bProcess = false;

		//cxt.SetAntiAlias(true);

		Vscbar_->WndProc(d, message, wParam, lParam);
		Hscbar_->WndProc(d, message, wParam, lParam);
	}
	break;

	case WM_MOUSEMOVE:
	{
		FPointF pt = mat_.DPtoLP(FPointF(lParam));
		pt.x += rc_.left;
		pt.y += rc_.top;

		if (rc_.PtInRect(pt))
			ret = D2DControls::WndProc(d, message, wParam, lParam);

		bProcess = false;

	}
	break;
	case WM_MOUSEWHEEL:
	{
		FPointF pt = mat_.DPtoLP(FPointF(lParam));
		pt.x += rc_.left;
		pt.y += rc_.top;

		if (rc_.PtInRect(pt))
			ret = D2DControls::WndProc(d, message, wParam, lParam);

		bProcess = false;
	}
	break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_CAPTURECHANGED:
	{
		FPointF pt = mat_.DPtoLP(FPointF(lParam));
		pt.x += rc_.left;
		pt.y += rc_.top;

		if (rc_.PtInRect(pt))
		{
			ret = D2DControls::WndProc(d, message, wParam, lParam);
			bProcess = false;
		}
	}
	break;
	case WM_D2D_OBJECT_UPDATE:
	{
		// Dropされたobjectを受け入れるか。
		if (bDropped_object_accept_)
		{
			D2D_OBJECT_UPDATE* pm = (D2D_OBJECT_UPDATE*) lParam;

			FPointF pt = mat_.DPtoLP(pm->globalpt);
			pt.x += rc_.left;
			pt.y += rc_.top;

			D2DControl* pc = (D2DControl*) pm->object;

			// 自分の領域に落とされた場合
			if (rc_.PtInRect(pt))
			{
				if (this != pc->parent_control_)
				{
					OnDropObject(pc);

					if (pm->stat == D2D_OBJECT_UPDATE::TYP::TRY_NEWPARENT)
						pm->stat = D2D_OBJECT_UPDATE::TYP::SUCCESS;
				}
				else
					pm->stat = D2D_OBJECT_UPDATE::TYP::MOVE;

				// return 1; 
				bProcess = true;

			}
		}
	}
	break;
	case WM_SIZE:
	{
		if (auto_resize_)
		{
			FRectF rc = parent_control_->GetContentRect();


			FSizeF sz = rc.Size();
			rc_.SetContentSize(sz);


			Vscbar_->WndProc(d, message, wParam, lParam);
			Hscbar_->WndProc(d, message, wParam, lParam);

			ret = 1;
		}
	}
	break;
	case WM_D2D_EVSLIDER_CHEANGED:
	{
		//			D2DSlider* sl = (D2DSlider*)wParam;
		//			float pos = (float)lParam;
		//			scale_ = 2*(sl->pos_ / (sl->max_ - sl->min_));

		d->redraw_ = 1;
	}
	break;
	}



	if (bProcess && ret == 0)
		ret = D2DControls::WndProc(d, message, wParam, lParam);


	mat.PopTransform();

	return ret;
}
void D2DControlsWithScrollbar::OnDropObject(D2DControl* xpc)
{
	auto xpc2 = xpc->UnPack();

	if (xpc2->parent_control_ != this)
	{
		int stat = xpc2->GetStat();
		stat &= ~VISIBLE; // WndProcをスルーするためにVISIBLEをとる
		xpc2->SetStat(stat);

		//		D2DDropedPack* pc = new D2DDropedPack();
		//		pc->CreateWindow( parent_, this, FRectF(0,0,50,50), VISIBLE, L"suicide" );
		//		pc->Pack( xpc2 );
	}
	else
	{
		// 元のparent_controlに戻ってきたので、元に戻す。

		int stat = (xpc2->GetStat() | VISIBLE);
		xpc2->SetStat(stat);
		FRectFBoxModel rc = xpc2->GetRect();


		rc.SetPoint(0, 0);

		xpc2->SetRect(rc);

	}

}


void D2DControlsWithScrollbar::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int typ, int id)
{

	FRectFBoxModel rca1(rc);

	bDropped_object_accept_ = false;
	rca1.BoderWidth_ = 1;



	D2DControls::CreateWindow(parent, pacontrol, rca1, stat, name, id);

	int Vtyp = (typ == 0 || typ == 1 ? VISIBLE : 0);
	int Htyp = (typ == 0 || typ == 2 ? VISIBLE : 0);


	FRectF xrc = rca1.GetContentRectZero();
	xrc.left = xrc.right - 18;


	if (typ == 0)
		xrc.bottom -= 18;
	Vscbar_ = new D2DScrollbar();
	Vscbar_->CreateWindow(parent, this, xrc, Vtyp, NONAME);
	Vscbar_->target_control_ = this;
	//Vscbar_->auto_resize_ = true;


	xrc = rca1.GetContentRectZero();
	xrc.top = xrc.bottom - 18;

	if (typ == 0)
		xrc.right -= 18;
	Hscbar_ = new D2DScrollbar();
	Hscbar_->CreateWindow(parent, this, xrc, Htyp, NONAME);
	Hscbar_->target_control_ = this;
	//Hscbar_->auto_resize_ = true;

	scrollbar_off_.height = 0;
	scrollbar_off_.width = 0;
	scale_ = 1;
	auto_resize_ = true;

}
void D2DControlsWithScrollbar::SetTotalSize(float cx, float cy)
{
	if (cy > 0)
	{
		cy = max(rc_.Height(), cy);
		Vscbar_->SetTotalSize(cy);
	}


	if (cx > 0)
	{
		cx = max(rc_.Width(), cx);
		Hscbar_->SetTotalSize(cx);
	}
}


void D2DControlsWithScrollbar::UpdateScrollbar(D2DScrollbar* bar)
{
	if (bar->info_.bVertical)
	{
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	}
	else
	{
		scrollbar_off_.width = bar->info_.position / bar->info_.thumb_step_c;
	}

}



#pragma endregion
