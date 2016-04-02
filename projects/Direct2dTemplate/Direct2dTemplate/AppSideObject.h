#pragma once


#include "appside_h.h"
#include "IDispatchImpl2.h"


class __declspec(uuid("{9A4E4CD1-C9F6-490D-9A90-3F783E20DCA6}")) CHelper
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CHelper, &__uuidof(CHelper)>
	, public IDispatchImpl2<IObjectAppHelper>
{
public:

	BEGIN_COM_MAP(CHelper)
		COM_INTERFACE_ENTRY(IObjectAppHelper)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP( )

	DECLARE_NO_REGISTRY()
	DECLARE_CLASSFACTORY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	STDMETHODIMP Hello();
	STDMETHODIMP put_EventHandler(VARIANT v );
	STDMETHODIMP EventTest();
	STDMETHODIMP jsinfo(VARIANT text );

	STDMETHODIMP GetCurrentDir(VARIANT* text );
	STDMETHODIMP GetTempDir( VARIANT* text );
	STDMETHODIMP GetExecName(VARIANT* text );
	STDMETHODIMP LoadJs( BSTR fnm );

	STDMETHODIMP ErrorMsg( BSTR message );
	STDMETHODIMP CreateControl( BSTR type, VARIANT prm, VARIANT* ret );

	CComVariant eventhandler_;
};

class __declspec(uuid("{62EBFAF7-97BB-4531-BD22-BE435FBE7601}")) CD2DControlHelper
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CD2DControlHelper, &__uuidof(CD2DControlHelper)>
	, public IDispatchImpl2<ID2DControlHelper>
{
public:
	
	CD2DControlHelper():c_(0){}
	BEGIN_COM_MAP(CD2DControlHelper)
		COM_INTERFACE_ENTRY(ID2DControlHelper)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP( )

	DECLARE_NO_REGISTRY()
	DECLARE_CLASSFACTORY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	STDMETHODIMP Hello();
	STDMETHODIMP put_Text( BSTR type );
	STDMETHODIMP put_onclick(VARIANT func );

	void* c_;
	CComVariant eventhandler_;
};





class __declspec(uuid("{3E46DC0A-DC9E-433c-89AB-9E271E1DBA0A}")) BrokerAppServer
	: public ATL::CAtlExeModuleT<BrokerAppServer>
{
public:
	DECLARE_LIBID(__uuidof(BrokerAppServer))

};


static BrokerAppServer __m;
template <class T> CComPtr<ITypeInfo> IDispatchImpl2<T>::pTypeInfo_;