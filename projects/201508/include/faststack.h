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

namespace V4 {

template <typename T>
class faststack
{
	public : 
		faststack( int maxcnt )
		{
			head_ = new T[maxcnt];
			ZeroMemory( head_, sizeof(T)*maxcnt );
			id_ = -1;
			maxcnt_ = maxcnt;
		}
		~faststack()
		{
			delete [] head_;
		}
		void push( T t )
		{
			_ASSERT( id_+1 < maxcnt_ );
			head_[++id_] = t;
		}
		T top()
		{
			_ASSERT( id_ > -1 );
			return head_[id_];
		}
		bool empty()
		{
			return ( id_ < 0 );
		}
		void pop()
		{
			head_[id_] = NULL;
			--id_;

			_ASSERT( id_ >= -1 );
		}
		bool include( T t )
		{
			if ( empty()) 
				return false;

			for (int i = 0; i < size(); i++)
			{
				if ( t == head_[i] )
					return true;
			}
			return false;
		}

		int size(){ return id_+1; }
		T* head(){ return head_; }

	protected :
		T* head_;
		int id_;
		int maxcnt_;

};

};
