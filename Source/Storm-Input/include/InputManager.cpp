#include "InputManager.h"

#include "InputHandler.h"
#include "SpecialKey.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"

#include "ThrowException.h"
#include "MemoryHelper.h"

#include "Version.h"

#include "LeanWindowsInclude.h"

#include <OISInputManager.h>
#include <OISException.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OISJoyStick.h>
#include <OISEvents.h>


namespace Storm
{
	namespace
	{
		// OIS Global device
		// https://github.com/wgois/OIS/blob/v1.5/demos/OISConsoleDemo/OISConsole.cpp

		OIS::InputManager* g_oisInputMgr = nullptr;
		OIS::Keyboard* g_keyboard = nullptr;
		OIS::Mouse* g_mouse = nullptr;

		constexpr const std::string_view g_deviceTypeLUT[] = {
			"OISUnknown",
			"OISKeyboard",
			"OISMouse",
			"OISJoyStick",
			"OISTablet",
			"OISOther"
		};
	}
}

namespace
{
	OIS::KeyCode traduce(Storm::SpecialKey key)
	{
#define STORM_SWITCH_CASE_TRADUCE(stormKey) case Storm::SpecialKey::stormKey: return OIS::KeyCode::stormKey
		switch (key)
		{
			STORM_SWITCH_CASE_TRADUCE(KC_1);
			STORM_SWITCH_CASE_TRADUCE(KC_2);
			STORM_SWITCH_CASE_TRADUCE(KC_3);
			STORM_SWITCH_CASE_TRADUCE(KC_4);
			STORM_SWITCH_CASE_TRADUCE(KC_5);
			STORM_SWITCH_CASE_TRADUCE(KC_6);
			STORM_SWITCH_CASE_TRADUCE(KC_7);
			STORM_SWITCH_CASE_TRADUCE(KC_8);
			STORM_SWITCH_CASE_TRADUCE(KC_9);
			STORM_SWITCH_CASE_TRADUCE(KC_0);
			STORM_SWITCH_CASE_TRADUCE(KC_MINUS);
			STORM_SWITCH_CASE_TRADUCE(KC_EQUALS);
			STORM_SWITCH_CASE_TRADUCE(KC_BACK);
			STORM_SWITCH_CASE_TRADUCE(KC_TAB);
			STORM_SWITCH_CASE_TRADUCE(KC_Q);
			STORM_SWITCH_CASE_TRADUCE(KC_W);
			STORM_SWITCH_CASE_TRADUCE(KC_E);
			STORM_SWITCH_CASE_TRADUCE(KC_R);
			STORM_SWITCH_CASE_TRADUCE(KC_T);
			STORM_SWITCH_CASE_TRADUCE(KC_Y);
			STORM_SWITCH_CASE_TRADUCE(KC_U);
			STORM_SWITCH_CASE_TRADUCE(KC_I);
			STORM_SWITCH_CASE_TRADUCE(KC_O);
			STORM_SWITCH_CASE_TRADUCE(KC_P);
			STORM_SWITCH_CASE_TRADUCE(KC_LBRACKET);
			STORM_SWITCH_CASE_TRADUCE(KC_RBRACKET);
			STORM_SWITCH_CASE_TRADUCE(KC_RETURN);
			STORM_SWITCH_CASE_TRADUCE(KC_LCONTROL);
			STORM_SWITCH_CASE_TRADUCE(KC_A);
			STORM_SWITCH_CASE_TRADUCE(KC_S);
			STORM_SWITCH_CASE_TRADUCE(KC_D);
			STORM_SWITCH_CASE_TRADUCE(KC_F);
			STORM_SWITCH_CASE_TRADUCE(KC_G);
			STORM_SWITCH_CASE_TRADUCE(KC_H);
			STORM_SWITCH_CASE_TRADUCE(KC_J);
			STORM_SWITCH_CASE_TRADUCE(KC_K);
			STORM_SWITCH_CASE_TRADUCE(KC_L);
			STORM_SWITCH_CASE_TRADUCE(KC_SEMICOLON);
			STORM_SWITCH_CASE_TRADUCE(KC_APOSTROPHE);
			STORM_SWITCH_CASE_TRADUCE(KC_GRAVE);
			STORM_SWITCH_CASE_TRADUCE(KC_LSHIFT);
			STORM_SWITCH_CASE_TRADUCE(KC_BACKSLASH);
			STORM_SWITCH_CASE_TRADUCE(KC_Z);
			STORM_SWITCH_CASE_TRADUCE(KC_X);
			STORM_SWITCH_CASE_TRADUCE(KC_C);
			STORM_SWITCH_CASE_TRADUCE(KC_V);
			STORM_SWITCH_CASE_TRADUCE(KC_B);
			STORM_SWITCH_CASE_TRADUCE(KC_N);
			STORM_SWITCH_CASE_TRADUCE(KC_M);
			STORM_SWITCH_CASE_TRADUCE(KC_COMMA);
			STORM_SWITCH_CASE_TRADUCE(KC_PERIOD);
			STORM_SWITCH_CASE_TRADUCE(KC_SLASH);
			STORM_SWITCH_CASE_TRADUCE(KC_RSHIFT);
			STORM_SWITCH_CASE_TRADUCE(KC_MULTIPLY);
			STORM_SWITCH_CASE_TRADUCE(KC_LMENU);
			STORM_SWITCH_CASE_TRADUCE(KC_SPACE);
			STORM_SWITCH_CASE_TRADUCE(KC_CAPITAL);
			STORM_SWITCH_CASE_TRADUCE(KC_F1);
			STORM_SWITCH_CASE_TRADUCE(KC_F2);
			STORM_SWITCH_CASE_TRADUCE(KC_F3);
			STORM_SWITCH_CASE_TRADUCE(KC_F4);
			STORM_SWITCH_CASE_TRADUCE(KC_F5);
			STORM_SWITCH_CASE_TRADUCE(KC_F6);
			STORM_SWITCH_CASE_TRADUCE(KC_F7);
			STORM_SWITCH_CASE_TRADUCE(KC_F8);
			STORM_SWITCH_CASE_TRADUCE(KC_F9);
			STORM_SWITCH_CASE_TRADUCE(KC_F10);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMLOCK);
			STORM_SWITCH_CASE_TRADUCE(KC_SCROLL);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD7);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD8);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD9);
			STORM_SWITCH_CASE_TRADUCE(KC_SUBTRACT);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD4);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD5);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD6);
			STORM_SWITCH_CASE_TRADUCE(KC_ADD);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD1);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD2);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD3);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPAD0);
			STORM_SWITCH_CASE_TRADUCE(KC_DECIMAL);
			STORM_SWITCH_CASE_TRADUCE(KC_OEM_102);
			STORM_SWITCH_CASE_TRADUCE(KC_F11);
			STORM_SWITCH_CASE_TRADUCE(KC_F12);
			STORM_SWITCH_CASE_TRADUCE(KC_F13);
			STORM_SWITCH_CASE_TRADUCE(KC_F14);
			STORM_SWITCH_CASE_TRADUCE(KC_F15);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPADEQUALS);
			STORM_SWITCH_CASE_TRADUCE(KC_AT);
			STORM_SWITCH_CASE_TRADUCE(KC_COLON);
			STORM_SWITCH_CASE_TRADUCE(KC_UNDERLINE);
			STORM_SWITCH_CASE_TRADUCE(KC_STOP);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPADENTER);
			STORM_SWITCH_CASE_TRADUCE(KC_RCONTROL);
			STORM_SWITCH_CASE_TRADUCE(KC_MUTE);
			STORM_SWITCH_CASE_TRADUCE(KC_CALCULATOR);
			STORM_SWITCH_CASE_TRADUCE(KC_PLAYPAUSE);
			STORM_SWITCH_CASE_TRADUCE(KC_MEDIASTOP);
			STORM_SWITCH_CASE_TRADUCE(KC_TWOSUPERIOR);
			STORM_SWITCH_CASE_TRADUCE(KC_VOLUMEDOWN);
			STORM_SWITCH_CASE_TRADUCE(KC_VOLUMEUP);
			STORM_SWITCH_CASE_TRADUCE(KC_WEBHOME);
			STORM_SWITCH_CASE_TRADUCE(KC_NUMPADCOMMA);
			STORM_SWITCH_CASE_TRADUCE(KC_DIVIDE);
			STORM_SWITCH_CASE_TRADUCE(KC_SYSRQ);
			STORM_SWITCH_CASE_TRADUCE(KC_RMENU);
			STORM_SWITCH_CASE_TRADUCE(KC_PAUSE);
			STORM_SWITCH_CASE_TRADUCE(KC_HOME);
			STORM_SWITCH_CASE_TRADUCE(KC_UP);
			STORM_SWITCH_CASE_TRADUCE(KC_PGUP);
			STORM_SWITCH_CASE_TRADUCE(KC_LEFT);
			STORM_SWITCH_CASE_TRADUCE(KC_RIGHT);
			STORM_SWITCH_CASE_TRADUCE(KC_END);
			STORM_SWITCH_CASE_TRADUCE(KC_DOWN);
			STORM_SWITCH_CASE_TRADUCE(KC_PGDOWN);
			STORM_SWITCH_CASE_TRADUCE(KC_INSERT);
			STORM_SWITCH_CASE_TRADUCE(KC_DELETE);
			STORM_SWITCH_CASE_TRADUCE(KC_LWIN);
			STORM_SWITCH_CASE_TRADUCE(KC_RWIN);
			STORM_SWITCH_CASE_TRADUCE(KC_APPS);
			STORM_SWITCH_CASE_TRADUCE(KC_POWER);
			STORM_SWITCH_CASE_TRADUCE(KC_SLEEP);
			STORM_SWITCH_CASE_TRADUCE(KC_WAKE);

			default: Storm::throwException<std::exception>("Unknown special key!");
		}
