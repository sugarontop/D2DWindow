#pragma once

namespace V4 {

/////////////////////////////////////////////////////////////
//
// Tab•t‚«Controls page1,page2,page3,,
//
/////////////////////////////////////////////////////////////

class D2DTabControls : public D2DControls
{
	public :
		D2DTabControls();
		virtual ~D2DTabControls();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );

		FRectF OnTagDraw( D2DContext& cxt );

		virtual FRectF GetContentRect();
		D2DControls* Child( UINT idx );
		UINT AddChild( LPCWSTR name );
		UINT CountChild(){ return controls_.size(); }
		
		void TagEnable( UINT idx, bool bEnable );
		
		void AutoScale( bool bWidth, bool bHeight );
		void SetActivePage( int idx );
	
	protected :
		UINT HitTagPoint( FPointF pt );

	private :
		UINT active_idx_;
		UINT scale_;

		enum AUTO{ WIDTH=0x1, HEIGHT=0x2 };

};

};
