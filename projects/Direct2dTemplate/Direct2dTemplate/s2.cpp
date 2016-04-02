#include "stdafx.h"
#include "D2DWindow.h"
#include "gdi32.h"
#include "MoveTarget.h"
#include "D2DWindowDataGrid.h"
#include "s1.h"
#define CLASSNAME L"D2DWindow"
using namespace V4;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OFFCX 3
#define TITLE_HEIGHT 26

D2DGridControl::D2DGridControl()
{
	item_min_height_ = 0;
}
void D2DGridControl::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height, D2DListboxItem* template_item, LPCWSTR name, int id )
{
	D2DControl::InnerCreateWindow( parent,pacontrol,rc,stat,name, id );

	xassert(item_min_height != 0 );
	item_min_height_ = item_min_height;
	template_item_= template_item;
	

	OnCreate();
}
void D2DGridControl::OnCreate()
{
	xassert(item_min_height_ != 0 );

	selected_idx_ = float_idx_ = -1;
	int cnt = (int)(rc_.GetContentRect().Height()/item_min_height_) + 2; // 高さに依存

	
	if ( template_item_ == NULL )
	{
		static D2DListboxItem default_item;
		template_item_ = &default_item;
	}


	for( int i = 0; i < cnt; i++ )
	{
		auto p = template_item_->Clone(); // new D2DListboxItem();

		p->OnCreate( this );
	
		Items_.push_back( p );
		
		loopitems_.Add( new Item( (void*)p)); 
		
	}
	loopitems_.AddEnd();

	title_height_ = template_item_->TitleHeight();

	FRectFBoxModel rc1(rc_.Width()-20,0, rc_.Width(), rc_.Height()-title_height_);
	rc1.Margin_.Set(1);
	rc1.Margin_.l = 0;

	if ( title_height_ != 0 )
		rc1.Margin_.t = 0;

	bar_ = new D2DScrollbar();
	bar_->CreateWindow( parent_, this, rc1, VISIBLE|BORDER|ALWAYSDRAW, L"listbox-scrollbar" );
	bar_->TotalScrollSize( 1*item_min_height_ );

	bar_->target_control_ = this; // バーのメッセージを受けるオブジェクト

	
}
void D2DGridControl::Clear()
{
	for( auto it = Items_.begin(); it != Items_.end(); ++it )
		delete (*it);
			
	Items_.clear();
	ItemsHeight_.clear();
	selected_idx_ = float_idx_ = -1;
}
LRESULT D2DGridControl::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !VISIBLE(stat_))
		return ret;

	D2DContext& cxt = d->cxt_;

	D2DMatrix mat(d->cxt_);		
	
	
	switch( message )
	{
		case WM_PAINT:
		{
			mat_ = mat.PushTransform();	
			{
				FRectF rcborder = rc_.GetBorderRect();
				
				if ( BORDER(stat_))
				{
					//DrawLineRect( cxt, rcborder, cxt.bluegray );
					DrawFillRect(cxt,rcborder,cxt.bluegray,cxt.white,1);
				}

				cxt.SetAntiAlias(false);
				{
					FRectF rc = rc_.GetContentRect();
					D2DRectFilter f(cxt, rc );

					mat.Offset( rcborder.left, rcborder.top + title_height_ );		
				
			
					mat.PushTransform();
					{				
						int cnt = loopitems_.Count();
						DrawContent( cxt, cnt, rc.Width()-OFFCX, item_min_height_, scrollbar_off_.height, NULL );
					}
					mat.PopTransform();
				}				
				cxt.SetAntiAlias(true);						
				{
					// draw srcrollbar
					FRectF rcc = rc_.GetContentRect(); 
					D2DMatrix mat(cxt);
					mat.PushTransform();
					bar_->WndProc(d, message, wParam, lParam );
					mat.PopTransform();
				}
			}
			mat.PopTransform();

		}
		break;

		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));
			if ( rc_.PtInRect( pt ) )
			{
				pt.y -= rc_.top;
				pt.x -= rc_.left;

				if ( 0 <= pt.y && pt.y < title_height_ )
				{
					// 上部のタイトル内の操作
					ret = template_item_->WndProc(d, message, wParam, lParam );
				}
				else if ( rc_.Width()-bar_->GetRect().Width() < pt.x )
				{			
					// スクロールバの操作
					ret = bar_->WndProc(d, message, wParam, lParam );
				}
				else
				{
					// Grid行の操作
					ret = RowsProc(d,message,pt );

				}
			}
		}
		break;

	}

	return ret;
}

LRESULT D2DGridControl::RowsProc(D2DWindow* parent, UINT message, const FPointF& pt)
{
	if ( message == WM_LBUTTONDOWN)
	{
	
		FPointF mousept(pt);
	
		mousept.y += (scrollbar_off_.height - title_height_); 
		mousept.x += scrollbar_off_.width;

		int idx = selected_idx_;

		idx = CalcIdx(mousept);


		if ( idx != selected_idx_ )
		{
			selected_idx_ = idx;

			// OnSelectChanged
		}
	}

	return 0;
}

