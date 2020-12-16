#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	struct StateSavingOrders;

	class StateSaver : private Storm::NonInstanciable
	{
	public:
		static void execute(const Storm::StateSavingOrders &savingOrder);
	};
}
