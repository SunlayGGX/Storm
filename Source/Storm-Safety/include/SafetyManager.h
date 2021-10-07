#pragma once

#include "ISafetyManager.h"
#include "Singleton.h"


namespace Storm
{
	class FreezeWatcher;
	class MemoryWatcher;

	class SafetyManager final :
		private Storm::Singleton<Storm::SafetyManager>,
		public Storm::ISafetyManager
	{
		STORM_DECLARE_SINGLETON(SafetyManager);

	public:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void run();

		void execute();

	public:
		void notifySimulationThreadAlive() final override;

	private:
		std::unique_ptr<Storm::FreezeWatcher> _freezeWatcher;
		std::unique_ptr<Storm::MemoryWatcher> _memoryWatcher;

		bool _isRunning;
		std::mutex _cvMutex;
		std::condition_variable _cv;
		std::thread _safetyThread;
	};
}
