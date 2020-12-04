#include "WindowsManager.h"

#include "resource.h"
#include "ThrowException.h"
#include "ThreadHelper.h"
#include "ThreadEnumeration.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "IInputManager.h"
#include "IThreadManager.h"
#include "IConfigManager.h"

#include "UIModality.h"


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
			int wmId = LOWORD(wParam);
			// Analyse menu selection
			switch (wmId)
			{
			case IDCLOSE:
				Storm::WindowsManager::instance().callQuitCallback();
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
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
}


Storm::WindowsManager::WindowsManager() :
	_accelerationTable{ nullptr },
	_windowVisuHandle{ nullptr }
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

	TCHAR szTitle[Storm::WindowsManager::MAX_TITLE_COUNT];

	HINSTANCE dllInstance = GetModuleHandle(nullptr);

	LoadString(dllInstance, IDS_TITLE, szTitle, MAX_TITLE_COUNT);
	std::wstring_view validator = szTitle;
	if (validator.empty())
	{
		constexpr const TCHAR defaultTitle[] = L"SlgApplication";
		memcpy(szTitle, defaultTitle, sizeof(defaultTitle));
	}

	TCHAR windowClass[Storm::WindowsManager::MAX_TITLE_COUNT];
	LoadString(dllInstance, IDR_STORM, windowClass, MAX_TITLE_COUNT);
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

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	int xPos = configMgr.getWantedScreenXPosition();
	int yPos = configMgr.getWantedScreenYPosition();

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
		_accelerationTable = LoadAccelerators(dllInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

		ShowCursor(true);
		ShowWindow(windowVisuHandle, SW_SHOWNORMAL);
		UpdateWindow(windowVisuHandle);

		_windowVisuHandle = windowVisuHandle;
		_windowClass = validator;

		LOG_COMMENT << "Windows for the application initialized correctly";

		this->callFinishInitializeCallback();
	}
	else
	{
		Storm::throwException<std::exception>("Windows not created correctly!");
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
