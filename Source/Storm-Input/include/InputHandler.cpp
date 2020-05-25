#include "InputHandler.h"

#include "SingletonHolder.h"
#include "ISimulatorManager.h"
#include "ITimeManager.h"


namespace
{
	template<class FuncCallback>
	auto makeForwardToSimulationLoopCallback(FuncCallback &&callback)
	{
		return [srcCallback = std::move(callback)]()
		{
			Storm::SingletonHolder::instance().getFacet<Storm::ISimulatorManager>()->executeOnSimulationLoop(srcCallback);
		};
	}
}


bool Storm::InputHandler::keyPressed(const OIS::KeyEvent &arg)
{
	if (arg.key != OIS::KeyCode::KC_ESCAPE)
	{
		std::lock_guard<std::mutex> lock{ _bindingMutex };
		Storm::prettyCallMultiCallback(_keyBindings[arg.text]._onKeyPressed);
	}
	else
	{
		LOG_COMMENT << "Escape key was pressed, we will quit the application!";
		Storm::SingletonHolder::instance().getFacet<Storm::ITimeManager>()->quit();
	}

	return true;
}

bool Storm::InputHandler::keyReleased(const OIS::KeyEvent &)
{
	// Unhandled
	return true;
}

bool Storm::InputHandler::mouseMoved(const OIS::MouseEvent &)
{
	// Unhandled
	return true;
}

bool Storm::InputHandler::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };

	switch (id)
	{
	case OIS::MB_Left:
		Storm::prettyCallMultiCallback(_leftMouseButton._onClick);
		break;

	case OIS::MB_Right:
		Storm::prettyCallMultiCallback(_rightMouseButton._onClick);
		break;

	case OIS::MB_Middle:
		Storm::prettyCallMultiCallback(_middleMouseButton._onClick);
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

Storm::CallbackIdType Storm::InputHandler::bindKey(unsigned int key, Storm::KeyBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _keyBindings[key]._onKeyPressed.add(makeForwardToSimulationLoopCallback(std::move(binding)));
}

void Storm::InputHandler::unbindKey(unsigned int key, Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_keyBindings[key]._onKeyPressed.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseRightClick(Storm::KeyBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _rightMouseButton._onClick.add(makeForwardToSimulationLoopCallback(std::move(binding)));
}

void Storm::InputHandler::unbindMouseRightClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_rightMouseButton._onClick.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseLeftClick(Storm::KeyBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _leftMouseButton._onClick.add(makeForwardToSimulationLoopCallback(std::move(binding)));
}

void Storm::InputHandler::unbindMouseLeftClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_leftMouseButton._onClick.remove(callbackId);
}

Storm::CallbackIdType Storm::InputHandler::bindMouseMiddleClick(Storm::KeyBinding &&binding)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	return _middleMouseButton._onClick.add(makeForwardToSimulationLoopCallback(std::move(binding)));
}

void Storm::InputHandler::unbindMouseMiddleClick(Storm::CallbackIdType callbackId)
{
	std::lock_guard<std::mutex> lock{ _bindingMutex };
	_middleMouseButton._onClick.remove(callbackId);
}
