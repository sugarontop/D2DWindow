
#pragma once
namespace V4 {

// Controlsの上部のタイトル
class D2DTopTitleBar : public D2DControl
{
	public :
		D2DTopTitleBar();
		virtual void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		

		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

	protected :
		SolidColor back_color_;
		SolidColor border_color_;
		SolidColor fore_color_;
		SolidColor active_back_color_;
};



class D2DVerticalStackControls : public D2DControls, public IUpdatar
{
	public :
		D2DVerticalStackControls();
		virtual void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id = -1);
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void RequestUpdate(D2DControl* p, int typ);

		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
};


};
