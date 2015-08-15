#pragma once
namespace V4 {

/*
       start caret  end
0      zS    zC     zE            zEE 
+-------+--+--+--+--+--------------+------------------
 a| b| c| D| E/ F| G| h| i| j| k| l|
--------+--+--+--+--+--------------------------------
        0    rC     rE
--------+--+--+--+--+--------------------------------
        | view area |
--------+--+--+--+--+--------------------------------
*/

// single line caret 位置計算
class SingleLineCaret
{
	public :
		SingleLineCaret( int zS, int zEE, int rE, int rC )
		{	
			rC = max(0,rC);
			zS = max(0,zS);

			xassert( zS <= zEE );
			if ( rC <= rE )
			{
				zS_ = zS;
				zEE_ = zEE;
				rE_ = rE;
				rC_ = rC;
			}
			else
			{
				zS_ = zS + rC - rE;
				zEE_ = zEE;
				rE_ = rE;
				rC_ = rE;
			}
			xassert( rC_ <= rE_ );
		}
		
		SingleLineCaret( LPCWSTR s,  int rE )
		{	
			zS_ = 0;
			zEE_ = lstrlen(s);
			rE_ = rE;
			rC_ = 0;
		}

		int CaretOff( int one )
		{
			_ASSERT( one == 1 || one == -1 || one == 0 );
			int rC = rC_ + one;
			
			if ( rC >= rE_ )
			{
				rC_ = rC - 3;				

				if ( zEE() - zC() < 4 )
				{
					rC_ = min( rE_, rC);
				}
				else
				{
					zS_ = zS_ + 3;
				}

			}
			else if ( rC <= 0 )
			{
				rC_ = rC + 3;

				if ( zC() < 4 )
				{
					rC_ = max(0, rC);
				}
				else
				{
					zS_ = zS_ - 3;
				}
			}
			else
				rC_ = rC;


			if ( zC() > zEE())
			{
				rC_ = zEE_-zS_;
			}
			else if ( zC() < 0 )
			{
				rC_ = 0;
			}

			zS_ = max(0, zS_ );

			return zS_;
		}


		// 文字の先頭オフセット位置
		int zS(){ return zS_; }

		// キャレットの絶対位置
		int zC(){ return zS_+rC_; }

		//表示文字の最後
		int zE(){ return zS_ + rE_; }

		// キャレットの相対位置、Textbox内での位置
		int rC(){ return rC_; }

		// 表示文字数
		int rE(){ return rE_; }

		// 文字の長さ
		int zEE(){ return zEE_; }

	private :
		int zS_,zEE_;
		int rC_,rE_;

	public :
	

};

class CharsRectF
{
	public :
		CharsRectF(IDWriteTextLayout* tx, bool bSingle );
		~CharsRectF(){ Clear(); }

		FRectF* calc( const FSizeF&  sz, LPCWSTR s, int slen, int* plen );
		
		void Clear();
		float LineHeight(){ return line_height_; }
	private :
	
		FRectF* rects_;
		bool bSingleLine_;
		int cnt_;
		IDWriteTextLayout* layout_;
		float line_height_;
		
};


class CreateTextLayout
{
	public :
		CreateTextLayout( IDWriteFactory* fc, LPCWSTR psz, UINT nCnt, IDWriteTextFormat* fmt, const FSizeF& sz, bool IsSibleLine )
		{
			fc_ = fc;
			psz_ = psz;
			len_ = nCnt;
			fmt_ = fmt;
			sz_ = sz;
			IsSibleLine_ = IsSibleLine;
		}

		int Create( int zStartPos, int zCaretPos, IDWriteTextLayout** layout);
	private :
		int CreateMultiTextLayout( int zStartPos, int zCaretPos, IDWriteTextLayout** layout);
		int CreateSingleTextLayout( int zStartPos, int zCaretPos, IDWriteTextLayout** layout);

		IDWriteFactory* fc_;
		LPCWSTR psz_;
		UINT len_;
		IDWriteTextFormat* fmt_;
		FSizeF sz_;
		bool IsSibleLine_;

};



// source is D2DWindowTextboxTSF.cpp

};
