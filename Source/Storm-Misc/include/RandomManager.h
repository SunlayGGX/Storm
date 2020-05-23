#pragma once

#include "Singleton.h"
#include "IRandomManager.h"


namespace Storm
{
	class RandomManager :
		private Storm::Singleton<RandomManager>,
		public Storm::IRandomManager
	{
		STORM_DECLARE_SINGLETON(RandomManager);
	};
}
