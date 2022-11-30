#include "Application.h"

#include "IExporterManager.h"
#include "SerializerManager.h"
#include "LoggerManager.h"
#include "ExporterConfigManager.h"
#include "PartioExporterManager.h"

#include "SingletonHolder.h"
#include "SingletonAllocator.h"

#include "ExportType.h"

#include "ExitCode.h"
#include "RAII.h"


namespace
{
	std::unique_ptr<Storm::SingletonAllocator<
		Storm::SingletonHolder,
		StormTool::LoggerManager,
		StormExporter::ExporterConfigManager,
		Storm::SerializerManager
	>> g_baseAlloc;

	std::unique_ptr<Storm::SingletonAllocator<
		StormExporter::PartioExporterManager
	>> g_partioAlloc;

	template<class AllocPtr>
	void allocate(AllocPtr &ptr)
	{
		if (!ptr)
		{
			ptr = std::make_unique<std::remove_cvref_t<decltype(*ptr)>>();
		}
	}

	void initialize(int argc, const char*const argv[])
	{
		allocate(g_baseAlloc);

		StormTool::LoggerManager::instance().initialize();
		StormExporter::ExporterConfigManager::instance().initialize(argc, argv);
		Storm::SerializerManager::instance().initialize(Storm::SerializerManager::ExporterToolTag{});

		const auto exportMode = StormExporter::ExporterConfigManager::instance().getExportType();
		switch (exportMode)
		{
		case StormExporter::ExportType::Partio:
		{
			allocate(g_partioAlloc);
			break;
		}
		default:
			assert(false && "Unhandled Export Type!");
			__assume(false);
		}

		Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterManager>().doInitialize();
	}

	void shutdown() noexcept
	{
		if (auto*const optionalExporterMgr = Storm::SingletonHolder::instance().getFacet<StormExporter::IExporterManager>(); 
			optionalExporterMgr)
		{
			optionalExporterMgr->doCleanUp();

			g_partioAlloc.reset();
		}

		if (g_baseAlloc)
		{
			Storm::SerializerManager::instance().cleanUp(Storm::SerializerManager::ExporterToolTag{});
			StormExporter::ExporterConfigManager::instance().cleanUp();
			StormTool::LoggerManager::instance().cleanUp();

			g_baseAlloc.reset();
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

	return Storm::SingletonHolder::instance().getSingleton<StormExporter::IExporterManager>().run();
}

void StormExporter::Application::staticForceShutdown()
{
	g_shutdowner.reset();
}
