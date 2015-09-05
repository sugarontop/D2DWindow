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

#include "D2DMisc2.h"
#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "D2DWindow.h"
#include "ItemLoopArray.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "tsf\TextContainer.h"	// CTextContainer
#include "tsf\IBridgeTSFInterface.h"
#include "gdi32.h"
#include "ControlHandle.h"
#include "msxmlex6.h"
#include "faststack.h"


#undef CreateWindow

#define COLOR_MOUSE_MOVE D2RGBA(153,217,234,255)
#define COLOR_SELECTED D2RGBA(132,137,227,100)


namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};

namespace V4 {

typedef std::function<LRESULT(D2DControl*,UINT,WPARAM,LPARAM)> OnWndProcExtDelegate;

class D2DControl;
class D2DControls;
class D2DWindow;
class D2DScrollbar;
class D2DControls;
class D2DControl : public D2DCaptureObject
{
	public :
		D2DControl();
		virtual ~D2DControl();

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id=-1 );
		
		int Stat( int new_stat);

		int GetStat()  const{ return stat_; }
		void SetStat(int s){ stat_ = s; }

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		virtual void UpdateScrollbar(D2DScrollbar* bar){};

		virtual void SetRect( const FRectF& rc ){ rc_ = rc; }	
		virtual void SetRect( const FPointF& pt, const FSizeF& sz ){ rc_.SetRect(pt,sz); }
		virtual void UpdateStatus( D2DControls* p){}
		virtual D2DControl* UnPack(){ return this; }
		virtual void StatActive(bool bActive);
		virtual bool IsAutoResize(){ return auto_resize_; }


		const FRectFBoxModel& GetRect() const { return rc_; }

		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;
		
		FRectF GetDPRect() const{ return dp_rc_; } // Device変換されたrc_
		
		virtual void BackColor(D2D1_COLOR_F clr){}
		//virtual void BackColor( ID2D1Brush* clr ){}
		//virtual void ForeColor( ID2D1Brush* clr){}
		virtual void OnHostage( D2DControls* p, bool block ){}
		
		
		CHDL GetCHDL() const { return chdl_; } // Get Control Handle
		D2DMat Matrix() const { return mat_; }
		FString GetName() const { return name_; }
		
		int GetID() const { return id_; }
		
		void SetName(LPCWSTR nm) { name_ = nm; }
		virtual void DestroyControl();


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
		FRectF dp_rc_; // device座標
		int stat_;
		FRectFBoxModel rc_;
		CHDL chdl_;
		FString name_;
		int id_;
};




class D2DControls : public D2DControl
{
	public :
		D2DControls():mouse_enter_(false){ scrollbar_off_ = FSizeF(0,0);clr_ = D2RGB(255,255,255);}

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual LRESULT KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		virtual void MeToLast();
		virtual FRectF GetContentRect(){ return rc_.GetContentRect(); }
		virtual bool IsAutoResize(){ return auto_resize_; }
		virtual void DestroyControl();
		void Clear();
		virtual void BackColor(D2D1_COLOR_F clr){ clr_ = clr; }
		
		void Lineup(bool vertical=true ); // childを整列させる

		std::shared_ptr<D2DControl> Detach( D2DControl* );
		void Attach( std::shared_ptr<D2DControl> ctrl );

		std::vector<std::shared_ptr<D2DControl>> controls_;
		bool mouse_enter_;
		FSizeF scrollbar_off_;
		bool bwindow_size_;
	
		std::pair<int,int> vector_idx;
		D2D1_COLOR_F clr_;

		
	
	public :
		LRESULT SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);		
};


