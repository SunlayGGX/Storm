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
			std::string _filePath;
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
