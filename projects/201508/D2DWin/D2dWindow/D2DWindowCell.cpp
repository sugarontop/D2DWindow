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

#include "stdafx.h"
#include "D2DWindowMessage.h"
#include "D2DWindowControl_easy.h"
#include "D2DCommon.h"
#include "D2DWindowCell.h"

using namespace V4;

#define MAGICNUMBER_ROWMAX 10000

/////////////////////////////////////////////////////////////////////////////////////////////////////
TableFRectF::TableFRectF()
{
	xpos_ = nullptr;
	ypos_ = nullptr;
	 
	rowcnt_ = 0;
	colcnt_ = 0;
}
TableFRectF::TableFRectF(int rowcnt, int colcnt)
{
	Set( rowcnt,colcnt );
}
void TableFRectF::Set(int rowcnt, int colcnt)
{
	_ASSERT( 0 < rowcnt && 0 < colcnt );
	ypos_ = new float[rowcnt+1];
	xpos_ = new float[colcnt+1];
		 
	rowcnt_ = rowcnt;
	colcnt_ = colcnt;
}
TableFRectF::~TableFRectF()
{
	delete [] xpos_;
	delete [] ypos_;
}

FRectF TableFRectF::GetRectF(int row, int col)
{
	_ASSERT( -1 < row && -1 < col );
	_ASSERT(row < rowcnt_ && col < colcnt_ );
	

	FRectF rc( xpos_[col], ypos_[row], xpos_[col+1], ypos_[row+1]);
	return rc;
}
void TableFRectF::SetColWidth(int col, float width)
{
	_ASSERT( 0 <= width );

	if (col < 0)
	{
		xpos_[0] = 0;
		for( int c = 1; c <= colcnt_; c++ )
		{
			xpos_[c] = xpos_[c-1] + width;
		}
		return;
	}

	float* old = new float[colcnt_+1];
	memcpy( old, xpos_, sizeof(float)*(colcnt_ + 1));

	xpos_[col+1] = xpos_[col] + width;

	for( int c = col+1; c <=colcnt_; c++ )
	{
		xpos_[c+1] = xpos_[c] + (old[c+1] - old[c]);
	}

	delete [] old;
}
void TableFRectF::SetRowHeight(int row, float height)
{
	_ASSERT(0 <= height);

	if (row < 0)
	{
		ypos_[0] = 0;
		for (int c = 1; c <= rowcnt_; c++)
		{
			ypos_[c] = ypos_[c - 1] + height;
		}
		return;
	}

	float* old = new float[rowcnt_ + 1];
	memcpy(old, ypos_, sizeof(float)*(rowcnt_ + 1));

	ypos_[row+1] = ypos_[row] + height;

	for (int c = row + 1; c <= rowcnt_; c++)
	{
		ypos_[c+1] = ypos_[c] + (old[c+1] - old[c]);
	}

	delete [] old;
}


UINT TableFRectF::magicnumber(int row, int col)
{
	_ASSERT( row < MAGICNUMBER_ROWMAX );
	
	return col * MAGICNUMBER_ROWMAX + row; // とりあえず
}
FRectF TableFRectF::GetRectF(UINT magicnumber)
{
	int col = magicnumber / MAGICNUMBER_ROWMAX;
	int row = magicnumber - col*MAGICNUMBER_ROWMAX;

	return GetRectF( row,col );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////


LRESULT D2DTableControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ( !VISIBLE(stat_))
		return ret;

	switch( message )
	{
		case WM_PAINT:
		{
			D2DContext& cxt = d->cxt_;
			FRectF rcc = rc_.GetContentRect();
			D2DMatrix mat(cxt);
			mat.PushTransform();
			mat.Offset(rcc);

			Draw(cxt);

			mat.PopTransform();
			return 0;
		}
		break; 
	}

	if ( ret == 0 )
		ret = SendMessageAll(d, message, wParam, lParam);

	return ret;
}
void D2DTableControls::Draw(D2DContext& cxt)
{
	D2DMatrix mat(cxt);
	for( auto it = tctrl_.begin(); it != tctrl_.end(); it++ )
	{
		FRectF rc = trc_.GetRectF(it->first);
		mat.PushTransform();
		D2DRectFilter f(cxt, rc); 
		
		mat.Offset( rc.left, rc.top );
		it->second->WndProc( parent_,WM_PAINT,0,0);

		mat.PopTransform();
	}
}

void D2DTableControls::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, int rowcnt, int colcnt, LPCWSTR name, int id )
{
	D2DControls::InnerCreateWindow(parent,pacontrol,rc,stat,name,id);
	
	FRectF rcc = rc.GetContentRect();
	
	trc_.Set(rowcnt,colcnt );

	trc_.SetRowHeight(-1, rcc.Height() / rowcnt);
	trc_.SetColWidth(-1, rcc.Width() / colcnt);


}


void D2DTableControls::SetControl(int row, int col, D2DControl* ctrl)
{
	_ASSERT( this == ctrl->parent_control_ );

	UINT mnum = trc_.magicnumber(row,col);
	tctrl_[ mnum ] = ctrl;
}



void D2DTableControls::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	_variant_t col,row,value;
	if ( findParameterMap(prms, L"value", _variant_t(0), value)) 
	{	
		value.ChangeType(VT_R4);
	
		if ( findParameterMap(prms, L"col", _variant_t(-1),col))  
		{	
			col.ChangeType(VT_I4);
			trc_.SetColWidth( col.iVal, value.fltVal );
		}
		if (findParameterMap(prms, L"row", _variant_t(-1), row))
		{
			row.ChangeType(VT_I4);
			trc_.SetRowHeight(row.iVal, value.fltVal);
		}
	}	

}
namespace V4 {
	bool findParameterMap(const std::map<std::wstring, VARIANT>& prms, LPCWSTR key, _variant_t& def, _variant_t& ret)
	{
		auto it = prms.find(key);
		if (prms.end() == it)
		{
			ret = def;
			return false;
		}

		ret = (*it).second;
		return true;
	}

};
