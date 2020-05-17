#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IConfigManager : public Storm::ISingletonHeldInterface<IConfigManager>
    {
    public:
        virtual ~IConfigManager() = default;
    };
}
