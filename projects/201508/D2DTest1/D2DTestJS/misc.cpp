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
#include "D2DTestJS.h"
#include "D2DWin.h"
#include "JsrtWrapper.h"
#include "misc.h"
using namespace V4;

 void ConvertValue(const std::map<wstring, JsValueRef>& m, std::map<wstring, variant>& ar)
{
	auto it = ar.begin();
	while (it != ar.end())
	{
		auto tit = m.find(it->first);

		if (tit != m.end())
		{
			variant v = it->second; // default value
			JsValueRef r = tit->second;

			if (v.vt == VT_I4 || v.vt == VT_R8 || v.vt == VT_INT)
			{
				double val;
				number::to_native(r, val);
				it->second = val;
			}
			else if (v.vt == VT_BSTR)
			{
				wstring val;
				string::to_native(r, val);
				it->second = variant(val.c_str());
			}
		}
		it++;
	}
}

 float Float(VARIANT v)
{
	variant r;
	::VariantChangeType(&r, &v, 0, VT_R4);
	return r.fltVal;
}

 HRESULT InvokeGetProperty(IDispatch* disp, DISPID id, VARIANT& val)
{
	EXCEPINFO ex; UINT er; DISPPARAMS pm = { NULL,0,0,0 };
	return disp->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &pm, &val, &ex, &er);
}
 HRESULT InvokeGetDISPID(IDispatch* disp, LPCWSTR nm, DISPID& id)
{
	LPOLESTR nma[1]; nma[0] = (LPOLESTR) nm;

	return disp->GetIDsOfNames(IID_NULL, nma, 1, LOCALE_USER_DEFAULT, &id);
}

 HRESULT InvokeGetProperty(IDispatch* disp, LPCWSTR nm, VARIANT& val)
{
	DISPID id;
	HRESULT hr = InvokeGetDISPID(disp, nm, id);
	if (SUCCEEDED(hr))
		hr = InvokeGetProperty(disp, id, val);
	return hr;
}

 int GetJsValueRefMap(object prms,  std::map<wstring,JsValueRef>& m )
{
	variant v1;
	JsValueToVariant(prms.Handle(), &v1);

	if ( v1.vt != VT_DISPATCH )
		return -1;

	
	ComPTR<IDispatchEx> px;
	v1.pdispVal->QueryInterface(&px);

	DISPPARAMS pm = { NULL,0,0,0 };
	DISPID dispid;

	auto hr = px->GetNextDispID(fdexEnumAll, DISPID_STARTENUM, &dispid);

	int cnt = 0;
	while (hr == S_OK)
	{
		DISPPARAMS dispParamsNoArgs = {0};

		// get key
		BSTR key;
		if (S_OK != px->GetMemberName(dispid, &key))
			break;


		// get value
		variant varItem;
		hr= px->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispParamsNoArgs, &varItem, NULL, NULL);

		JsValueRef vref;
		JsVariantToValue(&varItem, &vref);
		m[key] = vref;

		::SysFreeString(key);
		hr = px->GetNextDispID(fdexEnumAll, dispid, &dispid);
		cnt++;
	}
	return cnt;
}
