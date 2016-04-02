#pragma once

#include "TextLayout.h"
#include "D2DWindow.h"

namespace TSF {
class CTextStore;
class CTextEditSink;
class CTextContainer;
//----------------------------------------------------------------
// D2DTextbox::AppTSFInit()
// {
//	g_pTextInputCtrl = new TSF::CTextEditorCtrl();
//	CoCreateInstance 
//		IID_ITfThreadMgr g_pThreadMgr
//		IID_ITfKeystrokeMgr g_pKeystrokeMgr
//	CoCreateInstance
//		IID_ITfDisplayAttributeMgr __s_pdam
//  }
//CTextEditor
//	protected :
//		HWND hWnd_;
//		CTextEditSink* pTextEditSink_;	
//		CTextLayout layout_;
//		CTextStore* pTextStore_;
//	    ITfDocumentMgr* pDocumentMgr_;
//	    ITfContext* pInputContext_;
//	
//	public :
//		BOOL InitTSF();
//			new CTextStore(this);
//			pDocumentMgr_ = CreateDocumentMgr
//			pInputContext_ = CreateContext
//			new CTextEditSink(this);
//
//	    BOOL UninitTSF();
//    
//    
//	
//CTextEditSink : public ITfTextEditSink
//	private :
//		CTextEditor *_pEditor;
//	
//	
//class CTextStore : public ITextStoreACP
//	private :
//	    CTextEditor *_pEditor;
//		ITextStoreACPSink* TextStoreACPSink_;
//	
//
//class CTextLayout
//		BOOL Layout(D2DContext& hdc, const WCHAR *psz, ...
//		BOOL Render(D2DContext& hdc, const FRectF& rc, ... 
//	
//
//
//----------------------------------------------------------------

class CTextEditor 
{
	public:
		CTextEditor();
		virtual ~CTextEditor(); 
    
		void SetContainer( CTextContainer* ct ){ ct_ = ct; }
		CTextContainer* GetContainer(){ return ct_; }
    
		void MoveSelection(int nSelStart, int nSelEnd, bool bTrail=true);
		BOOL MoveSelectionAtPoint(POINT pt);
		BOOL MoveSelectionAtNearPoint(POINT pt);
		BOOL InsertAtSelection(LPCWSTR psz);
		BOOL DeleteAtSelection(BOOL fBack);
		BOOL DeleteSelection();

		void MoveSelectionNext();
		void MoveSelectionPrev();
		BOOL MoveSelectionUpDown(BOOL bUp, bool bShiftKey);
		BOOL MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey);

		void Render(V4::D2DContext& hdc);
		void CalcRender(V4::D2DContext& hdc);
		int GetSelectionStart() {return ct_->nSelStart_;}
		int GetSelectionEnd() {return ct_->nSelEnd_;}

		
		HWND GetWnd() {return hWnd_;}

		BOOL InitTSF(HWND hWnd);
		BOOL UninitTSF();
 
		void InvalidateRect();
		float GetLineHeight() {return layout_.GetLineHeight();}
		CTextLayout *GetLayout() {return &layout_;}
		int CurrentCaretPos();
		void ClearCompositionRenderInfo();
		BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);


		CTextContainer* ct_;
		IBridgeTSFInterface* bri_;
		D2D1_MATRIX_3X2_F mat_;

		void SetFocus();
	protected :	
		
		void Reset( IBridgeTSFInterface* rect_size );
	

	protected :
		HWND hWnd_;
		
		CTextLayout layout_;
		CTextEditSink* pTextEditSink_;	
	private:
		CTextStore* pTextStore_;

		COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
		int nCompositionRenderInfo_;
		TfEditCookie ecTextStore_;
	
			
		ITfDocumentMgr* pDocumentMgr_;
		ITfContext* pInputContext_;

};


class CTextEditSink : public ITfTextEditSink
{
	public:
		CTextEditSink(CTextEditor *pEditor);

		//
		// IUnknown methods
		//
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		//
		// ITfTextEditSink
		//
		STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

		HRESULT _Advise(ITfContext *pic);
		HRESULT _Unadvise();


		std::function<void()> OnChanged_;

	private:
		long _cRef;
		ITfContext *_pic;
		DWORD _dwEditCookie;
		CTextEditor *_pEditor;
};


class CTextEditorCtrl : public CTextEditor
{
	public :
		HWND Create(HWND hwndParent);

		LRESULT  WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		
		void SetContainer( CTextContainer* ct, IBridgeTSFInterface* ib );
		
		CTextEditor& GetEditor(){ return *this; }

		void OnEditChanged();

		void Password(bool bl){ layout_.Password(bl); }
	private :
		//void Move(int x, int y, int nWidth, int nHeight);
		void OnSetFocus(WPARAM wParam, LPARAM lParam);
		void OnPaint(V4::D2DContext& hdc);
		BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
		void OnLButtonDown(WPARAM wParam, LPARAM lParam);
		void OnLButtonUp(WPARAM wParam, LPARAM lParam);
		void OnMouseMove(WPARAM wParam, LPARAM lParam);

	private:
		UINT _uSelDragStart;

};

// mainframe.cpp‚É‚ ‚é
CTextEditorCtrl* GetTextEditorCtrl(); 



};
