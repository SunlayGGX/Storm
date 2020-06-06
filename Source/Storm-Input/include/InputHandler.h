#pragma once

#include <OISKeyboard.h>
#include <OISMouse.h>

#include "MultiCallback.h"
#include "InputEvents.h"


namespace Storm
{
	class InputHandler :
		public OIS::KeyListener,
		public OIS::MouseListener
	{
	private:
		enum
		{
			k_keyboardKeyBindingCount = 256
		};

	private:
		struct KeyBindings
		{
			Storm::MultiCallback<Storm::KeyBinding> _onKeyPressed;
		};

		struct MouseBindings
		{
			Storm::MultiCallback<Storm::MouseBinding> _onClick;
		};

		struct WheelBindings
		{
			Storm::MultiCallback<Storm::WheelBinding> _onWheelValueChanged;
		};

	public:
		bool keyPressed(const OIS::KeyEvent &arg) final override;
		bool keyReleased(const OIS::KeyEvent &arg) final override;
		bool mouseMoved(const OIS::MouseEvent &arg) final override;
		bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id) final override;
		bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id) final override;

	public:
		Storm::CallbackIdType bindKey(unsigned int key, Storm::KeyBinding &&binding);
		void unbindKey(unsigned int key, Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseRightClick(Storm::KeyBinding &&binding);
		void unbindMouseRightClick(Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseLeftClick(Storm::KeyBinding &&binding);
		void unbindMouseLeftClick(Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseMiddleClick(Storm::KeyBinding &&binding);
		void unbindMouseMiddleClick(Storm::CallbackIdType callbackId);
		Storm::CallbackIdType bindMouseWheelMoved(Storm::WheelBinding &&binding);
		void unbindMouseWheelMoved(Storm::CallbackIdType callbackId);

	public:
		void clear();

	private:
		std::mutex _bindingMutex;

		std::map<unsigned int, KeyBindings> _keyBindings;
		MouseBindings _leftMouseButton;
		MouseBindings _rightMouseButton;
		MouseBindings _middleMouseButton;
		WheelBindings _mouseWheel;
	};
}