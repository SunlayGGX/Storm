#include "InputManager.h"
#include "InputKeyState.h"
#include "ThrowException.h"


namespace
{
	template<class Keybindings>
	std::function<void()>& extractBindingFromState(Keybindings &keyBindings, const Storm::InputKeyState key)
	{
		switch (key)
		{
		case Storm::InputKeyState::Pressed:
			return keyBindings._keyPressedBinding;
		case Storm::InputKeyState::Released:
			return keyBindings._keyReleasedBinding;
		}

		Storm::throwException<std::exception>("Unknown key state!");
	}
}


Storm::InputManager::InputManager() = default;
Storm::InputManager::~InputManager() = default;

void Storm::InputManager::initialize_Implementation()
{

}

void Storm::InputManager::cleanUp_Implementation()
{

}

void Storm::InputManager::update()
{
	throw std::logic_error("The method or operation is not implemented.");
}

Storm::IInputManager* Storm::InputManager::bindKey(unsigned char key, Storm::InputKeyState keyState, std::function<void()> &&binding)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	extractBindingFromState(_keyBindings[key], keyState) = std::move(binding);
	return this;
}

Storm::IInputManager* Storm::InputManager::unbindKey(unsigned char key, Storm::InputKeyState keyState)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	extractBindingFromState(_keyBindings[key], keyState) = []() {};
	return this;
}

Storm::IInputManager* Storm::InputManager::unbindKey(unsigned char key)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	Storm::InputManager::KeyBinding &keyBinding = _keyBindings[key];
	keyBinding._keyPressedBinding = []() {};
	keyBinding._keyReleasedBinding = []() {};
	return this;
}

Storm::IInputManager* Storm::InputManager::bindMouseWheel(std::function<void(float)> &&binding)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._wheel = std::move(binding);
	return this;
}

Storm::IInputManager* Storm::InputManager::unbindMouseWheel()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._wheel = [](float) {};
	return this;
}

Storm::IInputManager* Storm::InputManager::bindLeftMouseButton(std::function<void()> &&binding)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._leftButton = std::move(binding);
	return this;
}

Storm::IInputManager* Storm::InputManager::unbindLeftMouseButton()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._leftButton = []() {};
	return this;
}

Storm::IInputManager* Storm::InputManager::bindRightMouseButton(std::function<void()> &&binding)
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._rightButton = std::move(binding);
	return this;
}

Storm::IInputManager* Storm::InputManager::unbindRightMouseButton()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_mouseBindings._rightButton = []() {};
	return this;
}
