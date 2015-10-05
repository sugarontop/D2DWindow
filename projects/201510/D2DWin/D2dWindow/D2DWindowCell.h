#pragma once


namespace V4 {


class TableFRectF
{
	public :
		TableFRectF();
		TableFRectF(int rowcnt, int colcnt);
		~TableFRectF();

		void Set(int rowcnt, int colcnt);
	
		FRectF GetRectF(int row, int col);

		void SetColWidth( int col, float width );
		void SetRowHeight( int row, float height );
		int ColCnt(){ return colcnt_; }
		int RowCnt() { return rowcnt_; }

		UINT magicnumber(int row, int col);
		FRectF GetRectF(UINT magicnumber);
	private :
		float* xpos_;
		float* ypos_;

		int rowcnt_,colcnt_;

};

class D2DTableControls : public D2DControls
{
	public :
		// D2DControl interface///////////////////////////////////////////
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, int rowcnt,int colcnt, LPCWSTR name, int id = -1);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		void SetControl( int row, int col, D2DControl* ctrl );
		void Draw(D2DContext& cxt);
	public :
		
	protected :
		TableFRectF trc_;
		std::map<UINT, D2DControl*> tctrl_;
};

};




