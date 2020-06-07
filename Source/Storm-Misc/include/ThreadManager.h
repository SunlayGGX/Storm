#pragma once

#include "Singleton.h"
#include "IThreadManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class ThreadManager :
		private Storm::Singleton<ThreadManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IThreadManager
	{
		STORM_DECLARE_SINGLETON(ThreadManager);

	public:
		void nameCurrentThread(const std::wstring &newName) final override;
	};
}
