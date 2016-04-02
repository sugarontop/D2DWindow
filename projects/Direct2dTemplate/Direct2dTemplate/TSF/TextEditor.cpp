#include "stdafx.h"
#include "tsf\TextEditor.h"
#include "tsf\TextStoreACP.h"
#include "D2DWindow.h"

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
extern ITfThreadMgr2* g_pThreadMgr;
#else
extern ITfThreadMgr* g_pThreadMgr;
#endif
extern TfClientId g_TfClientId;

using namespace TSF;

CTextEditor::CTextEditor() 
{
    pTextStore_ = NULL;    
	ct_ = NULL;
	pCompositionRenderInfo_ = NULL;
    nCompositionRenderInfo_ = 0;
	pDocumentMgr_ = NULL;
	pInputContext_ = NULL;

}

CTextEditor::~CTextEditor() 
{
    UninitTSF();
	
}

//----------------------------------------------------------------
//
// Application Initialize
//
//----------------------------------------------------------------

BOOL CTextEditor::InitTSF(HWND hWnd)
{
	BOOL ret = FALSE;

    pTextStore_ = new CTextStore(this);

	ITfDocumentMgr* pDocumentMgrPrev = NULL;
	

    if (!pTextStore_) 
		goto Exit;
    
	if (FAILED(g_pThreadMgr->CreateDocumentMgr(&pDocumentMgr_)))
		goto Exit;

    if (FAILED(pDocumentMgr_->CreateContext(g_TfClientId, 0, pTextStore_, &pInputContext_, &ecTextStore_)))
		goto Exit;

    if (FAILED(pDocumentMgr_->Push(pInputContext_)))
		goto Exit;

#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	g_pThreadMgr->SetFocus(pDocumentMgr_);
#else
	if (FAILED( g_pThreadMgr->AssociateFocus(hWnd, pDocumentMgr_, &pDocumentMgrPrev)))
		goto Exit;
#endif
	
	hWnd_ = hWnd;

	if ( pDocumentMgrPrev )
		pDocumentMgrPrev->Release();

	pTextEditSink_ = new CTextEditSink(this);
    if (!pTextEditSink_)
		goto Exit;

    pTextEditSink_->_Advise(pInputContext_);


	ret = TRUE;

	

Exit :
	//if ( pDocumentMgrPrev )
	//	pDocumentMgrPrev->Release();
    return ret;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::UninitTSF()
{
    if (pTextEditSink_)
    {
        pTextEditSink_->_Unadvise();
        pTextEditSink_->Release();
        pTextEditSink_ = NULL;
    }
	
	if (pDocumentMgr_)
    {
        pDocumentMgr_->Pop(TF_POPF_ALL);
		
		pDocumentMgr_->Release();
		pDocumentMgr_ = NULL;
	}

	if (pInputContext_)
	{
		pInputContext_->Release();
		pInputContext_ = NULL;
	}

    if (pTextStore_)
    {
        while( pTextStore_->Release() );
        pTextStore_ = NULL;
    }

    return TRUE;
}
//----------------------------------------------------------------
//
// move Caret 
//
//----------------------------------------------------------------

void CTextEditor::MoveSelection(int nSelStart, int nSelEnd, bool bTrail)
{
	if ( nSelEnd < nSelStart )
		std::swap( nSelStart, nSelEnd );

    int nTextLength = (int)ct_->GetTextLength();
    if (nSelStart >= nTextLength)
        nSelStart = nTextLength;

    if (nSelEnd >= nTextLength)
        nSelEnd = nTextLength;

    ct_->nSelStart_ = nSelStart; 
    ct_->nSelEnd_ = nSelEnd;

	ct_->bSelTrail_ = bTrail;


    pTextStore_->OnSelectionChange();
}

//----------------------------------------------------------------
//
//	caretが動く時
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionNext()
{
    int nTextLength = (int)ct_->GetTextLength();

	int zCaretPos = (ct_->bSelTrail_ ? ct_->nSelEnd_ : ct_->nSelStart_ );
	zCaretPos = min(nTextLength, zCaretPos+1); // 1:次の文字

    ct_->nSelStart_ = ct_->nSelEnd_ = zCaretPos;
    pTextStore_->OnSelectionChange();
}

//----------------------------------------------------------------
//
//　caretが動く時
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionPrev()
{
	int zCaretPos = (ct_->bSelTrail_ ? ct_->nSelEnd_ : ct_->nSelStart_ );
	zCaretPos = max(0, zCaretPos-1);

    ct_->nSelEnd_ = ct_->nSelStart_ = zCaretPos;
    pTextStore_->OnSelectionChange();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionAtPoint(POINT pt)
{
    BOOL bRet = FALSE;
    int nSel = (int)layout_.CharPosFromPoint(pt);
    if (nSel != -1)
    {
        MoveSelection(nSel, nSel,true);
        bRet = TRUE;
    }
    return bRet;
}

BOOL CTextEditor::MoveSelectionAtNearPoint(POINT pt)
{
    BOOL bRet = FALSE;
    int nSel = (int)layout_.CharPosFromNearPoint(pt);
    if (nSel != -1)
    {
        MoveSelection(nSel, nSel,true);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
// VK_DOWN,VK_UP
//
//----------------------------------------------------------------
BOOL CTextEditor::MoveSelectionUpDown(BOOL bUp, bool bShiftKey )
{
    FRectF rc;

	if ( bUp )
	{
		if (!layout_.RectFromCharPos(ct_->nSelStart_, &rc))
			return FALSE;
	}
	else
	{
		if (!layout_.RectFromCharPos(ct_->nSelEnd_, &rc))
		{
			//MoveSelectionNext();
			return false;
		}
			
	}

    POINT pt;
    pt.x = rc.left;
    if (bUp)
    {
        pt.y = rc.top - ((rc.bottom - rc.top) / 2);
        if (pt.y < 0)
            return FALSE;
    }
    else
    {
        pt.y = rc.bottom + ((rc.bottom - rc.top) / 2);
    }

	UINT nSel = bUp ? ct_->nSelEnd_ : ct_->nSelStart_ ;

	// 文字間の場合
	if ( MoveSelectionAtPoint(pt) ) 
	{				
		if ( bShiftKey )
		{
			UINT nSel2 = bUp ? ct_->nSelStart_ : ct_->nSelEnd_ ;
			MoveSelection(nSel, nSel2,!bUp );
		}
		
		return TRUE; 
	}

	// 行の最後のさらに後ろ側の場合
	if ( MoveSelectionAtNearPoint(pt) )
	{
		if ( bShiftKey )
		{
			UINT nSel2 = bUp ? ct_->nSelStart_ : ct_->nSelEnd_ ;
			MoveSelection(nSel, nSel2,!bUp );
		}
		
		return TRUE; 
	}
	
	return FALSE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey)
{
    BOOL bRet = FALSE;
    UINT nSel,nSel2;

    if (bFirst)
    {
		// when pushed VK_HOME
		ct_->nStartCharPos_ = 0;
		nSel2 = ct_->nSelEnd_;
        nSel = layout_.FineFirstEndCharPosInLine(ct_->nSelStart_, TRUE);
    }
    else
    {
        // when pushed VK_END
		ct_->nStartCharPos_ = 0;
		nSel2 = ct_->nSelStart_;
		nSel = layout_.FineFirstEndCharPosInLine(ct_->nSelEnd_, FALSE);
    }

    if (nSel != (UINT)-1)
    {
		if ( bShiftKey )
		{
			MoveSelection(nSel, nSel2,true);
		}
		else
		{
			MoveSelection(nSel, nSel);
		}

        bRet = TRUE;
    }

	
    return bRet;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::InvalidateRect()
{
	layout_.bRecalc_ = true;
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
	layout_.bRecalc_ = true;

    LONG lOldSelEnd = ct_->nSelEnd_;
    if (!ct_->RemoveText(ct_->nSelStart_, ct_->nSelEnd_ - ct_->nSelStart_))
        return FALSE;

	UINT nrCnt;
    if (!ct_->InsertText(ct_->nSelStart_, psz, lstrlen(psz), nrCnt))
        return FALSE;

    //ct_->nSelStart_ += nrCnt; // lstrlen(psz);
	ct_->nSelStart_ += lstrlen(psz);
    ct_->nSelEnd_ = ct_->nSelStart_;

    
	LONG acs = ct_->nSelStart_;
	LONG ecs = ct_->nSelEnd_;
	pTextStore_->OnTextChange(acs, lOldSelEnd, ecs);



    pTextStore_->OnSelectionChange();
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
	layout_.bRecalc_ = true;

    if (!fBack && (ct_->nSelEnd_ < (int)ct_->GetTextLength()))
    {
        if (!ct_->RemoveText(ct_->nSelEnd_, 1))
            return FALSE;

		
		LONG ecs = ct_->nSelEnd_;

        pTextStore_->OnTextChange(ecs, ecs + 1, ecs);
    }
	 
    if (fBack && (ct_->nSelStart_ > 0))
    {
        if (!ct_->RemoveText(ct_->nSelStart_ - 1, 1))
            return FALSE;

        ct_->nSelStart_--;
        ct_->nSelEnd_ = ct_->nSelStart_;

		LONG acs = ct_->nSelStart_;
        pTextStore_->OnTextChange(acs, acs + 1, acs );
        pTextStore_->OnSelectionChange();
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteSelection()
{
	layout_.bRecalc_ = true;

    ULONG nSelOldEnd = ct_->nSelEnd_;
    ct_->RemoveText(ct_->nSelStart_, ct_->nSelEnd_ - ct_->nSelStart_);

    ct_->nSelEnd_ = ct_->nSelStart_;

	LONG acs = ct_->nSelStart_;

    pTextStore_->OnTextChange(acs, nSelOldEnd, acs);
    pTextStore_->OnSelectionChange();

    return TRUE;
}
 
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
int CTextEditor::CurrentCaretPos()
{
	return (ct_->bSelTrail_ ? ct_->nSelEnd_ : ct_->nSelStart_ );
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::Render(D2DContext& cxt )
{	
	int zCaretPos = CurrentCaretPos(); //(ct_->bSelTrail_ ? ct_->nSelEnd_ : ct_->nSelStart_ );

	if ( layout_.bRecalc_ )
	{
		layout_.Layout(cxt, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_, zCaretPos, ct_->nStartCharPos_, cxt.text);
		layout_.bRecalc_ = false;
	}
		
	D2DMatrix mat( cxt );				
	mat.PushTransform();
	mat.Offset( ct_->offpt_.x, ct_->offpt_.y );

	int selstart = (int)ct_->nSelStart_ - ct_->nStartCharPos_;
	int selend = (int)ct_->nSelEnd_ - ct_->nStartCharPos_;

	layout_.Render(cxt, ct_->rc_, ct_->GetTextBuffer(), ct_->GetTextLength(), selstart, selend,ct_->bSelTrail_,pCompositionRenderInfo_, nCompositionRenderInfo_);

	mat.PopTransform();
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
void CTextEditor::CalcRender(D2DContext& cxt )
{
	int x = 0;
	
	layout_.Layout(cxt, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_,0, x, cxt.text);	

	layout_.bRecalc_ = false;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------



void CTextEditor::SetFocus()
{
    if (pDocumentMgr_)
    {
		g_pThreadMgr->SetFocus(pDocumentMgr_);
    }


}


//----------------------------------------------------------------
//
// 文字エリアの実設定
//
//----------------------------------------------------------------
void CTextEditor::Reset( IBridgeTSFInterface* bi )
{ 	
	bri_ = bi;
	
	if ( bi )
	{
		FRectFBoxModel rc1 = bi->GetClientRectEx();
		FRectF rc = rc1.GetContentRect().ZeroRect();

		ct_->rc_ = rc;

		//ct_->rc_.left = 0;
		//ct_->rc_.top = 0;
		//ct_->rc_.right= rc.right - rc.left; 
		//ct_->rc_.bottom= rc.bottom - rc.top;

		ct_->view_size_.cx = (LONG)(rc.right - rc.left);
		ct_->view_size_.cy = (LONG)(rc.bottom - rc.top);

		
	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::ClearCompositionRenderInfo()
{
    if (pCompositionRenderInfo_)
    {
        LocalFree(pCompositionRenderInfo_);
        pCompositionRenderInfo_ = NULL;
        nCompositionRenderInfo_ = 0;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
    if (pCompositionRenderInfo_)
    {
        void *pvNew = LocalReAlloc(pCompositionRenderInfo_, 
                                   (nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO),
                                   LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)pvNew;
    }
    else
    {
        pCompositionRenderInfo_ = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
                                   (nCompositionRenderInfo_ + 1) * sizeof(COMPOSITIONRENDERINFO));
        if (!pCompositionRenderInfo_)
            return FALSE;
    }
    pCompositionRenderInfo_[nCompositionRenderInfo_].nStart = nStart;
    pCompositionRenderInfo_[nCompositionRenderInfo_].nEnd = nEnd;
    pCompositionRenderInfo_[nCompositionRenderInfo_].da = *pda;
    nCompositionRenderInfo_++;

    return TRUE;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CTextEditorCtrl::SetContainer( CTextContainer* ct, IBridgeTSFInterface* rect_size )
{
	CTextEditor::SetContainer(ct);

	Reset(rect_size);

}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

HWND CTextEditorCtrl::Create(HWND hwndParent)
{
    hWnd_ = hwndParent;
	
	InitTSF(hWnd_);

	//HRESULT hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory_));

	pTextEditSink_->OnChanged_ = std::bind(&CTextEditorCtrl::OnEditChanged, this );// &CTextEditorCtrl::OnChanged;


	static CTextContainer dumy;
	SetContainer(  &dumy , NULL );

    return hWnd_;
}
void CTextEditorCtrl::OnEditChanged()
{
	

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CTextEditorCtrl::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;

    switch (message)
    {
        case WM_PAINT:
               OnPaint(d->cxt_);
		break;
        /*case WM_SETFOCUS:
            focusは別系統で
			this->OnSetFocus(wParam, lParam);
		break;*/
        case WM_KEYDOWN:
            if ( this->OnKeyDown(wParam, lParam) )
				ret = 1;
		break;
        case WM_LBUTTONDOWN:
            this->OnLButtonDown(wParam, lParam);
		break;
        case WM_LBUTTONUP:
            this->OnLButtonUp(wParam, lParam);
		break;
        case WM_MOUSEMOVE:           
            this->OnMouseMove(wParam, lParam);
		break;
        case WM_CHAR:
            // wParam is a character of the result string. 

			bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
			WCHAR wch = (WCHAR)wParam;

			if ( heldControl )
				return 0;
            
			// normal charcter input. not TSF.
			if ( wch >= L' ' ||  (wch == L'\r'&& !ct_->bSingleLine_ )) 
            {				
				if ( wch < 256 )
				{
					WCHAR wc[] = { wch, '\0'};
		            this->InsertAtSelection(wc);
			        InvalidateRect();
				}
				else
				{
					// outof input
					TRACE( L"out of char %c\n", wch );
				}
            }
		break;
	}
    return ret;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnPaint(D2DContext& hdc)
{
    Render(hdc);
}

//----------------------------------------------------------------
// 
//
//
//----------------------------------------------------------------

BOOL CTextEditorCtrl::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    BOOL ret = true; 

	bool heldShift   = (GetKeyState(VK_SHIFT)& 0x80) != 0;

	int nSelStart;
    int nSelEnd;
    switch (0xff & wParam)
    {
        case VK_LEFT:
             if (heldShift)
             {                 				 
				 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();
                 if (nSelStart > 0)
                 {					
					if ( nSelStart == nSelEnd )
						ct_->bSelTrail_ = false;

					if ( ct_->bSelTrail_ )
						MoveSelection(nSelStart, nSelEnd-1, true );
					else
						MoveSelection(nSelStart - 1, nSelEnd, false);											
                 }
             }
             else
             {
                 MoveSelectionPrev();
             }
		break;

        case VK_RIGHT:
             if (heldShift)
             {
                 nSelStart = GetSelectionStart();
                 nSelEnd = GetSelectionEnd();

				 if ( nSelStart == nSelEnd )
						ct_->bSelTrail_ = true;

				if ( ct_->bSelTrail_ )
					MoveSelection(nSelStart, nSelEnd + 1,true);
				else
					MoveSelection(nSelStart+1, nSelEnd,false);
             }
             else
             {
                 MoveSelectionNext();
             }
		break;

        case VK_UP:
             ret = MoveSelectionUpDown(TRUE, heldShift);
		break;

        case VK_DOWN:
             ret = MoveSelectionUpDown(FALSE, heldShift);
		break;

        case VK_HOME:
             ret = MoveSelectionToLineFirstEnd(TRUE, heldShift);
		break;

        case VK_END:
             ret = MoveSelectionToLineFirstEnd(FALSE, heldShift);
		break;

        case VK_DELETE:
             nSelStart = GetSelectionStart();
             nSelEnd = GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 DeleteAtSelection(FALSE);
             }
             else
             {
                 DeleteSelection();
             }
             
		break;

        case VK_BACK:
             nSelStart = GetSelectionStart();
             nSelEnd = GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 DeleteAtSelection(TRUE);
             }
             else
             {
                 DeleteSelection();
             }
             
		break;
		case VK_ESCAPE:
			nSelEnd = GetSelectionEnd();
			MoveSelection(nSelEnd, nSelEnd);
		break;
    }

	layout_.bRecalc_ = true;

	return ret;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    //SetFocus();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    _uSelDragStart = (UINT)-1;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);



    if (MoveSelectionAtPoint(pt))
    {
        InvalidateRect();
        _uSelDragStart = GetSelectionStart();
    }
	else
	{
		int end = ct_->nSelEnd_;
		MoveSelection( end, end, true);

	}
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    UINT nSelStart = GetSelectionStart();
    UINT nSelEnd = GetSelectionEnd();
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    if (MoveSelectionAtPoint(pt))
    {
        UINT nNewSelStart = GetSelectionStart();
        UINT nNewSelEnd = GetSelectionEnd();

		auto bl = true;
			if ( nNewSelStart < _uSelDragStart )
				bl = false;

        MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd),bl); // (nNewSelStart>_uSelDragStart));
        InvalidateRect();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        POINT pt;
        pt.x = max(0, GET_X_LPARAM(lParam));
        pt.y = max(0, GET_Y_LPARAM(lParam));

        if (MoveSelectionAtPoint(pt))		
        {
            UINT nNewSelStart = GetSelectionStart();
            UINT nNewSelEnd = GetSelectionEnd();

			auto bl = true;
			if ( nNewSelStart < _uSelDragStart )
				bl = false;

            MoveSelection(min(_uSelDragStart, nNewSelStart), max(_uSelDragStart, nNewSelEnd), bl); // (nNewSelStart>_uSelDragStart));
            InvalidateRect();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// CTextEditSink
//
//////////////////////////////////////////////////////////////////////////////
#include "DisplayAttribute.h"
#pragma region IUnknown
//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}
#pragma endregion
//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------
#define TES_INVALID_COOKIE  ((DWORD)(-1))

CTextEditSink::CTextEditSink(CTextEditor *pEditor)
{
    _cRef = 1;
    _dwEditCookie = TES_INVALID_COOKIE;
    _pEditor = pEditor;
}

//+---------------------------------------------------------------------------
//
// EndEdit
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CDispAttrProps *pDispAttrProps = GetDispAttrProps();
    if (pDispAttrProps)
    {
        IEnumTfRanges *pEnum;
        if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                             pDispAttrProps->GetPropTablePointer(),
                                                             pDispAttrProps->Count(),
                                                             &pEnum)) && pEnum)
        {
            ITfRange *pRange;
            if (pEnum->Next(1, &pRange, NULL) == S_OK)
            {
                // We check if there is a range to be changed.
                pRange->Release();

                _pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                ITfRange *pRangeEntire = NULL;
                ITfRange *pRangeEnd = NULL;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    IEnumTfRanges *pEnumRanges;
                    ITfReadOnlyProperty *pProp = NULL;

                    GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire)))
                    {
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE da;
                            TfGuidAtom guid;
                            if (GetDisplayAttributeData(ecReadOnly, pProp, pRange, &da, &guid) == S_OK)
                            {
                                ITfRangeACP *pRangeACP;
                                if (pRange->QueryInterface(IID_ITfRangeACP, (void **)&pRangeACP) == S_OK)
                                {
                                    LONG nStart;
                                    LONG nEnd;
                                    pRangeACP->GetExtent(&nStart, &nEnd);
                                    
									
									_pEditor->AddCompositionRenderInfo(nStart, nStart + nEnd, &da);


                                    pRangeACP->Release();
                                }
                            }
                        }
                    }
                }

                if (pRangeEntire)
                    pRangeEntire->Release();
                if (pRangeEnd)
                    pRangeEnd->Release();
 
            }
            pEnum->Release();
        }

        delete pDispAttrProps;
    }

	if ( OnChanged_ )
		OnChanged_();

	

    return S_OK;
}

#pragma region Advise_Unadvice
//+---------------------------------------------------------------------------
//
// CTextEditSink::Advise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwEditCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    if (source)
        source->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// CTextEditSink::Unadvise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        goto Exit;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    if (source)
        source->Release();

    if (_pic)
    {
        _pic->Release();
        _pic = NULL;
    }

    return hr;
}
#pragma endregion