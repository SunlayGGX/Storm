#pragma once
#include "Singleton.h"
#include "IWindowsManager.h"
#include "WindowsCallbacks.h"


namespace Storm
{
	class WindowsManager :
		private Storm::Singleton<Storm::WindowsManager>,
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

	public:
		void update() final override;

	public:
		void setWantedWindowsSize(int width, int height) final override;
		void retrieveWindowsDimension(float &outX, float &outY) const final override;

	public:
		void* getWindowHandle() const final override;
		const std::wstring& getWindowTitleName() const;
		void* getWindowAccelerationTable() const;

	public:
		void callQuitCallback() final override;
		void callFinishInitializeCallback() final override;

		void unbindCallback() final override;
		void bindQuitCallback(Storm::QuitDelegate &&callback) final override;
		void bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback, bool callNow) final override;

	private:
		void* /*HWND*/ _windowVisuHandle;
		std::wstring _windowClass;
		void* /*HACCEL*/ _accelerationTable;

		bool _hasOverridenWindowSize;
		int _wantedWidth;
		int _wantedHeight;

		mutable std::mutex _callbackMutex;
		Storm::QuitDelegate _quitCallback;
		Storm::FinishedInitializeDelegate _finishedInitCallback;
	};
}
