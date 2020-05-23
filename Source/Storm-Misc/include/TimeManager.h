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
		void serialize(SerializePackage &packet);

	private:
		mutable std::mutex _mutex;
		std::condition_variable _synchronizer;
		bool _isRunning;

		float _physicsTimeInSeconds;

		std::chrono::microseconds _simultationFrameTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> _currentTime;

		std::thread _thread;
	};
}
