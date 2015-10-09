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
#include "D2DWindowControl_easy.h"
#include "ItemLoopArray.h"
#include "D2DWindowDataGrid.h"
#include "D2DContextEx.h"
#include "gdi32.h"

namespace V4 {

D2DDataGrid::D2DDataGrid()
{

}
D2DDataGrid::~D2DDataGrid() 
{
	Clear();
}
void D2DDataGrid::Clear()
{
	if (databox_ && Destroy_)
		Destroy_(this);
}
void D2DDataGrid::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height,float title_height, int colcnt, LPCWSTR name, int id)
{
	D2DControl::InnerCreateWindow(parent, pacontrol, rc, stat, name, id);
	
	title_header_height_ = title_height;
	colcnt_ = colcnt;
	multi_select_ = ( 0 != (stat & MULTISELECT));


	{
		item_min_height_ = item_min_height; // itemの最小高。デフォル値となる。個々のitemの高さを設定する場合は、SetItemHeightにて変更可。
		float_idx_ = idivN_ = -1;
		data_cnt_ = 0;
		int cnt = (int) (rc.GetContentRect().Height() / item_min_height_) + 2; // 高さに依存

		for (int i = 0; i < cnt; i++)
		{
			loopitems_.Add(new Item(0));
		}
		loopitems_.AddEnd();


		FRectFBoxModel rc1(rc.Width() - 20, 0, rc.Width(), rc.Height()- title_header_height_);

		bar_ = new D2DScrollbar();
		bar_->CreateWindow(parent, this, rc1, stat, L"listbox-scrollbar");
		bar_->SetTotalSize(data_cnt_ * item_min_height_);

		bar_->target_control_ = this; // バーのメッセージを受けるオブジェクト


		border_color_.color = ColorF(ColorF::Black);
		fore_color_.color = ColorF(ColorF::Black);
		back_color_.color = D2RGBA(0,0,0,0);
		OnResutructRnderTarget(true);
	}

	OnCreate();
}
LRESULT D2DDataGrid::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	/*if (!VISIBLE(stat_))
	{
		if ( message == WM_D2D_RESTRUCT_RENDERTARGET )
		{
			OnResutructRnderTarget(wParam == 1);
		}		

		return ret;
	}*/

	bool bProcess = true;
	switch( message )
	{
		case WM_PAINT:
		{	
			auto& cxt = d->cxt_;			
			//cxt.cxt->GetTransform(&mat_);

			D2DMatrix mat(cxt);
			mat_ = mat.PushTransform();
			
			const FRectF rcb = rc_.GetBorderRect();
			D2DRectFilter f(cxt, rcb);


			const FRectF rcc = rc_.GetContentRect();

			DrawContent(cxt, loopitems_.Count(), rcc.Width(), item_min_height_, scrollbar_off_.height, this);


			mat.PopTransform();



			{
				// draw srcrollbar
				//D2DMatrix mat(cxt);
				mat.PushTransform();
				mat.Offset(rcc.left, rcc.top+ title_header_height_);
				bar_->WndProc(d, message, wParam, lParam);
				mat.PopTransform();
			}

			cxt.cxt->DrawRectangle(rcc, (ID2D1Brush*)border_color_.br);
			return 0;

		}
		break;	
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP(lParam);

			if (rc_.PtInRect(pt))
			{
				auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				float y = bar_->info_.position;
				y += (zDelta > 0 ? -10 : 10);

				float scroll_view_height = rc_.GetContentRect().Height()- title_header_height_;

				float pos_max = (bar_->info_.total_height - scroll_view_height) * bar_->info_.thumb_step_c;
				
				bar_->info_.position = max(0, min(y, pos_max));

				UpdateScrollbar(bar_);

				d->redraw_ = 1;
				return 1;
			}
		}
		break;
		
		/*case WM_SIZE:
		{
			FSizeF sz = parent_control_->GetRect().Size();
			rc_.right = rc_.left + sz.width;
		}
		break;*/
		
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		{
			if (stat_ & VISIBLE)
			{
				FPointF pt(lParam);
				pt = mat_.DPtoLP(pt);

				FRectF rc(rc_);
				rc.right -= bar_->GetRect().Width();


				if (message == WM_LBUTTONDOWN)
				{
					//ATLTRACE( L"left=%f top %f this=%x\n", mat_._31, mat_._32, this );
				}

				if (message == WM_LBUTTONDOWN)
				{
					if (rc.PtInRect(pt))
					{
						pt.x -= rc_.left;
						pt.y -= rc_.top;

						pt.y += scrollbar_off_.height;
						pt.x += scrollbar_off_.width;

						bool bCombobox;
						bool bCtontrolKey = ( 0!=(wParam&MK_CONTROL));
						OnItemSelected(pt, bCombobox, bCtontrolKey); // 内部でcaptureしている場合あり

						//if (!bCombobox) // comboboxはすぐlbが非表示になる
							d->SetCapture(this);

						ret = 1;
					}
					else
					{
						if (d->GetCapture() == this)
						{
							d->ReleaseCapture();

							
						}

						ret = 0;
					}
				}
				else if (message == WM_MOUSEMOVE)
				{
					active_ = rc.PtInRect2(pt);

					if (active_)
					{
						pt.x -= rc_.left;
						pt.y -= rc_.top;

						pt.y += scrollbar_off_.height;
						pt.x += scrollbar_off_.width;

						OnItemMouseFloat(pt);
						ret = 1;
					}
					else if (float_idx_ > -1)
					{
						float_idx_ = -1;
						d->redraw_ = 1;
					}


				}
				else if (message == WM_LBUTTONUP)
				{

					ret = 0;
					if (d->GetCapture() == this)
					{
						d->ReleaseCapture();
							
						if (onmouse_extend_)
							onmouse_extend_(this, WM_LBUTTONUP);
					}

				}

				
				{
					if (ret == 0 || message == WM_MOUSEMOVE)
					{
						ret = bar_->WndProc(d, message, wParam, lParam);
					}
				}
			}
		}
		break;
		/*case WM_D2D_RESTRUCT_RENDERTARGET:
		{
			OnResutructRnderTarget(wParam == 1);
		}
		break;*/
		case WM_KEYDOWN:
		{
			if (VK_ESCAPE == wParam)
			{				
				if (active_ )
				{									
					UnSelect();
				}
			}
		}
		break;
	}

	if ( ret == 0 && bProcess )
		ret = bar_->WndProc(d, message, wParam, lParam);

	return ret;

}
void D2DDataGrid::AllocateBuffer( int cnt )
{
	Clear();
	
	std::vector<LONG_PTR*> xar;
	xar.resize(cnt);
	databox_ = VectorToSerialEx(xar);
	data_cnt_ = cnt;

	
	ItemsHeight_.clear();
	float h = 0;
	for (int i = 0; i < cnt; i++)
	{
		float itemh = item_min_height_;
		ItemsHeight_.push_back(itemh);

		h += itemh;
	}
	
	bar_->SetTotalSize(h);
}
void D2DDataGrid::UpdateScrollbar(D2DScrollbar* bar)
{
	if (bar->info_.bVertical)
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c; // offセットの絶対値
	else
		scrollbar_off_.width = bar->info_.position;
}
float* D2DDataGrid::ColXPosition()
{
	// とりあえず

	_ASSERT( colcnt_ > 1 );

	float width = rc_.GetContentRect().Width();

	width -= 25;
	float wc = width / (colcnt_-1);

	float* w = new float[colcnt_+1];
	
	w[0] = 0;
	w[1] = w[0]+25;

	for( int i = 2; i <= colcnt_; i++ )
		w[i] = w[i-1]+ wc;

	return w;

}

