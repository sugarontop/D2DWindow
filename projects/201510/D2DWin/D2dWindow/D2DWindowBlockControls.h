#pragma once

namespace V4{

class D2DControlsSomeblocks : public D2DControls, public IUpdatar
{
	public :
		D2DControlsSomeblocks();

		virtual void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);


		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

	protected :
		FRectF Calc( int row, int col );
		FRectF CalcSplitBar(int idx);
		LRESULT MouseResize(D2DWindow* d, UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void RequestUpdate(D2DControl* p, int typ);
	
	protected :
		float wall_x_;
		float wall_y1_, wall_y2_;
		FRectF rcc_[4];
		int splitbar_move_;
		
};

 

};