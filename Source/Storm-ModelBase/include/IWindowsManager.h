#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IWindowsManager : public Storm::ISingletonHeldInterface<IWindowsManager>
    {
    public:
        virtual ~IWindowsManager() = default;
    };
}
