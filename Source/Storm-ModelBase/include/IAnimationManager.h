#pragma once


#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IAnimationManager : public Storm::ISingletonHeldInterface<Storm::IAnimationManager>
	{
	public:
		virtual ~IAnimationManager() = default;
	};
}
