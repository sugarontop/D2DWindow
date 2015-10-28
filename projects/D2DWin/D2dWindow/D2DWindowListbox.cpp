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
#include "stdafx.h"
#include "D2DWindow.h"
#include "D2DWindowControl_easy.h"
#include "ItemLoopArray.h"
#include "D2DWindowListbox.h"
#include "D2DContextEx.h"
#include "gdi32.h"
//#include "MoveTarget.h"


#define CLASSNAME L"D2DWindow"
using namespace V4;
using namespace std::placeholders;

#define TITLE_BAR_HEIGHT 24
#define INITIAL_BUFFER_SIZE 1024 //64
#define OFFCX 3


#pragma region D2DListbox
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////
// ls: D2DListbox
// etcはdataに対応している。VectorToSerial
// ls->SetData( data, cnt );
//////////////////////////////////////
void D2DListboxItem::Draw( D2DContext& cxt, FSizeF& sz, int stat, void* etc )
{
	FRectF rc(OFFCX,0,sz.width-OFFCX,sz.height);

	FString* s = (FString*)data_.data;

	if ( stat & STAT::MOUSEMOVE )
	{
		FillRoundRect(cxt,cxt.factory(), rc, 3, data_.color_mouse_move_back );
		V4::DrawCenterText( cxt, data_.color_text, rc,s->c_str(), s->length(), 0 );
	}
	else if ( (stat & STAT::SELECTED) && (stat & STAT::CAPTURED) )
	{
		FillRoundRect(cxt, cxt.factory(), rc, 3, data_.color_selected_back );		
		V4::DrawCenterText( cxt, data_.color_selected_text, rc,s->c_str(), s->length(), 0 );

	}	
	else if ( stat & STAT::SELECTED )
	{		
		FillRoundRect(cxt, cxt.factory(), rc, 3, data_.color_selected_back );
		V4::DrawCenterText( cxt, data_.color_selected_text, rc,s->c_str(), s->length(), 0 );
	}
	else
		V4::DrawCenterText( cxt, data_.color_text, rc,s->c_str(), s->length(), 0 );
}
D2DListboxItem* D2DListboxItem::Clone()
{
	auto r = new D2DListboxItem();
	return r;

}
void D2DListboxItem::Bind( D2DWindow* win, BindData& data )
{
	data_ = data;

	//layout_.Release();
}
void D2DListboxItem::BindImage( D2DContextText& cxtt, FSizeF& sz )
{
	//FString* s = (FString*)data_;

	//FRectF rc(OFFCX,0,sz.width-OFFCX,sz.height);
	//stext_.CreateLayout(cxtt,rc,s->c_str(),s->length(), 0);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


D2DListbox::~D2DListbox()
{
	if ( data_fstring_ )
	{
		LONG_PTR* p = data_fstring_.get();

		for( UINT i = 0; i < data_cnt_; i++ )
		{
			FString* ps = (FString*)p[i];
			delete ps;			
		}
		data_fstring_ = NULL;
	}
	
	for( auto it = Items_.begin(); it != Items_.end(); ++it )
		delete (*it);
}

#define COLOR_MOUSE_MOVE D2RGBA(153,217,234,255)
#define COLOR_SELECTED D2RGBA(132,137,227,100)

void D2DListbox::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, float item_min_height,D2DListboxItem* template_item, LPCWSTR name, int id )
{
	D2DControl::InnerCreateWindow( parent,pacontrol,rc,stat,name, id );

	{
		item_min_height_ = item_min_height; // itemの最小高。デフォル値となる。個々のitemの高さを設定する場合は、SetItemHeightにて変更可。
		selected_idx_ = float_idx_ = idivN_ =  -1;
		data_cnt_ = 0;
		int cnt = (int)(rc.GetContentRect().Height()/item_min_height_) + 2; // 高さに依存


		D2DListboxItem default_item;
		if ( template_item == NULL )
			template_item = &default_item;

		for( int i = 0; i < cnt; i++ )
		{
			auto p = template_item->Clone(); // new D2DListboxItem();
			p->OnCreate( this );
	
			Items_.push_back( p );
		
			loopitems_.Add( new Item( (void*)p)); 
		
		}
		loopitems_.AddEnd();


		FRectFBoxModel rc1(rc.Width()-20,0, rc.Width(), rc.Height());
		bar_ = new D2DScrollbar();
		bar_->CreateWindow( parent, this, rc1, stat, L"listbox-scrollbar" );
		bar_->SetTotalSize( 1*item_min_height_ );

		bar_->target_control_ = this; // バーのメッセージを受けるオブジェクト

		brush_fore_ = ColorF(ColorF::Black);
		brush_back_ = ColorF(ColorF::White);
		brush_selected_ = COLOR_SELECTED;
		brush_floating_ = COLOR_MOUSE_MOVE;
		brush_selected_fore_ = ColorF(ColorF::White);
	}

	OnCreate();
}

