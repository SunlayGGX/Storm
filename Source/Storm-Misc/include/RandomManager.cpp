#include "RandomManager.h"
#include "SerializePackage.h"


namespace
{
	struct Randomizer
	{
	public:
		template<class RandomEngineType, class Type>
		static Type computeRandom(RandomEngineType &randomEngine, Type min, Type max)
		{
			using RandomDistribution = std::conditional_t<std::is_floating_point_v<Type>, std::uniform_real_distribution<Type>, std::uniform_int_distribution<Type>>;
			return RandomDistribution{ min, max }(randomEngine);
		}
	};
}


Storm::RandomManager::RandomManager() :
	_seed{ static_cast<uint64_t>(time(nullptr)) }
{
	_randomEngine.seed(_seed);
}

Storm::RandomManager::~RandomManager() = default;

void Storm::RandomManager::serialize(Storm::SerializePackage &package)
{
	package << _seed;
	if (!package.isSerializing())
	{
		_randomEngine.seed(_seed);
	}
}

float Storm::RandomManager::randomizeFloat()
{
	return Randomizer::computeRandom(_randomEngine, 0.f, 1.f);
}

float Storm::RandomManager::randomizeFloat(float max)
{
	return Randomizer::computeRandom(_randomEngine, static_cast<decltype(max)>(0), max);
}

float Storm::RandomManager::randomizeFloat(float min, float max)
{
	return Randomizer::computeRandom(_randomEngine, min, max);
}

int32_t Storm::RandomManager::randomizeInteger(int32_t max)
{
	return Randomizer::computeRandom(_randomEngine, static_cast<decltype(max)>(0), max);
}

int64_t Storm::RandomManager::randomizeInteger(int64_t max)
{
	return Randomizer::computeRandom(_randomEngine, static_cast<decltype(max)>(0), max);
}

int64_t Storm::RandomManager::randomizeInteger(int64_t min, int64_t max)
{
	return Randomizer::computeRandom(_randomEngine, min, max);
}

int32_t Storm::RandomManager::randomizeInteger(int32_t min, int32_t max)
{
	return Randomizer::computeRandom(_randomEngine, min, max);
}
