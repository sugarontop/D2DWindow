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
