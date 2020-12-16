#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct StateSavingOrders;

	class StateWriter : private Storm::NonInstanciable
	{
	public:
		static void execute(Storm::StateSavingOrders &savingOrder);
	};
}
