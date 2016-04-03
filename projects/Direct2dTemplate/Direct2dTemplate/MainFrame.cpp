// Direct2dTemplate.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Direct2dTemplate.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "d2dcontextnew.h"
#include "tsf\TextEditor.h"

#include "EntryD2D.h"

#define WINDOW_TITLE L"D2DMainFrame H version"

using namespace GDI32;

#define MAX_LOADSTRING 100
#define st __s_d2dmainframe



static HWND ghMainFrameWnd;

// グローバル変数:
static HINSTANCE __hInst;						// 現在のインターフェイス
TCHAR __szTitle[MAX_LOADSTRING];				// タイトル バーのテキスト
TCHAR __szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

#define IDLE_TIME (5*1000)
#define IDLE_TIMER_ID 99

// javascript 
bool JsAppInit(HWND hMainWnd);
void JsAppClose();



V4::D2DWindow window; 

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
void				ExitInstance();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	CoInitialize(0);
	_tsetlocale ( 0, _T("japanese") ); 	 
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, __szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DIRECT2DTEMPLATE, __szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2DTEMPLATE));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ExitInstance();
	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2DTEMPLATE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; 
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= __szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}



HWND AfxMainFrameWindow()
{
	return ghMainFrameWnd;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	D2DTextbox::AppTSFInit();

   __hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   HWND hMainFrameWnd = CreateWindowW(__szWindowClass, __szTitle, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, 1.5*900, 800, NULL, NULL, hInstance, NULL);

   if (!hMainFrameWnd )
   {
      return FALSE;
   }

   ShowWindow(hMainFrameWnd, nCmdShow);
   UpdateWindow(hMainFrameWnd);

   ghMainFrameWnd = hMainFrameWnd;
   return TRUE;

}
void ExitInstance()
{
	D2DTextbox::AppTSFExit();

	CoUninitialize();

}


// バージョン情報ボックスのメッセージ ハンドラーです。
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


#ifndef LIGHTWINDOW
#define CLIENT_SPCCX 6
#define CLIENT_SPCCY 6
#define CAPTION_HEIGHT 30

struct D2DMainFrame
{
	CComPtr<IDWriteFactory> wrfactory;
	CComPtr<ID2D1Factory>  factory;
	CComPtr<IDWriteTextFormat> textformat;
	CComPtr<ID2D1HwndRenderTarget> cxt;

	CComPtr<ID2D1SolidColorBrush> br[4];
	CComPtr<ID2D1SolidColorBrush> black, white;

	FRectF btn[3];
	UINT btnStat;
	LPCWSTR title;
	
	enum COLORS{ MOUSE_FLOAT,CLOSEBTN,ACTIVECAPTION,CLOSE_MOUSE_FLOAT };
};

