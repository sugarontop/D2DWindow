#pragma once

#include "D2DWindow.h"

namespace V4 {

class MoveTargetEx 
{
public :
	MoveTargetEx(){};	
	void SetParameter( float inix, float iniy, float endx, float endy, int milisec )
	{
		inix_ = inix;
		iniy_ = iniy;
		endx_ = endx;
		endy_ = endy;

		_ASSERT(milisec);
		interval_msec_ = milisec;
		
	}
	DWORD TimeGetTime()
	{
		// ::timeGetTime(); ‘Š“–
		LARGE_INTEGER gtm;
		LARGE_INTEGER __s_frequency_;

		QueryPerformanceCounter(&gtm);
		QueryPerformanceFrequency( &__s_frequency_ );

		return (DWORD)( (float)gtm.QuadPart*1000.0f / (float)__s_frequency_.QuadPart);
	}
	void Start(D2DWindow* d)
	{
		d->mts_[this]=0;
		
		st_time_ = TimeGetTime();
		
		bool isend;
		UIFire( d, isend );

	}
	virtual bool UIFire( D2DWindow* d, bool& isend )
	{
		float rto = (float)(TimeGetTime() - st_time_)/interval_msec_;
		
		float x = inix_ + (endx_-inix_)*rto;
		float y = iniy_ + (endy_-iniy_)*rto;
		
		bool ret = Fire_( this, x, y );
		isend = false;
		if ( IsEnd(x,y))
		{			
			End_(this,endx_,endy_);
			ret = true;
			isend = true;		
		}
		
		d->redraw_ = 1;

		return ret; // redraw‚³‚¹‚é‚É‚Ítrue‚ð
	}
	bool IsEnd(float x, float y)
	{
		if ((inix_ !=endx_) && ( ( x!=0 ) &&(( inix_ <= endx_ && endx_ <= x )||( inix_ > endx_ && endx_ >= x )))) return true;
		if ((iniy_ !=endy_) && ( ( y!=0 )&& (( iniy_ <= endy_ && endy_ <= y )||( iniy_ > endy_ && endy_ >= y )))) return true;
		return false;
	}
	
	std::function<bool(MoveTargetEx* p, float offa, float offb)> Fire_;
	std::function<void(MoveTargetEx* p,float a, float b)> End_;

	private :
		float inix_,iniy_,endx_,endy_;
		int interval_msec_;
		DWORD st_time_;
};
/*
usage 
void D2DLooseLeaf::OnClickBtn( int btnid )
{
™	MoveTargetEx* m = new MoveTargetEx();

	FRectF rc = parent_control_->GetContentRect();
	D2DControl* target = parent_control_->controls_[ 1 ].get();

™	m->SetParameter(rc_.right,0,rc.Width(),0,500);

	target->Visible(true);
	target->SetRect( FRectF( rc_.right,0,1,rc_.Height()));

™	m->Fire_ = [target](MoveTargetEx* p, float offa, float offb)->bool
	{
		auto rc = target->GetRect();
		rc.right = offa;
		target->SetRect(rc);

		return true;
	};


™	m->End_ = [](MoveTargetEx* p, float offa, float offb)
	{
		delete p;
	};
	
™	m->Start( parent_ );
}

*/
};