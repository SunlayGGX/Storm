#pragma once


#include "Singleton.h"
#include "IInputManager.h"


namespace Storm
{
	class InputHandler;
	
	struct WithUI;
	struct NoUI;

	class InputManager :
		private Storm::Singleton<InputManager>,
		public Storm::IInputManager
	{
		STORM_DECLARE_SINGLETON(InputManager);

	private:
		bool initialize_Implementation(const Storm::WithUI &);
		void initialize_Implementation(const Storm::NoUI &);
		void initialize_Implementation(void* vptrHwnd);
		void cleanUp_Implementation(const Storm::WithUI &);
		void cleanUp_Implementation(const Storm::NoUI &);

	public:
		void update() override;

	private:
		void refreshMouseState();

		bool isAnyConventionalModifierDown() const;

	public:
		Storm::CallbackIdType bindKey(Storm::SpecialKey key, Storm::KeyBinding &&binding) final override;
		void unbindKey(Storm::SpecialKey key, Storm::CallbackIdType callbackId) final override;
		Storm::CallbackIdType bindMouseRightClick(Storm::MouseBinding &&binding) final override;
		void unbindMouseRightClick(Storm::CallbackIdType callbackId) final override;
		Storm::CallbackIdType bindMouseLeftClick(Storm::MouseBinding &&binding) final override;
		void unbindMouseLeftClick(Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseMiddleClick(Storm::MouseBinding &&binding) final override;
		void unbindMouseMiddleClick(Storm::CallbackIdType callbackId) final override;
		Storm::CallbackIdType bindMouseWheel(Storm::WheelBinding &&binding) final override;
		void unbindMouseWheel(Storm::CallbackIdType callbackId) final override;

	private:
		void handleNonBufferedKeys();
		void handleNonBufferedMouse();

	private:
		std::unique_ptr<Storm::InputHandler> _inputHandler;

		unsigned short _windowsResizedCallbackId;
	};
}
