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