/////////////////////////////////////////////////////////////
//
// Controlsの親玉、D2DWindowの第一Controls
//
/////////////////////////////////////////////////////////////
class D2DTopControls : public D2DControls
{
	public :
		D2DTopControls(){}
		virtual ~D2DTopControls();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow(D2DWindow* parent, D2DControls* must_be_null, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );
		void CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id=-1 );
		
		virtual void OnCreate();
		virtual void BackColor(D2D1_COLOR_F clr);

		std::function<FRectF(FRectF, GDI32::FSize)> calc_auto_size_;
		

		OnWndProcExtDelegate OnWndProcExt_;
	protected :
		FRectF CalcAutoSize( const GDI32::FSize& sz );

		FString error_msg_;
		float zoom_;
		bool bAutoSize_;
		D2D1_COLOR_F backcolor_;
};

/////////////////////////////////////////////////////////////
//
// アプリの左サイドのControls、高さ自動または幅自動、初期設定でどちらかに決まる
//
/////////////////////////////////////////////////////////////
class D2DAutoResizeControls : public D2DControls
{
	public :
		D2DAutoResizeControls(){}		
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		//virtual void CreateWindow(D2DWindow* parent, D2DControls* must_be_null, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );
		virtual void BackColor( ID2D1Brush* clr ){ background_ = clr; }
	protected :
		virtual void OnCreate();
		bool bVertical_;
		CComPtr<ID2D1Brush> background_; // default is white		
};


class D2DDropedPack : public D2DControls
{
	public :
		~D2DDropedPack();
		
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual D2DControl* UnPack(){ return hostage_; }
		virtual void Pack( D2DControl* pc );

		virtual void OnDraw(D2DContext& cxt );
	private :
		D2DControl* hostage_; // 人質
};


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


class D2DButton : public D2DControl
{
	public :
		D2DButton():dbg_(0){};

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		static void DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s );
		void OnDraw( D2DContext& cxt );
		
		std::function<void (D2DButton*)> OnClick_;
		std::function<void (D2DContext&, const FRectFBoxModel&, int, const FString& s)> ondraw_;

#ifdef _DEBUG
		
		D2DButton(int debug_level ):dbg_(debug_level){}
#endif
		int dbg_;



};


class D2DScrollbar : public D2DControl
{
	public :
		D2DScrollbar();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void OnCreate();

		void CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		float OffsetOnBtn( int typ );
		void SetTotalSize( float size );

		void Show( bool visible );
		void Clear();
		
		
		D2DScrollbarInfo info_;
		D2DControl* target_control_;		
		
};

class D2DListboxItem : public D2DCaptureObject
{
	public :
		D2DListboxItem(){};
		virtual ~D2DListboxItem(){};
		
		// Datagridで使用
		virtual void DrawBegin( D2DContext& cxt, FSizeF& sz ){};
		virtual void DrawEnd( D2DContext& cxt, FSizeF& sz ){};


		virtual void Draw( D2DContext& cxt, FSizeF& sz, int stat, void* etc );
		virtual float TitleHeight(){ return 0; }

		virtual D2DListboxItem* Clone();
		
		virtual void BindImage( D2DContextText& cxt, FSizeF& sz );
		virtual void OnCreate( D2DControls* parent ){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam){ return 0; } // OnSelectedItem時, captureされる。動作
		
		void* GetData(){ return data_.data; }
	public :
		struct BindData {
			void* data;
			ID2D1SolidColorBrush* color_mouse_move_back;
			ID2D1SolidColorBrush* color_selected_back;
			ID2D1SolidColorBrush* color_selected_text;
		};
		virtual void Bind( D2DWindow* win, BindData& data );


	protected :
		//void* data_; // D2DListboxItemではFStringを想定
		//CComPtr<IDWriteTextLayout> layout_;

		

		BindData data_;
		

		SingleLineText stext_;
};

