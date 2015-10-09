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
#include "D2DWindowTitle.h"

using namespace V4;


D2DTopTitleBar::D2DTopTitleBar()
{
	
}
void D2DTopTitleBar::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id)
{
	D2DControl::InnerCreateWindow(parent, pacontrol, rc, stat, name, id);

	FSizeF sz = parent_control_->GetRect().Size();
	rc_.right = rc_.left + sz.width;

	fore_color_.color = ColorF(ColorF::Black);
	border_color_.color = ColorF(ColorF::Black);
	back_color_.color = ColorF(ColorF::LightGray);
	active_back_color_.color = ColorF(ColorF::Gray);

	OnResutructRnderTarget(true);

}
void D2DTopTitleBar::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);
		fore_color_.br = parent_->GetSolidColor(fore_color_.color);
		active_back_color_.br = parent_->GetSolidColor(active_back_color_.color);
	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
		fore_color_.br = nullptr;
		active_back_color_.br = nullptr;
	}

}
void D2DTopTitleBar::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");
	ParameterColor(parent_, fore_color_, prms, L"forecolor");
	ParameterColor(parent_, active_back_color_, prms, L"active_backcolor");
}

LRESULT D2DTopTitleBar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (!VISIBLE(stat_))
		return ret;

	switch ( message )
	{
		case WM_PAINT :
		{	
			auto& cxt = d->cxt_;

			D2DMatrix mat(cxt);

			mat_ = mat.PushTransform();
			mat.Offset( rc_ );

			FRectF rc(0,0,rc_.Width(), rc_.Height());

				//DrawFillRect( cxt, rc, (ID2D1Brush*)border_color_.br, (ID2D1Brush*)back_color_.br, 1 );
				DrawControlRect( cxt, rc, (ID2D1Brush*) border_color_.br, (ID2D1Brush*) back_color_.br);
				
				DrawCenterText( cxt, (ID2D1Brush*)fore_color_.br, rc, name_, name_.length(), 0, 5.0f );


			mat.PopTransform();

		}
		break;
		case WM_SIZE :
		{
			FSizeF sz = parent_control_->GetRect().Size();
			rc_.right = rc_.left + sz.width;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(FPointF(lParam));
			if (rc_.PtInRect(pt))
			{
				d->SetCapture(this, &pt, &mat_);


			}
		}
		break;
		case WM_MOUSEMOVE :
		{
			if (d->GetCapture() == this)
			{
				auto mat = parent_->CaptureMat();
				FPointF pt = mat.DPtoLP(lParam);

				FPointF ptprv = parent_->CapturePoint(pt);


				FRectFBM rc = parent_control_->GetRect();
				
				rc.Offset(pt.x - ptprv.x, pt.y - ptprv.y);
				
				parent_control_->SetRect( rc );


				parent_->redraw_ = 1;


				ret = 1;
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				ret = 1;
			}

		}
		break;



	}
	
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
D2DVerticalStackControls::D2DVerticalStackControls()
{

}
void D2DVerticalStackControls::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControls::CreateWindow(parent,pacontrol,rc,stat,name,id);

	back_color_.color = D2RGBA(0,0,0,0);
	border_color_.color = D2RGBA(0, 0, 0, 0);
	order_reverse_ = false;
}

FSizeF D2DVerticalStackControls::GetChildSize(D2DControl* child) const
{
	 //RequestUpdate で設定されるのでここにはこないはず
	return rc_.Size();
}



LRESULT D2DVerticalStackControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!VISIBLE(stat_))
		return 0;
	
	switch( message )
	{
		case WM_PAINT :
		{
			auto& cxt = d->cxt_;
			D2DMatrix mat(cxt);

			DrawControlRect(cxt, rc_, (ID2D1Brush*) border_color_.br, (ID2D1Brush*) back_color_.br);
						

			D2DRectFilter fil(cxt, rc_);

			
						
			mat.PushTransform();
			mat.Offset(rc_);
					
			if (order_reverse_)
			{
				mat.Offset(0,rc_.Height());
				for (auto& it : controls_)
				{
					mat.Offset(0, -it->GetRect().bottom ); //bottom is top+height.

					
					mat.PushTransform();
					it->WndProc(d, message, wParam, lParam);
					mat.PopTransform();
				}
			}
			else
			{
				for(auto& it : controls_ )
				{
				
					mat.PushTransform(); 
					it->WndProc(d,message, wParam, lParam);
					mat.PopTransform();

					mat.Offset( 0, it->GetRect().bottom ); 
				
				}
			}			
			mat.PopTransform();
			return 0;
		}
		break;
		case WM_SIZE :
		{
			auto p = dynamic_cast<IUpdatar*>( parent_control_ );
			if ( p )
			{
				p->RequestUpdate(this, WM_SIZE );

				SendMessageAll(d,message,wParam,lParam);		
				return 0;
			}
			else
			{
				auto sz = parent_control_->GetChildSize(this);
				rc_.SetSize(sz);
			}
		}
		break;
		case WM_MOUSEMOVE :
		{

			int a = 0;
		}
		break;
	}

	return D2DControls::WndProc(d,message,wParam, lParam);
}



void D2DVerticalStackControls::RequestUpdate(D2DControl* p, int typ)
{
	if (typ == WM_SIZE)
	{
		int j = 0;
		float h = rc_.Size().height;


		std::vector<std::shared_ptr<D2DControl>> ar;
		std::shared_ptr<D2DControl> last;
		for (auto& it : controls_)
		{
			if (it.get()->GetStat() & STAT::HEIGHT_FIX)
			{
				ar.push_back( it );
			}
			else
				last = it;
		}

		if ( last )
			ar.push_back(last);



		for (auto& it : ar ) //controls_)
		{
			FRectF rcc = it->GetRect();

			if (it.get() == p)
			{
				auto pt = rcc.LeftTop();

				bool fixw = ISFLAG_ON(p->GetStat(), STAT::WIDTH_FIX);
				bool fixh = ISFLAG_ON(p->GetStat(), STAT::HEIGHT_FIX);

				FSizeF sz;
				sz.width = (fixw ? rcc.Width() : rc_.Width());
				sz.height = (fixh ? (rcc.Height()) : h);

				p->SetRect(pt, sz);
		

				break;
			}
			else
			{
				h = max(0.0f, ( h- (rcc.top +rcc.Size().height) ));
			}
			j++;
		}
		
	}
}
void D2DVerticalStackControls::OnResutructRnderTarget(bool bCreate)
{
	if (bCreate)
	{
		back_color_.br = parent_->GetSolidColor(back_color_.color);
		border_color_.br = parent_->GetSolidColor(border_color_.color);

	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
	}

	for (auto& it : controls_)
		it->OnResutructRnderTarget(bCreate);

}
void D2DVerticalStackControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");

}
