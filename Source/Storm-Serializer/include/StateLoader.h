#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct StateLoadingOrders;

	class StateLoader : private Storm::NonInstanciable
	{
	public:
		static void execute(Storm::StateLoadingOrders &inOutLoadingOrder);
	};
}
