#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class GraphicHelpers : private Storm::NonInstanciable
	{
	public:
		static void removeUselessZeros(std::wstring &inOutValue);
	};
}
