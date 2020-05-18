#include "Application.h"

#include "SingletonAllocator.h"
#include "SingletonHolder.h"
#include "LoggerManager.h"
#include "ConfigManager.h"

namespace
{
    using SingletonAllocatorAlias = Storm::SingletonAllocator<
        Storm::SingletonHolder,
        Storm::ConfigManager,
        Storm::LoggerManager
    >;

    std::unique_ptr<SingletonAllocatorAlias> g_singletonMaker;
}

Storm::Application::Application(int argc, const char* argv[])
{
    g_singletonMaker = std::make_unique<SingletonAllocatorAlias>();

    LOG_COMMENT << "Application Creation started";

    Storm::ConfigManager::instance().initialize(argc, argv);
    Storm::LoggerManager::instance().initialize();

    LOG_COMMENT << "Application Creation finished";
}

Storm::Application::~Application()
{
    LOG_COMMENT << "Application Cleanup";

    Storm::ConfigManager::instance().cleanUp();
    Storm::LoggerManager::instance().cleanUp();

    g_singletonMaker.reset();
}

Storm::ExitCode Storm::Application::run()
{
    LOG_COMMENT << "Application Run started";

    return ExitCode::k_success;
}
