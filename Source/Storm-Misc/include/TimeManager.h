#pragma once

#include "Singleton.h"
#include "ITimeManager.h"
#include "FPSWatcher.h"


namespace Storm
{
	class SerializePackage;

	class TimeManager :
		private Storm::Singleton<TimeManager>,
		public Storm::ITimeManager
	{
		STORM_DECLARE_SINGLETON(TimeManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void serialize(Storm::SerializePackage &packet);

	public:
		Storm::TimeWaitResult waitNextFrame() final override;
		Storm::TimeWaitResult waitForTime(std::chrono::milliseconds timeToWait) final override;
		void setExpectedFrameFPS(float fps) final override;
		float getExpectedFrameFPS() const final override;
		float getCurrentFPS(ExpectedFPSTag) const final override;
		float getCurrentFPS(RealTimeFPSTag) const final override;
		float getFPS(const std::thread::id &threadId, ExpectedFPSTag) const final override;
		float getFPS(const std::thread::id &threadId, RealTimeFPSTag) const final override;

		std::chrono::milliseconds getCurrentSimulationElapsedTime() const final override;
		float getCurrentPhysicsDeltaTime() const override;
		void setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) override;
		float getCurrentPhysicsElapsedTime() const override;
		void increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds) final override;

		bool simulationIsPaused() const final override;
		bool changeSimulationPauseState() final override;
		void quit() final override;

	private:
		Storm::TimeWaitResult waitImpl(std::chrono::microseconds timeToWait);

	private:
		mutable std::mutex _mutex;
		std::condition_variable _synchronizer;
		bool _isRunning;
		bool _isPaused;

		mutable std::map<std::thread::id, Storm::FPSWatcher> _fpsWatcherPerThread;

		std::atomic<float> _physicsTimeInSeconds;
		std::atomic<float> _physicsElapsedTimeInSeconds;

		std::chrono::microseconds _simulationFrameTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
	};
}