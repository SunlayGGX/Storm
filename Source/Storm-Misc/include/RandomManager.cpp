#include "RandomManager.h"


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

	std::mt19937_64& retrieveRandomEngine()
	{
		// The reason I don't initialize it into a known seed (and not making seed serialization and deserialization) is because it is truly useless.
		// It is impossible to control the random to make it predictable because in case of a thread loop dispatch (like with openmp, or Concurrency), we cannot control
		// how many data item count will be provided to the same engine, and how many threads are free to process those data, therefore we cannot predict how many increment the engine will execute at the end.
		// I.e : We have 6000 items to dispatch :
		// - run 1 => 2500 items will query random inside thread 1 (thread 1 engine position would be set to x + 2500), and 3500 items will query random inside thread 2 (thread 2 engine position would be set to x + 3500).
		// - run 2 => 1700 items will query random inside thread 1 (thread 1 engine position would be set to x + 1700), and 2300 items will query random inside thread 2 (thread 2 engine position would be set to x + 2300), and 2000 items will query random inside thread 3 (thread 3 engine position would be set to x + 2000).
		// 
		// The result is really different on the 2 runs, even though "x" is the same for both run...
		// 
		// So, even if they are initialized with the same seed, we cannot tell what number will be provided next since we don't know where the engine is from a previous random fetch count...

		thread_local std::mt19937_64 randomEngine{ static_cast<uint64_t>(time(nullptr)) };
		return randomEngine;
	}
}


Storm::RandomManager::RandomManager() = default;
Storm::RandomManager::~RandomManager() = default;

float Storm::RandomManager::randomizeFloat()
{
	return Randomizer::computeRandom(retrieveRandomEngine(), 0.f, 1.f);
}

float Storm::RandomManager::randomizeFloat(float max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), static_cast<decltype(max)>(0), max);
}

float Storm::RandomManager::randomizeFloat(float min, float max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), min, max);
}

int32_t Storm::RandomManager::randomizeInteger(int32_t max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), static_cast<decltype(max)>(0), max);
}

void Storm::RandomManager::shuffle(std::vector<int> &container, std::size_t endIndex)
{
	std::shuffle(std::begin(container), std::begin(container) + endIndex, retrieveRandomEngine());
}

int64_t Storm::RandomManager::randomizeInteger(int64_t max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), static_cast<decltype(max)>(0), max);
}

int64_t Storm::RandomManager::randomizeInteger(int64_t min, int64_t max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), min, max);
}

int32_t Storm::RandomManager::randomizeInteger(int32_t min, int32_t max)
{
	return Randomizer::computeRandom(retrieveRandomEngine(), min, max);
}
