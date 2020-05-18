#pragma once
#include "Singleton.h"
#include "IWindowsManager.h"


namespace Storm
{
    class WindowsManager :
        private Storm::Singleton<WindowsManager>,
        public Storm::IWindowsManager
    {
        STORM_DECLARE_SINGLETON(WindowsManager);

    private:
        enum
        {
            MAX_TITLE_COUNT = 100,
        };

    private:
        void initialize_Implementation();
        void cleanUp_Implementation();

        void update();

    public:
        void setWantedWindowsSize(int width, int height) final override;
        void retrieveWindowsDimension(float& outX, float& outY) const final override;

    public:
        HWND Storm::WindowsManager::getWindowHandle() const;
        const TCHAR* Storm::WindowsManager::getWindowTitleName() const;
        HACCEL Storm::WindowsManager::getWindowAccelerationTable() const;

    public:
        void callQuitCallback();
        void callFinishInitializeCallback();
        void unbindCallback();

    private:
        HWND _windowVisuHandle;
        TCHAR _windowClass[Storm::WindowsManager::MAX_TITLE_COUNT];
        HACCEL _accelerationTable;

        bool _hasOverridenWindowSize;
        int _wantedWidth;
        int _wantedHeight;
    };
}
