#pragma once


namespace Storm
{
	struct SystemSimulationStateObject;
	struct SimulationState
	{
	public:
		~SimulationState();

	public:
		std::vector<Storm::SystemSimulationStateObject> _pSystemStates;
	};
}
