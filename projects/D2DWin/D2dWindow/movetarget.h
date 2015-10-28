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

#include "D2DWindow.h"

namespace V4 {

/*
usage 
void D2DLooseLeaf::OnClickBtn( int btnid )
{
☆	MoveTarget* m = new MoveTarget();

	FRectF rc = parent_control_->GetContentRect();
	D2DControl* target = parent_control_->controls_[ 1 ].get();

☆	m->SetParameter(rc_.right,0,rc.Width(),0,500);

	target->Visible(true);
	target->SetRect( FRectF( rc_.right,0,1,rc_.Height()));

☆	m->Fire_ = [target](MoveTarget* p, float offa, float offb)->bool
	{
		auto rc = target->GetRect();
		rc.right = offa;
		target->SetRect(rc);

		return true;
	};


☆	m->End_ = [](MoveTarget* p, float offa, float offb)
	{
		delete p;
	};
	
☆	m->Start( parent_ );
}

*/

class MoveTarget
{
public:
	MoveTarget() 
	{
		QueryPerformanceFrequency(&__s_frequency_);
		brunning_ = false;
	};

	~MoveTarget(){ Clear(); }

	LARGE_INTEGER __s_frequency_;

	void Clear()
	{
		delete [] xtime_line_;
		delete [] ytime_line_;
		rto_ = 0;
		cnt_ = 0;
	}
	void SetParameter(float inix, float iniy, float endx, float endy, int milisec)
	{
		inix_ = inix;
		iniy_ = iniy;
		endx_ = endx;
		endy_ = endy;
		rto_ = 0;
		cnt_ = 0;
		xtime_line_ = nullptr;
		ytime_line_ = nullptr;

		_ASSERT(milisec);
		interval_msec_ = milisec;
		
	}

	static DWORD TimeGetTime(LARGE_INTEGER f)
	{
		// ::timeGetTime(); 相当, milli second.
		LARGE_INTEGER gtm;

		QueryPerformanceCounter(&gtm);
		return (DWORD) ((float) gtm.QuadPart*1000.0f / (float) f.QuadPart);
	}

	void CreateTimeline()
	{
		// 1000ms / 60fps -> 16.6ms
		int cnt = (int)((float)interval_msec_ / 16.6f) + 1;

		float* xtime_line = new float[cnt];
		float* ytime_line = new float[cnt];
		for( int i = 0; i < cnt-1; i++ )
		{
			float rto = (float)i / cnt;

			xtime_line[i] = inix_ + (endx_ - inix_)*rto;
			ytime_line[i] = iniy_ + (endy_ - iniy_)*rto;

		}

		xtime_line[cnt-1] = endx_;
		ytime_line[cnt-1] = endy_;

		xtime_line_= xtime_line;
		ytime_line_ = ytime_line;
		cnt_ = cnt;
	}


	void Start(D2DWindow* d)
	{
		d->mts_[this] = 0;

		brunning_ = true;

		CreateTimeline();

		st_time_ = TimeGetTime(__s_frequency_);
		bool isend;
		UIFire(d, isend);

	}
	virtual bool UIFire(D2DWindow* d, bool& isend)
	{
		int idx = min( cnt_-1, (int)((float) (TimeGetTime(__s_frequency_) - st_time_) / 16.6f));

		float x = xtime_line_[idx];
		float y = ytime_line_[idx];
		rto_ = (float)idx/cnt_;

		bool ret = Fire_(this, x, y);
		isend = false;
		if (IsEnd(x, y) && brunning_ )
		{
			End_(this, endx_, endy_);
			ret = true;
			isend = true;
		}

		d->redraw_ = 1;

		return ret; // redrawさせるにはtrueを
	}
	bool IsEnd(float x, float y)
	{
		if ((inix_ != endx_) && ((x != 0) && ((inix_ <= endx_ && endx_ <= x) || (inix_ > endx_ && endx_ >= x)))) return true;
		if ((iniy_ != endy_) && ((y != 0) && ((iniy_ <= endy_ && endy_ <= y) || (iniy_ > endy_ && endy_ >= y)))) return true;
		return false;
	}
	void Cancel()
	{
		brunning_ = false;
	}

	std::function<bool(MoveTarget* p, float offa, float offb)> Fire_;
	std::function<void(MoveTarget* p, float a, float b)> End_;

	float GetRto() { return rto_; }
private:
	float inix_, iniy_, endx_, endy_;
	int interval_msec_;
	DWORD st_time_;
	float rto_;
	float* xtime_line_;
	float* ytime_line_;
	int cnt_;
	bool brunning_;
	
};
};
