#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IAssetLoaderManager : public Storm::ISingletonHeldInterface<IAssetLoaderManager>
	{
	public:
		virtual ~IAssetLoaderManager() = default;
	};
}
