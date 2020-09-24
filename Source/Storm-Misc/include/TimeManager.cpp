#include "TimeManager.h"
#include "SerializePackage.h"

#include "TimeWaitResult.h"
#include "InvertPeriod.h"

#include "SingletonHolder.h"
#include "IWindowsManager.h"
#include "IConfigManager.h"
#include "IInputManager.h"
#include "IThreadManager.h"

#include "ThreadHelper.h"
#include "ThreadEnumeration.h"

#include "SpecialKey.h"

#include "GeneralSimulationData.h"

#include "UIField.h"

#define STORM_ELAPSED_TIME_FIELD_NAME "elapsed time"
#define STORM_DELTA_TIME_FIELD_NAME "delta time"
#define STORM_PAUSE_FIELD_NAME "paused"


Storm::TimeManager::TimeManager() :
	_physicsTimeInSeconds{ 0.01f },
	_startTime{ std::chrono::high_resolution_clock::now() },
	_isRunning{ false },
	_isPaused{ false },
	_shouldLogFPSWatching{ false },
	_physicsElapsedTimeInSeconds{ 0.f }
{
	this->setExpectedFrameFPS(60.f);
}

Storm::TimeManager::~TimeManager()
{
	this->quit();
	LOG_DEBUG << "Storm application run for " << this->getCurrentSimulationElapsedTime().count() << " ms";
}

void Storm::TimeManager::initialize_Implementation()
{
	Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	Storm::IWindowsManager &windowMgr = singletonHolder.getSingleton<Storm::IWindowsManager>();
	Storm::IInputManager &inputMgr = singletonHolder.getSingleton<Storm::IInputManager>();

	std::lock_guard<std::mutex> lock{ _mutex };
	_isRunning = true;

	const Storm::GeneralSimulationData &generalSimulationData = configMgr.getGeneralSimulationData();
	if (generalSimulationData._physicsTimeInSec > 0.f)
	{
		this->_physicsTimeInSeconds = generalSimulationData._physicsTimeInSec;
	}

	_isPaused = generalSimulationData._startPaused;

	const float requestedFps = generalSimulationData._expectedFps;
	if (requestedFps > 0.f)
	{
		this->setExpectedFrameFPS(requestedFps);
	}

	_shouldLogFPSWatching = configMgr.getShouldLogFpsWatching();

	_fields
		.bindField(STORM_ELAPSED_TIME_FIELD_NAME, _physicsElapsedTimeInSeconds)
		.bindField(STORM_DELTA_TIME_FIELD_NAME, this->_physicsTimeInSeconds)
		.bindField(STORM_PAUSE_FIELD_NAME, _isPaused)
		;

	_timeThread = std::thread{ [this]() 
	{
		STORM_REGISTER_THREAD(TimeThread);

		while (this->waitNextFrameOrExit())
		{
			_frameSynchronizer.notify_all();
		}
	} };

	windowMgr.bindQuitCallback([this]()
	{
		this->quit();
	});

	inputMgr.bindKey(Storm::SpecialKey::KC_SPACE, [this]() 
	{
		this->changeSimulationPauseState();
	});
}

void Storm::TimeManager::cleanUp_Implementation()
{
	this->quit();
	Storm::join(_timeThread);
}

void Storm::TimeManager::serialize(Storm::SerializePackage &packet)
{
	packet
		<< _physicsTimeInSeconds
		<< _physicsElapsedTimeInSeconds
		<< _isPaused
		;
}

Storm::TimeWaitResult Storm::TimeManager::waitNextFrame()
{
	return this->waitImpl(_frameSynchronizer, _simulationFrameTime);
}

Storm::TimeWaitResult Storm::TimeManager::waitForTime(std::chrono::milliseconds timeToWait)
{
	return this->waitImpl(_synchronizer, std::chrono::duration_cast<std::chrono::microseconds>(timeToWait));
}

bool Storm::TimeManager::waitNextFrameOrExit()
{
	return this->waitNextFrame() != Storm::TimeWaitResult::Exit;
}

bool Storm::TimeManager::waitForTimeOrExit(std::chrono::milliseconds timeToWait)
{
	return this->waitForTime(timeToWait) != Storm::TimeWaitResult::Exit;
}

Storm::TimeWaitResult Storm::TimeManager::getStateNoSyncWait() const
{
	std::unique_lock<std::mutex> lock{ _mutex };
	if (_isRunning)
	{
		return _isPaused ? Storm::TimeWaitResult::Pause : Storm::TimeWaitResult::Continue;
	}

	return Storm::TimeWaitResult::Exit;
}

