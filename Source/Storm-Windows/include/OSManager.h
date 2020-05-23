#pragma once

#include "Singleton.h"
#include "IOSManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class OSManager :
		private Storm::Singleton<OSManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IOSManager
	{
		STORM_DECLARE_SINGLETON(OSManager);

	public:
		std::wstring openFileExplorerDialog(const std::wstring &defaultStartingPath, const std::map<std::wstring, std::wstring> &filters) final override;
	};
}
