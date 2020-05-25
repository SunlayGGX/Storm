#pragma once


#include "Singleton.h"
#include "IInputManager.h"


namespace Storm
{
	class InputHandler;

	class InputManager :
		private Storm::Singleton<InputManager>,
		public Storm::IInputManager
	{
		STORM_DECLARE_SINGLETON(InputManager);

	private:
		bool initialize_Implementation();
		void initialize_Implementation(void* vptrHwnd);
		void cleanUp_Implementation();

	public:
		void update() override;

	public:
		Storm::CallbackIdType bindKey(unsigned int key, Storm::KeyBinding &&binding) final override;
		void unbindKey(unsigned int key, Storm::CallbackIdType callbackId) final override;
		Storm::CallbackIdType bindMouseRightClick(Storm::KeyBinding &&binding) final override;
		void unbindMouseRightClick(Storm::CallbackIdType callbackId) final override;
		Storm::CallbackIdType bindMouseLeftClick(Storm::KeyBinding &&binding) final override;
		void unbindMouseLeftClick(Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseMiddleClick(Storm::KeyBinding &&binding) final override;
		void unbindMouseMiddleClick(Storm::CallbackIdType callbackId) final override;

	private:
		void handleNonBufferedKeys();
		void handleNonBufferedMouse();

	private:
		std::unique_ptr<Storm::InputHandler> _inputHandler;
	};
}
