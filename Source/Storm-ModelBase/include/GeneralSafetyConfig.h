#pragma once


namespace Storm
{
	struct GeneralSafetyConfig
	{
	public:
		GeneralSafetyConfig();

	public:
		// Memory watching
		float _memoryThreshold;

		// Freeze watching
		std::chrono::seconds _freezeRefresh;
	};
}
