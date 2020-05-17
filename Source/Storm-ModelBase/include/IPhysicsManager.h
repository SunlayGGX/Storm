#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IPhysicsManager : public Storm::ISingletonHeldInterface<IPhysicsManager>
    {
    public:
        virtual ~IPhysicsManager() = default;
    };
}
