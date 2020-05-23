#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class ISimulatorManager : public Storm::ISingletonHeldInterface<ISimulatorManager>
	{
	public:
		virtual ~ISimulatorManager() = default;
	};
}
