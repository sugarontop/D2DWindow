/*
The MIT License (MIT)
Copyright (c) 2015 sugarontop@icloud.com
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

#import <msxml6.dll>
#include "D2DWindowMessage.h"
//////////////////////////////////////////////////////////////////////////
// menu.xmlで作成されるメインフレームメニュー　WM_D2D_COMMANDを送る
//////////////////////////////////////////////////////////////////////////

namespace V4 {

class D2DMenuItems : public D2DControls
{
	public :
		D2DMenuItems(){}
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, bool bVertical, int id=-1  );
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		D2DControl* GetActiveControl();
	private :
		bool bVertical_;
		int active_;
};
class D2DMainFrameMenu : public D2DControls
{
	public :
		D2DMainFrameMenu(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );
		void LoadMenu();
		struct MenuItem
		{
			_bstr_t title;
			float width;
		};

		void CreateSubMenu();
		void CreateSubMenuItem(MSXML2::IXMLDOMNode* nd);
	private :
		std::vector<MenuItem> ar_;
		int active_;

		D2DMenuItems* submenu_;
		MSXML2::IXMLDOMDocument2Ptr menuxml_;
};

class D2DMenuItemEx : public D2DControls
{
	public :
		D2DMenuItemEx(){}
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );

		void CreateSubMenu();
		MSXML2::IXMLDOMNodePtr nd_;
		bool bBottomLine_;
		D2DControls* submenu_;
	private :
		FString strL, strR;
		//Image xxxx
		int md_;
};




namespace D1 {



class D2DMenuItems : D2DControls
{
	public :
		D2DMenuItems(){};
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FPointF pt, const std::vector<FloatMenuItem>& items ); 
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
	private :
		
};



class D2DMenuItem : D2DControl
{
	public :
		D2DMenuItem(FloatMenuItem& info):info_(info){};
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol );
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		void DrawMenuItem(D2DContext& cxt);
	private :
		FloatMenuItem info_;
		int md_;
};


}




}; // V4



