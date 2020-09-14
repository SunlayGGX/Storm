#include "Application.h"

#include "SingletonAllocator.h"
#include "PackagerManager.h"
#include "ConfigManager.h"

#include "ExitCode.h"

namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		StormPackager::ConfigManager,
		StormPackager::PackagerManager
	>;

	std::unique_ptr<SingletonAllocatorAlias> g_singletonMaker;

	void initializeStormPackagerApplication(int argc, const char*const argv[])
	{
		g_singletonMaker = std::make_unique<SingletonAllocatorAlias>();

		StormPackager::ConfigManager::instance().initialize(argc, argv);
		StormPackager::PackagerManager::instance().initialize();
	}

	void cleanUpStormPackagerApplication()
	{
		if (g_singletonMaker != nullptr)
		{
			StormPackager::PackagerManager::instance().cleanUp();
			StormPackager::ConfigManager::instance().cleanUp();

			g_singletonMaker.reset();
		}
	}
}

StormPackager::Application::Application(int argc, const char*const argv[])
{
	initializeStormPackagerApplication(argc, argv);
}

StormPackager::Application::~Application()
{
	cleanUpStormPackagerApplication();
}

Storm::ExitCode StormPackager::Application::run()
{
	bool success;

	const StormPackager::ConfigManager &configMgr = StormPackager::ConfigManager::instance();
	if (configMgr.helpRequested())
	{
		configMgr.printHelp();
		success = true;
	}
	else
	{
		success = PackagerManager::instance().run();
	}

	return success ? Storm::ExitCode::k_success : Storm::ExitCode::k_failure;
}

