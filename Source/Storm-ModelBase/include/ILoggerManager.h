#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class ILoggerManager : public Storm::ISingletonHeldInterface<ILoggerManager>
    {
    public:
        virtual ~ILoggerManager() = default;
    };
}
