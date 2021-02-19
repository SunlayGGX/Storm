#include "GraphicHelpers.h"


void Storm::GraphicHelpers::removeUselessZeros(std::wstring &inOutValue)
{
	while (inOutValue.back() == L'0')
	{
		inOutValue.pop_back();
	}
}
