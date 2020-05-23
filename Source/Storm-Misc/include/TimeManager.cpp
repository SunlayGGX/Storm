#include "TimeManager.h"
#include "SerializePackage.h"

#include "TimeWaitResult.h"
#include "InvertPeriod.h"


Storm::TimeManager::TimeManager() :
	_physicsTimeInSeconds{ 0.05f },
	_startTime{ std::chrono::high_resolution_clock::now() },
	_isRunning{ false },
	_isPaused{ false }
{

}

Storm::TimeManager::~TimeManager()
{
	this->quit();
	LOG_DEBUG << "Storm application run for " << this->getCurrentSimulationElapsedTime().count() << " ms";
}

void Storm::TimeManager::initialize_Implementation()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_isRunning = true;
}

void Storm::TimeManager::cleanUp_Implementation()
{
	this->quit();
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
	return this->waitImpl(_simulationFrameTime);
}

Storm::TimeWaitResult Storm::TimeManager::waitForTime(std::chrono::milliseconds timeToWait)
{
	return this->waitImpl(std::chrono::duration_cast<std::chrono::microseconds>(timeToWait));
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
	return _physicsTimeInSeconds;
}

void Storm::TimeManager::setCurrentPhysicsDeltaTime(float deltaTimeInSeconds)
{
	_physicsTimeInSeconds = deltaTimeInSeconds;
}

float Storm::TimeManager::getCurrentPhysicsElapsedTime() const
{
	return _physicsElapsedTimeInSeconds;
}

void Storm::TimeManager::increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds)
{
	_physicsElapsedTimeInSeconds += timeIncreaseInSeconds;
}

void Storm::TimeManager::quit()
{
	{
		std::lock_guard<std::mutex> lock{ _mutex };
		_isRunning = false;
	}

	_synchronizer.notify_all();
}

bool Storm::TimeManager::simulationIsPaused() const
{
	std::lock_guard<std::mutex> lock{ _mutex };
	return _isPaused;
}

bool Storm::TimeManager::changeSimulationPauseState()
{
	std::lock_guard<std::mutex> lock{ _mutex };
	_isPaused = !_isPaused;
	return _isPaused;
}

Storm::TimeWaitResult Storm::TimeManager::waitImpl(std::chrono::microseconds timeToWait)
{
	std::unique_lock<std::mutex> lock{ _mutex };
	if (_isRunning)
	{
		_fpsWatcherPerThread[std::this_thread::get_id()].registerCurrent(timeToWait);

		if (!_synchronizer.wait_for(lock, timeToWait, [running = &_isRunning]()
		{
			return !running;
		}))
		{
			return _isPaused ? Storm::TimeWaitResult::Pause : Storm::TimeWaitResult::Continue;
		}
	}

	return Storm::TimeWaitResult::Exit;
}
