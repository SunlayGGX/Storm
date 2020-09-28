#include "InputHandler.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "IThreadManager.h"


namespace
{
	template<class ... Args, class FuncCallback>
	auto makeForwardCallbackBinderThread(FuncCallback &&callback)
	{
		return [binderID = std::this_thread::get_id(), srcCallback = std::move(callback)](Args &&... args)
		{
			Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(binderID, std::bind(srcCallback, std::forward<Args>(args)...));
		};
	}
}


bool Storm::InputHandler::keyPressed(const OIS::KeyEvent &arg)
{
	if (arg.key != OIS::KeyCode::KC_ESCAPE)
	{
		std::lock_guard<std::mutex> lock{ _bindingMutex };
		Storm::prettyCallMultiCallback(_keyBindings[arg.key]._onKeyPressed);
	}
	else
	{
		LOG_COMMENT << "Escape key was pressed, we will quit the application!";
		Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>().quit();
	}

	return true;
}

bool Storm::InputHandler::keyReleased(const OIS::KeyEvent &)
{
	// Unhandled
	return true;
}

bool Storm::InputHandler::mouseMoved(const OIS::MouseEvent &arg)
{
	// x and y are unhandled. z is the mouse wheel and this is this one we want to handle.
	if (arg.state.Z.rel != 0)
	{
		Storm::prettyCallMultiCallback(_mouseWheel._onWheelValueChanged, arg.state.Z.rel);
	}

	return true;
}

bool Storm::InputHandler::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };

	switch (id)
	{
	case OIS::MB_Left:
		Storm::prettyCallMultiCallback(_leftMouseButton._onClick, arg.state.X.abs, arg.state.Y.abs, arg.state.width, arg.state.height);
		break;

	case OIS::MB_Right:
		Storm::prettyCallMultiCallback(_rightMouseButton._onClick, arg.state.X.abs, arg.state.Y.abs, arg.state.width, arg.state.height);
		break;

	case OIS::MB_Middle:
		Storm::prettyCallMultiCallback(_middleMouseButton._onClick, arg.state.X.abs, arg.state.Y.abs, arg.state.width, arg.state.height);
		break;

	case OIS::MB_Button3:
	case OIS::MB_Button4:
	case OIS::MB_Button5:
	case OIS::MB_Button6:
	case OIS::MB_Button7:
	default:
		break;
	}

	return true;
}

bool Storm::InputHandler::mouseReleased(const OIS::MouseEvent &, OIS::MouseButtonID)
{
	// Unhandled
	return true;
}

Storm::CallbackIdType Storm::InputHandler::bindKey(OIS::KeyCode key, Storm::KeyBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _keyBindings[key]._onKeyPressed.add(makeForwardCallbackBinderThread(std::move(binding)));
}

void Storm::InputHandler::unbindKey(OIS::KeyCode key, Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_keyBindings[key]._onKeyPressed.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseRightClick(Storm::MouseBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _rightMouseButton._onClick.add(makeForwardCallbackBinderThread<int, int, int, int>(std::move(binding)));
}

void Storm::InputHandler::unbindMouseRightClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_rightMouseButton._onClick.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseLeftClick(Storm::MouseBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _leftMouseButton._onClick.add(makeForwardCallbackBinderThread<int, int, int, int>(std::move(binding)));
}

void Storm::InputHandler::unbindMouseLeftClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_leftMouseButton._onClick.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseMiddleClick(Storm::MouseBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _middleMouseButton._onClick.add(makeForwardCallbackBinderThread<int, int, int, int>(std::move(binding)));
}

void Storm::InputHandler::unbindMouseMiddleClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_middleMouseButton._onClick.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseWheelMoved(Storm::WheelBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _mouseWheel._onWheelValueChanged.add(makeForwardCallbackBinderThread<int>(std::move(binding)));
}

void Storm::InputHandler::unbindMouseWheelMoved(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_mouseWheel._onWheelValueChanged.remove(callbackId);
}

void Storm::InputHandler::clear()
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	for (auto &binding : _keyBindings)
	{
		binding.second._onKeyPressed.clear();
	}
	_middleMouseButton._onClick.clear();
	_leftMouseButton._onClick.clear();
	_rightMouseButton._onClick.clear();
	_mouseWheel._onWheelValueChanged.clear();
}
