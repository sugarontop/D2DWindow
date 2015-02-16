* VisualStudio2013 C++
* WIN7 SP1以降


* Path route
	.\; ->
	.\v4; ->
	..\..\library\v4;

* SourceFile

	stdafx.cpp
		link library.

	*** /D2DWindow
		Window libraray.

	*** /TSF
		TextServicesFramework, IME. no implement.

* note
	ボタンを押すとWindowControlが作成表示されます。赤いキャプションで移動できます。

* ID2D1HwndRenderTarget->ID2D1DeviceContext
	RenderTargetの種類を変更しました。