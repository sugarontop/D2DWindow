#pragma once 

#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "D2DWindow.h"
//#include "ItemLoopArray.h"
#include "D2DWindowMessage.h"
//#include "D2DWindowMessageStruct.h"
//#include "tsf\TextContainer.h"	// CTextContainer
//#include "tsf\IBridgeTSFInterface.h"
#include "gdi32.h"
//#include "ControlHandle.h"
//#include "msxmlex6.h"
#include "faststack.h"

#undef CreateWindow

typedef std::wstring FString;

namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};

namespace V4 {


class D2DControl;
class D2DControls;
class D2DWindow;
class D2DScrollbar;
class D2DControls;
class D2DControl : public D2DCaptureObject
{
	public :
		D2DControl();
		virtual ~D2DControl(){};

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id=-1 );
		
		int Stat( int new_stat);

		int GetStat()  const{ return stat_; }
		void SetStat(int s){ stat_ = s; }

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		virtual void UpdateScrollbar(D2DScrollbar* bar){};

		virtual void SetRect( const FRectF& rc ){ rc_ = rc; }	
		virtual void SetRect( const FPointF& pt, const FSizeF& sz ){ rc_.SetRect(pt,sz); }
		virtual void UpdateStatus( D2DControls* p){}
		const FRectFBoxModel& GetRect() const { return rc_; }
		virtual D2DControl* UnPack(){ return this; }
		virtual void StatActive(bool bActive);

		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;
		virtual bool IsAutoResize(){ return auto_resize_; }
		

		FRectF GetDPRect() const{ return dp_rc_; } // Deviceïœä∑Ç≥ÇÍÇΩrc_
		
		virtual void BackColorTest(D2D1_COLOR_F clr){}
		virtual void BackColor( ID2D1Brush* clr ){}
		virtual void ForeColor( ID2D1Brush* clr){}
		virtual void OnHostage( D2DControls* p, bool block ){}
		
		
		//CHDL GetCHDL() const { return chdl_; } // Get Control Handle
		D2DMat Matrix() const { return mat_; }
		FString GetName() const { return name_; }
		
		int GetID() const { return id_; }
		
		void SetName(LPCWSTR nm) { name_ = nm; }
		void DestroyWindow();


		D2DWindow* parent_;				
		D2DControls* parent_control_;
		void* target_;
		bool auto_resize_;
	protected :
		LRESULT SendMessage( UINT msg, WPARAM w,LPARAM l ){ return ::SendMessage( parent_->hWnd_, msg, w, l ); }
		void InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);
		virtual void OnCreate(){}
	protected :
		D2DMat mat_; 
		FRectF dp_rc_; // deviceç¿ïW
		int stat_;
		FRectFBoxModel rc_;
		CHDL chdl_;
		FString name_;
		int id_;
};




class D2DControls : public D2DControl
{
	public :
		D2DControls():mouse_enter_(false){ scrollbar_off_ = FSizeF(0,0);}

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		virtual void MeToLast();
		virtual FRectF GetContentRect(){ return rc_.GetContentRect(); }
		void Clear();
		virtual bool IsAutoResize(){ return auto_resize_; }
	
		virtual LRESULT KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam);
		
		void Lineup(bool vertical=true );

		std::shared_ptr<D2DControl> Detach( D2DControl* );
		void Attach( std::shared_ptr<D2DControl> ctrl );

		std::vector<std::shared_ptr<D2DControl>> controls_;
		bool mouse_enter_;
		FSizeF scrollbar_off_;
		bool bwindow_size_;
		std::pair<int,int> vector_idx;

		LRESULT SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		
};

/////////////////////////////////////////////////////////////
//
// ControlsÇÃêeã ÅAD2DWindowÇÃëÊàÍControls
//
/////////////////////////////////////////////////////////////
class D2DTopControls : public D2DControls
{
	public :
		D2DTopControls(){}		
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow(D2DWindow* parent, D2DControls* must_be_null, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );

		std::function<FRectF(FRectF, GDI32::FSize)> calc_auto_size_;
		
	protected :
		void CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id=-1 );

		FRectF CalcAutoSize( const GDI32::FSize& sz );

		FString error_msg_;
		float zoom_;
		bool bAutoSize_;
		CComPtr<ID2D1SolidColorBrush> background_; // default is white		
};

class D2DButton : public D2DControl
{
	public :
		D2DButton(){};

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
	
	protected :
		static void DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s );
		void OnDraw( D2DContext& cxt );
		std::function<void (D2DContext&, const FRectFBoxModel&, int, const FString& s)> ondraw_;

	public :
		std::function<void (D2DButton*)> OnClick_;
		
};


class D2DFrameWindowControl : public D2DControls
{
	public :
		D2DFrameWindowControl(){}
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
	protected :
		
};



void DrawRect( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, ID2D1Brush* br, float width );

void DrawButton( ID2D1RenderTarget* cxt, const D2D1_RECT_F& rc, FString str, ID2D1Brush* br1 );



};