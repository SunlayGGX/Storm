#pragma once

#include "Singleton.h"
#include "IOSManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
    class OSManager :
        private Storm::Singleton<OSManager, Storm::DefineDefaultInitAndCleanupImplementation>,
        public Storm::IOSManager
    {
        STORM_DECLARE_SINGLETON(OSManager);
    };
}
