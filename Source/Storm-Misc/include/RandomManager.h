#pragma once

#include "Singleton.h"
#include "IRandomManager.h"
#include "SingletonDefaultImplementation.h"

#include <random>


namespace Storm
{
	class SerializePackage;

	class RandomManager :
		private Storm::Singleton<RandomManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IRandomManager
	{
		STORM_DECLARE_SINGLETON(RandomManager);

	public:
		void serialize(Storm::SerializePackage &package);

	private:
		uint64_t _seed;
		std::mt19937_64 _randomEngine;
	};
}
