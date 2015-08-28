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


・ｿ// DDDTestJS.cpp : 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ繧ｨ繝ｳ繝医Μ 繝昴う繝ｳ繝医ｒ螳夂ｾｩ縺励∪縺吶・

#include "stdafx.h"
#include "D2DTestJS.h"
#include "D2DWin.h"
#include "s1.h"
#include "JsrtWrapper.h"
#include <Commdlg.h>
#define MAX_LOADSTRING 100

// 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚:
HINSTANCE hInst;                                // 迴ｾ蝨ｨ縺ｮ繧､繝ｳ繧ｿ繝ｼ繝輔ぉ繧､繧ｹ
WCHAR szTitle[MAX_LOADSTRING];                  // 繧ｿ繧､繝医Ν 繝舌・縺ｮ繝・く繧ｹ繝・
WCHAR szWindowClass[MAX_LOADSTRING];            // 繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え 繧ｯ繝ｩ繧ｹ蜷・

// 縺薙・繧ｳ繝ｼ繝・繝｢繧ｸ繝･繝ｼ繝ｫ縺ｫ蜷ｫ縺ｾ繧後ｋ髢｢謨ｰ縺ｮ螳｣險繧定ｻ｢騾√＠縺ｾ縺・
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	CoInitialize(0);
	_tsetlocale(0, _T("japanese"));
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // 繧ｰ繝ｭ繝ｼ繝舌Ν譁・ｭ怜・繧貞・譛溷喧縺励※縺・∪縺吶・
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_D2DTESTJS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ蛻晄悄蛹悶ｒ螳溯｡後＠縺ｾ縺・
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D2DTESTJS));

    MSG msg;

    // 繝｡繧､繝ｳ 繝｡繝・そ繝ｼ繧ｸ 繝ｫ繝ｼ繝・
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  髢｢謨ｰ: MyRegisterClass()
//
//  逶ｮ逧・ 繧ｦ繧｣繝ｳ繝峨え 繧ｯ繝ｩ繧ｹ繧堤匳骭ｲ縺励∪縺吶・
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D2DTESTJS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_D2DTESTJS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   髢｢謨ｰ: InitInstance(HINSTANCE, int)
//
//   逶ｮ逧・ 繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ 繝上Φ繝峨Ν繧剃ｿ晏ｭ倥＠縺ｦ縲√Γ繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え繧剃ｽ懈・縺励∪縺吶・
//
//   繧ｳ繝｡繝ｳ繝・
//
//        縺薙・髢｢謨ｰ縺ｧ縲√げ繝ｭ繝ｼ繝舌Ν螟画焚縺ｧ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ 繝上Φ繝峨Ν繧剃ｿ晏ｭ倥＠縲・
//        繝｡繧､繝ｳ 繝励Ο繧ｰ繝ｩ繝 繧ｦ繧｣繝ｳ繝峨え繧剃ｽ懈・縺翫ｈ縺ｳ陦ｨ遉ｺ縺励∪縺吶・
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縺ｫ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ蜃ｦ逅・ｒ譬ｼ邏阪＠縺ｾ縺吶・

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

struct Global
{
	~Global()
	{
		DDDestroyWindow(g_handle);
	}
	D2DWin g_handle;
};

Global global;


D2Ctrl OutputControl;

using namespace V4;
namespace CHAKRA2 {
	void Initialize(JSRuntime& grun, JSContext& rcxt);
	bool JavaScriptRun(JSContext& r, LPCWSTR src);
	void Destroy(JSRuntime);

	extern wstring ErrorMsg;
};
JSContext gcxt;
JSRuntime grun;
void EportFunctions();
std::map<wstring, D2Ctrl> wmap;
std::map<wstring, D2Ctrls> wbase;

