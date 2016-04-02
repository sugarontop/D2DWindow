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
			if ( message == WM_PAINT )
			{
				auto cxt = d->cxt_;
				D2DMatrix mat(cxt);
				mat.PushTransform();

				FRectFBoxModel rc( 10,10,510,310 );
				rc.Margin_.Set(1);
				rc.BoderWidth_ = 1;
				rc.Padding_.Set(0);

				mat.Offset( rc );

				// BorderRectを黒で塗りつぶす、
				FRectF rc2 = rc.GetBorderRectZero(); // <-- GetBorderRectZero()
				cxt.cxt->FillRectangle(rc2, cxt.black ); // 枠になる
				

				// 下位レベルで描画領域を制限
				FRectF rcContent = rc.GetContentRectZero(); // <-- GetContentRectZero
				D2DRectFilter fil(cxt, rcContent);
				{					
					// 全面を白、しかし領域内のみ描画されない
					cxt.cxt->FillRectangle( FRectF(0,0,600,600), cxt.white);

				}


				mat.PopTransform();
				
			}


			return 0;
		}


};


void EntryMain(V4::D2DWindow* parent)
{
	D2DTextbox::CreateInputControl(parent);	
	D2DTopControls* cs = new D2DTopControls();

	g_selection.ctrls = cs;

	
	//cs->CreateWindow(parent, VISIBLE, L"1 layer");


	//TRopeMainFramePanelA* t = new TRopeMainFramePanelA();
	//t->CreateWindow(parent, cs, FRectF(100,100,300,500),VISIBLE,NONAME);
	
}