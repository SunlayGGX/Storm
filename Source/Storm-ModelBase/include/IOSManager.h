#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IOSManager : public Storm::ISingletonHeldInterface<IOSManager>
	{
	public:
		virtual ~IOSManager() = default;

	public:
		virtual std::wstring openFileExplorerDialog(const std::wstring &explorerWindowsTitle, const std::map<std::wstring, std::wstring> &filters) = 0;
	};
}
