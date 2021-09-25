#pragma once


namespace Storm
{
	class FPSWatcher
	{
	public:
		constexpr FPSWatcher() = default;

		void registerCurrent(const std::chrono::microseconds expectedRefreshTime);
		float getFps() const;

		float getExpectedFps() const;

	private:
		std::chrono::high_resolution_clock::time_point _timeBuffer[5];
		float _cachedFPS = 0.f;
		std::chrono::microseconds _expectedRefreshTime{ 0 };
	};
}
