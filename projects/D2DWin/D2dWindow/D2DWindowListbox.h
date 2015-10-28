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

#include "ItemLoopArray.h"

namespace V4 {

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
			ID2D1SolidColorBrush* color_text;
		};
		virtual void Bind( D2DWindow* win, BindData& data );

	protected :
		BindData data_;
};

// Multilineでスクロールバーを表示するにはborder=1,padding>15にすること
class D2DListbox : public D2DControls
{
	friend class D2DCombobox;
	public :
		D2DListbox(){};
		virtual ~D2DListbox();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, D2DListboxItem* template_item, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

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

		LONG_PTR* GetBuffer(){ return serial_content_data_; }

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
		

		D2D1_COLOR_F brush_selected_, brush_floating_, brush_selected_fore_;
		D2D1_COLOR_F brush_fore_, brush_back_;
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

		
		OnWndProcExtDelegate OnWndProcExt_;
	protected :	
		float view_height_;
		FSizeF btnSize_;
		
		D2DListbox* lb_;
		D2DButton* btn_;
		int selected_idx_;
		D2DListboxItem* head_;
		bool bMouseEnter_;
		ComPTR<ID2D1PathGeometry> btn_path_;
		
		D2D1_COLOR_F brush_selected_, brush_floating_, brush_selected_fore_;
		D2D1_COLOR_F brush_fore_, brush_back_;

	protected :

		void Escape(D2DWindow* d);

		void ComboButtonClick(D2DButton* btn);
		bool bShowListbox_;
	
	protected :
		bool OnSelectedChanged( D2DListbox*,int idx );

};




//
//class D2DListboxItemEx : public D2DListboxItem
//{
//	public :
//		D2DListboxItemEx( D2DControls* ctrls );
//		
//		// Datagridで使用
//		//virtual void DrawBegin( D2DContext& cxt, FSizeF& sz ){};
//		//virtual void DrawEnd( D2DContext& cxt, FSizeF& sz ){};
//		//virtual void Draw( D2DContext& cxt, FSizeF& sz, int stat, void* etc );
//		virtual float TitleHeight(){ return height_; }
//
//		virtual D2DListboxItem* Clone();
//		
//		virtual void BindImage( D2DContextText& cxt, FSizeF& sz );
//		//virtual void OnCreate( D2DControls* parent ){};
//		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam){ return 0; } // OnSelectedItem時, captureされる。動作
//		
//		void* GetData(){ return data_.data; }
//	public :
//		struct BindData {
//			void* data;
//			ID2D1SolidColorBrush* color_mouse_move_back;
//			ID2D1SolidColorBrush* color_selected_back;
//			ID2D1SolidColorBrush* color_selected_text;
//		};
//		virtual void Bind( D2DWindow* win, BindData& data );
//
//	protected :
//		BindData data_;
//		D2DControls* ctrls_;
//		float height_;
//};

};

