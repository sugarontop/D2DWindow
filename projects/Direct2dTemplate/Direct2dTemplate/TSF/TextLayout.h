#pragma once

#include "D2DContextEx.h"

using namespace V4;
namespace TSF {

struct COMPOSITIONRENDERINFO 
{
    int nStart;
    int nEnd;
    TF_DISPLAYATTRIBUTE da;
};
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

struct CHARINFO 
{
	FRectF rc;
    float GetWidth() {return rc.right - rc.left;}
};

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

struct LINEINFO 
{
    int nPos;
    int nCnt;
    CHARINFO *prgCharInfo;
};


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------


//class LayoutLineInfo
//{
//	public :
//		LayoutLineInfo();
//		~LayoutLineInfo(); 
//	
//		void Create( LINEINFO* line, UINT cnt, int lineheight );
//		void Clear();
//		static void Render(V3::D2DContext& hdc, RECT rc, const WCHAR *psz,  UINT nCnt, LayoutLineInfo* info );
//
//	private :
//	
//		LINEINFO *prgLines_;
//		UINT nLineCnt_;
//		int nLineHeight_;
//
//};

class CTextLayout
{
	public:
		CTextLayout();
		virtual ~CTextLayout();

		BOOL Layout(D2DContext& hdc, const WCHAR *psz,  int nCnt, const SIZE& sz, bool bSingleLine, int nSelEnd,int& StarCharPos, IDWriteTextFormat* fmt);		
		BOOL Render(D2DContext& hdc, const FRectF& rc, const WCHAR *psz,  int nCnt, int nSelStart, int nSelEnd,bool bSelTrail,const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo);

	public :
		int CharPosFromPoint(const FPointF& pt);
		int CharPosFromNearPoint(const FPointF& pt);
		BOOL RectFromCharPos(UINT nPos, FRectF *prc);
    
		UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);
		float GetLineHeight() {return nLineHeight_;}
		float GetLineWidth(){ return row_width_; }

		void Password( bool bl ){ bPassword_ = bl; }
		bool GetTextLayout( IDWriteTextLayout** q );

		UINT GetLineCount(){ return nLineCnt_; }
		UINT GetViewLineCount(){ return tm_.lineCount; } // １行でも折り返されれば、２行になる。折り返し分を含めた行数

		bool bRecalc_;
		D2D1_COLOR_F selected_halftone_color_;
	private:
		void Clear();

		LINEINFO *prgLines_;
		UINT nLineCnt_;
		float nLineHeight_;
		LPCWSTR str_;
		FLOAT row_width_;
		bool bSingleLine_;
		bool bPassword_;
		int StarCharPos_;
		IDWriteTextLayout* DWTextLayout_;

		DWRITE_TEXT_METRICS tm_;		
};

};

