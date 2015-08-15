#pragma once

struct IBridgeTSFInterface
{
	IBridgeTSFInterface(){}
	virtual V4::FRectF GetClientRect() = 0;
	virtual IDWriteTextFormat* GetFormat() = 0;
	virtual V4::FRectFBoxModel GetClientRectEx() = 0;
};

