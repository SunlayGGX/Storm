#pragma once

#include "Singleton.h"
#include "ITimeManager.h"
#include "DeclareScriptableItem.h"

#include "FPSWatcher.h"

#include "UIFieldContainer.h"


namespace Storm
{
	class SerializePackage;

	class TimeManager final :
		private Storm::Singleton<TimeManager>,
		public Storm::ITimeManager
	{
		STORM_DECLARE_SINGLETON(TimeManager);
		STORM_IS_SCRIPTABLE_ITEM;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void serialize(Storm::SerializePackage &packet);

	public:
		Storm::TimeWaitResult waitNextFrame() final override;
		Storm::TimeWaitResult waitForTime(std::chrono::microseconds timeToWait) final override;
		Storm::TimeWaitResult waitForTime(std::chrono::milliseconds timeToWait) final override;
		bool waitNextFrameOrExit() final override;
		bool waitForTimeOrExit(std::chrono::milliseconds timeToWait) final override;
		Storm::TimeWaitResult getStateNoSyncWait() const final override;

		void setExpectedFrameFPS(float fps) final override;
		float getExpectedFrameFPS() const final override;
		float getCurrentFPS(ExpectedFPSTag) const final override;
		float getCurrentFPS(RealTimeFPSTag) const final override;
		float getFPS(const std::thread::id &threadId, ExpectedFPSTag) const final override;
		float getFPS(const std::thread::id &threadId, RealTimeFPSTag) const final override;

		std::chrono::milliseconds getCurrentSimulationElapsedTime() const final override;
		float getCurrentPhysicsDeltaTime() const override;
		bool setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) final override;
		float getCurrentPhysicsElapsedTime() const override;
		void setCurrentPhysicsElapsedTime(float physicsElapsedTimeInSeconds) final override;
		void increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds) final override;
		float advanceCurrentPhysicsElapsedTime() final override;
		void resetPhysicsElapsedTime() final override;

		void increaseCurrentPhysicsDeltaTime() final override;
		void decreaseCurrentPhysicsDeltaTime() final override;
		void increasePhysicsDeltaTimeStepSize() final override;
		void decreasePhysicsDeltaTimeStepSize() final override;
		void setPhysicsDeltaTimeStepSize(float newValue);

		bool simulationIsPaused() const final override;
		bool isRunning() const final override;
		bool changeSimulationPauseState() final override;
		void quit() final override;

	private:
		Storm::TimeWaitResult waitImpl(std::condition_variable &usedSynchronizer, std::chrono::microseconds timeToWait);

	private:
		mutable std::mutex _mutex;
		std::condition_variable _frameSynchronizer;
		std::condition_variable _synchronizer;
		bool _isRunning;
		bool _isPaused;

		bool _shouldLogFPSWatching;
		mutable std::map<std::thread::id, Storm::FPSWatcher> _fpsWatcherPerThread;
		unsigned char _timeToWatch;

		float _physicsDeltaTimeCoeff;

		std::atomic<float> _physicsTimeInSeconds;
		std::atomic<float> _physicsElapsedTimeInSeconds;

		std::chrono::microseconds _simulationFrameTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;

		Storm::UIFieldContainer _fields;

		std::thread _timeThread;
	};
}
