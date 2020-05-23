#include "Application.h"

#include "SingletonAllocator.h"
#include "SingletonHolder.h"
#include "LoggerManager.h"
#include "ConfigManager.h"
#include "WindowsManager.h"
#include "InputManager.h"
#include "GraphicManager.h"
#include "OSManager.h"

namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		Storm::SingletonHolder,
		Storm::ConfigManager,
		Storm::LoggerManager,
		Storm::OSManager,
		Storm::InputManager,
		Storm::WindowsManager,
		Storm::GraphicManager
	>;

	std::unique_ptr<SingletonAllocatorAlias> g_singletonMaker;
}

Storm::Application::Application(int argc, const char* argv[])
{
	g_singletonMaker = std::make_unique<SingletonAllocatorAlias>();

	LOG_COMMENT << "Application Creation started";

	Storm::ConfigManager::instance().initialize(argc, argv);

	Storm::OSManager::instance().initialize();

	Storm::LoggerManager::instance().initialize();

	Storm::InputManager::instance().initialize();

	Storm::GraphicManager::instance().initialize();
	Storm::WindowsManager::instance().initialize();

	LOG_COMMENT << "Application Creation finished";
}

Storm::Application::~Application()
{
	LOG_COMMENT << "Application Cleanup";

	Storm::InputManager::instance().cleanUp();
	Storm::GraphicManager::instance().cleanUp();
	Storm::WindowsManager::instance().cleanUp();
	Storm::ConfigManager::instance().cleanUp();
	Storm::OSManager::instance().cleanUp();
	Storm::LoggerManager::instance().cleanUp();

	g_singletonMaker.reset();
}

Storm::ExitCode Storm::Application::run()
{
	if (!Storm::ConfigManager::instance().shouldDisplayHelp())
	{
		LOG_COMMENT << "Creating Application Windows";
		Storm::WindowsManager::instance().initialize();

		LOG_COMMENT << "Start Application Run";
		// TODO
	}

	return ExitCode::k_success;
}
