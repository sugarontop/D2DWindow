#pragma once

#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "ItemLoopArray.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "tsf\TextContainer.h"	// CTextContainer
#include "tsf\IBridgeTSFInterface.h"
#include "gdi32.h"
#include "ControlHandle.h"
//#include "msxmlex6.h"
#include "faststack.h"
#undef CreateWindow


#define ENABLE( stat ) (!(stat&STAT::DISABLE))
#define VISIBLE( stat ) (stat&STAT::VISIBLE)
#define DEAD( stat ) (stat&STAT::DEAD)
#define BORDER( stat ) (stat&STAT::BORDER)




namespace V4 {

// DISABLE:表示はされる、マウス等はうけつけない 
// DEATH:表示されない、すべてのメッセージは無視される
// DEAD:死体、参照カウンタの影響でメモリ上では生きでも、Controlとしては死体を意味する
enum STAT{ VISIBLE=0x1,MOUSEMOVE=0x2,CLICK=0x4,CAPTURED=0x8,SELECTED=0x10, DISABLE=0x20,READONLY=0x40, DROPACCEPTOR=0x80, DROPPING=0x100,BORDER=0x200, ALWAYSDRAW=0x400, DEAD=0x800, FOCUS=0x1000, DEBUG1=0x2000 };


class D2DControl;
class D2DControls;
class D2DWindow;

class D2DCaptureObject
{
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

namespace Inner {

	class ToolTip
	{
		public :
			ToolTip():isShow_(false){};

			bool IsShow(){ return isShow_; }
			void Set( D2DControl*p, LPCWSTR str );
			void Show(bool bShow){ isShow_ = bShow; }
			void Draw();
		private :
			FString str_;
			FRectF rc_;
			bool isShow_;
			D2DControl* ctrl_;
			GDI32::FPoint pt_;
	};

};

// Object mode
enum MOUSE_MODE { NONE,MOVE,RESIZE,COLUM_RESIZE,ROW_RESIZE,CHILD_CTRL };
//enum OBJ_STATUS { NONE=0,SELECT=0x1,CAPTURED=0x2 };

class D2DWindow
{
	public :
		D2DWindow();
		//virtual ~D2DWindow(){ Clear(); }

		HWND CreateD2DWindow( DWORD WSEX_STYLE, HWND parent, DWORD WS_STYLE, RECT rc, UINT* img_resource_id=nullptr, int img_cnt=0 );		
		LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

		
		

		void SetCapture(D2DCaptureObject* p, FPointF* pt=NULL, D2DMat* mat=NULL );
		D2DCaptureObject* ReleaseCapture();
		D2DCaptureObject* GetCapture(){ return ( !capture_obj_.empty() ? capture_obj_.top() : NULL); }
		FPointF CapturePoint( const FPointF& pt )
		{ 
			FPointF p = capture_pt_;
			capture_pt_ = pt;			
			return p; 
		}
		FRectFBoxModel CaptureRect( const FRectFBoxModel& rc )
		{ 
			FRectFBoxModel p = capture_rect_;
			capture_rect_ = rc;			
			return p; 
		}
		D2DMat CaptureMat(){ return capture_matrix_; }


		void RoundPaint(D2DCaptureObject* p ){ roundpaint_obj_ = p; } // for FloatMenu

		bool CaptureIsInclude(D2DCaptureObject* p){ return capture_obj_.include( p ); }
		static int SecurityId(bool bNew);
		
		void Clear();
		void ShowToolTip( D2DControl* p, LPCWSTR message );

		bool GetResourceImage( UINT id, ID2D1Bitmap** bmp );



		HWND hWnd_;
		HWND hMainFrame_;	
		D2DContext cxt_;
		D2DRES res_;
		int redraw_;
		D2DMat capture_matrix_;

		
		std::function<void(D2DWindow*)> OnCreate;
		std::function<void(D2DWindow*)> OnDestroy;
		std::map<void*,int> mts_;
		std::shared_ptr<D2DControls> children_;
		faststack<D2DCaptureObject*> capture_obj_;
		std::vector<D2DControl*> drag_accepters_;
		std::vector< std::shared_ptr<D2DControl>> death_objects_;
		

		ControlHandle chandle_;
		static std::wstring appinfo_;

		void ResourceCreate( bool bCreate );		
		ID2D1SolidColorBrush* GetSolidColor(D2D1_COLOR_F clr);
	protected :

		std::map<DWORD,ID2D1SolidColorBrush*> SolidColorBank_;
		std::map<DWORD, ID2D1Bitmap*> BitmapBank_;
		
	protected :
		FPointF capture_pt_;
		FRectFBoxModel capture_rect_;
		Inner::ToolTip tooltip_;

		D2DCaptureObject* roundpaint_obj_;
		
};

};