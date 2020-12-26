#pragma once

#include "Singleton.h"
#include "IRandomManager.h"
#include "SingletonDefaultImplementation.h"

#include <random>


namespace Storm
{
	class SerializePackage;

	class RandomManager final :
		private Storm::Singleton<RandomManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IRandomManager
	{
		STORM_DECLARE_SINGLETON(RandomManager);

	public:
		void serialize(Storm::SerializePackage &package);

	public:
		float randomizeFloat() final override;
		float randomizeFloat(float min, float max) final override;
		float randomizeFloat(float max) final override;
		int32_t randomizeInteger(int32_t min, int32_t max) final override;
		int32_t randomizeInteger(int32_t max) final override;
		int64_t randomizeInteger(int64_t min, int64_t max) final override;
		int64_t randomizeInteger(int64_t max) final override;

	private:
		uint64_t _seed;
		std::mt19937_64 _randomEngine;
	};
}
