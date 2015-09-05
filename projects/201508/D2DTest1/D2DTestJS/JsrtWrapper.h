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

#pragma once

// Copyright 2015 furuya
// This original source is a jsrt-wappers.h MSFT.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


typedef _variant_t variant;
typedef std::wstring wstring;

#define ERRORCHECK(x)	JSRuntime::ErrorCode(x)

namespace V4
{
	class JSContext;
	class JSRuntime
	{
		public :
			JSRuntime():handle_(nullptr){}
			explicit JSRuntime(JsRuntimeHandle runtime):handle_(runtime){}
			
			JsRuntimeHandle Handle(){ return handle_; }
			bool IsValid(){ return handle_ != JS_INVALID_RUNTIME_HANDLE; }
			JSContext CreateContext();
			void Dispose();
			void GavageCollect();

			static void ErrorCode(JsErrorCode ec );
			static JSRuntime CreateRuntime();
			
			

		private :
			JsRuntimeHandle handle_;
			
			
	};


	class JSReference
	{
		protected :
			explicit JSReference(JsRef ref ):ref_(ref){}
			JSReference():ref_(JS_INVALID_REFERENCE){}
		public :
			JsRef Handle() const { return ref_; }
			bool IsValid() const { return ref_ != JS_INVALID_REFERENCE; }

			UINT AddRef();
			UINT Releaes();

		protected:
			JsRef ref_;
	};

	class JSContext : public JSReference
	{
		friend class JSRuntime;
		private :
			explicit JSContext( JsContextRef cxt ):JSReference(cxt){}
		public :
			JSContext():JSReference(){}
			
			class Scope
			{
				public :
					Scope(JSContext cxt )
					{
						JsGetCurrentContext(&previousContext);
						JsSetCurrentContext(cxt.ref_);
					}
					~Scope(){JsSetCurrentContext(previousContext);}
				private :
					JsContextRef previousContext;
			};

			
			static JsValueRef GetAndClearException();
			static JSContext Current();
			static JsValueRef Global();

			void Run( LPCWSTR src );
			
			
	};

	class ExceptionScriptRun
	{
		public :
			ExceptionScriptRun(JsValueRef ref);
			
			variant message_;
	};
	class ExceptionScriptCompile
	{
		public :
			ExceptionScriptCompile(JsValueRef ref);

			variant message_;
			variant line_;
			variant column_;
			variant length_;
			variant source_;
	};

	//////////////////////////////////////////////////////////////////////////////
	class property_id : public JSReference
	{
		public :
			property_id(){}
			property_id(JsPropertyIdRef propertyId) :
				JSReference(propertyId){}

			wstring name() const
			{
				const wchar_t *result;
				JsGetPropertyNameFromId(ref_, &result);
				return result;
			}
			
			static property_id create(const wstring& name)
			{
				JsPropertyIdRef propertyId;
				JsGetPropertyIdFromName(name.c_str(), &propertyId);
				return property_id(propertyId);
			}
	};


	//////////////////////////////////////////////////////////////////////////////
	class value : public JSReference
	{
		public :
			explicit value(JsRef r):JSReference(r){}
			value(){}

			JsValueType type() const
			{
				JsValueType type;
				JsGetValueType(ref_, &type);
				return type;
			}


			template<class T>
			static void to_native(JsValueRef r, T& t)
			{
				t = T(object(r));
				
			}

			template<class T>
			static void from_native( const T& t, JsValueRef* r ) {}

			template<>
			static void to_native(JsValueRef r, double& ret)
			{															
				ERRORCHECK(JsNumberToDouble(r, &ret));
			}
			template<>
			static void from_native( const double& value, JsValueRef* ret )
			{
				ERRORCHECK(JsDoubleToNumber(value, ret));
			}
			template<>
			static void to_native(JsValueRef r, bool& ret)
			{
				ERRORCHECK(JsBooleanToBool(r, &ret));
			}
			template<>
			static void from_native(const bool& value, JsValueRef* ret)
			{
				ERRORCHECK(JsBoolToBoolean(value, ret));
			}


			template<>
			static void to_native<wstring>(JsValueRef r, wstring& t)
			{
				JsValueRef stringValue;
				JsConvertValueToString(r, &stringValue);
				const wchar_t *result; 
				size_t length;
				ERRORCHECK(JsStringToPointer(stringValue, &result, &length));
				t = result;
			}
				
			template<>
			static void from_native( const wstring& value, JsValueRef* ret )
			{
				if (value.empty())
				{
					ERRORCHECK(JsGetNullValue(ret));
					return;
				}
				ERRORCHECK(JsPointerToString(value.c_str(), value.length(), ret));
			}

	};
	class string : public value
	{
		
		public:
			string() {}
			explicit string(JsValueRef r) :value(r) {}

			wstring data()
			{
				wstring r;
				to_native(ref_, r);
				return r;
			}
			static string create(wstring bl)
			{
				JsValueRef r;
				from_native(bl, &r);
				return string(r);
			} 
			


			
	};
	class number : public value
	{		
		public:
			explicit number(JsValueRef r) :value(r) {}
			number() {}

			double data()
			{
				double r;
				to_native(ref_, r);
				return r;
			}
			static number create(double bl)
			{
				JsValueRef r;
				from_native(bl, &r);
				return number(r);
			}
	};
	class boolean : public value
	{
		private:
			explicit boolean(JsValueRef r) :value(r) {}
		public :
			boolean(){}
			
			bool data()
			{
				bool r;
				to_native(ref_, r );
				return r;
			}
			static boolean create( bool bl )
			{
				JsValueRef r;
				from_native( bl, &r );
				return boolean(r);
			}

	};
	class object : public value
	{
		private:
			
