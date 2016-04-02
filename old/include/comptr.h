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

template <typename T>
class ComPTR
{
	public :
		ComPTR():p(nullptr){}
		ComPTR(const ComPTR<T>& lp )
		{
			p = lp.p;
			if (p != NULL)
				p->AddRef();
		}
		ComPTR(T* lp )
		{
			p = lp;
			if (p != NULL)
				p->AddRef();
		}
		~ComPTR()
		{
			if ( p )
				Release();	
		}
		bool CreateInstance(const IID& clsid)
		{					
			return ( S_OK == ::CoCreateInstance( clsid, NULL, CLSCTX_INPROC,  __uuidof(T), (void**)&p ));
		}
		bool CreateInstance( LPCWSTR LPCLSID )
		{			
			CLSID clsid;
			if ( S_OK == CLSIDFromProgID( obj, &LPCLSID ))
			{
				return ( S_OK == ::CoCreateInstance( clsid, NULL, CLSCTX_INPROC,  __uuidof(T), (void**)&p ));
			}
			return false;
		}
		operator T*() const throw(){ return p;}
		T& operator*() const{_ASSERT(p!=NULL);return *p;}
		T* operator->() const throw(){_ASSERT(p!=NULL);	return p;}
		T** operator&() throw()
		{
			_ASSERT(p==NULL);
			return &p;
		}
		T* operator=( const ComPTR<T>& lp )
		{
			if(!(*this==lp))
			{
				ComPTR(lp).Swap(*this);
			}
			return *this;
		}
		T* operator=(T* lp) throw()
		{
			if(*this!=lp)
			{
				ComPTR(lp).Swap(*this);
			}
			return *this;
		}
		bool operator==( const ComPTR& src ) const 
		{			
			return ( p == src.p );
		}
		bool operator==( T* lp ) const 
		{			
			return ( p == lp );
		}

		ULONG Release()
		{
			T* pTemp = p;
			if (pTemp)
			{
				p = NULL;
				return pTemp->Release();
			}
			return 0;
		}
		T* Detach() throw()
		{
			T* pt = p;
			p = NULL;
			return pt;
		}
		void Attach(T* p2) throw()
		{
			if (p)
			{
				ULONG ref = p->Release();
				_ASSERT(ref != 0 || p2 != p);
			}
			p = p2;
		}	
	protected :
		void Swap(ComPTR<T>& other)
		{
			T* pTemp = p;
			p = other.p;
			other.p = pTemp;
		}	
	public :
		T* p;
};
