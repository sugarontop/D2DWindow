#pragma once 


namespace V4 {

class SequentialData
{
	public :
		virtual ~SequentialData(){}
		virtual void* at( int idx ) = 0;
		virtual UINT size() = 0;
		virtual void add( LPARAM ){};
};

class D2DGridControl : public D2DControls
{
	public :
		D2DGridControl();
		~D2DGridControl(){ Clear(); }
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, D2DListboxItem* template_item, LPCWSTR name, int id=-1 );
		void Clear();
		

		void DrawContentTitle( D2DContext& cxt);
		void DrawContentLast( D2DContext& cxt );

		void DrawContent( D2DContext& cxt, int cnt, float cx, float itemHeight, float offy, void* etc );
		
		int SelectedIdx(){ return selected_idx_; }
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		LRESULT RowsProc(D2DWindow* parent, UINT message, const FPointF& pt);

		
		
	public : // Data関数
		typedef  std::shared_ptr<SequentialData> BodyData;

		void SetData( BodyData data );
		BodyData GetData(){ return serial_data_; }
		void DataUpdate();

		

	protected :
		virtual void OnCreate();
		int CalcdiNEx( float offy, float& h2 );
		int CalcIdx( const FPointF& mousept );
		

	protected :
		ItemLoopArray loopitems_;
		std::vector<D2DListboxItem*> Items_; // 表示クラス
		std::vector<float> ItemsHeight_;
		int selected_idx_;
		int float_idx_;

		float item_min_height_;
		float title_height_;
		
		
		int data_cnt_; // 表示する全データ行、削除フラグのある行は含まれない
		
		//LONG_PTR* serial_constent_data_;
		std::shared_ptr<SequentialData> serial_data_;
		D2DScrollbar* bar_;
		D2DListboxItem* template_item_;

		CComPtr<ID2D1SolidColorBrush> color_mouse_move_;
		CComPtr<ID2D1SolidColorBrush> color_selected_;
};

class StringSequentialData : public SequentialData
{
	public :		
		virtual void* at( int idx ){ FString& x = ar_[idx]; return (void*)&x; }
		virtual UINT size(){ return ar_.size(); }
	public :
		std::vector<FString> ar_;
};
/*
	D2DGridControl* c2 = new D2DGridControl();
	c2->CreateWindow(parent,cs,FRectF(10,10,100,500),VISIBLE|BORDER,NONAME );
	std::shared_ptr<StringSequentialData> d = std::make_shared<StringSequentialData>();
	for( int i = 0; i < 20; i++ )
		d->ar_.push_back( L"test" );

	c2->SetData( d );
*/



class D2DStatsuControlItem : public D2DListboxItem
{
	public :
		D2DStatsuControlItem(){}
		D2DStatsuControlItem(float* width):line_width_(width){};
	
		virtual void Draw( D2DContext& cxt, FSizeF& sz, int stat, void* etc );
		virtual D2DListboxItem* Clone();
	
	protected :
		float* line_width_;
	public :
		class StatusSequentialData : public SequentialData
		{
			public :						
				virtual void* at( int idx ){ return (void*)&ar_[idx]; } //[ sort_indext_[idx] ]; }
				virtual UINT size(){ return ar_.size(); }
			public :
				struct Row
				{
					int idx;
					DateTime time;
					FString info;
					FString msg;
				};
				void set( const std::vector<Row>& ar )
				{
					ar_ = ar;
					sort_indext_.resize( ar.size());
					for( UINT i = 0; i < sort_indext_.size(); i++ )
						sort_indext_[i] = i;
				}
				void add( Row row )
				{
					if ( row.idx < 0 )
					{
						row.idx = ar_.size();
					}

					ar_.push_back( row );
					sort_indext_.push_back( sort_indext_.size() );
				}
				void del( int idx )
				{
					if ( -1 < idx && idx < (int)ar_.size())
					{
						auto it = ar_.begin() + idx;
						ar_.erase(it);
					}
				}
				virtual void add( LPARAM lp)
				{
					Row* r = (Row*)lp;

					add( *r );
				}


			private :
				std::vector<Row> ar_;
				std::vector<int> sort_indext_;
		};
};


class D2DStatsuControlItemTemplate : public D2DStatsuControlItem
{
	public :
		D2DStatsuControlItemTemplate();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void DrawBegin( D2DContext& cxt, FSizeF& sz );
		virtual void DrawEnd( D2DContext& cxt, FSizeF& sz );
		virtual D2DListboxItem* Clone();
		void OnClickTitle(int idx);
		virtual float TitleHeight(){ return title_height; }
	protected :
		float line_width[4];
		float title_height;
		int md_;
		D2DMat mat_; 
};

/*
D2DGridControl* c2 = new D2DGridControl();

	static D2DStatsuControlItemTemplate x1;
	
	typedef D2DStatsuControlItem::StatusSequentialData BodyData;

	c2->CreateWindow(parent,cs,FRectF(20,50,520,250),VISIBLE|BORDER,26,&x1,NONAME );
	std::shared_ptr<BodyData> d = std::make_shared<BodyData>(); 
	std::vector<BodyData::Row> ar;

	// 表示行本体は１０行
	for( int i = 0; i < 10; i++ )
	{
		BodyData::Row r;

		r.idx = i;
		r.msg = L"test" + FString::Number(i);	
		r.time = DateTime::Now();
		ar.push_back( r );
	}

	// データポインタへ生データわたす
	d->set(ar);

	// DataGridへデータを渡す
	c2->SetData( d );
*/

};