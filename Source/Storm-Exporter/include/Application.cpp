#include "Application.h"

#include "SerializerManager.h"
#include "LoggerManager.h"
#include "ExporterConfigManager.h"
#include "SingletonAllocator.h"
#include "SingletonHolder.h"

#include "ExitCode.h"
#include "RAII.h"


namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		Storm::SingletonHolder,
		StormTool::LoggerManager,
		StormExporter::ExporterConfigManager,
		Storm::SerializerManager
	>;

	std::unique_ptr<SingletonAllocatorAlias> g_alloc;

	void initialize(int argc, const char*const argv[])
	{
		if (!g_alloc)
		{
			g_alloc = std::make_unique<SingletonAllocatorAlias>();
		}

		StormTool::LoggerManager::instance().initialize();
		StormExporter::ExporterConfigManager::instance().initialize(argc, argv);
		Storm::SerializerManager::instance().initialize(Storm::SerializerManager::ExporterToolTag{});
	}

	void shutdown() noexcept
	{
		if (g_alloc)
		{
			Storm::SerializerManager::instance().cleanUp(Storm::SerializerManager::ExporterToolTag{});
			StormExporter::ExporterConfigManager::instance().cleanUp();
			StormTool::LoggerManager::instance().cleanUp();

			g_alloc.reset();
		}
	}

	auto g_shutdowner = Storm::makeLazyRAIIObject([]()
	{
		shutdown();
	});
}


StormExporter::Application::Application(int argc, const char*const argv[])
{
	initialize(argc, argv);
}

StormExporter::Application::~Application()
{
	g_shutdowner.reset();
}

Storm::ExitCode StormExporter::Application::run()
{
	if (StormExporter::ExporterConfigManager::instance().printHelpAndShouldExit())
	{
		return Storm::ExitCode::k_success;
	}

	return Storm::ExitCode::k_success;
}

void StormExporter::Application::staticForceShutdown()
{
	g_shutdowner.reset();
}