int D2DGridControl::CalcIdx( const FPointF& mousept )
{
	int idx = 0; 

	float h = 0;
	for( auto it=ItemsHeight_.begin(); it!=ItemsHeight_.end(); ++it )
	{
		if ( h <= mousept.y  && mousept.y <= h+(*it) )
			break;

		h += (*it);
		++idx;
	}
	return idx;
}
void D2DGridControl::UpdateScrollbar(D2DScrollbar* bar)
{
	if ( bar->info_.bVertical )	
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c; // offセットの絶対値
	else
		scrollbar_off_.width = bar->info_.position;
}
int D2DGridControl::CalcdiNEx( float offy, float& h2 )
{
	int idivN = 0;
	float h = 0;
	h2 = 0;

	for( auto it = ItemsHeight_.begin(); it!=ItemsHeight_.end(); ++it )
	{				
		h += (*it);

		if ( offy > h )
		{
			++idivN;
			h2 = h;
		}
		else
			break;
	}
	return idivN;
}
void D2DGridControl::DrawContent( D2DContext& cxt, int cnt, float cx, float itemHeight, float offy, void* etc )
{
	auto ar = loopitems_.get(cnt);

	float h2 = 0;

	int idivN = CalcdiNEx( offy, h2 ); // 先頭の頭出し

	float offy1 = offy-  h2; 

	D2DMatrix mat(cxt);
	mat.PushTransform();
	{
		mat.Offset( -scrollbar_off_.width, -offy1);		
				
		mat.PushTransform();
		{
			FSizeF itemsz( cx, itemHeight ); 
			auto cobj = parent_->GetCapture();

			D2DContextText& tt = cxt.cxtt;

			bool bcap = ( parent_->GetCapture() == this );
				
			for( auto it = ar.begin(); it < ar.end(); ++it )
			{		
				Item* pd = (Item*)(*it);
				int data_idx = pd->idx() + idivN;
		
				D2DListboxItem* t = (D2DListboxItem*)pd->t_;

				if ( data_idx < (int)data_cnt_ )
				{
					itemsz.height = itemHeight; // t->Draw内で再設定される場合があるため


					t->Bind( parent_, (void*)(serial_data_)->at(data_idx)); //serial_constent_data_[data_idx]) );
					t->BindImage( tt, itemsz );
			
					if ( t == cobj )
						t->Draw(cxt, itemsz, (stat_ | (int) STAT::CAPTURED), etc);
					else if ( float_idx_ == data_idx )
						t->Draw(cxt, itemsz, (stat_ | (int) STAT::MOUSEMOVE), etc);
					else if ( selected_idx_ == data_idx )
					{
						if ( bcap )
							t->Draw(cxt, itemsz, (stat_ | (int)STAT::SELECTED|(int)STAT::CAPTURED), etc);
						else
							t->Draw(cxt, itemsz, (stat_ | (int) STAT::SELECTED), etc);
					}
					else
						t->Draw(cxt, itemsz, stat_, etc);


					DrawLine( cxt.cxt, FPointF(0,itemsz.height),FPointF(cx,itemsz.height),1,cxt.gray ); // 横線
				}

				mat.Offset( 0,itemsz.height );
			}
		}
		mat.PopTransform();
		
	}
	mat.PopTransform();

	if ( title_height_ )
	{
		mat.PushTransform();
		{
			mat.Offset( 0, -title_height_ );
			DrawContentTitle( cxt );
		}
		mat.PopTransform();	
	}

	DrawContentLast( cxt );

	
}
void D2DGridControl::DrawContentTitle(D2DContext& cxt )
{
	template_item_->DrawBegin(cxt, rc_.Size());
}
void D2DGridControl::DrawContentLast(D2DContext& cxt )
{
	template_item_->DrawEnd(cxt, rc_.Size());
}

void D2DGridControl::SetData( std::shared_ptr<SequentialData> data )
{ 
	serial_data_ = data; 
	data_cnt_ = data->size();

	ItemsHeight_.clear();
	
	// 可変高のItemに対応するが、初期状態はすべて同じ高さ
	float h = 0;
	ItemsHeight_.resize( data_cnt_ );
	for( UINT i = 0; i < data_cnt_; i++ )
	{
		float itemh = item_min_height_;		
		ItemsHeight_[i] = itemh;

		h+= itemh;
	}
	bar_->TotalScrollSize( h );
}

//////////////////////////////////////////////////////

