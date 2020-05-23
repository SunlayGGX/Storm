#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IRandomManager : public Storm::ISingletonHeldInterface<IRandomManager>
	{

	};
}
