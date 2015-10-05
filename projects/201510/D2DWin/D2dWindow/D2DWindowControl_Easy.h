#pragma once

#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "D2DWindow.h"
#include "D2DWindowMessage.h"
#include "gdi32.h"
#include "faststack.h"
#include "D2DCommon.h"




namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};

namespace V4 {

struct SolidColor
{
	D2D1_COLOR_F color;
	const _weak ID2D1SolidColorBrush* br;
};
class IUpdatar
{
	public:
		virtual void RequestUpdate(D2DControl* p, int typ) = 0;

};

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
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		virtual void UpdateScrollbar(D2DScrollbar* bar){};
		virtual void SetRect( const FRectFBoxModel& rc ){ rc_ = rc; }
		virtual void SetRect( const FPointF& pt, const FSizeF& sz ){ rc_.SetRect(pt,sz); }
		virtual void UpdateStatus( D2DControls* p){}
		virtual D2DControl* UnPack(){ return this; }
		virtual void StatActive(bool bActive);
		virtual bool IsAutoResize(){ return auto_resize_; }
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms){}
		virtual void OnResutructRnderTarget(bool bCreate) {}

		int Stat(int new_stat);
		int GetStat()  const { return stat_; }
		void SetStat(int s) { stat_ = s; }

		const FRectFBoxModel& GetRect() const { return rc_; }

		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;
		
		FRectF GetDPRect() const{ return dp_rc_; } // Device変換されたrc_
		
		virtual void BackColor(D2D1_COLOR_F clr){}
		virtual void OnHostage( D2DControls* p, bool block ){}
		
		
		//CHDL GetCHDL() const { return chdl_; } // Get Control Handle
		D2DMat Matrix() const { return mat_; }
		FString GetName() const { return name_; }
		
		int GetID() const { return id_; }
		
		void SetName(LPCWSTR nm) { name_ = nm; }
		virtual void DestroyControl();
		void SafeDestroyControl();


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
		//CHDL chdl_;
		FString name_;
		int id_;
};




class D2DControls : public D2DControl
{
	public :
		D2DControls():mouse_enter_(false){ scrollbar_off_ = FSizeF(0,0); } // backclr_ = D2RGB(255,255,255);}

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual LRESULT KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		virtual void MeToLast();
		virtual FRectF GetContentRect(){ return rc_.GetContentRect(); }
		virtual bool IsAutoResize(){ return auto_resize_; }
		virtual void DestroyControl();
		void Clear();
		//virtual void BackColor(D2D1_COLOR_F clr){ backclr_ = clr; }
		virtual void Bind(void* p) {};
		
		void Lineup(bool vertical=true ); // childを整列させる

		std::shared_ptr<D2DControl> Detach( D2DControl* );
		void Attach( std::shared_ptr<D2DControl> ctrl );
		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		
		
	public :
		std::vector<std::shared_ptr<D2DControl>> controls_;
		bool mouse_enter_;
		FSizeF scrollbar_off_;
		bool bwindow_size_;
	
		std::pair<int,int> vector_idx;
	protected :
		SolidColor back_color_;
		SolidColor border_color_;

	public :
		LRESULT SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);		
};
class D2DGroupControls : public D2DControls
{
	public :
		D2DGroupControls();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

		void ShowFrame( bool bShow ){ stat_ = ( bShow ? stat_&BORDER : stat_&~BORDER ); }
		void Moveable( bool bMoveable ){ move_able_ = (bMoveable ? 1 : 0); }

	protected :
		int move_able_;
		
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
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow(D2DWindow* parent, D2DControls* must_be_null, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );
		void CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id=-1 );
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		virtual void OnResutructRnderTarget(bool bCreate);

		virtual void OnCreate();
		virtual void BackColor(D2D1_COLOR_F clr);

		std::function<FRectF(FRectF, GDI32::FSize)> calc_auto_size_;
		

		OnWndProcExtDelegate OnWndProcExt_;
	protected :
		FRectF CalcAutoSize( const GDI32::FSize& sz );

		FString error_msg_;
		float zoom_;
		bool bAutoSize_;
		SolidColor back_color_;
};

class D2DButton : public D2DControl
{
	public :
		D2DButton():bChecked_(false){};

		enum TYP { NORMAL, CHKCKBOX, RADIO };

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		void CreateCheckboxButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id = -1);
		void CreateRadioButton(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id = -1);

		static void DrawNormalButton(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool);
		static void DrawCheckButton(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool bChecked);
		static void DrawRadioButton(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, bool bChecked);


		void OnDraw( D2DContext& cxt );
		
		std::function<void (D2DButton*)> OnClick_;
		std::function<void (D2DContext&,const FRectFBoxModel&, int,const FString&, bool)> ondraw_;
		
		void RadioButtonUpdate(bool bChecked);
		void LinkRadioButton(D2DButton* next);
	private :
		bool bChecked_;
		TYP typ_;
		D2DButton* prev_;
		D2DButton* next_;

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
class D2DImage : public D2DControl
{
	public :
		D2DImage();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void OnCreate();
		virtual void OnResutructRnderTarget(bool bCreate);

		bool LoadImage( ID2D1DeviceContext* cxt, LPCWSTR img );

	private :
		ComPTR<ID2D1Bitmap1> bmp_;
		FString filename_;

};


class D2DWaiter : public D2DControl
{
	public :
		D2DWaiter(){};
		~D2DWaiter();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		
		static void DrawContent2(D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s, int pos);
		void OnDraw( D2DContext& cxt );

		std::function<void (D2DContext&, const FRectFBoxModel&, int, const FString& s, int pos)> ondraw_;
		
		void Start();
		void Cancel();


	protected :

		int pos_;
		MoveTarget* mt_;
		LARGE_INTEGER freq_;
		DWORD start_time_;
		
};

class D2DFRectFBM : public D2DControl
{
	public :
		D2DFRectFBM(){};
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int typ, int id = -1);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
		virtual void OnResutructRnderTarget(bool bCreate);
		virtual void OnCreate();
	protected :
		SolidColor fore_color_;
		SolidColor back_color_;
		int typ_;
};

class D2DMessageBox : public D2DControls
{
	public:
		D2DMessageBox() {};
		void DoModal(D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype);

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void Close(int result);

		virtual void Draw(D2DContext& cxt);
		static void Show(D2DControl* parent, LPCWSTR msg, LPCWSTR title, int btntype);
		std::function<void(D2DMessageBox*, int)> onclose_;

	private:
		int Result_;

		std::wstring title_;
		std::wstring message_;
};






void SetCursor( HCURSOR h );


// VAARIANT CONVERT

D2D1_COLOR_F VariantColor( const VARIANT& v );
bool VariantBOOL(const VARIANT& src);
bool findParameterMap(const std::map<std::wstring, VARIANT>& prms, LPCWSTR key, _variant_t& def, _variant_t& ret);
void ParameterColor(D2DWindow* parent, SolidColor& clr, const std::map<std::wstring, VARIANT>& prms, LPCWSTR key);

bool CreateD2D1Bitmap(D2DContext& cxt, ID2D1Bitmap** ppBitmap, Binary& bin);
};
