#include "stdafx.h"
#include "D2DMatome.h"
#include "EntryD2D.h"

using namespace V4;

TSF::CTextEditorCtrl* GetTextEditorCtrl();

namespace V4 {
	Selection g_selection;
};


class Test : public D2DControl
{
	public :
		Test(){}

		virtual LRESULT WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch(message )
			{
				case WM_PAINT:
				{
					auto& cxt = d->cxt_;
					D2DMatrix mat(cxt);
					mat.PushTransform();
					{
						mat.Offset( rc_ );


						FRectF rc(0,0,rc_.GetContentRect().Size());
						FString str = L"Hello World";
						
						DrawFillRect( cxt, rc, cxt.black, cxt.ltgray, 1 );
						DrawCenterText( cxt, cxt.black, rc, str, str.length(), 1 );




					}
					mat.PopTransform();
				
				}
				break;
			}

			return 0;
		}


};


void EntryMain(V4::D2DWindow* parent)
{
	// craete tsf tool
	D2DTextbox::CreateInputControl(parent);	

	// create top control
	D2DTopControls* cs = new D2DTopControls();
	cs->CreateWindow(parent, VISIBLE, L"first layer");

	g_selection.ctrls = cs; // do nothing



	// create sample control
	FRectF rc( 50,50,FSizeF(300,300));
	Test* t = new Test();
	t->CreateWindow( parent, cs, rc, VISIBLE, NONAME );
	
}