#pragma once

#include "Singleton.h"
#include "IOSManager.h"
#include "SingletonDefaultImplementation.h"
#include "DeclareScriptableItem.h"


namespace Storm
{
	namespace details
	{
		class ProcessesHolder;
	}

	class StormProcess;

	class OSManager final :
		private Storm::Singleton<OSManager, Storm::DefineDefaultInitImplementationOnly>,
		public Storm::IOSManager
	{
		STORM_DECLARE_SINGLETON(OSManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		void cleanUp_Implementation();

	private:
		void clearProcesses();

	public:
		std::wstring openFileExplorerDialog(const std::wstring &defaultStartingPath, const std::map<std::wstring, std::wstring> &filters) final override;
		unsigned int obtainCurrentPID() const final override;
		std::string getComputerName() const final override;

		std::size_t startProcess(Storm::StormProcessStartup &&startup) final override;
		int queryProcessExitCode(const std::size_t processUID, bool &outReturned, bool &outFailure) const final override;
		int waitForProcessExitCode(const std::size_t processUID, bool &outFailure) final override;

	public:
		void makeBipSound(const std::chrono::milliseconds bipDuration) final override;

	public:
		bool preventShutdown() final override;

	private:
		std::unique_ptr<Storm::details::ProcessesHolder> _processHolder;
	};
}