static D2DMainFrame __s_d2dmainframe;
void D2DInitial(HWND hWnd1)
{
	HRESULT hr;
	D2D1_FACTORY_OPTIONS options;	
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),&options,(void**)&st.factory );
	_ASSERT(hr==S_OK);

	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&st.wrfactory));
	_ASSERT(hr==S_OK);

	hr = st.wrfactory->CreateTextFormat( L"Arial",0,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,14,L"",&st.textformat);
	_ASSERT(hr==S_OK);

	hr = st.factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd1, D2D1::SizeU(1,1), D2D1_PRESENT_OPTIONS_NONE), &st.cxt);
	_ASSERT(hr==S_OK);

	st.cxt->CreateSolidColorBrush( ColorF(ColorF::Black), &st.black );
	st.cxt->CreateSolidColorBrush( ColorF(ColorF::White), &st.white );
	st.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,255), &st.br[D2DMainFrame::MOUSE_FLOAT] );
	st.cxt->CreateSolidColorBrush( D2RGBA(199,80,80,255), &st.br[D2DMainFrame::CLOSEBTN] );
	st.cxt->CreateSolidColorBrush( D2RGBA(144,169,184,255), &st.br[D2DMainFrame::ACTIVECAPTION] );
	st.cxt->CreateSolidColorBrush( D2RGBA(224,67,67,255), &st.br[D2DMainFrame::CLOSE_MOUSE_FLOAT] );

	st.btn[0] = FRectF(0,0,26,20); // MINI BUTTON
	st.btn[1] = FRectF(0,0,27,20); // MAX BUTTON
	st.btn[2] = FRectF(0,0,45,20); // CLOSE BUTTON

	st.btnStat = 0;

	st.title = WINDOW_TITLE;
	
	st.factory.Release();
	st.wrfactory.Release();

}
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
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(__hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
	{
		

		D2DInitial(hWnd);

		JsAppInit(hWnd);

		
		window.OnCreate = EntryMain;

		RECT rcc;
		::GetClientRect(hWnd,&rcc);

		UINT id[] = { IDB_PNG1 };
		HWND hwnd = window.CreateD2DWindow( 0, hWnd, WS_CHILD|WS_VISIBLE, rcc, id, _countof(id) );

		
		auto IdleMessage = [](HWND hwnd, UINT msg, UINT_PTR id, DWORD time )
		{
			if ( IDLE_TIMER_ID == id )
				SendMessage(hwnd, WM_D2D_IDLE, 0, 0);
		};


		::SetTimer( hWnd, IDLE_TIMER_ID, IDLE_TIME, IdleMessage );
	}
	break;
	case WM_D2D_JS_ERROR :
	{
		window.WndProc(WM_D2D_JS_ERROR,wParam,lParam);


	}
	break;
	case WM_SIZE:
	{
		UINT cx =  LOWORD(lParam);
		UINT cy =  HIWORD(lParam);	
		st.cxt->Resize( D2D1::SizeU(cx,cy));

		

		FSize sz(lParam);
		::MoveWindow( window.hWnd_, CLIENT_SPCCX,CAPTION_HEIGHT, sz.cx-CLIENT_SPCCX*2, sz.cy-(CAPTION_HEIGHT+CLIENT_SPCCY), TRUE );
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		st.cxt->BeginDraw();
		D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();		
		st.cxt->SetTransform(mat);
		st.cxt->Clear(D2RGBA(110,129,141,255));// 枠線

		RECT rc;
		GetClientRect(hWnd,&rc);

		WINDOWPLACEMENT xx;
		xx.length = sizeof(xx);
		GetWindowPlacement(hWnd,&xx);

		if ( xx.showCmd == SW_MAXIMIZE )
		{
			// まだまだ、検討余地あり
			// rc.right : 1936
			// SM_CXFULLSCREEN : 1920
			// SM_CXMAXIMIZED 1936
			// SM_CXMAXTRACK 3860
			WINDOWINFO ri;
			ri.cbSize = sizeof(ri);
			GetWindowInfo(hWnd,&ri);

			rc.bottom = ri.rcClient.bottom-ri.cyWindowBorders;
			rc.right = ri.rcClient.right-ri.cxWindowBorders;

			mat._31 = ri.cxWindowBorders;
			mat._32 = ri.cyWindowBorders;

			st.cxt->SetTransform(mat);
		}

		const RECT rcClient = rc;

		FRectF rcf(rc);
		FRectF rcCaption(rcf);

		
		FRectF rcwaku(rcf);
		rcwaku.left++; rcwaku.right--;
		rcwaku.top++; rcwaku.bottom--;
				
		st.cxt->FillRectangle( &rcwaku, st.br[D2DMainFrame::ACTIVECAPTION] );


		rcCaption = rcwaku;

		rcCaption.bottom=rcCaption.top+CAPTION_HEIGHT;		
		st.cxt->FillRectangle( rcCaption, st.br[D2DMainFrame::ACTIVECAPTION] );

		FRectF rctext(1,1,rcwaku.right,CAPTION_HEIGHT); 
				
		st.cxt->DrawText( st.title, lstrlen(st.title), st.textformat, &rctext, st.black );

		mat._31 += (float)( rcClient.right - (26+27+45+CLIENT_SPCCX));	// X ZeroPoint
		mat._32 += 1;													// Y ZeroPoint
				

		for( int i = 0; i < 3; i++ )
		{
			FRectF rc;
			st.cxt->SetTransform(mat);
			
			if ( i == 0 )
			{
				// Draw Minimize button.
				rc.SetRect(10,12,17,14);
				auto br = ( st.btnStat == 0 ? st.black : st.white );				
				if ( st.btnStat == 1 )
				{
					st.cxt->FillRectangle( st.btn[i], st.br[D2DMainFrame::MOUSE_FLOAT] );
					st.cxt->FillRectangle( rc, st.white );
				}
				else
					st.cxt->FillRectangle( rc, st.black );
			}
			else if ( i == 1 )
			{
				// Draw Maxmize button.
				// when floatting
				if ( st.btnStat == 2 )
				{					
					FRectF rck = st.btn[i];
					rck.left++;rck.right--;
					st.cxt->FillRectangle( rck, st.br[D2DMainFrame::MOUSE_FLOAT] );				
				}				
				auto br = ( st.btnStat == 2 ? st.white :  st.black );
				rc.SetRect(9,6,10,14); st.cxt->FillRectangle( rc, br );
				rc.SetRect(9,13,19,14); st.cxt->FillRectangle( rc, br );
				rc.SetRect(18,6,19,14); st.cxt->FillRectangle( rc, br );
				rc.SetRect(9,6,19,8); st.cxt->FillRectangle( rc, br );
			}
			else if ( i == 2 )
			{
				// Draw close button.				

				auto br = ( st.btnStat == 3 ? st.br[D2DMainFrame::CLOSE_MOUSE_FLOAT] : st.br[D2DMainFrame::CLOSEBTN]);
				st.cxt->FillRectangle( st.btn[i], br);
				FRectF rc1,rc2;
				rc1.SetRect(19,6,21,7); 				
				rc2.SetRect(25,6,27,7); 
				for( int ii = 0; ii < 7; ii++ )
				{					
					st.cxt->FillRectangle( rc1, st.white );
					rc1.Offset(1,1);
					st.cxt->FillRectangle( rc2, st.white );
					rc2.Offset(-1,1);
				}
			}
		
			mat._31 += st.btn[i].right;
		}
		st.cxt->EndDraw();
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		JsAppClose();
		PostQuitMessage(0);
	break;
	case WM_NCCALCSIZE:
		return 0;
	break;
	case WM_NCHITTEST:
	{
		RECT rc;
		GetClientRect(hWnd,&rc);

		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		ScreenToClient( hWnd, &pt );
		
		POINT ptbtn = pt;
		
		ptbtn.x -= (rc.right - (26+27+45+CLIENT_SPCCX));
		ptbtn.y -= 1;
		FRectF rcbtn(0,0,(26+27+45),20);
		UINT prv = st.btnStat;
		
		UINT nstat = 0;
		if ( rcbtn.PtInRect( ptbtn ) )
		{
			if ( ptbtn.x < 26 )
				nstat =1;
			else if ( ptbtn.x < (26+27) )
				nstat =2;
			else
				nstat =3;	
		}
		else if ( pt.x < 30 && pt.y < CAPTION_HEIGHT && 0 < pt.y && 0 < pt.x )
		{
			// 左上タイトルで終了
			nstat =3;	
			st.btnStat = nstat;	
			return HTCLIENT;
		}


		if ( nstat != prv )
		{
			st.btnStat = nstat;			
			InvalidateRect(hWnd,NULL,FALSE); // redraw
		}

		int w = 5;
		int width = rc.right;
		int height = rc.bottom;
		if ( !rcbtn.PtInRect( ptbtn )  )
		{
			if ( pt.y < w )
			{
				if ( pt.x < w )
					return HTTOPLEFT;
				else if ( width - pt.x < w)
					return HTTOPRIGHT;
				else
					return HTTOP;
			}
			else if ( height-pt.y < w )
			{
				if ( pt.x < w )
					return HTBOTTOMLEFT;
				else if ( width - pt.x < w)
					return HTBOTTOMRIGHT;
				else
					return HTBOTTOM;
			}					
			else if ( pt.y < CAPTION_HEIGHT )
				return HTCAPTION;
			else if ( pt.x < w )
				return HTLEFT;
			else if ( width - pt.x < w )
				return HTRIGHT;
		}
		return HTCLIENT;
	}
	break;
	case WM_LBUTTONUP:
	{
		// OnCLicked 

		if ( st.btnStat == 1 )
			ShowWindow( hWnd, SW_MINIMIZE );
		else if ( st.btnStat == 2 )
			ShowWindow( hWnd, SW_MAXIMIZE );			
		else if ( st.btnStat == 3 )
			DestroyWindow( hWnd );
	}
	break;
	case WM_ERASEBKGND :
	{			
		InvalidateRect(window.hWnd_,NULL,FALSE); 
		return 1;
	}
	break;
	
	default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
#endif