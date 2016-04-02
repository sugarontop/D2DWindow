#pragma once 


namespace V4 {
//class D2DPropertyControl : public D2DControls
//{
//	public :
//		D2DPropertyControl(){};
//		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
//		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
//
//		virtual void BackColor( ID2D1Brush* br ){ brush_back_ = br; }
//		virtual void BorderColor( ID2D1Brush* br){ brush_border_ = br; }
//
//		
//
//		struct Item
//		{
//			FString name;
//			D2DControl* ctrl;
//			
//		};
//
//		std::vector<Item> ar_;
//
//
//	private :
//		CComPtr<ID2D1Brush> brush_border_;
//		CComPtr<ID2D1Brush> brush_back_;
//		
//};

class D2DScrollbar;
class D2DControlsWithHScrollbar : public D2DControls
{
	public :
		D2DControlsWithHScrollbar();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		
		
		virtual void UpdateScrollbar(D2DScrollbar* bar);
	private :
		D2DScrollbar* scbar_;
		FSizeF scrollbar_off_;
};




};