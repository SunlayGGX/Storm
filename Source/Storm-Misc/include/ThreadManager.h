#pragma once

#include "Singleton.h"
#include "IThreadManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class AsyncActionExecutor;

	class ThreadManager :
		private Storm::Singleton<ThreadManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IThreadManager
	{
		STORM_DECLARE_SINGLETON(ThreadManager);

	public:
		void registerCurrentThread(Storm::ThreadEnumeration threadEnum, const std::wstring &newName) final override;
		void executeOnThread(const std::thread::id &threadId, Storm::AsyncAction &&action) final override;
		void executeOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action) final override;
		void executeDefferedOnThread(Storm::ThreadEnumeration threadEnum, Storm::AsyncAction &&action) final override;
		void processCurrentThreadActions() final override;
		void processActionsOfThread(Storm::ThreadEnumeration threadEnum) final override;
		bool isExecutingOnThread(Storm::ThreadEnumeration threadEnum) const final override;

		void setCurrentThreadPriority(const Storm::ThreadPriority priority) const final override;

	private:
		void executeOnThreadInternal(const std::thread::id &threadId, Storm::AsyncAction &&action);

	private:
		mutable std::recursive_mutex _mutex;
		std::map<std::thread::id, std::unique_ptr<Storm::AsyncActionExecutor>> _toExecute;

		std::map<std::thread::id, Storm::ThreadEnumeration> _threadEnumMapping;
		std::map<Storm::ThreadEnumeration, std::thread::id> _threadIdMapping;

		std::map<Storm::ThreadEnumeration, std::vector<Storm::AsyncAction>> _pendingThreadsRegisteringActions;
	};
}
