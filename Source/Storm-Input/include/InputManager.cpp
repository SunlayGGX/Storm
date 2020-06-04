#include "InputManager.h"

#include "InputHandler.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"

#include "ThrowException.h"
#include "MemoryHelper.h"

#include "Version.h"

#include <OISInputManager.h>
#include <OISException.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OISJoyStick.h>
#include <OISEvents.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#	include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN


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

Storm::CallbackIdType Storm::InputManager::bindKey(unsigned int key, Storm::KeyBinding &&binding)
{
	return _inputHandler->bindKey(key, std::move(binding));
}

void Storm::InputManager::unbindKey(unsigned int key, Storm::CallbackIdType callbackId)
{
	_inputHandler->unbindKey(key, callbackId);
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

void Storm::InputManager::handleNonBufferedKeys()
{
	// TODO
	// (It is here we handle the modifier keys)
}

void Storm::InputManager::handleNonBufferedMouse()
{
	// TODO
}
