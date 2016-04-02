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


#include <string>
#include <map>
#include <vector>
#include <memory>

// The msxml6.dll is included in .NETframework3.0.

#import <msxml6.dll>	// make msxml6.tlh, msxml6.tli 
namespace V4 {

class XMLDOMDocument6
{
	public :
		XMLDOMDocument6(){}
		bool Load( LPCWSTR url );
		bool LoadXML( BSTR xml );

		bool Create();

		MSXML2::IXMLDOMElementPtr Root(){ return doc->GetdocumentElement();}

		static long ParsePropertys( MSXML2::IXMLDOMNode* nd, std::map<std::wstring,std::wstring>& ret );
		static long ParseList( MSXML2::IXMLDOMNodeList* nd, std::vector<MSXML2::IXMLDOMNodePtr>& ls );

	public :
		MSXML2::IXMLDOMDocument3Ptr doc; // MSXML2::IXMLxxxPtrはスマートポインタ
};


// etc c function //////////////////////////////////////////////////////////

std::vector<_bstr_t> parse_candidate( _bstr_t s );
_bstr_t GetCurrentDirectory();


// vectorをshred_ptrを使った一連の配列へ変更
template<typename T, typename S>
std::shared_ptr<T> vector_to_sharedptr( std::vector<S>& ar )
{
	T* v = new T[ar.size()];
	int i = 0;

	for( auto& it : ar ) v[i++] = it;

	return std::shared_ptr<T>( v , std::default_delete<T[]>());
}

};
