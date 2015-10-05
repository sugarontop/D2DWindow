#include "stdafx.h"
#include "undo.h"


CUndoInfoStackEx& CUndoInfoStackEx::FactorySingleton()
{
	static CUndoInfoStackEx undo(50);
	
	return undo;
}