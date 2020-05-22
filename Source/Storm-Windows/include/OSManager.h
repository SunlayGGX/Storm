#pragma once

#include "Singleton.h"
#include "IOSManager.h"


namespace Storm
{
    class OSManager :
        private Storm::Singleton<OSManager>,
        public Storm::IOSManager
    {
        STORM_DECLARE_SINGLETON(OSManager);
    };
}
