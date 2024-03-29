#pragma once

#include "MassCoeffConfig.h"


namespace Storm
{
	enum class FluidParticleLoadDenseMode;
	struct SceneFluidDefaultCustomConfig;

	struct SceneFluidBlockConfig
	{
	public:
		SceneFluidBlockConfig();

	public:
		Storm::Vector3 _firstPoint;
		Storm::Vector3 _secondPoint;
		Storm::FluidParticleLoadDenseMode _loadDenseMode;
	};

	struct SceneFluidUnitParticleConfig
	{
	public:
		SceneFluidUnitParticleConfig();

	public:
		Storm::Vector3 _position;
	};

	struct SceneFluidConfig
	{
	public:
		SceneFluidConfig();

	public:
		unsigned int _fluidId;
		std::vector<Storm::SceneFluidBlockConfig> _fluidGenConfig;
		std::vector<Storm::SceneFluidUnitParticleConfig> _fluidUnitParticleGenConfig;
		float _density;
		float _particleVolume;
		float _dynamicViscosity;
		float _cinematicViscosity;
		float _soundSpeed;
		float _kPressureStiffnessCoeff; // k1 in the pressure state equation of Taits
		float _kPressureExponentCoeff; // k2 in the pressure state equation of Taits
		float _pressureInitRelaxationCoefficient;
		float _relaxationCoefficient;
		bool _gravityEnabled;
		bool _removeParticlesCollidingWithRb;
		bool _removeOutDomainParticles;
		bool _smoothingRestDensity;

		float _uniformDragCoefficient; // F = 1/2 rho * |vj - vi|(vj - vi) * Cd * Ai => _uniformDragCoefficient = 1/2 * Cd * Ai

		Storm::MassCoeffConfig _massCoeffControlConfig;

		std::unique_ptr<Storm::SceneFluidDefaultCustomConfig> _customSimulationSettings;
	};
}