// Multilineでスクロールバーを表示するにはborder=1,padding>15にすること
class D2DListbox : public D2DControls
{
	friend class D2DCombobox;
	public :
		D2DListbox();
		virtual ~D2DListbox();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, D2DListboxItem* template_item, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);

		void DrawContent( D2DContext& cxt, int cnt, float cx, float cy, float offy, void* etc=NULL );

		void DrawContentTest( D2DContext& cxt, int cnt, float cx, float itemHeight, float offy );

		void ShowWindow( bool bShow );
		void Clear();

		
		void SetDataEx( std::shared_ptr<LONG_PTR> data, UINT cnt );

		LONG_PTR* GetData(){ return serial_content_data_; }
		
		int SelectedIdx(){ return selected_idx_; }
		UINT Size(){ return data_cnt_; }
		void SetItemHeight( UINT id, float height );
		void OnActive( bool bActive );
		

		void UnSelectAll();
		bool IsShowScrollbarV();
		virtual void OnCreate();

		// FString を表示する場合、256行まで
		void AddString( LPCWSTR str );
		void ClearString();
		void UpdateString( int idx, LPCWSTR str );
		void RemoveString( int idx );
		FString GetString( int idx );
		

		// Key-Value型
		int  AddKeyValue( LPCWSTR key, LPCWSTR value );
		FString GetValue( LPCWSTR key );
		void ClearValue();
		void UpdateValue( LPCWSTR key, LPCWSTR str );
		void RemoveValue( LPCWSTR key );
		FString Getkey( int idx );

	protected :
		void SetData( LONG_PTR* datahead, UINT cnt ); // Listbox内はシリアルなデータを++,--でAccessする。
		void OnItemSelected( const FPointF& pt, bool& bCombobox );
		void OnItemMouseFloat( const FPointF& pt );
		void FStringClear();
		void Expand();

		int CalcdiNEx( float offy, float& h2 );
		int CalcIdx( const FPointF& mousept );

		int float_idx_;
		int selected_idx_;
		D2DScrollbar* bar_;
		int idivN_;
		
		std::shared_ptr<LONG_PTR> buffer_data_;
		V4::ItemLoopArray loopitems_;
		std::vector<D2DListboxItem*> Items_; // 表示クラス
		std::vector<float> ItemsHeight_;


		float item_min_height_;
		UINT buffer_size_;

		LONG_PTR* serial_content_data_;
		UINT data_cnt_;

		std::shared_ptr<LONG_PTR> data_fstring_; // only FString data.

		std::map<FString,int> keymap_;
		//std::unordered_map<BStr,int> keymap_;

		/*CComPtr<ID2D1SolidColorBrush> color_mouse_move_;
		CComPtr<ID2D1SolidColorBrush> color_selected_;*/

	public :
		std::function<bool(D2DListbox*,int,D2DListboxItem*)> onselected_changed_;
		OnWndProcExtDelegate OnWndProcExt_;
};

class D2DCombobox : public D2DControls
{
	public :
		D2DCombobox();
		virtual ~D2DCombobox();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat,D2DListboxItem* template_item,FSizeF btnSize, LPCWSTR name, int id=-1 );
		void Clear();
		void SetData( LONG_PTR* datahead, UINT cnt );
		virtual void StatActive(bool bActive);
		//virtual LRESULT KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam);

		int SelectedIdx(){ return lb_->SelectedIdx(); }
		void SetSelectedIdx( int idx );
		FString GetText();
		
		int  AddKeyValue( LPCWSTR key, LPCWSTR txt ){ return lb_->AddKeyValue(key,txt); }

		virtual void OnCreate();

		D2DListbox* GetListbox(){ return lb_; }
		std::function<void(D2DCombobox* cmb, int)> onselected_changed_;

		MSXML2::IXMLDOMNodePtr targetNode_;
		OnWndProcExtDelegate OnWndProcExt_;
	protected :	
		float view_height_;
		FSizeF btnSize_;
		
		D2DListbox* lb_;
		D2DButton* btn_;
		int selected_idx_;
		D2DListboxItem* head_;
		bool bMouseEnter_;
		CComPtr<ID2D1PathGeometry> btn_path_;
		/*CComPtr<ID2D1SolidColorBrush> color_mouse_move_;
		CComPtr<ID2D1SolidColorBrush> color_selected_;*/
	protected :

		void Escape(D2DWindow* d);

		void ComboButtonClick(D2DButton* btn);
		bool bShowListbox_;
	
	protected :
		bool OnSelectedChanged( D2DListbox*,int idx );

};

