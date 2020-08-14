#pragma once

#include "SingletonHeldInterfaceBase.h"

namespace Storm
{
	class ISpaceManager : public Storm::ISingletonHeldInterface<Storm::ISpaceManager>
	{
	public:
		virtual ~ISpaceManager() = default;

	public:

	};
}
