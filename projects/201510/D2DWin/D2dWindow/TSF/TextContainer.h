#pragma once

#include "D2DMisc.h"
//----------------------------------------------------------------
//
// String buffer
//
//----------------------------------------------------------------
namespace TSF {

class CTextContainer
{
	public:
		CTextContainer(); 
		virtual ~CTextContainer();

		BOOL InsertText(int nPos, const WCHAR *psz, UINT nCnt, UINT& nResultCnt);
		BOOL RemoveText(int nPos, UINT nCnt);
		BOOL GetText(int nPos, WCHAR *psz, UINT nBuffSize);

		UINT GetTextLength() {return nTextSize_;}
		const WCHAR *GetTextBuffer() {return psz_;}
		void CaretLast();
		void Clear();

	public :
		int nSelStart_, nSelEnd_;
		bool bSelTrail_;
		V4::FRectF rc_;

		SIZE view_size_;

		bool bSingleLine_;
		UINT LimitCharCnt_;

		V4::FPointF offpt_; // Textboxì‡ÇÃï∂éöÇÃà⁄ìÆÅASinglelineÇ≈èdóv
		int nStartCharPos_;
	private:
		BOOL EnsureBuffer(UINT nNewTextSize);
	
		WCHAR* psz_;

		UINT nBufferSize_;
		UINT nTextSize_;

	
};


};
