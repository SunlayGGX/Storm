#pragma once


namespace Storm
{
	struct SimulationState;

	struct StateSavingOrders
	{
	public:
		struct SavingSettings
		{
		public:
			std::filesystem::path _filePath;
			uint8_t _overwrite : 1;
			uint8_t _autoPathIfNoOwerwrite : 1;
		};

	public:
		StateSavingOrders();
		StateSavingOrders(StateSavingOrders &&);
		~StateSavingOrders();

	public:
		Storm::StateSavingOrders::SavingSettings _settings;
		std::unique_ptr<Storm::SimulationState> _simulationState;
	};
}
