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

	if (dynamic_cast<IUpdatar*>(pacontrol) && rc.IsZero() )
	{
		dynamic_cast<IUpdatar*>(pacontrol)->RequestUpdate(this, WM_SIZE);

	}

	back_color_.color = D2RGBA(0,0,0,0);
	border_color_.color = D2RGBA(0, 0, 0, 0);
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
					

			for(auto& it : controls_ )
			{
				
				mat.PushTransform(); 
				it->WndProc(d,message, wParam, lParam);
				mat.PopTransform();

				mat.Offset( 0, it->GetRect().Size().height );
				
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
			}
			else
			{
				auto sz = parent_control_->GetRect().GetContentRect().Size();

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

		for (auto& it : controls_)
		{
			FRectF rcc = it->GetRect();
			if (it.get() == p)
			{
				auto pt = rcc.LeftTop();
				auto nsz = FSizeF( rc_.Width(), h );

				p->SetRect(pt, nsz);


				break;
			}
			else
			{
				h -= rcc.Size().height;


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
		//fore_color_.br = parent_->GetSolidColor(fore_color_.color);
		//active_back_color_.br = parent_->GetSolidColor(active_back_color_.color);
	}
	else
	{
		back_color_.br = nullptr;
		border_color_.br = nullptr;
		//fore_color_.br = nullptr;
		//active_back_color_.br = nullptr;
	}

}
void D2DVerticalStackControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	ParameterColor(parent_, back_color_, prms, L"backcolor");
	ParameterColor(parent_, border_color_, prms, L"bordercolor");
	//ParameterColor(parent_, fore_color_, prms, L"forecolor");
	//ParameterColor(parent_, active_back_color_, prms, L"active_backcolor");
}
