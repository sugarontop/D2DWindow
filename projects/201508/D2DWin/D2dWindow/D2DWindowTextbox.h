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

#include "TSF\IBridgeTSFInterface.h"
#include "TSF\TextContainer.h"

namespace V4 {
//struct IBridgeTSFInterface
//{
//	IBridgeTSFInterface(){}
//	virtual V4::FRectF GetClientRect() = 0;
//	virtual IDWriteTextFormat* GetFormat() = 0;
//	virtual V4::FRectFBoxModel GetClientRectEx() = 0;
//};



class D2DTextbox : public D2DControl, public IBridgeTSFInterface
{
	public :
		typedef std::wstring wstring;

		static bool AppTSFInit();
		static void AppTSFExit();
		static void CreateInputControl(D2DWindow* parent);
		static std::map<HWND,TSF::CTextEditorCtrl*> s_text_inputctrl;

		enum TYP { SINGLELINE=0x1, MULTILINE=0x2, PASSWORD=0x4, RIGHT=0x8, CENTER=0x10,VCENTER=0x20 };
		
		D2DTextbox(TSF::CTextEditorCtrl* ctrl);
		D2DTextbox(TYP typ, TSF::CTextEditorCtrl* ctrl);
	public :
		// IBridgeInterface///////////////////////////////////////////
		virtual FRectF GetClientRect();
		virtual IDWriteTextFormat* GetFormat();
		virtual FRectFBoxModel GetClientRectEx();

		// D2DControl interface///////////////////////////////////////////
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void StatActive(bool bActive);
		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);

		// functions ////////////////////////////////////////
		virtual void SetRect( const FRectF& rc );// スクロールバーへサイズの変更送信
		void SetSize(const FSizeF& sz);// SetRectする
	
		wstring GetText();
		int InsertText( LPCWSTR str, int pos=-1, int strlen=-1 );
		
		void SetFont( ComPTR<IDWriteTextFormat> fmt );
		void SetText(LPCWSTR str);
		void SetText(VARIANT value);
		void SetViewText(LPCWSTR str);
		TYP GetType(){ return typ_; }
		void SetReadOnly(bool bReadOnly );
		
		void Clear(){ SetText(L""); }

		void TabEnable();
		

		virtual D2DTextbox* Clone(D2DControls* pa);

		wstring FilterInputString( LPCWSTR s, UINT len );

		

		// scroll //////////////////////////////////////////////
		float RowHeight();
		UINT RowCount( bool all );
		virtual void UpdateScrollbar(D2DScrollbar* bar);

		//Event Handlers/////////////////////////////////////////////////////////////////
		std::function<HRESULT(D2DTextbox*,UINT,WPARAM,LPARAM)> EventMessageHanler_;

		std::function<void(D2DTextbox*)> OnEnter_; // activeになった時、Enterキーではない
		std::function<void(D2DTextbox*)> OnLeave_; // activeでなくなった時
		
		std::function<bool(D2DTextbox*,const wstring&)> OnValidation_;
		
		OnWndProcExtDelegate OnWndProcExt_;
	protected :
		std::function<LRESULT(D2DTextbox*, UINT key)> OnPushKey_;

	public :

		D2D1_COLOR_F brush_fore_;
		D2D1_COLOR_F brush_back_;
		D2D1_COLOR_F brush_border_;
		D2D1_COLOR_F brush_active_border_;
		

	private :
		
		int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL Clipboard( HWND hwnd, TCHAR ch );
		int TabCountCurrentRow();
		bool TryTrimingScrollbar();
		void ActiveSw();
		void CalcRender(bool bLayoutUpdate);
	private :
		TYP typ_;
		bool bActive_;

		TSF::CTextEditorCtrl* ctrl_;
		TSF::CTextContainer ct_;
		FPointF offpt_;
		float temp_font_height_;
		bool bUpdateScbar_;

		
		ComPTR<IDWriteTextFormat> fmt_;
		ComPTR<IDWriteTextLayout> text_layout_;
	
		std::shared_ptr<D2DScrollbar> scbar_; // vscrollbar
		D2DMat matEx_; // スクロール用matrix
};

class D2DStatic : public D2DControl
{
	public :
		D2DStatic(){}
		// D2DControl interface///////////////////////////////////////////
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, int alignment, LPCWSTR name, int id = -1);

		D2D1_COLOR_F brush_fore_;
		D2D1_COLOR_F brush_back_;

		virtual void SetParameters(const std::map<std::wstring, VARIANT>& prms);
	private :
		std::wstring title_;
		ComPTR<IDWriteTextLayout> textlayout_;
		FPointF offpt_;
		bool isblack_;
		int alignment_;

};


};
