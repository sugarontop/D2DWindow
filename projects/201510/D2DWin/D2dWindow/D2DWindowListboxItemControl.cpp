#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl_easy.h"
#include "ItemLoopArray.h"
#include "D2DWindowListbox.h"
#include "D2DContextEx.h"
#include "gdi32.h"
#include "D2DWindowListboxItemControl.h"

namespace V4 {


LRESULT D2DControlsTest::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
		{
			D2DMatrix mat(d->cxt_);
			mat_ = mat.PushTransform();
			mat.Offset(rc_);

			//DrawDebugCross(d->cxt_, d->cxt_.black);

			if (this == d->GetCapture())
			{
				DrawRect(d->cxt_, rc_.ZeroRect(), d->cxt_.red, 1);

			}

			for (auto& it : controls_)
			{
				it->WndProc(d, message, wParam, lParam);
			}

			mat.PopTransform();
		}
		break;
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP(lParam);
			if (rc_.PtInRect(pt))
			{
				for (auto& it : controls_)
				{
					it->WndProc(d, message, wParam, lParam);
				}


			}
			else if (this == d->GetCapture())
			{
				d->ReleaseCapture();
			}
		}
		break;
		default:
			return D2DControls::WndProc(d, message, wParam, lParam);

	}


	return 0;
} 

};
