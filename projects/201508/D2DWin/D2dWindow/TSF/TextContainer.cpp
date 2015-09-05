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

#include "stdafx.h"
#include "TextContainer.h"
using namespace TSF;
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
CTextContainer::CTextContainer() 
{
    psz_ = NULL;
    nBufferSize_ = 0;
    nTextSize_ = 0;
	nSelStart_ = nSelEnd_ = 0;
	LimitCharCnt_ = 65500;
	bSingleLine_ = true;
	view_size_ = {0,0};
	nStartCharPos_ = 0;

}
CTextContainer::~CTextContainer() 
{
	Clear();
}	
void CTextContainer::Clear()
{
	delete [] psz_;
	psz_ = NULL;
	nBufferSize_ = 0;
	nTextSize_ = 0;
}
BOOL CTextContainer::InsertText(int nPos, const WCHAR* psz, UINT nCnt, UINT& nResultCnt)
{
	xassert( 0 <= nCnt );

	nResultCnt = 0;

	if ( bSingleLine_ )
	{
		for( UINT i=0; i < nCnt; i++ )
		{
			if ( psz[i] == L'\r' || psz[i] == L'\n')
			{
				nCnt = i; // 単行なので\r\nはNG
				break;
			}
		}
	}

	if ( LimitCharCnt_ < GetTextLength() + nCnt )
	{
		nCnt = LimitCharCnt_ - GetTextLength();
	}
	else if ( nCnt == 0 && nPos == 0 )
	{
		EnsureBuffer(1);
		psz_[0] = 0;
		return TRUE;
	}
	
	if (!EnsureBuffer(nTextSize_ + nCnt))
    {
        return FALSE;
    }

	
    memmove(psz_ + nPos + nCnt, psz_ + nPos, (nTextSize_ - nPos) * sizeof(WCHAR));
    memcpy(psz_ + nPos, psz, nCnt * sizeof(WCHAR));
    nTextSize_ += nCnt;

	psz_[nTextSize_] = 0;

	nResultCnt = nCnt;
 
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::RemoveText(int nPos, UINT nCnt)
{
    if (!nCnt)
        return TRUE;

    if (nPos + nCnt - 1 > nTextSize_)
        nCnt = nTextSize_ - nPos;

    memmove(psz_ + nPos, psz_ + nPos + nCnt, (nTextSize_ - nPos - nCnt) * sizeof(WCHAR));
	nTextSize_ -= nCnt;

	psz_[nTextSize_] = 0;

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::GetText(int nPos, WCHAR *psz, UINT nCnt)
{
    if (!nCnt)
        return FALSE;

    if (nPos + nCnt - 1 > nTextSize_)
        nCnt = nTextSize_ - nPos;
   
    memcpy(psz, psz_ + nPos, nCnt * sizeof(WCHAR));

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::EnsureBuffer(UINT nNewTextSize)
{
    if (!nNewTextSize)
    {
	    Clear();
        return FALSE;
    }

    if (nNewTextSize+1 <= nBufferSize_ )
        goto Exit;

	if ( nNewTextSize < 64 )
		nNewTextSize = max( 8,(int)(nNewTextSize * 2 )+1);
	else
		nNewTextSize = (int)(nNewTextSize * 1.5f)+1;

    if (psz_)
    { 
        WCHAR* pvNew = new WCHAR[nNewTextSize];

		if (!pvNew)
            return FALSE;

		// ZeroMemory( pvNew, nNewTextSize * sizeof(WCHAR));
		memcpy( pvNew, psz_, nBufferSize_ * sizeof(WCHAR));

		delete [] psz_;

		psz_ = pvNew;
    }
    else
    {
		psz_ = new WCHAR[nNewTextSize];

		if (!psz_)
            return FALSE;
		
		// ZeroMemory( psz_, nNewTextSize * sizeof(WCHAR));        
    }
    nBufferSize_ = nNewTextSize;
Exit:
    return TRUE;
}

void CTextContainer::CaretLast()
{
	nSelStart_ = nSelEnd_ = GetTextLength();

}