void D2DListbox::OnCreate()
{
	buffer_size_ = INITIAL_BUFFER_SIZE;

}
void D2DListbox::Clear()
{
	if ( data_fstring_ )
	{
		FStringClear();
		return;
	}
	
	for( auto it = Items_.begin(); it != Items_.end(); ++it )
		delete (*it);
			
	Items_.clear();
	ItemsHeight_.clear();
	selected_idx_ = float_idx_ = -1;
	keymap_.clear();
}

void D2DListbox::FStringClear()
{
	if ( data_fstring_ )
	{
		LONG_PTR* p = data_fstring_.get();

		for( UINT i = 0; i < data_cnt_; i++ )
		{
			FString* ps = (FString*)p[i];
			delete ps;			
		}
		data_fstring_ = NULL;

		data_cnt_ = 0;
		buffer_size_ = INITIAL_BUFFER_SIZE;

		float_idx_ = -1;
		selected_idx_ = -1;

		bar_->Clear();
		UpdateScrollbar(bar_);
	}
}

void D2DListbox::UnSelectAll()
{
	selected_idx_ = -1;
	float_idx_ = -1;
	parent_->redraw_ = 1;
}
void D2DListbox::SetItemHeight( UINT id, float height )
{
	_ASSERT( id < ItemsHeight_.size() );
	
	ItemsHeight_[ id ] = height;
	
	// バーの高さを再計算
	float h = 0;
	for( auto& it : ItemsHeight_ )
	{
		h += it;
	}

	bar_->SetTotalSize( h );
}


/////////////////////////////////////////////////////////////////////////////////////
void D2DListbox::Expand()
{
	buffer_size_ = data_cnt_ * 2 ;

	LONG_PTR* p = new LONG_PTR[buffer_size_];
	ZeroMemory( p, sizeof(LONG_PTR)*buffer_size_ );
	auto new_data_fstring = std::shared_ptr<LONG_PTR>( p, std::default_delete<LONG_PTR[]>() );

//	for( int i = 0; i < data_cnt_; i++ )
//	{
//		new_data_fstring.get()[i] = data_fstring_.get()[i] ;
//	}

	memcpy( new_data_fstring.get(), data_fstring_.get(), sizeof(LONG_PTR)*data_cnt_);

	data_fstring_ = new_data_fstring;

	SetData( data_fstring_.get(), data_cnt_ );

}

