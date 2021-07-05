#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct StormProcessStartup;

	class IOSManager : public Storm::ISingletonHeldInterface<IOSManager>
	{
	public:
		virtual ~IOSManager() = default;

	public:
		virtual std::wstring openFileExplorerDialog(const std::wstring &explorerWindowsTitle, const std::map<std::wstring, std::wstring> &filters) = 0;
		virtual unsigned int obtainCurrentPID() const = 0;

		virtual std::size_t startProcess(Storm::StormProcessStartup &&startup) = 0;
		virtual int queryProcessExitCode(const std::size_t processUID, bool &outReturned, bool &outFailure) const = 0;
		virtual int waitForProcessExitCode(const std::size_t processUID, bool &outFailure) = 0;

	public:
		virtual void makeBipSound(const std::chrono::milliseconds bipDuration) = 0;

	public:
		virtual bool preventShutdown() = 0;
	};
}
