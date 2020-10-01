#include "SerializerManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IConfigManager.h"
#include "ISimulatorManager.h"

#include "GeneralSimulationData.h"

#include "ThreadEnumeration.h"

#include "ExitCode.h"

#include "SerializeRecordPendingData.h"
#include "SerializeRecordHeader.h"

#include "ThreadHelper.h"
#include "InvertPeriod.h"


namespace
{
	std::chrono::milliseconds computeSerializerThreadRefreshDuration()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

		const float expectedFps = configMgr.getGeneralSimulationData()._expectedFps;
		if (expectedFps > 0.f)
		{
			// I want Serializer thread to run 5 time slower than the expected fps
			// (I don't want to set a fixed time because I don't want the serializing queue to grow too big, 
			// but I don't want it to compute too much because it takes CPU resources, context switching and needlessly locking the thread manager for nothing). 
			return Storm::ChronoHelper::toFrameDuration<std::chrono::milliseconds>(expectedFps / 5.f);
		}
		else
		{
			return std::chrono::milliseconds{ 100 };
		}
	}
}


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
	bool normalExit;

	try
	{
		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();

		const std::chrono::milliseconds serializerRefreshTime = computeSerializerThreadRefreshDuration();

		const auto serializingIterationExecutorLambda = [this, &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>()]()
		{
			// processing the current thread action (which is the serializer actions) will fetch the data to serialize.
			threadMgr.processCurrentThreadActions();

			// Execute the serializing actions on the data we just fetched.
			this->execute();
		};

		while (timeMgr.waitForTimeOrExit(serializerRefreshTime))
		{
			serializingIterationExecutorLambda();
		}

		// One last time to ensure there is no more data in the queue to be serialized that was pushed when we waited.
		serializingIterationExecutorLambda();

		normalExit = true;
	}
	catch (const std::exception &e)
	{
		LOG_FATAL << "Serializer thread unexpected exit (with std::exception) : " << e.what();
		normalExit = false;
	}
	catch (...)
	{
		LOG_FATAL << "Serializer thread unexpected exit (with unknown exception)";
		normalExit = false;
	}

	if (!normalExit)
	{
		singletonHolder.getSingleton<Storm::ISimulatorManager>().exitWithCode(Storm::ExitCode::k_otherThreadTermination);
		this->clearRecordQueue();
	}
}

void Storm::SerializerManager::execute()
{
	if (!_pendingRecord.empty())
	{
		if (_recordHeader)
		{
			this->processRecordQueue_Unchecked();
		}
		else
		{
			Storm::throwException<std::exception>("Cannot process recording if header isn't set (we haven't called beginRecord before)! Aborting!");
		}
	}
}

void Storm::SerializerManager::clearRecordQueue()
{
	_pendingRecord = decltype(_pendingRecord){};
}

void Storm::SerializerManager::processRecordQueue_Unchecked()
{
	do
	{
		this->processRecord(*_pendingRecord.front());
		_pendingRecord.pop();
	} while(!_pendingRecord.empty());
}

void Storm::SerializerManager::processRecord(const Storm::SerializeRecordPendingData &record)
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::SerializerManager::recordFrame(Storm::SerializeRecordPendingData &&frameRecord)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SerializerThread, [this, rec = std::move(frameRecord)]()
	{
		_pendingRecord.emplace(std::make_unique<Storm::SerializeRecordPendingData>(std::move(rec)));
	});
}

void Storm::SerializerManager::beginRecord(Storm::SerializeRecordHeader &&recordHeader)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SerializerThread, [this, rec = std::move(recordHeader)]()
	{
		_recordHeader = std::make_unique<Storm::SerializeRecordHeader>(std::move(rec));
	});
}