struct D2DStaticColor
{
	ID2D1Brush* forecolor;
	ID2D1Brush* backcolor;
	ID2D1Brush* wakucolor;
	ID2D1Brush* active_wakucolor;
};
#ifndef TEXTBOXTEST

// The Multiline with scrollba must be "padding.r >= 15".

class D2DTextbox : public D2DControl, public IBridgeTSFInterface
{
	public :

		static bool AppTSFInit();
		static void AppTSFExit();
		static void CreateInputControl(D2DWindow* parent);
		static std::map<HWND,TSF::CTextEditorCtrl*> s_text_inputctrl;

		enum TYP { SINGLELINE=0x1, MULTILINE=0x2, PASSWORD=0x4, RIGHT=0x8, CENTER=0x10,VCENTER=0x20 };

		D2DTextbox(TSF::CTextEditorCtrl* ctrl);
		D2DTextbox(TSF::CTextEditorCtrl* ctrl, TYP typ);
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		virtual void SetRect( const FRectF& rc );

		// IBridgeInterface///////////////////////////////////////////
		virtual FRectF GetClientRect();
		virtual IDWriteTextFormat* GetFormat();
		virtual FRectFBoxModel GetClientRectEx();


		// functions ////////////////////////////////////////
		FString GetText();
		int InsertText( LPCWSTR str, int pos=-1, int strlen=-1 );
		
		const FString& GetViewText();

		void SetFont( CComPtr<IDWriteTextFormat> fmt );
		void SetText(LPCWSTR str);
		void SetText(VARIANT value);
		void SetViewText(LPCWSTR str);
		TYP GetType(){ return typ_; }
		void SetReadOnly(bool bReadOnly );
		void SetSize( const FSizeF& sz );
		void Clear(){ SetText(L""); }

		void TabEnable();

		virtual D2DTextbox* Clone(D2DControls* pacontrol);

		std::wstring FilterInputString( LPCWSTR s, UINT len );

		// scroll //////////////////////////////////////////////
		float RowHeight();
		UINT RowCount( bool all );
		virtual void UpdateScrollbar(D2DScrollbar* bar);

		///////////////////////////////////////////////////////////////////
		std::function<HRESULT(D2DTextbox*,UINT,WPARAM,LPARAM)> EventMessageHanler_;

		std::function<LRESULT(D2DTextbox*,UINT key)> OnPushKey_;

		std::function<void(D2DTextbox*)> OnEnter_; // activeになった時、Enterキーではない
		std::function<void(D2DTextbox*)> OnLeave_; // activeでなくなった時
		
		std::function<bool(D2DTextbox*,const FString&)> OnValidation_;

		

		virtual void StatActive(bool bActive);

		//MSXML2::IXMLDOMNodePtr targetNode_;

		D2D1_COLOR_F brush_fore_;
		D2D1_COLOR_F brush_back_;
		D2D1_COLOR_F brush_border_;
		D2D1_COLOR_F brush_active_border_;
		OnWndProcExtDelegate OnWndProcExt_;
	private :
		
		int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL Clipboard( HWND hwnd, TCHAR ch );
		int TabCountCurrentRow();
		bool TryTrimingScrollbar();
	private :
		TYP typ_;
		bool bActive_;

		TSF::CTextEditorCtrl* ctrl_;
		TSF::CTextContainer ct_;
		FPointF offpt_;
		FString view_text_;
		float temp_font_height_;
		bool bUpdateScbar_;

		
		CComPtr<IDWriteTextFormat> fmt_;
		CComPtr<IDWriteTextLayout> text_layout_;
		
	protected :
		void CalcRender( bool bLayoutUpdate );
	private :
		
		std::shared_ptr<D2DScrollbar> scbar_; // vscrollbar
		D2DMat matEx_; 
};

