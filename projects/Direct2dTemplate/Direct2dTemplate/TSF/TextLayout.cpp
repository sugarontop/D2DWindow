#include "stdafx.h"
#include "TextLayout.h"
#include "D2DContextNew.h"
#include "D2DContextEx.h"
#include "D2DCharRect.h"
using namespace V4;
using namespace TSF;


static LPCWSTR __password = L"**************************************************"; // 50 chars

CTextLayout::CTextLayout()
{
	prgLines_ = NULL;
	nLineCnt_ = 0;
	str_ = NULL;
	row_width_ = 0;
	bPassword_ = false;
	bRecalc_ = true;
	DWTextLayout_ = nullptr;
	selected_halftone_color_ = D2RGBA(0,140,255,100);
}
CTextLayout::~CTextLayout()
{
	Clear();
}

//----------------------------------------------------------------
//
//
// Calc layout 行数の把握と文字単位にPOS,LEN,RECTを取得
//----------------------------------------------------------------

BOOL CTextLayout::Layout(D2DContext& cxt, const WCHAR *psz, int nCnt,const SIZE& sz, bool bSingleLine, int zCaret, int& StarCharPos,IDWriteTextFormat* fmt)
{
//TRACE( L"CTextLayout::Layout\n" );

	Clear();
	bSingleLine_ = bSingleLine;

	str_ = psz;
	nLineCnt_ = 1;
	StarCharPos_ = 0;

    // 行数を計算
 
     nLineCnt_ = 0;
	
	{
		BOOL bNewLine = TRUE;
		for (int i = 0; i < nCnt; i++)
		{
			switch (psz[i])
			{
				case 0x0d:
				case 0x0a:                
					if (bNewLine)
						nLineCnt_++;
					bNewLine = TRUE;
					break;
				default:
					if (bNewLine)
						nLineCnt_++;
					bNewLine = FALSE;
					break;
			}
		}
	}
			
	// 行数分のLINEINFOを作成, 文字0でもLINEINFOは１つ作成
	prgLines_ = new LINEINFO[max(1,nLineCnt_)];
	
    // Count character of each line.　文字単位にPOS,LEN,RECTを取得
   
	int nCurrentLine = -1;
	prgLines_[0].nPos = 0;
	prgLines_[0].nCnt = 0;
	prgLines_[0].prgCharInfo = nullptr;
	


	// prgLines_ の設定
	{
		UINT crlf = 0;
		int nNewLine = 1;

		for (int i = 0; i < nCnt; i++)
		{
			switch (psz[i])
			{
				case 0x0d:
				case 0x0a:
					nNewLine++;

					if (nNewLine == 2)
					{
						nCurrentLine++;
						prgLines_[nCurrentLine].nPos = i;
						prgLines_[nCurrentLine].nCnt = 0; // CRCRの場合、nCntは0		
						prgLines_[nCurrentLine].prgCharInfo = nullptr;			
						nNewLine = 1;
					}
				
								
					xassert( crlf == 0 || crlf == psz[i] ); // CRLFはだめ、CRCR.. or LFLF..はOK
					crlf = psz[i];
					break;
				default:
					if (nNewLine)
					{
						nCurrentLine++;
						prgLines_[nCurrentLine].nPos = i;
						prgLines_[nCurrentLine].nCnt = 1;
						prgLines_[nCurrentLine].prgCharInfo = nullptr;			

					}
					else
					{
						prgLines_[nCurrentLine].nCnt++;

					}
					nNewLine = 0;
					crlf = 0;
					break;
			}
		}
	}

//#ifdef TEXTBOXTEST
//	for (UINT i = 0; i < nLineCnt_; i++)
//	{
//		TRACE( L"prgLines_ %d %d\n", prgLines_[i].nPos, prgLines_[i].nCnt );
//	}
//#endif

    //TEXTMETRIC tm;
    //GetTextMetricsEx(hdc,fmt, &tm);
    //nLineHeight_ = tm.tmHeight + tm.tmExternalLeading; // equal to ascent+descent+linegap.



	if ( DWTextLayout_ )
		DWTextLayout_->Release();


	{		
		LPCWSTR str = ( bPassword_ ? __password : psz );

		CreateTextLayout ct(cxt.wfactory, str, nCnt, fmt, sz, bSingleLine_);

		StarCharPos = ct.Create( StarCharPos, zCaret, &DWTextLayout_ ); //★TextLayout_

		StarCharPos_ = StarCharPos;
			
		CharsRectF cr( DWTextLayout_, false );

		int slen = nCnt;
		int len;

		// 文字文のRECT取得
		FRectF* prcs = cr.calc( sz, NULL, slen, &len );

		nLineHeight_ = cr.LineHeight();
			

		// prgLines_[lineno].prgCharInfo[col].rc の設定
		{
			UINT rcIdx = 0;
			// Get the rectangle of each characters. RECTを取得
			for (UINT r = 0; r < nLineCnt_; r++)
			{
				prgLines_[r].prgCharInfo = NULL;

				UINT nCnt = prgLines_[r].nCnt;
				if (nCnt)
				{
					UINT nCntK = nCnt+1; // 改行分
				
					prgLines_[r].prgCharInfo = new CHARINFO[nCntK];
			
					UINT col;
					for (col = 0; col < nCnt ; col++)
					{ 				
						FRectF rc = prcs[rcIdx++];
						prgLines_[r].prgCharInfo[col].rc = rc;
					}

					// 行の最後の処理				
					{
						FRectF rc = prgLines_[r].prgCharInfo[col-1].rc;
						rc.left = rc.right; //前の文字の右端を左端として、rectを作成
						prgLines_[r].prgCharInfo[col].rc = rc;
						rcIdx++;
					}
				}
				else // 空行
				{
					prgLines_[r].prgCharInfo = new CHARINFO[1];				
					FRectF rc = prcs[rcIdx++];

					xassert( rc.left == rc.right );

					prgLines_[r].prgCharInfo[0].rc = rc;

					xassert( prgLines_[r].nCnt == 0 );
				}
			}
		}
		
		if ( nCnt )
		{		
			// 1行目の文字幅の取得、右寄せのためにrow_width_必要
			row_width_ = 0;
			for (int col = 0; col < prgLines_[0].nCnt ; col++)
			{
				row_width_ += prgLines_[0].prgCharInfo[col].rc.Width();
			}
		}


		{
		
			DWTextLayout_->GetMetrics(&tm_);
		}

		return TRUE;
	}


    return FALSE;
}
//----------------------------------------------------------------
// Rnderを開始、　DrawTextLayoutで文字表示とCaretを位置を計算して表示
//
//
//----------------------------------------------------------------

