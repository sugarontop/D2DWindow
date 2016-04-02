

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Sat Apr 02 12:10:46 2016
 */
/* Compiler settings for AppSide.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __AppSide_h_h__
#define __AppSide_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IObjectAppHelper_FWD_DEFINED__
#define __IObjectAppHelper_FWD_DEFINED__
typedef interface IObjectAppHelper IObjectAppHelper;

#endif 	/* __IObjectAppHelper_FWD_DEFINED__ */


#ifndef __ID2DControlHelper_FWD_DEFINED__
#define __ID2DControlHelper_FWD_DEFINED__
typedef interface ID2DControlHelper ID2DControlHelper;

#endif 	/* __ID2DControlHelper_FWD_DEFINED__ */


#ifndef __IObjectAppHelper_FWD_DEFINED__
#define __IObjectAppHelper_FWD_DEFINED__
typedef interface IObjectAppHelper IObjectAppHelper;

#endif 	/* __IObjectAppHelper_FWD_DEFINED__ */


#ifndef __ID2DControlHelper_FWD_DEFINED__
#define __ID2DControlHelper_FWD_DEFINED__
typedef interface ID2DControlHelper ID2DControlHelper;

#endif 	/* __ID2DControlHelper_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IObjectAppHelper_INTERFACE_DEFINED__
#define __IObjectAppHelper_INTERFACE_DEFINED__

/* interface IObjectAppHelper */
/* [unique][dual][uuid][object] */ 


EXTERN_C const IID IID_IObjectAppHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5FC01A67-EE06-40DE-A7E7-DEB4D6A0724C")
    IObjectAppHelper : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Hello( void) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_EventHandler( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE EventTest( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE jsinfo( 
            /* [in] */ VARIANT text) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCurrentDir( 
            /* [retval][out] */ VARIANT *text) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetTempDir( 
            /* [retval][out] */ VARIANT *text) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetExecName( 
            /* [retval][out] */ VARIANT *text) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE LoadJs( 
            /* [in] */ BSTR fnm) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ErrorMsg( 
            /* [in] */ BSTR message) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateControl( 
            /* [in] */ BSTR type,
            /* [in] */ VARIANT prm,
            /* [retval][out] */ VARIANT *ret) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IObjectAppHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectAppHelper * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectAppHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectAppHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IObjectAppHelper * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IObjectAppHelper * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IObjectAppHelper * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IObjectAppHelper * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Hello )( 
            IObjectAppHelper * This);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_EventHandler )( 
            IObjectAppHelper * This,
            /* [in] */ VARIANT v);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *EventTest )( 
            IObjectAppHelper * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *jsinfo )( 
            IObjectAppHelper * This,
            /* [in] */ VARIANT text);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCurrentDir )( 
            IObjectAppHelper * This,
            /* [retval][out] */ VARIANT *text);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetTempDir )( 
            IObjectAppHelper * This,
            /* [retval][out] */ VARIANT *text);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetExecName )( 
            IObjectAppHelper * This,
            /* [retval][out] */ VARIANT *text);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *LoadJs )( 
            IObjectAppHelper * This,
            /* [in] */ BSTR fnm);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ErrorMsg )( 
            IObjectAppHelper * This,
            /* [in] */ BSTR message);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateControl )( 
            IObjectAppHelper * This,
            /* [in] */ BSTR type,
            /* [in] */ VARIANT prm,
            /* [retval][out] */ VARIANT *ret);
        
        END_INTERFACE
    } IObjectAppHelperVtbl;

    interface IObjectAppHelper
    {
        CONST_VTBL struct IObjectAppHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectAppHelper_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IObjectAppHelper_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IObjectAppHelper_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IObjectAppHelper_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IObjectAppHelper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IObjectAppHelper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IObjectAppHelper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IObjectAppHelper_Hello(This)	\
    ( (This)->lpVtbl -> Hello(This) ) 

#define IObjectAppHelper_put_EventHandler(This,v)	\
    ( (This)->lpVtbl -> put_EventHandler(This,v) ) 

#define IObjectAppHelper_EventTest(This)	\
    ( (This)->lpVtbl -> EventTest(This) ) 

#define IObjectAppHelper_jsinfo(This,text)	\
    ( (This)->lpVtbl -> jsinfo(This,text) ) 

#define IObjectAppHelper_GetCurrentDir(This,text)	\
    ( (This)->lpVtbl -> GetCurrentDir(This,text) ) 

#define IObjectAppHelper_GetTempDir(This,text)	\
    ( (This)->lpVtbl -> GetTempDir(This,text) ) 

#define IObjectAppHelper_GetExecName(This,text)	\
    ( (This)->lpVtbl -> GetExecName(This,text) ) 

#define IObjectAppHelper_LoadJs(This,fnm)	\
    ( (This)->lpVtbl -> LoadJs(This,fnm) ) 

#define IObjectAppHelper_ErrorMsg(This,message)	\
    ( (This)->lpVtbl -> ErrorMsg(This,message) ) 

#define IObjectAppHelper_CreateControl(This,type,prm,ret)	\
    ( (This)->lpVtbl -> CreateControl(This,type,prm,ret) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IObjectAppHelper_INTERFACE_DEFINED__ */


#ifndef __ID2DControlHelper_INTERFACE_DEFINED__
#define __ID2DControlHelper_INTERFACE_DEFINED__

/* interface ID2DControlHelper */
/* [unique][dual][uuid][object] */ 


EXTERN_C const IID IID_ID2DControlHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2460C99C-FD46-43D7-B559-69BD3B4EA6B9")
    ID2DControlHelper : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Hello( void) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Text( 
            /* [in] */ BSTR type) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_onclick( 
            /* [in] */ VARIANT func) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ID2DControlHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID2DControlHelper * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID2DControlHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID2DControlHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ID2DControlHelper * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ID2DControlHelper * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ID2DControlHelper * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ID2DControlHelper * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Hello )( 
            ID2DControlHelper * This);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Text )( 
            ID2DControlHelper * This,
            /* [in] */ BSTR type);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_onclick )( 
            ID2DControlHelper * This,
            /* [in] */ VARIANT func);
        
        END_INTERFACE
    } ID2DControlHelperVtbl;

    interface ID2DControlHelper
    {
        CONST_VTBL struct ID2DControlHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID2DControlHelper_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID2DControlHelper_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID2DControlHelper_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ID2DControlHelper_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ID2DControlHelper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ID2DControlHelper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ID2DControlHelper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ID2DControlHelper_Hello(This)	\
    ( (This)->lpVtbl -> Hello(This) ) 

#define ID2DControlHelper_put_Text(This,type)	\
    ( (This)->lpVtbl -> put_Text(This,type) ) 

#define ID2DControlHelper_put_onclick(This,func)	\
    ( (This)->lpVtbl -> put_onclick(This,func) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ID2DControlHelper_INTERFACE_DEFINED__ */



#ifndef __BrokerAppServer_LIBRARY_DEFINED__
#define __BrokerAppServer_LIBRARY_DEFINED__

/* library BrokerAppServer */
/* [uuid] */ 




EXTERN_C const IID LIBID_BrokerAppServer;
#endif /* __BrokerAppServer_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


