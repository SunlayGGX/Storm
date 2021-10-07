#pragma once

#include "ISafetyManager.h"
#include "Singleton.h"


namespace Storm
{
	class SafetyManager final :
		private Storm::Singleton<Storm::SafetyManager>,
		public Storm::ISafetyManager
	{
		STORM_DECLARE_SINGLETON(SafetyManager);

	public:

	};
}
