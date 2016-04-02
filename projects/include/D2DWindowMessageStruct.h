#pragma once

#include "D2DMisc.h"


namespace V4 
{


struct D2D_OBJECT_UPDATE
{
	enum TYP{ NONE,NEWPARENT,TRYNEWPARENT_FAIL,MOVE,SUCCESS,TRY_NEWPARENT };
	
	TYP stat;
	FPointF globalpt;
	void* object;
};


// WM_D2D_DRAGDROP		
struct D2D_DROPOBJECT
{
	FString filename;
	FPointF pt;

};







};





