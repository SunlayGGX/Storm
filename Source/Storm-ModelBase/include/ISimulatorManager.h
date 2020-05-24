#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	using SimulationCallback = std::function<void()>;

	class ISimulatorManager : public Storm::ISingletonHeldInterface<ISimulatorManager>
	{
	public:
		virtual ~ISimulatorManager() = default;

	public:
		// Add a method to be executed at the end of the current simulation loop iteration.
		virtual void executeOnSimulationLoop(Storm::SimulationCallback func) = 0;
	};
}
