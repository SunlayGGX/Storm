#pragma once

#include "Singleton.h"
#include "SingletonDefaultImplementation.h"


namespace StormPackager
{
	class ConfigManager : private Storm::Singleton<StormPackager::ConfigManager, Storm::DefineDefaultCleanupImplementationOnly>
	{
		STORM_DECLARE_SINGLETON(ConfigManager);

	private:
		void initialize_Implementation(int argc, const char*const argv[]);

	public:

	};
}
