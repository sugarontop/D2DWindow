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

namespace GDI32
{
	struct FPoint: public POINT
	{
		public :
			FPoint(){ x=y=0; }		
			FPoint( long xa, long ya ){ x=xa; y=ya; }
			FPoint(LPARAM lp ){ x = LOWORD(lp); y = HIWORD(lp);}
	};
	struct FSize : public SIZE
	{
		public :
			FSize(){ cx=cy=0; }
			FSize(long cxx, long cyy ){ cx=cxx; cy=cyy; }
			FSize(LPARAM lp ){ cx = LOWORD(lp); cy = HIWORD(lp);}
	};
	struct FRect : public RECT
	{
		public :
			FRect(){left=right=top=bottom = 0;}
			FRect(long l, long t, long r, long b ){left=l;right=r;top=t;bottom=b;}
			FRect(FPoint pt, FSize sz){ left=pt.x; top=pt.y;right=pt.x+sz.cx;bottom=top+sz.cy; }

			bool PtInRect( FPoint& p ) const;
			FSize Size() const{ return FSize(right-left,bottom-top); }
			void Size( FSize& sz ) { right=left+sz.cx; bottom=top+sz.cy; }
			FPoint LeftTop() const { return FPoint(left,top); }
			FPoint RightBottom() const { return FPoint(right,bottom); }
			int Width() const { return right-left; }
			int Height() const { return bottom-top; }
			void Normalize();

			void Offset( int cx, int cy );
			void Inflate( int cx, int cy );
			FRect ZRect(){ return FRect( 0,0,right-left,bottom-top); }
	};

	void LPtoDP( HDC hdc, FRect& rc );
	void DPtoLP( HDC hdc, FRect& rc );

	struct Font
	{
		Font( int height, LPCWSTR fontnm );
		~Font();

		HFONT Select( HDC hdc );
	
		HFONT hfont;
	};

	struct Brush
	{
		Brush( int stockobject );
		Brush( COLORREF clr );
		~Brush();

		HBRUSH hBrush;
	};
	struct Region
	{
		Region( const FRect& rc );
		~Region();

		HRGN hRegion;
	};


};