void D2DListbox::AddString( LPCWSTR str )
{
	const UINT unitmax = buffer_size_;

	if ( data_cnt_ >= buffer_size_ )
	{
		Expand();
	}


	if ( data_fstring_ == NULL )
	{
		LONG_PTR* p = new LONG_PTR[unitmax];
		ZeroMemory( p, sizeof(LONG_PTR)*unitmax );
		data_fstring_ = std::shared_ptr<LONG_PTR>( p, std::default_delete<LONG_PTR[]>() );
	}

	FString* ps = new FString(str);
	data_fstring_.get()[ data_cnt_++ ] = (LONG_PTR)ps;

	SetData( data_fstring_.get(), data_cnt_ );
}
void D2DListbox::ClearString()
{
	//Clear();

	FStringClear();
}
void D2DListbox::UpdateString( int idx, LPCWSTR str )
{
	_ASSERT( idx < (int)data_cnt_ );

	FString* ps = (FString*)data_fstring_.get()[ idx ];
	delete ps;
	ps = new FString( str );

	data_fstring_.get()[ idx ] = (LONG_PTR)ps;

}
void D2DListbox::RemoveString( int idx )
{
	FString* ps = (FString*)data_fstring_.get()[ idx ];
	delete ps;

	for( UINT i = idx; i <data_cnt_-1; i++ )
	{
		data_fstring_.get()[ i ] = data_fstring_.get()[ i + 1 ];
	}
	data_fstring_.get()[ data_cnt_-1 ] = 0;
	
	data_cnt_--;
}
FString D2DListbox::GetString( int idx )
{
	_ASSERT( idx < (int)data_cnt_ );

	if ( idx < 0 )
		return L"";
	
	return *(FString*)data_fstring_.get()[ idx ];
}
///////////////////////////////////////////////////////////////////////////////////////////////

int D2DListbox::AddKeyValue( LPCWSTR key, LPCWSTR value )
{
	int idx = data_cnt_;

	AddString(value);
	
	keymap_[ key ] = idx;
	
	return idx;
}
FString D2DListbox::GetValue( LPCWSTR key )
{
	auto it = keymap_.find( key );

	if ( it != keymap_.end())
	{	
		int idx = it->second;
		return GetString(idx);
	}
	return L"";
}
void D2DListbox::ClearValue()
{
	keymap_.clear();
	ClearString();
}
void D2DListbox:: UpdateValue( LPCWSTR key, LPCWSTR str )
{
	auto it = keymap_.find( key );

	if ( it != keymap_.end())
	{	
		int idx = it->second;
		UpdateString( idx, str );
	}
}
void D2DListbox::RemoveValue( LPCWSTR key )
{
	auto it = keymap_.find( key );

	if ( it != keymap_.end())
	{	
		int idx = it->second;
		RemoveString( idx );
	}
}
FString D2DListbox::Getkey( int idx )
{
	for( auto& it : keymap_ )
	{
		if ( it.second == idx )
			return it.first;
	}
	return L"";	
}


void D2DListbox::SetData( LONG_PTR* datahead, UINT cnt )
{
	// static auto datahead = VectorToSerial(gar);
	// ls->SetData( datahead, gar.size());
	// ls->onselected_changed_ = [](D2DListbox* ls, int id, D2DListboxItem* item){ ... };

	_ASSERT( bar_ );

	serial_content_data_ = datahead;
	data_cnt_ = cnt;

	
	
	ItemsHeight_.clear();
	
	// 可変高のItemに対応するが、初期状態はすべて同じ高さ
	float h = 0;
	for( UINT i = 0; i < cnt; i++ )
	{
		float itemh = item_min_height_;		
		ItemsHeight_.push_back( itemh );

		h+= itemh;
	}
	bar_->SetTotalSize( h );

	/*ItemsHeight_.push_back( item_min_height_ );
	float h = item_min_height_ * cnt;
	bar_->SetTotalSize( h );*/
}

void D2DListbox::SetDataEx( std::shared_ptr<LONG_PTR> datahead, UINT cnt )
{
	buffer_data_ = datahead;

	SetData( datahead.get(), cnt );

}

bool D2DListbox::IsShowScrollbarV()
{
	return (loopitems_.Count() < (int)data_cnt_);
}

int D2DListbox::CalcdiNEx( float offy, float& h2 )
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