void Storm::TimeManager::setExpectedFrameFPS(float fps)
{
	_simulationFrameTime = std::chrono::microseconds{ static_cast<std::chrono::microseconds::rep>(std::roundf(Storm::InvertPeriod<std::chrono::microseconds::period>::value / fps)) };
}

float Storm::TimeManager::getExpectedFrameFPS() const
{
	return Storm::ChronoHelper::toFps(_simulationFrameTime);
}

float Storm::TimeManager::getCurrentFPS(ExpectedFPSTag tag) const
{
	return this->getFPS(std::this_thread::get_id(), tag);
}

float Storm::TimeManager::getCurrentFPS(RealTimeFPSTag tag) const
{
	return this->getFPS(std::this_thread::get_id(), tag);
}

float Storm::TimeManager::getFPS(const std::thread::id &threadId, ExpectedFPSTag) const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _fpsWatcherPerThread[threadId].getExpectedFps();
}

float Storm::TimeManager::getFPS(const std::thread::id &threadId, RealTimeFPSTag) const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _fpsWatcherPerThread[threadId].getFps();
}

std::chrono::milliseconds Storm::TimeManager::getCurrentSimulationElapsedTime() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _startTime);
}

float Storm::TimeManager::getCurrentPhysicsDeltaTime() const
{
	return this->_physicsTimeInSeconds;
}

bool Storm::TimeManager::setCurrentPhysicsDeltaTime(float deltaTimeInSeconds)
{
	if (this->_physicsTimeInSeconds != deltaTimeInSeconds)
	{
		this->_physicsTimeInSeconds = deltaTimeInSeconds;
		_fields.pushField(STORM_DELTA_TIME_FIELD_NAME);
		return true;
	}

	return false;
}

float Storm::TimeManager::getCurrentPhysicsElapsedTime() const
{
	return _physicsElapsedTimeInSeconds;
}

void Storm::TimeManager::increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds)
{
	_physicsElapsedTimeInSeconds += timeIncreaseInSeconds;
	_fields.pushField(STORM_ELAPSED_TIME_FIELD_NAME);
}

void Storm::TimeManager::advanceCurrentPhysicsElapsedTime()
{
	this->increaseCurrentPhysicsElapsedTime(this->_physicsTimeInSeconds);
}

void Storm::TimeManager::quit()
{
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		_isRunning = false;
	}

	_frameSynchronizer.notify_all();
	_synchronizer.notify_all();
}

bool Storm::TimeManager::simulationIsPaused() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _isPaused;
}

bool Storm::TimeManager::isRunning() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _isRunning;
}

bool Storm::TimeManager::changeSimulationPauseState()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_isPaused = !_isPaused;
	_fields.pushField(STORM_PAUSE_FIELD_NAME);
	return _isPaused;
}

Storm::TimeWaitResult Storm::TimeManager::waitImpl(std::condition_variable &usedSynchronizer, std::chrono::microseconds timeToWait)
{
	std::unique_lock<std::mutex> lock{ _mutex };
	if (_isRunning)
	{
		if (_shouldLogFPSWatching)
		{
			Storm::FPSWatcher &currentWatcher = _fpsWatcherPerThread[std::this_thread::get_id()];
			currentWatcher.registerCurrent(timeToWait);

			if (_timeToWatch > 230 || timeToWait > std::chrono::microseconds{ 50000 })
			{
				const float currentRealtimeFps = currentWatcher.getFps();
				const float expectedFps = currentWatcher.getExpectedFps();
				if (currentRealtimeFps < (expectedFps / 5.f))
				{
					LOG_WARNING << "The current thread is below 20% of its expected fps (was " << currentRealtimeFps << " fps but we expected " << expectedFps << ')';
				}

				// I don't care if this is buggy (not as expected. I just don't want to watch every time and flood the logs so if we can skip some watching, it is good enough for me...
				++_timeToWatch;
			}
		}

		if (!usedSynchronizer.wait_for(lock, timeToWait, [running = &_isRunning]()
		{
			return !running;
		}))
		{
			return _isPaused ? Storm::TimeWaitResult::Pause : Storm::TimeWaitResult::Continue;
		}
	}

	return Storm::TimeWaitResult::Exit;
}
