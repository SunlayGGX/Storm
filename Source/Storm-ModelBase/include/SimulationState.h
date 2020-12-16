#pragma once


namespace Storm
{
	struct SystemSimulationStateObject;
	struct SimulationState
	{
	public:
		~SimulationState();

	public:
		float _currentPhysicsTime;
		std::vector<Storm::SystemSimulationStateObject> _pSystemStates;
	};
}
