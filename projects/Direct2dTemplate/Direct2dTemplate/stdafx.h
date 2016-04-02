// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"


#define USE_ID2D1DEVICECONTEXT

#include "common_stdafx.h"

#define JSRUN_CLIENT
#define JSCRIPT_ENGINE_VERSION 9


#import <msxml6.dll>	// make msxml6.tlh, msxml6.tli 


#define BS(a)	_bstr_t(L##a)
