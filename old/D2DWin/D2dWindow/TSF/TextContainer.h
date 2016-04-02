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

		V4::FPointF offpt_; // Textbox内の文字の移動、Singlelineで重要
		int nStartCharPos_;
	private:
		BOOL EnsureBuffer(UINT nNewTextSize);
	
		WCHAR* psz_;

		UINT nBufferSize_;
		UINT nTextSize_;

	
};


};
