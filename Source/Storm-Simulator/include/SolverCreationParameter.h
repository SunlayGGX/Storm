#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	enum class SimulationMode;

	struct SolverCreationParameter
	{
	public:
		const Storm::SimulationMode _simulationMode;
		const float _kernelLength;
		const Storm::ParticleSystemContainer &_particleSystems;
	};
}
