#pragma once

#include "SingletonHeldInterfaceBase.h"
#include "CallbackIdType.h"
#include "InputEvents.h"


namespace Storm
{
	enum class InputKeyState;
	enum class SpecialKey;

	class IInputManager : public Storm::ISingletonHeldInterface<IInputManager>
	{
	public:
		virtual ~IInputManager() = default;

	public:
		virtual void update() = 0;

		virtual Storm::CallbackIdType bindKey(Storm::SpecialKey key, Storm::KeyBinding &&binding) = 0;
		virtual void unbindKey(Storm::SpecialKey key, Storm::CallbackIdType callbackId) = 0;
		virtual Storm::CallbackIdType bindMouseRightClick(Storm::MouseBinding &&binding) = 0;
		virtual void unbindMouseRightClick(Storm::CallbackIdType callbackId) = 0;
		virtual Storm::CallbackIdType bindMouseLeftClick(Storm::MouseBinding &&binding) = 0;
		virtual void unbindMouseLeftClick(Storm::CallbackIdType callbackId) = 0;
		virtual Storm::CallbackIdType bindMouseMiddleClick(Storm::MouseBinding &&binding) = 0;
		virtual void unbindMouseMiddleClick(Storm::CallbackIdType callbackId) = 0;
		virtual Storm::CallbackIdType bindMouseWheel(Storm::WheelBinding &&binding) = 0;
		virtual void unbindMouseWheel(Storm::CallbackIdType callbackId) = 0;
	};
}
