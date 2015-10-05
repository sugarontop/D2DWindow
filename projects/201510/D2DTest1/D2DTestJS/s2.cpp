#include "stdafx.h"
#include "s1.h"
#include "JsrtWrapper.h"

//using namespace V4;
//int main()
//{
//	auto r = JSRuntime::CreateRuntime();
//
//	auto cxt = r.CreateContext();
//
//	{
//		JSContext::Scope ct(cxt);
//
//		JsValueRef gl = cxt.Global();
//
//		DefineHostCallback(gl, L"echo", Echo, nullptr);
//
//		try
//		{
//			cxt.Run(L"\n\nvar a = 1+100w; echo(a); ");
//		}
//		catch (ExceptionScriptRun ex1)
//		{
//
//		}
//		catch (ExceptionScriptCompile ex1)
//		{
//			std::wstring a = ex1.message_.bstrVal;
//		}
//	}
//	r.GavageCollect();
//	r.Dispose();
//	return 0;
//}

using namespace V4;

namespace CHAKRA2 {
	void Initialize(JSRuntime& grun, JSContext& rcxt);
	bool JavaScriptRun(JSContext& r, LPCWSTR src);
	void Destroy(JSRuntime);

	wstring ErrorMsg;
};



namespace CHAKRA2 {
void Initialize(JSRuntime& grun, JSContext& rcxt)
{
	auto r = JSRuntime::CreateRuntime();
	
	JSContext cxt = r.CreateContext();
	
	
	JSContext::Scope ct(cxt);

	JsValueRef gl = cxt.Global();

	rcxt = cxt;
	
	grun = r;

	



}
	
bool JavaScriptRun(JSContext& cxt, LPCWSTR src)
{

	try
	{
		//auto cxt = gcxt; // r.CreateContext();
		//JSContext::Scope ct(cxt);

		JsSetCurrentContext(cxt.Handle());

		cxt.Run( src );


		return true;
	}
	catch (ExceptionScriptRun ex1)
	{
		if ( ex1.message_.vt == VT_NULL || ex1.message_.vt == VT_EMPTY )
			ErrorMsg = L"ÉGÉâÅ[î≠ê∂";
		else
			ErrorMsg = ex1.message_.bstrVal;
	}
	catch (ExceptionScriptCompile ex1)
	{
		ErrorMsg = ex1.message_.bstrVal;
	}

	return false;
}

void Destroy(JSRuntime r)
{
	JsSetCurrentContext(0);

	r.GavageCollect();
	r.Dispose();
}

}



