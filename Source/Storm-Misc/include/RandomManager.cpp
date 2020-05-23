#include "RandomManager.h"
#include "SerializePackage.h"


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
