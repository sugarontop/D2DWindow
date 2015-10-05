#pragma once

namespace V4 {

class D2DButtonGruop : public D2DControl //, public D2DCaptureWindow
{
	public :
		D2DButtonGruop(){};
		virtual void MeToLast(){};
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id, int btncount );
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		std::function<FPointF(FSizeF)> GetPlace_;
		int btncnt_;
	private :
		void DrawButton( D2DContext& cxt );
		int btnmode_;


};
class D2DTitleBar : public D2DControls //, public D2DCaptureWindow
{
	public :
		D2DTitleBar(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void MeToLast(){};
		//virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id );
	private :
		virtual void OnCreate();
		FPointF GetPlaceRightButton(FSizeF sz );
		D2DButtonGruop* btngrp_;

};


class D2DControlsEx : public D2DControls //, public D2DCaptureWindow
{
	public :
		D2DControlsEx():bExpand_(true),md_(0){};
		virtual ~D2DControlsEx();


		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id );
		virtual void SetRect( const FRectF& rc );
		virtual void MeToLast();
		void Expand( bool bExpand );
		void Dialog( bool bDialog );
		void TopDialog();

		bool bExpand_;
		bool bDialog_;
		D2DMat mat2_;
		FRectF rc2_;
		int md_;
	protected :
		LRESULT WndProc0(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT WndProc1(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);


		int wndstat_;
		float titlebar_height_;
		D2DTitleBar* tbar_;
};

};


