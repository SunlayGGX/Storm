#pragma once


namespace Storm
{
	struct SimulationState;

	struct StateLoadingOrders
	{
	public:
		struct LoadingSettings
		{
		public:
			std::string _filePath;

			uint8_t _loadPhysicsTime : 1;
			uint8_t _loadVelocities : 1;
			uint8_t _loadForces : 1;
		};

	public:
		~StateLoadingOrders();

	public:
		Storm::StateLoadingOrders::LoadingSettings _settings;
		std::unique_ptr<Storm::SimulationState> _simulationState;
	};
}
