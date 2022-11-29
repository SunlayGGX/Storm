#include "Application.h"

#include "SerializerManager.h"
#include "LoggerManager.h"
#include "SingletonAllocator.h"

#include "ExitCode.h"
#include "RAII.h"


namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		StormTool::LoggerManager,
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
		Storm::SerializerManager::instance().initialize();
	}

	void shutdown() noexcept
	{
		if (g_alloc)
		{
			Storm::SerializerManager::instance().cleanUp();
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
	if (argc < 3)
	{
		Storm::throwException<Storm::Exception>("We must have at least the mode and record file to export as arguments!");
	}

	initialize(argc, argv);
}

StormExporter::Application::~Application()
{
	g_shutdowner.reset();
}

Storm::ExitCode StormExporter::Application::run()
{
	return Storm::ExitCode::k_success;
}

void StormExporter::Application::staticForceShutdown()
{
	g_shutdowner.reset();
}