//
//  髢｢謨ｰ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  逶ｮ逧・    繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え縺ｮ繝｡繝・そ繝ｼ繧ｸ繧貞・逅・＠縺ｾ縺吶・
//
//  WM_COMMAND  - 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ 繝｡繝九Η繝ｼ縺ｮ蜃ｦ逅・
//  WM_PAINT    - 繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え縺ｮ謠冗判
//  WM_DESTROY  - 荳ｭ豁｢繝｡繝・そ繝ｼ繧ｸ繧定｡ｨ遉ｺ縺励※謌ｻ繧・
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 驕ｸ謚槭＆繧後◆繝｡繝九Η繝ｼ縺ｮ隗｣譫・
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC 繧剃ｽｿ逕ｨ縺吶ｋ謠冗判繧ｳ繝ｼ繝峨ｒ縺薙％縺ｫ霑ｽ蜉縺励※縺上□縺輔＞...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_CREATE:
	{
		auto DDDControlCreate = [hWnd](D2DWin w)
		{
			


			auto t = DDMkTopControls(w, V4::FRectF(0, 0, 0, 0), NONAME);

			auto tab = DDMkControls(t, V4::FRectF(0, 0, 0, 0), L"tab", NONAME);
			auto t0 = DDGetPage(tab, 0);

			wbase[L"page0"] = t0;


			V4::FRectFBoxModel rctxt(20,60, V4::FSizeF(500, 450));
			rctxt.Padding_.Set(3);

			rctxt.Padding_.r = 15; // scroll bar

			auto txm = DDMkTextbox(t0, rctxt, 1, NONAME);
			DDSetText(txm, L"var a=1+2; echo(a); //Script window. Input Javascript source. ..");

			std::map<std::wstring,VARIANT> prm;
			prm[L"forecolor"] = _variant_t((LONG)D2DRGBADWORD(0,0,200,255));
			prm[L"backcolor"] = _variant_t((LONG)D2DRGBADWORD(190, 190, 190, 155));
						

			DDSetParameter( txm, prm );


			auto otxm = DDMkTextbox(t0, V4::FRectF(150, 520, V4::FSizeF(500, 150)), 1, L"output");
			DDSetText(otxm, L"output");
			OutputControl = otxm;

			wmap[L"output"] = otxm;


			auto btnClear = DDMkButton(t0, FRectFBM(20, 600, V4::FSizeF(100, 30)), L"Clear"); 
			DDEvent0( D2EVENT0_MODE::CLICK, btnClear, [txm, otxm](D2EVENT0_MODE ev, D2Ctrl){
				DDSetText( txm, L"" );
				DDSetText(otxm, L"");
			});
			
			


			auto btnRun = DDMkButton(t0,FRectFBM(20,550,V4::FSizeF(100,30)), L"Run" );
			DDEvent0(D2EVENT0_MODE::CLICK, btnRun, [txm, otxm](D2EVENT0_MODE ev, D2Ctrl)
			{
				BSTR src = DDGetText(txm);

				try
				{ 

					if (!CHAKRA2::JavaScriptRun(gcxt, src))
					{
						
						DDSetText(otxm, CHAKRA2::ErrorMsg.c_str());
					}
				}
				catch( V4::FString er )
				{
					DDSetText( otxm, er );
				}

				::SysFreeString(src);

			});
					

			auto btnLoad = DDMkButton(t0, FRectFBM(20, 650, V4::FSizeF(100, 30)), L"Load");
			DDEvent0(D2EVENT0_MODE::CLICK, btnLoad, [hWnd,txm](D2EVENT0_MODE ev, D2Ctrl)
			{
				/*OPENFILENAME ofn;

				WCHAR cb[64]; lstrcpy( cb, L"js" );
				WCHAR cb2[MAX_PATH]; lstrcpy(cb2, L"*.js");
				WCHAR cb3[64]; lstrcpy(cb3, L"*.js");
				WCHAR cb4[MAX_PATH]; 
				WCHAR cb5[64]; lstrcpy(cb5, L"test");

				GetCurrentDirectory( MAX_PATH,cb4 );

				ZeroMemory(&ofn, sizeof(ofn));
				
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd; 
				ofn.lpstrDefExt = cb;
				ofn.lpstrFile = cb2;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = cb3;
				ofn.nFilterIndex = 0;
				ofn.lpstrInitialDir = cb4;
				ofn.lpstrTitle = cb5;
				ofn.Flags =  OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				
				if ( ::GetOpenFileName(&ofn) )
				{
					wstring fnm = ofn.lpstrFile;
					
					BSTR src;
					DDOpenTextFile( fnm.c_str(), &src );

					DDSetText( txm, src );

					::SysFreeString(src);

				}*/

				BSTR src;
				if ( DDOpenTextFile( L"readme.txt", &src))
				{

					DDSetText(txm, src);

					::SysFreeString(src);
				}


			});
			
			
			CHAKRA2::Initialize(grun, gcxt);
			
			JSContext::Scope ct(gcxt);
			EportFunctions();
		};

		global.g_handle = DDMkWindow(hWnd, DDDControlCreate);

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_SIZE:
	{
		FSizeF sz(lParam);
		
		DDResizeWindow( global.g_handle, sz.width, sz.height );

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;

    case WM_DESTROY:
		CHAKRA2::Destroy(grun);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 繝舌・繧ｸ繝ｧ繝ｳ諠・ｱ繝懊ャ繧ｯ繧ｹ縺ｮ繝｡繝・そ繝ｼ繧ｸ 繝上Φ繝峨Λ繝ｼ縺ｧ縺吶・
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
