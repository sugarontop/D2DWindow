/*
The MIT License (MIT)
Copyright (c) 2015 admin@sugarontop.net
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

#include <ctffunc.h>

#define MAX_DISPATTRGUID 10

namespace TSF {

class CDispAttrProps
{
public:
    CDispAttrProps() 
    {
        for (int i = 0;i < MAX_DISPATTRGUID; i++)
        {
             _guid[i] = GUID_NULL;
             _rgpguid[i] = &_guid[i];
        }
        _nCount = 0;
    }

    void Add(REFGUID rguid)
    {
        if (!FindGuid(rguid) && _nCount < MAX_DISPATTRGUID)
        {
            _guid[_nCount] = rguid;
            _nCount++;
        }
        
    }

    BOOL FindGuid(REFGUID rguid)
    {
        int i;
        for (i = 0; i < _nCount; i++)
        {
            if (IsEqualGUID(_guid[i], rguid))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    int Count()
    {
        return _nCount;
    }

    GUID *GetPropTable()
    {
        return _guid;
    }

    const GUID **GetPropTablePointer()
    {
        return _rgpguid;
    }

private:
    GUID _guid[MAX_DISPATTRGUID];
    const GUID* _rgpguid[MAX_DISPATTRGUID];
    int _nCount;
};

CDispAttrProps *GetDispAttrProps();

HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();
HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, CDispAttrProps *pDispAttrProps);
HRESULT GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfClientId *pguid);

HRESULT GetAttributeColor(TF_DA_COLOR *pdac, COLORREF *pcr);
HRESULT SetAttributeColor(TF_DA_COLOR *pdac, COLORREF cr);
HRESULT SetAttributeSysColor(TF_DA_COLOR *pdac, int nIndex);
HRESULT ClearAttributeColor(TF_DA_COLOR *pdac);

};
