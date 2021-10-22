#include "SerializerManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IConfigManager.h"

#include "SceneSimulationConfig.h"

#include "SerializeRecordContraintsData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordPendingData.h"

#include "SerializeConstraintLayout.h"
#include "SerializeParticleSystemLayout.h"
#include "SerializeRecordHeader.h"

#include "RecordWriter.h"
#include "RecordReader.h"

#include "RecordArchiver.h"

#include "ThreadingSafety.h"
#include "ThreadHelper.h"
#include "ThreadFlaggerObject.h"
#include "ThreadEnumeration.h"

#include "FuncMovePass.h"

#include "InvertPeriod.h"

#include "StateSavingOrders.h"
#include "StateWriter.h"
#include "StateReader.h"

#include "StormExiter.h"


namespace
{
	std::chrono::milliseconds computeSerializerThreadRefreshDuration()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

		const float expectedFps = configMgr.getSceneSimulationConfig()._expectedFps;
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

	template<class Func>
	void executeOnSerializerThread(Func &&func)
	{
		Storm::IThreadManager &threadMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>();
		threadMgr.executeOnThread(Storm::ThreadEnumeration::SerializerThread, std::forward<Func>(func));
	}
}


Storm::SerializerManager::SerializerManager() = default;
Storm::SerializerManager::~SerializerManager() = default;

void Storm::SerializerManager::initialize_Implementation()
{
	LOG_COMMENT << "Starting Serializer module initialization";

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (configMgr.shouldArchive())
	{
		LOG_COMMENT << "Creating Archiver.";

		_archiver = std::make_unique<Storm::RecordArchiver>();

		std::string archivePathCpy = _archiver->_archivePath;
		{
			std::lock_guard<std::mutex> lock{ _mutex };
			std::swap(archivePathCpy, _archivePathCachedNoCleanUp);
		}

		_archiver->preArchive();
	}

	_serializeThread = std::thread{ [this]()
	{
		STORM_REGISTER_THREAD(SerializerThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::SerializingThread;
		this->run();
	} };

	LOG_COMMENT << "Serializer module initialization finished";
}

void Storm::SerializerManager::cleanUp_Implementation()
{
	LOG_COMMENT << "Serializer module Cleanup";
	Storm::join(_serializeThread);

	STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::SerializingThread;
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().processActionsOfThread(Storm::ThreadEnumeration::SerializerThread);
	this->execute();

	this->endRecordInternal();

	if (_archiver)
	{
		_archiver->execute();
		_archiver.reset();
	}
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
	catch (const Storm::Exception &e)
	{
		LOG_FATAL <<
			"Serializer thread unexpected exit (with Storm::Exception) : " << e.what() << ".\n" << e.stackTrace();
		normalExit = false;
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
		Storm::requestExitOtherThread();
		this->clearRecordQueue();
	}
}

void Storm::SerializerManager::execute()
{
	assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");

	if (!_pendingRecord.empty())
	{
		if (_recordWriter) STORM_LIKELY
		{
			this->processRecordQueue_Unchecked();
		}
		else
		{
			Storm::throwException<Storm::Exception>("Cannot process recording if header isn't set (we haven't called beginRecord before)! Aborting!");
		}
	}

	if (_stateSavingRequestOrders)
	{
		Storm::StateWriter::execute(*_stateSavingRequestOrders);
		_stateSavingRequestOrders.reset();
	}
}

void Storm::SerializerManager::clearRecordQueue()
{
	assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");
	_pendingRecord = decltype(_pendingRecord){};
}

void Storm::SerializerManager::processRecordQueue_Unchecked()
{
	assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");
	do
	{
		_recordWriter->write(*_pendingRecord.front());
		_pendingRecord.pop();
	} while(!_pendingRecord.empty());

	_recordWriter->flush();
}

void Storm::SerializerManager::recordFrame(Storm::SerializeRecordPendingData &&frameRecord)
{
	executeOnSerializerThread([this, rec = Storm::FuncMovePass<Storm::SerializeRecordPendingData>{ std::move(frameRecord) }]() mutable
	{
		assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");

		_pendingRecord.emplace(std::make_unique<Storm::SerializeRecordPendingData>(std::move(rec._object)));
	});
}

void Storm::SerializerManager::beginRecord(Storm::SerializeRecordHeader &&recordHeader)
{
	executeOnSerializerThread([this, rec = Storm::FuncMovePass<Storm::SerializeRecordHeader>{ std::move(recordHeader) }]() mutable
	{
		assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");

		std::lock_guard<std::mutex> lock{ _mutex };

		if (!_recordReader)
		{
			if (!_recordWriter)
			{
				_recordWriter = std::make_unique<Storm::RecordWriter>(std::move(rec._object));
				LOG_COMMENT << "Recording started";
			}
			else
			{
				Storm::throwException<Storm::Exception>("We are already recording. Stop the current recording before starting another one!");
			}
		}
		else
		{
			Storm::throwException<Storm::Exception>("Cannot record and replay at the same time!");
		}
	});
}

void Storm::SerializerManager::endRecordInternal()
{
	assert(Storm::isSerializerThread() && "This method should only be executed inside the serializer thread!");

	if (_recordWriter)
	{
		const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();

		assert(!timeMgr.isRunning() && "This method should be called after the simulation ended!");

		_recordWriter->endWrite(timeMgr.getCurrentPhysicsElapsedTime());
		LOG_COMMENT << "Recording ended";

		_recordWriter.reset();
	}
}

const Storm::SerializeRecordHeader& Storm::SerializerManager::beginReplay()
{
	assert(Storm::isSimulationThread() && "this method should only be called from simulation thread.");

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
			Storm::throwException<Storm::Exception>("We are already reading!");
		}
	}
	else
	{
		Storm::throwException<Storm::Exception>("Cannot record and replay at the same time!");
	}
}

