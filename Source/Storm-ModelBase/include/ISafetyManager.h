#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class ISafetyManager : public Storm::ISingletonHeldInterface<Storm::ISafetyManager>
	{
	public:
		virtual ~ISafetyManager() = default;

	public:

	};
}
