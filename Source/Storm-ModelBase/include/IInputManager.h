#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class InputKeyState;

	class IInputManager : public Storm::ISingletonHeldInterface<IInputManager>
	{
	public:
		virtual ~IInputManager() = default;

	public:
		virtual void update() = 0;

		virtual IInputManager* bindKey(unsigned char key, Storm::InputKeyState keyState, std::function<void()> &&binding) = 0;
		virtual IInputManager* unbindKey(unsigned char key, Storm::InputKeyState keyState) = 0;
		virtual IInputManager* unbindKey(unsigned char key) = 0;
		virtual IInputManager* bindMouseWheel(std::function<void(float)> &&binding) = 0;
		virtual IInputManager* unbindMouseWheel() = 0;
		virtual IInputManager* bindLeftMouseButton(std::function<void()> &&binding) = 0;
		virtual IInputManager* unbindLeftMouseButton() = 0;
		virtual IInputManager* bindRightMouseButton(std::function<void()> &&binding) = 0;
		virtual IInputManager* unbindRightMouseButton() = 0;
	};
}
