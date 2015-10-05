#include "stdafx.h"
#include "D2DTestJS.h"
#include "D2DWin.h"
#include "JsrtWrapper.h"

using namespace V4;

extern std::map<wstring, D2Ctrl> wmap;
extern std::map<wstring, D2Ctrls> wbase;

static std::map<wstring, D2Ctrl> jswmap;

static bool GetControl(wstring guid, D2Ctrl& d2)
{
	auto it = jswmap.find(guid);

	if (it == jswmap.end())
		return false;

	d2 = it->second;
	return true;
}
static bool GetControls(wstring guid, D2Ctrls& d2)
{
	auto it = wbase.find(guid);

	if (it == wbase.end())
		return false;

	d2 = it->second;
	return true;
}

static void echo()
{
	auto global = JSContext::Global();

	static function1<number, object> f1;

	f1.callee = [](object obj)->number {

		wstring s;
		string::to_native(obj.Handle(), s);

		auto ct = wmap[L"output"];

		DDSetText( ct, s.c_str());

		

		return number::create(0);
	};

	f1.Regist(global, L"echo");
}

static HRESULT InvokeGetProperty(IDispatch* disp, DISPID id, VARIANT& val)
{
	EXCEPINFO ex; UINT er; DISPPARAMS pm = { NULL,0,0,0 };
	return disp->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &pm, &val, &ex, &er);
}
static HRESULT InvokeGetDISPID(IDispatch* disp, LPCWSTR nm, DISPID& id)
{
	LPOLESTR nma[1]; nma[0] = (LPOLESTR) nm;

	return disp->GetIDsOfNames(IID_NULL, nma, 1, LOCALE_USER_DEFAULT, &id);
}

static HRESULT InvokeGetProperty(IDispatch* disp, LPCWSTR nm, VARIANT& val)
{
	DISPID id;
	HRESULT hr = InvokeGetDISPID(disp, nm, id);
	if (SUCCEEDED(hr))
		hr = InvokeGetProperty(disp, id, val);
	return hr;
}

static int GetJsValueRefMap(object prms,  std::map<wstring,JsValueRef>& m )
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


static void createbutton()
{
	auto global = JSContext::Global();

	static function4<string, number, number, number, number> f1;

	f1.callee = [](number x1, number y1, number w1, number h1)->string {

		auto ct = wbase[L"page0"];

		float x = (float) x1.data();
		float y = (float) y1.data();
		float w = max(0, (float) w1.data());
		float h = max(0, (float) h1.data());

		auto d2 = DDMkButton(ct, FRectFBM(x, y, FSizeF(w, h)), L"NOTDEF");

		FString guid = FString::NewGuid();

		jswmap[guid] = d2;


		return string::create(guid);
	};

	f1.Regist(global, L"createbutton");
}


static void ConvertValue(const std::map<wstring, JsValueRef>& m, std::map<wstring, variant>& ar)
{
	auto it = ar.begin();
	while( it != ar.end())
	{
		auto tit = m.find( it->first );

		if ( tit != m.end())
		{
			variant v = it->second; // default value
			JsValueRef r = tit->second;

			if ( v.vt == VT_I4 || v.vt == VT_R8 || v.vt == VT_INT)
			{
				double val;				
				number::to_native(r, val );
				it->second = val;
			}
			else if ( v.vt == VT_BSTR )
			{
				wstring val;
				string::to_native( r, val );
				it->second = variant(val.c_str());
			}
		}
		it++;
	}
}

static float Float(VARIANT v)
{
	variant r;
	::VariantChangeType( &r, &v,0, VT_R4 );	
	return r.fltVal;
}

