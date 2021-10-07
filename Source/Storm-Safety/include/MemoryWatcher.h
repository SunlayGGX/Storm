#pragma once



namespace Storm
{
	struct GeneralSafetyConfig;

	class MemoryWatcher
	{
	private:
		enum class CloseRequestLevel
		{
			None,
			Closed,
			Exit
		};

	public:
		MemoryWatcher(const Storm::GeneralSafetyConfig &safetyConfig);

	public:
		void execute();

	private:
		std::size_t _startConsumedMemory;
		const double _alertCoeffThreshold;
		const double _endCoeffThreshold;
		Storm::MemoryWatcher::CloseRequestLevel _closeRequested;
		bool _alertLogged;
		std::chrono::high_resolution_clock::time_point _closeTimeRequest;
	};
}