void D2DListbox::DrawContent( D2DContext& cxt, int cnt, float cx, float itemHeight, float offy, void* etc )
{
	auto ar = loopitems_.get(cnt);

	float h2 = 0;
	

	int idivN = CalcdiNEx( offy, h2 ); // 先頭の頭出し
	idivN_ = idivN;
	float offy1 = offy-  h2; 

	D2DMatrix mat(cxt);
	mat.PushTransform();
	mat.Offset( -scrollbar_off_.width, -offy1);
	mat.Offset( rc_.left, rc_.top );
	
	FSizeF itemsz( cx, itemHeight ); 
	auto cobj = parent_->GetCapture();

	D2DContextText& tt = cxt.cxtt;

	bool bcap = ( parent_->GetCapture() == this );

	// 色作成
	auto color_mouse_move = MakeBrsuh( parent_->cxt_, brush_floating_ );
	auto color_selected = MakeBrsuh( parent_->cxt_, brush_selected_ );
	auto color_selected_fore = MakeBrsuh(parent_->cxt_, brush_selected_fore_);
	auto color_text = MakeBrsuh(parent_->cxt_, brush_fore_ );
				
	for( auto it = ar.begin(); it < ar.end(); ++it )
	{		
		Item* pd = (Item*)(*it);
		int data_idx = pd->idx() + idivN;
		
		D2DListboxItem* t = (D2DListboxItem*)(pd->t_);

		if ( data_idx < (int)data_cnt_ )
		{
			itemsz.height = itemHeight; // t->Draw内で再設定される場合があるため

			D2DListboxItem::BindData bd;
			bd.data = (void*)(serial_content_data_[data_idx]);
			bd.color_mouse_move_back = color_mouse_move;
			bd.color_selected_back = color_selected;
			bd.color_selected_text = color_selected_fore;
			bd.color_text = color_text;

			t->Bind( parent_, bd ); //(void*)(serial_constent_data_[data_idx]) );
			t->BindImage( tt, itemsz );
			
			if ( t == cobj )
				t->Draw(cxt, itemsz, (stat_ | (int) STAT::CAPTURED), etc);			
			else if ( selected_idx_ == data_idx )
			{
				if ( bcap )
					t->Draw(cxt, itemsz, (stat_ | (int)STAT::SELECTED|(int)STAT::CAPTURED), etc);
				else
					t->Draw(cxt, itemsz, (stat_ | (int) STAT::SELECTED), etc);
			}
			else if (float_idx_ == data_idx)
				t->Draw(cxt, itemsz, (stat_ | (int) STAT::MOUSEMOVE), etc);
			else
				t->Draw(cxt, itemsz, stat_, etc);
		}

		mat.Offset( 0,itemsz.height );
	}

	mat.PopTransform();
}



