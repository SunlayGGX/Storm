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
		virtual void retrieveWindowsDimension(int &outX, int &outY) const = 0;
		virtual void retrieveWindowsDimension(float& outX, float& outY) const = 0;

		virtual void* getWindowHandle() const = 0;

	public:
		virtual void callQuitCallback() = 0;
		virtual void callFinishInitializeCallback() = 0;
		virtual void callWindowsResizedCallback() = 0;

		virtual void unbindQuitCallback(unsigned short callbackId) = 0;
		virtual unsigned short bindQuitCallback(Storm::QuitDelegate &&callback) = 0;
		virtual void bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback) = 0;
		virtual unsigned short bindWindowsResizedCallback(Storm::WindowsResizedDelegate &&callback) = 0;
		virtual void unbindWindowsResizedCallback(unsigned short callbackId) = 0;

		virtual void focus() = 0;

	public:
		virtual void update() = 0;
	};
}
