#pragma once


namespace Storm
{
	enum class SimulationMode;
	enum class KernelMode;
	enum class ViscosityMethod;
	enum class ParticleRemovalMode;

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

		bool _midUpdateViscosity;

		bool _startPaused;

		bool _simulationNoWait;

		unsigned char _recomputeNeighborhoodStep;

		Storm::SimulationMode _simulationMode;
		std::string _simulationModeStr;

		Storm::ViscosityMethod _fluidViscoMethod;
		Storm::ViscosityMethod _rbViscoMethod;

		bool _hasFluid;

		bool _fixRigidBodyAtStartTime;
		float _freeRbAtPhysicsTime;

		float _endSimulationPhysicsTimeInSeconds;

		bool _shouldRemoveRbCollidingPAtStateFileLoad;
		bool _considerRbWallAtCollingingPStateFileLoad;
		bool _removeFluidForVolumeConsistency;

		Storm::ParticleRemovalMode _fluidParticleRemovalMode;

		bool _noStickConstraint;
		bool _applyDragEffect;
		bool _useCoandaEffect;

		float _exitSimulationFloorLevel;
	};
}
