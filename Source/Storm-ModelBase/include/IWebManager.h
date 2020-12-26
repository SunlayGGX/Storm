#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IWebManager : public Storm::ISingletonHeldInterface<IWebManager>
	{
	public:
		virtual ~IWebManager() = default;

	public:
	};
}