void D2DDataGrid::DrawContent(D2DContext& cxt, int cnt, float cx, float itemHeight, float offy, void* etc)
{
	_ASSERT( cx >= 0 );

	auto ar = loopitems_.get(cnt);
	float h2 = 0;

	int idivN = CalcdiNEx(offy, h2); // 先頭の頭出し
	idivN_ = idivN;
	float offy1 = offy - h2;

	D2DMatrix mat(cxt);

	mat.PushTransform();
	{
		mat.Offset(rc_.left, rc_.top);

		FSizeF itemsz(cx, itemHeight);
		RowInfo rinfo;
		rinfo.selected_rows = selected_idx_s_;
		rinfo.float_row = float_idx_;
		rinfo.sz_row = itemsz;
		rinfo.col_xpos = ColXPosition();

		// ☆initial method タイトル表示
		rinfo.row = -1;
		DrawRow_(this, cxt, rinfo);
		mat.Offset(0, title_header_height_ );
		
		FRectF rcc1 = rc_.ZeroRect();
		D2DRectFilter f(cxt, rcc1);

		// ☆
		mat.Offset(-scrollbar_off_.width, -offy1);
		mat.PushTransform();
		for (auto it = ar.begin(); it < ar.end(); ++it)
		{
			Item* pd = (Item*) (*it);
			int data_idx = pd->idx() + idivN;

			if (data_idx < (int) data_cnt_)
			{
				rinfo.row = data_idx;
				DrawRow_(this, cxt, rinfo );
			}

			mat.Offset(0, itemsz.height);
		}
		mat.PopTransform();
	
	
		// ☆last method 境界線
		rinfo.row = -2;
		DrawRow_(this, cxt, rinfo);
		delete [] rinfo.col_xpos;
	}
	mat.PopTransform();

	
}