//// Textboxに右キーのオプションを追加、キャプションも追加
//class D2DTextboxEx : public D2DControls
//{
//	public :			
//		D2DTextboxEx(){};
//		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
//		//void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
//		virtual void OnCreate();
//
//		float CaptionHeight_;
//	protected :
//			
//		static LRESULT TabFunction(D2DTextbox* tx,UINT key);
//		static LRESULT FloatMenuTest(D2DTextboxEx* tx,FPointF pt);
//	protected :
//		D2DTextbox* tx_;
//			
//		FString CaptionTitle_;
//		std::function<LRESULT(D2DTextboxEx* tx,FPointF pt)> floatmenu_;
//
//};
#endif

class D2DCheckbox : public D2DControl
{
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		bool IsCheck(){ return bl_; }
		void SetCheck(bool bl){ bl_ = bl; }

		std::function<void (D2DCheckbox*)> onclick_;
	private :
		bool bl_;

};












class D2DStatic : public D2DControl
{
	public :
		D2DStatic():alignment_(0)
		{
			brush_fore_ = ColorF(ColorF::Black);
			brush_back_ = ColorF(ColorF::White);
		
		}
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, LPCWSTR name );

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		
		
		
		FString GetText(){ return name_; }
		void SetText( LPCWSTR str, int align=-1 );
		void SetAlignment( int md ){ alignment_ = md; }

		void SetFont( IDWriteTextFormat* fmt );
	public :
		D2D1_COLOR_F brush_fore_;
		D2D1_COLOR_F brush_back_;

	private :
		
		SingleLineText stext_;
		int alignment_;
};



class D2DWaiter : public D2DControl
{
	public :
		D2DWaiter(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		static void DrawContent( D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, int pos );
		void OnDraw( D2DContext& cxt );

		std::function<void (D2DContext&, const FRectFBoxModel&, int, const FString& s, int pos)> ondraw_;
		int pos_;
		void Start();

		void* mt_;
};



class D2DSlider : public D2DControls
{
	public :
		D2DSlider(){}

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );


		float max_, min_, pos_;
		FRectF bar_;
		
};

void SetCursor( HCURSOR h );


/////////////////////////////////////////////////////////////
//
//　赤で表示される警告用のDialogbox
//
/////////////////////////////////////////////////////////////
class D2DMessageDialogbox : public D2DControls
{
	public:
		D2DMessageDialogbox(LPCWSTR msg);
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		void ShowDialog(D2DWindow* parent, const FRectFBoxModel& rc, LPCWSTR name, int id);

		void InitDraw();
		void Close();
	protected:
		virtual void OnCreate();
		FString msg_;
		ColorF back_;
};


CComPtr<IDWriteTextFormat> CreateFont( LPCWSTR fontnm, float height, DWRITE_FONT_WEIGHT bwd=DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR, UINT tabwidth=4 );




/////////////////////////////////////////////////////////////
//
// グリッド配置
//
/////////////////////////////////////////////////////////////
class D2DPlacementGrid : public D2DControls
{
	public :
		D2DPlacementGrid():scale_(1),brush_border_(0),brush_back_(0){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		virtual void PlaceBegin();
		virtual void Place( UINT row, UINT col, D2DControl* child );
		virtual void PlaceEnd();

		
		void SetWidth( std::vector<float>& ar );

		void SetScale( float rto );

	public :
		struct Item
		{
			UINT row,col;
			D2DControl* ctrl;
			FRectF rc;
			FPointF offpt;
		};
		std::vector<Item> ar_;	
		

		ColorF brush_border_;
		ColorF brush_back_;
	protected :
		Values<float> widths_;
		float scale_;
		bool recalc_;
		
};

class D2DPlacementGridEx : public D2DPlacementGrid
{
	public :
		D2DPlacementGridEx(){};

		enum TYP{ LEFT=0x1,CENTER=0x2,RIGHT=0x4,TOP=0x8,VCENTER=0x10,BOTTOM=0x20, CENTER_VCENTER=0 };


		struct ItemEx
		{
			UINT row,col;
			D2DControl* ctrl;
			FRectF rc;
			TYP typ;

