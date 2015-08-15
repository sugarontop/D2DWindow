#pragma once

#include <list>
#include <map>

template <class T>
class CUndoInfoStack 
{
	protected :
		CUndoInfoStack( int maxsize ):m_max_size(maxsize){};
		~CUndoInfoStack(){ flush(); }
	public :
		void push( void* addr, T t )
		{
			if ( m_map.find( addr ) == m_map.end() ) 
			{
				m_map[addr] = new std::list<T>();
			}

			std::list<T>* ls = m_map[addr];
			ls->push_back( t );
			
			while ( (int)ls->size() > m_max_size )
			{
				ls->pop_front();
			}						
								
		}
		
		T pop( void* addr )
		{
			std::list<T>* ls = m_map[addr];
			T t = ls->back();			
			ls->pop_back();
			
			return t;		
		}
		int size( void* addr )
		{
			return (int)m_map[addr]->size();
		}			 
		void flush()
		{
			std::map<void*, std::list<T>* >::iterator it = m_map.begin();
			
			while( it != m_map.end() )
			{
				std::list<T>* ls = it->second;
				
				delete ls;
				
				it++;
				
			}
			
			m_map.clear();
		
		}
		
		static CUndoInfoStack<T>& FactorySingleton();
	
	protected :
		
		std::map<void*, std::list<T>* > m_map;
		int m_max_size;
		


};



class CUndoInfo
{
	public :
		CUndoInfo(){};
		enum TYPE{ DEL, ADD };
		TYPE m_type;
		int m_textPosition;
		int m_length;
		std::wstring m_text;
};

typedef CUndoInfoStack<CUndoInfo>	CUndoInfoStackEx;	


class CSelected
{
	public :
		CSelected( int textPostion ):m_textposition_start(textPostion),m_textposition_end(textPostion){}

		void SetEnd( int textPosition ){ m_textposition_end = textPosition; }
		int StartPostion(){ return min(m_textposition_start,m_textposition_end); }
		int Length(){ return abs(m_textposition_start-m_textposition_end); }
			
	protected :
		int m_textposition_start;
		int m_textposition_end;
};