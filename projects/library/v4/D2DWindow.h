#pragma once

#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "faststack.h"
#include "D2DWindowMessage.h"
#include "gdi32.h"
#include "ControlHandle.h"
#include "tsf\TextContainer.h"	// CTextContainer
#include "tsf\IBridgeTSFInterface.h"

#undef CreateWindow

#define LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS 100
#define ENABLE( stat ) (!(stat&STAT::DISABLE))
#define VISIBLE( stat ) (stat&STAT::VISIBLE)
#define DEAD( stat ) (stat&STAT::DEAD)
#define BORDER( stat ) (stat&STAT::BORDER)

namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};


namespace V4 {

enum STAT{ VISIBLE=0x1,MOUSEMOVE=0x2,CLICK=0x4,CAPTURED=0x8,SELECTED=0x10, DISABLE=0x20,READONLY=0x40, DROPACCEPTOR=0x80, DROPPING=0x100,BORDER=0x200, ALWAYSDRAW=0x400, DEAD=0x800, FOCUS=0x1000, DEBUG1=0x2000 };


class D2DControl;
class D2DControls;
class D2DWindow;

class D2DCaptureObject
{
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		virtual void OnCaptureActive( bool bActive ){};
};
struct D2DRES
{
	int id;
};

class D2DWindow
{
	public :
		D2DWindow();
		virtual ~D2DWindow(){};

		HWND CreateD2DWindow( DWORD WSEX_STYLE, HWND parent, DWORD WS_STYLE, RECT rc );		
		LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

		

		void SetCapture(D2DCaptureObject* p, FPointF* pt=NULL );
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
		bool CaptureIsInclude(D2DCaptureObject* p){ return capture_obj_.include( p ); }
		static int SecurityId(bool bNew);
		
		void Clear();



		HWND hWnd_;
		HWND hMainFrame_;	
		D2DContext cxt_;
		D2DRES res_;
		int redraw_;
		
		std::function<void(D2DWindow*)> OnCreate;
		std::map<void*,int> mts_;
		std::shared_ptr<D2DControls> children_;
		faststack<D2DCaptureObject*> capture_obj_;
		std::vector<D2DControl*> drag_accepters_;
		std::vector< std::shared_ptr<D2DControl>> death_objects_;


		ControlHandle chandle_;
		static std::wstring appinfo_;

	protected :

		std::map<DWORD,CComPtr<ID2D1SolidColorBrush>> colorBank_;
		
	protected :
		FPointF capture_pt_;
		FRectFBoxModel capture_rect_;

		
};






};