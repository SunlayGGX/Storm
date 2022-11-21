#pragma once

#include "Singleton.h"
#include "IRandomManager.h"
#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class SerializePackage;

	class RandomManager final :
		private Storm::Singleton<RandomManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IRandomManager
	{
		STORM_DECLARE_SINGLETON(RandomManager);

	public:
		float randomizeFloat() final override;
		float randomizeFloat(float min, float max) final override;
		float randomizeFloat(float max) final override;
		int32_t randomizeInteger(int32_t min, int32_t max) final override;
		int32_t randomizeInteger(int32_t max) final override;
		int64_t randomizeInteger(int64_t min, int64_t max) final override;
		int64_t randomizeInteger(int64_t max) final override;
		Storm::Vector3 randomizeVector3(const Storm::Vector3 &max) final override;

		void shuffle(std::vector<int> &container, std::size_t endIndex) final override;
	};
}
