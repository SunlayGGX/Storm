#pragma once

#include "SPHBaseSolver.h"


namespace Storm
{
	class ParticleSystem;
	class ParticleSelector;

	class WCSPHSolver : public Storm::ISPHBaseSolver
	{
	public:
		void execute(Storm::ParticleSystemContainer &particleSystems, const float kernelLength, const float k_deltaTime) final override;
	};
}