#undef STORM_SWITCH_CASE_TRADUCE
	}
}


Storm::InputManager::InputManager() :
	_inputHandler{ std::make_unique<Storm::InputHandler>() }
{}

Storm::InputManager::~InputManager() = default;

bool Storm::InputManager::initialize_Implementation()
{
	LOG_COMMENT << "Starting to initialize the Input Manager. We would evaluate if Windows is created. If not, we will suspend initialization and come back later.";

	Storm::IWindowsManager &windowsMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IWindowsManager>();

	HWND hwnd = static_cast<HWND>(windowsMgr.getWindowHandle());
	if (hwnd != nullptr)
	{
		this->initialize_Implementation(hwnd);
		return true;
	}
	else
	{
		bool initRes = false;
		windowsMgr.bindFinishInitializeCallback([this, res = &initRes](void* hwndOnceReady, bool calledAtBindingTime)
		{
			if (calledAtBindingTime)
			{
				// If this was called at binding time, then this is on the same thread so it is okay to still reference res (we haven't left initialize_Implementation yet).
				this->initialize_Implementation(hwndOnceReady);
				*res = true;
			}
			else
			{
				this->initialize(hwndOnceReady);
			}
		});

		LOG_WARNING << "HWND not valid, Input initialization will be suspended and done asynchronously later.";
		return initRes;
	}
}

