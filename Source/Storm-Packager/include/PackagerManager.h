#pragma once

#include "Singleton.h"
#include "SingletonDefaultImplementation.h"


namespace StormPackager
{
	class PackagerManager : private Storm::Singleton<StormPackager::PackagerManager, Storm::DefineDefaultInitAndCleanupImplementation>
	{
		STORM_DECLARE_SINGLETON(PackagerManager);

	public:

	};
}
