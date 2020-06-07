#pragma once

#include "Singleton.h"
#include "IThreadManager.h"


namespace Storm
{
	class ThreadManager :
		private Storm::Singleton<ThreadManager>,
		public Storm::IThreadManager
	{
		STORM_DECLARE_SINGLETON(ThreadManager);
	};
}