			FPointF offpt;
		};
		std::vector<ItemEx> arEx_;	

		typedef std::function<FRectF(FRectF,TYP&)> PlaceFunc2;
		

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void Place( D2DControl* child, PlaceFunc2 f );
		virtual void PlaceEnd();
	private :
		std::unordered_map<D2DControl*, PlaceFunc2> m_;
};



class D2DMessageBox : public D2DControls
{
	public :
		D2DMessageBox(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void Close( int result );

		virtual void Draw( D2DContext& cxt );

		void DoModal( D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype );

		static void Show( D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype );


		std::function<void(D2DMessageBox*,int)> onclose_;

	private :
		int Result_;

		FString title_;
		FString message_;
};

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

//////////////////////////////////////////////////////////////////////////
// menu.xmlで作成されるメインフレームメニュー　WM_D2D_COMMANDを送る
//////////////////////////////////////////////////////////////////////////

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
		XMLDOMDocument6 menuxml_;
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



class D2DDebugControl : public D2DControl
{
	public :
		D2DDebugControl(){};

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		void OnDraw(D2DContext& cxt );


		std::function<void(D2DContext& cxt)> debug_draw_;
		
		bool filter_on_;
	protected :
		virtual void OnCreate();
		void DrawText( D2DContextText& cxt, const D2D1_RECT_F& rc, LPCWSTR str );
	private :
		FPointF mpt_;		
		bool scale_on_;
};

class D2DImageRect : public D2DImage
{
	public :
		D2DImageRect(){}
		D2DImageRect( const FPointF& pt ):pt_(pt){}
		
		FRectF GetRect(){ return FRectF( pt_ , FSizeF(prop.size) ); }
		FSizeF GetSize(){ return FSizeF(prop.size); }
		void SetPoint(const FPointF& pt ){ pt_ = pt;}

	private :
		FPointF pt_;
};

/////////////////////////////////////////////////////////////
//
// 右ボタンで表示されるメニュー
//
/////////////////////////////////////////////////////////////
class D2DFloatMenu : public D2DControl
{
	public :

		D2DFloatMenu(FString* title, int titlecnt );
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void OnCreate();

		std::function<void(D2DControl* obj, int idx)> OnSelected_;
	private :
		struct Pack
		{
			FString title;
			FRectF rc;

			void Draw(D2DContext& cxt);
		};
		std::vector<Pack> items_;

		int selected_idx_;
};




/////////////////////////////////////////////////////////////
//
// Tab付きControls page1,page2,page3,,
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



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DStatic* CreateStatic( D2DControls* pacontrol, FSizeF sz, LPCWSTR text );
//D2DTextbox* CreateTextbox( D2DControls* pacontrol, FSizeF sz, LPCWSTR text );
D2DPlacementGrid* CreateGrid(D2DControls* pacontrol, FSizeF sz );
D2DButton* CreateButton( D2DControls* pacontrol, FSizeF sz, LPCWSTR text );
const D2DMessageBox* MessageBox( D2DControl* p, LPCWSTR msg, LPCWSTR title, int btn_type );


};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下、必要かどうか不明
namespace V4 {


/////////////////////////////////////////////////////////////
//
// sheetの上位層
//
/////////////////////////////////////////////////////////////
class D2DSheetWindow;

class D2DBookWindow : public D2DControls
{
	public :
		D2DBookWindow();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		void SetActive(D2DSheetWindow* p);
	private :
		
		ColorF brush_border_;
		ColorF brush_back_;
		D2DSheetWindow* active_;
		
};

/////////////////////////////////////////////////////////////
//
// DblClickで大きくなり、移動できるControls
//
/////////////////////////////////////////////////////////////

class D2DSheetWindow : public D2DControls
{
	public :
		D2DSheetWindow();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );


