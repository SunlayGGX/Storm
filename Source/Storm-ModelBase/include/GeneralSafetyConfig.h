#pragma once


namespace Storm
{
	struct GeneralSafetyConfig
	{
	public:
		GeneralSafetyConfig();

	public:
		// Memory watching
		double _memoryThreshold;
		bool _enableMemoryWatcher;

		// Freeze watching
		std::chrono::seconds _freezeRefresh;
	};
}