void Storm::InputManager::initialize_Implementation(void* vptrHwnd)
{
	HWND hwnd = static_cast<HWND>(vptrHwnd);

	OIS::ParamList oisParamsList;
	std::ostringstream wnd;
	wnd << (std::size_t)hwnd;

	oisParamsList.emplace("WINDOW", wnd.str());
	oisParamsList.emplace("w32_mouse", "DISCL_FOREGROUND");
	oisParamsList.emplace("w32_mouse", "DISCL_NONEXCLUSIVE");
	/*oisParamsList.emplace("w32_keyboard", "DISCL_FOREGROUND");
	oisParamsList.emplace("w32_keyboard", "DISCL_NONEXCLUSIVE");*/

	Storm::g_oisInputMgr = OIS::InputManager::createInputSystem(oisParamsList);

	Storm::g_oisInputMgr->enableAddOnFactory(OIS::InputManager::AddOn_All);
	const unsigned int oisVersionInt = OIS::InputManager::getVersionNumber();

	Storm::Version oisVersion { 
		static_cast<Storm::Version::VersionNumber>(oisVersionInt >> 16), 
		static_cast<Storm::Version::VersionNumber>((oisVersionInt >> 8) & 0x000000FF), 
		static_cast<Storm::Version::VersionNumber>(oisVersionInt & 0x000000FF) 
	};

	LOG_COMMENT << "OIS Created to manage input :\n"
		"OIS Version: " << oisVersion << "\n"
		"Release Name: " << Storm::g_oisInputMgr->getVersionName() << "\n"
		"Manager: " << Storm::g_oisInputMgr->inputSystemName() << "\n"
		"Total Keyboards: " << Storm::g_oisInputMgr->getNumberOfDevices(OIS::OISKeyboard) << "\n"
		"Total Mice: " << Storm::g_oisInputMgr->getNumberOfDevices(OIS::OISMouse) << "\n"
		"Total JoySticks: " << Storm::g_oisInputMgr->getNumberOfDevices(OIS::OISJoyStick) << "(Note that we won't support them)";

	if (oisVersion != Storm::Version{ 1, 5 })
	{
		// Typically when the developer that grabbed Storm (either forked or cloned the repository) did not take the right version of OIS.
		LOG_WARNING << "Detected mismatch between the OIS version we have here from what was used when developing Storm application !";
	}

	OIS::DeviceList list = Storm::g_oisInputMgr->listFreeDevices();
	for (const auto &device : list)
	{
		LOG_COMMENT << "\n\tDevice: " << g_deviceTypeLUT[device.first] << " Vendor: " << device.second;
	}

	g_keyboard = static_cast<OIS::Keyboard*>(Storm::g_oisInputMgr->createInputObject(OIS::OISKeyboard, true));
	g_keyboard->setTextTranslation(OIS::Keyboard::TextTranslationMode::Off);
	g_keyboard->setEventCallback(_inputHandler.get());

	g_mouse = static_cast<OIS::Mouse*>(Storm::g_oisInputMgr->createInputObject(OIS::OISMouse, true));
	g_mouse->setEventCallback(_inputHandler.get());

	RECT rcWindow;
	GetWindowRect(hwnd, &rcWindow);

	const OIS::MouseState& mouseState = g_mouse->getMouseState();
	mouseState.width = std::abs(rcWindow.right - rcWindow.left);
	mouseState.height = std::abs(rcWindow.top - rcWindow.bottom);
}

