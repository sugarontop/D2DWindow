#pragma once

namespace V4 {

class StringStream
{
	public :
		StringStream()
		{
			len_ = 256;
			p_ = new WCHAR[ len_ ];
			pos_ = 0;
		}
		~StringStream(){ clear(); }

		StringStream& append( LPCWSTR s, int len = -1 )
		{
			if ( len < 0 ) len = lstrlen(s);

			while( 1 )
			{
				if ( pos_ + len < len_ )
				{
					memcpy( p_+ pos_, s, sizeof(WCHAR)*len );
					pos_ += len;
					break;
				}
				else
				{
					bigger();
				}
			}
			return *this;
		}
		StringStream& append( WCHAR ch )
		{
			int len = 1;

			while( 1 )
			{
				if ( pos_ + len < len_ )
				{
					memcpy( p_+ pos_, &ch, sizeof(WCHAR)*len );
					pos_ += len;
					break;
				}
				else
				{
					bigger();
				}
			}
			return *this;
		}

		StringStream& operator <<( LPCWSTR s )
		{
			append( s, -1 );
			return *this;
		}
		StringStream& operator <<( WCHAR ch )
		{
			append( ch );
			return *this;
		}
		
		StringStream& operator <<( const StringStream& s )
		{
			append( s.p_, s.pos_ );
			return *this;
		}
		
		void bigger()
		{
			int newlen = len_ * 2;
			WCHAR* np = new WCHAR[newlen];
			memcpy( np , p_,  pos_* sizeof(WCHAR));
			delete [] p_;
			p_ = np;
			len_ = newlen;
		}
		int pos() const { return pos_; }
		int seek( int off ){ pos_ += off; return pos_; }
		int seektop(){ pos_ = 0;  return pos_; }
		void clear()
		{
			delete [] p_; p_ = nullptr;
			pos_ = len_ = 0;
		}
		
		BSTR ToString()
		{
			append( L'\0');
			
			return	::SysAllocString(p_);
		}

	private :
		WCHAR* p_;	// buffer
		int pos_;	// last write position.
		int len_;	// buffer length
};

};