LRESULT D2DListbox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;	
	if (!VISIBLE(stat_))
		return ret;

	switch( message )
	{
		//case WM_D2D_PAINT:
		case WM_PAINT:
		{
			auto& cxt = d->cxt_;

			cxt.cxt->GetTransform( &mat_ );
										
			const FRectF rcc = rc_.GetBorderRect();
			D2DRectFilter f(cxt, rcc);		

			FillRectangle( cxt, rcc, MakeBrsuh(cxt, brush_back_));
							
			{
				int cnt = loopitems_.Count();

				// Listbox内の表示
				if ( IsShowScrollbarV() )
				{					
							
					DrawContent( cxt, cnt, rcc.Width()-bar_->GetRect().Width()+OFFCX, item_min_height_, scrollbar_off_.height, this );										
					{
						// スクロールバー
						D2DMatrix mat(cxt);
						mat.PushTransform();
						mat.Offset( rcc.left, rcc.top );
						bar_->WndProc(d, message, wParam, lParam );
						mat.PopTransform();
					}
				}
				else
				{
					DrawContent( cxt, cnt, rcc.Width()-OFFCX, item_min_height_, scrollbar_off_.height,this );					
				}
			}
					
			cxt.cxt->DrawRectangle(rcc, cxt.black, 1);
			
						
		}
		break;
		
		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE && ENABLE(stat_) )
			{				
				//Escape( d ); // ReleaseCaputerされる


				//parent_->SetCapture(this);

			}
			else if ( wParam == VK_RETURN && ENABLE(stat_) )
			{
				//btn_->WndProc(parent_,message,wParam,lParam);
			}

			ret = parent_control_->KeyProc(this,message,wParam,lParam );
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		{
			if ( stat_ & VISIBLE )
			{
				FPointF pt(lParam);
				pt = mat_.DPtoLP( pt );			

				FRectF rc(rc_);
				rc.right -= bar_->GetRect().Width();


				if ( message == WM_LBUTTONDOWN )
				{
					//ATLTRACE( L"left=%f top %f this=%x\n", mat_._31, mat_._32, this );
				}

				if ( message == WM_LBUTTONDOWN )
				{					
					if ( rc.PtInRect(pt) )
					{
						pt.x -= rc_.left;
						pt.y -= rc_.top;
						
						pt.y += scrollbar_off_.height;
						pt.x += scrollbar_off_.width;
						
						bool bCombobox;
						OnItemSelected( pt, bCombobox ); // 内部でcaptureしている場合あり

						if ( !bCombobox ) // comboboxはすぐlbが非表示になる
							d->SetCapture(this);
						
						ret = 1; 
					}
					else
					{
						if ( d->GetCapture() == this )
						{
							d->ReleaseCapture();
						}
						
						ret = 0;
					}
				}
				else if ( message == WM_MOUSEMOVE )
				{
					if ( rc.PtInRect(pt) )
					{
						pt.x -= rc_.left;
						pt.y -= rc_.top;
						
						pt.y += scrollbar_off_.height;
						pt.x += scrollbar_off_.width;
						

						OnItemMouseFloat( pt );
						ret = 1;
					}	
					else if ( float_idx_ > -1 )
					{
						float_idx_ = -1;
						d->redraw_ = 1;
					}

					
				}
				else if ( message == WM_LBUTTONUP ) 
				{
					
					ret = 0;
					if ( d->GetCapture() == this )
						{
							d->ReleaseCapture();
						}

				}


				if (IsShowScrollbarV() )
				{
					if ( ret == 0 || message == WM_MOUSEMOVE )
					{												
						ret = bar_->WndProc(d, message, wParam, lParam );
					}
				}
			}
			
		}
		break;
		case WM_MOUSEWHEEL:
		{
			FPointF pt = mat_.DPtoLP( lParam );		

			if ( rc_.PtInRect(pt))
			{
				auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				float y = bar_->info_.position;
				y += ( zDelta > 0 ? -10: 10);


				bar_->info_.position = max(0,min(y, bar_->info_.total_height)); // 違う

				UpdateScrollbar(bar_);

				d->redraw_ = 1;
				return 1;
			}
		}
		break;
		default :
			if ( WM_D2D_APP_USER <= message )
			{
				if ( OnWndProcExt_ )
					ret = OnWndProcExt_( this, message, wParam, lParam );
			}
	}

	return ret;
}

void D2DListbox::OnActive( bool bActive )
{
	if ( !bActive )
	{
		float_idx_ = -1;
		selected_idx_ = -1;
	}
}

int D2DListbox::CalcIdx( const FPointF& mousept )
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
void D2DListbox::OnItemMouseFloat( const FPointF& mousept )
{
	int float_idx = CalcIdx(mousept);

	if ( float_idx_ != float_idx )
	{
		parent_->redraw_ = 1;
		float_idx_ = float_idx;

		//TRACE( L"float point (%f %f) float id=%d selected id=%d \n" , mousept.x, mousept.y, float_idx_, selected_idx_ );
	}
}

void D2DListbox::OnItemSelected( const FPointF& mousept, bool& bCombobox )
{
	if ( idivN_ < 0 || Items_.size() == 0 ) 
		return ;
	if ( data_cnt_ == 0 )
		return;

	selected_idx_ = CalcIdx(mousept);
	float_idx_ = -1;

	int item_idx = selected_idx_ - idivN_; // (int)(scrollbar_off_.height / item_height_ );
	
	
	_ASSERT( item_idx < (int)Items_.size());


	bCombobox = false;

	TRACE( L"click %f,%f  --> idx=%d itemidx=%d\n", mousept.x, mousept.y, selected_idx_, item_idx );

	// selected_idx_ is 0 start, data index. Size is equal to sizeof(datas) / sizeof(data).
	// item_idx is is 0 start, viewitem index.

	//parent_control_->WndProc( parent_, WM_D2D_EVLISTBOX_SELECTED_CHANGED, selected_idx_, (LPARAM)this);

	if ( onselected_changed_ )
		bCombobox = onselected_changed_( this, selected_idx_, Items_[item_idx] ); // Comboboxの場合だけtrueを返す。
	else
		SendMessage( WM_D2D_EVSELECT_CHEANGED, MAKEWPARAM( selected_idx_, id_ ), (LPARAM)this);


}

