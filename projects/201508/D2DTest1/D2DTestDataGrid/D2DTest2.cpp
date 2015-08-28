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


・ｿ// D2DTest2.cpp : 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ繧ｨ繝ｳ繝医Μ 繝昴う繝ｳ繝医ｒ螳夂ｾｩ縺励∪縺吶・

#include "stdafx.h"
#include "D2DTest2.h"
#include "D2DWin.h"

#define MAX_LOADSTRING 100

// 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚:
HINSTANCE hInst;								// 迴ｾ蝨ｨ縺ｮ繧､繝ｳ繧ｿ繝ｼ繝輔ぉ繧､繧ｹ
TCHAR szTitle[MAX_LOADSTRING];					// 繧ｿ繧､繝医Ν 繝舌・縺ｮ繝・く繧ｹ繝・
TCHAR szWindowClass[MAX_LOADSTRING];			// 繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え 繧ｯ繝ｩ繧ｹ蜷・

// 縺薙・繧ｳ繝ｼ繝・繝｢繧ｸ繝･繝ｼ繝ｫ縺ｫ蜷ｫ縺ｾ繧後ｋ髢｢謨ｰ縺ｮ螳｣險繧定ｻ｢騾√＠縺ｾ縺・
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	CoInitialize(0);	
	_tsetlocale ( 0, _T("japanese") ); 	 
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	
	MSG msg;
	HACCEL hAccelTable;

	// 繧ｰ繝ｭ繝ｼ繝舌Ν譁・ｭ怜・繧貞・譛溷喧縺励※縺・∪縺吶・
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D2DTEST2, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ蛻晄悄蛹悶ｒ螳溯｡後＠縺ｾ縺・
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D2DTEST2));

	// 繝｡繧､繝ｳ 繝｡繝・そ繝ｼ繧ｸ 繝ｫ繝ｼ繝・
	while (GetMessage(&msg, NULL, 0, 0))
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
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D2DTEST2));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D2DTEST2);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚縺ｫ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ蜃ｦ逅・ｒ譬ｼ邏阪＠縺ｾ縺吶・

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  髢｢謨ｰ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  逶ｮ逧・    繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え縺ｮ繝｡繝・そ繝ｼ繧ｸ繧貞・逅・＠縺ｾ縺吶・
//
//  WM_COMMAND	- 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ 繝｡繝九Η繝ｼ縺ｮ蜃ｦ逅・
//  WM_PAINT	- 繝｡繧､繝ｳ 繧ｦ繧｣繝ｳ繝峨え縺ｮ謠冗判
//  WM_DESTROY	- 荳ｭ豁｢繝｡繝・そ繝ｼ繧ｸ繧定｡ｨ遉ｺ縺励※謌ｻ繧・
//
//

struct Global
{
	~Global()
	{
		DDDestroyWindow(g_handle);
	}
	D2DWin g_handle;
};

Global global;
void CreatePage0(D2Ctrls t1);


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
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
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 謠冗判繧ｳ繝ｼ繝峨ｒ縺薙％縺ｫ霑ｽ蜉縺励※縺上□縺輔＞...
		EndPaint(hWnd, &ps);
		break;
	case WM_CREATE:
	{
		auto D2DControlCreate = [](D2DWin w)
		{

			auto t = DDMkTopControls( w, V4::FRectF(0,0,0,0),NONAME);
			
			CreatePage0(t);
			
		};
		
		global.g_handle = DDMkWindow( hWnd, D2DControlCreate );
		
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_SIZE:
	{
		DDResizeWindow( global.g_handle, LOWORD(lParam), HIWORD(lParam));


		return DefWindowProc(hWnd, message, wParam, lParam);		
	};
	break;
	case WM_DESTROY:
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
