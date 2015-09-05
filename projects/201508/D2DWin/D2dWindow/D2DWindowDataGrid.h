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

#include "ItemLoopArray.h"
namespace V4 {

	class D2DDataGrid : public D2DControls
	{
		public:
			D2DDataGrid();
			virtual ~D2DDataGrid();
			virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
			virtual void SetRect(const FRectFBoxModel& rc);
			virtual void OnResutructRnderTarget(bool bCreate);

			void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, float title_height, int colcnt, LPCWSTR name, int id);
			void SetTitle( int title_cnt );
			void AllocateBuffer( int cnt );
			void Clear();
			int GetSelectedIdx();
			void UnSelect();


			std::set<int> GetSelectIdxes(){ return selected_idx_s_; }

			virtual void UpdateScrollbar(D2DScrollbar* bar);
			

			std::shared_ptr<LONG_PTR> GetDatabox(int* cnt){ *cnt = data_cnt_; return databox_; }
			std::function<void(D2DDataGrid*)> Destroy_;

			
			struct RowInfo
			{
				int row;				
				std::set<int> selected_rows;

				int float_row;
				FSizeF sz_row;

				const float* col_xpos;

			};

			std::function<void(D2DDataGrid*,D2DContext& cxt, RowInfo& rinfo )> DrawRow_;
			std::function<bool(D2DDataGrid*, int)> onselected_changed_;
			std::function<void(D2DDataGrid*, UINT)> onmouse_extend_;

		protected :
			void DrawContent(D2DContext& cxt, int cnt, float cx, float itemHeight, float offy, void* etc);
			int CalcdiNEx(float offy, float& h2);

			void OnItemSelected( FPointF pt, bool& bCombobox, bool bControlKey);
			void OnItemMouseFloat( FPointF pt);
			int CalcIdx(const FPointF& mousept);
			virtual float* ColXPosition();

		protected:
			std::shared_ptr<LONG_PTR> databox_;
			float item_min_height_;
		
			V4::ItemLoopArray loopitems_;
			int idivN_;
			int data_cnt_;
			

			std::set<int> selected_idx_s_;
			int float_idx_;
			std::vector<float> ItemsHeight_;
			D2DScrollbar* bar_;
			int colcnt_;
			float title_header_height_;
			bool multi_select_;
			bool active_;
	};


	class D2DDataGridListbox : public D2DDataGrid
	{
		public :
			D2DDataGridListbox(){}
			void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, LPCWSTR name, int id);
			virtual void OnResutructRnderTarget(bool bCreate);

			void AddString(LPCWSTR key, LPCWSTR val);
			int Count();
			LPCWSTR GetValue( LPCWSTR key );
			LPCWSTR GetKey( int idx );
			void Clear();
			virtual float* ColXPosition();
			virtual void SetRect(const FRectFBoxModel& rc);
			
			
		private :
			struct kv
			{
				std::wstring key;
				std::wstring val;
			};

			std::map<std::wstring, kv*> keyvalues_;
			std::vector<std::shared_ptr<kv>> ar_;


			SolidColor wfloating;
			SolidColor wfloating_w;
			SolidColor wselecting;
			SolidColor wselecting_w;
	};
	
	class D2DDropdownList : public D2DControls
	{
		public :
			D2DDropdownList(){};
			void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, LPCWSTR name, int id);
			virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
			void AddString(LPCWSTR key, LPCWSTR val);
			virtual void SetRect(const FRectFBoxModel& rc);
			D2DDataGridListbox* GetListbox(){ return ls_; }
			
		private :
			D2DDataGridListbox* ls_;
			FRectF headrc_;
			std::wstring value_;
			std::wstring key_;

	};


};
