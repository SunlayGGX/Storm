#include "WindowsManager.h"

#include "resource.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IThreadManager.h"
#include "ISimulatorManager.h"
#include "IConfigManager.h"

#include "GeneralGraphicConfig.h"
#include "GeneralApplicationConfig.h"
#include "GeneratedGitConfig.h"
#include "InternalReferenceConfig.h"

#include "DynamicMenuBuilder.h"

#include "ThreadHelper.h"
#include "ThreadEnumeration.h"
#include "StringAlgo.h"
#include "MFCHelper.h"

#include "UIModality.h"

#include "StormProcessOpener.h"


namespace
{
	LRESULT CALLBACK wndProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hdc;

		switch (message)
		{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			Storm::WindowsManager::instance().callQuitCallback();
			break;

		case WM_COMMAND:
		{
			std::size_t outProcessUID;

			Storm::WindowsManager &windowsMgr = Storm::WindowsManager::instance();

			const int wmId = LOWORD(wParam);
			// Analyze menu selection
			switch (wmId)
			{
			case IDCLOSE:
			case ID_FILE_QUIT:
				windowsMgr.callQuitCallback();
				break;

			case ID_STORM_FILE_SAVE:
				Storm::SingletonHolder::instance().getSingleton<Storm::ISimulatorManager>().saveSimulationState();
				break;

			case ID_TOOLS_STORM: // In fact it is the logviewer.
			case ID_STORM_LOG_VIEWER:
				Storm::StormProcessOpener::openStormLogViewer(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID);
				break;

			case ID_TOOLS_SCRIPT:
			case ID_STORM_SCRIPT:
			case ID_NOTEPAD_SCRIPT:
				Storm::StormProcessOpener::openRuntimeScript(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID);
				break;

			case ID_STORM_CONFIG:
			case ID_NOTEPAD_SCENECONFIGXML:
				Storm::StormProcessOpener::openCurrentConfigFile(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID);
				break;
				
			case ID_LINK_GITHUB:
				Storm::StormProcessOpener::openStormGithubLink(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID);
				break;

			case ID_LINK_APPLICATIONROOT:
				Storm::StormProcessOpener::openStormRootExplorer(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID);
				break;

			case ID_FILE_RESTART:
				if (Storm::StormProcessOpener::openStormRestarter(Storm::StormProcessOpener::OpenParameter{
					._failureQuit = false
				}, outProcessUID))
				{
					windowsMgr.callQuitCallback();
				}
				break;

			default:
				if (!windowsMgr.getMenuBuilderHandler()(wmId))
				{
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
		}
		break;

		case WM_SIZE:
		{
			Storm::WindowsManager::instance().callWindowsResizedCallback(LOWORD(lParam), HIWORD(lParam));
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	BOOL ctrlHandlerRoutine(_In_ DWORD dwCtrlType)
	{
		switch (dwCtrlType)
		{
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			Storm::WindowsManager::instance().callQuitCallback();
			std::this_thread::sleep_for(std::chrono::seconds{ 2 });
			return TRUE;

		default:
			return FALSE;
		}
	}

	template<std::size_t maxTitleCount, class AddedStrType>
	void appendToTitle(TCHAR(&inOutTitle)[maxTitleCount], std::size_t &titlePositionIter, const AddedStrType &addedStr, const std::string_view &operation)
	{
		try
		{
			if (Storm::StringAlgo::extractSize(addedStr) > 0)
			{
				const std::wstring convertedAddedWStr = Storm::toStdWString(addedStr);

				const std::size_t currentAddedStrLength = convertedAddedWStr.size();
				const std::size_t toCopy = std::max(titlePositionIter - (currentAddedStrLength + 4), static_cast<std::size_t>(0));
				if (toCopy != 0)
				{
					inOutTitle[titlePositionIter++] = L' ';
					inOutTitle[titlePositionIter++] = L'-';
					inOutTitle[titlePositionIter++] = L' ';
					memcpy(inOutTitle + titlePositionIter, convertedAddedWStr.c_str(), currentAddedStrLength * sizeof(TCHAR));
					titlePositionIter += currentAddedStrLength;
				}
			}
		}
		catch (const std::exception &e)
		{
			LOG_ERROR << "Couldn't embed the " << operation << " into the application title. Reason : " << e.what();
		}
	}
}


Storm::WindowsManager::WindowsManager() :
	_accelerationTable{ nullptr },
	_windowVisuHandle{ nullptr },
	_menuBuilderHandler{ std::make_unique<Storm::DynamicMenuBuilder>() }
{

}

Storm::WindowsManager::~WindowsManager() = default;

void Storm::WindowsManager::initialize_Implementation(const Storm::WithUI &)
{
	LOG_COMMENT << "Starting creating the Windows for the application";

	::SetConsoleCtrlHandler(ctrlHandlerRoutine, TRUE);

	std::condition_variable syncronizer;
	bool canLeave = false;
	std::mutex syncMutex;

	_windowsThread = std::thread{ [this, &syncronizer, &canLeave, &syncMutex]()
	{
		STORM_REGISTER_THREAD(WindowsAndInputThread);

		this->initializeInternal();

		{
			std::unique_lock<std::mutex> lock{ syncMutex };
			canLeave = true;
			syncronizer.notify_all();
		}

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		inputMgr.bindMouseLeftClick([this, &inputMgr](int, int, int, int)
		{
			this->focus();
			inputMgr.clearKeyboardState();
		});

		constexpr const std::chrono::milliseconds k_windowsThreadRefreshRate{ 100 };
		while (timeMgr.waitForTimeOrExit(k_windowsThreadRefreshRate))
		{
			this->update();
			inputMgr.update();
			threadMgr.processCurrentThreadActions();
		}
	} };

	std::unique_lock<std::mutex> lock{ syncMutex };
	syncronizer.wait(lock, [&canLeave]() { return canLeave; });
}

void Storm::WindowsManager::initialize_Implementation(const Storm::NoUI &)
{
	LOG_COMMENT << "No UI requested, we will skip UI generation.";

	::SetConsoleCtrlHandler(ctrlHandlerRoutine, TRUE);

	_windowsThread = std::thread{ [this]()
	{
		STORM_REGISTER_THREAD(WindowsAndInputThread);

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

		constexpr const std::chrono::milliseconds k_windowsThreadRefreshRate{ 500 };
		while (timeMgr.waitForTimeOrExit(k_windowsThreadRefreshRate))
		{
			threadMgr.processCurrentThreadActions();
		}
	} };
}

void Storm::WindowsManager::cleanUp_Implementation(const Storm::WithUI &)
{
	this->unbindCallbacks();
	DestroyWindow(static_cast<HWND>(_windowVisuHandle));
	PostQuitMessage(0);
	Storm::join(_windowsThread);
}

void Storm::WindowsManager::cleanUp_Implementation(const Storm::NoUI &)
{
	this->unbindCallbacks();
	Storm::join(_windowsThread);
}

void Storm::WindowsManager::initializeInternal()
{
	STORM_STATIC_ASSERT(Storm::WindowsManager::MAX_TITLE_COUNT > 20, "Minimal title character size must be 20.");

	LOG_COMMENT << "Building Application form UI (main Window)";

	const Storm::SingletonHolder &singletonMgr = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonMgr.getSingleton<Storm::IConfigManager>();

	TCHAR szTitle[Storm::WindowsManager::MAX_TITLE_COUNT];

	HINSTANCE dllInstance = ::GetModuleHandle(nullptr);

	if (!::LoadString(dllInstance, IDS_TITLE, szTitle, MAX_TITLE_COUNT))
	{
		LOG_ERROR << "Windows application name couldn't be loaded. Error code is " << Storm::toStdString(GetLastError());
	}

	std::wstring_view validator = szTitle;
	if (validator.empty())
	{
		constexpr const TCHAR defaultTitle[] = L"SlgApplication";
		memcpy(szTitle, defaultTitle, sizeof(defaultTitle));
	}

	std::size_t titleLength = validator.size();
	if (titleLength < Storm::WindowsManager::MAX_TITLE_COUNT)
	{
		try
		{
			const std::wstring currentProcessIdStr = toStdWString(Storm::toStdString(configMgr.getCurrentPID()));
			const std::size_t currentProcessIdStrLength = currentProcessIdStr.size();
			const std::size_t toCopy = std::max(Storm::WindowsManager::MAX_TITLE_COUNT - 4 - static_cast<int>(titleLength + currentProcessIdStrLength), 0);
			if (toCopy != 0)
			{
				szTitle[titleLength++] = L' ';
				szTitle[titleLength++] = L'(';
				memcpy(szTitle + titleLength, currentProcessIdStr.c_str(), currentProcessIdStrLength * sizeof(decltype(currentProcessIdStr)::value_type));
				titleLength += currentProcessIdStrLength;
				szTitle[titleLength++] = L')';
			}
		}
		catch (const std::exception &e)
		{
			LOG_ERROR << "Couldn't embed the process id into the application title. Reason : " << e.what();
		}

		appendToTitle(szTitle, titleLength, configMgr.getSceneName(), "current scene name");
		appendToTitle(szTitle, titleLength, configMgr.getSimulationTypeName(), "current simulation type");

		const Storm::GeneralApplicationConfig &generalAppConfig = configMgr.getGeneralApplicationConfig();
		if (generalAppConfig._showBranchInTitle)
		{
			const Storm::GeneratedGitConfig &generatedGitCfg = configMgr.getInternalGeneratedGitConfig();
			appendToTitle(szTitle, titleLength, generatedGitCfg._gitBranchWStr, "The git branch exe was built unto");
		}

		szTitle[titleLength] = L'\0';
	}

	TCHAR windowClass[Storm::WindowsManager::MAX_TITLE_COUNT];
	if (!::LoadString(dllInstance, IDS_STORM, windowClass, MAX_TITLE_COUNT))
	{
		LOG_ERROR << "Windows application class couldn't be loaded. Error code is " << Storm::toStdString(GetLastError());
	}

	validator = windowClass;
	if (validator.empty())
	{
		constexpr const TCHAR defaultClass[] = L"SlgApplication";
		memcpy(windowClass, defaultClass, sizeof(defaultClass));
	}

	{
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &wndProcCallback;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = dllInstance;
		wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_STORM));
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = MAKEINTRESOURCE(IDR_STORM);
		wcex.lpszClassName = windowClass;
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		RegisterClassEx(&wcex);
	}

	const Storm::GeneralGraphicConfig &generalGraphicConfig = configMgr.getGeneralGraphicConfig();

	int xPos = generalGraphicConfig._wantedApplicationXPos;
	int yPos = generalGraphicConfig._wantedApplicationYPos;

	if (xPos == std::numeric_limits<int>::max())
	{
		xPos = CW_USEDEFAULT;
	}
	if (yPos == std::numeric_limits<int>::max())
	{
		yPos = CW_USEDEFAULT;
	}

	HWND windowVisuHandle = CreateWindow(
		windowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		xPos,
		yPos,
		CW_USEDEFAULT,
		0,
		nullptr,
		nullptr,
		dllInstance,
		nullptr
	);

	if (windowVisuHandle != nullptr)
	{
		LOG_DEBUG << "Building the dynamic part of the menu toolbar.";
		HMENU mainMenu = ::GetMenu(windowVisuHandle);
		if (mainMenu != nullptr)
		{
			const std::vector<Storm::InternalReferenceConfig> &referencesConfig = configMgr.getInternalReferencesConfig();
			if (!referencesConfig.empty())
			{
				HMENU linkReferenceSubmenu = Storm::MFCHelper::findMenuByName(mainMenu, STORM_TEXT("References"));
				for (const Storm::InternalReferenceConfig &referenceConfig : referencesConfig)
				{
					if (!referenceConfig._url.empty())
					{
						_menuBuilderHandler->appendMenu(linkReferenceSubmenu, Storm::toStdWString(referenceConfig._name), [&referenceConfig]()
						{
							std::size_t outProcessUID;
							Storm::StormProcessOpener::openStormUrlLink(Storm::StormProcessOpener::OpenParameter{
								._failureQuit = false
							}, outProcessUID, referenceConfig._url);
						});
					}
				}
			}
			else
			{
				Storm::MFCHelper::setMenuEnabled(mainMenu, ID_LINK_REFERENCES, false);
			}
		}
		else
		{
			LOG_DEBUG_ERROR << "Menu couldn't be grabbed from Main windows. Something went wrong.";
		}

		LOG_DEBUG << "Loading the input accelerator.";
		_accelerationTable = ::LoadAccelerators(dllInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

		LOG_DEBUG << "Finishing the main window UI creation.";
		::ShowCursor(true);
		::ShowWindow(windowVisuHandle, SW_SHOWNORMAL);
		::UpdateWindow(windowVisuHandle);

		_windowVisuHandle = windowVisuHandle;
		_windowClass = validator;

		LOG_COMMENT << "Windows for the application initialized correctly";

		this->callFinishInitializeCallback();
	}
	else
	{
		Storm::throwException<Storm::Exception>("Windows not created correctly!");
	}
}

void Storm::WindowsManager::update()
{
	MSG msg;

	if (::PeekMessage(&msg, static_cast<HWND>(_windowVisuHandle), 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			this->callQuitCallback();
			this->cleanUp(Storm::WithUI{});
		}

		// Distribute message
		if (!::TranslateAccelerator(msg.hwnd, static_cast<HACCEL>(_accelerationTable), &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

void Storm::WindowsManager::retrieveWindowsDimension(int &outX, int &outY) const
{
	if (_windowVisuHandle)
	{
		RECT windowsRect;
		GetClientRect(static_cast<HWND>(_windowVisuHandle), &windowsRect);

		outX = windowsRect.right - windowsRect.left;
		outY = windowsRect.bottom - windowsRect.top;
	}
	else
	{
		outX = 0;
		outY = 0;
	}
}

void Storm::WindowsManager::retrieveWindowsDimension(float &outX, float &outY) const
{
	int intX;
	int intY;
	this->retrieveWindowsDimension(intX, intY);
	outX = static_cast<float>(intX);
	outY = static_cast<float>(intY);
}

void* Storm::WindowsManager::getWindowHandle() const
{
	return _windowVisuHandle;
}

const std::wstring& Storm::WindowsManager::getWindowTitleName() const
{
	return _windowClass;
}

void* Storm::WindowsManager::getWindowAccelerationTable() const
{
	return _accelerationTable;
}

unsigned short Storm::WindowsManager::bindQuitCallback(Storm::QuitDelegate &&callback)
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	return _quitCallback.add(std::move(callback));
}

void Storm::WindowsManager::bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback)
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	if (_windowVisuHandle != nullptr)
	{
		callback(_windowVisuHandle, true);
	}
	else
	{
		_finishedInitCallback.add(std::move(callback));
	}
}

unsigned short Storm::WindowsManager::bindWindowsResizedCallback(Storm::WindowsResizedDelegate &&callback)
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	return _windowsResizedCallback.add(std::move(callback));
}

void Storm::WindowsManager::unbindWindowsResizedCallback(unsigned short callbackId)
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	_windowsResizedCallback.remove(callbackId);
}

void Storm::WindowsManager::callQuitCallback()
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	Storm::prettyCallMultiCallback(_quitCallback);
}

void Storm::WindowsManager::callFinishInitializeCallback()
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	Storm::prettyCallMultiCallback(_finishedInitCallback, _windowVisuHandle, false);
	_finishedInitCallback.clear();
}