int D2DDataGrid::CalcdiNEx(float offy, float& h2)
{
	int idivN = 0;
	float h = 0;
	h2 = 0;

	for (auto it = ItemsHeight_.begin(); it != ItemsHeight_.end(); ++it)
	{
		h += (*it);

		if (offy > h)
		{
			++idivN;
			h2 = h;
		}
		else
			break;
	}

	return idivN;

}
void D2DDataGrid::OnItemMouseFloat( FPointF mousept)
{
	mousept.y -= title_header_height_;

	int float_idx = CalcIdx(mousept);

	if (float_idx_ != float_idx)
	{
		parent_->redraw_ = 1;
		float_idx_ = float_idx;
	}
}

void D2DDataGrid::OnItemSelected( FPointF mousept, bool& bCombobox, bool bControlKey)
{
	if (idivN_ < 0 ) //|| Items_.size() == 0)
		return;
	if (data_cnt_ == 0)
		return;

	mousept.y -= title_header_height_;

	int selected_idx = CalcIdx(mousept);
	
	
	if ( multi_select_ && bControlKey )
	{
		// 
	}
	else
		selected_idx_s_.clear();
	
	selected_idx_s_.insert(selected_idx);
	
		
	
	float_idx_ = -1;

	int item_idx = selected_idx - idivN_;

	bCombobox = false;

	if (onselected_changed_)
		bCombobox = onselected_changed_(this, selected_idx); // Comboboxの場合だけtrueを返す。
}

