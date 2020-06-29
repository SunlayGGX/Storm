#pragma once


namespace Storm
{
	enum class SimulationMode;
	enum class KernelMode;

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
		float _maxDensityError;

		bool _startPaused;

		Storm::SimulationMode _simulationMode;
		Storm::KernelMode _kernelMode;
	};
}