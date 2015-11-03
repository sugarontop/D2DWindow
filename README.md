D2DWindow
=========

Direct2D Windows Framework.(VS2015,C++11, no ATL, no MFC, no BOOST)

<pre>
[source]
 projects
    /D2DTest1
        /D2DTestDataGrid <-- main
          /Debug
                D2DTestDataGrid.exe <-- sample exe
                D2DCommon.dll
                D2DWin.dll
    /D2DWin
        コントロールクラス、C++クラスをC関数へ変換
    /D2DCommon
        簡単なAPI群
    /include
        共通のヘッダファイル


[outline]
WinMain
     MainFrameWindow(HWND)
          TopChildWindow(HWND) --D2DWindow
               TopControls --D2DTopControls
                    Controls
	                    Control

</pre>

履歴

- 2015/08     start
- 2015/11/03  0.2.7
