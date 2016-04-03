#include "stdafx.h"
#include "D2DMatome.h"
#include "EntryD2D.h"

TSF::CTextEditorCtrl* GetTextEditorCtrl();

using namespace V4;

#define _SAMPLE2

namespace V4 
{
	Selection g_selection;
};

#ifdef _SAMPLE1

namespace V4 
{
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
			virtual void OnCreate(){}
	};
};

void EntryMain(V4::D2DWindow* parent)
{
	// craete TSF tool
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

#endif


#ifdef _SAMPLE2

#include "ChakraCore.h"
#include "CJsValueRef.h"
#include "HiggsJson.h"

CJsValueRef JsRun(LPCWSTR script);

namespace V4
{
	class Test : public D2DControl
	{
	public:
		Test() {}

		virtual LRESULT WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_PAINT:
			{
				auto& cxt = d->cxt_;
				D2DMatrix mat(cxt);
				mat.PushTransform();
				{
					mat.Offset(rc_);

					FRectF rc(0, 0, rc_.GetContentRect().Size());
					

					DrawFillRect(cxt, rc, cxt.black, cxt.ltgray, 1);
					DrawCenterText(cxt, cxt.black, rc, str_, str_.length(), 1);
				}
				mat.PopTransform();
			}
			break;
			}
			return 0;
		}
		virtual void OnCreate()
		{
			auto x = JsRun( L"test()");
			auto m = x.ToMap();						
			auto mm = m[L"message"];
			str_ = mm.ToString();

			FRectF rc(0,0,100,26);
			D2DButton* btn = new D2DButton();
			btn->CreateWindow( parent_, parent_control_, rc, VISIBLE, L"run js" );
			btn->OnClick_ = [this](D2DButton*)
			{
				auto x = JsRun(L"test()");
				auto m = x.ToMap();
				auto mm = m[L"message_jp"];
				str_ = mm.ToString();

				parent_->redraw_ = 1;

			};

		}

		FString str_;
	};
};

void EntryMain(V4::D2DWindow* parent)
{
	// craete TSF tool
	D2DTextbox::CreateInputControl(parent);

	// create top control
	D2DTopControls* cs = new D2DTopControls();
	cs->CreateWindow(parent, VISIBLE, L"first layer");

	g_selection.ctrls = cs; // do nothing


	// create sample control
	FRectF rc(50, 50, FSizeF(300, 300));
	Test* t = new Test();
	t->CreateWindow(parent, cs, rc, VISIBLE, NONAME);
}

#endif
