#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IThreadManager : public Storm::ISingletonHeldInterface<IThreadManager>
	{
	public:
		virtual ~IThreadManager() = default;
	};
}
