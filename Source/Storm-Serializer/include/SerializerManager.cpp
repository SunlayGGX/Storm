#include "SerializerManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"

#include "ThreadEnumeration.h"

#include "ThreadHelper.h"


Storm::SerializerManager::SerializerManager() = default;
Storm::SerializerManager::~SerializerManager() = default;

void Storm::SerializerManager::initialize_Implementation()
{
	LOG_COMMENT << "Starting Serializer module initialization";

	_serializeThread = std::thread{ [this]()
	{
		STORM_REGISTER_THREAD(SerializerThread);
		this->run();
	} };

	LOG_COMMENT << "Serializer module initialization finished";
}

void Storm::SerializerManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Serializer module Cleanup";
	Storm::join(_serializeThread);
}

void Storm::SerializerManager::run()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

	while (timeMgr.waitNextFrameOrExit())
	{
		threadMgr.processCurrentThreadActions();
	}
}
