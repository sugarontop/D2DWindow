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
// for D2DWindow reserved range(from WM_APP to WM_APP+399)

#define WM_D2D	WM_APP

#define WM_UI_FIRE				(WM_D2D+1)
#define WM_D2D_NCLBUTTONDOWN	(WM_D2D+2)

#define WM_D2D_UNACTIVE			(WM_D2D+4)

#define WM_D2D_BUTTON_CLICK						(WM_D2D+8)
#define WM_D2D_TEXTBOX_SETTEXT					(WM_D2D+9)
#define WM_D2D_TEXTBOX_GETTEXT					(WM_D2D+10)

#define WM_D2D_MENU_SELECT						(WM_D2D+11)
#define WM_D2D_PAINT							(WM_D2D+12) // WM_D2D_PAINT_CAPTURE
#define WM_D2D_GET_CONTROLS						(WM_D2D+13)
#define WM_D2D_CREATE_CONTROLS					(WM_D2D+14)

#define WM_D2D_TEST								(WM_D2D+15)

#define WM_D2D_IDLE								(WM_D2D+16)
#define WM_D2D_DROPOBJECT_SEARCH_ACCEPTOR		(WM_D2D+17)
#define WM_D2D_DROPOBJECT_FOUND_ACCEPTOR		(WM_D2D+18)
#define WM_D2D_OBJECT_UPDATE					(WM_D2D+19)
#define WM_D2D_OBJECT_SUICIDE					(WM_D2D+20)

#define WM_D2D_MESSAGEBOX_CLOSED				(WM_D2D+21)
#define WM_D2D_INTERNET_GET_COMPLETED			(WM_D2D+22)

#define WM_D2D_TAB_ACTIVE						(WM_D2D+23)
#define WM_D2D_COMMAND							(WM_D2D+24) // WM_COMMAND wparam, lparam. Lparam is security id.メニューと連動

#define WM_D2D_SETCAPTURE						(WM_D2D+25)

#define WM_D2D_GRIDCONTROL						(WM_D2D+26) // wparam:0 is add msg.

//#define WM_D2D_TEXTBOX_FLOAT_MENU				(WM_D2D+27)
#define WM_D2D_BROADCAST_UPDATE					(WM_D2D+28) // 一般的な状態の変更イベント、なるべくすべてに行きわたらせる
#define WM_D2D_RESTRUCT_RENDERTARGET			(WM_D2D+29) // wp:0 release, wp:1 relink

#define WM_D2D_CLOSE_CONTROL					(WM_D2D+30)	// wp:none, lp:none
//#define WM_D2D_DESTROY_CONTROL					(WM_D2D+31)	// wp:none, lp:D2DControl* target
#define WM_D2D_DRAGDROP							(WM_D2D+32)
#define WM_D2D_DISPATCHER_DELEGATE				(WM_D2D+33)

#define WM_D2D_MAINFRAME_GET_FLOAT_MENU			(WM_D2D+34) // float menu 回収


#define WM_D2D_EVENT_FIRST						(WM_D2D+100)

#define WM_D2D_EVTEXTBOX_SINGLELINE_PUSHKEY		(WM_D2D+101)
#define WM_D2D_EVTEXTBOX_CHAR					(WM_D2D+102)
#define WM_D2D_EVSELECT_CHEANGED				(WM_D2D+103)
#define WM_D2D_EVSLIDER_CHEANGED				(WM_D2D+104)


#define WM_D2D_EVENT_LAST						(WM_D2D+110)




//#define WM_D2D_APP_COMMOND						(WM_D2D+200)	// APPLICATION単位の汎用コマンド
//#define WM_D2D_DLL_COMMOND						(WM_D2D+201)	// DLL OBJECTへの汎用コマンド
//#define WM_D2D_INET_COMPLETE					(WM_D2D+202)



#define WM_APP_SHOW_MSGBOX						(WM_D2D+300)

#define WM_DD_COMMAND							(WM_D2D+301)
#define WM_DD_SELECT_CHANGED					(WM_D2D+302)


#define WM_D2D_RESERVED_LAST					(WM_D2D+399)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WM_MOUSEMOVE : wParam
#define MK_F_MOUSE_ENTER						0x100
#define MK_F_MOUSE_LEAVE						0x200


// LRESULT return value
#define LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS	0x8000


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for user application (from WM_APP+400 to WM_APP+16383)
#define WM_D2D_APP_USER_FIRST					(WM_D2D+400)
#define WM_D2D_APP_USER							WM_D2D_APP_USER_FIRST


struct FloatMenuItem
{
	MSG msg;	// event
	short imgid;			// 0: noimage, -1:header
	std::wstring text;
	std::wstring keyboard;
	bool enable;
};
