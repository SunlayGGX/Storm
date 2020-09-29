#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class ISerializerManager : public Storm::ISingletonHeldInterface<Storm::ISerializerManager>
	{
	public:
		virtual ~ISerializerManager() = default;
	};
}
