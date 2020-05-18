#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
    class IWindowsManager : public Storm::ISingletonHeldInterface<IWindowsManager>
    {
    public:
        virtual ~IWindowsManager() = default;

    public:
        virtual void setWantedWindowsSize(int width, int height) = 0;
        virtual void retrieveWindowsDimension(float& outX, float& outY) const = 0;
    };
}
