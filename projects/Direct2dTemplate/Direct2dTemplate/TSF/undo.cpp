#include "stdafx.h"
#include "undo.h"

// 2016/4/2, not implement

CUndoInfoStackEx& CUndoInfoStackEx::FactorySingleton()
{
	static CUndoInfoStackEx undo(50);
	
	return undo;
}