int D2DDataGrid::CalcIdx(const FPointF& mousept)
{
	int idx = 0;

	float h = 0;
	for (auto it = ItemsHeight_.begin(); it != ItemsHeight_.end(); ++it)
	{
		if (h <= mousept.y  && mousept.y <= h + (*it))
			break;

		h += (*it);
		++idx;
	}
	return idx;
}
int D2DDataGrid::GetSelectedIdx()
{
	if (selected_idx_s_.empty()) return -1;

	return *selected_idx_s_.begin();
}
void D2DDataGrid::UnSelect()
{
	selected_idx_s_.clear();
}
void D2DDataGrid::SetRect(const FRectFBoxModel& rc)
{
	rc_ = rc;

	// 遅くなる
	//{

	//	int cnt = (int) (rc.GetContentRect().Height() / item_min_height_) + 2; // 高さに依存

	//	loopitems_.Clear();
	//	for (int i = 0; i < cnt; i++)
	//	{
	//		loopitems_.Add(new Item(0));
	//	}
	//	loopitems_.AddEnd();
	//}

	FRectFBoxModel rc1(rc.Width() - 20, 0, rc.Width(), rc.Height() - title_header_height_);
	bar_->SetRect(rc1);
}
void D2DDataGrid::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");
	ParameterColor(parent_, fore_color_, prms, L"forecolor");
}
void D2DDataGrid::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);
		fore_color_.br = parent_->GetSolidColor(fore_color_.color);

	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
		fore_color_.br = nullptr;
	}

	bar_->OnResutructRnderTarget(bCreate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SolidColor GetSolidColor(D2DWindow* d, D2D1_COLOR_F  clr)
{
	SolidColor r;
	r.color = clr;
	r.br = d->GetSolidColor(r.color);
	return r;
}

void D2DDataGridListbox::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, LPCWSTR name, int id)
{	
	D2DDataGrid::CreateWindow(parent, pacontrol, rc, stat, item_min_height,0, 1, name, id);



	wfloating.color = D2RGBA(0, 0, 255, 30);
	wfloating_w.color = D2RGB(0, 0, 255);
	wselecting.color = D2RGBA(255, 0, 0, 30);
	wselecting_w.color = D2RGB(255, 0, 0);
	
	border_color_.color = ColorF(ColorF::Black);
	fore_color_.color = ColorF(ColorF::Black);
	back_color_.color = D2RGBA(0, 0, 0, 0);
	OnResutructRnderTarget(true);

	DrawRow_ = [this](D2DDataGrid*, D2DContext& cxt, RowInfo& rinfo){

		auto& r = rinfo;
		
		if ( r.row < 0 )
			return;

		LPCWSTR key = ar_[r.row]->key.c_str();
		auto str = keyvalues_[key]->val;
		FRectF rc(3, 0, rinfo.sz_row);

		if (r.selected_rows.find(r.row) != r.selected_rows.end())
		{
			DrawRect(cxt,rc, (ID2D1Brush*)wselecting_w.br, 1 );
			FillRectangle(cxt, rc, (ID2D1Brush*) wselecting.br );

		}
		else if (r.row == r.float_row)
		{
			DrawRect(cxt, rc, (ID2D1Brush*) wfloating_w.br, 1);
			FillRectangle(cxt, rc, (ID2D1Brush*) wfloating.br);
		}

		DrawCenterText(cxt, (ID2D1Brush*)fore_color_.br, rc, str.c_str(), str.length(), 0);
	};

	title_header_height_ = 0;

}
void D2DDataGridListbox::AddString(LPCWSTR key, LPCWSTR val)
{
	std::shared_ptr<kv> x = std::shared_ptr<kv>( new kv());
	x->key = key;
	x->val = val;
	ar_.push_back(x);

	keyvalues_[ key ] = x.get();
}
int D2DDataGridListbox::Count()
{
	return ar_.size();
}
LPCWSTR D2DDataGridListbox::GetValue(LPCWSTR key)
{
	auto it = keyvalues_.find( key );
	if ( it != keyvalues_.end())
	{
		return it->second->val.c_str();
	}
	return nullptr;
}
void D2DDataGridListbox::Clear()
{
	keyvalues_.clear();
	ar_.clear();
}
LPCWSTR D2DDataGridListbox::GetKey(int idx)
{
	if ( idx < (int)ar_.size())
		return ar_[idx]->key.c_str();
	return nullptr;
}
float* D2DDataGridListbox::ColXPosition()
{
	float* r = new float[1];
	*r = rc_.Size().width;
	return r;
}
void D2DDataGridListbox::SetRect(const FRectFBoxModel& rc)
{
	D2DDataGrid::SetRect(rc);
	

}
void D2DDataGridListbox::OnResutructRnderTarget(bool bCreate)
{
	D2DDataGrid::OnResutructRnderTarget(bCreate);

	if (bCreate)
	{
		wfloating = GetSolidColor(parent_, wfloating.color );
		wfloating_w = GetSolidColor(parent_, wfloating_w.color);
		wselecting = GetSolidColor(parent_, wselecting.color);
		wselecting_w = GetSolidColor(parent_, wselecting_w.color);
	}
	else
	{
		wfloating.br = nullptr;
		wfloating_w.br = nullptr;
		wselecting.br = nullptr;
		wselecting_w.br = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DDropdownList::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, float item_min_height, LPCWSTR name, int id)
{
	D2DControl::InnerCreateWindow(parent, pacontrol, rc, stat, name, id);
	auto_resize_ = false;

	ls_ = new D2DDataGridListbox();

	FRectFBoxModel rcc = rc.ZeroRect();
	rcc.BorderWidth_ = 0;

	ls_->CreateWindow(parent,this, rcc, VISIBLE|BORDER, item_min_height, NONAME, -1 );

	headrc_ = rc_.GetContentRect();

	headrc_.SetSize(headrc_.Size().width, item_min_height);

	ls_->Visible(false);
	
	ls_->onmouse_extend_ = [this](D2DDataGrid* gls, UINT msg ){

		D2DDataGridListbox* ls = (D2DDataGridListbox*)gls;
		if ( msg == WM_LBUTTONUP )
		{
			int idx = ls->GetSelectedIdx();
			
			key_ = ls->GetKey(idx);
			value_ = ls->GetValue(key_.c_str());


			ls->Visible(false);
			ls_->UnSelect();

			if ( this == ls->parent_->GetCapture())
			{
				parent_->ReleaseCapture();



			}
		}


	};
	fore_color_.color = ColorF(ColorF::Black);
	border_color_.color = ColorF(ColorF::Black);
	back_color_.color = D2RGBA(0, 0, 0, 0);

	OnResutructRnderTarget(true);

}
void D2DDropdownList::AddString(LPCWSTR key, LPCWSTR val)
{
	ls_->AddString(key,val);

}

void D2DDropdownList::SetRect(const FRectFBoxModel& rc)
{
	rc_ = rc;

	float item_min_height = headrc_.Size().height;

	headrc_ = rc_.GetContentRect();
	headrc_.SetSize(headrc_.Size().width, item_min_height);

	FRectFBoxModel rcc(0, 0, rc.Width(), rc.Height());
	ls_->SetRect(rcc);
}

LRESULT D2DDropdownList::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if (!VISIBLE(stat_))
		return ret;

	
	switch (message)
	{
		case WM_PAINT:
		case WM_D2D_PAINT:
		{
			auto& cxt = d->cxt_;

			D2DMatrix mat(cxt);

			if (message == WM_D2D_PAINT)
				mat.PushTransformInCaputre(mat_);
			else
				mat_ = mat.PushTransform();


			auto rcc = headrc_.ZeroRect();
			const float hh = rcc.Height();

			// stroke line
			{
				FRectF rcb = rc_.GetBorderRect();
				rcb.bottom = rcb.top + hh + rc_.BorderWidth_;

				D2DRectFilter f(cxt, rcb);
				
				cxt.cxt->DrawRectangle(rcb, (ID2D1Brush*) border_color_.br);

				cxt.cxt->FillRectangle(rcb, (ID2D1Brush*) back_color_.br);
			}

			

			// content			
			
			
			FRectF rcc1 = rc_.GetContentRect();
			mat.Offset(rcc1);
			{
				{
						

					FillRectangle( cxt, rcc, (ID2D1Brush*) back_color_.br);

					//DrawControlRect( cxt, rcc, (ID2D1Brush*)border_color_.br,(ID2D1Brush*) back_color_.br);
					D2DRectFilter f(cxt, rcc);
									
					DrawCenterText( cxt, (ID2D1Brush*)fore_color_.br, rcc, value_.c_str(), value_.length(),0,3);
					
					FRectF rccb(rcc);
					rccb.left = rccb.right - 25;
					DrawCenterText(cxt, (ID2D1Brush*)fore_color_.br, rccb, L"<<", 2, 0, 0);
				}
			
				if (ls_->IsVisible())
				{
					mat.Offset(-(rc_.BorderWidth_ + rc_.Padding_.l), hh );
					ls_->WndProc(d,message,wParam,lParam );								
				}
			}
			mat.PopTransform();



			return 0;
		}	
		break;
		case WM_SIZE:
		{
			FSizeF sz;

			auto p = dynamic_cast<IUpdatar*>( parent_control_ );
			if ( p  )
			{
				p->RequestUpdate(this, WM_SIZE );

				sz = rc_.GetContentRect().Size();
				headrc_.right = headrc_.left + sz.width;
			}
			else if (auto_resize_ )
			{
				FRectFBM rc = parent_control_->GetRect();
						
				sz = rc.GetContentRect().Size();
				rc_.right = rc_.left + sz.width;
				headrc_.right = headrc_.left + sz.width;				
			}

			SetRect( rc_ );
			
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt(lParam);
			pt = mat_.DPtoLP(pt);
			FRectF rc(headrc_);

			if (rc.PtInRect(pt))
			{				
				if ( !ls_->IsVisible())
				{
					ret = 1;

					d->SetCapture(this);
					ls_->Visible(true);
				}
			}
		}
		break;

		case WM_KEYDOWN:
		{
			if ( VK_ESCAPE == wParam )
			{
				if ( this == d->GetCapture())
				{
					d->ReleaseCapture();
					ret = 1;

					ls_->Visible(false);
					ls_->UnSelect();
				}
			}
		}
		break;

	}


	if ( ret == 0 )
	{
		ret = ls_->WndProc(d, message, wParam, lParam);

	}


	return ret;
}

void D2DDropdownList::OnResutructRnderTarget(bool bCreate)
{	
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);
		fore_color_.br = parent_->GetSolidColor(fore_color_.color);
	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
		fore_color_.br = nullptr;
	}
	
	ls_->OnResutructRnderTarget(bCreate);
}
void D2DDropdownList::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");
	ParameterColor(parent_, fore_color_, prms, L"forecolor");
	
	ls_->SetParameters( prms );
}


};
