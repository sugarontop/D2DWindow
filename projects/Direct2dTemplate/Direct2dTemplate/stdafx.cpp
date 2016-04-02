#include "stdafx.h"


#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")

#ifdef USE_ID2D1DEVICECONTEXT
#pragma comment(lib,"d3d11.lib")
#endif
//_bstr_t BStr::spc_ = L"";