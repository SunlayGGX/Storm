#pragma once


namespace Storm
{
	enum class SimulationMode;
	enum class KernelMode;

	struct SceneSimulationConfig
	{
	public:
		SceneSimulationConfig();

	public:
		Storm::Vector3 _gravity;
		float _particleRadius;

		Storm::KernelMode _kernelMode;
		float _kernelCoefficient;
		float _kernelIncrementSpeedInSeconds;
		float _maxKernelIncrementCoeff;

		float _cflCoeff;
		float _maxCFLTime;
		int _maxCFLIteration;
		bool _computeCFL;

		float _physicsTimeInSec;
		float _expectedFps;

		unsigned int _maxPredictIteration;
		unsigned int _minPredictIteration;
		float _maxDensityError;

		bool _startPaused;

		bool _simulationNoWait;

		unsigned char _recomputeNeighborhoodStep;

		Storm::SimulationMode _simulationMode;
		std::string _simulationModeStr;

		bool _removeFluidParticleCollidingWithRb;

		bool _hasFluid;

		bool _fixRigidBodyAtStartTime;

		float _endSimulationPhysicsTimeInSeconds;
	};
}