
#pragma once

#include "TextLayout.h"
#include "D2DWindow.h"

namespace TSF {
class CTextStore;
class CTextEditSink;
class CTextContainer; // D2DWindow.h‚Åinclude‚³‚ê‚Ä‚¢‚é
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextEditor 
{
public:
    CTextEditor();
    ~CTextEditor(); 
    
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

    void SetWnd(HWND hwnd) {_hwnd = hwnd;}
    HWND GetWnd() {return _hwnd;}

    BOOL InitTSF();
    BOOL UninitTSF();
    void SetFocusDocumentMgr();

	

    void InvalidateRect();

    float GetLineHeight() {return _layout.GetLineHeight();}
    CTextLayout *GetLayout() {return &_layout;}
	void Reset( IBridgeTSFInterface* rect_size );

	void ClearCompositionRenderInfo();
    BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);

	CTextContainer* ct_;
	IBridgeTSFInterface* bri_;
	D2D1_MATRIX_3X2_F mat_;
	
	int CurrentCaretPos();

protected :
	HWND _hwnd;
	CTextEditSink* _pTextEditSink;	
	CTextLayout _layout;
	

private:
    COMPOSITIONRENDERINFO *_pCompositionRenderInfo;
    int _nCompositionRenderInfo;

    CTextStore* _pTextStore;
    CComPtr<ITfDocumentMgr>_pDocumentMgr;
    CComPtr<ITfContext> _pInputContext;
	
    TfEditCookie _ecTextStore;
	
};

#define TES_INVALID_COOKIE  ((DWORD)(-1))

class CTextEditSink : public ITfTextEditSink
{
public:
    CTextEditSink(CTextEditor *pEditor);
    virtual ~CTextEditSink() {};

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

		LRESULT  WndProcX(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		
		void SetContainer( CTextContainer* ct, IBridgeTSFInterface* ib );
		
		CTextEditor& GetEditor(){ return *this; }

		void OnEditChanged();

		void Password(bool bl){ _layout.Password(bl); }
private :
    void Move(int x, int y, int nWidth, int nHeight);
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