void Storm::WindowsManager::callWindowsResizedCallback()
{
	int newWidth;
	int newHeight;

	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };

	this->retrieveWindowsDimension(newWidth, newHeight);
	Storm::prettyCallMultiCallback(_windowsResizedCallback, newWidth, newHeight);
}

void Storm::WindowsManager::callWindowsResizedCallback(unsigned int newWidth, unsigned int newHeight)
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	Storm::prettyCallMultiCallback(_windowsResizedCallback, newWidth, newHeight);
}

void Storm::WindowsManager::unbindQuitCallback(unsigned short callbackId)
{
	_quitCallback.remove(callbackId);
}

void Storm::WindowsManager::unbindCallbacks()
{
	std::lock_guard<std::recursive_mutex> autoLocker{ _callbackMutex };
	_quitCallback.clear();
	_finishedInitCallback.clear();
	_windowsResizedCallback.clear();
}

void Storm::WindowsManager::focus()
{
	if (_windowVisuHandle != nullptr)
	{
		//::PostMessage(static_cast<HWND>(_windowVisuHandle), WM_SETFOCUS, 0, 0);
		::SetFocus(static_cast<HWND>(_windowVisuHandle));
	}
}

Storm::DynamicMenuBuilder& Storm::WindowsManager::getMenuBuilderHandler() noexcept
{
	return *_menuBuilderHandler;
}
