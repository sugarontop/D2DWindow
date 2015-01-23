#pragma once

#include <unordered_map>

namespace V4 {


typedef UINT CHDL;


class D2DControl;
class ControlHandle
{
	public :
		ControlHandle(){}
		CHDL CreateControlHandle(D2DControl* ctrl);	
		void DeleteControlHandle( CHDL id );	
		D2DControl* GetFromControlHandle(CHDL id );	
		D2DControl* GetFromControlName(LPCWSTR nm );

	private :
		std::unordered_map<CHDL,D2DControl*> m1_;
		std::unordered_map<std::wstring,CHDL> m2_;
		static CHDL handle_;
	
};

};

