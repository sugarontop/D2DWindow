#include "stdafx.h"
#include "tsf\TextEditor.h"
#include "tsf\TextStoreACP.h"
#include "D2DWindow.h"


extern ITfThreadMgr *g_pThreadMgr;
extern TfClientId g_TfClientId;

using namespace TSF;

CTextEditor::CTextEditor() 
{
    _pTextStore = NULL;    
	ct_ = NULL;
	_pCompositionRenderInfo = NULL;
    _nCompositionRenderInfo = 0;
         
}

CTextEditor::~CTextEditor() 
{
    UninitTSF();
	
	if (_pTextStore)
    {
        _pTextStore->Release();
        _pTextStore = NULL;
    }

    /*if (_pDocumentMgr)
    {
        _pDocumentMgr->Release();
        _pDocumentMgr = NULL;
    }*/

	
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


    _pTextStore->OnSelectionChange();
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
    _pTextStore->OnSelectionChange();
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
    _pTextStore->OnSelectionChange();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionAtPoint(POINT pt)
{
    BOOL bRet = FALSE;
    int nSel = (int)_layout.CharPosFromPoint(pt);
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
    int nSel = (int)_layout.CharPosFromNearPoint(pt);
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
		if (!_layout.RectFromCharPos(ct_->nSelStart_, &rc))
			return FALSE;
	}
	else
	{
		if (!_layout.RectFromCharPos(ct_->nSelEnd_, &rc))
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
        nSel = _layout.FineFirstEndCharPosInLine(ct_->nSelStart_, TRUE);
    }
    else
    {
        // when pushed VK_END
		ct_->nStartCharPos_ = 0;
		nSel2 = ct_->nSelStart_;
		nSel = _layout.FineFirstEndCharPosInLine(ct_->nSelEnd_, FALSE);
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
	_layout.bRecalc_ = true;
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
	_layout.bRecalc_ = true;

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
	_pTextStore->OnTextChange(acs, lOldSelEnd, ecs);



    _pTextStore->OnSelectionChange();
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
	_layout.bRecalc_ = true;

    if (!fBack && (ct_->nSelEnd_ < (int)ct_->GetTextLength()))
    {
        if (!ct_->RemoveText(ct_->nSelEnd_, 1))
            return FALSE;

		
		LONG ecs = ct_->nSelEnd_;

        _pTextStore->OnTextChange(ecs, ecs + 1, ecs);
    }
	 
    if (fBack && (ct_->nSelStart_ > 0))
    {
        if (!ct_->RemoveText(ct_->nSelStart_ - 1, 1))
            return FALSE;

        ct_->nSelStart_--;
        ct_->nSelEnd_ = ct_->nSelStart_;

		LONG acs = ct_->nSelStart_;
        _pTextStore->OnTextChange(acs, acs + 1, acs );
        _pTextStore->OnSelectionChange();
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
	_layout.bRecalc_ = true;

    ULONG nSelOldEnd = ct_->nSelEnd_;
    ct_->RemoveText(ct_->nSelStart_, ct_->nSelEnd_ - ct_->nSelStart_);

    ct_->nSelEnd_ = ct_->nSelStart_;

	LONG acs = ct_->nSelStart_;

    _pTextStore->OnTextChange(acs, nSelOldEnd, acs);
    _pTextStore->OnSelectionChange();

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

	if ( _layout.bRecalc_ )
	{
		_layout.Layout(cxt, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_, zCaretPos, ct_->nStartCharPos_, cxt.text);
		_layout.bRecalc_ = false;
	}
		
	D2DMatrix mat( cxt );				
	mat.PushTransform();
	mat.Offset( ct_->offpt_.x, ct_->offpt_.y );

	int selstart = (int)ct_->nSelStart_ - ct_->nStartCharPos_;
	int selend = (int)ct_->nSelEnd_ - ct_->nStartCharPos_;

	_layout.Render(cxt, ct_->rc_, ct_->GetTextBuffer(), ct_->GetTextLength(), selstart, selend,ct_->bSelTrail_,_pCompositionRenderInfo, _nCompositionRenderInfo);

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
	
	_layout.Layout(cxt, ct_->GetTextBuffer(), ct_->GetTextLength(), ct_->view_size_, ct_->bSingleLine_,0, x, cxt.text);	

	_layout.bRecalc_ = false;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InitTSF()
{

    _pTextStore = new CTextStore(this);
    if (!_pTextStore)
    {
        return FALSE;
    }

    if (FAILED(g_pThreadMgr->CreateDocumentMgr(&_pDocumentMgr)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->CreateContext(g_TfClientId, 0, _pTextStore, &_pInputContext, &_ecTextStore)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->Push(_pInputContext)))
    {
        return FALSE;
    }

	// Win7のバグ対応コードぽい,(0,0)にIMEが表示されてしまう
    CComPtr<ITfDocumentMgr> pDocumentMgrPrev;
    g_pThreadMgr->AssociateFocus(_hwnd, _pDocumentMgr, &pDocumentMgrPrev);

	 _pTextEditSink = new CTextEditSink(this);
    if (!_pTextEditSink)
    {
        return FALSE;
    }

    _pTextEditSink->_Advise(_pInputContext);


    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::UninitTSF()
{
    if (_pTextEditSink)
    {
        _pTextEditSink->_Unadvise();
        _pTextEditSink->Release();
        _pTextEditSink = NULL;
    }
	
	if (_pDocumentMgr)
    {
        _pDocumentMgr->Pop(TF_POPF_ALL);
		
		// この後releaseされるのでNULLにしない
    }

    /*if (_pInputContext)
    {
        _pInputContext->Release();
        _pInputContext = NULL;
    }*/

    /*if (_pDocumentMgr)
    {
        _pDocumentMgr->Release();
        _pDocumentMgr = NULL;
    }*/

    if (_pTextStore)
    {
        _pTextStore->Release();
        _pTextStore = NULL;
    }

    return TRUE;
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::SetFocusDocumentMgr()
{
    if (_pDocumentMgr)
    {
        // g_pThreadMgr->SetFocus(_pDocumentMgr);
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
    if (_pCompositionRenderInfo)
    {
        LocalFree(_pCompositionRenderInfo);
        _pCompositionRenderInfo = NULL;
        _nCompositionRenderInfo = 0;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
    if (_pCompositionRenderInfo)
    {
        void *pvNew = LocalReAlloc(_pCompositionRenderInfo, 
                                   (_nCompositionRenderInfo + 1) * sizeof(COMPOSITIONRENDERINFO),
                                   LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        _pCompositionRenderInfo = (COMPOSITIONRENDERINFO *)pvNew;
    }
    else
    {
        _pCompositionRenderInfo = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
                                   (_nCompositionRenderInfo + 1) * sizeof(COMPOSITIONRENDERINFO));
        if (!_pCompositionRenderInfo)
            return FALSE;
    }
    _pCompositionRenderInfo[_nCompositionRenderInfo].nStart = nStart;
    _pCompositionRenderInfo[_nCompositionRenderInfo].nEnd = nEnd;
    _pCompositionRenderInfo[_nCompositionRenderInfo].da = *pda;
    _nCompositionRenderInfo++;

    return TRUE;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void CTextEditorCtrl::Move(int x, int y, int nWidth, int nHeight)
{
    if (IsWindow(_hwnd))
        MoveWindow(_hwnd, x, y, nWidth, nHeight, TRUE);
}

void CTextEditorCtrl::SetContainer( CTextContainer* ct, IBridgeTSFInterface* rect_size )
{
	CTextEditor::SetContainer(ct);

	Reset(rect_size);

}
//TSF::CTextContainer* CTextEditor::GetContainer()
//{
//	return ct_;
//}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

HWND CTextEditorCtrl::Create(HWND hwndParent)
{
    _hwnd = hwndParent;
	SetWnd(_hwnd);
	InitTSF();

	//HRESULT hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory_));

	_pTextEditSink->OnChanged_ = std::bind(&CTextEditorCtrl::OnEditChanged, this );// &CTextEditorCtrl::OnChanged;


	static CTextContainer dumy;
	SetContainer(  &dumy , NULL );

    return _hwnd;
}
void CTextEditorCtrl::OnEditChanged()
{
	

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CTextEditorCtrl::WndProcX(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;
    CTextEditorCtrl *ptic = this;
	HWND hwnd = d->hWnd_;

	//if ( _hwnd )
	//	_ASSERT(_hwnd == hwnd );

    switch (message)
    {
        /*case WM_CREATE:
            SetThis(hwnd, ((CREATESTRUCT *)lParam)->lpCreateParams);
            SetTimer(hwnd, TIMERID_CARET, GetCaretBlinkTime(), NULL);
            ptic->OnCreate(hwnd, wParam, lParam);
		break;*/

        //case WM_DESTROY:
        //    //ptic = GetThis(hwnd);
        //    if (ptic)
        //    {
        //        ptic->OnDestroy();
        //    }
        //    break;

        case WM_SETFOCUS:
            //ptic = GetThis(hwnd);
            ptic->OnSetFocus(wParam, lParam);
            break;

        case WM_PAINT:
            
               OnPaint(d->cxt_);
            
            break;

        case WM_KEYDOWN:
            if ( ptic->OnKeyDown(wParam, lParam) )
				ret = 1;
            break;

        case WM_LBUTTONDOWN:
            ptic->OnLButtonDown(wParam, lParam);
            break;

        case WM_LBUTTONUP:
            ptic->OnLButtonUp(wParam, lParam);
            break;

        case WM_MOUSEMOVE:
           
            ptic->OnMouseMove(wParam, lParam);
            break;


        case WM_IME_COMPOSITION:
            if (lParam & GCS_RESULTSTR)
            {
                HIMC himc = ImmGetContext(hwnd);

                if (himc)
                {
                    LONG nSize = ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0);
                    if (nSize)
                    {
                        LPWSTR psz = (LPWSTR)LocalAlloc(LPTR, nSize + sizeof(WCHAR));
                        if (psz)
                        {
                            ImmGetCompositionString(himc, GCS_RESULTSTR, psz, nSize);
                            LocalFree(psz);
                        }
                    }
                }
                ImmReleaseContext(hwnd, himc);

                //
                // If we don't want to receive WM_IME_CHAR or WM_CHAR with
                // this result string, we should not call DefWindowProc()
                // with GCS_RESULTSTR, GCS_RESULTREADSTR, GCS_RESULTCLAUSE and
                // GCS_RESULTREADCLAUSE flags. 
                //
                // lParam &= ~(GCS_RESULTCLAUSE |
                //             GCS_RESULTREADCLAUSE |
                //             GCS_RESULTREADSTR |
                //             GCS_RESULTSTR);
                // if (!lParam)
                //     break;
                //
            }
            return 0;// DefWindowProc(hwnd, message, wParam, lParam);

        case WM_IME_CHAR:
            //
            // wParam is a character of the result string. 
            // if we don't want to receive WM_CHAR message for this character,
            // we should not call DefWindowProc().
            //
            return 0; // DefWindowProc(hwnd, message, wParam, lParam);

        case WM_CHAR:
            //
            // wParam is a character of the result string. 
            //

			bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
			WCHAR wch = (WCHAR)wParam;

			if ( heldControl )
				return 0;
            
			if ( wch >= ' ' ||  (wch == '\r'&& !ct_->bSingleLine_ )) 
            {
				WCHAR wc[] = { wch, '\0'};

                ptic->InsertAtSelection(wc);
                InvalidateRect();
            }
            break;
    
        //default:
        //    return DefWindowProc(hwnd, message, wParam, lParam);
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
	
	_layout.bRecalc_ = true;
	
	return ret;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditorCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    SetFocusDocumentMgr();
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
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
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

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

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