#pragma region CaretRect
class CaretRect
{
	public :
		CaretRect( bool bTrail ):bTrail_(bTrail),cnt_(0){};

		void Push(const FRectF& rc, int row, int colStart, int colLast )
		{
			if ( bTrail_ )
			{
				rc_ = rc;
				row_ = row;
				col_ = colLast;
			}
			else if ( !bTrail_ && cnt_ == 0) 
			{
				rc_ = rc;
				row_ = row;
				col_ = colStart;
			}
					
			cnt_++;		
		}

		FRectF Get(){ return rc_; }
		bool empty() { return cnt_ == 0; }

		int row(){ return row_; }
		int col(){ return col_; }


		bool IsComplete( int rowno )
		{
			if ( cnt_ == 0 ) return false;

			if ( bTrail_ )
			{
				if ( row_ == rowno )
					return false;
			}

			return true;
		}

	private :
		bool bTrail_;
		int cnt_;
		FRectF rc_;
		int row_,col_;
};
#pragma endregion

BOOL CTextLayout::Render(D2DContext& cxt, const FRectF& rc,LPCWSTR psz,  int nCnt, int nSelStart, int nSelEnd,bool bTrail,
	const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo)
{
	{
		_ASSERT( rc.left == 0 );
		_ASSERT( rc.top == 0 );
		
		_ASSERT( DWTextLayout_ );

		cxt.cxt->DrawTextLayout( FPointF(), DWTextLayout_, cxt.black ); // D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

    // Render selection,caret
    FRectF rcSelCaret(0,0,1.0f, (float)nLineHeight_);

	CaretRect xcaret(bTrail);

    if (nLineCnt_)
    {
#pragma region CaretRectX
	
        for (UINT r = 0; r < nLineCnt_; r++)
        {           
			
			if ((nSelEnd >= prgLines_[r].nPos) && (nSelStart <= prgLines_[r].nPos + prgLines_[r].nCnt))			
            {
                int nSelStartInLine = 0;
                int nSelEndInLine = prgLines_[r].nCnt;
				
				if (nSelStart > prgLines_[r].nPos)
                {
				    nSelStartInLine = nSelStart - prgLines_[r].nPos;
				}

                if (  nSelEnd < prgLines_[r].nPos + prgLines_[r].nCnt)
				{
                    nSelEndInLine = nSelEnd - prgLines_[r].nPos;
					
				}
	                
				// caret, select範囲指定の場合
				if (nSelStartInLine != nSelEndInLine && nSelEndInLine != -1)
                {
                    for (int j = nSelStartInLine; j < nSelEndInLine; j++)
                    {
						auto br = MakeBrsuh(cxt, selected_halftone_color_ );
						cxt.cxt->FillRectangle( prgLines_[r].prgCharInfo[j].rc, br ); // SELECTED 色でfill, セレクトされた時のハーフトン塗り
                    }

					bool blast = bTrail;

					if ( blast )
					{
						rcSelCaret = prgLines_[r].prgCharInfo[nSelEndInLine-1].rc;
						rcSelCaret.left = rcSelCaret.right;
					}
					else
					{
						rcSelCaret = prgLines_[r].prgCharInfo[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left;
					}
					rcSelCaret.right++;

					xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine );

				
                }				
                else // caret, 範囲指定されてない場合
                {					
					if (nSelStartInLine == prgLines_[r].nCnt && prgLines_[r].prgCharInfo )
					{
						rcSelCaret = prgLines_[r].prgCharInfo[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;
	
						_ASSERT( rcSelCaret.bottom >= 0 );
						_ASSERT( rcSelCaret.bottom < 65000 );

						xcaret.Push(rcSelCaret, r, nSelStartInLine, nSelEndInLine);
						
                
					}
					else if ( nSelStartInLine > -1 && prgLines_[r].prgCharInfo )
					{
						rcSelCaret = prgLines_[r].prgCharInfo[nSelStartInLine].rc;
						rcSelCaret.right = rcSelCaret.left + 1;
		
						_ASSERT( rcSelCaret.bottom >= 0 );
						xcaret.Push(rcSelCaret,r, nSelStartInLine, nSelEndInLine);
						                
					}
				}
					
            }

			// キャレット表示位置が確定
			if ( xcaret.IsComplete(r) )
			{
				if ( bTrail )
					break;
				
			}
			
			for (UINT j = 0; j < nCompositionRenderInfo; j++)
			{
		
				if ((pCompositionRenderInfo[j].nEnd >= prgLines_[r].nPos) &&
					(pCompositionRenderInfo[j].nStart <= prgLines_[r].nPos + prgLines_[r].nCnt))
				{
					UINT nCompStartInLine = 0;
					UINT nCompEndInLine = prgLines_[r].nCnt;
					int  nBaseLineWidth = (nLineHeight_ / 18) + 1;
    
					if (pCompositionRenderInfo[j].nStart > prgLines_[r].nPos)
						nCompStartInLine = pCompositionRenderInfo[j].nStart - prgLines_[r].nPos;
    
					if (pCompositionRenderInfo[j].nEnd < prgLines_[r].nPos + prgLines_[r].nCnt)
						nCompEndInLine = pCompositionRenderInfo[j].nEnd - prgLines_[r].nPos;
    
					for (UINT k = nCompStartInLine; k < nCompEndInLine; k++)
					{
						UINT uCurrentCompPos = prgLines_[r].nPos + k - pCompositionRenderInfo[j].nStart;
						BOOL bClause = FALSE;
     
						if (k + 1 == nCompEndInLine)
						{
							bClause = TRUE;
						}

						if ((pCompositionRenderInfo[j].da.crText.type != TF_CT_NONE) &&
							(pCompositionRenderInfo[j].da.crBk.type != TF_CT_NONE))
						{
						   /* SetBkMode(hdc, OPAQUE);
							SetTextColor(hdc, GetAttributeColor(&pCompositionRenderInfo[j].da.crText));
							SetBkColor(hdc, GetAttributeColor(&pCompositionRenderInfo[j].da.crBk));
 
							RECT rc = prgLines_[r].prgCharInfo[k].rc;
							ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE, &rc, 
										psz + prgLines_[r].nPos + k, 1, NULL);*/



							int a = 0;
						}
     
						if (pCompositionRenderInfo[j].da.lsStyle != TF_LS_NONE)
						{
							// 変換途中の下線の描画
							{
								FRectF rc = prgLines_[r].prgCharInfo[k].rc;
     
								FPointF pts[2];
								pts[0].x = rc.left;
								pts[0].y = rc.bottom;
								pts[1].x = rc.right - (bClause ? nBaseLineWidth : 0);
								pts[1].y = rc.bottom;
								//Polyline(hdc, pts, 2);
								


								cxt.cxt->DrawLine( pts[0], pts[1], cxt.black );     
							}
						}
					}
				}
			}
		}
#pragma endregion


		if ( xcaret.empty() )
		{
			//次行の文字なし先頭列
			rcSelCaret.left = 0;
			rcSelCaret.right = 1;

			rcSelCaret.Offset( 0, nLineHeight_*nLineCnt_ );			
			xcaret.Push(rcSelCaret, nLineCnt_, 0, 0);
		}
		
		
		// 情報
		int CaretRow = xcaret.row()+1;
		int CaretCol = xcaret.col();
		int LineNumber = nLineCnt_;
				

		// Caret表示
		DrawCaret(cxt, xcaret.Get()); 

    }
    else
    {
		// 文字がない場合Caret表示
		DrawCaret(cxt, rcSelCaret);
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::RectFromCharPos(UINT nPos, FRectF *prc)
{
    FRectF& retrc = *prc;

	retrc.Clear();

    UINT sum = 0;
	UINT current_rowno = 0;
	for (UINT r = 0; r < nLineCnt_; r++)
	{		
		if ( prgLines_[r].nCnt )
			sum += prgLines_[r].nCnt+1; // 1 is \r
		else
			sum += 1;

		current_rowno = r;
		if ( nPos < sum )
		{			
			break;
		}
	}
	
	UINT pos = nPos - prgLines_[current_rowno].nPos;

	if ( pos <= (UINT)prgLines_[current_rowno].nCnt && prgLines_[current_rowno].nCnt )
	{
		retrc = prgLines_[current_rowno].prgCharInfo[pos].rc;
		return TRUE;
	}
	else if ( pos > 0 )
	{
		retrc = prgLines_[current_rowno].prgCharInfo[pos-1].rc;
		retrc.left = retrc.right;		
		return TRUE;
	}
	else if ( sum )
	{
		retrc = prgLines_[current_rowno].prgCharInfo[0].rc; // 空行で先頭の場合、
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

static bool PtInRect0( const FRectF& rc, const FPointF& pt, bool bSinleLine )
{	
	if ( bSinleLine )
	{
		// rc.bottomは見ないことにした
		if ( rc.left <= pt.x && pt.x <= rc.right && rc.top <= pt.y )
			return true;
	}
	else
	{
		if ( rc.PtInRect( pt )) return true;
	}

	if ( pt.x == 0 && rc.left == 0 && rc.right == 0 && rc.top <= pt.y && pt.y <= rc.bottom )
		return true;
	return false;

}
static bool PtInRectRightSide( const FRectF& rc, const FPointF& pt )
{
	return ( rc.top <= pt.y && pt.y <= rc.bottom && rc.right <= pt.x );
}


int CTextLayout::CharPosFromPoint(const FPointF& pt)
{
	xassert( StarCharPos_ == 0 || (StarCharPos_ && this->bSingleLine_) );
		
		
	for (UINT r = 0; r < nLineCnt_; r++)
	{
		// 行内
		for (int j = 0; j < prgLines_[r].nCnt; j++)
		{
			FRectF& rc = prgLines_[r].prgCharInfo[j].rc;

			if (PtInRect0(rc, pt, bSingleLine_))
			{
				int nWidth = prgLines_[r].prgCharInfo[j].GetWidth();
				if (pt.x > prgLines_[r].prgCharInfo[j].rc.left + (nWidth * 3 / 4))
				{
					return prgLines_[r].nPos + j + 1 +StarCharPos_ ;
				}
				return prgLines_[r].nPos + j + StarCharPos_;
			}
		}

		// 行端
		{
			if ( r == 1 )
			{
				r = 1;
			}

			int j = prgLines_[r].nCnt;
			FRectF& rc = prgLines_[r].prgCharInfo[j].rc;
			if ( PtInRectRightSide( rc, pt ))			
				return prgLines_[r].nPos + j + StarCharPos_;
		}
	}
	
	return -1;
}

int CTextLayout::CharPosFromNearPoint(const FPointF& pt)
{
	struct st
	{
		int i,j;
	};
	
	std::vector<st> ar;

    for (UINT i = 0; i < nLineCnt_; i++)
    {
	    for (int j = 0; j < prgLines_[i].nCnt+1; j++) // +1 is return key
        {
			xassert( prgLines_[i].prgCharInfo );
			
			FRectF rc = prgLines_[i].prgCharInfo[j].rc;

			if ( rc.top <= pt.y && pt.y <= rc.bottom )
			{
				st s;
				s.i = i;
				s.j = j;
				ar.push_back(s);
			}			
		}
	}
		
	
	UINT last = ar.size();	
	
	if ( last )
	{
		st s = ar[last-1];

		return prgLines_[s.i].nPos + s.j;
	
	}		
			
    return -1;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
    if (bFirst)
	{
		// 先頭列へ
		for (UINT i = 0; i < nLineCnt_; i++)
		{
			if ((prgLines_[i].nPos <= (int)uCurPos) && ( (int)uCurPos <= prgLines_[i].nPos + prgLines_[i].nCnt))
			{
				return prgLines_[i].nPos;
			}
		}

	}
	else
	{	
		// 行の最後尾
		for (UINT i = 0; i < nLineCnt_; i++)
		{
			if ((prgLines_[i].nPos <= (int)uCurPos) && ( (int)uCurPos <= prgLines_[i].nPos + prgLines_[i].nCnt))
			{
				{
					UINT idx = prgLines_[i].nPos + prgLines_[i].nCnt;
					return idx;
				}
			}
		}
	}
    return (UINT)(-1);
}


//----------------------------------------------------------------
//
// memory clear
//
//----------------------------------------------------------------

void CTextLayout::Clear()
{
    if (prgLines_)
    {
        for (UINT i = 0; i < nLineCnt_; i++)
        {
            if (prgLines_[i].prgCharInfo)
            {
				delete [] prgLines_[i].prgCharInfo;
            }
        }
		delete [] prgLines_;
		prgLines_ = NULL;
    }
    nLineCnt_ = 0;

	if ( DWTextLayout_ )
	{
		DWTextLayout_->Release();
		DWTextLayout_ = nullptr;
	}
}

bool CTextLayout::GetTextLayout( IDWriteTextLayout** q )
{
	_ASSERT((*q) == nullptr);
	if ( HR(DWTextLayout_->QueryInterface(q)))
	{
		(*q)->AddRef();
		return true;
	}
	return false;
}