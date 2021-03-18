#pragma once


namespace Storm
{
	struct GeneralSimulationConfig
	{
	public:
		GeneralSimulationConfig();

	public:
		bool _allowNoFluid;

		int64_t _stateRefreshFrameCount;
	};
}
