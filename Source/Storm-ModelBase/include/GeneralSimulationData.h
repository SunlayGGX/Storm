#pragma once


namespace Storm
{
	enum class SimulationMode;

	struct GeneralSimulationData
	{
	public:
		GeneralSimulationData();

	public:
		Storm::Vector3 _gravity;
		float _particleRadius;
		float _kernelCoefficient;

		float _physicsTimeInSeconds;
		float _expectedFps;

		unsigned int _maxPredictIteration;

		bool _startPaused;

		Storm::SimulationMode _simulationMode;
	};
}