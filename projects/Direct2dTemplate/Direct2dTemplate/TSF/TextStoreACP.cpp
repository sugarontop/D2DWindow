#include "stdafx.h"
#include "tsf\TextStoreACP.h"
#include "tsf\TextEditor.h"
#include "initguid.h"
#include "InputScope.h"
#include "tsattrs.h"
using namespace TSF;


#define Round(x)	((LONG)(x+0.5f))

CTextStore::CTextStore(CTextEditor *pEditor):_pEditor(pEditor)
{
	_cRef = 1;
	TextStoreACPSink_ = NULL;
	_nAttrVals = NULL;

	m_fLocked =FALSE;
	m_dwLockType = 0;
	m_fPendingLockUpgrade = FALSE;


}
#pragma region IUnknown
//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextStore::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITextStoreACP))
    {
        *ppvObj = (ITextStoreACP *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextStore::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextStore::Release()
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


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask)
{

    if (!IsEqualGUID(riid, IID_ITextStoreACPSink))
    {
        return TS_E_NOOBJECT;
    }

    if (FAILED(punk->QueryInterface(IID_ITextStoreACPSink, (void **)&TextStoreACPSink_)))
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::UnadviseSink(IUnknown *punk)
{
    // we're dealing with TSF. We don't have to check punk is same instance of TextStoreACPSink_.
    TextStoreACPSink_->Release();
    TextStoreACPSink_ = NULL;

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
{
    if (!TextStoreACPSink_)
    {
        return E_UNEXPECTED;
    }
	if(NULL == phrSession)
    {
        return E_INVALIDARG;
    }

	if ( _pEditor->ct_ == NULL )
		return S_OK;
		

	if(m_fLocked)
    {
        //the document is locked

        if(dwLockFlags & TS_LF_SYNC)
        {
            /*
            The caller wants an immediate lock, but this cannot be granted because 
            the document is already locked.
            */
            *phrSession = TS_E_SYNCHRONOUS;
            return S_OK;
        }
        else    
        {
            //the request is asynchronous 

            /*
            The only type of asynchronous lock request this application 
            supports while the document is locked is to upgrade from a read 
            lock to a read/write lock. This scenario is referred to as a lock 
            upgrade request. 
            */
            if(((m_dwLockType & TS_LF_READWRITE) == TS_LF_READ) && 
                ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE))
            {
                m_fPendingLockUpgrade = TRUE;

                *phrSession = TS_S_ASYNC;

                return S_OK;
            }

        }
        return E_FAIL;
    }

	_LockDocument(dwLockFlags);

    *phrSession = TextStoreACPSink_->OnLockGranted(dwLockFlags);

	_UnlockDocument();

    return S_OK;
}

BOOL CTextStore::_LockDocument(DWORD dwLockFlags)
{
    if(m_fLocked)
    {
        return FALSE;
    }
    
    m_fLocked = TRUE;
    m_dwLockType = dwLockFlags;
    
    return TRUE;
}

void CTextStore::_UnlockDocument()
{
    HRESULT hr;
    
    m_fLocked = FALSE;
    m_dwLockType = 0;
    
    //if there is a pending lock upgrade, grant it
    if(m_fPendingLockUpgrade)
    {
        m_fPendingLockUpgrade = FALSE;

        RequestLock(TS_LF_READWRITE, &hr);
    }

    //if any layout changes occurred during the lock, notify the manager
//    if(m_fLayoutChanged)
//    {
//        m_fLayoutChanged = FALSE;
//        m_AdviseSink.pTextStoreACPSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);
//    }
}
BOOL CTextStore::_IsLocked(DWORD dwLockType) 
{ 
   // if(m_dwInternalLockType)
   // {
   //     return TRUE;
  //  }

    return m_fLocked && (m_dwLockType & dwLockType); 
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetStatus(TS_STATUS *pdcs)
{
    pdcs->dwDynamicFlags = 0;
    pdcs->dwStaticFlags = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::QueryInsert(LONG acpInsertStart, LONG acpInsertEnd, ULONG cch, LONG *pacpResultStart, LONG *pacpResultEnd)
{
    *pacpResultStart = acpInsertStart;
    *pacpResultEnd = acpInsertEnd;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection, ULONG *pcFetched)
{
	//does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }


    *pcFetched = 0;
    if ((ulCount > 0) && ((ulIndex == 0) || (ulIndex == TS_DEFAULT_SELECTION)))
    {
        pSelection[0].acpStart = _pEditor->GetSelectionStart();
        pSelection[0].acpEnd = _pEditor->GetSelectionEnd();
        pSelection[0].style.ase = TS_AE_END;
        pSelection[0].style.fInterimChar = FALSE;
        *pcFetched = 1;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection)
{
    //does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }
	
	if (ulCount > 0)
    { 
        _pEditor->MoveSelection((int)pSelection[0].acpStart, (int)pSelection[0].acpEnd,true);
        _pEditor->InvalidateRect();
    }

    return S_OK;;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetText(LONG acpStart, LONG acpEnd, __out_ecount(cchPlainReq) WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut, TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut, LONG *pacpNext)
{

    //does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }


	if ((cchPlainReq == 0) && (ulRunInfoReq == 0))
    {
        return S_OK;
    }

    if (acpEnd == -1)
        acpEnd = _pEditor->ct_->GetTextLength();

    acpEnd = min(acpEnd, acpStart + (int)cchPlainReq);

    if ((acpStart != acpEnd) &&
        !_pEditor->ct_->GetText(acpStart, pchPlain, acpEnd - acpStart))
    {
        return E_FAIL;
    }

    *pcchPlainOut = acpEnd - acpStart;
    if (ulRunInfoReq)
    {
        prgRunInfo[0].uCount = acpEnd - acpStart;
        prgRunInfo[0].type = TS_RT_PLAIN;
        *pulRunInfoOut = 1;
    }

    *pacpNext = acpEnd;

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, __in_ecount(cch) const WCHAR *pchText, ULONG cch, TS_TEXTCHANGE *pChange)
{
	//does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }


    LONG acpRemovingEnd;

    if (acpStart > (LONG)_pEditor->ct_->GetTextLength())
        return E_INVALIDARG;

    acpRemovingEnd = min(acpEnd, (LONG)_pEditor->ct_->GetTextLength() + 1);
    if (!_pEditor->ct_->RemoveText(acpStart, acpRemovingEnd - acpStart))
        return E_FAIL;

	UINT nrCnt;
    if (!_pEditor->ct_->InsertText(acpStart, pchText, cch, nrCnt))
        return E_FAIL;

    pChange->acpStart = acpStart;
    pChange->acpOldEnd = acpEnd;
    pChange->acpNewEnd = acpStart + cch;

    _pEditor->InvalidateRect();
    return S_OK;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    PrepareAttributes(cFilterAttrs, paFilterAttrs);
    if (!_nAttrVals)
        return S_FALSE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    PrepareAttributes(cFilterAttrs, paFilterAttrs);
    if (!_nAttrVals)
        return S_FALSE;
    return S_OK;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset)
{
    *pacpNext = 0;
    *pfFound = FALSE;
    *plFoundOffset = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched)
{
    *pcFetched = 0;
    for (int i = 0; (i < (int)ulCount) && (i < _nAttrVals) ; i++)
    {
        paAttrVals[i] = _attrval[i];
        *pcFetched++;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetEndACP(LONG *pacp)
{
    //does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }
	if(NULL == pacp)
    {
        return E_INVALIDARG;
    }

	
	*pacp = _pEditor->ct_->GetTextLength();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetActiveView(TsViewCookie *pvcView)
{
    *pvcView = 0;
    return S_OK;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped)
{
//does the caller have a lock
    if(!_IsLocked(TS_LF_READ))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }
	
	FRectF rcStart;
    FRectF rcEnd;

    if (_pEditor->GetLayout()->RectFromCharPos(acpStart, &rcStart) &&
        _pEditor->GetLayout()->RectFromCharPos(acpEnd, &rcEnd))
    {
        if (rcStart.top == rcEnd.top)
        {
            prc->left = Round( min(rcStart.left, rcEnd.left));
            prc->right = Round(max(rcStart.right, rcEnd.right));
        }
        else
        {            
			// 最後の文字が改行位置にくる。
			//::OutputDebugStringEx( L"GetTextExtA %d %d,  %d %d\n",acpStart,acpEnd, rcStart.top, rcEnd.top );

			_pEditor->GetLayout()->RectFromCharPos(acpEnd-1, &rcEnd);

			prc->left = Round(min(rcStart.left, rcEnd.left));
            prc->right = Round(max(rcStart.right, rcEnd.right));
        }
        prc->top = Round(min(rcStart.top, rcEnd.top));
        prc->bottom = Round(max(rcStart.bottom, rcEnd.bottom));
    }
    else
    {
        prc->left = 0;
        prc->right = 0;
        prc->top = 0;
        prc->bottom = 0;
    }



// 候補ダイアログボックスの位置
	FRectF rcClient = _pEditor->bri_->GetClientRectEx().GetContentRect();
	::OffsetRect( prc, Round(rcClient.left), Round(rcClient.top) );


  	D2DMat m( _pEditor->mat_ );

	auto xrc = m.LPtoDP( FRectF(*prc) );
	*prc = xrc.GetRECT();


    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->left);
    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->right);



    *pfClipped = FALSE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetScreenExt(TsViewCookie vcView, RECT *prc)
{
	*prc = _pEditor->bri_->GetClientRect().GetRECT();

	D2DMat m( _pEditor->mat_);
	auto xrc = m.LPtoDP( FRectF(*prc) );
	*prc = xrc.GetRECT();

    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->left);
    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->right);
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetWnd(TsViewCookie vcView, HWND *phwnd)
{
    *phwnd = _pEditor->GetWnd();
    return S_OK;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::InsertTextAtSelection(DWORD dwFlags, __in_ecount(cch) const WCHAR *pchText, ULONG cch, LONG *pacpStart, LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
	//does the caller have a lock
    if(!_IsLocked(TS_LF_READWRITE))
    {
        //the caller doesn't have a lock
        return TS_E_NOLOCK;
    }


    LONG acpStart = _pEditor->GetSelectionStart();
    LONG acpEnd = _pEditor->GetSelectionEnd();

    if (dwFlags & TS_IAS_QUERYONLY)
    {
        *pacpStart = acpStart;
        *pacpEnd = acpStart + cch;
        return S_OK;
    }

    if (!_pEditor->ct_->RemoveText(acpStart, acpEnd - acpStart))
        return E_FAIL;

	UINT nrCnt;
    if (pchText && !_pEditor->ct_->InsertText(acpStart, pchText, cch, nrCnt))
        return E_FAIL;

    
    if (pacpStart)
    {
        *pacpStart = acpStart;
    }

    if (pacpEnd)
    {
        *pacpEnd = acpStart + cch;
    }

    if (pChange)
    {
        pChange->acpStart = acpStart;
        pChange->acpOldEnd = acpEnd;
        pChange->acpNewEnd = acpStart + cch;
    }

    _pEditor->MoveSelection(acpStart, acpStart + cch,true);
    _pEditor->InvalidateRect();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject, LONG *pacpStart, 
        LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
    return E_NOTIMPL;
}

#define IF_ATTR_INPUTSCOPE           1
#define IF_ATTR_FONT_STYLE_HEIGHT    2
#define IF_ATTR_FONT_FACENAME        3
#define IF_ATTR_FONT_SIZEPTS         4
#define IF_ATTR_TEXT_READONLY        5
#define IF_ATTR_TEXT_ORIENTATION     6
#define IF_ATTR_TEXT_VERTICALWRITING 7

static const GUID *__s_rgSupportedAttr[7] = {&GUID_PROP_INPUTSCOPE,
                                    &TSATTRID_Font_Style_Height,
                                    &TSATTRID_Font_FaceName,
                                    &TSATTRID_Font_SizePts,
                                    &TSATTRID_Text_ReadOnly,
                                    &TSATTRID_Text_Orientation,
                                    &TSATTRID_Text_VerticalWriting};

void CTextStore::PrepareAttributes(ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    _nAttrVals= 0;
    memset(_attrval, 0, sizeof(_attrval));

    for (int i = 0; i < ARRAYSIZE(__s_rgSupportedAttr); i++)
    {
        if (cFilterAttrs)
        {
            BOOL fFound = FALSE;
            for (ULONG j = 0; j < cFilterAttrs; j++)
            {
                if (IsEqualGUID(*__s_rgSupportedAttr[i], paFilterAttrs[j]))
                {
                    fFound = TRUE;
                    break;
                }
            }

            if (!fFound)
            {
                continue;
            }
        }

        _attrval[_nAttrVals].idAttr = *__s_rgSupportedAttr[i];
        _attrval[_nAttrVals].dwOverlapId = i + 1;

        switch (i + 1)
        {
            case IF_ATTR_INPUTSCOPE:
                _attrval[_nAttrVals].varValue.vt = VT_UNKNOWN;
                _attrval[_nAttrVals].varValue.punkVal = NULL;
                break;

            case IF_ATTR_FONT_STYLE_HEIGHT:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = _pEditor->GetLineHeight();
                break;

            case IF_ATTR_FONT_FACENAME:
                _attrval[_nAttrVals].varValue.vt = VT_BSTR;
                _attrval[_nAttrVals].varValue.bstrVal = NULL;
                break;

            case IF_ATTR_FONT_SIZEPTS:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = (int)((double)_pEditor->GetLineHeight() / 96.0 * 72.0);
                break;

            case IF_ATTR_TEXT_READONLY:
                _attrval[_nAttrVals].varValue.vt = VT_BOOL;
                _attrval[_nAttrVals].varValue.bVal = FALSE;
                break;

            case IF_ATTR_TEXT_ORIENTATION:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = 0;
                break;

            case IF_ATTR_TEXT_VERTICALWRITING:
                _attrval[_nAttrVals].varValue.vt = VT_BOOL;
                _attrval[_nAttrVals].varValue.bVal = FALSE;
                break;
        }

        _nAttrVals++;
    }

}

void CTextStore::OnSelectionChange()
{
    if (TextStoreACPSink_)
        TextStoreACPSink_->OnSelectionChange();
}

void CTextStore::OnTextChange(LONG acpStart, LONG acpOldEnd, LONG acpNewEnd)
{
    if (TextStoreACPSink_)
    {
        TS_TEXTCHANGE textChange;
        textChange.acpStart = acpStart;
        textChange.acpOldEnd = acpOldEnd;
        textChange.acpNewEnd = acpNewEnd;
        TextStoreACPSink_->OnTextChange(0, &textChange);
    }
}

void CTextStore::OnLayoutChange()
{
    if (TextStoreACPSink_)
    {
        TextStoreACPSink_->OnLayoutChange(TS_LC_CHANGE, 0x01);
    }
}