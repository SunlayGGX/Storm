#include "TypeIdGenerator.h"


Storm::TypeIdGenerator::TypeIdGenerator() :
	_mutex{ nullptr },
	_capacity{ 64 },
	_registeredKeysType{ nullptr },
	_ids{ nullptr },
	_idsGen{ 0 }
{
	auto mutTmp = std::make_unique<std::mutex>();
	auto keyTmp = std::make_unique<Storm::TypeIdGenerator::KeyType[]>(_capacity);
	auto idsTmp = std::make_unique<unsigned int[]>(_capacity);

	_mutex = mutTmp.release();
	_registeredKeysType = keyTmp.release();
	_ids = idsTmp.release();
}

Storm::TypeIdGenerator::~TypeIdGenerator()
{
	delete[] _ids;
	delete[] _registeredKeysType;
	delete _mutex;
}

unsigned int Storm::TypeIdGenerator::produceID(const std::string_view func)
{
	std::lock_guard lock{ *_mutex };
	if (_idsGen == _capacity)
	{
		std::size_t newCapacity = _capacity * 2;
		auto keyTmp = std::make_unique<Storm::TypeIdGenerator::KeyType[]>(newCapacity);
		auto idsTmp = std::make_unique<unsigned int[]>(newCapacity);

		std::copy(std::make_move_iterator(_registeredKeysType), std::make_move_iterator(_registeredKeysType + _capacity), keyTmp.get());
		std::copy(_ids, _ids + _capacity, idsTmp.get());

		delete[] _ids;
		delete[] _registeredKeysType;

		_registeredKeysType = keyTmp.release();
		_ids = idsTmp.release();
		_capacity = newCapacity;
	}

	const auto endKeys = _registeredKeysType + _idsGen;
	const auto found = std::find(_registeredKeysType, endKeys, func);
	const auto index = found - _registeredKeysType;

	if (found == endKeys)
	{
		_ids[index] = _idsGen++;
		_registeredKeysType[index] = func;
	}

	return _ids[index];
}
