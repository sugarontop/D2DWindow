#include "stdafx.h"
#include "D2DWindowControl_Easy.h"
#include "D2DWindowBlockControls.h"

using namespace V4;


#define XAS(x) _ASSERT(x)

constexpr float split_border_width_ = 7;
/*
	結局４段ブロックによるControlsの組分け
        x	
	+---+------------+
	|   |            |
	|   |            |
y1	+---+            |
	|   | ___________+ 
	|   |            | y2
	+---+------------+


*/

D2DControlsSomeblocks::D2DControlsSomeblocks()
{

}
void D2DControlsSomeblocks::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& _rc, int stat, LPCWSTR name, int id)
{
	FRectFBM rc = _rc;
	FSizeF sz = rc.Size();

	auto_resize_ = false;
	
	if (sz.width == 0 || sz.height == 0 )
	{
		sz = pacontrol->GetRect().Size();

		rc.SetSize(sz);
		auto_resize_ = true;
	}
		
	D2DControls::InnerCreateWindow(parent, pacontrol, rc, stat, name, id);

	//if (dynamic_cast<IUpdatar*>(pacontrol) && rc.IsZero() == 0)
	//{
	//	dynamic_cast<IUpdatar*>(pacontrol)->RequestUpdate(this, WM_SIZE);

	//}
	wall_x_ = rc.Width() / 4;
	wall_y1_ = rc.Height()/2;
	wall_y2_ = rc.Height()/4*3;	


	rcc_[0] = Calc(0, 0);
	rcc_[1] = Calc(0, 1);
	rcc_[2] = Calc(1, 0);
	rcc_[3] = Calc(1, 1);

	
}
FRectF D2DControlsSomeblocks::Calc(int row, int col)
{
	XAS( row < 2 && col < 2 );

	FRectF rc;

	if (col == 0 && row == 0 ) // idx:0
	{
		rc.left = rc.top = 0;
		rc.right = wall_x_;
		rc.bottom = wall_y1_;
	}
	else if (col == 1 && row == 0 ) // idx:1
	{	
		rc.top = 0;
		rc.left = wall_x_ + split_border_width_;
		rc.right = rc_.Size().width;// - rc.left;
		rc.bottom = wall_y2_;
	}
	if (col == 0 && row == 1) // idx:2
	{
		rc.left = 0;
		rc.top = wall_y1_+ split_border_width_;
		rc.right = wall_x_;
		rc.bottom = rc_.Size().height;// - rc.top;
	}
	else if (col == 1 && row == 1) // idx:3
	{
		rc.left = wall_x_ + split_border_width_;
		rc.top = wall_y2_ + split_border_width_;
		rc.right = rc_.Size().width;// - rc.left;
		rc.bottom = rc_.Size().height;// - rc.top;
	}
	return rc;
}
FRectF D2DControlsSomeblocks::CalcSplitBar(int idx)
{
	FRectF rc;
	if ( idx == 0 )
	{
		rc.SetRect(rcc_[0].left, rcc_[0].bottom, rcc_[2].right, rcc_[2].top);
	}
	else if ( idx == 1 )
	{
		rc.SetRect(rcc_[1].left, rcc_[1].bottom, rcc_[3].right, rcc_[3].top);
	}
	else if ( idx == 2 )
	{
		rc.SetRect(rcc_[0].right, rcc_[0].top, rcc_[1].left, rcc_[3].bottom);
	}
	return rc;

}
LRESULT D2DControlsSomeblocks::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch( message )
	{
		case WM_PAINT :
		{			
			auto& cxt = d->cxt_;
			D2DMatrix mat(cxt);

			mat_ = mat.PushTransform();
			mat.Offset(rc_);

			int j = 0;
			for( auto& it : controls_ )
			{
				mat.PushTransform();				
				
				mat.Offset(rcc_[j++] );
				//FillRectangle(cxt, FRectF(-20,-20,20,20),cxt.bluegray);
				
				it->WndProc(d,message,wParam,lParam);
				
				mat.PopTransform();
			}

			
			for( int i = 0; i < 2; i++ )
				FillRectangle( cxt, CalcSplitBar(i), cxt.gray);
			
			
			DrawRect(cxt, rc_.ZeroRect(), cxt.ltgray,1);
			mat.PopTransform();
			return 0;
		}
		break;
		case WM_SIZE :
		{
			if (auto_resize_ )
			{
				auto sz = parent_control_->GetRect().Size();
				rc_.SetSize(sz);

				rcc_[0] = Calc(0, 0);
				rcc_[1] = Calc(0, 1);
				rcc_[2] = Calc(1, 0);
				rcc_[3] = Calc(1, 1);
			}
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			ret = MouseResize(d,message, wParam, lParam);

		break;

	}



	if ( ret == 0 )
		ret = D2DControls::WndProc(d, message, wParam, lParam);

	return ret;
}
LRESULT D2DControlsSomeblocks::MouseResize(D2DWindow* d,UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	switch( msg )
	{
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lParam);
			splitbar_move_ = -1;

			for (int i = 0; i < 3; i++)
			{
				FRectF rc = CalcSplitBar(i);

				if ( rc.PtInRect(pt))
				{					
					splitbar_move_ = i;

					d->SetCapture(this, &pt, &mat_);
					ret = 1;
					break;
				}
			}

		}
		break;
		case WM_MOUSEMOVE:
		{
			int typ = 0;
			if (d->GetCapture() != this)
			{
				FPointF pt = mat_.DPtoLP(lParam);
				
				for (int i = 0; i < 3; i++)
				{
					FRectF rc = CalcSplitBar(i);

					if (rc.PtInRect(pt))
					{
						typ = i+1;
					}
				}
			
			}
			else 
			{				
				auto mat = parent_->CaptureMat();
				FPointF pt = mat.DPtoLP(lParam);
				FPointF ptprv = parent_->CapturePoint(pt);

				float offx = pt.x - ptprv.x; 
				float offy = pt.y - ptprv.y;

				if (splitbar_move_ == 0 )
				{
					rcc_[0].bottom += offy;
					rcc_[2].top += offy;

					ret = 1;
					typ = 1;
				}
				else if (splitbar_move_ == 1)
				{
					rcc_[1].bottom += offy;
					rcc_[3].top += offy;

					ret = 1;
					typ = 2;
				}
				else if (splitbar_move_ == 2)
				{
					rcc_[0].right += offx;
					rcc_[2].right += offx;
					rcc_[1].left += offx;
					rcc_[3].left += offx;

					
					ret = 1;
					typ = 3;
				}
			}	

			
			if (typ == 1 || typ == 2)
				SetSystemCursor((UINT) IDC_SIZENS);				
			else if (typ == 3)
				SetSystemCursor((UINT) IDC_SIZEWE);
			
			if (d->GetCapture() == this && typ > 0 )
			{
				d->redraw_ = 1;

				
				SendMessageAll(d, WM_SIZE, 0, 0);
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				d->ReleaseCapture();
				ret = 1;


				if (splitbar_move_ > -1)
				{

					D2DControls::WndProc(d, WM_SIZE, 0, 0);
					
					splitbar_move_ = -1;

				}
			}
		}
		break;
	}

	return ret;
}
void D2DControlsSomeblocks::RequestUpdate(D2DControl* p, int typ)
{
	if (typ == WM_SIZE )
	{
		int j = 0;
		for( auto& it : controls_ )
		{
			if ( it.get() == p )
			{
				
				auto nsz = rcc_[j].Size();

				p->SetRect(FPointF(0,0),nsz);


				break;
			}
			j++;
		}
	}
}
void D2DControlsSomeblocks::OnResutructRnderTarget(bool bCreate)
{
	for (auto& it : controls_)
	{
		it->OnResutructRnderTarget(bCreate);
	}
}
void D2DControlsSomeblocks::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{

}
