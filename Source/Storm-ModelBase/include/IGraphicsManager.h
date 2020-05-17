#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IGraphicsManager : public Storm::ISingletonHeldInterface<IGraphicsManager>
    {
    public:
        virtual ~IGraphicsManager() = default;
    };
}