void D2DStatsuControlItem::Draw( D2DContext& cxt, FSizeF& sz, int stat, void* etc )
{
	FRectF rc(0,0,sz.width,sz.height);

	/*if ( stat & STAT::MOUSEMOVE )
	{
		CComPtr<ID2D1SolidColorBrush> br;
		cxt.cxt->CreateSolidColorBrush(D2RGBA(153,217,234,30),&br);
		FillRoundRect(cxt,cxt.factory(), rc, 3, br );
	}
	else if ( (stat & STAT::SELECTED) && (stat & STAT::CAPTURED) )
	{
		CComPtr<ID2D1SolidColorBrush> br;
		cxt.cxt->CreateSolidColorBrush(D2RGBA(153,217,234,100),&br);
		FillRoundRect(cxt, cxt.factory(), rc, 3, br );
	}	
	else if ( stat & STAT::SELECTED )
	{		
		FillRoundRect(cxt, cxt.factory(), rc, 3, cxt.halftone );
	}*/

	if ( stat & STAT::SELECTED )
	{		
		FillRoundRect(cxt, cxt.factory(), rc, 3, cxt.halftone );
	}

	
	FString x;
 
	StatusSequentialData::Row* s = (StatusSequentialData::Row*)data_;

	rc.right = rc.left+line_width_[0];
	auto cidx = FString::Number(s->idx);
	DrawCenterText( cxt.cxt, cxt.black, rc, cidx, cidx.length(), 1 );

	rc.left = rc.right;
	rc.right = rc.left+line_width_[1];	
	FString s1 = s->time.ToString();
	DrawCenterText( cxt.cxt, cxt.black, rc, s1, s1.length(), 1 );
	rc.left = rc.right;
	rc.right = rc.left+line_width_[2];	
	DrawCenterText( cxt.cxt, cxt.black, rc,s->msg.c_str(), s->msg.length(), 1 );
	
	
	rc.left = rc.right+3;
	rc.right = rc.left+line_width_[3];	
	DrawCenterText( cxt.cxt, cxt.black, rc,s->msg.c_str(), s->msg.length(), 0 );
	
}
D2DListboxItem* D2DStatsuControlItem::Clone()
{
	return NULL; // new D2DStatsuControlItem(NULL);
}
//////// 
D2DStatsuControlItemTemplate::D2DStatsuControlItemTemplate()
{
	float w[]= { 50,160,50,230 };
	memcpy( line_width, w, sizeof(float)*4);

	title_height = TITLE_HEIGHT;

}
void D2DStatsuControlItemTemplate::DrawBegin( D2DContext& cxt, FSizeF& sz )
{
	D2DMatrix mat(cxt);
	mat_ = mat.PushTransform();	
	
	
	FString titles[] = { L"id",L"date",L"stat",L"message" };

	FRectF rc(0,0,0,title_height);

	for( int x = 0; x < sizeof(titles)/sizeof(FString); x++ )
	{
		rc.right = rc.left + line_width[x];

		FillRect(cxt,rc,cxt.gray);
		V4::DrawCenterText( cxt.cxt, cxt.white, rc, titles[x], titles[x].length(), 1 );

		rc.left = rc.right;

	}

	mat.PopTransform();
}

void D2DStatsuControlItemTemplate::DrawEnd( D2DContext& cxt, FSizeF& sz )
{
	float xx = 0;
	for( int x = 0; x < sizeof(line_width)/sizeof(float)-1; x++ )
	{
		xx += line_width[x];
		DrawLine( cxt.cxt, FPointF(xx,0),FPointF(xx,sz.height),1,cxt.gray );// 縦線	
	}
}
D2DListboxItem* D2DStatsuControlItemTemplate::Clone()
{
	return new D2DStatsuControlItem(line_width);
}
LRESULT D2DStatsuControlItemTemplate::WndProc(D2DWindow* d, UINT message, WPARAM wParam,LPARAM lParam )
{
	LRESULT ret = 0;

	switch( message )
	{
		case WM_LBUTTONDOWN:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			d->SetCapture(this,&pt);

			if ( md_ != -1 )
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			else
			{
				// タイトル内をクリック、ソート処理
				int j = -1;
				float w = 0;
				for( int i = 0; i < sizeof(line_width)/sizeof(float); i++ )
				{					
					w += line_width[i];
					if ( pt.x < w )
					{
						j = i;
						ret = 1;						
						break;
					}
				}
				d->ReleaseCapture();

				if ( j > -1 )
					OnClickTitle(j);
			}

			ret = 1;
		}
		break;
		case WM_MOUSEMOVE:
		{
			FPointF pt = mat_.DPtoLP( FPointF(lParam));

			auto h = d->GetCapture();
			if ( h == NULL )
			{
				int j = -1;

				float w = 0;

				for( int i = 0; i < sizeof(line_width)/sizeof(float)-1; i++ )
				{					
					w += line_width[i];
					if ( w-2 <= pt.x && pt.x <= w+2 )
					{
						j = i;
						ret = 1;
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
						break;
					}
					
				}
				md_ = j;
			}
			else if ( h == this )
			{
				FPointF pt1 = d->CapturePoint(pt);

				
				float cx = line_width[md_] + ( pt.x-pt1.x );
				line_width[md_] = max(1.0,cx);

				ret = 1;
				d->redraw_ = 1;
				
			}

		}
		break;
		case WM_LBUTTONUP:
		{
			if ( d->GetCapture() == this )
			{
				md_ = -1;
				d->ReleaseCapture();
				ret = 1;
			}
		}
		break;
	}	
	return ret;	
}
void D2DStatsuControlItemTemplate::OnClickTitle(int idx)
{
	int id = idx;

}