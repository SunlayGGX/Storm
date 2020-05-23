#pragma once

#include "Singleton.h"
#include "ITimeManager.h"


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
		void setCurrentFPS(float fps) const final override;
		float getCurrentFPS() const final override;

		std::chrono::milliseconds getCurrentSimulationElapsedTime() const final override;
		float getCurrentPhysicsDeltaTime() const override;
		void setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) override;
		float getCurrentPhysicsElapsedTime() const override;

		void quit() final override;

	private:
		mutable std::mutex _mutex;
		std::condition_variable _synchronizer;
		bool _isRunning;

		float _physicsTimeInSeconds;
		float _physicsElapsedTimeInSeconds;
		std::thread::id _simulationThreadId;

		std::chrono::microseconds _simulationFrameTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _currentTime;

		std::thread _thread;
	};
}
