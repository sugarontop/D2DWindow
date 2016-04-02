
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D2DMainFrameWnd(HWND)
	D2DWindow(HWND)
		D2DTopControl
			以下各コントロール









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSF

class D2DTextbox
{
	TSF::CTextEditorCtrl* ctrl_;
	TSF::CTextContainer ct_;
}

class CTextContainer
{

	UINT nSelStart_, nSelEnd_;
	V4::FPointF offpt_; // Textbox内の文字の移動、Singlelineで重要

}

class CTextEditor 
{
	HWND _hwnd;
	CTextEditSink* _pTextEditSink;	

    CTextLayout _layout;

    COMPOSITIONRENDERINFO *_pCompositionRenderInfo;
    int _nCompositionRenderInfo;

    CTextStore* _pTextStore;
}

class CTextEditorCtrl : public CTextEditor
{


}

class CTextStore : public ITextStoreACP
{

}
class CTextEditSink : public ITfTextEditSink
{

}