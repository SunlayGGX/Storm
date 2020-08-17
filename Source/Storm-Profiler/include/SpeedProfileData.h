#pragma once


namespace Storm
{
	class SpeedProfileData
	{
	private:
		enum : std::size_t { k_speedBufferCount = 11 };

	public:
		SpeedProfileData();

	public:
		void startTime();
		void stopTime();

		const float& computationSpeed() const;

	private:
		std::chrono::high_resolution_clock::time_point _lastStartTime;

		float* _bufferPtr;
		float _lastSpeedBuffer[Storm::SpeedProfileData::k_speedBufferCount];
		float _currentComputationSpeed;
	};
}
