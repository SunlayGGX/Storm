#pragma once

#include "SingletonHeldInterfaceBase.h"
#include "CallbackIdType.h"


namespace Storm
{
	class IEmitterManager : public Storm::ISingletonHeldInterface<IEmitterManager>
	{
	public:
		virtual ~IEmitterManager() = default;
	};
}
