// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

void AppInit();
void AppExit();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		break;
		case DLL_THREAD_ATTACH:		
		break;
		case DLL_THREAD_DETACH:		
		break;
		case DLL_PROCESS_DETACH:
			AppExit();
		break;
		}
	return TRUE;
}

