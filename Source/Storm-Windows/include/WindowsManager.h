#pragma once

#include "Singleton.h"
#include "IWindowsManager.h"
#include "WindowsCallbacks.h"
#include "MultiCallback.h"
#include "DeclareScriptableItem.h"


namespace Storm
{
	struct WithUI;
	struct NoUI;

	class DynamicMenuBuilder;

	class WindowsManager final :
		private Storm::Singleton<Storm::WindowsManager>,
		public Storm::IWindowsManager
	{
		STORM_DECLARE_SINGLETON(WindowsManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		enum
		{
			MAX_TITLE_COUNT = 256,
		};

	private:
		void initialize_Implementation(const Storm::WithUI &);
		void initialize_Implementation(const Storm::NoUI &);
		void cleanUp_Implementation(const Storm::WithUI &);
		void cleanUp_Implementation(const Storm::NoUI &);

		void initializeInternal();

	public:
		void update() final override;

	public:
		void retrieveWindowsDimension(int &outX, int &outY) const final override;
		void retrieveWindowsDimension(float &outX, float &outY) const final override;

	public:
		void* getWindowHandle() const final override;
		const std::wstring& getWindowTitleName() const;
		void* getWindowAccelerationTable() const;

	public:
		void callQuitCallback() final override;
		void callFinishInitializeCallback() final override;
		void callWindowsResizedCallback() final override;

		void unbindQuitCallback(unsigned short callbackId) final override;
		unsigned short bindQuitCallback(Storm::QuitDelegate &&callback) final override;
		void bindFinishInitializeCallback(Storm::FinishedInitializeDelegate &&callback) final override;
		unsigned short bindWindowsResizedCallback(Storm::WindowsResizedDelegate &&callback) final override;
		void unbindWindowsResizedCallback(unsigned short callbackId) final override;
		unsigned short bindWindowsMovedCallback(Storm::WindowsMovedDelegate &&callback) final override;
		void unbindWindowsMovedCallback(unsigned short callbackId) final override;

	private:
		void unbindCallbacks();

	public:
		void callWindowsResizedCallback(unsigned int newWidth, unsigned int newHeight);
		void callWindowsMovedCallback(unsigned int newX, unsigned int newY);

	public:
		void focus() final override;

	public:
		Storm::DynamicMenuBuilder& getMenuBuilderHandler() noexcept;

	public:
		void restartApplication(const std::string_view &additionalArgs);
		void resetApplication();

	private:
		void* /*HWND*/ _windowVisuHandle;
		std::wstring _windowClass;
		void* /*HACCEL*/ _accelerationTable;

		std::unique_ptr<Storm::DynamicMenuBuilder> _menuBuilderHandler;

		mutable std::recursive_mutex _callbackMutex;
		Storm::MultiCallback<Storm::QuitDelegate> _quitCallback;
		Storm::MultiCallback<Storm::FinishedInitializeDelegate> _finishedInitCallback;
		Storm::MultiCallback<Storm::WindowsResizedDelegate> _windowsResizedCallback;
		Storm::MultiCallback<Storm::WindowsMovedDelegate> _windowsMovedCallback;

		std::thread _windowsThread;
	};
}
