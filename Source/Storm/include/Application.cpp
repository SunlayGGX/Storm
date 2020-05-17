#include "Application.h"

#include "SingletonAllocator.h"
#include "SingletonHolder.h"

namespace
{
    using SingletonAllocatorAlias = Storm::SingletonAllocator<
        Storm::SingletonHolder
    >;

    std::unique_ptr<SingletonAllocatorAlias> g_singletonMaker;
}

Storm::Application::Application(int argc, const char* argv[])
{
    g_singletonMaker = std::make_unique<SingletonAllocatorAlias>();
}

Storm::Application::~Application()
{
    g_singletonMaker.reset();
}

Storm::ExitCode Storm::Application::run()
{
    return ExitCode::k_success;
}
