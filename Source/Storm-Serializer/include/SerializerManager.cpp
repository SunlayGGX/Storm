#include "SerializerManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IConfigManager.h"
#include "ISimulatorManager.h"

#include "GeneralSimulationData.h"

#include "ThreadEnumeration.h"

#include "ExitCode.h"

#include "SerializeRecordContraintsData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"
#include "SerializeRecordHeader.h"

#include "RecordWriter.h"
#include "RecordReader.h"

#include "ThreadingSafety.h"
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

	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().processActionsOfThread(Storm::ThreadEnumeration::SerializerThread);
	this->execute();
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
		if (_recordWriter)
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
		_recordWriter->write(*_pendingRecord.front());
		_pendingRecord.pop();
	} while(!_pendingRecord.empty());

	_recordWriter->flush();
}

void Storm::SerializerManager::recordFrame(Storm::SerializeRecordPendingData &&frameRecord)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SerializerThread, [this, rec = std::move(frameRecord)]() mutable
	{
		_pendingRecord.emplace(std::make_unique<Storm::SerializeRecordPendingData>(std::move(rec)));
	});
}

void Storm::SerializerManager::beginRecord(Storm::SerializeRecordHeader &&recordHeader)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SerializerThread, [this, rec = std::move(recordHeader)]() mutable
	{
		std::lock_guard<std::mutex> lock{ _mutex };

		if (!_recordReader)
		{
			if (!_recordWriter)
			{
				_recordWriter = std::make_unique<Storm::RecordWriter>(std::move(rec));
				LOG_COMMENT << "Recording started";
			}
			else
			{
				Storm::throwException<std::exception>("We are already recording. Stop the current recording before starting another one!");
			}
		}
		else
		{
			Storm::throwException<std::exception>("Cannot record and replay at the same time!");
		}
	});
}

void Storm::SerializerManager::endRecord()
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SerializerThread, [this]() mutable
	{
		if (_recordWriter)
		{
			_recordWriter->endWrite();
			LOG_COMMENT << "Recording ended";
		}
	});
}

const Storm::SerializeRecordHeader& Storm::SerializerManager::beginReplay()
{
	assert(isSimulationThread() && "this method should only be called from simulation thread.");

	std::lock_guard<std::mutex> lock{ _mutex };
	if (!_recordWriter)
	{
		if (!_recordReader)
		{
			_recordReader = std::make_unique<Storm::RecordReader>();
			return _recordReader->getHeader();
		}
		else
		{
			Storm::throwException<std::exception>("We are already reading!");
		}
	}
	else
	{
		Storm::throwException<std::exception>("Cannot record and replay at the same time!");
	}
}

bool Storm::SerializerManager::obtainNextFrame(Storm::SerializeRecordPendingData &outPendingData) const
{
	assert(isSimulationThread() && "this method should only be called from simulation thread.");
	return _recordReader->readNextFrame(outPendingData);
}

const Storm::SerializeRecordHeader& Storm::SerializerManager::getRecordHeader() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	if (_recordReader)
	{
		return _recordReader->getHeader();
	}
	else if (_recordWriter)
	{
		return _recordWriter->getHeader();
	}
	else
	{
		Storm::throwException<std::exception>("We cannot query a record header except in a recording mode (either record or replay)!");
	}
}
