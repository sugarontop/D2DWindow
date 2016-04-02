#pragma once

namespace V4 {

class Item
{
	friend class ItemLoopArray;
	public :
		Item():idx_(-1),next_(0),prev_(0),t_(0){};
		Item(void* t ):t_(t){}
		virtual ~Item(){}

		int idx(){ return idx_; }
	private :		
		int idx_;
		Item* next_;
		Item* prev_;
	public :
		void* t_;
};

class ItemLoopArray
{
	public :
		ItemLoopArray():head_(0),active_(0),cnt_(0){};
		virtual ~ItemLoopArray(){ Clear(); }

		int Count(){ return cnt_; }
		void Add( Item* p )
		{
			int i = 0;

			if ( head_ == NULL )
			{
				head_ = p;
				head_->idx_ = i;
			}
			else
			{
				Item* pt = head_;

				while( pt->next_ ) 
					pt = pt->next_;

				pt->next_ = p;

				p->idx_ = pt->idx_ + 1;
			}		
			++cnt_;
		}
		void AddEnd()
		{
			Item* pt = head_;
			while (pt )
			{
				Item* p1 = pt->next_;
				
				if ( p1 )
					p1->prev_ = pt;
				else
				{
					head_->prev_ = pt;
					pt->next_ = head_;
					break;
				}

				pt = p1;
			}

			active_ = head_;
		}
		void Clear()
		{
			Item* pt = head_;
			while (pt )
			{
				Item* dp = pt;
				pt = pt->next_;


				if ( head_ != dp )
					delete dp;
				else
					head_->next_ = NULL;
			}


			delete head_;
			head_ = NULL;
			active_ = NULL;
			cnt_ =0;
		}


		std::vector<Item*> get( int cnt )
		{			
			std::vector<Item*> ar(cnt);

			auto p = active_;			
			for( int i = 0; i < cnt; i++ )
			{
				ar[i]= p;
				p = p->next_;
			}
			
			return ar;
		}
		/*void Up()
		{
			auto p = active_;			
			active_ = active_->next_;
			p->idx_ = p->prev_->idx_ + 1;			
		}
		void Down()
		{
			auto p = active_;			
			active_ = active_->prev_;
			active_->idx_ = p->idx_ - 1;			
		}*/

	private :
		Item* head_;
		Item* active_;
		int cnt_;
};
////////////////////////////////
// すべてのデータを++,--でアクセスする
template <typename T>
std::shared_ptr<LONG_PTR> VectorToSerial( const std::vector<T*>& ar_)
{
	LONG_PTR* array_data = new LONG_PTR[ar_.size()];
			
	for( UINT i = 0; i < ar_.size(); i++ )
	{
		array_data[i] = (LONG_PTR)ar_[i];
	}
	return std::shared_ptr<LONG_PTR>( array_data );
} 
template <typename T>
std::shared_ptr<LONG_PTR> VectorToSerialEx( const std::vector<T*>& ar_)
{
	LONG_PTR* array_data = new LONG_PTR[ar_.size()];
			
	for( UINT i = 0; i < ar_.size(); i++ )
	{
		array_data[i] = (LONG_PTR)ar_[i];
	}
	return std::shared_ptr<LONG_PTR>( array_data, std::default_delete<LONG_PTR[]>());
} 

template <typename T>
std::shared_ptr<LONG_PTR> VectorToSerialEx2( const std::vector<std::shared_ptr<T>>& ar_)
{
	LONG_PTR* array_data = new LONG_PTR[ar_.size()];
			
	for( UINT i = 0; i < ar_.size(); i++ )
	{
		array_data[i] = (LONG_PTR)ar_[i];
	}
	return std::shared_ptr<LONG_PTR>( array_data, std::default_delete<LONG_PTR[]>());
} 


////////////////////////////////


template<class T>
class Values
{
	public :
		Values():cnt_(0),values_(nullptr){}
		Values( int cnt, T* values )
		{
			cnt_ = cnt;
			values_ = new T[cnt];
			memcpy( values_, values, sizeof(T)*cnt);
		}
		~Values()
		{
			clear();
		}

		Values& operator=( std::vector<T>& ar )
		{
			int i = 0;			
			values_ = new T[ar.size()];
			for( auto& it : ar )
				values_[i++] = it;
			cnt_ = i;
			return *this;
		}
		void uniqeVaule( int cnt, T val )
		{
			cnt_ = cnt;
			values_ = new T[cnt_];
			for( int i=0;i < cnt; i++ )
				values_[i] = val;

		}

		void clear() { delete [] values_; cnt_=0;}
		int count() const { return cnt_; }
		T* get(){ return values_; }

		T operator[](int idx) const
		{			
			_ASSERT( idx < cnt_ );
			return values_[idx];
		}

	private :
		T* values_;
		int cnt_;
};




};