void Storm::InputManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Starting OIS destruction.";

	OIS::InputManager::destroyInputSystem(Storm::g_oisInputMgr);
	_inputHandler->clear();
	
	LOG_COMMENT << "OIS destruction completed.";
}

void Storm::InputManager::update()
{
	this->callSequentialToInitCleanup([this]()
	{
		if (g_keyboard != nullptr)
		{
			g_keyboard->capture();
			if (!g_keyboard->buffered())
			{
				this->handleNonBufferedKeys();
			}
		}
		if (g_mouse != nullptr)
		{
			g_mouse->capture();
			if (!g_mouse->buffered())
			{
				this->handleNonBufferedMouse();
			}
		}
	});
}

Storm::CallbackIdType Storm::InputManager::bindKey(Storm::SpecialKey key, Storm::KeyBinding &&binding)
{
	return _inputHandler->bindKey(traduce(key), std::move(binding));
}

void Storm::InputManager::unbindKey(Storm::SpecialKey key, Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindKey(traduce(key), callbackId);
}

Storm::CallbackIdType Storm::InputManager::bindMouseRightClick(Storm::KeyBinding &&binding)
{
	return _inputHandler->bindMouseRightClick(std::move(binding));
}

void Storm::InputManager::unbindMouseRightClick(Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindMouseRightClick(callbackId);
}

Storm::CallbackIdType Storm::InputManager::bindMouseLeftClick(Storm::KeyBinding &&binding)
{
	return _inputHandler->bindMouseLeftClick(std::move(binding));
}

void Storm::InputManager::unbindMouseLeftClick(Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindMouseLeftClick(callbackId);
}

Storm::CallbackIdType Storm::InputManager::bindMouseMiddleClick(Storm::KeyBinding &&binding)
{
	return _inputHandler->bindMouseMiddleClick(std::move(binding));
}

void Storm::InputManager::unbindMouseMiddleClick(Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindMouseMiddleClick(callbackId);
}

Storm::CallbackIdType Storm::InputManager::bindMouseWheel(Storm::WheelBinding &&binding)
{
	return _inputHandler->bindMouseWheelMoved(std::move(binding));
}

void Storm::InputManager::unbindMouseWheel(Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindMouseWheelMoved(callbackId);
}

void Storm::InputManager::handleNonBufferedKeys()
{
	// TODO
	// (It is here we handle the modifier keys)
}

void Storm::InputManager::handleNonBufferedMouse()
{
	// TODO
}
