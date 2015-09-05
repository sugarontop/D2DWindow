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
#include "D2DWindowMessage.h"
#include "D2DWindowControl_easy.h"
#include "D2DCommon.h"
#include "D2DWindowTextbox.h"

using namespace V4;

LRESULT D2DStatic::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	if (!VISIBLE(stat_))
		return ret;

	switch (message)
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);

			mat.PushTransform(); 
			mat_ = mat; 
			
			FRectF rc1 = rc_.GetBorderRect();
			D2DRectFilter f(cxt, rc1);

			mat.Offset( rc1 );
			
			if ( isblack_ )
				cxt.cxt->DrawTextLayout(offpt_, textlayout_, cxt.black);
			else
			{
				auto fore = MakeBrsuh(cxt, brush_fore_);
				cxt.cxt->DrawTextLayout(offpt_, textlayout_, fore);
			}
		
			mat.PopTransform();
		}
		break;
	}
	return 0;

}
void D2DStatic::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, int alignment, LPCWSTR name, int id )
{
	D2DControl::CreateWindow(parent,pacontrol,rc,stat,name,id );

	
	brush_fore_ = ColorF(ColorF::Black);
	isblack_ = true;
	alignment_ = alignment;
}

_variant_t findmap(const std::map<std::wstring, VARIANT>& prms, LPCWSTR key, _variant_t& def)
{
	auto it = prms.find(key);
	if (prms.end() == it)
		return def;

	_variant_t r = (*it).second;
	return r;
}

void D2DStatic::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	_variant_t fn = findmap(prms, L"fontname", _variant_t(DEFAULTFONT));
	_variant_t h = findmap(prms, L"fontheight", _variant_t(DEFAULTFONT_HEIGHT)); h.ChangeType(VT_R4);
	_variant_t b = findmap(prms, L"fontbold", _variant_t(400)); b.ChangeType(VT_I4);
	_variant_t txt = findmap(prms, L"text", _variant_t(title_.c_str()));
	_variant_t color = findmap(prms, L"color", _variant_t((int)0)); color.ChangeType(VT_UI4);

	FRectF rcc = rc_.GetContentRect();

	ComPTR<IDWriteFactory2> wf = parent_->cxt_.cxtt.wfactory;
	ComPTR<IDWriteTextFormat> tf;
	auto hr = wf->CreateTextFormat( fn.bstrVal, 0, (DWRITE_FONT_WEIGHT)b.iVal, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, h.fltVal, L"", &tf);
	if ( HR(hr))
	{
		title_ = txt.bstrVal;

		ComPTR<IDWriteTextLayout> tl;
		if ( HR(wf->CreateTextLayout(title_.c_str(), title_.length(), tf, rcc.Width(), rcc.Height(), &tl )))
		{
			textlayout_ = tl;
					
			DWRITE_TEXT_METRICS tm;
			textlayout_->GetMetrics(&tm);
			
			if (alignment_ == 0)
				offpt_.x = 0;
			else if (alignment_ == 1)
				offpt_.x = (rcc.Width() -  tm.width)/2; 
			else if (alignment_ == 2)
				offpt_.x = rcc.Width() - tm.width;


			offpt_.y = (rc_.Height()- tm.height)/2; //センタ-配置

		}
	}

	brush_fore_ = D2DRGBA( color.ulVal );
	isblack_ = ( color.ulVal == 0 );
}
