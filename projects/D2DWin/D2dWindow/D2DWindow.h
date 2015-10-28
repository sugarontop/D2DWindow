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

#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "D2DWindowMessage.h"
#include "gdi32.h"
#include "faststack.h"

#undef CreateWindow

namespace V4 {

#define ENABLE( stat ) (!(stat&STAT::DISABLE))
#define VISIBLE( stat ) (stat&STAT::VISIBLE)
#define DEAD( stat ) (stat&STAT::DEAD)
#define BORDER( stat ) (stat&STAT::BORDER)


// DISABLE:表示はされる、マウス等はうけつけない 
// DEATH:表示されない、すべてのメッセージは無視される
// DEAD:死体、参照カウンタの影響でメモリ上では生きでも、Controlとしては死体を意味する

enum STAT { VISIBLE = 0x1, MOUSEMOVE = 0x2, CLICK = 0x4, CAPTURED = 0x8, SELECTED = 0x10, DISABLE = 0x20, READONLY = 0x40, DROPACCEPTOR = 0x80, WIDTH_FIX = 0x100, HEIGHT_FIX = 0x200, DROPPING = 0x400, BORDER = 0x800, ALWAYSDRAW = 0x1000, DEAD = 0x2000, FOCUS = 0x4000, DEBUG1 = 0x8000,MULTISELECT = 0x10000};



class D2DControl;
class D2DControls;
class D2DWindow;
class MoveTarget;

class D2DCaptureObject
{
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		virtual void OnCaptureActive( bool bActive ){};
};

namespace Inner {


};

// Object mode
enum MOUSE_MODE { NONE,MOVE,RESIZE,COLUM_RESIZE,ROW_RESIZE,CHILD_CTRL };

class D2DWindow
{
	public :
		D2DWindow();
		virtual ~D2DWindow(){ Clear();}

		HWND CreateD2DWindow( DWORD WSEX_STYLE, HWND parent, DWORD WS_STYLE, RECT rc );		
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

		bool CaptureIsInclude(D2DCaptureObject* p){ return capture_obj_.include( p ); }
		static int SecurityId(bool bNew);
		
		void Clear();
		void CallDispatcher( DispatherDelegate* pfunc, LPVOID lp );



		HWND hWnd_;
		HWND hMainFrame_;	
		D2DContext cxt_;
		
		int redraw_;
		D2DMat capture_matrix_;
		

		
		std::function<void(D2DWindow*)> OnCreate;
		std::function<void(D2DWindow*)> OnDestroy;
		std::map<MoveTarget*, int> mts_;
		std::shared_ptr<D2DControls> children_;
		faststack<D2DCaptureObject*> capture_obj_;
		std::vector<D2DControl*> drag_accepters_;


		struct DeathObject
		{
			std::shared_ptr<D2DControl> target;
			bool is_alive_in_parentControl;
		};

		std::vector< DeathObject> death_objects_;

		bool resource_test_;

	
		
		// resource function
		void ResourceCreate(bool bCreate);
		ID2D1SolidColorBrush* GetSolidColor(D2D1_COLOR_F clr);
	protected :
		
		std::map<DWORD, ID2D1SolidColorBrush*> SolidColorBank_;
		
	protected :
		FPointF capture_pt_;
		FRectFBoxModel capture_rect_;		
		D2DCaptureObject* roundpaint_obj_;
};

};
