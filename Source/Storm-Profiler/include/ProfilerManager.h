#pragma once

#include "Singleton.h"
#include "IProfilerManager.h"


namespace Storm
{
	class ProfilerManager :
		private Storm::Singleton<Storm::ProfilerManager>,
		public Storm::IProfilerManager
	{
		STORM_DECLARE_SINGLETON(ProfilerManager);
	};
}