static void createwindow()
{
	auto global = JSContext::Global();

	static function1<string, object> f1;

	f1.callee = [](object prms)->string {

		auto ct = wbase[L"page0"];
		std::map<wstring,variant> def;
		def[L"x"] = 0;
		def[L"y"] = 0;
		def[L"w"] = 100;
		def[L"h"] = 100;
		def[L"name"] = L"title";
		def[L"parent"] = L"";

		std::map<wstring, JsValueRef> m;
		
		GetJsValueRefMap(prms, m);

		

		ConvertValue( m, def ); //attach prms value to def value.

		FRectFBM rc;
		rc.left = Float(def[L"x"]);
		rc.top = Float(def[L"y"]);
		rc.right = rc.left + Float(def[L"w"]);
		rc.bottom = rc.top + Float(def[L"h"]);
		
		auto d2 = DDMkControlsWithTitle( ct, rc, L"",  def[L"name"].bstrVal );
		FString guid = FString::NewGuid();
		wbase[guid] = d2;

		return string::create(guid);
		
	};

	f1.Regist(global, L"createwindow");
}
static void createtextbox()
{
	auto global = JSContext::Global();

	static function1<string, object> f1;

	f1.callee = [](object prms)->string {

		auto ct = wbase[L"page0"];
		std::map<wstring, variant> def;
		def[L"x"] = 0;
		def[L"y"] = 0;
		def[L"w"] = 200;
		def[L"h"] = 26;
		def[L"name"] = L"title";
		def[L"parent"] = L"";

		std::map<wstring, JsValueRef> m;

		GetJsValueRefMap(prms, m);



		ConvertValue(m, def); //attach prms value to def value.

		FRectFBM rc;
		rc.left = Float(def[L"x"]);
		rc.top = Float(def[L"y"]);
		rc.right = rc.left + Float(def[L"w"]);
		rc.bottom = rc.top + Float(def[L"h"]);

		D2Ctrls parent;
		if (GetControls(def[L"parent"].bstrVal, parent))
		{
			ct = parent;
		}
				
		auto d2 = DDMkTextbox(ct, rc, 0, def[L"name"].bstrVal);

		FString guid = FString::NewGuid();

		jswmap[guid] = d2;
		
		return string::create(guid);
	};

	f1.Regist(global, L"createtextbox");
}
static void settext()
{
	auto global = JSContext::Global();

	static function2<number, string, string> f;

	f.callee = [](string ctrl, string str)->number {
		
		D2Ctrl d2;
		if ( GetControl( ctrl.data(), d2 ) )
		{
			wstring s = str.data();		
			DDSetText( d2, s.c_str() );
		}
		return number::create(0);
	};

	f.Regist(global, L"settext");
}
static void destroy()
{
	auto global = JSContext::Global();

	static function1<number, string> f;

	f.callee = [](string ctrl)->number {
		
		D2Ctrl d2;
		if (GetControl(ctrl.data(), d2))
		{			
			DDDestroyControl(d2);
		}
		return number::create(0);
	};

	f.Regist(global, L"destroy");
}
static void hide()
{
	auto global = JSContext::Global();

	static function1<number, string> f;

	f.callee = [](string ctrl)->number {

		D2Ctrl d2;
		if (GetControl(ctrl.data(), d2))
		{
			DDShow(d2,false);
		}		
		return number::create(0);
	};

	f.Regist(global, L"hide");
}
static void show()
{
	auto global = JSContext::Global();

	static function1<number, string> f;

	f.callee = [](string ctrl)->number {

		D2Ctrl d2;
		if (GetControl(ctrl.data(), d2))
		{
			DDShow(d2, true);
		}
		return number::create(0);
	};

	f.Regist(global, L"show");
}
static void move()
{
	auto global = JSContext::Global();

	static function3<number, string,number,number> f;

	f.callee = [](string ctrl,number,number)->number {

		D2Ctrl d2;
		if (GetControl(ctrl.data(), d2))
		{
			//DDDestroyControl(d2);
		}


		return number::create(0);
	};

	f.Regist(global, L"move");
}

HRESULT InvokeMethod0(IDispatch* disp, DISPID id, VARIANT& ret)
{
	EXCEPINFO ex; UINT er; DISPPARAMS pm = { NULL,0,0,0 };
	return disp->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &pm, &ret, &ex, &er);
}

static void click()
{
	auto global = JSContext::Global();
	static function2<number, string, object> f;

	f.callee = [](string ctrl, object jsfunc)->number {

		D2Ctrl d2;
		if (GetControl(ctrl.data(), d2))
		{			
			variant v;
			JsValueToVariant(jsfunc.Handle(), &v);
			
			if ( v.vt == VT_DISPATCH )
			{
				IDispatch* pdisp = v.pdispVal;
				pdisp->AddRef();
																						
				DDEvent0(D2EVENT0_MODE::CLICK, d2, [pdisp](D2EVENT0_MODE ev, D2Ctrl x){
					variant ret; 
					InvokeMethod0(pdisp, 0, ret );						
				});
			}
		}
		return number::create(0);
	};

	f.Regist(global, L"click");
}


void EportFunctions()
{
	echo();


	createtextbox();
	createbutton();
	createwindow();
	
	settext();	
	destroy();
	hide();
	show();
	move();

	// EventHandler
	click();
	

}