		public :
			explicit object(JsValueRef r) :value(r) {}
			explicit object(value r):value(r) {}
		public:
			object() {}

			
			static object create()
			{
				JsValueRef r;
				JsCreateObject(&r);
				return object(r);
			}

			template<class T>
			T get_property_native(property_id name)
			{
				value v = get_property(name);

				T returnValue;
				to_native<T>( v.Handle(), returnValue);

				return returnValue;
			}
			
			value get_property(property_id name)
			{
				JsValueRef value1;
				JsGetProperty(Handle(), name.Handle(), &value1);
				return value(value1);
			}

	};


	/////////////////////////////////////////////////////////////////////////
	template<typename R, typename P1, typename P2>
	class function2
	{
	public:
		function2<R, P1, P2>() {}

		JsErrorCode Regist(JsValueRef globalObject, LPCWSTR callbackName)
		{
			if (callee)
			{
				JsPropertyIdRef propertyId;
				(JsGetPropertyIdFromName(callbackName, &propertyId));
				JsValueRef function = create(this);
				JsSetProperty(globalObject, propertyId, function, true);
				return JsNoError;
			}
			return JsErrorFatal;
		}


		std::function<R(P1, P2)> callee;

	private:
		R operator()(P1 p1, P2 p2)
		{
			return callee(p1, p2);
		}

		static JsValueRef CALLBACK thunk(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
		{
			if (argumentCount < 3)
				return JS_INVALID_REFERENCE;

			function2<R, P1, P2>& func = *(function2<R, P1, P2>*)callbackState;

			P1 obj1(arguments[1]);
			P2 obj2(arguments[2]);



			R robj = func(obj1,obj2 );

			return robj.Handle();
		}

		static JsValueRef create(void* function)
		{
			JsValueRef ref;
			JsCreateFunction(thunk, function, &ref);
			return ref;
		}
	};

	/////////////////////////////////////
	template<typename R, typename P1>
	class function1
	{
	public:
		function1<R, P1>() {}

		JsErrorCode Regist(JsValueRef globalObject, LPCWSTR callbackName)
		{
			if (callee)
			{
				JsPropertyIdRef propertyId;
				(JsGetPropertyIdFromName(callbackName, &propertyId));
				JsValueRef function = create(this);
				JsSetProperty(globalObject, propertyId, function, true);
				return JsNoError;
			}
			return JsErrorFatal;
		}


		std::function<R(P1)> callee;

	private:
		R operator()(P1 p1)
		{
			return callee(p1);
		}

		static JsValueRef CALLBACK thunk(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
		{
			if (argumentCount < 2)
				return JS_INVALID_REFERENCE;

			function1<R, P1>& func = *(function1<R, P1>*)callbackState;

			P1 obj1(arguments[1]);

			R robj = func(obj1);

			return robj.Handle();
		}

		static JsValueRef create(void* function)
		{
			JsValueRef ref;
			JsCreateFunction(thunk, function, &ref);
			return ref;
		}
	};
	/////////////////////////////////////////////////////////////////////////
	template<typename R, typename P1, typename P2, typename P3>
	class function3
	{
	public:
		function3<R, P1, P2,P3>() {}

		JsErrorCode Regist(JsValueRef globalObject, LPCWSTR callbackName)
		{
			if (callee)
			{
				JsPropertyIdRef propertyId;
				(JsGetPropertyIdFromName(callbackName, &propertyId));
				JsValueRef function = create(this);
				JsSetProperty(globalObject, propertyId, function, true);
				return JsNoError;
			}
			return JsErrorFatal;
		}


		std::function<R(P1, P2,P3)> callee;

	private:
		R operator()(P1 p1, P2 p2,P3 p3)
		{
			return callee(p1, p2, p3);
		}

		static JsValueRef CALLBACK thunk(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
		{
			if (argumentCount < 3)
				return JS_INVALID_REFERENCE;

			function3<R, P1, P2, P3>& func = *(function3<R, P1, P2,P3>*)callbackState;

			P1 obj1(arguments[1]);
			P2 obj2(arguments[2]);
			P3 obj3(arguments[3]);
			R robj = func(obj1, obj2, obj3);

			return robj.Handle();
		}

		static JsValueRef create(void* function)
		{
			JsValueRef ref;
			JsCreateFunction(thunk, function, &ref);
			return ref;
		}
	};
	/////////////////////////////////////////////////////////////////////////
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	class function4
	{
	public:
		function4<R, P1, P2, P3,P4>() {}

		JsErrorCode Regist(JsValueRef globalObject, LPCWSTR callbackName)
		{
			if (callee)
			{
				JsPropertyIdRef propertyId;
				(JsGetPropertyIdFromName(callbackName, &propertyId));
				JsValueRef function = create(this);
				JsSetProperty(globalObject, propertyId, function, true);
				return JsNoError;
			}
			return JsErrorFatal;
		}


		std::function<R(P1, P2, P3, P4)> callee;

	private:
		R operator()(P1 p1, P2 p2, P3 p3, P4 p4)
		{
			return callee(p1, p2, p3, p4);
		}

		static JsValueRef CALLBACK thunk(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
		{
			if (argumentCount < 4)
				return JS_INVALID_REFERENCE;

			function4<R, P1, P2, P3, P4>& func = *(function4<R, P1, P2, P3, P4>*)callbackState;

			P1 obj1(arguments[1]);
			P2 obj2(arguments[2]);
			P3 obj3(arguments[3]);
			P4 obj4(arguments[4]);
			R robj = func(obj1, obj2, obj3, obj4);

			return robj.Handle();
		}

		static JsValueRef create(void* function)
		{
			JsValueRef ref;
			JsCreateFunction(thunk, function, &ref);
			return ref;
		}
	};
};

