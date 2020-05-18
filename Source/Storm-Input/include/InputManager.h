#pragma once


#include "Singleton.h"
#include "IInputManager.h"


namespace Storm
{
	class InputManager :
		private Storm::Singleton<InputManager>,
		public Storm::IInputManager
	{
		STORM_DECLARE_SINGLETON(InputManager);

	private:
		enum
		{
			k_keyboardKeyBindingCount = 256
		};

	private:
		struct KeyBinding
		{
			std::function<void()> _keyPressedBinding = []() {};
			std::function<void()> _keyReleasedBinding = []() {};
		};

		struct MouseBinding
		{
			std::function<void()> _leftButton = []() {};
			std::function<void()> _rightButton = []() {};
			std::function<void(float)> _wheel = [](float) {};
		};

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void update() override;

	public:
		IInputManager* bindKey(unsigned char key, Storm::InputKeyState keyState, std::function<void()> &&binding) final override;
		IInputManager* unbindKey(unsigned char key, Storm::InputKeyState keyState) final override;
		IInputManager* unbindKey(unsigned char key) final override;
		IInputManager* bindMouseWheel(std::function<void(float)> &&binding) final override;
		IInputManager* unbindMouseWheel() final override;
		IInputManager* bindLeftMouseButton(std::function<void()> &&binding) final override;
		IInputManager* unbindLeftMouseButton() final override;
		IInputManager* bindRightMouseButton(std::function<void()> &&binding) final override;
		IInputManager* unbindRightMouseButton() final override;

	private:
		mutable std::mutex _mutex;
		KeyBinding _keyBindings[Storm::InputManager::k_keyboardKeyBindingCount];
		MouseBinding _mouseBindings;
	};
}
