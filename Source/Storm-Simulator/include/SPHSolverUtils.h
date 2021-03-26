#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class SPHSolverUtils : private Storm::NonInstanciable
	{
	public:
		template<class DataContainerType>
		static void removeRawEndData(const unsigned int pSystemId, std::size_t toRemoveCount, DataContainerType &data)
		{
			if (auto found = data.find(pSystemId); found != std::end(data))
			{
				auto &data = found->second;
				while (toRemoveCount != 0)
				{
					data.pop_back();
					--toRemoveCount;
				}
			}
			else
			{
				Storm::throwException<Storm::Exception>("Cannot find particle system data bound to particle system " + std::to_string(pSystemId));
			}
		}

	};
}
