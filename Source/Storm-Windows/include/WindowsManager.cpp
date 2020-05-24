#include "WindowsManager.h"

#include "resource.h"
#include "ThrowException.h"


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

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}
}


Storm::WindowsManager::WindowsManager() :
	_hasOverridenWindowSize{ false },
	_accelerationTable{ nullptr },
	_windowVisuHandle{ nullptr }
{

}

Storm::WindowsManager::~WindowsManager() = default;

void Storm::WindowsManager::initialize_Implementation()
{
	LOG_COMMENT << "Starting creating the Windows for the application";

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

	HWND windowVisuHandle;

	if (_hasOverridenWindowSize)
	{
		windowVisuHandle = CreateWindow(
			windowClass,
			szTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			0,
			_wantedWidth,
			_wantedHeight,
			nullptr,
			nullptr,
			dllInstance,
			nullptr
		);
	}
	else
	{
		windowVisuHandle = CreateWindow(
			windowClass,
			szTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			0,
			CW_USEDEFAULT,
			0,
			nullptr,
			nullptr,
			dllInstance,
			nullptr
		);
	}

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
		Storm::throwException<std::exception>("Window not created correctly!");
	}
}

void Storm::WindowsManager::cleanUp_Implementation()
{
	this->unbindCallback();
	DestroyWindow(static_cast<HWND>(_windowVisuHandle));
	PostQuitMessage(0);
}

void Storm::WindowsManager::update()
{
	MSG msg;

	if (::PeekMessage(&msg, static_cast<HWND>(_windowVisuHandle), 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			this->callQuitCallback();
			this->cleanUp();
		}

		// Distribute message
		if (!::TranslateAccelerator(msg.hwnd, static_cast<HACCEL>(_accelerationTable), &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

void Storm::WindowsManager::setWantedWindowsSize(int width, int height)
{
	_wantedWidth = width;
	_wantedHeight = height;
	_hasOverridenWindowSize = true;
}

void Storm::WindowsManager::retrieveWindowsDimension(float &outX, float &outY) const
{
	if (_windowVisuHandle)
	{
		RECT windowsRect;
		GetClientRect(static_cast<HWND>(_windowVisuHandle), &windowsRect);

		outX = static_cast<float>(windowsRect.right - windowsRect.left);
		outY = static_cast<float>(windowsRect.bottom - windowsRect.top);
	}
	else
	{
		outX = 0.f;
		outY = 0.f;
	}
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
	std::lock_guard<std::mutex> autoLocker{ _callbackMutex };
	return _quitCallback.add(std::move(callback));
}

void Storm::WindowsManager::bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback)
{
	std::lock_guard<std::mutex> autoLocker{ _callbackMutex };
	if (_windowVisuHandle != nullptr)
	{
		callback(_windowVisuHandle, true);
	}
	else
	{
		_finishedInitCallback.add(std::move(callback));
	}
}

void Storm::WindowsManager::callQuitCallback()
{
	std::lock_guard<std::mutex> autoLocker{ _callbackMutex };
	_quitCallback();
}

void Storm::WindowsManager::callFinishInitializeCallback()
{
	std::lock_guard<std::mutex> autoLocker{ _callbackMutex };
	_finishedInitCallback(_windowVisuHandle, false);
	_finishedInitCallback.clear();
}

void Storm::WindowsManager::unbindQuitCallback(unsigned short callbackId)
{
	_quitCallback.remove(callbackId);
}

void Storm::WindowsManager::unbindCallbacks()
{
	std::lock_guard<std::mutex> autoLocker{ _callbackMutex };
	_quitCallback.clear();
	_finishedInitCallback.clear();
}