void D2DListbox::UpdateScrollbar(D2DScrollbar* bar)
{
	if ( bar->info_.bVertical )	
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c; // offセットの絶対値
	else
		scrollbar_off_.width = bar->info_.position;
}
void D2DListbox::ShowWindow( bool bShow )
{
	if ( bShow )
	{
		stat_ |= STAT::VISIBLE;
		bar_->Stat( bar_->GetStat() | STAT::VISIBLE);

		bar_->info_.stat = D2DScrollbarInfo::STAT::NONE;
		bar_->info_.position = 0;
		scrollbar_off_.height  = 0;
	}
	else
	{
		stat_ &= ~STAT::VISIBLE;
		//bar_->stat_ &= ~STAT::VISIBLE;

		bar_->Stat( bar_->GetStat() & ~STAT::VISIBLE);
	}
}
void D2DListbox::SetParameters(const std::map<std::wstring, VARIANT>& prms)
{
	for (auto& it : prms)
	{
		auto& key = it.first;
		if (key == L"forecolor")
		{
			brush_fore_ = VariantColor(it.second);
		}
		else if (key == L"backcolor")
		{
			brush_back_ = VariantColor(it.second);		
		}
		else if (key == L"selectedcolor")
		{
			brush_selected_ = VariantColor(it.second);
		}
		else if (key == L"floatingcolor")
		{
			brush_floating_ = VariantColor(it.second);
		}
		else if (key == L"selectedforecolor")
		{
			brush_selected_fore_ = VariantColor(it.second);
		}
	}

}


#pragma endregion



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DCombobox
D2DCombobox::D2DCombobox()
{

}
D2DCombobox::~D2DCombobox()
{
	delete head_;
}
FString D2DCombobox::GetText()
{
	return GetListbox()->GetString( selected_idx_ );
}

void D2DCombobox::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, FRectFBoxModel rc, int stat, D2DListboxItem* template_item, FSizeF btnSize, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );

	btnSize_ = btnSize;
	view_height_ = btnSize_.height;

	FRectFBoxModel rcb(rc.Width()-btnSize_.width,0, rc.Width(),view_height_ );

	float mcy = btnSize.height*0.25;		
	float mcx = btnSize.width*0.25;		
	rcb.Padding_.SetRL(mcx);
	rcb.Padding_.SetTB(mcy);
	

	btn_ = new D2DButton();
	btn_->CreateWindow( parent, this, rcb, stat , L"__btn" );
	btn_->OnClick_ = std::bind( &D2DCombobox::ComboButtonClick, this, _1 ); 
	
	CreateTraiangle(parent->cxt_.factory(),2, mcx*1.3,mcy,&btn_path_);
	auto triangle = btn_path_.p;

	// ボタンの描画
	//btn_->ondraw_ = [triangle](D2DContext& cxt, const FRectFBoxModel& rc, int stat, const FString& s)
	//{		
	//	D2DMatrix m(cxt);
	//	m.PushTransform();

	//	if ( stat & MOUSEMOVE )			
	//		DrawFillRectTypeS( cxt, rc.GetBorderRect(),cxt.halftone );
	//	else
	//		DrawFillRectTypeS( cxt, rc.GetBorderRect(),cxt.ltgray );
	//		

	//	FRectF rc1 = rc.GetContentRect();
	//	m.Offset(rc1.left, rc1.top);

	//	m.Offset(2,3);
	//	if ( stat & CAPTURED )
	//		cxt.cxt->FillGeometry( triangle, cxt.white );
	//	else
	//		cxt.cxt->FillGeometry( triangle, cxt.black );

	//	m.PopTransform();

	//};

	_ASSERT( btnSize.width > 0 && btnSize.height > 0 );

	FRectFBoxModel rclb( rc.GetContentRectZero() );
	

	rclb.BorderWidth_ = 1.0f;

	D2DListboxItem default_item;
	if ( template_item == NULL )
		template_item = &default_item;


