#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IInputManager : public Storm::ISingletonHeldInterface<IInputManager>
    {
    public:
        virtual ~IInputManager() = default;
    };
}
