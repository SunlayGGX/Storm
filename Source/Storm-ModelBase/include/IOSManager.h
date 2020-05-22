#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IOSManager : public Storm::ISingletonHeldInterface<IOSManager>
    {
    public:
        virtual ~IOSManager() = default;

    public:

    };
}