rclb.Offset(0,-2);

	lb_ = new D2DListbox();
	lb_->CreateWindow( parent,this , rclb, stat, btnSize_.height,template_item, L"__LB" );
	lb_->onselected_changed_ = std::bind( &D2DCombobox::OnSelectedChanged, this, _1, _2 );

	selected_idx_ = -1;
	bShowListbox_ = false;
	bMouseEnter_ = false;

	head_ = template_item->Clone();

	brush_fore_ = ColorF(ColorF::Black);
	brush_back_ = ColorF(ColorF::White);
	brush_selected_ = COLOR_SELECTED;
	brush_floating_ = COLOR_MOUSE_MOVE;
	brush_selected_fore_ = ColorF(ColorF::White);
}

void D2DCombobox::SetSelectedIdx( int idx )
{
	_ASSERT( idx >= -1 );
	selected_idx_ = idx;
}

bool D2DCombobox::OnSelectedChanged( D2DListbox* ls,int idx )
{
	Escape(parent_);

	if ( selected_idx_ != idx )
	{
		selected_idx_ = idx;

		if ( onselected_changed_ )
			onselected_changed_( this, idx );

		WPARAM wp = MAKEWPARAM( selected_idx_, id_ );
		SendMessage( WM_D2D_EVSELECT_CHEANGED, wp, (LPARAM)this);
		
	}


	return true;
}
void D2DCombobox::Escape(D2DWindow* d)
{
	if ( d->GetCapture() == this )
	{
		lb_->ShowWindow( false );
		d->ReleaseCapture();
		bShowListbox_ = false;
	}
}
void D2DCombobox::ComboButtonClick(D2DButton* btn)
{	
	if ( !bShowListbox_ )
	{
		lb_->ShowWindow(true);

		parent_->redraw_ = 1;

		
		if ( parent_->GetCapture() != this )
			parent_->SetCapture( this );
		
		bShowListbox_ = true;
	}
	else
		Escape( parent_ );
	
}
LRESULT D2DCombobox::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	switch( message )
	{
		
		case WM_PAINT:
		case WM_D2D_PAINT:		
		{
			auto cxt = d->cxt_;

			if ( stat_ & VISIBLE )		
			{												
				if ( message == WM_D2D_PAINT )
					cxt.cxt->SetTransform( &mat_ );
				else				
					cxt.cxt->GetTransform( &mat_ );
				
				
				FRectF rcb = rc_.GetBorderRect();
				// 1行目とボタンの外枠
				
				FRectF rc( rcb.left, rcb.top, rcb.right, rcb.top+view_height_ );
								
				auto br = (stat_&FOCUS ? cxt.red : cxt.black );
				
				if ( stat_ & BORDER )
				{
					//DrawFillRect( cxt, rc, br, cxt.white, 1 );
					DrawControlRect(cxt, rc, (ID2D1Brush*) br, (ID2D1Brush*) cxt.white);
				}
				// Listbox内の表示
				// スクロールバー
				D2DMatrix mat(cxt);
				mat.PushTransform();
				mat.Offset( rcb.left, rcb.top );

				auto color_mouse_move = MakeBrsuh(parent_->cxt_, brush_floating_);
				auto color_selected = MakeBrsuh(parent_->cxt_, brush_selected_);
				auto color_selected_fore = MakeBrsuh(parent_->cxt_, brush_selected_fore_);
				auto color_text = MakeBrsuh(parent_->cxt_, brush_fore_);



				if ( selected_idx_ > -1 )
				{
					LONG_PTR* data = lb_->GetData();

					D2DListboxItem::BindData bd;
					bd.data = (void*)data[selected_idx_];
					bd.color_mouse_move_back = color_mouse_move;
					bd.color_selected_back = color_selected;
					bd.color_selected_text = color_selected_fore;
					bd.color_text = color_text;

					head_->Bind( parent_, bd ); // (void*)data[selected_idx_] );
					head_->Draw( cxt, FSizeF( rcb.Width()-btnSize_.width, view_height_), 0,0 );
				}


				btn_->WndProc(d,WM_PAINT,wParam,lParam);

				mat.Offset( 0, btnSize_.height );
				
				
				mat.Offset( 0, 1 );

								
				if ( bShowListbox_ )				
					lb_->WndProc(d, WM_PAINT, wParam, lParam );

				mat.PopTransform();
			}
		}
		break;
		case WM_DESTROY:
		{
			lb_->WndProc(d, message, wParam, lParam ); 
		}
		break;
		case WM_LBUTTONDOWN:
		{		
			if ( ENABLE(stat_))
			{
			
				FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); // 親座標で変換
				bool bInner = rc_.PtInRect2( pt2 );
							
				if ( d->GetCapture() == this )
				{				
					if ( !bInner )
					{
						// 枠外なので
						Escape( d );
					}

					ret = 1;
				}
				else if (bInner)
				{
					ret = btn_->WndProc(d, message, wParam, lParam );
				}

				if ( bShowListbox_ ) //&& ret == 0 )
					ret = lb_->WndProc(d, message, wParam, lParam ); 				
			}			
			
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( ENABLE(stat_))
			{
				FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); // 親座標で変換
				if ( rc_.PtInRect2( pt2 ) || bMouseEnter_ )
				{			
					bMouseEnter_ = true;

					ret = btn_->WndProc(d, message, wParam, lParam ); 

					if ( bShowListbox_&& ret == 0 )
						ret = lb_->WndProc(d, message, wParam, lParam ); 
				}
				else
				{
					bMouseEnter_ = false;
				}

			} 
		}
		break;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if ( ENABLE(stat_))
			{
				FPointF pt2 = mat_.DPtoLP( FPointF(lParam)); // 親座標で変換
				if ( rc_.PtInRect2( pt2 ) )
				{			
					ret = btn_->WndProc(d, message, wParam, lParam ); 

					if ( bShowListbox_&& ret == 0 )
						ret = lb_->WndProc(d, message, wParam, lParam ); 
				}
			}
		}
		break;
		case WM_KEYDOWN:
		{
			if ( wParam == VK_ESCAPE && ENABLE(stat_) )
			{				
				Escape( d ); // ReleaseCaputerされる


				parent_->SetCapture(this);

			}
			else if ( wParam == VK_RETURN && ENABLE(stat_) )
			{
				btn_->WndProc(parent_,message,wParam,lParam);
			}

			ret = parent_control_->KeyProc(this,message,wParam,lParam );
		}
		break;
		case WM_KEYUP:
		{
			if ( wParam == VK_RETURN && ENABLE(stat_) )
			{
				btn_->WndProc(parent_,message,wParam,lParam);
			}			
		}
		break;
		case WM_D2D_EVSELECT_CHEANGED:
		{
			

			int a = 0;
		}
		break;

	}

	if ( WM_D2D_APP_USER <= message )
	{
		if ( OnWndProcExt_ )
			ret = OnWndProcExt_( this, message, wParam, lParam );
	}

	return ret;
}
void D2DCombobox::SetData( LONG_PTR* datahead, UINT cnt )
{
	lb_->SetData( datahead, cnt );

}
void D2DCombobox::StatActive(bool bActive)
{

	/*if ( bActive )
	{
		parent_->SetCapture( btn_ );	
		btn_->StatActive(bActive);
	}*/


	if ( bActive )
		stat_ |= FOCUS;		
	else
		stat_ &= ~FOCUS;
}

void D2DCombobox::Clear()
{

	selected_idx_ = -1;
	GetListbox()->ClearString();
	//GetListbox()->Clear();

}

void D2DCombobox::OnCreate()
{
	

}
#pragma endregion



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region D2DListboxItemEx
//D2DListboxItemEx::D2DListboxItemEx(D2DControls* ctrls )
//{
//	ctrls_ = ctrls;
//
//	height_ = ctrls->GetRect().Size().height;
//}
//
//
//D2DListboxItem* D2DListboxItemEx::Clone()
//{
//	return new D2DListboxItemEx(ctrls_);
//}

#pragma endregion
