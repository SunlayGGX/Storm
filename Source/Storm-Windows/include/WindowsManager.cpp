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
    m_quitCallback{ []() {} },
    m_finishInitializeCallback{ [](HWND) {} },
    _hasOverridenWindowSize{ false }
{
}

Storm::WindowsManager::~WindowsManager() = default;

void Storm::WindowsManager::initialize_Implementation()
{
    TCHAR szTitle[MAX_TITLE_COUNT];

    HINSTANCE dllInstance = GetModuleHandle(nullptr);

    LoadString(dllInstance, IDS_TITLE, szTitle, MAX_TITLE_COUNT);
    std::wstring validator = szTitle;
    if (validator.empty())
    {
        constexpr const TCHAR defaultTitle[] = L"SlgApplication";
        memcpy(szTitle, defaultTitle, sizeof(defaultTitle));
    }

    LoadString(dllInstance, IDR_STORM, _windowClass, MAX_TITLE_COUNT);
    validator = _windowClass;
    if (validator.empty())
    {
        constexpr const TCHAR defaultClass[] = L"SlgApplication";
        memcpy(_windowClass, defaultClass, sizeof(defaultClass));
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
        wcex.lpszClassName = _windowClass;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        RegisterClassEx(&wcex);
    }

    if (_hasOverridenWindowSize)
    {
        _windowVisuHandle = CreateWindow(
            _windowClass,
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
        _windowVisuHandle = CreateWindow(
            _windowClass,
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

    if (_windowVisuHandle != nullptr)
    {
        _accelerationTable = LoadAccelerators(dllInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

        this->callFinishInitializeCallback();

        ShowCursor(true);
        ShowWindow(_windowVisuHandle, SW_SHOWNORMAL);
        UpdateWindow(_windowVisuHandle);
    }
    else
    {
        Storm::throwException<std::exception>("Window not created correctly!");
    }
}

void Storm::WindowsManager::cleanUp_Implementation()
{
    this->unbindCallback();
    DestroyWindow(_windowVisuHandle);
    PostQuitMessage(0);
}

void Storm::WindowsManager::update()
{
    MSG msg;

    if (::PeekMessage(&msg, _windowVisuHandle, 0, 0, PM_REMOVE))
    {
        // Closing message ?
        if (msg.message == WM_QUIT)
        {
            this->destroy();
        }

        // Distribute message
        if (!::TranslateAccelerator(msg.hwnd, _accelerationTable, &msg))
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

void Storm::WindowsManager::retrieveWindowsDimension(float& outX, float& outY) const
{
    if (_windowVisuHandle)
    {
        RECT windowsRect;
        GetClientRect(_windowVisuHandle, &windowsRect);

        outX = static_cast<float>(windowsRect.right - windowsRect.left);
        outY = static_cast<float>(windowsRect.bottom - windowsRect.top);
    }
    else
    {
        outX = 0.f;
        outY = 0.f;
    }
}

HWND Storm::WindowsManager::getWindowHandle() const
{
    return _windowVisuHandle;
}

const TCHAR* Storm::WindowsManager::getWindowTitleName() const
{
    return _windowClass;
}

HACCEL Storm::WindowsManager::getWindowAccelerationTable() const
{
    return _accelerationTable;
}

void Storm::WindowsManager::bindQuitCallback(QuitDelegate callback)
{
    std::lock_guard<std::mutex> autoLocker{ m_callbackMutex };
    m_quitCallback = ((callback != nullptr) ? callback : []() {});
}

void Storm::WindowsManager::bindFinishInitializeCallback(FinishInitializeDelegate callback, bool callNow)
{
    std::lock_guard<std::mutex> autoLocker{ m_callbackMutex };
    if (callback != nullptr)
    {
        m_finishInitializeCallback = callback;

        if (callNow && _windowVisuHandle != nullptr)
        {
            m_finishInitializeCallback(_windowVisuHandle);
        }
    }
    else
    {
        m_finishInitializeCallback = [](HWND) {};
    }
}

void Storm::WindowsManager::callQuitCallback()
{
    std::lock_guard<std::mutex> autoLocker{ m_callbackMutex };
    m_quitCallback();
}

void Storm::WindowsManager::callFinishInitializeCallback()
{
    std::lock_guard<std::mutex> autoLocker{ m_callbackMutex };
    m_finishInitializeCallback(_windowVisuHandle);
}

void Storm::WindowsManager::unbindCallback()
{
    std::lock_guard<std::mutex> autoLocker{ m_callbackMutex };
    m_quitCallback = []() {};
    m_finishInitializeCallback = [](HWND) {};
}

