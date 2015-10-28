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

namespace V4 {

// bin_ = 参照カウンタ(DWORD)+長さ(DWORD)+バイナリデータ(bi_length byte)

class Binary
{
	public :
		Binary():bin_(0){}
		Binary( const BYTE* bi, DWORD bi_length )
		{
			DWORD alclen = 4 + bi_length;
			bin_ = new BYTE[ alclen+sizeof(DWORD)*2];
			
			DWORD cnt = 1;
			memcpy( bin_, &cnt, sizeof(DWORD) );						
			memcpy( bin_+sizeof(DWORD), &bi_length, sizeof(DWORD) );	
			
			if ( bi )					
				memcpy( bin_+sizeof(DWORD)*2, bi, bi_length );		
			else
				memset( bin_+sizeof(DWORD)*2 , 0, bi_length );									
		}
		Binary( const Binary& s ):bin_(0)
		{
			this->operator=(s);
		}
		
		~Binary()
		{
			clear();
		}

		BYTE* get() const
		{								
			return bin_ + sizeof(DWORD)*2;
		}
		DWORD length() const{ return *(DWORD*)(bin_ + sizeof(DWORD)); }

		void clear()
		{
			if ( bin_ )
			{
				DWORD& cnt = *(DWORD*)bin_;
				cnt--;
			
				if ( cnt == 0 )
					delete [] bin_;
			
				bin_ = NULL;
			}
		}

		Binary& operator=(const Binary& src)
		{
			if ( this == &src ) return *this;

			clear();

			if ( src.bin_ == nullptr ) return *this;

			DWORD& cnt = *(DWORD*)src.bin_;
			cnt++;

			bin_ = src.bin_;

			return *this;
		}

		Binary clone() const
		{
			return Binary(get(), length());
		}
		
		Binary cut(DWORD pos, DWORD xlen) const
		{
			_ASSERT( pos + xlen <= length());
			_ASSERT( 0 < xlen );
			return Binary(get()+pos, xlen);
		}
		Binary& shrink(DWORD xlen )
		{
			_ASSERT( xlen < length());
			memcpy( bin_+sizeof(DWORD), &xlen, sizeof(DWORD) );	
			return *this;
		}
		bool isnull() const { return (bin_ == nullptr); }
	private :
		BYTE* bin_;
};

class Stream
{
	public :
		Stream()
		{
			d_ = NULL;
			Clear();			
		}
		~Stream()
		{
			delete [] d_;
		}
		void Clear()
		{
			delete [] d_;

			alloc_len_ = 256;
			pos_ = 0;
			use_len_ = 0;
			d_ = new BYTE[alloc_len_];

			ZeroMemory(d_,alloc_len_);
		}
		void Write( LPVOID p, DWORD sz )
		{
			for( int i = 0; i < 2; i++ )
			{
				if ( use_len_+sz < alloc_len_ )
				{
					CopyMemory( d_+pos_, p, sz );
					pos_ += sz;
					use_len_+= sz;
					break;
				}
				else
				{
					Resize( (DWORD)((alloc_len_ + sz )*1.5)) ;
				}
			}
		}
		DWORD Seek( int typ, long off )
		{
			if ( typ == 0 )
				pos_ = off;
			else if ( typ == 1 )
				pos_ += off;
			else if ( typ == 2 )
				pos_ = use_len_-off;
			
			_ASSERT( pos_ < alloc_len_ );
			_ASSERT( 0 <= pos_ );

			return pos_;
		}

		Binary get() const
		{
			return Binary(d_, use_len_ );
		}
		Binary get_utf8() const
		{
			int charlen = lstrlen((LPCWSTR)d_);
			int cchMultiByte = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)d_, charlen, NULL, 0, NULL, NULL);
			Binary r(0,cchMultiByte);
			int result = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)d_, charlen, (LPSTR)r.get(), cchMultiByte, NULL, NULL);
			_ASSERT( result == cchMultiByte );
			return r;
		}

		std::wstring get_wstring() const 
		{
			std::wstring r;
			r.append( (LPCWSTR)d_, use_len_ );
			return r;
		}

		void Resize( DWORD newlen )
		{
			BYTE* nd = new BYTE[newlen];
			ZeroMemory(nd,newlen);
			CopyMemory( nd, d_, use_len_ );
			delete [] d_;
			d_ = nd;
			alloc_len_ = newlen;
		}
		DWORD length() const { return use_len_; }
		DWORD pos() const { return pos_; }

		void Write( int val ){Write( &val, sizeof(int)); }
		void Write( DWORD val ){Write( &val, sizeof(DWORD)); }
		void Write( LPCWSTR val ){Write( (LPVOID)val, 2*lstrlen(val)); }
		void Write( float val ){Write( &val, sizeof(float)); }
		void Write( std::wstring& val ){Write( (LPVOID)val.c_str(), 2*val.length()); }
		void Write( Binary& b ){ Write(b.get(), b.length()) ; }
		void Write( Stream& sm ){ Write( sm.d_, sm.length() ); }
	private :
		DWORD alloc_len_, use_len_, pos_;
		BYTE* d_;

};


};