	private :
		LRESULT WndProcNormal(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT WndProcSmall(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		enum MD{ NONE, MOVE };

		ColorF brush_border_;
		ColorF brush_back_;
		bool bSmall_;
		MD md_;
		FRectF small_rc_;
};


/////////////////////////////////////////////////////////////
//
// Scrollbar付きControls
//
/////////////////////////////////////////////////////////////

class D2DControlsWithScrollbar : public D2DControls
{
	public :
		D2DControlsWithScrollbar();
		virtual ~D2DControlsWithScrollbar();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int typ=0, int id=-1  );
		
		virtual void OnDropObject( D2DControl* pc );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		void SetTotalSize( float cx, float cy );

		enum TYP{ VSCROLLBAR, HSCROLLBAR };

		void ShowScrollbar( TYP typ, bool visible );



		bool bDropped_object_accept_;
	private :
		D2DScrollbar* Vscbar_;
		D2DScrollbar* Hscbar_;
		FSizeF scrollbar_off_;
		float scale_;
		
};

/////////////////////////////////////////////////////////////
//
// Viewをslider表示でアニメーション
//
/////////////////////////////////////////////////////////////
class D2DSlideControls : public D2DControls
{
	public :
		D2DSlideControls(){}
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		void OpenView( int duration_time=600);
		void CloseView(bool bVertical = true,int duration_time=600);
		bool IsOpen();		
	private :
		virtual void OnCreate();
		FSizeF initial_rc_size_;
		
};

class D2DButtonThreeState : public D2DButton
{
	public :
		D2DButtonThreeState(){};

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		
		bool bpushed_;
		float rad_;
};
class D2DMenuItem;
class D2DMenu : public D2DControl
{
	public :
		//enum TYP{ CLOSE, OPEN };
		D2DMenu(){};
		virtual ~D2DMenu();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void SetRect( const FRectFBoxModel& rc );
		
		Grid grid_;
		int float_idx_;
		int menu_colum_idx_;
		D2DMenuItem* item_;

		bool bShowMenuItem_;

		void CreateMenuItem( int id );


		struct MenuItemInfo
		{
			MenuItemInfo():menu_id(0){} //,next_(0),child_(0){}
			~MenuItemInfo()
			{				
				//delete child_;
				//delete next_;
			}
			int menu_id;
			FString title;
			
			std::shared_ptr<MenuItemInfo> next_;
			std::shared_ptr<MenuItemInfo> child_;			
		};

		std::shared_ptr<D2DMenu::MenuItemInfo> AddItem( LPCWSTR title );
	protected:
		//MenuItemInfo* iteminfo_;
		std::shared_ptr<D2DMenu::MenuItemInfo> iteminfo_;
};



class D2DMenuItem : public D2DControls
{
	public :
		D2DMenuItem(){};
		
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, D2DMenu* menu, std::shared_ptr<D2DMenu::MenuItemInfo> info );
		
		bool click_;		

		D2DControl* menu_parent_;
};
class D2DClickItem : public D2DControl
{
	public :
		D2DClickItem(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name );
	private :
		bool bfloat_;
};
/////////////////////////////////////////////////////////////
//
// 右上横に赤い移動マークのついたウインドウ
//
/////////////////////////////////////////////////////////////
class D2DControlWindow : public D2DControls
{
	public :
		enum MD{ NONE, MOVE };

		D2DControlWindow():brush_fore_(0),brush_back_(0){};
		~D2DControlWindow();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		void OnDraw( D2DContext& cxt, FRectF& rc1 );

		std::function<void(int,bool)> onkeydown_;

	protected :
		MD md_;
		bool iscapture_;

		D2DButton* btn1_;
		ColorF brush_fore_;
		ColorF brush_back_;

		

};

class D2DHorizenMenu : public D2DControls
{
	public :
		enum TYP{ CLOSE, OPEN };

		D2DHorizenMenu(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name );


		virtual void OnCreate( D2DWindow* d, D2DControls* pacontrol );

	protected :
		void FireOpenClose();
	protected :
		int typ_;
		int closing_height_;
		FRectF rcb_;
		Grid grid_;


};


};
