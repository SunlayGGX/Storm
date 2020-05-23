#pragma once

#include "SingletonHeldInterfaceBase.h"
#include "WindowsCallbacks.h"


namespace Storm
{
	class IWindowsManager : public Storm::ISingletonHeldInterface<IWindowsManager>
	{
	public:
		virtual ~IWindowsManager() = default;

	public:
		virtual void setWantedWindowsSize(int width, int height) = 0;
		virtual void retrieveWindowsDimension(float& outX, float& outY) const = 0;

		virtual void* getWindowHandle() const = 0;

	public:
		virtual void callQuitCallback() = 0;
		virtual void callFinishInitializeCallback() = 0;

		virtual void unbindCallback() = 0;
		virtual void bindQuitCallback(Storm::QuitDelegate &&callback) = 0;
		virtual void bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback, bool callNow) = 0;

	public:
		virtual void update() = 0;
	};
}
