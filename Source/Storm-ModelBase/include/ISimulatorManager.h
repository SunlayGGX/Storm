#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	using SimulationCallback = std::function<void()>;

	class ISimulatorManager : public Storm::ISingletonHeldInterface<ISimulatorManager>
	{
	public:
		virtual ~ISimulatorManager() = default;
	};
}