bool Storm::SerializerManager::obtainNextFrame(Storm::SerializeRecordPendingData &outPendingData) const
{
	assert(Storm::isSimulationThread() && "this method should only be called from simulation thread.");
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
		Storm::throwException<Storm::Exception>("We cannot query a record header except in a recording mode (either record or replay)!");
	}
}

std::shared_ptr<Storm::SerializeSupportedFeatureLayout> Storm::SerializerManager::getRecordSupportedFeature() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	if (_recordReader)
	{
		return _recordReader->getHeader()._supportedFeaturesLayout;
	}
	else
	{
		Storm::throwException<Storm::Exception>("We cannot query SupportedFeature except in a replay mode!");
	}
}

bool Storm::SerializerManager::resetReplay()
{
	assert(Storm::isSimulationThread() && "this method should only be called from simulation thread.");
	if (_recordReader)
	{
		return _recordReader->resetToBeginning();
	}
	else
	{
		Storm::throwException<Storm::Exception>("We aren't replaying, therefore we cannot reset it!");
	}
}

void Storm::SerializerManager::saveState(Storm::StateSavingOrders &&savingOrder)
{
	executeOnSerializerThread([this, savingOrderFwd = Storm::FuncMovePass<Storm::StateSavingOrders>{ std::move(savingOrder) }]() mutable
	{
		assert(_stateSavingRequestOrders == nullptr && "A state saving request is already pending!");
		_stateSavingRequestOrders = std::make_unique<Storm::StateSavingOrders>(std::move(savingOrderFwd._object));
	});
}

void Storm::SerializerManager::loadState(Storm::StateLoadingOrders &inOutLoadingOrder)
{
	assert(Storm::isSimulationThread() && "this method should only be called from simulation thread.");

	Storm::StateReader::execute(inOutLoadingOrder);
}

std::string Storm::SerializerManager::getArchivePath() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _archivePathCachedNoCleanUp;
}
